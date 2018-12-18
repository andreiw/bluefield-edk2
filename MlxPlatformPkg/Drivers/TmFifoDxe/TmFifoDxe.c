/** @file
  Copyright (c) 2017, Mellanox Technologies. All rights reserved.
  Copyright (c) 2012-2014, ARM Limited. All rights reserved.

  This program and the accompanying materials are licensed and made
  available under the terms and conditions of the BSD License which
  accompanies this distribution.  The full text of the license may be
  found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS"
  BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER
  EXPRESS OR IMPLIED.
**/

#include "TmFifoDxe.h"
#include "BlueFieldPlatform.h"
#include "BlueFieldEfiInfo.h"
#include <Library/NetLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

typedef struct {
  MAC_ADDR_DEVICE_PATH      TmFifo;
  EFI_DEVICE_PATH_PROTOCOL  End;
} TMFIFO_DEVICE_PATH;

TMFIFO_DEVICE_PATH TmFifoPathTemplate =  {
  {
    {
      MESSAGING_DEVICE_PATH, MSG_MAC_ADDR_DP,
      {
        (UINT8) (sizeof (MAC_ADDR_DEVICE_PATH)),
        (UINT8) ((sizeof (MAC_ADDR_DEVICE_PATH)) >> 8)
      }
    },
    { { 0 } },
    0
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    { sizeof (EFI_DEVICE_PATH_PROTOCOL), 0 }
  }
};

static UINT8 DefaultMac[NET_ETHER_ADDR_LEN] = {
  0x00, 0x1A, 0xCA, 0xFF, 0xFF, 0x01
  };

static UINT8 BCastMac[NET_ETHER_ADDR_LEN] = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };

/*
 *  UEFI Start() function
 *
 *  Parameters:
 *
 *  @param Snp:  A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
 *
 *  Description:
 *
 *    This function starts a network interface. If the network interface
 *    successfully starts, then EFI_SUCCESS will be returned.
 */
EFI_STATUS
EFIAPI
TmFifoSnpStart (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
 )
{
  // Check Snp instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Check state
  if ((Snp->Mode->State == EfiSimpleNetworkStarted)    ||
      (Snp->Mode->State == EfiSimpleNetworkInitialized)  ) {
    return EFI_ALREADY_STARTED;
  }

  // Change state
  Snp->Mode->State = EfiSimpleNetworkStarted;
  return EFI_SUCCESS;
}

/*
 *  UEFI Stop() function
 *
 */
EFI_STATUS
EFIAPI
TmFifoSnpStop (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp
  )
{
  // Check Snp Instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Check state of the driver
  if (Snp->Mode->State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  // Change the state
  switch (Snp->Mode->State) {
    case EfiSimpleNetworkStarted:
    case EfiSimpleNetworkInitialized:
      Snp->Mode->State = EfiSimpleNetworkStopped;
      break;
    default:
      return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}


/*
 *  UEFI Initialize() function
 *
 */
EFI_STATUS
EFIAPI
TmFifoSnpInitialize (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
  IN        UINTN                        RxBufferSize    OPTIONAL,
  IN        UINTN                        TxBufferSize    OPTIONAL
  )
{
  MLNX_EFI_INFO *Info = (MLNX_EFI_INFO *)MLNX_EFI_INFO_ADDR;

  // Check Snp Instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // First check that driver has not already been initialized
  if (Snp->Mode->State == EfiSimpleNetworkInitialized) {
    DEBUG ((EFI_D_WARN, "TMFIFO Driver already initialized\n"));
    return EFI_SUCCESS;
  } else if (Snp->Mode->State == EfiSimpleNetworkStopped) {
    DEBUG ((EFI_D_WARN, "TMFIFO Driver not started\n"));
    return EFI_NOT_STARTED;
  }

  // Initiate the FIFO
  if (!Info->TmFifoInit) {
    Info->TmFifoInit = 1;
    TmFifoInit ();
  }

  // Write the current configuration to the register
  gBS->Stall (TMFIFO_STALL);

  // Check that a buff size was specified
  if (TxBufferSize > 0) {
    if (RxBufferSize == 0) {
      RxBufferSize = TMFIFO_RX_DATA_SIZE;
    }
  }

  // Declare the driver as initialized
  Snp->Mode->State = EfiSimpleNetworkInitialized;

  return EFI_SUCCESS;
}

/*
 *  UEFI Reset () function
 *
 */
EFI_STATUS
EFIAPI
TmFifoSnpReset (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
  IN        BOOLEAN Verification
  )
{
  // Check Snp Instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // First check that driver has not already been initialized
  if (Snp->Mode->State == EfiSimpleNetworkStarted) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver not yet initialized\n"));
    return EFI_DEVICE_ERROR;
  } else if (Snp->Mode->State == EfiSimpleNetworkStopped) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver not started\n"));
    return EFI_NOT_STARTED;
  }

  // Initiate the FIFO
  TmFifoInit();

  return EFI_SUCCESS;
}

