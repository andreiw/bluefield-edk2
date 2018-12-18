/** @file
  Copyright (c) 2012-2014, ARM Limited. All rights reserved.

  This program and the accompanying materials are licensed and made
  available under the terms and conditions of the BSD License which
  accompanies this distribution.  The full text of the license may be
  found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS"
  BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER
  EXPRESS OR IMPLIED.
**/

#ifndef __TMFIFO_DXE_H__
#define __TMFIFO_DXE_H__

#include <Uefi.h>
#include <Uefi/UefiSpec.h>
#include <Base.h>

// Protocols used by this driver
#include <Protocol/SimpleNetwork.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/PxeBaseCode.h>
#include <Protocol/DevicePath.h>

// Libraries used by this driver
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DevicePathLib.h>

#include "rsh_def.h"
#include "TmFifoLib.h"

#define TMFIFO_STALL     2

#define TMFIFO_MAX_PACKET_SIZE       0x600

#define TMFIFO_RX_DATA_SIZE          10560

#define TMFIFO_TX_RING_NUM_ENTRIES   32
#define TMFIFO_RX_PKT_SIZE           1536

#define TMFIFO_CONS_RING_SIZE        1024

/*------------------------------------------------------------------------------
  TMFIFO Information Structure
 -----------------------------------------------------------------------------*/
struct TMFIFO_DRIVER {
  // Driver signature
  UINT32            Signature;
  EFI_HANDLE        ControllerHandle;

  // EFI SNP protocol instances
  EFI_SIMPLE_NETWORK_PROTOCOL Snp;
  EFI_SIMPLE_NETWORK_MODE SnpMode;

  // EFI Snp statistics instance
  EFI_NETWORK_STATISTICS Stats;

  // Saved transmitted buffers so we can notify consumers when packets have
  // been sent.
  UINT16  PacketTag;
  UINT16  NextPacketTag;
  VOID    *TxRing[TMFIFO_TX_RING_NUM_ENTRIES];

  // Rx Packet
  UINT16 RxSize;
  TMFIFO_MSG_HDR RxHdr;
  UINT8  RxPkt[TMFIFO_RX_PKT_SIZE];

  // Console ring
  UINT8  ConsRing[TMFIFO_CONS_RING_SIZE];
  UINT16 ConsHead, ConsTail;

  // Console Tx word
  UINT64 ConsTxWord;
  UINT16 ConsTxSize;

  UINT32 RxBase;
  UINT32 TxBase;
};

#define TMFIFO_SIGNATURE             SIGNATURE_32('t', 'm', 'i', 'o')
#define INSTANCE_FROM_SNP_THIS(a)    CR(a, TMFIFO_DRIVER, Snp, TMFIFO_SIGNATURE)

/*------------------------------------------------------------------------------
  UEFI-Compliant functions for EFI_SIMPLE_NETWORK_PROTOCOL

  Refer to the Simple Network Protocol section (21.1) in the UEFI 2.3.1
  Specification for related definitions
 -----------------------------------------------------------------------------*/

/*
 *  UEFI Start() function
 *
 *  Parameters:
 *
 *  @param pobj:  A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
 *
 *  Description:
 *
 *  This function starts a network interface. If the network interface
 *  successfully starts, then EFI_SUCCESS will be returned.
 */
EFI_STATUS
EFIAPI
SnpStart (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp
  );

/*
 *  UEFI Stop() function
 *
 */
EFI_STATUS
EFIAPI
SnpStop (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp
  );

/*
 *  UEFI Initialize() function
 *
 */
EFI_STATUS
EFIAPI
SnpInitialize (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
  IN        UINTN rx_buff_size,
  IN        UINTN tx_buff_size
  );

/*
 *  UEFI Reset() function
 *
 */
EFI_STATUS
EFIAPI
SnpReset (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
  IN        BOOLEAN ext_ver
  );

/*
 *  UEFI Shutdown() function
 *
 */
EFI_STATUS
EFIAPI
SnpShutdown (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp
  );

