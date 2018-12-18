/** @file
  Copyright (c) 2017, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials are licensed and made
  available under the terms and conditions of the BSD License which
  accompanies this distribution.  The full text of the license may be
  found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS"
  BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER
  EXPRESS OR IMPLIED.
**/

#include "BlueFieldPlatform.h"
#include "BlueFieldEfiInfo.h"
#include "TmFifoDxe.h"
#include <Library/BaseMemoryLib.h>

/* TMFIFO Rx status register offset and field shift/width. */
#define TMFIFO_RX_STS_COUNT_SHIFT               0
#define TMFIFO_RX_STS_COUNT_WIDTH               9

/* TMFIFO Rx control register offset and field shift/width. */
#define TMFIFO_RX_CTL_LWM_SHIFT                 0
#define TMFIFO_RX_CTL_LWM_WIDTH                 8
#define TMFIFO_RX_CTL_HWM_SHIFT                 8
#define TMFIFO_RX_CTL_HWM_WIDTH                 8
#define TMFIFO_RX_CTL_MAX_ENTRIES_SHIFT         32
#define TMFIFO_RX_CTL_MAX_ENTRIES_WIDTH         9

/* TMFIFO Tx status register offset and field shift/width. */
#define TMFIFO_TX_STS_COUNT_SHIFT               0
#define TMFIFO_TX_STS_COUNT_WIDTH               9

/* TMFIFO Tx control register offset and field shift/width. */
#define TMFIFO_TX_CTL_LWM_SHIFT                 0
#define TMFIFO_TX_CTL_LWM_WIDTH                 8
#define TMFIFO_TX_CTL_HWM_SHIFT                 8
#define TMFIFO_TX_CTL_HWM_WIDTH                 8
#define TMFIFO_TX_CTL_MAX_ENTRIES_SHIFT         32
#define TMFIFO_TX_CTL_MAX_ENTRIES_WIDTH         9