/*
 *  UEFI Shutdown () function
 *
 */
EFI_STATUS
EFIAPI
TmFifoSnpShutdown (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp
  )
{
  // Check Snp Instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // First check that driver has not already been initialized
  if (Snp->Mode->State == EfiSimpleNetworkStarted) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver not yet initialized\n"));
    return EFI_DEVICE_ERROR;
  } else if (Snp->Mode->State == EfiSimpleNetworkStopped) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver not started\n"));
    return EFI_NOT_STARTED;
  }

  // Back to the started and thus not initialized state
  Snp->Mode->State = EfiSimpleNetworkStarted;

  return EFI_SUCCESS;
}

/**
  Enable and/or disable the receive filters of the TmFifo

  Please refer to the UEFI specification for the precedence rules among the
  Enable, Disable and ResetMCastFilter parameters.

  @param[in]  Snp               A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL
                                instance.
  @param[in]  Enable            A bit mask of receive filters to enable.
  @param[in]  Disable           A bit mask of receive filters to disable.
  @param[in]  ResetMCastFilter  Set to TRUE to reset the contents of the
                                multicast receive filters on the network
                                interface to their default values.
  @param[in]  MCastFilterCnt    Number of multicast HW MAC addresses in the new
                                MCastFilter list. This value must be less than
                                or equal to the MCastFilterCnt field of
                                EFI_SIMPLE_NETWORK_MODE. This field is optional
                                if ResetMCastFilter is TRUE.
  @param[in]  MCastFilter       A pointer to a list of new multicast receive
                                filter HW MAC addresses. This list will replace
                                any existing multicast HW MAC address list. This
                                field is optional if ResetMCastFilter is TRUE.

  @retval  EFI_SUCCESS            The receive filters of the TmFifo were updated.
  @retval  EFI_NOT_STARTED        The TmFifo has not been started.
  @retval  EFI_INVALID_PARAMETER  One or more of the following conditions is
                                  TRUE :
                                  . This is NULL
                                  . Multicast is being enabled (the
                                    EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST bit is
                                    set in Enable, it is not set in Disable, and
                                    ResetMCastFilter is FALSE) and
                                    MCastFilterCount is zero.
                                  . Multicast is being enabled and
                                    MCastFilterCount is greater than
                                    Snp->Mode->MaxMCastFilterCount.
                                  . Multicast is being enabled and MCastFilter
                                    is NULL
                                  . Multicast is being enabled and one or more
                                    of the addresses in the MCastFilter list are
                                    not valid multicast MAC addresses.
  @retval  EFI_DEVICE_ERROR       The TmFifo has been started but not
                                  initialized.

**/
EFI_STATUS
EFIAPI
TmFifoSnpReceiveFilters (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  UINT32                       Enable,
  IN  UINT32                       Disable,
  IN  BOOLEAN                      ResetMCastFilter,
  IN  UINTN                        MCastFilterCnt  OPTIONAL,
  IN  EFI_MAC_ADDRESS              *MCastFilter  OPTIONAL
  )
{
  EFI_SIMPLE_NETWORK_MODE  *Mode;

  // Check Snp Instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Mode = Snp->Mode;

  // Check that driver was started and initialized
  if (Mode->State == EfiSimpleNetworkStarted) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver not initialized\n"));
    return EFI_DEVICE_ERROR;
  } else if (Mode->State == EfiSimpleNetworkStopped) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver in stopped state\n"));
    return EFI_NOT_STARTED;
  }

  if ((Enable  & (~Mode->ReceiveFilterMask)) ||
      (Disable & (~Mode->ReceiveFilterMask))) {
    return EFI_INVALID_PARAMETER;
  }

  // Update the receive filter.
  // Ignore the multicast filter settings for now.
  Mode->ReceiveFilterSetting = (Mode->ReceiveFilterSetting | Enable) &
                                 (~Disable);

  return EFI_SUCCESS;
}

