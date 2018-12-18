/** @file
  Override BDS library function

Copyright (c) 2018, Mellanox Technologies. All rights reserved.<BR>

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "InternalBdsLib.h"
#include "String.h"

#include <Library/IpmiLib.h>
#include <IndustryStandard/Ipmi.h>

#define MAX_IPMB_PACKET_SIZE       33

/**
  This function interfaces to the Management Controller (e.g: BMC) to
  get the system boot options.

  This implements IPMI "Get System Boot Options" command.

  @param  BdsCommonOptionList   The header of the boot option list.
  @param  CompletionCode        The completion code of the IPMI command.

  @retval EFI_SUCCESS           Success get the system boot option
  @retval EFI_STATUS            Failed to get the system boot option

**/
EFI_STATUS
BdsGetBootOptionsFromManagementController (
  IN OUT IPMI_GET_THE_SYSTEM_BOOT_OPTIONS *IpmiBootOptions,
  IN OUT UINT8                            *CompletionCode
  )
{
  EFI_STATUS                    Status;
  UINT8                         NetFunction;
  UINT8                         Command;
  IPMI_GET_BOOT_OPTIONS_REQUEST RequestData;
  UINT32                        RequestDataSize;
  UINT8                         ResponseData[MAX_IPMB_PACKET_SIZE];
  UINT32                        ResponseDataSize;
  UINT32                        Size;

  NetFunction = IPMI_NETFN_CHASSIS;
  Command     = IPMI_CHASSIS_GET_SYSTEM_BOOT_OPTIONS;

  RequestData.ParameterSelector.Uint8 = IPMI_BOOT_OPTIONS_PARAMETER_BOOT_FLAGS;
  RequestData.SetSelector             = 0;
  RequestData.BlockSelector           = 0;
  RequestDataSize = sizeof (IPMI_GET_BOOT_OPTIONS_REQUEST);

  Status = IpmiSubmitCommand (
                    NetFunction,
                    Command,
                    (UINT8 *) &RequestData,
                    RequestDataSize,
                    ResponseData,
                    &ResponseDataSize);

  if (EFI_ERROR (Status))
    return Status;

  // The response data size of "Get System Boot Options" command must
  // include the Completion Code and the boot parameter bytes.
  Size = sizeof (IPMI_GET_THE_SYSTEM_BOOT_OPTIONS) + 1;
  // Completion code MUST be the first byte of the response data.
  //    00h = command completed normally
  //    80h = parameter not supported.
  // The response data should contain at least the Completion Code
  // byte of the IPMI command.
  *CompletionCode = ResponseData[0];

  if (*CompletionCode == 0 && ResponseDataSize == Size) {
    CopyMem (IpmiBootOptions, &ResponseData[1], Size - 1);
    // Verify whether the response is malformed.
    if (IpmiBootOptions->Parameter != 0x01 &&
         IpmiBootOptions->Valid != IPMI_BOOT_OPTIONS_PARAMETER_BOOT_FLAGS)
      return EFI_PROTOCOL_ERROR;
  }

  return (*CompletionCode == 0) ? EFI_SUCCESS : EFI_UNSUPPORTED;
}

/**
  This function returns whether the variable has a Messaging Device Path.

  @param  VariableName          EFI Variable name which indicates the Boot####

**/
BOOLEAN
BdsVariableHasMessagingDevicePath (
  IN  CHAR16                          *VariableName
  )
{
  UINT8                     *Variable;
  UINT8                     *TempPtr;
  UINTN                     VariableSize;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  BOOLEAN                   NetworkOption;

  //
  // Read the variable. We will never free this data.
  //
  Variable = BdsLibGetVariableAndSize (
              VariableName,
              &gEfiGlobalVariableGuid,
              &VariableSize
              );
  if (Variable == NULL) {
    return FALSE;
  }

  //
  // Validate Boot#### variable data.
  //
  if (!ValidateOption(Variable, VariableSize)) {
    FreePool (Variable);
    return FALSE;
  }

  //
  // Parse the Boot#### variable data and retrieve the device path.
  // One may consider use some macro to abstract the code.
  //
  TempPtr     =  Variable;
  // Skip the option attribute
  TempPtr    += sizeof (UINT32);
  // Skip the device path size
  TempPtr    += sizeof (UINT16);
  // Skip the option description string
  TempPtr    += StrSize((CHAR16 *) TempPtr);
  // Get the option's device path
  DevicePath  = (EFI_DEVICE_PATH_PROTOCOL *) TempPtr;

  //
  // The Messaging Device Path is used to describe the connection of devices
  // outside the resource domain of the system. This Device Path can abstract
  // information like networking protocol IP addresses. This Device Path might
  // be appended to ACPI Device Path. The ACPI Device Path is used to describe
  // devices whose enumeration is not described in an industry-standard
  // fashion. This Device Path is a linkage to the ACPI name space. So browse
  // the Device Path until a Messaging Device Path node is found.
  //
  if (DevicePathType (DevicePath) == ACPI_DEVICE_PATH &&
       DevicePathSubType (DevicePath) == ACPI_DP) {
    while (!IsDevicePathEnd (DevicePath) &&
            DevicePathType (DevicePath) != MESSAGING_DEVICE_PATH)
      DevicePath = NextDevicePathNode (DevicePath);
  }

  if (DevicePathType (DevicePath) != MESSAGING_DEVICE_PATH) {
    FreePool (Variable);
    return FALSE;
  }

  NetworkOption = FALSE;

  switch (DevicePathSubType (DevicePath)) {
  case MSG_MAC_ADDR_DP:
  case MSG_IPv4_DP:
  case MSG_IPv6_DP:
  case MSG_INFINIBAND_DP:
    NetworkOption = TRUE;
    break;

  default:
    NetworkOption = FALSE;
  }

  FreePool (Variable);
  return NetworkOption;
}

