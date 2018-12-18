/** @file

  Copyright (c) 2017, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
  License which accompanies this distribution.  The full text of the license
  may be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __I2C_SMBUS_RUNTIME_H__
#define __I2C_SMBUS_RUNTIME_H__

#include <Uefi.h>

#include <Protocol/SmbusHc.h>

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <sys/bswap.h>

#include <Library/UefiRuntimeLib.h>
#include <Library/DxeServicesTableLib.h>

#include <Pi/PiI2c.h>

// TYU base address including Cause, GPIO, PLL and SMBUS memory space.
// Note that memory regions accessed by the driver are Cause region: from
// 0x2801200 to 0x28012bf, and Smbus region: from 0x2804000 to 0x28057ff.
#define TYU_BASE_ADDR   0x02800000

#define TYU_SMBUS_CNT   3   // Number of SMBUS blocks within TYU.

// Note that the following SMBUS, CAUSE and GPIO register addresses refer to
// their respective offsets within the TYU address space.

#define TYU_SMBUS_SIZE          0x800
#define TYU_SMBUS_ADDR          0x4000

#define TYU_CAUSE_SIZE          0x20
#define TYU_CAUSE_MASTER_ADDR   0x1200
#define TYU_CAUSE_SLAVE_ADDR    \
    (TYU_CAUSE_MASTER_ADDR + (TYU_SMBUS_CNT * TYU_CAUSE_SIZE))

#define TYU_CAUSE_COALESCE_ADDR 0x1300

//
// Macros to retrieve TYU regions base addresses.
//

#define TYU_GET_SMBUS_REGS_ADDR(num)         \
    (TYU_SMBUS_ADDR + ((num) * TYU_SMBUS_SIZE))
#define TYU_GET_CAUSE_MASTER_REGS_ADDR(num)  \
    (TYU_CAUSE_MASTER_ADDR + ((num) * TYU_CAUSE_SIZE))
#define TYU_GET_CAUSE_SLAVE_REGS_ADDR(num)   \
    (TYU_CAUSE_SLAVE_ADDR + ((num) * TYU_CAUSE_SIZE))

//
// TYU - Configuration for PLL
//

// SMBus Master core clock frequency. Timing configurations are
// strongly dependent on the core clock frequency of the Smbus
// Master. Default value is set to 400MHz as required in Bluefield.
#define BLUEFIELD_TYU_PLL_OUT_FREQ  (400 * 1000 * 1000)
// Reference clock for Bluefield is 156 MHz.
#define BLUEFIELD_TYU_PLL_IN_FREQ   (156 * 1000 * 1000)
// YU PLL configuration bits are exposed to SW in TYU boot record block.
#define TYU_CORE_PLL_CFG_LSB        0x035c

#define GET_PLL_CORE_F(val)         (((val) >>  3) & 0x1fff) // 13 bits
#define GET_PLL_CORE_OD(val)        (((val) >> 16) & 0x000f) //  4 bits
#define GET_PLL_CORE_R(val)         (((val) >> 20) & 0x003f) //  6 bits

// PLL configuration bits for CPU core frequency.
#define TYU_CORE_PLL_0_LSB        0x0384

//
// TYU - Configuration for cause.
//

// Cause OR event enable register.
#define TYU_CAUSE_OR_EVTEN0_BITS    0x14
// Cause OR clear bits register.
#define TYU_CAUSE_OR_CLEAR_BITS     0x18
// Cause Arbiter bits register.
#define TYU_CAUSE_ARBITER_BITS      0x1c

// TYU Cause Status flags. Note that those bits might be considered
// as interrupt enabled bits.
#define CAUSE_TRANSACTION_ENDED     0x001   // Transaction ended with STOP
#define CAUSE_M_ARBITRATION_LOST    0x002   // Arbitration lost
#define CAUSE_UNEXPECTED_START      0x004   // Unexpected start detected
#define CAUSE_UNEXPECTED_STOP       0x008   // Unexpected stop detected
#define CAUSE_WAIT_FOR_FW_DATA      0x010   // Wait for transfer continuation
#define CAUSE_PUT_STOP_FAILED       0x020   // Failed to generate STOP
#define CAUSE_PUT_START_FAILED      0x040   // Failed to generate START
#define CAUSE_CLK_TOGGLE_DONE       0x080   // Clock toggle completed
#define CAUSE_M_FW_TIMEOUT          0x100   // Transfer timeout occured
#define CAUSE_M_GW_BUSY_FALL        0x200   // Master busy bit reset

#define CAUSE_ARBITER_BITS_MASK     0x000003ff // 10 bits

// TYU Slave cause status flags. Note that those bits might be considered
// as interrupt enabled bits.

// Write transaction received successfully
#define CAUSE_WRITE_SUCCESS         0x000001
// Write transaction terminated due to unexpected token
#define CAUSE_WRITE_UNEXPECTED_TOK  0x000002
// External master is trying to write more than 128 Bytes
#define CAUSE_WRITE_TOO_LONG        0x000004
// Read transaction ended successfully with NACK
#define CAUSE_READ_SUCCESS_NACK     0x000008
// Read transaction ended unexpected with NACK
#define CAUSE_READ_UNEXPECTED_NACK  0x000010
// Transaction failed due to arbitration lost
#define CAUSE_S_ARBITRATION_LOST    0x000080
// Read transaction terminated due to unexpected start
#define CAUSE_READ_UNEXPECTED_START 0x000100
// Read transaction terminated due to unexpected stop
#define CAUSE_READ_UNEXPECTED_STOP  0x000200
// Read transaction aborted due to stretch timeout
#define CAUSE_READ_TIMEOUT          0x000400
// Waiting for ACK/NACK
#define CAUSE_WAIT_FOR_ACK_NACK     0x001000
// Read transaction received, waiting for response
#define CAUSE_READ_WAIT_FW_RESPONSE 0x002000
// Write transaction aborted due to stretch timeout
#define CAUSE_WRITE_TIMEOUT         0x004000
// Incorrect slave address at the beginning of read phase
#define CAUSE_BAD_SLAVE_ADDRESS     0x008000
// SCL is idle while SDA is driven by slave
#define CAUSE_SCL_IDLE_SLAVE_SDA    0x010000
// Timeout while waiting for response
#define CAUSE_S_FW_TIMEOUT          0x020000
// Slave busy bit reset
#define CAUSE_S_GW_BUSY_FALL        0x040000
// Master acked last written byte, need to supply more bytes
#define CAUSE_MASTER_EXPECTING_DATA 0x080000
// Master nacked byte but didn't generate stop
#define CAUSE_NO_STOP_AFTER_NACK    0x100000

#define CAUSE_SLAVE_ARBITER_BITS_MASK     0x001fffff // 21 bits

//
// Cause Coalesce registers.
//

#define TYU_CAUSE_COALESCE_0        0x00
#define TYU_CAUSE_COALESCE_1        0x04
#define TYU_CAUSE_COALESCE_2        0x08

//
// TYU - Configuration for GPIO 0
//

#define TYU_GPIO_0_FUNC_EN_0        0x2028  // Functional enable register
#define TYU_GPIO_0_FORCE_OE_EN      0x2030  // Force OE enable register.
// Note that Smbus GWs are on GPIOs 30:25. Two pins are used to control
// SDA/SCL lines.
//
//  SMBUS GW0 -> bits[26:25]
//  SMBUS GW1 -> bits[28:27]
//  SMBUS GW2 -> bits[30:29]
//
#define TYU_GPIO_SMBUS_GW_PINS(num) (25 + ((num) << 1))

#define TYU_GPIO_SMBUS_GW_0_MASK    0xf9ffffff
#define TYU_GPIO_SMBUS_GW_1_MASK    0xe7ffffff
#define TYU_GPIO_SMBUS_GW_2_MASK    0x9fffffff

#define TYU_GPIO_SMBUS_GW_ASSERT_PINS(num, val) \
                ((val) | (0x3 << TYU_GPIO_SMBUS_GW_PINS((num))))

#define TYU_GPIO_SMBUS_GW_RESET_PINS(mask, val) ((val) & (mask))

// Smbus Timing Parameters
#define SMBUS_TIMER_SCL_LOW_SCL_HIGH    0x00
#define SMBUS_TIMER_FALL_RISE_SPIKE     0x04
#define SMBUS_TIMER_THOLD               0x08
#define SMBUS_TIMER_TSETUP_START_STOP   0x0c
#define SMBUS_TIMER_TSETUP_DATA         0x10
#define SMBUS_THIGH_MAX_TBUF            0x14
#define SMBUS_SCL_LOW_TIMEOUT           0x18

// Smbus Timing frequency mode
#define SMBUS_SCL_HIGH_100KHZ           4810
#define SMBUS_SCL_HIGH_400KHZ           1011
#define SMBUS_SCL_HIGH_1MHZ             600

//
// Smbus Master GW registers
//

#define SMBUS_MASTER_GW         0x200   // Smbus Master GW
#define SMBUS_RS_BYTES          0x300   // Number of bytes received and sent
#define SMBUS_MASTER_PEC        0x304   // Packet error check (PEC) value
#define SMBUS_MASTER_STATUS     0x308   // Status bits (ACK/NACK/FW Timeout)
#define SMBUS_READ_SHIFT        0x30c   // Shift left GW data bytes
#define SMBUS_MASTER_FSM        0x310   // Smbus Master Finite State Machine
#define SMBUS_MASTER_CLK        0x314   // Toggle Clock
#define SMBUS_MASTER_CFG        0x318   // SDA and SCL configuration
// When enabled, the master will issue a stop condition in case of timeout
// while waiting for FW response.
#define SMBUS_EN_FW_TIMEOUT     0x31c

// Smbus Master GW control bits offset in SMBUS_MASTER_GW[31:3].
#define MASTER_LOCK_BIT_OFF         31
#define MASTER_BUSY_BIT_OFF         30
#define MASTER_START_BIT_OFF        29  // Control start.
#define MASTER_CTL_WRITE_BIT_OFF    28  // Control write phase.
#define MASTER_WRITE_BIT_OFF        21  // Control write bytes.
#define MASTER_SEND_PEC_BIT_OFF     20
#define MASTER_CTL_READ_BIT_OFF     19  // Control read phase.
#define MASTER_PARSE_EXP_BIT_OFF    11  // Control parse expected bytes.
#define MASTER_SLV_ADDR_BIT_OFF     12  // Slave address.
#define MASTER_READ_BIT_OFF         4   // Control read bytes.
#define MASTER_STOP_BIT_OFF         3   // Control stop.

// Smbus Master GW Data descriptor.
#define MASTER_DATA_DESC_ADDR   0x280   // Data registers address.
#define MASTER_DATA_DESC_SIZE   0x80    // Master data descriptor size in bytes.
// Maximum bytes to read/write per Smbus transaction.
#define MASTER_DATA_R_LENGTH  MASTER_DATA_DESC_SIZE         // 128 bytes
#define MASTER_DATA_W_LENGTH (MASTER_DATA_DESC_SIZE - 1)    // 127 bytes

// Smbus Master GW Status flags
#define SMBUS_STATUS_BYTE_CNT_DONE  0x1 // All bytes were transmitted
#define SMBUS_STATUS_NACK_RCV       0x2 // NACK received
#define SMBUS_STATUS_READ_ERR       0x4 // Slave's byte count > 128 bytes
#define SMBUS_STATUS_FW_TIMEOUT     0x8 // Timeout occured

#define SMBUS_MASTER_STATUS_MASK        0x0000000f // 4 bits

#define SMBUS_MASTER_FSM_STOP_MASK      0x80000000
#define SMBUS_MASTER_FSM_PS_STATE_MASK  0x00008000

//
// Smbus Slave Parameters:
//

// Smbus slave GW
#define SMBUS_SLAVE_GW              0x400
// Number of bytes received and sent from/to master
#define SMBUS_SLAVE_RS_MASTER_BYTES 0x500
// Packet error check (PEC) value
#define SMBUS_SLAVE_PEC             0x504
// Shift left GW data bytes
#define SMBUS_SLAVE_READ_SHIFT      0x508
// Smbus Slave Finite State Machine (FSM)
#define SMBUS_SLAVE_FSM             0x510
// Smbus CR Master configuration register
#define SMBUS_SLAVE_CRMASTER_CFG    0x524
// When enabled, FSM will return to idle in case of stretch timeout
// while waiting for FW response.
#define SMBUS_SLAVE_EN_FW_TIMEOUT   0x528
// Should be set when all raised causes handled, and cleared by HW on
// every new cause.
#define SMBUS_SLAVE_READY           0x52c
// Smbus Device Default Address as defined in SMBus spec
#define SMBUS_SLAVE_ARP_ADDR        0x530
// If set, then the Slave is in middle of ARP transaction
#define SMBUS_SLAVE_ARP_STATUS      0x534
// Slave cause register
#define SMBUS_SLAVE_CAUSE           0x53c
// Smbus CR Master FSM
#define SMBUS_SLAVE_CRMASTER_FSM    0x540
// Slave SDA and SCL output
#define SMBUS_SLAVE_CLK_OUTPUT      0x544

// Smbus Slave GW control bits offset in SMBUS_SLAVE_GW[31:19]
#define SLAVE_LOCK_BIT_OFF         31   // Lock bit
#define SLAVE_BUSY_BIT_OFF         30   // Busy bit
#define SLAVE_WRITE_BIT_OFF        29   // Control write enable
#define SLAVE_WRITE_BYTES_BIT_OFF  22   // Number of bytes to write
#define SLAVE_SEND_PEC_BIT_OFF     21   // Send PEC byte when set to 1
#define SLAVE_NACK_BIT_OFF         20   // Nack bit
#define SLAVE_CONT_WRITE_BIT_OFF   19   // Continue write transaction

// Smbus Slave GW Data descriptor
#define SLAVE_DATA_DESC_ADDR   0x480    // Address
#define SLAVE_DATA_DESC_SIZE   0x80     // Data descriptor size in bytes
#define SLAVE_DATA_DESC_SKIP   1        // Bytes to skip within data descriptor

// Smbus Slave configuration registers
#define SMBUS_SLAVE_ADDR_CFG        0x514
#define SMBUS_SLAVE_ADDR_CNT        16
#define SMBUS_SLAVE_ADDR_EN_BIT     7
#define SMBUS_SLAVE_ADDR_MASK       0x7f

// Timeout is given in microseconds.
#define SMBUS_TRANSFER_TIMEOUT          (200 * 1000) // 200ms
#define SMBUS_START_TRANS_TIMEOUT       (300 * 1000) // 300ms
#define SMBUS_WRITE_TRANS_TIMEOUT       (6   * 1000) //   6ms
// Polling frequency expressed in microseconds.
#define SMBUS_POLL_FREQ                 1
// Timeout is given in nanosecond.
#define SMBUS_WAIT_FOR_COALESCE_TIMEOUT (10 * 1000 * 1000)

/* Encapsulates timing parameters */
typedef struct {
    // Clock high period
    UINT16 scl_high;
    // Clock low period
    UINT16 scl_low;
    // Data Rise Time
    UINT8  sda_rise;
    // Data Fall Time
    UINT8  sda_fall;
    // Clock Rise Time
    UINT8  scl_rise;
    // Clock Fall Time
    UINT8  scl_fall;
    // Hold time after (REPEATED) START
    UINT16 hold_start;
    // Data hold time
    UINT16 hold_data;
    // REPEATED START Condition setup time
    UINT16 setup_start;
    // STOP Condition setup time
    UINT16 setup_stop;
    // Data setup time
    UINT16 setup_data;
    // Padding
    UINT16 pad;
    // Bus free time between STOP and START
    UINT16 buf;
    // Thigh max
    UINT16 thigh_max;
    // Detect clock low timeout
    UINT32 timeout;
} I2C_SMBUS_TIMER_PARAMS;