/**
  Modify or reset the current station address

  @param[in]  Snp               A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL
                                instance.
  @param[in]  Reset             Flag used to reset the station address to the
                                TMFIFO's permanent address.
  @param[in]  New               New station address to be used for the network
                                interface.

  @retval  EFI_SUCCESS            The TMFIFO's station address was updated.
  @retval  EFI_NOT_STARTED        The TMFIFO has not been started.
  @retval  EFI_INVALID_PARAMETER  One or more of the following conditions is
                                  TRUE :
                                  . The "New" station address is invalid.
                                  . "Reset" is FALSE and "New" is NULL.
  @retval  EFI_DEVICE_ERROR       The TMFIFO has been started but not
                                  initialized.

**/
EFI_STATUS
EFIAPI
TmFifoSnpStationAddress (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  BOOLEAN                      Reset,
  IN  EFI_MAC_ADDRESS              *New
)
{
  DEBUG ((DEBUG_NET, "SnpStationAddress()\n"));

  // Check Snp instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Check that driver was started and initialized
  if (Snp->Mode->State == EfiSimpleNetworkStarted) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver not initialized\n"));
    return EFI_DEVICE_ERROR;
  } else if (Snp->Mode->State == EfiSimpleNetworkStopped) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver in stopped state\n"));
    return EFI_NOT_STARTED;
  }

  // Get the Permanent MAC address if need reset
  if (Reset) {
    New = (EFI_MAC_ADDRESS*) DefaultMac;
  } else {
    // Otherwise use the specified new MAC address
    if (New == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    // Not valid if multicast or broadcast address.
    if (New->Addr[0] & 0x01) {
      return EFI_INVALID_PARAMETER;
    }
  }

  CopyMem (&Snp->Mode->CurrentAddress, New, NET_ETHER_ADDR_LEN);

  return EFI_SUCCESS;
}

/*
 *  UEFI Statistics() function
 *
 */
EFI_STATUS
EFIAPI
TmFifoSnpStatistics (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
  IN        BOOLEAN Reset,
  IN  OUT   UINTN *StatSize,
      OUT   EFI_NETWORK_STATISTICS *Statistics
  )
{
  TMFIFO_DRIVER  *LanDriver;
  EFI_STATUS      Status;

  // Check Snp instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  LanDriver = INSTANCE_FROM_SNP_THIS (Snp);

  DEBUG ((DEBUG_NET, "SnpStatistics()\n"));

  // Check that driver was started and initialized
  if (Snp->Mode->State == EfiSimpleNetworkStarted) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver not initialized\n"));
    return EFI_DEVICE_ERROR;
  } else if (Snp->Mode->State == EfiSimpleNetworkStopped) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver in stopped state\n"));
    return EFI_NOT_STARTED;
  }

  // Reset the statistics.
  if (Reset) {
    ZeroMem (&LanDriver->Stats, sizeof (EFI_NETWORK_STATISTICS));
  }

  Status = EFI_SUCCESS;
  if (StatSize == NULL) {
    if (Statistics != NULL) {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    if (Statistics == NULL) {
      Status = EFI_BUFFER_TOO_SMALL;
    } else {
      // Fill in the statistics
      CopyMem (
        Statistics, &LanDriver->Stats,
        MIN (*StatSize, sizeof (EFI_NETWORK_STATISTICS))
        );
      if (*StatSize < sizeof (EFI_NETWORK_STATISTICS)) {
        Status = EFI_BUFFER_TOO_SMALL;
      }
    }
    *StatSize = sizeof (EFI_NETWORK_STATISTICS);
  }

  return Status;
}


