/** @file
  Implement EFI RealTimeClock runtime services.

  Copyright (c) 2017, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
  License which accompanies this distribution.  The full text of the license
  may be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/RealTimeClock.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/DxeServicesTableLib.h>

#include <Protocol/Rtc.h>

// Global variable for Set Virtual Address Change event.
STATIC EFI_EVENT mSetVirtualAddressChangeEvent = NULL;

// The I2C RTC interface from the Bluefield RTC protocol used
// to provide time and date related runtime services.
STATIC BLUEFIELD_RTC_PROTOCOL mI2cRtc;


STATIC BOOLEAN mBluefieldI2cRtcProtocolPresent = FALSE;

#define IS_LEAP_YEAR(yy) \
    ((yy) % 4 == 0 && ((yy) % 100 != 0 || (yy) % 400 == 0))

/**
  Retrieves the BlueField RTC protocol.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ABORTED           Could not retrieve gBluefieldRtcProtocolGuid.

**/
STATIC
EFI_STATUS
LocateBluefieldI2cRtcProtocol (
  VOID
  )
{
  BLUEFIELD_RTC_PROTOCOL *I2cRtc;
  EFI_STATUS              Status;

  // Find the Bluefield Rtc protocol. Currently we expect only one but if
  // there are multiple we need to further disambiguate.
  Status = gBS->LocateProtocol (&gBluefieldRtcProtocolGuid,
                                NULL,
                                (VOID **) &I2cRtc
                                );
  if (EFI_ERROR(Status))
    return EFI_ABORTED;

  ASSERT (I2cRtc          != NULL);
  ASSERT (I2cRtc->SetTime != NULL);
  ASSERT (I2cRtc->GetTime != NULL);
  mI2cRtc = *I2cRtc;

  mBluefieldI2cRtcProtocolPresent = TRUE;

  return EFI_SUCCESS;
}