// I2C device request. This structure is based on the EFI_I2C_REQUEST_PACKET
// to describe a single I2C SMBus transaction. Since this structure is used
// at Runtime, Operation array could not be extended dynamically.
// This structure allows a maximum of two SMBus operations per transaction.
typedef struct {
  // Number of elements in the operation array
  UINTN             OperationCount;
  // Description of the I2C operation
  EFI_I2C_OPERATION Operation [2];
} I2C_SMBUS_REQUEST_PACKET;

// Encapsulates SMBus IO memory addresses.
typedef struct {
  UINTN     Smbus;          // Smbus registers base address.
  UINTN     CauseMaster;    // Master cause registers base address.
  UINTN     CauseSlave;     // Slave cause registers base address.
} SMBUS_IO_MEM;

#define I2C_SMBUS_MAX_DEVICES   7

// Encapsulates the phsysical bus informations. This includes the memory
// regions written/read by the driver and the list of slave devices that
// are attached to the bus. Theorically, the I2C bus can support up to 127
// slave devices (7-bit addressing scheme). Here we limit this number to
// I2C_SMBUS_MAX_DEVICES. We assume that the UEFI DXE driver won't manage
// as many slave devices.
typedef struct {
  UINTN         Id;             // Bus identifier.
  SMBUS_IO_MEM  Io;             // Io memory regions information.
  UINT8         Devices [I2C_SMBUS_MAX_DEVICES]; // List of slave devices.
  UINT8         DevicesCnt;     // Number of slave devices.
  UINT8         Slaves [SMBUS_SLAVE_ADDR_CNT];   // List of slave controllers.
  UINT8         SlavesCnt;      // Number of slave controllers
  UINT8         Status;         // Whether it is enabled or disabled.
} SMBUS_INFO;