/*
 *  UEFI MCastIPtoMAC() function
 *
 */
EFI_STATUS
EFIAPI
TmFifoSnpMcastIptoMac (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
  IN        BOOLEAN IsIpv6,
  IN        EFI_IP_ADDRESS *Ip,
      OUT   EFI_MAC_ADDRESS *McastMac
  )
{
  DEBUG ((DEBUG_NET, "SnpMcastIptoMac()\n"));

  // Check Snp instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Check that driver was started and initialized
  if (Snp->Mode->State == EfiSimpleNetworkStarted) {
    DEBUG ((EFI_D_WARN, "Warning: TmFifo Driver not initialized\n"));
    return EFI_DEVICE_ERROR;
  } else if (Snp->Mode->State == EfiSimpleNetworkStopped) {
    DEBUG ((EFI_D_WARN, "Warning: TmFifo Driver in stopped state\n"));
    return EFI_NOT_STARTED;
  }

  // Check parameters
  if ((McastMac == NULL) || (Ip == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  // Make sure MAC address is empty
  ZeroMem (McastMac, sizeof (EFI_MAC_ADDRESS));

  // If we need ipv4 address
  if (!IsIpv6) {
    // Most significant 25 bits of a multicast HW address are set.
    // 01-00-5E is the IPv4 Ethernet Multicast Address (see RFC 1112)
    McastMac->Addr[0] = 0x01;
    McastMac->Addr[1] = 0x00;
    McastMac->Addr[2] = 0x5E;

    // Lower 23 bits from ipv4 address
    // Clear the most significant bit (25th bit of MAC must be 0)
    McastMac->Addr[3] = (Ip->v4.Addr[1] & 0x7F);
    McastMac->Addr[4] = Ip->v4.Addr[2];
    McastMac->Addr[5] = Ip->v4.Addr[3];
  } else {
    // Most significant 16 bits of multicast v6 HW address are set
    // 33-33 is the IPv6 Ethernet Multicast Address (see RFC 2464)
    McastMac->Addr[0] = 0x33;
    McastMac->Addr[1] = 0x33;

    // lower four octets are taken from ipv6 address
    McastMac->Addr[2] = Ip->v6.Addr[8];
    McastMac->Addr[3] = Ip->v6.Addr[9];
    McastMac->Addr[4] = Ip->v6.Addr[10];
    McastMac->Addr[5] = Ip->v6.Addr[11];
  }

  return EFI_SUCCESS;
}


/*
 *  UEFI NvData() function
 *
 */
EFI_STATUS
EFIAPI
TmFifoSnpNvData (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* pobj,
  IN        BOOLEAN read_write,
  IN        UINTN offset,
  IN        UINTN buff_size,
  IN  OUT   VOID *data
  )
{
  DEBUG ((DEBUG_NET, "SnpNvData()\n"));

  return EFI_UNSUPPORTED;
}


/*
 *  UEFI GetStatus () function
 *
 */
EFI_STATUS
EFIAPI
TmFifoSnpGetStatus (
  IN   EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  OUT  UINT32                       *IrqStat  OPTIONAL,
  OUT  VOID                         **TxBuff  OPTIONAL
  )
{
  UINT16          PacketTag;
  TMFIFO_DRIVER *LanDriver;

  // Check preliminaries
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  LanDriver = INSTANCE_FROM_SNP_THIS (Snp);

  // Check that driver was started and initialized
  if (Snp->Mode->State == EfiSimpleNetworkStarted) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver not initialized\n"));
    return EFI_DEVICE_ERROR;
  } else if (Snp->Mode->State == EfiSimpleNetworkStopped) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver in stopped state\n"));
    return EFI_NOT_STARTED;
  }

  if (LanDriver->PacketTag != LanDriver->NextPacketTag) {
    PacketTag = LanDriver->PacketTag;
    if (TxBuff != NULL) {
      *TxBuff = LanDriver->TxRing[PacketTag % TMFIFO_TX_RING_NUM_ENTRIES];
    }
    LanDriver->PacketTag++;
  } else if (TxBuff != NULL) {
      *TxBuff = NULL;
  }

  return EFI_SUCCESS;
}


