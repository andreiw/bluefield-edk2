/** @file
*  Function for determining whether we are running on simualtor.
*
*  Copyright (c) 2016, Mellanox Technologies. All rights reserved.
*
*  This program and the accompanying materials are licensed and made
*  available under the terms and conditions of the BSD License which
*  accompanies this distribution.  The full text of the license may be
*  found at http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS"
*  BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER
*  EXPRESS OR IMPLIED.
*
**/

#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include <Library/IoLib.h>

#include "rsh_def.h"

/**
  Return whether we are running on the simulator.

  @return whether we are running on the simulator.

**/
STATIC inline
BOOLEAN
IsSimulator (
  )
{
  STATIC UINT64 RshDevInfo = -1;

  if (RshDevInfo == -1) {
    RshDevInfo = MmioRead64 (PcdGet64 (PcdRshimBase) + RSH_DEV_INFO);
  }
  return (RshDevInfo & RSH_DEV_INFO__DEVICE_REV_MASK) == RSH_DEV_INFO__DEVICE_REV_MASK;
}

#endif
