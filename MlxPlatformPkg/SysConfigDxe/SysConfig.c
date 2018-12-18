/** @file
  SysConfig driver implementation.

Copyright (c) 2018, Mellanox Technologies. All rights reserved.

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/BaseCryptLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/Eeprom.h>

#include "SysConfig.h"

#define MAX_STRING_LEN          128

SYS_CONFIG_PRIVATE_DATA         *mPrivateData;
BOOLEAN                         mAuthChecked = FALSE;

MLX_SYS_CONFIG_HII_VENDOR_DEVICE_PATH(mHiiVendorDevicePath);

/**
  Encode the password using a simple algorithm.

  @param Password The string to be encoded.
  @param MaxSize  The size of the string.

**/
BOOLEAN
EncodePassword (
  IN      CHAR16                              *Password,
  IN      UINTN                               PasswordSize,
     OUT  UINT8                               *Credential
  )
{
  BOOLEAN           Status;
  UINTN             HashSize;
  VOID              *Hash;

  HashSize = Sha1GetContextSize ();
  Hash     = AllocatePool (HashSize);
  ASSERT (Hash != NULL);

  Status = Sha1Init (Hash);
  if (!Status) {
    goto Done;
  }

  Status = Sha1Update (Hash, Password, PasswordSize);
  if (!Status) {
    goto Done;
  }

  Status = Sha1Final (Hash, Credential);

Done:
  FreePool (Hash);
  return Status;
}

//
// Check whether old password is available
//
BOOLEAN
HasPassword (
  IN       SYS_CONFIG_PRIVATE_DATA      *PrivateData
  )
{
  UINTN                           Index;

  for (Index = 0; Index < sizeof (PrivateData->Configuration.Password); Index++) {
    if (PrivateData->Configuration.Password[Index] != 0) {
      return TRUE;
    }
  }

  return FALSE;
}


/**
  Validate the user's password.

  @param PrivateData This driver's private context data.
  @param StringId    The user's input.

  @retval EFI_SUCCESS   The user's input matches the password.
  @retval EFI_NOT_READY The user's input does not match the password.
**/
EFI_STATUS
ValidatePassword (
  IN       SYS_CONFIG_PRIVATE_DATA      *PrivateData,
  IN       EFI_STRING_ID                StringId
  )
{
  EFI_STATUS                      Status;
  CHAR16                          *Password;
  UINT8                           *EncodedPassword;

  //
  // Done if no password configured
  //
  if (!HasPassword(PrivateData)) {
    return EFI_SUCCESS;
  }

  //
  // Get user input password
  //
  Password = HiiGetString (PrivateData->HiiHandle, StringId, NULL);
  if (Password == NULL) {
    return EFI_NOT_READY;
  }

  //
  // Validate old password
  //
  EncodedPassword = AllocateZeroPool (CREDENTIAL_LEN);
  ASSERT (EncodedPassword != NULL);
  EncodePassword (Password, StrLen (Password) * sizeof (CHAR16), EncodedPassword);
  if (CompareMem (EncodedPassword, PrivateData->Configuration.Password, CREDENTIAL_LEN) != 0) {
    //
    // Old password mismatch, return EFI_NOT_READY to prompt for error message
    //
    Status = EFI_NOT_READY;
  } else {
    Status = EFI_SUCCESS;
  }

  FreePool (Password);
  FreePool (EncodedPassword);

  return Status;
}

STATIC EFI_STATUS
SaveConfig (
  SYS_CONFIG                      *Configuration
  )
{
  return gRT->SetVariable (
                  SYS_CONFIG_VAR,
                  &gMlxSysConfigGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS |
                    EFI_VARIABLE_RUNTIME_ACCESS,
                  sizeof (SYS_CONFIG),
                  Configuration
                  );
}