// Enums for SMBUS_INFO. Status field.
enum {
  I2C_S_DISABLED = 0x0,
  I2C_S_ENABLED  = 0x1,
  I2C_S_SLAVE    = 0x2
};

#define MLX_SMBUS_SIGNATURE      SIGNATURE_32 ('S', 'M', 'B', 'X')

// Encapsulates Smbus context information.
typedef struct {
  UINT32                Signature;
  EFI_HANDLE            Controller;
  UINT64                CoreFrequency;
  EFI_LOCK              Lock;
  EFI_SMBUS_HC_PROTOCOL SmbusHc;
} SMBUS_CONTEXT;

#define I2C_SMBUS_SC_FROM_CONTROLLER (a) \
    CR (a, SMBUS_CONTEXT, SmbusHc, MLX_SMBUS_SIGNATURE)

extern SMBUS_CONTEXT gSmbusContext;

#define I2C_DEVICE_INDEX(bus, address) (((address) & 0xff) | (bus) << 8)
#define I2C_DEVICE_ADDRESS(index)      ((index) & 0xff)

EFI_STATUS
EFIAPI
I2cSmbusInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

/**
  This function register a given slave device to a given SMBUS device.
  address

  @param[in] Bus        The Bus identifier of the SMBUS master device.
  @param[in] Address    The slave address to which the device is preassigned.

  @return EFI_STATUS
**/
EFI_STATUS
EFIAPI
I2cSmbusRegisterSlaveDevice (
  IN UINT8 BusId,
  IN UINT8 Address
  );