/* Several utility macros to get/set the register fields. */
#define TMFIFO_GET_FIELD(reg, field) \
  (((reg) >> field##_SHIFT) & ((1UL << field##_WIDTH) - 1))

#define TMFIFO_SET_FIELD(reg, field, value) ({ \
  UINT64 _mask = ((1UL << field##_WIDTH) - 1) << field##_SHIFT; \
  ((reg) & ~_mask) | (((value) << field##_SHIFT) & _mask); \
})

/* Number of words to sent as sync data. */
#define TMFIFO_SYNC_WORDS                       (1536 / 8)

/* Use big-endian format for RxHdr.Len. */
#undef ntohs
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define ntohs(x) SwapBytes16(x)
#else
# define ntohs(x) (x)
#endif

UINT32
EFIAPI
TmFifoConsAvailSpace(
  IN TMFIFO_DRIVER *LanDriver
)
{
  UINT32 Used;

  Used = (LanDriver->ConsTail >= LanDriver->ConsHead) ?
    (LanDriver->ConsTail - LanDriver->ConsHead) :
    (TMFIFO_CONS_RING_SIZE - LanDriver->ConsHead + LanDriver->ConsTail);

  return TMFIFO_CONS_RING_SIZE - Used;
}

BOOLEAN
EFIAPI
TmFifoPoll (
  IN OUT TMFIFO_DRIVER *LanDriver
  )
{
  UINT64 Data;
  UINT32 Avail = 0, Len;
  TMFIFO_MSG_HDR Hdr;

  // Flush the pending console Tx word if not in the middle of a network packet
  if (LanDriver->RxHdr.Len == 0 && LanDriver->ConsTxSize > 0 &&
      sizeof (Hdr) + LanDriver->ConsTxSize <= TmFifoTxAvail (0) * 8) {
    Hdr.Data = 0;
    Hdr.Type = VIRTIO_ID_CONSOLE;
    ((UINT8*)&Hdr.Len)[0] = (LanDriver->ConsTxSize >> 8) & 0xFF;
    ((UINT8*)&Hdr.Len)[1] = LanDriver->ConsTxSize & 0xFF;
    TmFifoWrite64 (RSH_TM_TILE_TO_HOST_DATA, Hdr.Data);
    TmFifoWrite64 (RSH_TM_TILE_TO_HOST_DATA, LanDriver->ConsTxWord);
    LanDriver->ConsTxSize = 0;
  }

  // Read packet header
  if (LanDriver->RxHdr.Len == 0) {
    Avail = TmFifoRxAvail ();
    if (Avail * sizeof(UINT64) < sizeof (LanDriver->RxHdr))
      return FALSE;

    LanDriver->RxHdr.Data = TmFifoRead64 (RSH_TM_HOST_TO_TILE_DATA);
    Avail--;
    if (LanDriver->RxHdr.Len == 0)
      return FALSE;

    if (ntohs (LanDriver->RxHdr.Len) >= sizeof (LanDriver->RxPkt)) {
      DEBUG ((EFI_D_ERROR, "Error: RxHdr.Len = %d too big\n",
        ntohs (LanDriver->RxHdr.Len)));
      LanDriver->RxHdr.Data = 0;
      return FALSE;
    }
  }

  while (LanDriver->RxSize < ntohs (LanDriver->RxHdr.Len)) {
    // Check the available data in the FIFO
    if (Avail == 0) {
      Avail = TmFifoRxAvail ();
      if (Avail == 0)
        return FALSE;
    }

    Data = TmFifoRead64 (RSH_TM_HOST_TO_TILE_DATA);
    Avail--;
    CopyMem (&LanDriver->RxPkt[LanDriver->RxSize], &Data, sizeof (UINT64));
    LanDriver->RxSize += sizeof (UINT64);
  }

  // Packet handling
  switch (LanDriver->RxHdr.Type) {
  case VIRTIO_ID_NET:
    // Drop the network packet if interface is not up.
    if (LanDriver->SnpMode.State != EfiSimpleNetworkInitialized) {
      LanDriver->RxSize = 0;
      LanDriver->RxHdr.Len = 0;
    }
    break;

  case VIRTIO_ID_CONSOLE:
    Len = ntohs (LanDriver->RxHdr.Len);

    // Save the console input if there is enough space, or else drop it
    if (TmFifoConsAvailSpace(LanDriver) > Len) {
      if (LanDriver->ConsTail + Len <= TMFIFO_CONS_RING_SIZE) {
        CopyMem (&LanDriver->ConsRing[LanDriver->ConsTail], LanDriver->RxPkt,
                 Len);
        LanDriver->ConsTail += Len;
        if (LanDriver->ConsTail >= TMFIFO_CONS_RING_SIZE) {
          LanDriver->ConsTail -= TMFIFO_CONS_RING_SIZE;
        }
      } else {
        CopyMem (&LanDriver->ConsRing[LanDriver->ConsTail], LanDriver->RxPkt,
                 TMFIFO_CONS_RING_SIZE - LanDriver->ConsTail);
        CopyMem (&LanDriver->ConsRing[0],
                 &LanDriver->RxPkt[TMFIFO_CONS_RING_SIZE - LanDriver->ConsTail],
                 Len - (TMFIFO_CONS_RING_SIZE - LanDriver->ConsTail));
        LanDriver->ConsTail = Len - (TMFIFO_CONS_RING_SIZE -
                                     LanDriver->ConsTail);
      }
    }
    // continue the default logic

  default:
    // Drop
    LanDriver->RxSize = 0;
    LanDriver->RxHdr.Len = 0;
    return FALSE;
  }

  return TRUE;
}

UINT32
EFIAPI
TmFifoRxSize (
  VOID
  )
{
  UINT64    Ctl;

  Ctl = TmFifoRead64 (RSH_TM_HOST_TO_TILE_CTL);
  return TMFIFO_GET_FIELD (Ctl, TMFIFO_RX_CTL_MAX_ENTRIES);
}

UINT32
EFIAPI
TmFifoTxSize (
  VOID
  )
{
  UINT64    Ctl;

  Ctl = TmFifoRead64 (RSH_TM_TILE_TO_HOST_CTL);
  return TMFIFO_GET_FIELD (Ctl, TMFIFO_TX_CTL_MAX_ENTRIES);
}

UINT32
EFIAPI
TmFifoRxAvail (
  VOID
  )
{
  UINT64 Sts;

  Sts = TmFifoRead64 (RSH_TM_HOST_TO_TILE_STS);
  return TMFIFO_GET_FIELD(Sts, TMFIFO_RX_STS_COUNT);
}

UINT32
EFIAPI
TmFifoTxAvail (
  IN UINT32 Reserve
  )
{
  UINT64 Sts;
  UINT32 Used, TxFifoSize = TmFifoTxSize();

  Sts = TmFifoRead64 (RSH_TM_TILE_TO_HOST_STS);
  Used = TMFIFO_GET_FIELD(Sts, TMFIFO_TX_STS_COUNT);

  return (TxFifoSize > Reserve + Used) ?
    (TxFifoSize - Reserve - Used) : 0;
}

VOID
EFIAPI
TmFifoInit (
  VOID
  )
{
  UINT64    Ctl;
  UINT32    Avail, Num;
  TMFIFO_MSG_HDR Hdr;

  // Get Tx FIFO size and set the low/high watermark
  Ctl = TmFifoRead64 (RSH_TM_TILE_TO_HOST_CTL);
  Ctl = TMFIFO_SET_FIELD (Ctl, TMFIFO_TX_CTL_LWM, 0);
  Ctl = TMFIFO_SET_FIELD (Ctl, TMFIFO_TX_CTL_HWM, TmFifoTxSize() - 1);
  TmFifoWrite64 (RSH_TM_TILE_TO_HOST_CTL, Ctl);

  // Get Rx FIFO size and set the low/high watermark
  Ctl = TmFifoRead64 (RSH_TM_HOST_TO_TILE_CTL);
  Ctl = TMFIFO_SET_FIELD (Ctl, TMFIFO_RX_CTL_LWM, 0);
  Ctl = TMFIFO_SET_FIELD (Ctl, TMFIFO_RX_CTL_HWM, TmFifoRxSize() - 1);
  TmFifoWrite64 (RSH_TM_HOST_TO_TILE_CTL, Ctl);

  // Send zero-length Sync data to the host side
  Avail = TmFifoTxAvail(0);
  if (Avail > TMFIFO_SYNC_WORDS)
    Avail = TMFIFO_SYNC_WORDS;
  Hdr.Data = 0;
  Hdr.Type = VIRTIO_ID_NET;
  for (Num = 0; Num < Avail; Num++) {
    TmFifoWrite64 (RSH_TM_TILE_TO_HOST_DATA, Hdr.Data);
  }
}

UINT64
EFIAPI
TmFifoRead64 (
  IN UINT32 Address
  )
{
  return MmioRead64 (PcdGet64 (PcdRshimBase) + Address);
}

UINT64
EFIAPI
TmFifoWrite64(
  IN UINT32 Address,
  IN UINT64 Value
  )
{
  return MmioWrite64 (PcdGet64 (PcdRshimBase) + Address, Value);
}

UINT64
EFIAPI
TmFifoConsWrite(
  IN UINT8     *Buffer,
  IN UINTN     BuffSize
  )
{
  UINT64         Value;
  TMFIFO_MSG_HDR Hdr;
  UINTN          BuffSizeLeft = BuffSize, ConsTxSize = 0;
  TMFIFO_DRIVER *LanDriver;
  MLNX_EFI_INFO *Info = (MLNX_EFI_INFO *)MLNX_EFI_INFO_ADDR;

  if (!Info->TmFifoInit) {
    Info->TmFifoInit = 1;
    TmFifoInit();
  }

  // Fill in the temporary Tx Word first
  LanDriver = (TMFIFO_DRIVER *)Info->TmFifo;
  if (LanDriver != NULL) {
    if (LanDriver->ConsTxSize + BuffSizeLeft <= sizeof (UINT64)) {
      CopyMem ((UINT8*)&LanDriver->ConsTxWord + LanDriver->ConsTxSize,
               Buffer, BuffSizeLeft);
      LanDriver->ConsTxSize += BuffSizeLeft;
      BuffSizeLeft = 0;
    } else {
      CopyMem ((UINT8*)&LanDriver->ConsTxWord + LanDriver->ConsTxSize,
               Buffer, sizeof (UINT64) - LanDriver->ConsTxSize);
      LanDriver->ConsTxSize = sizeof (UINT64);
      BuffSizeLeft -= sizeof (UINT64) - LanDriver->ConsTxSize;
    }
    ConsTxSize = sizeof (UINT64);
  }

  // Return if no more data, which means it's packed into the Tx word
  if (BuffSizeLeft == 0) {
    return BuffSize;
  }

  // Return if no available fifo buffer
  if (sizeof(Hdr) + ConsTxSize + BuffSizeLeft > TmFifoTxAvail(0) * 8) {
    return (BuffSize - BuffSizeLeft);
  }

  // Write the packet header
  Hdr.Data = 0;
  Hdr.Type = VIRTIO_ID_CONSOLE;
  ((UINT8*)&Hdr.Len)[0] = ((BuffSizeLeft + ConsTxSize) >> 8) & 0xFF;
  ((UINT8*)&Hdr.Len)[1] = (BuffSizeLeft + ConsTxSize) & 0xFF;
  TmFifoWrite64 (RSH_TM_TILE_TO_HOST_DATA, Hdr.Data);

  // Write the buffered Tx word
  if (ConsTxSize > 0) {
    TmFifoWrite64 (RSH_TM_TILE_TO_HOST_DATA, LanDriver->ConsTxWord);
    LanDriver->ConsTxSize = 0;
  }

  // Write the 8-byte blocks
  while (BuffSizeLeft >= sizeof (UINT64)) {
    CopyMem ((UINT8*)&Value, Buffer, sizeof (UINT64));
    TmFifoWrite64 (RSH_TM_TILE_TO_HOST_DATA, Value);
    BuffSizeLeft -= sizeof (UINT64);
    Buffer += sizeof (UINT64);
  }

  // Write the leftover data and padded it to 8 bytes.
  if (BuffSizeLeft > 0) {
    Value = 0;
    CopyMem ((UINT8*)&Value, Buffer, BuffSizeLeft);
    TmFifoWrite64 (RSH_TM_TILE_TO_HOST_DATA, Value);
  }

  return BuffSize;
}

UINT64
EFIAPI
TmFifoConsRead(
  IN OUT UINT8 *Buffer,
  IN UINTN     BuffSize
  )
{
  TMFIFO_DRIVER *LanDriver;
  MLNX_EFI_INFO *Info = (MLNX_EFI_INFO *)MLNX_EFI_INFO_ADDR;

  LanDriver = (TMFIFO_DRIVER *)Info->TmFifo;
  if (!LanDriver) {
    return 0;
  }

  if (LanDriver->ConsTail >= LanDriver->ConsHead) {
    if (BuffSize > LanDriver->ConsTail - LanDriver->ConsHead) {
      BuffSize = LanDriver->ConsTail - LanDriver->ConsHead;
    }
    if (BuffSize > 0) {
      CopyMem (Buffer, &LanDriver->ConsRing[LanDriver->ConsHead], BuffSize);
    }
    LanDriver->ConsHead += BuffSize;
  } else {
    if (BuffSize <= TMFIFO_CONS_RING_SIZE - LanDriver->ConsHead) {
      CopyMem (Buffer, &LanDriver->ConsRing[LanDriver->ConsHead], BuffSize);
      LanDriver->ConsHead += BuffSize;
      if (LanDriver->ConsHead >= TMFIFO_CONS_RING_SIZE) {
        LanDriver->ConsHead -= TMFIFO_CONS_RING_SIZE;
      }
    } else {
      UINT32 LeftBytes;

      CopyMem (Buffer, &LanDriver->ConsRing[LanDriver->ConsHead],
               TMFIFO_CONS_RING_SIZE - LanDriver->ConsHead);
      LeftBytes = BuffSize - (TMFIFO_CONS_RING_SIZE - LanDriver->ConsHead);
      if (LeftBytes > LanDriver->ConsTail) {
        LeftBytes = LanDriver->ConsTail;
      }
      CopyMem (Buffer + TMFIFO_CONS_RING_SIZE - LanDriver->ConsHead,
               &LanDriver->ConsRing[0], LeftBytes);
      LanDriver->ConsHead = LeftBytes;
      BuffSize = TMFIFO_CONS_RING_SIZE - LanDriver->ConsHead + LeftBytes;
    }
  }

  return BuffSize;
}

BOOLEAN
EFIAPI
TmFifoConsPoll(
  VOID
  )
{
  TMFIFO_DRIVER *LanDriver;
  MLNX_EFI_INFO *Info = (MLNX_EFI_INFO *)MLNX_EFI_INFO_ADDR;

  LanDriver = (TMFIFO_DRIVER *)Info->TmFifo;
  if (!LanDriver) {
    return FALSE;
  }

  // Do the polling when the network interface is not up, or else
  // the network interface will do the polling instead.
  if (LanDriver->SnpMode.State != EfiSimpleNetworkInitialized) {
    TmFifoPoll(LanDriver);
  }

  return TmFifoConsAvailSpace(LanDriver) < TMFIFO_CONS_RING_SIZE?
    TRUE : FALSE;
}
