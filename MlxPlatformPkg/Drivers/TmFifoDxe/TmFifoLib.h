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

typedef struct TMFIFO_DRIVER TMFIFO_DRIVER;

#define VIRTIO_ID_NET          1 // virtio net
#define VIRTIO_ID_CONSOLE      3 // virtio console

typedef union {
#pragma pack(1)
  struct {
          UINT8   Type;        // Message Type
          UINT16  Len;         // Payload Length in Big Endian
          UINT8 Unused[5];     // Reserved, set to 0
  };
#pragma pack()
  UINT64 Data;
} TMFIFO_MSG_HDR;

/*
 * Initialize the FIFO.
 *
 */
VOID
EFIAPI
TmFifoInit (
  VOID
  );

/*
 *  Poll the TmFifo and receive a packet into the packet buffer.
 *
 *  @param LanDriver    The TmFifo driver.
 *
 *  @return TRUE if a complete packet is ready, otherwise FALSE.
 */
BOOLEAN
EFIAPI
TmFifoPoll (
  IN OUT    TMFIFO_DRIVER *LanDriver
  );

/*
 * Get Rx available words.
 *
 *  @return the number of words available in Rx FIFO.
 */
UINT32
EFIAPI
TmFifoRxAvail (
  VOID
  );

/*
 *  Get Tx available words.
 *
 *  @param Reserve      The number of words to reserve so the Tx fifo is
 *                      considered full when the available space is below
 *                      this value.
 *
 *  @return the number of words available for writing in Tx FIFO.
 */
UINT32
EFIAPI
TmFifoTxAvail (
  IN UINT32 Reserve
  );

/*
 * Get Rx FIFO size.
 *
 *  @return the number of words of Rx FIFO size.
 */
UINT32
EFIAPI
TmFifoRxSize (
  VOID
  );

/*
 * Get Tx FIFO size.
 *
 *  @return the number of words of Tx FIFO size.
 */
UINT32
EFIAPI
TmFifoTxSize (
  VOID
  );

/*
 * Purge / Drain the FIFO.
 *
 */
VOID
EFIAPI
TmFifoPurgeFifo (
  VOID
  );

/*
 * Read 64-bit word from TMFIFO.
 * Data is transmitted in little endian to be consistent with host driver.
 *
 * @param Address      The MMIO address to read.
 *
 * @return the 64-bit word read from FIFO.
 */
UINT64
EFIAPI
TmFifoRead64 (
  IN UINT32 Address
  );

/*
 * Write 64-bit word to TMFIFO.
 * Data is transmitted in little endian to be consistent with host driver.
 *
 * @param Address      The MMIO address to write.
 * @param Value        The 64-bit word to write.
 *
 * @return the mmio write status code.
 */
UINT64
EFIAPI
TmFifoWrite64 (
  IN UINT32 Address,
  IN UINT64 Value
  );

/*
 * Send data buffer to TMFIFO console.
 *
 * @param Buffer       The buffer address.
 * @param BuffSize     The buffer size to write.
 *
 * @return the write status code.
 */
UINT64
EFIAPI
TmFifoConsWrite(
  IN UINT8     *Buffer,
  IN UINTN     BuffSize
  );

/*
 * Read data from TMFIFO console.
 *
 * @param Buffer       The buffer address.
 * @param BuffSize     The buffer size to read.
 *
 * @return the number of bytes written.
 */
UINT64
EFIAPI
TmFifoConsRead(
  IN OUT UINT8 *Buffer,
  IN UINTN     BuffSize
  );

/*
 * Return TRUE if data is available from TMFIFO console.
 *
 */
BOOLEAN
EFIAPI
TmFifoConsPoll(
  VOID
  );