/*
 *  UEFI Transmit() function
 *
 */
EFI_STATUS
EFIAPI
TmFifoSnpTransmit (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  UINTN                        HdrSize,
  IN  UINTN                        BuffSize,
  IN  VOID*                        Buff,
  IN  EFI_MAC_ADDRESS              *SrcAddr  OPTIONAL,
  IN  EFI_MAC_ADDRESS              *DstAddr  OPTIONAL,
  IN  UINT16                       *Protocol OPTIONAL
  )
{
  TMFIFO_DRIVER *LanDriver;
  UINT32 TxFreeSpace;
  UINT16 PacketTag, LocalProtocol;
  TMFIFO_MSG_HDR Hdr;
  UINT64 Value;
  UINT8 *Data = (UINT8 *)Buff;

  // Check preliminaries
  if ((Snp == NULL) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  LanDriver = INSTANCE_FROM_SNP_THIS (Snp);

  // Check that driver was started and initialized
  if (Snp->Mode->State == EfiSimpleNetworkStarted) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver not initialized\n"));
    return EFI_DEVICE_ERROR;
  } else if (Snp->Mode->State == EfiSimpleNetworkStopped) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver in stopped state\n"));
    return EFI_NOT_STARTED;
  }

  // Ensure header is correct size if non-zero
  if (HdrSize) {
    if (HdrSize != Snp->Mode->MediaHeaderSize) {
      return EFI_INVALID_PARAMETER;
    }

    if ((DstAddr == NULL) || (Protocol == NULL)) {
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Check validity of BufferSize. It should be at least 16 bytes.
  //
  if (BuffSize < Snp->Mode->MediaHeaderSize || BuffSize < 2 * sizeof (UINT64)) {
      return EFI_BUFFER_TOO_SMALL;
  }

  // Get FIFO free space in bytes
  TxFreeSpace = TmFifoTxAvail (8) * sizeof (UINT64);
  if (TxFreeSpace < BuffSize + sizeof (TMFIFO_MSG_HDR)) {
    return EFI_NOT_READY;
  }

  // Get header information from data buffer if not provided.

  if (HdrSize == 0 || DstAddr == NULL) {
    DstAddr = (EFI_MAC_ADDRESS *) Data;
  }
  if (HdrSize == 0 || SrcAddr == NULL) {
    SrcAddr = (EFI_MAC_ADDRESS *) (Data + NET_ETHER_ADDR_LEN);
  }
  if (HdrSize == 0 || Protocol == NULL) {
    LocalProtocol = *(UINT16 *) (Data + 12);
  } else {
    LocalProtocol = HTONS(*Protocol);
  }

  // Check for the nature of the frame
  if (NET_MAC_IS_MULTICAST(DstAddr, BCastMac, NET_ETHER_ADDR_LEN)) {
    LanDriver->Stats.TxMulticastFrames += 1;
  } else if (DstAddr->Addr[0] == 0xFF) {
    LanDriver->Stats.TxBroadcastFrames += 1;
  } else {
    LanDriver->Stats.TxUnicastFrames += 1;
  }

  // Write the packet header
  Hdr.Data = 0;
  Hdr.Type = VIRTIO_ID_NET;
  Hdr.Len = HTONS(BuffSize);
  TmFifoWrite64 (RSH_TM_TILE_TO_HOST_DATA, Hdr.Data);

  // Write Dst-MAC (6B) + Src-MAC (2B)
  CopyMem (&Value, DstAddr->Addr, NET_ETHER_ADDR_LEN);
  CopyMem ((UINT8*)&Value + 6, SrcAddr->Addr, 2);
  TmFifoWrite64 (RSH_TM_TILE_TO_HOST_DATA, Value);

  // Write Src-MAC (4B) + Protocol (2B) + Data (2B)
  CopyMem ((UINT8*)&Value, (UINT8*)SrcAddr->Addr + 2, 4);
  *(UINT16*)((UINT8*)&Value + 4) = LocalProtocol;
  CopyMem ((UINT8*)&Value + 6, Data + 14, 2);
  TmFifoWrite64 (RSH_TM_TILE_TO_HOST_DATA, Value);

  // Write the 8-byte blocks
  BuffSize -= 2 * sizeof (UINT64);
  Data += 2 * sizeof (UINT64);
  while (BuffSize >= sizeof (UINT64)) {
    CopyMem ((UINT8*)&Value, Data, sizeof (UINT64));
    TmFifoWrite64 (RSH_TM_TILE_TO_HOST_DATA, Value);
    BuffSize -= sizeof (UINT64);
    Data += sizeof (UINT64);
  }

  // Write the leftover data and padded it to 8 bytes.
  if (BuffSize > 0) {
    Value = 0;
    CopyMem ((UINT8*)&Value, Data, BuffSize);
    TmFifoWrite64 (RSH_TM_TILE_TO_HOST_DATA, Value);
  }

  LanDriver->Stats.TxTotalFrames += 1;
  LanDriver->Stats.TxGoodFrames += 1;
  LanDriver->Stats.TxTotalBytes += BuffSize;

  // Save the address of the submitted packet so we can notify the consumer that
  // it has been sent in GetStatus. When the packet tag appears in the Tx Status
  // Fifo, we will return Buffer in the TxBuff parameter of GetStatus.
  PacketTag = LanDriver->NextPacketTag;
  LanDriver->TxRing[PacketTag % TMFIFO_TX_RING_NUM_ENTRIES] = Buff;
  LanDriver->NextPacketTag++;

  return EFI_SUCCESS;
}


/*
 *  UEFI Receive() function
 *
 */
EFI_STATUS
EFIAPI
TmFifoSnpReceive (
  IN        EFI_SIMPLE_NETWORK_PROTOCOL* Snp,
      OUT   UINTN *HdrSize                OPTIONAL,
  IN  OUT   UINTN *BuffSize,
      OUT   VOID *Data,
      OUT   EFI_MAC_ADDRESS *SrcAddr      OPTIONAL,
      OUT   EFI_MAC_ADDRESS *DstAddr      OPTIONAL,
      OUT   UINT16 *Protocol              OPTIONAL
  )
{
  TMFIFO_DRIVER  *LanDriver;
  UINT8          *RawData;
  ETHER_HEAD     *EthHdr;
  UINT32          PLength; // Packet length
  UINT32          ReceiveFilterSetting;

  // Check preliminaries
  if ((Snp == NULL) || (Data == NULL) || (BuffSize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  LanDriver = INSTANCE_FROM_SNP_THIS (Snp);

  // Check that driver was started and initialized
  if (Snp->Mode->State == EfiSimpleNetworkStarted) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver not initialized\n"));
    return EFI_DEVICE_ERROR;
  } else if (Snp->Mode->State == EfiSimpleNetworkStopped) {
    DEBUG ((EFI_D_WARN, "Warning: TMFIFO Driver in stopped state\n"));
    return EFI_NOT_STARTED;
  }

  // Check if a complete packet has been received.
  if (!TmFifoPoll (LanDriver)) {
    return EFI_NOT_READY;
  }

  // Get the received packet length
  PLength = NTOHS(LanDriver->RxHdr.Len);

  // Ignore non-networking packets
  if (PLength == 0 || LanDriver->RxHdr.Type != VIRTIO_ID_NET) {
    return EFI_NOT_READY;
  }

  // Ready for next packet
  LanDriver->RxHdr.Len = 0;
  LanDriver->RxSize = 0;

  LanDriver->Stats.RxTotalBytes += PLength;
  LanDriver->Stats.RxTotalFrames++;

  // Check buffer size
  if (*BuffSize < PLength) {
    *BuffSize = PLength;
    LanDriver->Stats.RxDroppedFrames++;
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Software filtering processing.
  //
  EthHdr = (ETHER_HEAD *)LanDriver->RxPkt;
  ReceiveFilterSetting = Snp->Mode->ReceiveFilterSetting;

  if (!(ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS)) {
    if ((EthHdr->DstMac[0] & 0x01) == 0) {
      if (CompareMem (EthHdr->DstMac, &Snp->Mode->CurrentAddress,
                      NET_ETHER_ADDR_LEN) != 0) {
        // If not promiscuous, drop unicast packets if not destined to this MAC.
        // No need to update the drop-counter (similar to HW NIC behavior).
        return EFI_NOT_READY;
      }
    } else if (CompareMem (EthHdr->DstMac, BCastMac, NET_ETHER_ADDR_LEN) == 0) {
      // Drop broadcast packets if not allowed.
      if (!(ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST)) {
        LanDriver->Stats.RxDroppedFrames++;
        return EFI_NOT_READY;
      }
    }
  }

  // Update buffer size
  *BuffSize = PLength;

  if (HdrSize != NULL)
    *HdrSize = Snp->Mode->MediaHeaderSize;

  // Format the pointer
  RawData = (UINT8*)Data;

  // Get Rx Packet
  CopyMem (RawData, LanDriver->RxPkt, PLength);

  // Get the destination MAC address
  if (DstAddr != NULL) {
    CopyMem (DstAddr, RawData, NET_ETHER_ADDR_LEN);
  }

  // Get the source MAC address
  if (SrcAddr != NULL) {
    CopyMem (SrcAddr, RawData + NET_ETHER_ADDR_LEN, NET_ETHER_ADDR_LEN);
  }

  // Get the Ethernet type
  if (Protocol != NULL) {
    *Protocol = (RawData[12] << 8) + RawData[13];
  }

  LanDriver->Stats.RxGoodFrames++;

  return EFI_SUCCESS;
}

/*
 *  Entry point for the TMFIFO driver
 *
 */
EFI_STATUS
TmFifoDxeEntry (
  IN EFI_HANDLE Handle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS                   Status;
  TMFIFO_DRIVER               *LanDriver;
  EFI_SIMPLE_NETWORK_PROTOCOL *Snp;
  EFI_SIMPLE_NETWORK_MODE     *SnpMode;
  TMFIFO_DEVICE_PATH          *TmFifoPath;
  UINT8                       *CfgMacAddr;
  EFI_HANDLE                   ControllerHandle;

  // Allocate Resources
  LanDriver = AllocateZeroPool (sizeof (TMFIFO_DRIVER));
  if (LanDriver == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  TmFifoPath = (TMFIFO_DEVICE_PATH*)AllocateCopyPool (
                 sizeof (TMFIFO_DEVICE_PATH), &TmFifoPathTemplate);
  if (TmFifoPath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Initialize pointers
  Snp = &(LanDriver->Snp);
  SnpMode = &(LanDriver->SnpMode);
  Snp->Mode = SnpMode;

  // Set the signature of the LAN Driver structure
  LanDriver->Signature = TMFIFO_SIGNATURE;

  // Assign fields and func pointers
  Snp->Revision = EFI_SIMPLE_NETWORK_PROTOCOL_REVISION;
  Snp->WaitForPacket = NULL;
  Snp->Initialize = TmFifoSnpInitialize;
  Snp->Start = TmFifoSnpStart;
  Snp->Stop = TmFifoSnpStop;
  Snp->Reset = TmFifoSnpReset;
  Snp->Shutdown = TmFifoSnpShutdown;
  Snp->ReceiveFilters = TmFifoSnpReceiveFilters;
  Snp->StationAddress = TmFifoSnpStationAddress;
  Snp->Statistics = TmFifoSnpStatistics;
  Snp->MCastIpToMac = TmFifoSnpMcastIptoMac;
  Snp->NvData = TmFifoSnpNvData;
  Snp->GetStatus = TmFifoSnpGetStatus;
  Snp->Transmit = TmFifoSnpTransmit;
  Snp->Receive = TmFifoSnpReceive;

  // Start completing simple network mode structure
  SnpMode->State = EfiSimpleNetworkStopped;
  SnpMode->HwAddressSize = NET_ETHER_ADDR_LEN; // HW address is 6 bytes
  SnpMode->MediaHeaderSize = sizeof (ETHER_HEAD); // Media header size
  SnpMode->MaxPacketSize = TMFIFO_MAX_PACKET_SIZE;
  SnpMode->NvRamSize = 0;       // No NVRAM with this device
  SnpMode->NvRamAccessSize = 0; // No NVRAM with this device
  SnpMode->ReceiveFilterMask = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST |
                               EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST |
                               EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS;

  // We do not intend to filter anything for the time being.
  SnpMode->ReceiveFilterSetting = 0;

  // No support for MCast MAC Addresses for now
  SnpMode->MaxMCastFilterCount = 0;
  SnpMode->MCastFilterCount = 0;

  // Set the interface type (1: Ethernet or 6: IEEE 802 Networks)
  SnpMode->IfType = NET_IFTYPE_ETHERNET;

  // Mac address is changeable as it is loaded from erasable memory
  SnpMode->MacAddressChangeable = TRUE;

  // Can only transmit one packet at a time
  SnpMode->MultipleTxSupported = FALSE;

  // MediaPresent checks for cable connection and partner link
  SnpMode->MediaPresentSupported = FALSE;
  SnpMode->MediaPresent = TRUE;

  // Set broadcast address
  SetMem (&SnpMode->BroadcastAddress, sizeof (EFI_MAC_ADDRESS), 0xFF);

  // Check the persistent variable for configured MAC, or create one if it
  // doesn't exist.
  GetEfiGlobalVariable2 (L"RshimMacAddr", (VOID**)&CfgMacAddr, NULL);
  if (CfgMacAddr != NULL) {
    CopyMem (DefaultMac, CfgMacAddr, sizeof (DefaultMac));
    FreePool (CfgMacAddr);
  } else {
    gRT->SetVariable (
                  L"RshimMacAddr",
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS |
                    EFI_VARIABLE_RUNTIME_ACCESS |
                    EFI_VARIABLE_NON_VOLATILE,
                  sizeof (DefaultMac),
                  DefaultMac
                  );
  }

  // Set a default MAC address. This address could be overwritten later
  // by persistent variable.
  CopyMem (&SnpMode->CurrentAddress, DefaultMac, sizeof (DefaultMac));

  // Assign fields for device path
  CopyMem (&TmFifoPath->TmFifo.MacAddress, &Snp->Mode->CurrentAddress,
           NET_ETHER_ADDR_LEN);
  TmFifoPath->TmFifo.IfType = Snp->Mode->IfType;

  // Initialise the protocol
  ControllerHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ControllerHandle,
                  &gEfiSimpleNetworkProtocolGuid, Snp,
                  &gEfiDevicePathProtocolGuid, TmFifoPath,
                  NULL
                  );
  // Say what the status of loading the protocol structure is
  if (EFI_ERROR(Status)) {
    FreePool (LanDriver);
  } else {
    MLNX_EFI_INFO *Info = (MLNX_EFI_INFO *)MLNX_EFI_INFO_ADDR;

    LanDriver->ControllerHandle = ControllerHandle;
    Info->TmFifo = LanDriver;
  }

  return Status;
}