/**
  Apply the boot override when the boot device selector into PXE boot.

  The function looks up all existing network boot options and prepends
  them to the the least significant indexes of the boot list.

  @param  BdsCommonOptionList   The header of the boot option list.
  @param  VariableName          EFI Variable name which indicates the BootOrder.
  @param  NextBootOnly          If set to FALSE the boot options are
                                persistent; the BootOrder variable is
                                updated accordingly.

  @retval EFI_SUCCESS           Success create the boot option
  @retval EFI_STATUS            Failed to get the boot option

**/
EFI_STATUS
BdsBootOptionForcePxe (
  IN  LIST_ENTRY                      *BdsCommonOptionList,
  IN  CHAR16                          *VariableName,
  IN  BOOLEAN                          NextBootOnly
  )
{
  EFI_STATUS        Status;
  UINT16            *OptionOrder;
  UINTN             OptionOrderSize;
  UINT16            *OptionOrderTemp1, *OptionOrderTemp2;
  UINTN             OptionOrderTemp1Size, OptionOrderTemp2Size;
  UINTN             Index, IndexTemp1, IndexTemp2;
  BDS_COMMON_OPTION *Option;
  CHAR16            OptionName[20];

  //
  // Zero Buffer in order to get all BOOT#### variables
  //
  ZeroMem (OptionName, sizeof (OptionName));

  //
  // Read the BootOrder, or DriverOrder variable.
  //
  OptionOrder = BdsLibGetVariableAndSize (
                  VariableName,
                  &gEfiGlobalVariableGuid,
                  &OptionOrderSize
                  );
  if (OptionOrder == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Allocate a temporary option lists to sort out options. Note that
  // only network boot options are currently considered as PXE options.
  //
  OptionOrderTemp1 = AllocateZeroPool (OptionOrderSize);
  OptionOrderTemp2 = AllocateZeroPool (OptionOrderSize);
  // Initialize the size and indexes of the temporary lists.
  OptionOrderTemp1Size = IndexTemp1 = 0;
  OptionOrderTemp2Size = IndexTemp2 = 0;

  for (Index = 0; Index < OptionOrderSize / sizeof (UINT16); Index++) {
    UnicodeSPrint (
            OptionName,
            sizeof (OptionName),
            L"Boot%04x",
            OptionOrder[Index]
            );
    if (BdsVariableHasMessagingDevicePath (OptionName)) {
      OptionOrderTemp1[IndexTemp1++] = OptionOrder[Index];
      OptionOrderTemp1Size += sizeof (UINT16);
    } else {
      OptionOrderTemp2[IndexTemp2++] = OptionOrder[Index];
      OptionOrderTemp2Size += sizeof (UINT16);
    }
  }

  //
  // Override the boot order.
  //
  CopyMem (OptionOrder, OptionOrderTemp1, OptionOrderTemp1Size);
  CopyMem (OptionOrder + (OptionOrderTemp1Size / sizeof (UINT16)), OptionOrderTemp2,
                OptionOrderTemp2Size);

  FreePool (OptionOrderTemp1);
  FreePool (OptionOrderTemp2);

  //
  // Re initialize the boot options list head.
  //
  InitializeListHead (BdsCommonOptionList);

  for (Index = 0; Index < OptionOrderSize / sizeof (UINT16); Index++) {
    UnicodeSPrint (
            OptionName,
            sizeof (OptionName),
            L"Boot%04x",
            OptionOrder[Index]
            );
    Option = BdsLibVariableToOption (BdsCommonOptionList, OptionName);
    if (Option != NULL) {
      Option->BootCurrent = OptionOrder[Index];
    }
  }

  //
  // Update the BootOrder variable if the options are requested to be
  // persistent for all future boots.
  //
  if (NextBootOnly == FALSE) {
    Status = gRT->SetVariable (
                    VariableName,
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    OptionOrderSize,
                    OptionOrder
                    );
    //
    // Variable update with existing variable implementation shouldn't fail.
    //
    ASSERT_EFI_ERROR (Status);
  }

  FreePool (OptionOrder);

  return EFI_SUCCESS;
}

/**
  Get the system boot options for boot override.

  @param  BdsCommonOptionList   The header of the boot option list.
  @param  VariableName          EFI Variable name which indicates the BootOrder.

  @retval EFI_SUCCESS           Success create the boot option
  @retval EFI_STATUS            Failed to get the boot option

**/
EFI_STATUS
EFIAPI
BdsLibBootOptionOverride (
  IN  LIST_ENTRY                      *BdsCommonOptionList,
  IN  CHAR16                          *VariableName
  )
{
  IPMI_GET_THE_SYSTEM_BOOT_OPTIONS              IpmiBootOptions;
  IPMI_BOOT_OPTIONS_RESPONSE_PARAMETER_5_DATA_1 ParamData1;
  IPMI_BOOT_OPTIONS_RESPONSE_PARAMETER_5_DATA_2 ParamData2;
  EFI_STATUS                                    Status;
  UINT8                                         CompletionCode;
  BOOLEAN                                       NextBootOnly;

  //
  // First of all, issue the “Get System Boot Options” command to the BMC,
  // If the response contains boot override information, then the boot order
  // is adjusted based on the data. If the response shows there is no boot
  // override request, then return and follow the normal boot order.
  //
  Status = BdsGetBootOptionsFromManagementController (
                            &IpmiBootOptions,
                            &CompletionCode
                            );

  DEBUG ((EFI_D_INFO, "Get System Boot Options for Boot Override: %r\n",
            Status));

  if (EFI_ERROR (Status))
    return Status;

  //
  // Process the boot options; check whether a boot override is requested.
  //

  NextBootOnly = TRUE;

  //
  // Read Parameter 5 (Boot Flags) data. Process the first two data bytes
  // only. Ignore the rest of the data bytes; these are irrelevant for the
  // moment.
  // Boot flags 'Parameter Data 1' indicates whether the boot flags are valid
  // and whether the options are persistent or apply to the next boot only.
  // Boot flags 'Parameter Data 2' indicates the boot device selector.
  //
  ParamData1.Uint8 = IpmiBootOptions.Data1;
  ParamData2.Uint8 = IpmiBootOptions.Data2;

  if (!ParamData1.Bits.BootFlagValid)
    return EFI_ABORTED;

  if (ParamData1.Bits.PersistentOptions)
    NextBootOnly = FALSE;

  switch (ParamData2.Bits.BootDeviceSelector) {
  case IPMI_BOOT_DEVICE_SELECTOR_NO_OVERRIDE:
    return EFI_ABORTED; // Nothing to do here.

  case IPMI_BOOT_DEVICE_SELECTOR_PXE:
    Print (L"Boot override: force PXE\n");
    return BdsBootOptionForcePxe (
                        BdsCommonOptionList,
                        VariableName,
                        NextBootOnly
                        );

  case IPMI_BOOT_DEVICE_SELECTOR_HARDDRIVE:
  case IPMI_BOOT_DEVICE_SELECTOR_HARDDRIVE_SAFE_MODE:
  case IPMI_BOOT_DEVICE_SELECTOR_DIAGNOSTIC_PARTITION:
  case IPMI_BOOT_DEVICE_SELECTOR_CD_DVD:
  case IPMI_BOOT_DEVICE_SELECTOR_BIOS_SETUP:
  case IPMI_BOOT_DEVICE_SELECTOR_REMOTE_FLOPPY:
  case IPMI_BOOT_DEVICE_SELECTOR_REMOTE_CD_DVD:
  case IPMI_BOOT_DEVICE_SELECTOR_PRIMARY_REMOTE_MEDIA:
  case IPMI_BOOT_DEVICE_SELECTOR_REMOTE_HARDDRIVE:
  case IPMI_BOOT_DEVICE_SELECTOR_FLOPPY:
  default:
    return EFI_UNSUPPORTED;
  }

  return EFI_ABORTED;
}
