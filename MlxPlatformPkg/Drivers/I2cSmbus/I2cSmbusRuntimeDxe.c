/** @file

  Copyright (c) 2017, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
  License which accompanies this distribution.  The full text of the license
  may be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

// I2cSmbusRuntimeDxe is a driver supporting I2C SMBus controller on BlueField.
// It interfaces with the hardware bus using the SMBus protocol, which exposes
// Host Controller functionality to drivers of specific devices on I2C bus.
//
// I2cSmbusRuntimeDxe produces the EFI_SMBUS_HC_PROTOCOL which provides SMBus
// host controller management and basic data transactions over SMBus. There is
// one EFI_SMBUS_HC_PROTOCOL instance for each SMBus host controller.
//
// Following function is implemented:
//
//    // Perform an SMBus transaction in master mode on the host controller.
//    EFI_SMBUS_HC_EXECUTE_OPERATION     Execute;
//
// The Execute() function provides a standard way to execute an operation as
// defined in the System Management Bus (SMBus) Specification. The resulting
// transaction will be either the SMBus slave device accepts the transaction
// or this function returns with error. Note that this function should be used
// by I2C device drivers since it provides hardware access to the bus. In fact,
// the Execute function is responsible for serializing SMBus operations using
// the basic locking mechanism. Hence, extreme care must be taken by other
// consumers of this API.

#include <Simulator.h>
#include <Library/BaseMemoryLib.h>

#include "I2cSmbusRuntimeDxe.h"
#include "I2cSmbusEeprom.h"
#include "I2cSmbusRtc.h"
#include "I2cSmbusIpmb.h"

// Global variable for Set Virtual Address Map event.
STATIC EFI_EVENT gSetVirtualAddressMapEvent = NULL;

// Global control space memory region that is converted to a virtual address
// when Set Virtual Address Map is signaled. Note that this base address refers
// to the TYU address space managed by the I2cSmbusRuntimeDxe driver. This
// includes CAUSE, GPIO and SMBUS address space.
STATIC UINTN gBaseAddress;

// Global I2C Master context.  In this implementation, the I2cSmbusRuntimeDxe
// supports a single master controller. To support multiple controllers, this
// global variable should be declared as an array.
SMBUS_CONTEXT gSmbusContext;

// Global SMBUS hardware Information.
STATIC SMBUS_INFO gSmbusInfo [TYU_SMBUS_CNT];

#define ALIGN_ROUNDUP(x, align) (((x) + (align) - 1) & ~((align) - 1))

#define IOSWAP32(x)             SwapBytes32 ((x))

STATIC
UINT32
TYU_READ (
  IN UINTN   Off
  )
{
  return MmioRead32 (gBaseAddress + Off);
}

STATIC
EFI_STATUS
TYU_WRITE (
  IN UINTN   Off,
  IN UINT32  Value
  )
{
  return MmioWrite32 (gBaseAddress + Off, Value);
}

// This function is used to read data from Master GW Data Descriptor.
// Data bytes in the Master GW Data Descriptor are shifted left so the
// data starts at the MSB of the descriptor registers as set by the
// underlying hardware. TYU_READ_DATA enables byte swapping while
// reading data bytes, and MUST be called by the SMBus read routines
// to copy data from the 32 * 32-bit HW Data registers a.k.a Master GW
// Data Descriptor.
STATIC
UINT32
TYU_READ_DATA (
  IN UINTN   Off
  )
{
  return IOSWAP32 (TYU_READ (Off));
}

// This function is used to write data to the Master GW Data Descriptor.
// Data copied to the Master GW Data Descriptor MUST be shifted left so
// the data starts at the MSB of the descriptor registers as required by
// the underlying hardware. TYU_WRITE_DATA enables byte swapping when
// writing data bytes, and MUST be called by the SMBus write routines to
// copy data to the 32 * 32-bit HW Data registers a.k.a Master GW Data
// Descriptor.
STATIC
EFI_STATUS
TYU_WRITE_DATA (
  IN UINTN   Off,
  IN UINT32  Value
  )
{
  return TYU_WRITE (Off, IOSWAP32 (Value));
}

// Return core frequency in Hz based on PLL parameters.
STATIC
UINT64
I2cSmbusGetCoreFrequency (
  IN UINTN CorePllReg
  )
{
  //
  // Compute PLL output frequency as follow:
  //
  //                                       CORE_F + 1
  // PLL_OUT_FREQ = PLL_IN_FREQ * ----------------------------
  //                              (CORE_R + 1) * (CORE_OD + 1)
  //
  // Where PLL_OUT_FREQ and PLL_IN_FREQ refer to CoreFrequency and
  // PadFrequency, respectively.
  //
  // For a CoreFrequency of 400MHz, expected PLL parameters are:
  //
  //  CORE_F     = 511
  //  CORE_R     = 24
  //  CORE_OD    = 7
  //  CORE_BWADJ = 255
  //
  UINT64 CoreFrequency;
  UINT64 PadFrequency;
  UINT32 CorePllControl;
  UINT16 CoreF;
  UINT8  CoreOd, CoreR;

  // PLL configuration bits are exposed to SW in TYU. This function is
  // responsible for reading these bits and deriving the TYU frequency.

  CorePllControl = TYU_READ (CorePllReg);
  PadFrequency   = BLUEFIELD_TYU_PLL_IN_FREQ;

  CoreF  = GET_PLL_CORE_F (CorePllControl);
  CoreOd = GET_PLL_CORE_OD (CorePllControl);
  CoreR  = GET_PLL_CORE_R (CorePllControl);

  CoreFrequency  = PadFrequency * (CoreF + 1);
  CoreFrequency /= ((CoreR + 1) * (CoreOd + 1));

  return CoreFrequency;
}

// Convert elapsed time in nanoseconds to elapsed ticks.
STATIC
UINT32
I2cSmbusGetTicks (
  IN UINT64  Frequency,
  IN UINT64  NanoSeconds,
  IN UINT8   Minimum
  )
{
  UINT32 Ticks;

  //
  // Compute ticks as follow:
  //
  //          Ticks
  // Time = --------- x 10^9    =>    Ticks = Time x Frequency x 10^-9
  //        Frequency
  //

  Ticks = (NanoSeconds * Frequency) / 1000000000;

  // The number of ticks is rounded down and if minimum is equal to 1 then
  // add one tick.
  if (Minimum == 1)
    Ticks += 1;

  return Ticks;
}

STATIC
UINT32
I2cSetTimer(
  IN UINT64  NanoSeconds,
  IN BOOLEAN Option,
  IN UINT32  Mask,
  IN UINT8   Offset)
{
  UINT64 Frequency = gSmbusContext.CoreFrequency;

  return ((I2cSmbusGetTicks(Frequency, NanoSeconds, Option) & Mask) << Offset);
}

STATIC
VOID
I2cTimerConfig(
  IN SMBUS_INFO                 *SmbusInfo,
  IN I2C_SMBUS_TIMER_PARAMS     *TimerParams
  )
{
    UINT32 TimeReg;

    TimeReg  = I2cSetTimer (TimerParams->scl_high,    0, 0xffff,  0);
    TimeReg |= I2cSetTimer (TimerParams->scl_low,     0, 0xffff, 16);
    TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_TIMER_SCL_LOW_SCL_HIGH, TimeReg);

    TimeReg  = I2cSetTimer (TimerParams->sda_rise,    0, 0xff,    0);
    TimeReg |= I2cSetTimer (TimerParams->sda_fall,    0, 0xff,    8);
    TimeReg |= I2cSetTimer (TimerParams->scl_rise,    0, 0xff,   16);
    TimeReg |= I2cSetTimer (TimerParams->scl_fall,    0, 0xff,   24);
    TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_TIMER_FALL_RISE_SPIKE, TimeReg);

    TimeReg  = I2cSetTimer (TimerParams->hold_start,  1, 0xffff,  0);
    TimeReg |= I2cSetTimer (TimerParams->hold_data,   1, 0xffff, 16);
    TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_TIMER_THOLD, TimeReg);

    TimeReg  = I2cSetTimer (TimerParams->setup_start, 1, 0xffff,  0);
    TimeReg |= I2cSetTimer (TimerParams->setup_stop,  1, 0xffff, 16);
    TYU_WRITE(SmbusInfo->Io.Smbus + SMBUS_TIMER_TSETUP_START_STOP, TimeReg);

    TimeReg = I2cSetTimer (TimerParams->setup_data,   1, 0xffff,  0);
    TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_TIMER_TSETUP_DATA, TimeReg);

    TimeReg  = I2cSetTimer (TimerParams->buf,         0, 0xffff,  0);
    TimeReg |= I2cSetTimer (TimerParams->thigh_max,   0, 0xffff, 16);
    TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_THIGH_MAX_TBUF, TimeReg);

    TimeReg = TimerParams->timeout;
    TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_SCL_LOW_TIMEOUT, TimeReg);
}

STATIC
VOID
I2cTimerSetup(
  IN SMBUS_INFO     *SmbusInfo,
  IN UINT32         TimerConfig)
{
  I2C_SMBUS_TIMER_PARAMS TimerParams;

  // Parameter         100KHz link   400KHz link     1MHz link
  //
  // Clock frequency   100 KHz       400 KHz         1 MHz
  // Clock low timeout 35 msec       35 msec         35 msec
  // scl_high_time     4.87/4.81 us  1.05/1.01us     0.6 us   //switche/HCA
  // scl_low_time      5 us          1.3 us          1.3 us
  // sda_rise_time     100 ns        100 ns          100 ns
  // sda_fall_time     100 ns        100 ns          100 ns
  // scl_rise_time     100 ns        100 ns          100 ns
  // scl_fall_time     100 ns        100 ns          100 ns
  // thold_start       4.0 us        600 ns          600 ns
  // thold_data        300 ns        300 ns          300 ns
  // tsetup_start      4.8 us        700 ns          600 ns
  // tsetup_stop       4.0 us        600 ns          600 ns
  // tsetup_data       250 ns        100 ns          100 ns
  // Tbuf              20  us        20  us          20  us
  // thigh_max         50  us        50  us          50  us
  //
  // Spike pulse width 50  ns        50  ns          50  ns // this translates
  //                                              // to scl/sda_rise/fall_time
  //

  switch (TimerConfig) {
  default:
    // Default settings is 100 KHz
    DEBUG ((DEBUG_ERROR,
            "I2cTimerSetup: Illegal value %d: defaulting to 100 Khz\n",
            TimerConfig));

    // FALLTHROUGH

  case 100:
    TimerParams.scl_high    = SMBUS_SCL_HIGH_100KHZ;
    TimerParams.scl_low     = 5000;
    TimerParams.hold_start  = 4000;
    TimerParams.setup_start = 4800;
    TimerParams.setup_stop  = 4000;
    TimerParams.setup_data  = 250;
    break;

  case 400:
    TimerParams.scl_high    = SMBUS_SCL_HIGH_400KHZ;
    TimerParams.scl_low     = 1300;
    TimerParams.hold_start  = 600;
    TimerParams.setup_start = 700;
    TimerParams.setup_stop  = 600;
    TimerParams.setup_data  = 100;
    break;

  case 1000:
    TimerParams.scl_high    = SMBUS_SCL_HIGH_1MHZ;
    TimerParams.scl_low     = 1300;
    TimerParams.hold_start  = 600;
    TimerParams.setup_start = 600;
    TimerParams.setup_stop  = 600;
    TimerParams.setup_data  = 100;
    break;
  }

  TimerParams.sda_rise  = TimerParams.sda_fall = 50;
  TimerParams.scl_rise  = TimerParams.scl_fall = 50;
  TimerParams.hold_data = 300;
  TimerParams.buf       = 20000;
  TimerParams.thigh_max = 5000;
  // Note that the SCL_LOW_TIMEOUT value is not related to the bus
  // frequency, it is impacted by the time it takes the driver to
  // complete data transmission before transaction abort.
  TimerParams.timeout   = 10000000;

  I2cTimerConfig (SmbusInfo, &TimerParams);
}

// Configure the I2C SMBus. It consists on Master initialization and Timer
// settings.
STATIC
VOID
I2cSmbusConfigure (
  IN SMBUS_INFO *SmbusInfo
  )
{
  UINT32 ConfigReg;
  UINT32 GpioMask;

  //
  // Smbus Master initialization
  //

  // TYU - Configuration for GPIO pins. Those pins must be asserted in
  // TYU_GPIO_0_FUNC_EN_0, i.e. GPIO 0 is controlled by HW, and must
  // be reset in TYU_GPIO_0_FORCE_OE_EN, i.e. GPIO_OE will be driven
  // instead of HW_OE.

  ConfigReg = TYU_READ (TYU_GPIO_0_FUNC_EN_0);
  ConfigReg = TYU_GPIO_SMBUS_GW_ASSERT_PINS (SmbusInfo->Id, ConfigReg);
  TYU_WRITE (TYU_GPIO_0_FUNC_EN_0, ConfigReg);

  GpioMask = ~((UINT32) 0);

  // Work out what mask we should be using to reset GPIO pins.
  switch (SmbusInfo->Id) {
  case 0:
    GpioMask = TYU_GPIO_SMBUS_GW_0_MASK;
    break;
  case 1:
    GpioMask = TYU_GPIO_SMBUS_GW_1_MASK;
    break;
  case 2:
    GpioMask = TYU_GPIO_SMBUS_GW_2_MASK;
    break;
  }

  ConfigReg = TYU_READ (TYU_GPIO_0_FORCE_OE_EN);
  ConfigReg = TYU_GPIO_SMBUS_GW_RESET_PINS (GpioMask, ConfigReg);
  TYU_WRITE (TYU_GPIO_0_FORCE_OE_EN, ConfigReg);

  //
  // Smbus Timing initialization.
  //

  I2cTimerSetup (SmbusInfo, PcdGet32 (PcdI2cSmbusFrequencyKhz));
}

STATIC
VOID
I2cSmbusEnable (
  IN UINT8 BusId
  )
{
  SMBUS_INFO   *SmbusInfo;

  // Retrieve the SmbusInfo associated with the I2C bus.
  SmbusInfo                 = &gSmbusInfo[BusId];
  SmbusInfo->Id             = BusId;


  // Setup the IO memory regions.
  SmbusInfo->Io.Smbus       = TYU_GET_SMBUS_REGS_ADDR (BusId);
  SmbusInfo->Io.CauseMaster = TYU_GET_CAUSE_MASTER_REGS_ADDR (BusId);
  SmbusInfo->Io.CauseSlave  = TYU_GET_CAUSE_SLAVE_REGS_ADDR (BusId);

  // I2C SMBus configuration: Master and Timer initialization.
  I2cSmbusConfigure (SmbusInfo);

  // Mark the bus as enabled.
  SmbusInfo->Status = I2C_S_ENABLED;
  DEBUG ((DEBUG_ERROR, "I2cSmbusEnable: I2C bus %u enabled\n", BusId));
}

VOID
EFIAPI
I2cSmbusSetVirtualAddressNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{

  // We convert both Contexts, Master Controller and Eeprom into virtual
  // address.
  EfiConvertPointer (0x0, (VOID **)&gEepromContext.I2cSmbusExecute);
  EfiConvertPointer (0x0, (VOID **)&gEepromContext.EepromProtocol);
  EfiConvertPointer (0x0, (VOID **)&gEepromContext);

  EfiConvertPointer (0x0, (VOID **)&gRtcContext.I2cSmbusExecute);
  EfiConvertPointer (0x0, (VOID **)&gRtcContext.RtcProtocol);
  EfiConvertPointer (0x0, (VOID **)&gRtcContext);

  EfiConvertPointer (0x0, (VOID **)&gSmbusContext.SmbusHc.Notify);
  EfiConvertPointer (0x0, (VOID **)&gSmbusContext.SmbusHc.GetArpMap);
  EfiConvertPointer (0x0, (VOID **)&gSmbusContext.SmbusHc.ArpDevice);
  EfiConvertPointer (0x0, (VOID **)&gSmbusContext.SmbusHc.Execute);
  EfiConvertPointer (0x0, (VOID **)&gSmbusContext.SmbusHc);
  EfiConvertPointer (0x0, (VOID **)&gSmbusContext);

  // We assume that the base address used in the runtime driver needs
  // to be converted into a virtual address.
  EfiConvertPointer (0x0, (VOID **)&gBaseAddress);

  return;
}

STATIC
EFI_STATUS
I2cSmbusInitializeController (
  VOID
  )
{
  EFI_STATUS   Status;
  UINT8        BusIdx, BusMask;

  // In the current implementation, we assume that there is a single Host
  // Controller that manages the I2C SMBus hardware.

  // The EFI_SMBUS_HC_PROTOCOL provides SMBus host controller management and
  // basic data transactions over SMBus.  There is one EFI_SMBUS_HC_PROTOCOL
  // instance for each SMBus host controller.
  gSmbusContext.SmbusHc.Execute   = I2cSmbusExecute;
  gSmbusContext.SmbusHc.ArpDevice = I2cSmbusArpDevice;
  gSmbusContext.SmbusHc.GetArpMap = I2cSmbusGetArpMap;
  gSmbusContext.SmbusHc.Notify    = I2cSmbusNotify;

  gSmbusContext.Signature         = MLX_SMBUS_SIGNATURE;
  gSmbusContext.CoreFrequency     =
                            I2cSmbusGetCoreFrequency (TYU_CORE_PLL_CFG_LSB);
  // I2cMasterContext->Lock is responsible for serializing I2C operations
  EfiInitializeLock (&gSmbusContext.Lock, TPL_NOTIFY);

  // Read the I2C SMBus bitmask. The asserted bits indicates that the
  // bus should be enabled.
  BusMask = PcdGet8 (PcdI2cSmbusBitmask);

  // Enable The I2C busses.
  for (BusIdx = 0; BusIdx < TYU_SMBUS_CNT; BusIdx++) {
    if ((1 << BusIdx) & BusMask)
      I2cSmbusEnable (BusIdx);
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
      &gSmbusContext.Controller,
      &gEfiSmbusHcProtocolGuid,
      &gSmbusContext.SmbusHc,
      NULL
      );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
        "InitializeController: Failed to install protocol interfaces!\n"));
    return Status;
  }
  DEBUG ((DEBUG_ERROR,
    "InitializeController: Succesfully installed SMBus Host Controller\n"));

  // Initialize the EEPROM slave device
  I2cSmbusInstallEepromProtocol ();

  // Initialize the RTC slave device
  I2cSmbusInstallRtcProtocol ();

  // Initialize the IPMB device
  I2cSmbusInstallIpmbProtocol ();

  return EFI_SUCCESS;
}

//
// I2cSmbusRuntimeDxe entry point.
//
EFI_STATUS
EFIAPI
I2cSmbusInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;

  // Read Control Space base address.
  gBaseAddress  = TYU_BASE_ADDR;

  //
  // Initialize the I2C Controller
  //
  Status = I2cSmbusInitializeController ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
            "I2cSmbusInitialize: I2cSmbusInitializeController failed: %r\n",
             Status));
    return Status;
  }

  //
  // Declare the controller as EFI_MEMORY_RUNTIME and the memory region
  // as Runtime Memory Mapped IO.
  //
  Status = gDS->AddMemorySpace (
      EfiGcdMemoryTypeMemoryMappedIo,
      gBaseAddress, SIZE_32KB,
      EFI_MEMORY_UC | EFI_MEMORY_RUNTIME
      );
  if (EFI_ERROR (Status))
    DEBUG ((DEBUG_WARN,
            "I2cSmbusInitialize: AddMemorySpace failed: %r\n", Status));

  Status = gDS->SetMemorySpaceAttributes (
      gBaseAddress,
      SIZE_32KB,
      EFI_MEMORY_UC | EFI_MEMORY_RUNTIME
      );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
            "I2cSmbusInitialize: SetMemorySpaceAttributes failed: %r\n",
             Status));
    return Status;
  }

  //
  // Register for the virtual address change event
  //
  Status = gBS->CreateEventEx (
      EVT_NOTIFY_SIGNAL,
      TPL_NOTIFY,
      I2cSmbusSetVirtualAddressNotifyEvent,
      NULL,
      &gEfiEventVirtualAddressChangeGuid,
      &gSetVirtualAddressMapEvent
  );
  if (EFI_ERROR (Status))
    DEBUG ((EFI_D_ERROR,
            "I2cSmbusInitialise: Create event failed: %r\n", Status));

  return Status;
}

// This functions registers slave devices that are connected to the
// physical bus 'BusId' at address 'Address'.
EFI_STATUS
EFIAPI
I2cSmbusRegisterSlaveDevice (
  IN UINT8 BusId,
  IN UINT8 Address
  )
{
  SMBUS_INFO   *SmbusInfo;
  UINT8        *DevicesCnt;

  if (BusId >= TYU_SMBUS_CNT)
    return EFI_INVALID_PARAMETER;

  // Retrieve the SmbusInfo associated with the I2C bus and check
  // whether it is enabled.
  SmbusInfo  = &gSmbusInfo[BusId];
  if ((SmbusInfo->Status & I2C_S_ENABLED) == 0)
    return EFI_DEVICE_ERROR;

  DevicesCnt = &SmbusInfo->DevicesCnt;
  if (*DevicesCnt < I2C_SMBUS_MAX_DEVICES)
    SmbusInfo->Devices[(*DevicesCnt)++] = Address;
  else
    return EFI_DEVICE_ERROR;

  DEBUG ((DEBUG_ERROR,
          "I2cSmbusRegisterSlaveDevice: device registred at 0x%x, bus %u\n",
          Address, BusId));
  return EFI_SUCCESS;
}

// Stalls the CPU for the number of microseconds specified by MicroSeconds.
// Note that different calls are used whether at boot time or at runtime.
STATIC
VOID
I2cSmbusStall (
  IN UINTN MicroSeconds
  )
{
  if (EfiAtRuntime ())
    MicroSecondDelay(MicroSeconds);
  else
    gBS->Stall (MicroSeconds);
}

// Function to aquire lock only at boot time. It calls EfiAquireLock() at
// boot time, and simply returns at runtime. Note that 'UefiLib' cannot
// handle the EfiAquireLock() call at Runtime.
VOID
I2cSmbusAcquireLock (
  IN EFI_LOCK  *Lock
  )
{
  if (!EfiAtRuntime ()) {
    EfiAcquireLock (Lock);
  }
}

// Function to release lock only at boot time. It calls EfiReleaseLock()
// at boot time, and simply returns at runtime. Note that "UefiLib" cannot
// handle the EfiReleaseLock() call at Runtime.
VOID
I2cSmbusReleaseLock (
  IN EFI_LOCK  *Lock
  )
{
  if (!EfiAtRuntime ()) {
    EfiReleaseLock (Lock);
  }
}

// Function to poll a set of bits at a specific address, i.e. check if
// the bits are equal to zero when EqualZero is set to TRUE, and not equal
// to zero when eq_zero is set to FALSE.
STATIC
UINT32
I2cSmbusPoll (
  IN UINT32  Address,
  IN UINT32  Bitmask,
  IN BOOLEAN EqualZero,
  IN UINTN   Timeout
  )
{
  UINT32 Bits;

  Timeout = (Timeout / SMBUS_POLL_FREQ) + 1;

  do {
    Bits  = TYU_READ (Address) & Bitmask;
    if (EqualZero ? Bits == 0 : Bits != 0)
      return EqualZero ? 1 : Bits;
    I2cSmbusStall (SMBUS_POLL_FREQ);
  } while (Timeout-- != 0);

  return 0;
}

// The I2cSmbusRuntimeDxe driver MUST make sure that the SMBus Master GW
// is idle before using it. This function polls the Master FSM stop bit
// and returns whether the Master GW is idle.
STATIC
BOOLEAN
I2cSmbusMasterIsIdle (
  SMBUS_INFO *SmbusInfo
  )
{
  UINTN  Timeout     = SMBUS_START_TRANS_TIMEOUT;
  UINT32 FsmAddr     = SmbusInfo->Io.Smbus + SMBUS_MASTER_FSM;
  UINT32 FsmStopMask = SMBUS_MASTER_FSM_STOP_MASK;

  if (I2cSmbusPoll (FsmAddr, FsmStopMask, TRUE, Timeout))
    return TRUE;

  return FALSE;
}

// Wrapper function to poll Cause arbiter bits and return cause status.
// Note that timeout is given in microseconds.
STATIC
UINT16
I2cSmbusPollCauseStatus (
  IN SMBUS_INFO     *SmbusInfo,
  IN UINTN          Timeout
  )
{
  UINT32 CauseArbiterAddr = SmbusInfo->Io.CauseMaster + TYU_CAUSE_ARBITER_BITS;
  UINT32 CauseStatusMask  = CAUSE_ARBITER_BITS_MASK;

  return I2cSmbusPoll (CauseArbiterAddr, CauseStatusMask, FALSE, Timeout);
}

// Return SMBus Transaction status, i.e. whether succeeded or failed.
STATIC
EFI_STATUS
I2cSmbusReturnTransactionStatus (
  SMBUS_INFO    *SmbusInfo
  )
{
  UINT16 CauseStatusBits;
  UINT8  MasterStatusBits;

  //
  // First, poll cause status bits.
  //
  CauseStatusBits = I2cSmbusPollCauseStatus (SmbusInfo,
                                                SMBUS_TRANSFER_TIMEOUT);

  //
  // Parse both Cause and Master GW bits, and return transaction status.
  //

  MasterStatusBits  = TYU_READ (SmbusInfo->Io.Smbus + SMBUS_MASTER_STATUS);
  MasterStatusBits &= SMBUS_MASTER_STATUS_MASK;

  // When transaction ended with STOP, all bytes were transmitted,
  // and no NACK recieved, then the transaction ended successfully.
  // On the other hand, when the GW is configured with the stop bit
  // de-asserted then the SMBus expects the following GW configuration
  // for transfer continuation.
  if ((CauseStatusBits & CAUSE_WAIT_FOR_FW_DATA) ||
       ((CauseStatusBits & CAUSE_TRANSACTION_ENDED) &&
        (MasterStatusBits & SMBUS_STATUS_BYTE_CNT_DONE) &&
         !(MasterStatusBits & SMBUS_STATUS_NACK_RCV)))
    return EFI_SUCCESS;

  // In case of timeout on GW busy, the ISR will clear busy bit but
  // transaction ended bits cause will not be set so the transaction
  // fails. Then, we must check Master GW status bits.
  if ((CauseStatusBits & (CAUSE_TRANSACTION_ENDED | CAUSE_M_GW_BUSY_FALL))
       && (MasterStatusBits & (SMBUS_STATUS_NACK_RCV |
                     SMBUS_STATUS_READ_ERR | SMBUS_STATUS_FW_TIMEOUT)))
    return EFI_DEVICE_ERROR;

  if (CauseStatusBits & (CAUSE_M_ARBITRATION_LOST |
        CAUSE_UNEXPECTED_START | CAUSE_UNEXPECTED_STOP  |
        CAUSE_PUT_STOP_FAILED  | CAUSE_PUT_START_FAILED |
        CAUSE_CLK_TOGGLE_DONE  | CAUSE_M_FW_TIMEOUT))
    return EFI_DEVICE_ERROR;

  return EFI_NO_RESPONSE;
}

STATIC
SMBUS_INFO *
I2cSmbusGetSmbusInfoFromSlaveDeviceAddr (
  IN UINT8  DeviceAddress
  )
{
  SMBUS_INFO *SmbusInfo;
  UINT8      BusIdx;
  UINT8      DeviceIdx, DeviceCnt;

  // Retrieve the SmbusInfo associated with the I2C bus.
  for (BusIdx = 0; BusIdx < TYU_SMBUS_CNT; BusIdx++) {
    SmbusInfo = &gSmbusInfo[BusIdx];
    if ((SmbusInfo->Status & I2C_S_ENABLED) == 0)
      continue;

    DeviceCnt = SmbusInfo->DevicesCnt;
    for (DeviceIdx = 0; DeviceIdx < DeviceCnt; DeviceIdx++) {
      if (SmbusInfo->Devices[DeviceIdx] == DeviceAddress)
        return SmbusInfo;
    }
  }

  return NULL;
}

STATIC
UINT8
I2cSmbusPrepareControlBytes (
  IN UINT8      SlaveAddress,
  IN UINTN      Command,
  IN UINTN      Transmitted,
  IN OUT UINT32 *ControlWord
  )
{
  UINT8 ControlWordLength = 0;

  *ControlWord  = 0;
  // According to the specification, an SMBus command refers to the data
  // sent to the slave during SMBus cycles. It is usually sent after the
  // slave address byte. As an example the host controller tells the EEPROM
  // of the specific offset to be read from (or written to). In this case
  // command bytes refer to that offset. Note that the number of bytes sent
  // as a command strongly depends on the EEPROM address width.

  // Based on the slave device the host controller should determine the
  // control bytes and its length, to send before data bytes. These control
  // bytes usually include the SMBus command byte, and/or internal address
  // bytes (specific to the slave device CSRs).

  // Currently the host controller interacts with EEPROM and RTC devices, the
  // SMBus operation requires two 8-bit address following the device address
  // to interact with EEPROM and an 8-bit address word to interact with RTC.
  // So the length of the control bytes is set accordingly. In case where
  // different slave devices are supported, a mapping function between slave
  // device addresses and control width must be called.
  if (SlaveAddress == gEepromContext.Chip) {
    ControlWordLength = gEepromContext.AddressWidth;

    Command += Transmitted;
    // Set the control word which is prepended to data bytes in Master GW. The
    // control bytes holds the command bytes. These bytes are appended to the
    // slave address and are copied to the first word into the Master GW Data
    // Descriptor. Control bytes are expected to mainly vary between 1 and 4.
    *ControlWord |= ((Command >> 8) & 0xff) << 24; // Command MSB -> Byte[3]
    *ControlWord |= (Command        & 0xff) << 16; // Command LSB -> Byte[2]
                                                   // 0           -> Byte[1]
                                                   // 0           -> Byte[0]

  } else {
    // This configuration is the default one as defined by the SMBus spec.
    // Indeed, SMBus protocols define a single command byte following the
    // slave address byte. So, apply these settings to all slave devices
    // unless there is an exception as for the UVPS EEPROM above. Note that
    // the RTC uses the default configuration here.
    ControlWordLength = 1;

    Command += Transmitted;
    // Set the control word whcih corresponds to the internal address byte
    // of the device registers/memory.
    *ControlWord |= (Command & 0xff) << 24; // Command -> Byte[3]
                                            // 0       -> Byte[2]
                                            // 0       -> Byte[1]
                                            // 0       -> Byte[0]
  }

  return ControlWordLength;
}

STATIC
VOID
I2cSmbusReadData (
  IN SMBUS_INFO *SmbusInfo,
  IN OUT UINT8  *DataBuffer,
  IN UINTN      DataLength,
  IN UINT32     DataDescAddr,
  IN OUT UINTN  *Read
  )
{
  UINT8  ByteOff;
  UINT32 DataWord;
  UINT32 DataWordOff, DataWordAddr;

  DataWordOff  = 0;
  DataWordAddr = SmbusInfo->Io.Smbus + DataDescAddr;
  for (; DataWordOff < (DataLength & ~0x3); DataWordOff += 4, (*Read) += 4) {
    DataWord = TYU_READ_DATA (DataWordAddr + DataWordOff);
    *((UINT32 *)(DataBuffer + DataWordOff)) = DataWord;
  }

  if (!(DataLength & 0x3))
    return;

  DataWord = TYU_READ_DATA (DataWordAddr + DataWordOff);

  // Shift right remaining data.
  for (ByteOff = 0; ByteOff < (DataLength & 0x3); ByteOff++, (*Read)++) {
    DataBuffer[DataWordOff + ByteOff] = DataWord & 0xff;
    DataWord >>= 8;
  }
}

STATIC
EFI_STATUS
I2cSmbusRead (
  IN UINT8              SlaveAddress,
  IN OUT UINT8          *DataBuf,
  IN UINTN              DataLength,
  IN OUT UINTN          *ByteRead,
  IN UINT8              PecEnable
  )
{
  UINT8      ReadSize;
  UINT32     ControlWord;
  UINT32     ControlData;
  UINT8      DataDescLength;
  SMBUS_INFO *SmbusInfo;
  EFI_STATUS Status = EFI_SUCCESS;

  SmbusInfo = I2cSmbusGetSmbusInfoFromSlaveDeviceAddr (SlaveAddress);
  if (!SmbusInfo)
    return EFI_NOT_FOUND;

  *ByteRead = 0;
  // The Smbus Data Read flow:
  // - The control bits are copied to the first Master GW control word.
  // - The slave address byte is copied to the first data word in the Master
  //   GW Data Descriptor.
  // Note that Master GW data is shifted left so the data will start at the
  // beginning.
  I2cSmbusAcquireLock (&gSmbusContext.Lock);

  // Check whether the SMBus Master GW is idle.
  if (!I2cSmbusMasterIsIdle (SmbusInfo))
    goto out;

  // Write Slave address to the MSB of control data. Slave address is shifted
  // left by 1 as required by hardware.
  ControlData  = (SlaveAddress & 0x7f) << 1;

  while (DataLength > 0) {
    // Write control data into Master GW Data Descriptor.
    TYU_WRITE_DATA (SmbusInfo->Io.Smbus + MASTER_DATA_DESC_ADDR, ControlData);

    // Set number of data bytes to read. Unlike the write, the read operation
    // allows the master controller to read up to 128 bytes.
    DataDescLength = (DataLength <= MASTER_DATA_R_LENGTH ? \
                                DataLength : MASTER_DATA_R_LENGTH);
    ReadSize       = DataDescLength - 1; // The HW requires that SW
                                         // subtract one byte.

    // Set Master GW control word.
    ControlWord  = 0;
    ControlWord |= 0x1               << MASTER_LOCK_BIT_OFF;
    ControlWord |= 0x1               << MASTER_BUSY_BIT_OFF;
    ControlWord |= SlaveAddress      << MASTER_SLV_ADDR_BIT_OFF;
    ControlWord |= 0x1               << MASTER_START_BIT_OFF;
    ControlWord |= 0x1               << MASTER_STOP_BIT_OFF;
    ControlWord |= ReadSize          << MASTER_READ_BIT_OFF;
    ControlWord |= 0x1               << MASTER_CTL_READ_BIT_OFF;
    ControlWord |= 0                 << MASTER_WRITE_BIT_OFF;
    ControlWord |= 0                 << MASTER_CTL_WRITE_BIT_OFF;
    ControlWord |= 0                 << MASTER_PARSE_EXP_BIT_OFF;
    ControlWord |= (PecEnable & 0x1) << MASTER_SEND_PEC_BIT_OFF;

    // Clear status bits.
    TYU_WRITE (SmbusInfo->Io.Smbus       + SMBUS_MASTER_STATUS,      0x0);
    // Set the cause data.
    TYU_WRITE (SmbusInfo->Io.CauseMaster + TYU_CAUSE_OR_CLEAR_BITS, ~0x0);
    // Zero PEC byte.
    TYU_WRITE (SmbusInfo->Io.Smbus       + SMBUS_MASTER_PEC,         0x0);
    // Zero sent and received byte count.
    TYU_WRITE (SmbusInfo->Io.Smbus       + SMBUS_RS_BYTES,           0x0);

    // GW activation
    TYU_WRITE (SmbusInfo->Io.Smbus       + SMBUS_MASTER_GW, ControlWord);

    // Poll cause status and check master status bits.
    Status = I2cSmbusReturnTransactionStatus (SmbusInfo);
    if (EFI_ERROR (Status))
      break;

    // Read data from Master GW Data registers.
    I2cSmbusReadData (SmbusInfo,
                      &DataBuf[(*ByteRead)],
                      DataDescLength,
                      MASTER_DATA_DESC_ADDR,
                      ByteRead);

    // After a read operation the SMBus FSM ps (present state) needs to be
    // 'manually' reset. This should be removed in next tag integration.
    TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_MASTER_FSM,
                SMBUS_MASTER_FSM_PS_STATE_MASK);

    // Update remaining data bytes to read.
    DataLength  -= DataDescLength;
  }

out:
  I2cSmbusReleaseLock (&gSmbusContext.Lock);
  return Status;
}

STATIC
VOID
I2cSmbusWriteData (
  IN IN SMBUS_INFO    *SmbusInfo,
  IN OUT CONST UINT8  *DataBuf,
  IN UINTN            DataLength,
  IN UINT32           DataDescAddr
  )
{
  UINT32 DataWord;
  UINT32 DataWordOff, DataWordAddr;

  DataWordOff  = 0;
  DataWordAddr = SmbusInfo->Io.Smbus + DataDescAddr;
  // Write data bytes to the Smbus GW Data Descriptor registers.
  // Note that 'DataBuf' MUST be a 4-byte aligned buffer.
  for (; DataWordOff < ALIGN_ROUNDUP (DataLength, 4); DataWordOff += 4) {
    DataWord  = *((UINT32 *)(DataBuf + DataWordOff));
    TYU_WRITE_DATA (DataWordAddr + DataWordOff, DataWord);
  }
}

STATIC
EFI_STATUS
I2cSmbusWrite (
  IN UINT8              SlaveAddress,
  IN UINT32             Command,
  IN OUT CONST UINT8    *DataBuf,
  IN UINTN              DataLength,
  IN OUT UINTN          *ByteSent,
  IN UINTN              PecEnable
  )
{
  SMBUS_INFO *SmbusInfo;
  UINT8      DataDesc[MASTER_DATA_DESC_SIZE] = { 0 };
  UINT8      DataDescLength, DataByteLength, WriteSize;
  UINT8      ByteIdx, ByteOff;
  UINT8      StartBit, StopBit;
  UINT8      ControlWordLength;
  UINT32     ControlWord, ControlData;
  BOOLEAN    LastTransaction, FirstTransaction;
  EFI_STATUS Status = EFI_SUCCESS;

  SmbusInfo = I2cSmbusGetSmbusInfoFromSlaveDeviceAddr (SlaveAddress);
  if (!SmbusInfo)
    return EFI_NOT_FOUND;

  *ByteSent = 0;
  // The Smbus Data Write flow:
  // - The control bits are copied to the first Master GW control word.
  // - The slave address byte and SMBus command bytes are copied to the first
  //   data word in the Master GW Data Descriptor, followed by the data bytes.
  // Note that Master GW data is shifted left so the data will start at the
  // beginning.
  I2cSmbusAcquireLock (&gSmbusContext.Lock);

  // Check whether the SMBus Master GW is idle.
  if (!I2cSmbusMasterIsIdle (SmbusInfo))
    goto out;

  FirstTransaction = TRUE;
  LastTransaction  = FALSE;

  do {

    // The write transaction might require sending more than
    // MASTER_DATA_W_LENGTH bytes, i.e. Master GW data limit. Note that
    // the EEPROM might be capable of a PAGE write cycle which writes up
    // to 128 data bytes.
    //
    // The SMBus is able to handle such transactions through multiple GW
    // configurations. For instance, if one needs to write 260 bytes, then
    // we need three GW configurations to perform that transaction; The first
    // GW load will include a START token, but no STOP token because the Master
    // needs to write more than 128 bytes (including the slave address).
    // The second GW load will include neither a START token nor a STOP
    // token. Data bytes will start at the beginning of the Master GW
    // Data descriptor registers and the slave address is omitted. The
    // last GW load will include a STOP token only, as well as the
    // remaining data.
    // Note that this kind of transaction was tested on PD; the SMBus Master
    // GW works properly, however the EEPROM device attached to the bus does
    // not support it. Right now, the FastModels support this improved SMBus
    // transaction format, but once the real hardware is available and/or the
    // EEPROM on PD gets fixed, I will run new experiments and update this
    // code accordingly.

    ByteIdx           = 0;
    ControlWordLength = 0;

    if (FirstTransaction) {
      // Prepare the control bytes and its length, to send before data bytes.
      // Control bytes strongly depend on the SMBus Command as well as the
      // slave device i.e. these bytes enables the control of the slave device.
      ControlWordLength = I2cSmbusPrepareControlBytes (SlaveAddress,
                                                       Command,
                                                       *ByteSent,
                                                       &ControlData);
      // Write Slave address to the data descriptor buffer. Slave address is
      // shifted left by 1 as required by hardware.
      DataDesc[ByteIdx++] = (SlaveAddress & 0x7f) << 1;

      // Copy control data to data descriptor buffer.
      for (ByteOff = 0; ByteOff < ControlWordLength; ByteOff++)
        DataDesc[ByteIdx++] = (ControlData >> (24 - (8 * ByteOff))) & 0xff;
    }

    // Set the number of data bytes to write. Unlike the read, the write
    // operation allows the master controller to write up to 127 bytes only.
    // The first data bytes might be reserved for control bytes (e.g. command
    // bytes). The total number of bytes to write to the Master GW data
    // descriptor includes the slave address byte, control bytes and data
    // bytes.
    // On the other hand, when multiple GW configurations are need, then the
    // master can send up to 128 bytes except for the initial GW configuration.
    // The total number of bytes to write to the Master GW data descriptor
    // will include the data bytes only.
    if (DataLength <= (MASTER_DATA_W_LENGTH - ControlWordLength)) {
      DataByteLength  = DataLength;
      LastTransaction = TRUE;
    } else {
      DataByteLength  = MASTER_DATA_W_LENGTH - ControlWordLength;
    }

    // Add the slave address byte and the control bytes to data descriptor
    // length, when issuing the first transaction. Note that the length of
    // the control bytes would be zero for the following write transactions.
    DataDescLength  = ControlWordLength + DataByteLength;
    DataDescLength += (FirstTransaction) ? 1 : 0;

    WriteSize       = DataDescLength - 1; // The HW requires that SW
                                          // subtract one byte.

    // Copy data to write to data descriptor (non-aligned) buffer.
    for (; ByteIdx < DataDescLength; ByteIdx++)
      DataDesc[ByteIdx] = DataBuf[(*ByteSent)++];

    // Copy the data descriptor to the Master GW data registers.
    I2cSmbusWriteData (SmbusInfo, DataDesc, DataDescLength,
                        MASTER_DATA_DESC_ADDR);

    StartBit = FirstTransaction ? 1 : 0;
    StopBit  = LastTransaction  ? 1 : 0;

    // Set Master GW control word.
    ControlWord  = 0;
    ControlWord |= 0x1               << MASTER_LOCK_BIT_OFF;
    ControlWord |= 0x1               << MASTER_BUSY_BIT_OFF;
    ControlWord |= SlaveAddress      << MASTER_SLV_ADDR_BIT_OFF;
    ControlWord |= StartBit          << MASTER_START_BIT_OFF;
    ControlWord |= StopBit           << MASTER_STOP_BIT_OFF;
    ControlWord |= 0                 << MASTER_READ_BIT_OFF;
    ControlWord |= 0                 << MASTER_CTL_READ_BIT_OFF;
    ControlWord |= WriteSize         << MASTER_WRITE_BIT_OFF;
    ControlWord |= 0x1               << MASTER_CTL_WRITE_BIT_OFF;
    ControlWord |= 0                 << MASTER_PARSE_EXP_BIT_OFF;
    ControlWord |= (PecEnable & 0x1) << MASTER_SEND_PEC_BIT_OFF;

    // Clear status bits.
    TYU_WRITE (SmbusInfo->Io.Smbus       + SMBUS_MASTER_STATUS,      0x0);
    // Set the cause data.
    TYU_WRITE (SmbusInfo->Io.CauseMaster + TYU_CAUSE_OR_CLEAR_BITS, ~0x0);
    // Zero PEC byte.
    TYU_WRITE (SmbusInfo->Io.Smbus       + SMBUS_MASTER_PEC,         0x0);
    // Zero sent and received byte count.
    TYU_WRITE (SmbusInfo->Io.Smbus       + SMBUS_RS_BYTES,           0x0);

    // GW activation
    TYU_WRITE (SmbusInfo->Io.Smbus       +  SMBUS_MASTER_GW, ControlWord);

    FirstTransaction = FALSE;

    // Poll master status and check status bits. An ACK is sent when completing
    // writing data to the bus (Master 'byte_count_done' bit is set to 1).
    Status = I2cSmbusReturnTransactionStatus (SmbusInfo);
    if (EFI_ERROR (Status))
      break;

    // EEPROM devices require 5ms delay after I2C Write to copy out data
    // from the cache.
    if (!IsSimulator())
      I2cSmbusStall (SMBUS_WRITE_TRANS_TIMEOUT);

    // Update remaining data bytes to write. Note that control bytes does not
    // count. Indeed, these bytes are required by the write transfer.
    DataLength -= DataByteLength;
  } while (DataLength > 0);

out:
  I2cSmbusReleaseLock (&gSmbusContext.Lock);
  return Status;
}

STATIC
EFI_STATUS
I2cSmbusStartTransaction (
  IN UINT8                    SlaveAddress,
  IN UINTN                    Command,
  IN EFI_I2C_REQUEST_PACKET   *RequestPacket,
  IN OUT UINTN                *Transmitted,
  IN UINT8                    PecCheckBit
  )
{
  EFI_STATUS         Status;
  UINTN              Count;
  UINTN              ReadMode;
  EFI_I2C_OPERATION  *Operation;

  ASSERT (RequestPacket != NULL);

  Status = EFI_SUCCESS;

  for (Count = 0; Count < RequestPacket->OperationCount; Count++) {
    Operation     = &RequestPacket->Operation[Count];
    ReadMode      = Operation->Flags & I2C_FLAG_READ;

    if (ReadMode) {
      Status = I2cSmbusRead (SlaveAddress,
                             Operation->Buffer,
                             Operation->LengthInBytes,
                             Transmitted,
                             PecCheckBit
                             );
    } else {
      Status = I2cSmbusWrite (SlaveAddress,
                              Command,
                              Operation->Buffer,
                              Operation->LengthInBytes,
                              Transmitted,
                              PecCheckBit
                              );
    }

    // Check for errors while reading or writing data.
    if (EFI_ERROR (Status))
      break;
  }

  return Status;
}

// The Execute() function provides a standard way to execute an operation as
// defined in the System Management Bus (SMBus) Specification. The resulting
// transaction will be either that the SMBus slave devices accept it or that
// this function returns with error.
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
  )
{
  EFI_STATUS Status       = EFI_SUCCESS;
  UINT8      SlaveAddr    = (UINT8)(SlaveAddress.SmbusDeviceAddress);

  // Note that this SMBus hardware protocol is defined by the System Management
  // Bus (SMBus) Specification and is not related to UEFI, i.e. not related to
  // Platform Initialization (PI) Architecture. We assume that this particular
  // SMBus hardware protocol instance is able to recreate slave device cycles
  // using customized 'WriteBlock' and 'ReadBlock' transactions only. These
  // transactions are used to interface I2C devices to the Mellanox SMBus host
  // controller.
  // Note that EFI_SMBUS_OPERATION usage might be optional. Indeed the host
  // controller is supposed to handle all of the SMBus protocols. Currently,
  // EfiSmbusWriteBlock/EfiSmbusReadBlock refer to all of the write/read cycles
  // and are used by a single consumer, i.e. the BLUEFIELD_EEPROM_PROTOCOL.
  // This might be updated when multiple consumers are available.
  if ((Operation != EfiSmbusWriteBlock) &&
          (Operation != EfiSmbusReadBlock))
    return EFI_UNSUPPORTED;

  // Start the I2C SMBus transaction.
  Status = I2cSmbusStartTransaction (SlaveAddr,
                                     Command,
                                     (EFI_I2C_REQUEST_PACKET *) Buffer,
                                     Length,
                                     (PecCheck == TRUE)
                                     );
  return Status;
}

// The ArpDevice() function provides a standard way for a device driver to
// enumerate the entire SMBus or specific devices on the bus.
//
// *TBD*
// Currently we are unsure whether the SMBus should provide enumeration or not.
// Note that information about attached devices should be provided through PCDs.
// In fact hardware-based I2C enumeration isn't safe.
EFI_STATUS
EFIAPI
I2cSmbusArpDevice (
  IN CONST EFI_SMBUS_HC_PROTOCOL    *This,
  IN       BOOLEAN                  ArpAll,
  IN       EFI_SMBUS_UDID           *SmbusUdid OPTIONAL,
  IN OUT   EFI_SMBUS_DEVICE_ADDRESS *SlaveAddress OPTIONAL
  )
{
  return EFI_UNSUPPORTED;
}

// The GetArpMap() function returns the mapping of all the SMBus devices that
// were enumerated by the SMBus host driver. Note that this function is not
// supported.
EFI_STATUS
EFIAPI
I2cSmbusGetArpMap (
  IN CONST EFI_SMBUS_HC_PROTOCOL    *This,
  IN OUT   UINTN                    *Length,
  IN OUT   EFI_SMBUS_DEVICE_MAP     **SmbusDeviceMap
  )
{
  return EFI_UNSUPPORTED;
}

// The Notify() function registers all the callback functions to allow the
// bus driver to call these functions when the SlaveAddress/Data pair happens.
// Note that this function is not supported.
EFI_STATUS
EFIAPI
I2cSmbusNotify (
  IN CONST  EFI_SMBUS_HC_PROTOCOL     *This,
  IN CONST  EFI_SMBUS_DEVICE_ADDRESS  SlaveAddress,
  IN CONST  UINTN                     Data,
  IN CONST  EFI_SMBUS_NOTIFY_FUNCTION NotifyFunction
  )
{
  return EFI_UNSUPPORTED;
}

//
// Smbus Slave Controller implementation.
//

// This function enables the address of the Smbus slave.
EFI_STATUS
EFIAPI
I2cSmbusSlaveEnableAddress (
  IN        UINT8  Bus,
  IN        UINT8  Slave
  )
{
  SMBUS_INFO  *SmbusInfo;
  UINT8       Reg, RegCnt, Byte, TmpAddr, AvailReg, AvailByte;
  BOOLEAN     Avail, Disabled;
  UINT32      SlaveReg, TmpSlaveReg, AvailSlaveReg;

  ASSERT (Bus < TYU_SMBUS_CNT);

  SmbusInfo = &gSmbusInfo[Bus];
  Avail     = FALSE;
  Disabled  = FALSE;

  RegCnt = SMBUS_SLAVE_ADDR_CNT >> 2;

  // Read the slave registers. There are 4 * 32-bit slave registers.
  // Each slave register can hold up to 4 * 8-bit slave configuration
  // (7-bit address, 1 status bit (1 if enabled, 0 if not)).
  for (Reg = 0; Reg < RegCnt; Reg++) {
    SlaveReg = TYU_READ (SmbusInfo->Io.Smbus +
                          SMBUS_SLAVE_ADDR_CFG + (Reg * 0x4));

    // Each register holds 4 slave addresses. So, we have to keep
    // the byte order consistent with the value read in order to
    // update the register correctly, if needed.
    TmpSlaveReg = SlaveReg;
    for (Byte = 0; Byte < 4; Byte++) {
      TmpAddr = TmpSlaveReg & 0xff;

      // Mark the first available slave address slot, i.e. its
      // enabled bit should be unset. This slot might be used
      // later on to register our slave.
      if (!Avail && !(TmpAddr & (1 << SMBUS_SLAVE_ADDR_EN_BIT))) {
        Avail         = TRUE;
        AvailReg      = Reg;
        AvailByte     = Byte;
        AvailSlaveReg = SlaveReg;
      }

      // Parse slave address bytes and check whether the
      // slave address already exists and it's enabled,
      // i.e. most significant bit is set.
      if ((TmpAddr & SMBUS_SLAVE_ADDR_MASK) == Slave) {
        if (TmpAddr & (1 << SMBUS_SLAVE_ADDR_EN_BIT))
          return EFI_SUCCESS;
        Disabled = TRUE;
        break;
      }

      // Parse next byte
      TmpSlaveReg >>= 8;
    }

    // Exit the loop if the slave address is found
    if (Disabled)
      break;
  }

  if (!Avail && !Disabled)
    return EFI_OUT_OF_RESOURCES;  // No room for a new slave address

  if (Avail) {
    Reg  = AvailReg;
    Byte = AvailByte;
    // Set the slave address
    AvailSlaveReg |= (Slave << (Byte * 8));
    SlaveReg        = AvailSlaveReg;
  }

  // Enable the slave address and update the register.
  SlaveReg |= ((1 << SMBUS_SLAVE_ADDR_EN_BIT) << (Byte * 8));
  TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_SLAVE_ADDR_CFG + (Reg * 0x4),
              SlaveReg);

  return EFI_SUCCESS;
}

// This function disables the address of the Smbus slave.
EFI_STATUS
EFIAPI
I2cSmbusSlaveDisableAddress (
  IN        UINT8  Bus,
  IN        UINT8  Slave
  )
{
  SMBUS_INFO  *SmbusInfo;
  UINT32      SlaveReg, TmpSlaveReg;
  UINT8       TmpAddr, Reg, RegCnt, Byte;
  BOOLEAN     Exist;

  ASSERT (Bus < TYU_SMBUS_CNT);

  SmbusInfo = &gSmbusInfo[Bus];
  Exist     = FALSE;
  RegCnt    = SMBUS_SLAVE_ADDR_CNT >> 2;

  // Read the slave registers. There are 4 * 32-bit slave registers.
  // Each slave register can hold up to 4 * 8-bit slave configuration
  // (7-bit address, 1 status bit (1 if enabled, 0 if not)).
  for (Reg = 0; Reg < RegCnt; Reg++) {
    SlaveReg = TYU_READ (SmbusInfo->Io.Smbus +
                          SMBUS_SLAVE_ADDR_CFG + (Reg * 0x4));

    // Check whether the address slots are empty.
    if (SlaveReg == 0)
      continue;

    // Each register holds 4 slave addresses. So, we have to keep
    // the byte order consistent with the value read in order to
    // update the register correctly, if needed.
    TmpSlaveReg = SlaveReg;
    Byte        = 0;
    while (TmpSlaveReg != 0) {
      TmpAddr = TmpSlaveReg & SMBUS_SLAVE_ADDR_MASK;
      // Parse slave address bytes and check whether the
      // slave address already exists.
      if (TmpAddr == Slave) {
        Exist = TRUE;
        break;
      }

      // Parse next byte
      TmpSlaveReg >>= 8;
      Byte         += 1;
    }

    // Exit the loop if the slave address is found.
    if (Exist)
      break;
  }

  if (!Exist)
    return EFI_SUCCESS;   // slave is not registered, nothing to do.

  // Disable the slave address slot.
  SlaveReg &= (SMBUS_SLAVE_ADDR_MASK << (Byte * 8));
  TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_SLAVE_ADDR_CFG + (Reg * 0x4),
               SlaveReg);

  return EFI_SUCCESS;
}

// Enable the slave function in hardware; this should turn on the
// interrupt bits when a master write/read requests are received.
STATIC
VOID
I2cSmbusSlaveConfigure (
  SMBUS_INFO    *SmbusInfo
  )
{
  UINT32 IntReg;

  TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_SLAVE_FSM, 0); // reset FSM

  // Enable slave cause interrupt bits. Drive CAUSE_READ_WAIT_FW_RESPONSE
  // and CAUSE_WRITE_SUCCESS, these are enabled when an external masters
  // issue a Read and Write, respectively. But, clear all interrupts
  // first.
  TYU_WRITE (SmbusInfo->Io.CauseSlave + TYU_CAUSE_OR_CLEAR_BITS, ~0);
  IntReg = CAUSE_READ_WAIT_FW_RESPONSE | CAUSE_WRITE_SUCCESS;
  TYU_WRITE (SmbusInfo->Io.CauseSlave + TYU_CAUSE_OR_EVTEN0_BITS, IntReg);

  // Finally, set the 'ready' bit to start handling transactions
  TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_SLAVE_READY, 0x1);
}

// This function initialize the Smbus Slave hardware with the given
// bus and slave parameters.
EFI_STATUS
EFIAPI
I2cSmbusSlaveInitialize (
  IN UINT8 Bus,
  IN UINT8 Slave
  )
{
    EFI_STATUS  Status;
    SMBUS_INFO  *SmbusInfo;
    UINT8       *SlavesCnt;

    if (Bus >= TYU_SMBUS_CNT)
      return EFI_INVALID_PARAMETER;

    SmbusInfo = &gSmbusInfo[Bus];
    if ((SmbusInfo->Status & I2C_S_ENABLED) == 0)
      return EFI_NOT_READY;

    SlavesCnt = &SmbusInfo->SlavesCnt;
    if (*SlavesCnt >= SMBUS_SLAVE_ADDR_CNT)
      return EFI_OUT_OF_RESOURCES;

    Status = I2cSmbusSlaveEnableAddress (Bus, Slave);
    if (EFI_ERROR (Status))
      return Status;

    // Smbus slave initialization
    I2cSmbusSlaveConfigure (SmbusInfo);

    SmbusInfo->Slaves[(*SlavesCnt)++] = Slave;
    // Mark the I2C Smbus slave function as enabled.
    SmbusInfo->Status                |= I2C_S_SLAVE;

    DEBUG ((DEBUG_ERROR,
            "I2cSmbusSlaveInitialize: Smbus slave %u initialized at 0x%x\n",
            Bus, Slave));

    return EFI_SUCCESS;
}

// This function polls the coalesce bit that indicate whether I2C master
// requests are recieved.
STATIC
BOOLEAN
I2cSmbusPollCoalesce (
  IN     SMBUS_INFO    *SmbusInfo,
  IN     BOOLEAN       Read,
  IN OUT UINT8         *RecvBytes
  )
{
  UINT32  Coalesce0Reg, CauseReg;
  UINT8   IsSet, BusSlave, SlaveShift;
  UINT64  CpuFrequency;
  UINTN   Ticks, Timeout;
  BOOLEAN HasCoalesce;
  UINT32  RwBytesReg;

  ASSERT (RecvBytes != NULL);

  IsSet       = 0;
  *RecvBytes  = 0;
  HasCoalesce = FALSE;
  Timeout     = SMBUS_WAIT_FOR_COALESCE_TIMEOUT;

  BusSlave    = SmbusInfo->Id;
  // Slave interrupt bits start after master interrupt bits. Set slave
  // bit shift, then.
  SlaveShift  = TYU_SMBUS_CNT;

  // Convert polling timeout to ticks; first determine the A72 core
  // frequency from PLL configuration, then derive the ticks:
  //
  //    Ticks = Time (s) x CPU Frequency (Hz)
  //
  CpuFrequency = I2cSmbusGetCoreFrequency (TYU_CORE_PLL_0_LSB);
  Ticks        = I2cSmbusGetTicks (CpuFrequency, Timeout, 1);

  while (Ticks-- != 0) {
    Coalesce0Reg = TYU_READ (TYU_CAUSE_COALESCE_ADDR);
    IsSet        = Coalesce0Reg & (1 << (BusSlave + SlaveShift));
    if (IsSet)
        break;
  }

  if (!IsSet || (Ticks == 0))
    return FALSE;

  // Check the source of the interrupt, i.e. whether a Read or Write
  CauseReg = TYU_READ (SmbusInfo->Io.CauseSlave + TYU_CAUSE_ARBITER_BITS);
  if ((CauseReg & CAUSE_READ_WAIT_FW_RESPONSE) && Read)
    HasCoalesce = TRUE;
  else if ((CauseReg & CAUSE_WRITE_SUCCESS) && !Read)
    HasCoalesce = TRUE;

  // Clear cause bits
  TYU_WRITE (SmbusInfo->Io.CauseSlave + TYU_CAUSE_OR_CLEAR_BITS, ~0x0);

  if (HasCoalesce) {
    // The SMBUS_SLAVE_RS_MASTER_BYTES includes the number of bytes
    // from/to master. These are defined by 8-bits each. If the lower
    // 8 bits are set, then the master expects to read N bytes from
    // the slave, if the higher 8 bits are set then the slave expect
    // N bytes from the master.
    RwBytesReg = TYU_READ (SmbusInfo->Io.Smbus + SMBUS_SLAVE_RS_MASTER_BYTES);
    *RecvBytes  = (RwBytesReg >> 8) & 0xff;

    // For now, the slave supports 128 bytes transfer. Discard remaining
    // data bytes if the master wrote more than SLAVE_DATA_DESC_SIZE, i.e,
    // the actual size of the slave data descriptor.
    //
    // Note that we will never expect to transfer more than 128 bytes; as
    // specified in the SMBus standard, block transactions cannot exceed
    // 32 bytes.
    *RecvBytes = (*RecvBytes > SLAVE_DATA_DESC_SIZE) ?
                   SLAVE_DATA_DESC_SIZE : *RecvBytes;
  }

  return HasCoalesce;
}

STATIC
SMBUS_INFO *
I2cSmbusGetSmbusInfoFromSlave (
  IN UINT8  Slave
  )
{
  SMBUS_INFO *SmbusInfo;
  UINT8      BusIdx;
  UINT8      SlaveIdx, SlaveCnt;

  // Retrieve the SmbusInfo associated with the I2C bus.
  for (BusIdx = 0; BusIdx < TYU_SMBUS_CNT; BusIdx++) {
    SmbusInfo = &gSmbusInfo[BusIdx];
    if (SmbusInfo->Status != (I2C_S_ENABLED | I2C_S_SLAVE))
      continue;

    SlaveCnt = SmbusInfo->SlavesCnt;
    for (SlaveIdx = 0; SlaveIdx < SlaveCnt; SlaveIdx++) {
      if (SmbusInfo->Slaves[SlaveIdx] == Slave)
        return SmbusInfo;
    }
  }

  return NULL;
}

STATIC
BOOLEAN
I2cSmbusSlaveWaitForIdle (
  SMBUS_INFO    *SmbusInfo,
  UINT32        Timeout
  )
{
  UINT32 CauseArbiterAddr = SmbusInfo->Io.CauseSlave + TYU_CAUSE_ARBITER_BITS;
  UINT32 CauseGwBusyMask = CAUSE_S_GW_BUSY_FALL;

  if (I2cSmbusPoll (CauseArbiterAddr, CauseGwBusyMask, FALSE, Timeout))
    return TRUE;

  return FALSE;
}

// Send byte to 'external' SMBUS master
EFI_STATUS
EFIAPI
I2cSmbusSlaveSend (
  IN       UINT8 Slave,
  IN OUT   UINTN *ByteWritten,
  IN CONST EFI_SMBUS_SLAVE_FUNCTION SlaveFunction
  )
{
  EFI_STATUS  Status;
  SMBUS_INFO  *SmbusInfo;
  UINT8       WriteSize, PecEn, Address, Byte, ByteCnt;
  UINT8       DataDesc[SLAVE_DATA_DESC_SIZE] = { 0 };
  UINT32      Control32, Data32;
  UINT8       RecvBytes = 0;

  SmbusInfo = I2cSmbusGetSmbusInfoFromSlave (Slave);
  if (!SmbusInfo)
    return EFI_NOT_FOUND;

  // Check whether the 'master' issued a write request.
  if (!I2cSmbusPollCoalesce (SmbusInfo, FALSE, &RecvBytes))
    return EFI_TIMEOUT;

  Address = Byte = 0;
  // Read bytes received from the external master. These bytes should
  // be located in the first data descriptor register of the slave GW.
  // These bytes are the slave address byte and the internal register
  // address, if supplied.
  if (RecvBytes > 0) {
    Data32 = TYU_READ_DATA (SmbusInfo->Io.Smbus + SLAVE_DATA_DESC_ADDR);

    // Parse the received bytes
    switch (RecvBytes) {
    case 2:
      Byte = (Data32 >> 8) & 0xff;
      // fall-through
    case 1:
      Address = (Data32 & 0xff) >> 1;
    }

    // Check whether it's our slave address.
    if (Slave != Address)
      return EFI_DEVICE_ERROR;
  }

  // I2C read transactions may start by a WRITE followed by a READ.
  // Indeed, most slave devices would expect the internal address
  // following the slave address byte. So, write that byte first,
  // and then, send the requested data bytes to the master.
  if (RecvBytes > 1) {
    Status = SlaveFunction (Slave, &Byte, 1,
                             SMBUS_SLAVE_WRITE_REQUESTED, NULL);
    if (EFI_ERROR (Status))
      return Status;
  }

  // Now, send data to the master; currently, the driver supports
  // READ_BYTE, READ_WORD and BLOCK READ protocols. Note that the
  // hardware can send up to 128 bytes per transfer. That is the
  // size of its data registers.
  SlaveFunction (Slave, DataDesc, SLAVE_DATA_DESC_SIZE,
                  SMBUS_SLAVE_READ_REQUESTED, &ByteCnt);
  *ByteWritten = ByteCnt;

  // Send a stop condition to the backend.
  SlaveFunction (Slave, NULL, 0, SMBUS_SLAVE_STOP, NULL);

  //
  // Handle the actual transfer.
  //

  // Set the number of bytes to write to master
  WriteSize = (ByteCnt - 1) & 0x7f;

  // Write data to Slave GW data descriptor
  I2cSmbusWriteData (SmbusInfo, DataDesc, ByteCnt, SLAVE_DATA_DESC_ADDR);

  PecEn = 0; // Disable PEC since it is not supported.

  // Prepare control word
  Control32  = 0;
  Control32 |= 0         << SLAVE_LOCK_BIT_OFF;
  Control32 |= 1         << SLAVE_BUSY_BIT_OFF;
  Control32 |= 1         << SLAVE_WRITE_BIT_OFF;
  Control32 |= WriteSize << SLAVE_WRITE_BYTES_BIT_OFF;
  Control32 |= PecEn     << SLAVE_SEND_PEC_BIT_OFF;

  TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_SLAVE_GW, Control32);

  // Wait until the transfer is completed; the driver will wait
  // until the GW is idle, a cause will rise on fall of GW busy.
  I2cSmbusSlaveWaitForIdle (SmbusInfo, SMBUS_TRANSFER_TIMEOUT);

  // Release the Slave GW
  TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_SLAVE_RS_MASTER_BYTES, 0x0);
  TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_SLAVE_PEC,             0x0);
  TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_SLAVE_READY,           0x1);

  return EFI_SUCCESS;
}

// Receive bytes from 'external' smbus master.
EFI_STATUS
EFIAPI
I2cSmbusSlaveReceive (
  IN       UINT8 Slave,
  IN OUT   UINTN *ByteRead,
  IN CONST EFI_SMBUS_SLAVE_FUNCTION SlaveFunction
  )
{
  SMBUS_INFO *SmbusInfo;
  UINT8      DataDesc[SLAVE_DATA_DESC_SIZE] = { 0 };
  UINT8      Address;
  UINT8      RecvBytes = 0;

  SmbusInfo = I2cSmbusGetSmbusInfoFromSlave (Slave);
  if (!SmbusInfo)
    return EFI_NOT_FOUND;

  // Check whether the 'master' issued a write request.
  if (!I2cSmbusPollCoalesce (SmbusInfo, FALSE, &RecvBytes))
    return EFI_TIMEOUT;

  // Read data from Slave GW data descriptor.
  I2cSmbusReadData (SmbusInfo, DataDesc, RecvBytes, SLAVE_DATA_DESC_ADDR,
                     ByteRead);

  // Check whether its our slave address.
  Address = DataDesc[0] >> 1;
  if (Slave != Address)
    return EFI_DEVICE_ERROR;

  // Copy out the received data to the slave backend.
  SlaveFunction (Slave, DataDesc, RecvBytes,
                  SMBUS_SLAVE_WRITE_REQUESTED, NULL);

  // Send a stop condition to the backend.
  SlaveFunction (Slave, NULL, 0, SMBUS_SLAVE_STOP, NULL);

  // Release the Slave GW.
  TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_SLAVE_RS_MASTER_BYTES, 0x0);
  TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_SLAVE_PEC,             0x0);
  TYU_WRITE (SmbusInfo->Io.Smbus + SMBUS_SLAVE_READY,           0x1);

  return EFI_SUCCESS;
}