EFI_STATUS
EFIAPI
I2cSmbusExecute (
  IN CONST EFI_SMBUS_HC_PROTOCOL    *This,
  IN CONST EFI_SMBUS_DEVICE_ADDRESS SlaveAddress,
  IN CONST EFI_SMBUS_DEVICE_COMMAND Command,
  IN CONST EFI_SMBUS_OPERATION      Operation,
  IN CONST BOOLEAN                  PecCheck,
  IN OUT   UINTN                    *Length,
  IN OUT   VOID                     *Buffer
  );

EFI_STATUS
EFIAPI
I2cSmbusArpDevice (
  IN CONST EFI_SMBUS_HC_PROTOCOL    *This,
  IN       BOOLEAN                  ArpAll,
  IN       EFI_SMBUS_UDID           *SmbusUdid OPTIONAL,
  IN OUT   EFI_SMBUS_DEVICE_ADDRESS *SlaveAddress OPTIONAL
  );

EFI_STATUS
EFIAPI
I2cSmbusGetArpMap (
  IN CONST EFI_SMBUS_HC_PROTOCOL    *This,
  IN OUT   UINTN                    *Length,
  IN OUT   EFI_SMBUS_DEVICE_MAP     **SmbusDeviceMap
  );

EFI_STATUS
EFIAPI
I2cSmbusNotify (
  IN CONST  EFI_SMBUS_HC_PROTOCOL     *This,
  IN CONST  EFI_SMBUS_DEVICE_ADDRESS  SlaveAddress,
  IN CONST  UINTN                     Data,
  IN CONST  EFI_SMBUS_NOTIFY_FUNCTION NotifyFunction
  );

