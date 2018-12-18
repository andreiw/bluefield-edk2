/** @file
  GUID used as HII FormSet and HII Package list GUID in the SysConfigDxe driver
  within the MlxPlatformPkg package.

Copyright (c) 2018, Mellanox Technologies. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available
under the terms and conditions of the BSD License that accompanies this
distribution. The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __MLX_SYS_CONFIG_HII_H__
#define __MLX_SYS_CONFIG_HII_H__

//
// Used for save password credential and form browser.
// Also used as provider identifier.
//
#define MLX_SYS_CONFIG_GUID \
  { \
    0x9c759c02, 0xe5a3, 0x45e4, { 0xac, 0xfc, 0xc3, 0x4a, 0x50, 0x06, 0x28, 0xa6 } \
  }

extern EFI_GUID gMlxSysConfigGuid;

#endif