STATIC EFI_STATUS
ResetEfiStore (
  VOID
  )
{
  BLUEFIELD_EEPROM_PROTOCOL *I2cEeprom;
  EFI_STATUS                Status;
  UINTN                     StoreSize;
  UINT8                     *Store;
  CHAR16                    *StringBuffer;
  EFI_INPUT_KEY             Key;

  //
  // Popup a menu to confirm
  //
  StringBuffer = AllocateZeroPool (MAX_STRING_LEN * sizeof (CHAR16));
  if (StringBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  StrCpyS (
    StringBuffer,
    MAX_STRING_LEN,
    L"Press ENTER to confirm, or other key to cancel."
    );
  CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer, NULL);
  FreePool (StringBuffer);
  if (Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
    return EFI_ABORTED;
  }

  Status = gBS->LocateProtocol (&gBluefieldEepromProtocolGuid,
                                NULL, (VOID **) &I2cEeprom);
  if (! EFI_ERROR (Status)) {
    StoreSize = PcdGet32 (PcdVariableStoreSize);
    Store = (UINT8 *) AllocatePool (StoreSize + 1);

    if (Store != NULL) {
      //
      // Clears the Efi store. The Variable code will reinitialize
      // it during next reboot.
      //
      SetMem (Store, StoreSize, 0xff);
      Status = I2cEeprom->Transfer (I2cEeprom, 0, StoreSize,
                                    (UINT8 *) Store, EEPROM_WRITE);
      FreePool (Store);
    } else {
      Status = EFI_OUT_OF_RESOURCES;
    }
  }

  return Status;
}

/**
  Encode the password using a simple algorithm.

  @param PrivateData This driver's private context data.
  @param StringId    The password from User.

  @retval  EFI_SUCCESS The operation is successful.
  @return  Other value if fails.

**/
EFI_STATUS
SetPassword (
  IN SYS_CONFIG_PRIVATE_DATA      *PrivateData,
  IN EFI_STRING_ID                StringId
  )
{
  EFI_STATUS                      Status;
  CHAR16                          *Password;

  //
  // Get user input password and encode it.
  //
  Password = HiiGetString (PrivateData->HiiHandle, StringId, NULL);
  if (Password == NULL) {
    return EFI_NOT_READY;
  }
  if (StrLen (Password) == 0) {
    ZeroMem (PrivateData->Configuration.Password,
             sizeof(PrivateData->Configuration.Password));
  } else {
    EncodePassword (Password, StrLen (Password) * sizeof (CHAR16),
                    PrivateData->Configuration.Password);
  }
  FreePool (Password);

  //
  // Set password
  //
  Status = SaveConfig(&PrivateData->Configuration);
  return Status;
}

/**
  Get password from user input.

  @param[out]  Credential     Points to the input password.

**/
VOID
GetPassword (
  OUT CHAR8                                 *Credential
  )
{
  EFI_INPUT_KEY Key;
  CHAR16        PasswordMask[CREDENTIAL_LEN + 1];
  CHAR16        Password[CREDENTIAL_LEN];
  UINTN         PasswordLen;
  CHAR16        *QuestionStr;
  CHAR16        *LineStr;

  PasswordLen = 0;
  while (TRUE) {
    PasswordMask[PasswordLen]     = L'_';
    PasswordMask[PasswordLen + 1] = L'\0';
    LineStr = HiiGetString (mPrivateData->HiiHandle, STRING_TOKEN (STR_DRAW_A_LINE), NULL);
    QuestionStr = HiiGetString (mPrivateData->HiiHandle, STRING_TOKEN (STR_INPUT_PASSWORD), NULL);
    CreatePopUp (
      EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE,
      &Key,
      QuestionStr,
      LineStr,
      PasswordMask,
      NULL
      );
    FreePool (QuestionStr);
    FreePool (LineStr);

    //
    // Check key stroke
    //
    if (Key.ScanCode == SCAN_NULL) {
      if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
        break;
      } else if (Key.UnicodeChar == CHAR_BACKSPACE) {
        if (PasswordLen > 0) {
          PasswordLen--;
        }
      } else if ((Key.UnicodeChar == CHAR_NULL) ||
                 (Key.UnicodeChar == CHAR_TAB) ||
                 (Key.UnicodeChar == CHAR_LINEFEED)) {
        continue;
      } else {
        Password[PasswordLen] = Key.UnicodeChar;
        PasswordMask[PasswordLen] = L'*';
        PasswordLen++;
        if (PasswordLen == CREDENTIAL_LEN) {
          break;
        }
      }
    }
  }

  PasswordLen = PasswordLen * sizeof (CHAR16);
  EncodePassword (Password, PasswordLen, (UINT8 *)Credential);
}