/**
  Returns whether the given Time is valid.

  @param  Time                  A pointer to storage to receive a snapshot of
                                the current time.

  @retval TRUE                  Time is valid.
  @retval FALSE                 Time is invalid.

**/
STATIC
BOOLEAN
EFIAPI
IsTimeValid(
  IN EFI_TIME *Time
  )
{
  UINT8 MonthDayCount[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  if ( (Time->Year < 1900 || Time->Year > 2099)
    || (Time->Month < 1 || Time->Month > 12)
    || (Time->Day < 1 || Time->Day > MonthDayCount[Time->Month - 1]
      || (Time->Month == 2 && (!IS_LEAP_YEAR (Time->Year) && Time->Day > 28)))
    || (Time->Hour > 23)
    || (Time->Minute > 59)
    || (Time->Second > 59)
    || (Time->Nanosecond > 999999999)
    || ((Time->TimeZone != EFI_UNSPECIFIED_TIMEZONE)
      || ((Time->TimeZone < -1440 || Time->TimeZone > 1440)
        &&  Time->TimeZone != 2047))
    || (Time->Daylight & (~(EFI_TIME_ADJUST_DAYLIGHT | EFI_TIME_IN_DAYLIGHT)))
     ) {
    return FALSE;
  }

  return TRUE;
}

/**
  Returns the current time and date information, and the time-keeping
  capabilities of the hardware platform.

  @param  Time                  A pointer to storage to receive a snapshot of
                                the current time.
  @param  Capabilities          An optional pointer to a buffer to receive the
                                real time clock device's capabilities.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER Time is NULL.
  @retval EFI_DEVICE_ERROR      The time could not be retrieved due to hardware
                                error.

**/
EFI_STATUS
EFIAPI
GetTime (
  OUT EFI_TIME               *Time,
  OUT EFI_TIME_CAPABILITIES  *Capabilities
  )
{
  EFI_STATUS Status = EFI_SUCCESS;

  // Check whether the BlueField I2C RTC protocol is present.
  if (!mBluefieldI2cRtcProtocolPresent)
    return EFI_UNSUPPORTED;

  Status = mI2cRtc.GetTime (&mI2cRtc,
                            &Time->Year,
                            &Time->Month,
                            &Time->Day,
                            &Time->Hour,
                            &Time->Minute,
                            &Time->Second
                            );

  Time->Nanosecond  = 0;
  // *TBD* Get those info from System Variable?
  Time->Daylight    = 0;
  Time->TimeZone    = EFI_UNSPECIFIED_TIMEZONE;

  if (EFI_ERROR (Status))
    return EFI_DEVICE_ERROR;

  return EFI_SUCCESS;
}

/**
  Sets the current local time and date information.

  @param  Time                  A pointer to the current time.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The time could not be set due due to hardware
                                error.

**/
EFI_STATUS
EFIAPI
SetTime (
  IN EFI_TIME  *Time
  )
{
  EFI_STATUS Status = EFI_SUCCESS;

  // Check whether the BlueField I2C RTC protocol is present.
  if (!mBluefieldI2cRtcProtocolPresent)
    return EFI_UNSUPPORTED;

  // Check whether the input Time is valid, i.e. Time parameters are within
  // the range specified by UEFI.
  if (!IsTimeValid (Time))
      return EFI_INVALID_PARAMETER;

  Status = mI2cRtc.SetTime (&mI2cRtc,
                            Time->Year,
                            Time->Month,
                            Time->Day,
                            Time->Hour,
                            Time->Minute,
                            Time->Second
                            );

  return EFI_ERROR (Status) ? EFI_DEVICE_ERROR: Status;
}


/**
  Returns the current wakeup alarm clock setting.

  @param  Enabled               Indicates if the alarm is currently enabled or
                                disabled.
  @param  Pending               Indicates if the alarm signal is pending and
                                requires acknowledgement.
  @param  Time                  The current alarm setting.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Any parameter is NULL.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be retrieved due to
                                a hardware error.

**/
EFI_STATUS
EFIAPI
GetWakeupTime (
  OUT BOOLEAN     *Enabled,
  OUT BOOLEAN     *Pending,
  OUT EFI_TIME    *Time
  )
{
  // Not a required feature
  return EFI_UNSUPPORTED;
}


/**
  Sets the system wakeup alarm clock time.

  @param  Enabled               Enable or disable the wakeup alarm.
  @param  Time                  If Enable is TRUE, the time to set the wakeup
                                alarm for.

  @retval EFI_SUCCESS           If Enable is TRUE, then the wakeup alarm was
                                enabled. If Enable is FALSE, then the wakeup
                                alarm was disabled.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be set due to a
                                hardware error.
  @retval EFI_UNSUPPORTED       A wakeup timer is not supported on this
                                platform.

**/
EFI_STATUS
EFIAPI
SetWakeupTime (
  IN BOOLEAN      Enabled,
  OUT EFI_TIME    *Time
  )
{
  // Not a required feature
  return EFI_UNSUPPORTED;
}

/**
  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.

  This is a notification function registered on
  EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event. It converts pointers to new virtual
  address.

  @param  Event        Event whose notification function is being invoked.
  @param  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
RealTimeClockSetVirtualAddressNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  // The Real Time Clock driver supports the OS calling RTC functions in
  // virtual mode. Therefore, call EfiConvertPointer () to convert any stored
  // physical addresses to virtual addresses. After the OS transistions to
  // calling in virtual mode, all future runtime calls will be made in
  // virtual mode.
  EfiConvertPointer (0x0, (VOID **) &mI2cRtc.GetTime);
  EfiConvertPointer (0x0, (VOID **) &mI2cRtc.SetTime);
  EfiConvertPointer (0x0, (VOID **) &mI2cRtc);
}

/**
  This is the declaration of an EFI image entry point. This can be the entry
  point to an application written to this specification, an EFI boot service
  driver, or an EFI runtime driver.

  @param  ImageHandle           Handle that identifies the loaded image.
  @param  SystemTable           System Table for this image.

  @retval EFI_SUCCESS           The operation completed successfully.

**/
EFI_STATUS
EFIAPI
InitializeRealTimeClock (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  EFI_HANDLE  Handle;
  EFI_STATUS  Status;

  // Check whether the BlueField I2C RTC protocol is present.
  if (!mBluefieldI2cRtcProtocolPresent)
    LocateBluefieldI2cRtcProtocol ();

  SystemTable->RuntimeServices->GetTime       = GetTime;
  SystemTable->RuntimeServices->SetTime       = SetTime;
  SystemTable->RuntimeServices->GetWakeupTime = GetWakeupTime;
  SystemTable->RuntimeServices->SetWakeupTime = SetWakeupTime;

  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiRealTimeClockArchProtocolGuid,
                  NULL,
                  NULL
                  );

  ASSERT_EFI_ERROR (Status);

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  RealTimeClockSetVirtualAddressNotifyEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &mSetVirtualAddressChangeEvent
                  );

  ASSERT_EFI_ERROR (Status);

  return Status;
}