/*
 *  UEFI ReceiveFilters() function
 *
 */
EFI_STATUS
EFIAPI
SnpReceiveFilters (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
  IN        UINT32 enable,
  IN        UINT32 disable,
  IN        BOOLEAN reset_mfilter,
  IN        UINTN num_mfilter,
  IN        EFI_MAC_ADDRESS *mfilter
  );

/*
 *  UEFI StationAddress() function
 *
 */
EFI_STATUS
EFIAPI
SnpStationAddress (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
  IN        BOOLEAN reset,
  IN        EFI_MAC_ADDRESS *new_maddr
  );

/*
 *  UEFI Statistics() function
 *
 */
EFI_STATUS
EFIAPI
SnpStatistics (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
  IN        BOOLEAN reset,
  IN  OUT   UINTN *stat_size,
      OUT   EFI_NETWORK_STATISTICS *stat_table
  );

/*
 *  UEFI MCastIPtoMAC() function
 *
 */
EFI_STATUS
EFIAPI
SnpMcastIptoMac (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
  IN        BOOLEAN use_ipv6,
  IN        EFI_IP_ADDRESS *ip_addr,
      OUT   EFI_MAC_ADDRESS *mac_addr
  );

/*
 *  UEFI NvData() function
 *
 */
EFI_STATUS
EFIAPI
SnpNvData (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
  IN        BOOLEAN read_write,
  IN        UINTN offset,
  IN        UINTN buff_size,
  IN  OUT   VOID *data
  );

/*
 *  UEFI GetStatus() function
 *
 */
EFI_STATUS
EFIAPI
SnpGetStatus (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
      OUT   UINT32 *irq_stat  OPTIONAL,
      OUT   VOID **tx_buff    OPTIONAL
  );

/*
 *  UEFI Transmit() function
 *
 */
EFI_STATUS
EFIAPI
SnpTransmit (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
  IN        UINTN hdr_size,
  IN        UINTN buff_size,
  IN        VOID *data,
  IN        EFI_MAC_ADDRESS *src_addr   OPTIONAL,
  IN        EFI_MAC_ADDRESS *dest_addr  OPTIONAL,
  IN        UINT16 *protocol            OPTIONAL
  );

/*
 *  UEFI Receive() function
 *
 */
EFI_STATUS
EFIAPI
SnpReceive (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
      OUT   UINTN *hdr_size               OPTIONAL,
  IN  OUT   UINTN *buff_size,
      OUT   VOID *data,
      OUT   EFI_MAC_ADDRESS *src_addr     OPTIONAL,
      OUT   EFI_MAC_ADDRESS *dest_addr    OPTIONAL,
      OUT   UINT16 *protocol              OPTIONAL
  );

/*------------------------------------------------------------------------------
  UEFI-Compliant functions for EFI_COMPONENT_NAME2_PROTOCOL

  Refer to the Component Name Protocol section (10.5) in the UEFI 2.3.1
  Specification for related definitions
 -----------------------------------------------------------------------------*/

/*
 *  UEFI GetDriverName() function
 *
 */
EFI_STATUS
EFIAPI
SnpGetDriverName (
  IN        EFI_COMPONENT_NAME2_PROTOCOL *Snp,
  IN        CHAR8 *Lang,
      OUT   CHAR16 **DriverName
  );

/*
 *  UEFI GetControllerName() function
 *
 */
EFI_STATUS
EFIAPI
SnpGetControllerName (
  IN        EFI_COMPONENT_NAME2_PROTOCOL *Cnp,
  IN        EFI_HANDLE ControllerHandle,
  IN        EFI_HANDLE ChildHandle            OPTIONAL,
  IN        CHAR8 *Lang,
      OUT   CHAR16 **ControllerName
  );

/*------------------------------------------------------------------------------
  Utility functions
 -----------------------------------------------------------------------------*/

EFI_MAC_ADDRESS
GetCurrentMacAddress (
  VOID
  );

#endif // __TMFIFO_DXE_H__