/**
  This function is called to authenticate the user if the password has been
  configurated. It loops until the correct password is provided.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.

**/
void AuthCheck(EFI_HII_CONFIG_ACCESS_PROTOCOL *This)
{
  SYS_CONFIG_PRIVATE_DATA          *PrivateData;
  CHAR8                            Password[CREDENTIAL_LEN];
  EFI_INPUT_KEY                    Key;
  CHAR16                           *QuestionStr;
  CHAR16                           *PromptStr;

  PrivateData = SYS_CONFIG_PRIVATE_FROM_THIS (This);

  // Return if already checked or no password if configured.
  if (mAuthChecked || !HasPassword(PrivateData)) {
    return;
  }

  //
  // Get password from user.
  //
  while (TRUE) {
    //
    // Input password.
    //
    GetPassword (Password);

    //
    // Compare password consistency, and compare it against the saved password.
    //
    if (CompareMem (Password, PrivateData->Configuration.Password, CREDENTIAL_LEN) == 0) {
      break;
    }

    QuestionStr = HiiGetString (mPrivateData->HiiHandle, STRING_TOKEN (STR_PASSWORD_INCORRECT), NULL);
    PromptStr   = HiiGetString (mPrivateData->HiiHandle, STRING_TOKEN (STR_INPUT_PASSWORD_AGAIN), NULL);
    CreatePopUp (
      EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE,
      &Key,
      QuestionStr,
      L"",
      PromptStr,
      NULL
      );
    FreePool (QuestionStr);
    FreePool (PromptStr);
  }

  mAuthChecked = TRUE;
}

/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Request                A null-terminated Unicode string in
                                 <ConfigRequest> format.
  @param  Progress               On return, points to a character in the Request
                                 string. Points to the string's null terminator if
                                 request was successful. Points to the most recent
                                 '&' before the first failing name/value pair (or
                                 the beginning of the string if the failure is in
                                 the first name/value pair) if the request was not
                                 successful.
  @param  Results                A null-terminated Unicode string in
                                 <ConfigAltResp> format which has all values filled
                                 in for the names in the Request string. String to
                                 be allocated by the called function.

  @retval EFI_SUCCESS            The Results is filled with the requested values.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this
                                 driver.

**/
EFI_STATUS
EFIAPI
ExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                       Status;
  UINTN                            BufferSize;
  SYS_CONFIG_PRIVATE_DATA          *PrivateData;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;
  EFI_STRING                       ConfigRequest;
  EFI_STRING                       ConfigRequestHdr;
  UINTN                            Size;
  CHAR16                           *StrPointer;
  BOOLEAN                          AllocatedRequest;

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize the local variables.
  //
  *Progress         = Request;
  AllocatedRequest  = FALSE;

  PrivateData = SYS_CONFIG_PRIVATE_FROM_THIS (This);
  HiiConfigRouting = PrivateData->HiiConfigRouting;

  //
  // Get Buffer Storage data from EFI variable.
  // Try to get the current setting from variable.
  //
  BufferSize = sizeof (SYS_CONFIG);
  Status = gRT->GetVariable (
            SYS_CONFIG_VAR,
            &gMlxSysConfigGuid,
            NULL,
            &BufferSize,
            &PrivateData->Configuration
            );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  if (Request == NULL) {
    //
    // Request is set to NULL, construct full request string.
    //

    //
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    ConfigRequestHdr = HiiConstructConfigHdr (&gMlxSysConfigGuid, SYS_CONFIG_VAR, PrivateData->DriverHandle);
    Size = (StrLen (ConfigRequestHdr) + 32 + 1) * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (Size);
    ASSERT (ConfigRequest != NULL);
    AllocatedRequest = TRUE;
    UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, (UINT64)BufferSize);
    FreePool (ConfigRequestHdr);
  } else {
    //
    // Check routing data in <ConfigHdr>.
    // Note: if only one Storage is used, then this checking could be skipped.
    //
    if (!HiiIsConfigHdrMatch (Request, &gMlxSysConfigGuid, NULL)) {
      return EFI_NOT_FOUND;
    }

    //
    // Set Request to the unified request string.
    //
    ConfigRequest = Request;

    //
    // Check whether Request includes Request Element.
    //
    if (StrStr (Request, L"OFFSET") == NULL) {
      //
      // Check Request Element does exist in Reques String
      //
      StrPointer = StrStr (Request, L"PATH");
      if (StrPointer == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      if (StrStr (StrPointer, L"&") == NULL) {
        Size = (StrLen (Request) + 32 + 1) * sizeof (CHAR16);
        ConfigRequest    = AllocateZeroPool (Size);
        ASSERT (ConfigRequest != NULL);
        AllocatedRequest = TRUE;
        UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", Request, (UINT64)BufferSize);
      }
    }
  }

  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //
  Status = HiiConfigRouting->BlockToConfig (
                                HiiConfigRouting,
                                ConfigRequest,
                                (UINT8 *) &PrivateData->Configuration,
                                BufferSize,
                                Results,
                                Progress
                                );

  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    FreePool (ConfigRequest);
  }

  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  } else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

  return Status;
}