/**
  The SMBUS slave backend function does some actions.

  @param SlaveAddress   The SMBUS Slave hardware address to which the SMBUS
                        device is preassigned.
  @param Data           Pointer to the data of the SMBus that requires some
                        handling.
  @param Flags          Flags to qualify the I2C operation, e.g., read
                        request, write request or stop received.

  @return EFI_STATUS
**/
typedef
EFI_STATUS
(EFIAPI *EFI_SMBUS_SLAVE_FUNCTION)(
  IN        UINT8  SlaveAddress,
  IN        UINT8  *Data,
  IN        UINT8  DataLength,
  IN        UINT32 Flags,
  IN OUT    UINT8  *ByteTransferred
);

// Flags to be supplied to EFI_SMBUS_SLAVE_FUNCTION.
enum {
    SMBUS_SLAVE_STOP,
    SMBUS_SLAVE_WRITE_REQUESTED,
    SMBUS_SLAVE_READ_REQUESTED
};

/**
  This function initialize a given Smbus Slave device. It enables the slave
  function within a given SMBUS hardware and register the associated slave
  address

  @param[in] Bus               The SMBUS hardware to setup.
  @param[in] Slave             The SMBUS Slave hardware address to which
                               the device is preassigned.

  @return EFI_STATUS
**/
EFI_STATUS
EFIAPI
I2cSmbusSlaveInitialize (
  IN        UINT8 Bus,
  IN        UINT8 Slave
  );

