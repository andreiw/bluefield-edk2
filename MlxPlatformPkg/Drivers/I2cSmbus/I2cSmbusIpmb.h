/** @file

  Copyright (c) 2018, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
  License which accompanies this distribution.  The full text of the license
  may be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __I2C_SMBUS_IPMB_H__
#define __I2C_SMBUS_IPMB_H__

#include <Uefi.h>

#include <IndustryStandard/Ipmi.h>

#include <Protocol/SmbusHc.h>
#include <Protocol/IpmiProtocol.h>

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Pi/PiI2c.h>

// Define the minimal number of retries the IPMB driver may issue if a given
// request failed.
#define MIN_IPMB_RETRY  1

#define MIN_IPMB_PACKET_SIZE       7    // Packet Header + Checksum
#define MAX_IPMB_PACKET_SIZE       33   // Packet Header + Data + Checksum
// The IPMB packet data size doesn't include the Checksum byte field.
#define MAX_IPMB_PACKET_DATA_SIZE  \
    (MAX_IPMB_PACKET_SIZE - MIN_IPMB_PACKET_SIZE - 1)

// The IPMB response data size doesn't include the checksum byte field nor
// the Completion Code byte.
#define MAX_IPMB_RESPONSE_DATA_SIZE (MAX_IPMB_PACKET_DATA_SIZE - 2)
// The minimum IPMB response size encapsulates the packet header, the
// Completion Code byte and the Checksum.
#define MIN_IPMB_RESPONSE_SIZE      (MIN_IPMB_PACKET_SIZE + 1)
// The maximum IPMB response size encapsulates the packet header, the
// Completion Code byte, the Data and the Checksum.
#define MAX_IPMB_RESPONSE_SIZE      \
    (MIN_IPMB_RESPONSE_SIZE + MAX_IPMB_PACKET_DATA_SIZE + 1)

// This is the generic IPMB packet format, the final checksum can't be
// represented in this structure and will show up as the last data byte.
typedef struct {
  UINT8 RsSa;       // Responser I2C slave address.
  UINT8 NfLn;       // The network function to be accessed within the target.
  UINT8 ChkSum1;    // Checksum, calculated over the header (RsSa, NfLn).
  UINT8 RqSa;       // Requester I2C slave address.
  UINT8 SeqLn;      // Sequence field.
  UINT8 Cmd;        // Command byte as required by the network function.
  UINT8 Data[MAX_IPMB_PACKET_DATA_SIZE + 1]; // Data bytes and checksum byte.
} IPMB_REQUEST_PACKET;

// This is the standard IPMB response format where the first byte of
// IPMB packet data is interpreted as a command completion code.
typedef struct {
  UINT8 RqSa;       // Requester I2C slave address.
  UINT8 NfLn;       // The network function to be accessed within the target.
  UINT8 ChkSum1;    // Checksum, calculated over the header (RsSa, NfLn).
  UINT8 RsSa;       // Responser I2C slave address.
  UINT8 SeqLn;      // Sequence field.
  UINT8 Cmd;        // Command byte.
  UINT8 CompCode;   // Completion code.
  UINT8 Data[MAX_IPMB_RESPONSE_DATA_SIZE + 1]; // Data bytes and checksum byte.
} IPMB_RESPONSE_PACKET;

#define IPMB_SIGNATURE          SIGNATURE_32 ('I', 'P', 'M', 'B')

// Encapsulates IPMB context information.
typedef struct {
  UINT32                         Signature;
  EFI_HANDLE                     ControllerHandle;
  UINT8                          I2cSmbus;
  UINT8                          SlaveAddress;
  UINT8                          BmcAddress;
  EFI_SMBUS_HC_EXECUTE_OPERATION I2cSmbusExecute;
  IPMI_PROTOCOL                  IpmiProtocol;
} IPMB_CONTEXT;

extern IPMB_CONTEXT gIpmbContext;

/**
  This service enables submitting commands via Ipmi.

  @param[in]         This              This point for IPMI_PROTOCOL structure.
  @param[in]         NetFunction       Net function of the command.
  @param[in]         Command           IPMI Command.
  @param[in]         RequestData       Command Request Data.
  @param[in]         RequestDataSize   Size of Command Request Data.
  @param[out]        ResponseData      Command Response Data. The completion
                                       code is the first byte of response data.
  @param[in, out]    ResponseDataSize  Size of Command Response Data.

  @retval EFI_SUCCESS            The command byte stream was successfully
                                 submit to the device and a response was
                                 successfully received.
  @retval EFI_NOT_FOUND          The command was not successfully sent to
                                 the device or a response was not successfully
                                 received from the device.
  @retval EFI_NOT_READY          Ipmi Device is not ready for Ipmi command
                                 access.
  @retval EFI_DEVICE_ERROR       Ipmi Device hardware error.
  @retval EFI_TIMEOUT            The command time out.
  @retval EFI_UNSUPPORTED        The command was not successfully sent to the
                                 device.
  @retval EFI_OUT_OF_RESOURCES   The resource allcation is out of resource or
                                 data size error.
**/
EFI_STATUS
EFIAPI
I2cSmbusIpmiSubmitCommand (
  IN     IPMI_PROTOCOL                 *This,
  IN     UINT8                         NetFunction,
  IN     UINT8                         Command,
  IN     UINT8                         *RequestData,
  IN     UINT32                        RequestDataSize,
     OUT UINT8                         *ResponseData,
  IN OUT UINT32                        *ResponseDataSize
  );

EFI_STATUS
EFIAPI
I2cSmbusInstallIpmbProtocol (
  VOID
  );

#endif // __I2C_SMBUS_IPMB_H__