/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Configuration          A null-terminated Unicode string in <ConfigResp>
                                 format.
  @param  Progress               A pointer to a string filled in with the offset of
                                 the most recent '&' before the first failing
                                 name/value pair (or the beginning of the string if
                                 the failure is in the first name/value pair) or
                                 the terminating NULL if all was successful.

  @retval EFI_SUCCESS            The Results is processed successfully.
  @retval EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this
                                 driver.

**/
EFI_STATUS
EFIAPI
RouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  EFI_STATUS                       Status;
  UINTN                            BufferSize;
  SYS_CONFIG_PRIVATE_DATA          *PrivateData;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;

  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PrivateData = SYS_CONFIG_PRIVATE_FROM_THIS (This);
  HiiConfigRouting = PrivateData->HiiConfigRouting;
  *Progress = Configuration;

  //
  // Check routing data in <ConfigHdr>.
  // Note: if only one Storage is used, then this checking could be skipped.
  //
  if (!HiiIsConfigHdrMatch (Configuration, &gMlxSysConfigGuid, NULL)) {
    return EFI_NOT_FOUND;
  }

  //
  // Convert <ConfigResp> to buffer data by helper function ConfigToBlock()
  //
  BufferSize = sizeof (SYS_CONFIG);
  Status = HiiConfigRouting->ConfigToBlock (
                               HiiConfigRouting,
                               Configuration,
                               (UINT8 *) &PrivateData->Configuration,
                               &BufferSize,
                               Progress
                               );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Store Buffer Storage back to EFI variable
  //
  Status = SaveConfig(&PrivateData->Configuration);

  return Status;
}

/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Action                 Specifies the type of action taken by the browser.
  @param  QuestionId             A unique value which is sent to the original
                                 exporting driver so that it can identify the type
                                 of data to expect.
  @param  Type                   The type of value for the question.
  @param  Value                  A pointer to the data being sent to the original
                                 exporting driver.
  @param  ActionRequest          On return, points to the action requested by the
                                 callback function.

  @retval EFI_SUCCESS            The callback successfully handled the action.
  @retval EFI_OUT_OF_RESOURCES   Not enough storage is available to hold the
                                 variable and its data.
  @retval EFI_DEVICE_ERROR       The variable could not be saved.
  @retval EFI_UNSUPPORTED        The specified Action is not supported by the
                                 callback.

**/
EFI_STATUS
EFIAPI
DriverCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  SYS_CONFIG                   *Configuration;
  SYS_CONFIG_PRIVATE_DATA      *PrivateData;
  EFI_STATUS                   Status;

  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) &&
    (Action != EFI_BROWSER_ACTION_FORM_CLOSE)) || (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;
  PrivateData = SYS_CONFIG_PRIVATE_FROM_THIS (This);

  //
  // Retrive uncommitted data from Browser
  //
  Configuration = AllocateZeroPool (sizeof (SYS_CONFIG));
  ASSERT (Configuration != NULL);
  if (!HiiGetBrowserData (&gMlxSysConfigGuid, SYS_CONFIG_VAR, sizeof (SYS_CONFIG),
                         (UINT8 *) Configuration)) {
    FreePool (Configuration);
    Configuration = NULL;
  }

  switch (Action) {
  case EFI_BROWSER_ACTION_CHANGING:
    switch (QuestionId) {
    case KEY_SET_PASSWORD:
      //
      // Only used to update the state.
      //
      if ((Type == EFI_IFR_TYPE_STRING) && (Value->string == 0) &&
        (PrivateData->PasswordState == BROWSER_STATE_SET_PASSWORD)) {
        PrivateData->PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
        Status = EFI_INVALID_PARAMETER;
        goto done;
      }

      //
      // When try to set a new password, user will be chanlleged with old password.
      // The Callback is responsible for validating old password input by user,
      // If Callback return EFI_SUCCESS, it indicates validation pass.
      //
      switch (PrivateData->PasswordState) {
      case BROWSER_STATE_VALIDATE_PASSWORD:
        Status = ValidatePassword (PrivateData, Value->string);
        if (Status == EFI_SUCCESS) {
          PrivateData->PasswordState = BROWSER_STATE_SET_PASSWORD;
        }
        break;

      case BROWSER_STATE_SET_PASSWORD:
        Status = SetPassword (PrivateData, Value->string);
        CopyMem(Configuration->Password, PrivateData->Configuration.Password,
                CREDENTIAL_LEN);
        PrivateData->PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
        break;

      default:
        break;
      }
      break;

    default:
      break;
    }
    break;

  case EFI_BROWSER_ACTION_CHANGED:
    switch (QuestionId) {
    case KEY_RESET_EFI_VAR:
      Status = ResetEfiStore();
      if (Configuration != NULL && !EFI_ERROR (Status)) {
        SetMem (Configuration, sizeof (SYS_CONFIG), 0);
      }
      break;

    default:
      break;
    }
    break;

  default:
    Status = EFI_UNSUPPORTED;
    break;
  }

  if (Configuration != NULL && !EFI_ERROR (Status)) {
    //
    // Update uncommitted data of Browser
    //
    HiiSetBrowserData (
       &gMlxSysConfigGuid,
       SYS_CONFIG_VAR,
       sizeof (SYS_CONFIG),
       (UINT8 *) Configuration,
       NULL
       );
  }

done:
  if (Configuration != NULL) {
    FreePool (Configuration);
  }

  return Status;
}