/**
  This function installs the slave address of a given device in the I2C
  bus to an available slot within the SMBUS Slave hardware and mark it
  as active so the hardware would enable associated interrupt bits i.e.,
  coalesce bits. Once enabled, all master requests addressing the given
  Smbus slave are automatically ACK-ed.

  @param[in] Bus               The SMBUS hardware to setup.
  @param[in] Slave             The SMBUS Slave hardware address to which
                               the device is preassigned.

  @return EFI_STATUS
**/
EFI_STATUS
EFIAPI
I2cSmbusSlaveEnableAddress (
  IN        UINT8  Bus,
  IN        UINT8  Slave
  );

/**
  This function disables the slave address of a given device in the I2C
  bus within the SMBUS Slave hardware. It marks it as inactive so the
  hardware wouldn't enable associated interrupt bits i.e., coalesce bits.
  Once disabled, all master requests addressing the given Smbus slave are
  automatically NACK-ed.

  Some I2C busses are shared between UEFI and the Linux kernel; the kernel
  might use address slots to enable I2C slave functions. So, this function
  keeps the slot address reserved to UEFI slave functions. Only the address
  enable bit is set to 0. This prevents EFI_OUT_OF_RESOURCES when the
  slave is enabled after soft reset.

  @param[in] Bus               The SMBUS hardware to setup.
  @param[in] Slave             The SMBUS Slave hardware address to which
                               the device is preassigned.

  @return EFI_STATUS
**/
EFI_STATUS
EFIAPI
I2cSmbusSlaveDisableAddress (
  IN        UINT8  Bus,
  IN        UINT8  Slave
  );

/**
  This function allow a given slave controller to send data on the I2C bus.

  @param[in]      Slave             The SMBUS Slave hardware address to which
                                    the SMBUS device is preassigned.
  @param[in][out] ByteWritten       Pointer to the number of byte read from
                                    the SMBUS master.
  @param[in]      SlaveFunction     Pointer to the backend slave function.

  @return EFI_STATUS
**/
EFI_STATUS
EFIAPI
I2cSmbusSlaveSend (
  IN       UINT8 Slave,
  IN OUT   UINTN *ByteWritten,
  IN CONST EFI_SMBUS_SLAVE_FUNCTION SlaveFunction
  );

/**
  This function allow a given slave controller to receive data on the I2C bus.

  @param[in]      Slave             The SMBUS Slave hardware address to which
                                    the SMBUS device is preassigned.
  @param[in][out] ByteRead          Pointer to the number of byte read from the
                                    SMBUS master.
  @param[in]      SlaveFunction     Pointer to the backend slave function.

  @return EFI_STATUS
**/
EFI_STATUS
EFIAPI
I2cSmbusSlaveReceive (
  IN       UINT8 Slave,
  IN OUT   UINTN *ByteRead,
  IN CONST EFI_SMBUS_SLAVE_FUNCTION SlaveFunction
  );

#endif // __I2C_SMBUS_RUNTIME_H__