/**
  Main entry for this driver.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to SystemTable.

  @retval EFI_SUCESS     This function always complete successfully.

**/
EFI_STATUS
EFIAPI
SysConfigInit (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                      Status;
  EFI_HII_CONFIG_ROUTING_PROTOCOL *HiiConfigRouting;
  EFI_FORM_BROWSER_EXTENSION_PROTOCOL *FormBrowserEx;
  EFI_INPUT_KEY                   HotKey;
  SYS_CONFIG                      *Configuration;
  UINTN                           BufferSize;
  BOOLEAN                         ActionFlag;
  EFI_STRING                      ConfigRequestHdr;
  SYS_CONFIG_PRIVATE_DATA         *PrivateData;

  //
  // Initialize driver private data
  //
  PrivateData = AllocateZeroPool (sizeof (SYS_CONFIG_PRIVATE_DATA));
  if (PrivateData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  PrivateData->Signature = SYS_CONFIG_SIGNATURE;
  PrivateData->ConfigAccess.ExtractConfig = ExtractConfig;
  PrivateData->ConfigAccess.RouteConfig = RouteConfig;
  PrivateData->ConfigAccess.Callback = DriverCallback;
  PrivateData->PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
  PrivateData->AuthCheck = AuthCheck;
  mPrivateData = PrivateData;

  //
  // Locate ConfigRouting protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &HiiConfigRouting);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  PrivateData->HiiConfigRouting = HiiConfigRouting;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &PrivateData->DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &PrivateData->ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Publish our HII data
  //
  PrivateData->HiiHandle = HiiAddPackages (
                   &gMlxSysConfigGuid,
                   PrivateData->DriverHandle,
                   SysConfigStrings,
                   SysConfigVfrBin,
                   NULL
                   );
  if (PrivateData->HiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize configuration data
  //
  Configuration = &PrivateData->Configuration;
  ZeroMem (Configuration, sizeof (SYS_CONFIG));

  //
  // Try to read NV config EFI variable first
  //
  ConfigRequestHdr = HiiConstructConfigHdr (&gMlxSysConfigGuid, SYS_CONFIG_VAR, PrivateData->DriverHandle);
  ASSERT (ConfigRequestHdr != NULL);

  BufferSize = sizeof (SYS_CONFIG);
  Status = gRT->GetVariable (SYS_CONFIG_VAR, &gMlxSysConfigGuid, NULL, &BufferSize, Configuration);
  if (EFI_ERROR (Status)) {
    //
    // Store zero data Buffer Storage to EFI variable
    //
    Status = SaveConfig(Configuration);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    ActionFlag = HiiSetToDefaults (ConfigRequestHdr, EFI_HII_DEFAULT_CLASS_STANDARD);
    if (!ActionFlag) {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    ActionFlag = HiiValidateSettings (ConfigRequestHdr);
    if (!ActionFlag) {
      DEBUG ((EFI_D_ERROR, "Failed to validate SysConfig.\n"));
    }
  }
  FreePool (ConfigRequestHdr);

  //
  // Remove the F9/F10 HotKeys since it causes confusion in some terminals.
  //
  Status = gBS->LocateProtocol (&gEfiFormBrowserExProtocolGuid, NULL, (VOID **) &FormBrowserEx);
  if (!EFI_ERROR (Status)) {
    HotKey.UnicodeChar = CHAR_NULL;
    HotKey.ScanCode    = SCAN_F9;
    FormBrowserEx->RegisterHotKey (&HotKey, 0, 0, NULL);
    HotKey.ScanCode    = SCAN_F10;
    FormBrowserEx->RegisterHotKey (&HotKey, 0, 0, NULL);
  }

  return EFI_SUCCESS;
}
