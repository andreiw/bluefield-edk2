/** @file
*  Secondary System Description Table (SSDT)
*
*  Copyright (c) 2017, Mellanox Technologies. All rights reserved.
**/

#include "BlueFieldPlatform.h"
#include "rsh_def.h"
#include "bf1_irqs.h"

// ARS status code.
// These are local maintained state which is used to communicate with ATF.
#define ARS_STATUS_COMPLETE     0
#define ARS_STATUS_INPROGRESS   1
#define ARS_STATUS_NO_ARS       2
#define ARS_STATUS_PRE_STOP     3

// ARS functions.
#define ARS_FUNC_QUERY_CAP      1
#define ARS_FUNC_START_ARS      2
#define ARS_FUNC_QUERY_STATUS   3
#define ARS_FUNC_CLEAR_ERROR    4
#define ARS_FUNC_TRANS_SPA      5

// CKNV() return code
#define ARS_CKNV_ERROR          0xF0
#define ARS_CKNV_NO_REGION      0xF1
#define ARS_CKNV_NO_NVDIMM      0xF2

// ACPI status code
#define ACPI_STATUS_SUCCESS     0x00    // Success
#define ACPI_STATUS_ENOSUPPORT  0x01    // Not supported
#define ACPI_STATUS_EINVAL      0x02    // Invalid input

//
// Macro to set the CSR active page.
//
// To open a page for access, the host should do the following:
// 1. Write the desired page number to the OPEN_PAGE register;
// 2. Wait for the page switching latency as reported in the
//    PAGE_SWITCH_LATENCY0 and PAGE_SWITCH_LATENCY1 registers;
// 3. Read the OPEN_PAGE register at offset 0 to validate that page access
//    is updated to the desired page. If the OPEN_PAGE register value is not
//    equal to the desired page, the page access change failed.
// 4. Access the desired offset.
//
#define I2C_SET_PAGE(PAGE) \
  DATA = PAGE \
  Store(Store(BUFF, OPPG), BUFF) \
  If(LNotEqual(STAT, 0x00)) { \
    return (BUFF) \
  } \
  Sleep(DELY) \
  Store(OPPG, BUFF) \
  If(LNotEqual(STAT, 0x00)) { \
    return (BUFF) \
  } \
  If(LNotEqual(DATA, PAGE)) { \
    return (Buffer(4){3, 0, 0, 0}) \
  }

//
// Macro to read I2C REG into DST[OFF].
// - In case of failures, DST[0] = 'status'.
// - If PAGE is not 0, update the page select register first, and
//   restore it at the end.
//
#define I2C_READ(REG, DST, OFF, PAGE) \
  STAT = 0 \
  If(LNotEqual(PAGE, 0)) { \
    I2C_SET_PAGE(PAGE) \
  } \
  Store(REG, BUFF) \
  If(LEqual(STAT, 0x00)) { \
    Store(DATA, Index(DST, OFF)) \
  } Else { \
    Store(STAT, Index(DST, 0)) \
    return (BUFF) \
  } \
  If(LNotEqual(PAGE, 0)) { \
    I2C_SET_PAGE(0) \
  }

// Read I2C REG0(bit 7:0) and REG1(bit 18:8) into
// DST[OFF] after right-shift.
#define I2C_READ2(REG0, REG1, DST, OFF, PAGE, SHIFT) \
  STAT = 0 \
  If(LNotEqual(PAGE, 0)) { \
    I2C_SET_PAGE(PAGE) \
  } \
  Store(REG0, BUFF) \
  If(LEqual(STAT, 0x00)) { \
    Local2 = DATA \
  } Else { \
    Store(STAT, Index(DST, 0)) \
    return (BUFF) \
  } \
  Store(REG1, BUFF) \
  If(LEqual(STAT, 0x00)) { \
    Local2 = (Local2 + (DATA << 8)) >> SHIFT \
    Store(Local2, Index(DST, OFF)) \
  } Else { \
    Store(STAT, Index(DST, 0)) \
    return (BUFF) \
  } \
  If(LNotEqual(PAGE, 0)) { \
    I2C_SET_PAGE(0) \
  }

//
// NVDIMM Configuration and Status register map.
// Spec: Draft 8 of proposed JESD245B (item 2233.54A)
//
#define NVD_REG_MAP(I2cDev)                                     \
  Field(\_SB.I2C1.NCTL, BufferAcc, Lock, Preserve) {            \
    Connection(I2cDev),                                         \
    AccessAs(BufferAcc, AttribByte),                            \
                                                                \
    /* Page-0 registers */                                      \
    Offset(0x00),    /* OPEN_PAGE */                            \
    OPPG, 8,                                                    \
    Offset(0x01),    /* STD_NUM_PAGES */                        \
    SNPG, 8,                                                    \
    Offset(0x02),    /* VENDOR_START_PAGES */                   \
    VSPG, 8,                                                    \
    Offset(0x03),    /* VENDOR_NUM_PAGES */                     \
    VNPG, 8,                                                    \
    Offset(0x04),    /* HWREV */                                \
    HWRV, 8,                                                    \
    Offset(0x05),    /* LAST_CSAVE_DURATION0 (page-2) */        \
    LSD1, 8,                                                    \
    Offset(0x06),    /* SPECREV */                              \
    SREV, 8,                                                    \
    Offset(0x07),    /* SLOT0_FWREV0 */                         \
    S0R0, 8,                                                    \
    Offset(0x08),    /* SLOT0_FWREV1 */                         \
    S0R1, 8,                                                    \
    Offset(0x09),    /* SLOT1_FWREV0 */                         \
    S1R0, 8,                                                    \
    Offset(0x0A),    /* SLOT1_FWREV1 */                         \
    S1R1, 8,                                                    \
    Offset(0x0B),    /* SLOT0_SUBFWREV */                       \
    S0SR, 8,                                                    \
    Offset(0x0C),    /* SLOT1_SUBFWREV */                       \
    S1SR, 8,                                                    \
    Offset(0x0D),    /* RESTORE_OPS_COUNT0 (page-2) */          \
    ROC1, 8,                                                    \
    Offset(0x0E),    /* ERASE_SUCCESS_COUNT0 (page-2) */        \
    ESC0, 8,                                                    \
    Offset(0x0F),    /* ERASE_SUCCESS_COUNT1 (page-2) */        \
    ESC1, 8,                                                    \
    Offset(0x10),    /* CAPABILITIES0 */                        \
    CAP0, 8,                                                    \
    Offset(0x11),    /* CAPABILITIES1 */                        \
    CAP1, 8,                                                    \
    Offset(0x12),    /* MIN_ES_OPERATING_TEMP (page-1) */       \
    EIOT, 8,                                                    \
    Offset(0x13),    /* MAX_ES_OPERATING_TEMP (page-1) */       \
    EMOT, 8,                                                    \
    Offset(0x14),    /* ENERGY_SOURCE_POLICY */                 \
    ESPO, 8,                                                    \
    Offset(0x15),    /* HOST_MAX_OPERATION_RETRY */             \
    HMOR, 8,                                                    \
    Offset(0x16),    /* CSAVE_TRIGGER_SUPPORT */                \
    CTSP, 8,                                                    \
    Offset(0x17),    /* EVENT_NOTIFICATION_SUPPORT */           \
    ENSP, 8,                                                    \
    Offset(0x18),    /* CSAVE_TIMEOUT0 */                       \
    CST0, 8,                                                    \
    Offset(0x19),    /* CSAVE_TIMEOUT1 */                       \
    CST1, 8,                                                    \
    Offset(0x1A),    /* PAGE_SWITCH_LATENCY0  */                \
    PSL0, 8,                                                    \
    Offset(0x1B),    /* PAGE_SWITCH_LATENCY1  */                \
    PSL1, 8,                                                    \
    Offset(0x1C),    /* RESTORE_TIMEOUT0 */                     \
    RST0, 8,                                                    \
    Offset(0x1D),    /* RESTORE_TIMEOUT1 */                     \
    RST1, 8,                                                    \
    Offset(0x1E),    /* ERASE_TIMEOUT0 */                       \
    ETO0, 8,                                                    \
    Offset(0x1F),    /* ERASE_TIMEOUT1 */                       \
    ETO1, 8,                                                    \
    Offset(0x20),    /* ARM_TIMEOUT0 */                         \
    ATO0, 8,                                                    \
    Offset(0x21),    /* ARM_TIMEOUT1 */                         \
    ATO1, 8,                                                    \
    Offset(0x22),    /* FIRMWARE_OPS_TIMEOUT0 */                \
    FOT0, 8,                                                    \
    Offset(0x23),    /* FIRMWARE_OPS_TIMEOUT1 */                \
    FOT1, 8,                                                    \
    Offset(0x24),    /* ABORT_CMD_TIMEOUT0 */                   \
    ACTO, 8,                                                    \
    Offset(0x29),    /* CSAVE_POWER_REQ0 */                     \
    CPR0, 8,                                                    \
    Offset(0x2A),    /* CSAVE_POWER_REQ1 */                     \
    CPR1, 8,                                                    \
    Offset(0x2B),    /* CSAVE_IDLE_POWER_REQ0 */                \
    CIP0, 8,                                                    \
    Offset(0x2C),    /* CSAVE_IDLE_POWER_REQ1 */                \
    CIP1, 8,                                                    \
    Offset(0x2D),    /* CSAVE_MIN_VOLT_REQ0 */                  \
    CIV0, 8,                                                    \
    Offset(0x2E),    /* CSAVE_MIN_VOLT_REQ1 */                  \
    CIV1, 8,                                                    \
    Offset(0x2F),    /* CSAVE_MAX_VOLT_REQ0 */                  \
    CAV0, 8,                                                    \
    Offset(0x30),    /* CSAVE_MAX_VOLT_REQ1 */                  \
    CAV1, 8,                                                    \
    Offset(0x31),    /* VENDOR_LOG_PAGE_SIZE */                 \
    VLPS, 8,                                                    \
    Offset(0x32),    /* REGION_BLOCK_SIZE */                    \
    RBSZ, 8,                                                    \
    Offset(0x38),    /* MIN_OPERATING_TEMP0 */                  \
    MIO0, 8,                                                    \
    Offset(0x39),    /* MIN_OPERATING_TEMP1 */                  \
    MIO1, 8,                                                    \
    Offset(0x3A),    /* MAX_OPERATING_TEMP0 */                  \
    MAO0, 8,                                                    \
    Offset(0x3B),    /* MAX_OPERATING_TEMP1 */                  \
    MAO1, 8,                                                    \
    Offset(0x42),    /* RUNNING_FW_SLOT (page-3) */             \
    RFWS, 8,                                                    \
    Offset(0x70),    /* SET_ES_POLICY_STATUS */                 \
    ESPS, 8,                                                    \
    Offset(0x71),    /* ES_TEMP0 (page-1) */                    \
    ETP0, 8,                                                    \
    Offset(0x72),    /* ES_TEMP1 (page-1) */                    \
    ETP1, 8,                                                    \
    Offset(0x73),    /* ES_RUNTIME0 (page-1) */                 \
    ERT0, 8,                                                    \
    Offset(0x74),    /* ES_RUNTIME1 (page-1) */                 \
    ERT1, 8,                                                    \
    Offset(0x80),    /* CSAVE_INFO */                           \
    CSIF, 8,                                                    \
    Offset(0x81),    /* DRAM_THRESHOLD_ECC_COUNT (page-2) */    \
    DTEC, 8,                                                    \
    Offset(0x82),    /* HOST_MANAGED_ES_ATTRIBUTES (page-2) */  \
    HMEA, 8,                                                    \
    Offset(0x84),    /* CSAVE_FAIL_INFO0 */                     \
    CFI0, 8,                                                    \
    Offset(0x85),    /* CSAVE_FAIL_INFO1 */                     \
    CFI1, 8,                                                    \
    Offset(0x90),    /* NVM_LIFETIME_ERROR_THRESHOLD */         \
    NLET, 8,                                                    \
    Offset(0x91),    /* ES_LIFETIME_ERROR_THRESHOLD */          \
    ELET, 8,                                                    \
    Offset(0x92),    /* ES_TEMP_ERROR_THRESHOLD */              \
    ETET, 8,                                                    \
    Offset(0x98),    /* NVM_LIFETIME_WARNING_THRESHOLD */       \
    NLWT, 8,                                                    \
    Offset(0x99),    /* ES_LIFETIME_WARNING_THRESHOLD */        \
    ELWT, 8,                                                    \
    Offset(0x9C),    /* ES_TEMP_WARNING_THRESHOLD0 */           \
    ETW0, 8,                                                    \
    Offset(0x9D),    /* ES_TEMP_WARNING_THRESHOLD1 */           \
    ETW1, 8,                                                    \
    Offset(0xA0),    /* MODULE_HEALTH */                        \
    MODH, 8,                                                    \
    Offset(0xA1),    /* MODULE_HEALTH_STATUS0 */                \
    MHS0, 8,                                                    \
    Offset(0xA2),    /* MODULE_HEALTH_STATUS1 */                \
    MHS1, 8,                                                    \
    Offset(0xA5),    /* ERROR_THRESHOLD_STATUS */               \
    ETHS, 8,                                                    \
    Offset(0xA7),    /* WARNING_THRESHOLD_STATUS */             \
    WTHS, 8,                                                    \
    Offset(0xA9),    /* AUTO_ES_HEALTH_FREQUENCY */             \
    AEHF, 8,                                                    \
    Offset(0xC0),    /* NVM_LIFETIME */                         \
    NVLT, 8                                                     \
  }                                                             \
  /* Overlapped addresses on different pages. */                \
  Alias(ESPS, ESLT)  /* ES_LIFETIME (page-1) */                 \
  Alias(HWRV, EHWR)  /* ES_HWREV (page-1) */                    \
  Alias(SREV, EFR0)  /* ES_FWREV0 (page-1) */                   \
  Alias(S0R0, EFR1)  /* ES_FWREV1 (page-1) */                   \
  Alias(CAP0, ECT0)  /* ES_CHARGE_TIMEOUT0 (page-1) */          \
  Alias(CAP1, ECT1)  /* ES_CHARGE_TIMEOUT1 (page-1) */          \
  Alias(ESPO, ESAT)  /* ES_ATTRIBUTES (page-1) */               \
  Alias(HMOR, ESTE)  /* ES_TECH (page-1) */                     \
  Alias(CSIF, DEEC)  /* DRAM_ECC_ERROR_COUNT (page-2) */        \
  Alias(HWRV, LSD0)  /* LAST_CSAVE_DURATION0 (page-2) */        \
  Alias(SREV, LRD0)  /* LAST_RESTORE_DURATION0 (page-2) */      \
  Alias(S0R0, LRD1)  /* LAST_RESTORE_DURATION1 (page-2) */      \
  Alias(S0R1, LED0)  /* LAST_ERASE_DURATION0 (page-2) */        \
  Alias(S1R0, LED1)  /* LAST_ERASE_DURATION1 (page-2) */        \
  Alias(S1R1, SOC0)  /* SAVE_OPS_COUNT0 (page-2) */             \
  Alias(S0SR, SOC1)  /* SAVE_OPS_COUNT1 (page-2) */             \
  Alias(S1SR, ROC0)  /* RESTORE_OPS_COUNT0 (page-2) */          \
  Alias(CAP0, PCC0)  /* POWER_CYCLE_COUNT0 (page-2) */          \
  Alias(CAP1, PCC1)  /* POWER_CYCLE_COUNT1 (page-2) */          \
                                                                \
  /* Buffer to hold the I2C result. */                          \
  Name(BUFF, Buffer(8){ 0 })                                    \
  CreateByteField(BUFF, 0x00, STAT)                             \
  CreateByteField(BUFF, 0x02, DATA)                             \
  CreateByteField(BUFF, 0x07, DELY)                             \

//
// Macro of NVDIMM DSM
//
// Arguments: (4)
//   Arg0 - A Buffer containing a UUID
//   Arg1 - An Integer containing the Revision ID
//   Arg2 - An Integer containing the Function Index
//   Arg3 - A Package that contains function-specific arguments
//
// Return Value:
//   If Function Index = 0, a Buffer containing a function index
//   bitfield. Otherwise, the return value and type depends on the
//   UUID and revision ID (see below).
//
#define NVD_DSM()                                               \
  Method (_DSM, 4, Serialized) {                                \
    /* Only support MSFT DSM. */                                \
    /* DSM is implemented according to Microsoft definition: */ \
    /* https://msdn.microsoft.com/en-us/library/windows/hardware/mt604741(v=vs.85).aspx */ \
    If(LNotEqual(Arg0, ToUUID("1EE68B36-D4BD-4A1A-9A16-4F8E53D46E05"))) { \
      return (Buffer(One){0})                                   \
    }                                                           \
    /* Check function 0 first. */                               \
    If(LEqual(Arg2, Zero)) {                                    \
      return (FT00())                                           \
    }                                                           \
                                                                \
    /* Read page switching delay in milliseconds. */            \
    Store(PSL0, BUFF)                                           \
    If(LNotEqual(STAT, 0x00)) {                                 \
      return (Buffer(One){1})                                   \
    }                                                           \
    Local0 = DATA                                               \
    Store(PSL1, BUFF)                                           \
    If(LNotEqual(STAT, 0x00)) {                                 \
      return (Buffer(One){1})                                   \
    }                                                           \
    Local1 = DATA                                               \
    DELY = ((Local1 & 0x7F) << 8) + Local0                      \
    If(Local1 & 0x80) {                                         \
      DELY = DELY * 1000                                        \
    }                                                           \
                                                                \
    I2C_SET_PAGE(0)                                             \
    Switch(Arg2) {                                              \
      case (One) { /* Get NVDIMM-N Identification  */           \
        Name(BU01, Buffer(50){ 0 })                             \
        I2C_SET_PAGE(3)                                         \
        Store(RFWS, BUFF) /* page-3 */                          \
        I2C_SET_PAGE(0)                                         \
        Local0 = DATA                                           \
        Local1 = Local0 >> 4 /* RUNNING_FW_SLOT */              \
        Local0 = Local0 & 0xF /* SELECT_FW_SLOT */              \
        I2C_READ(SREV, BU01, 0x04, 0)                           \
        I2C_READ(SNPG, BU01, 0x05, 0)                           \
        I2C_READ(VSPG, BU01, 0x06, 0)                           \
        I2C_READ(VNPG, BU01, 0x07, 0)                           \
        I2C_READ(HWRV, BU01, 0x08, 0)                           \
        If(LEqual(Local1, Zero)) {                              \
          I2C_READ(S0R0, BU01, 0x0C, 0)                         \
          I2C_READ(S0R1, BU01, 0x0D, 0)                         \
        } Else {                                                \
          I2C_READ(S1R0, BU01, 0x0C, 0)                         \
          I2C_READ(S1R1, BU01, 0x0D, 0)                         \
        }                                                       \
        Store(Local1, Index(BU01, 0x0E))                        \
        Store(2, Index(BU01, 0x0F))                             \
        I2C_READ(CAP0, BU01, 0x10, 0)                           \
        I2C_READ(CTSP, BU01, 0x11, 0)                           \
        I2C_READ(HMOR, BU01, 0x12, 0)                           \
        I2C_READ(ENSP, BU01, 0x13, 0)                           \
        I2C_READ(CST0, BU01, 0x14, 0)                           \
        I2C_READ(CST1, BU01, 0x15, 0)                           \
        I2C_READ(RST0, BU01, 0x18, 0)                           \
        I2C_READ(RST1, BU01, 0x19, 0)                           \
        I2C_READ(ETO0, BU01, 0x1C, 0)                           \
        I2C_READ(ETO1, BU01, 0x1D, 0)                           \
        I2C_READ(ATO0, BU01, 0x20, 0)                           \
        I2C_READ(ATO1, BU01, 0x21, 0)                           \
        I2C_READ(FOT0, BU01, 0x24, 0)                           \
        I2C_READ(FOT1, BU01, 0x25, 0)                           \
        I2C_READ(ACTO, BU01, 0x28, 0)                           \
        I2C_READ2(MIO0, MIO1, BU01, 0x2C, 0, 4)                 \
        I2C_READ2(MAO0, MAO1, BU01, 0x2D, 0, 4)                 \
        I2C_READ(RBSZ, BU01, 0x2E, 0)                           \
        return (BU01)                                           \
      }                                                         \
      case (2) { /* Get Save Operation Requirements */          \
        Name(BU02, Buffer(12){ 0 })                             \
        I2C_READ(CPR0, BU02, 0x04, 0)                           \
        I2C_READ(CPR1, BU02, 0x05, 0)                           \
        I2C_READ(CIP0, BU02, 0x06, 0)                           \
        I2C_READ(CIP1, BU02, 0x07, 0)                           \
        I2C_READ(CIV0, BU02, 0x08, 0)                           \
        I2C_READ(CIV1, BU02, 0x09, 0)                           \
        I2C_READ(CAV0, BU02, 0x0A, 0)                           \
        I2C_READ(CAV1, BU02, 0x0B, 0)                           \
        return (BU02)                                           \
      }                                                         \
      case (3) { /* Get Energy Source Identification */         \
        Name(BU03, Buffer(19){ 0 })                             \
        I2C_READ(ESPO, BU03, 0x04, 0)                           \
        Store(ESPS, BUFF)                                       \
        If(LNotEqual(STAT, 0x00)) {                             \
          return (BUFF)                                         \
        }                                                       \
        If(LAnd(DATA, 0x04)) { /* device-managed */             \
          I2C_READ(EHWR, BU03, 0x05, 1)                         \
          I2C_READ(EFR0, BU03, 0x07, 1)                         \
          I2C_READ(EFR1, BU03, 0x08, 1)                         \
          I2C_READ(AEHF, BU03, 0x09, 0)                         \
          I2C_READ(ECT0, BU03, 0x0A, 1)                         \
          I2C_READ(ECT1, BU03, 0x0B, 1)                         \
          I2C_READ(EIOT, BU03, 0x0C, 1)                         \
          I2C_READ(EMOT, BU03, 0x0D, 1)                         \
          I2C_READ(ESAT, BU03, 0x0E, 1)                         \
          I2C_READ(ESTE, BU03, 0x0F, 1)                         \
        } ElseIf (LAnd(DATA, 0x08)) { /* host-managed */        \
          I2C_READ(AEHF, BU03, 0x10, 0)                         \
          I2C_READ(HMEA, BU03, 0x11, 2)                         \
          Store(0x02, Index(BU03, 0x12)) /* supercap */         \
        }                                                       \
        return (BU03)                                           \
      }                                                         \
      case (4) { /* Get Last Backup Information */              \
        Name(BU04, Buffer(12){ 0 })                             \
        I2C_READ(CSIF, BU04, 0x04, 0)                           \
        I2C_READ(CFI0, BU04, 0x08, 0)                           \
        I2C_READ(CFI1, BU04, 0x09, 0)                           \
        return (BU04)                                           \
      }                                                         \
      case (5) { /* Get NVM Thresholds */                       \
        Name(BU05, Buffer(6){ 0 })                              \
        I2C_READ(NLWT, BU05, 0x04, 0)                           \
        I2C_READ(NLET, BU05, 0x05, 0)                           \
        return (BU05)                                           \
      }                                                         \
      case (6) { /* Set NVM Lifetime Percentage Warning Threshold */ \
        STAT = 0                                                \
        DATA = Arg3                                             \
        Store(Store(BUFF, NLWT), BUFF)                          \
        return (BUFF)                                           \
      }                                                         \
      case (7) { /* Get Energy Source Thresholds */             \
        Name(BU07, Buffer(8){ 0 })                              \
        I2C_READ(ELWT, BU07, 0x04, 0)                           \
        I2C_READ(ELET, BU07, 0x05, 0)                           \
        I2C_READ2(ETW0, ETW1, BU07, 0x06, 0, 4)                 \
        I2C_READ(ETET, BU07, 0x07, 0)                           \
        return (BU07)                                           \
      }                                                         \
      case (8) { /* Set Energy Source Lifetime Warning Threshold */ \
        STAT = 0                                                \
        DATA = Arg3                                             \
        Store(Store(BUFF, ELWT), BUFF)                          \
        return (BUFF)                                           \
      }                                                         \
      case (9) { /* Set Energy Source Temperature Warning Threshold */ \
        STAT = 0                                                \
        /* Convert the format by left shifting 4 bit and */     \
        /* save it into two bytes. */                           \
        DATA = Arg3 << 4                                        \
        Store(Store(BUFF, ETW0), BUFF)                          \
        DATA = Arg3 >> 4                                        \
        If(LEqual(STAT, 0x00)) {                                \
          Store(Store(BUFF, ETW1), BUFF)                        \
          return (BUFF)                                         \
        }                                                       \
      }                                                         \
      case (10) { /* Get Critical Health Info */                \
        Name(BU10, Buffer(5){ 0 })                              \
        I2C_READ(MODH, BU10, 0x05, 0)                           \
        return (BU10)                                           \
      }                                                         \
      case (11) { /* Get NVDIMM-N Health Info */                \
        Name(BU11, Buffer(13){ 0 })                             \
        I2C_READ(MHS0, BU11, 0x04, 0)                           \
        I2C_READ(MHS1, BU11, 0x05, 0)                           \
        /* FIXME: 0x06 & 0x07 should be from SPD. */            \
        I2C_READ(ETHS, BU11, 0x08, 0)                           \
        I2C_READ(WTHS, BU11, 0x09, 0)                           \
        I2C_READ(NVLT, BU11, 0x0A, 0)                           \
        I2C_READ(DEEC, BU11, 0x0B, 2)                           \
        I2C_READ(DTEC, BU11, 0x0C, 2)                           \
        return (BU11)                                           \
      }                                                         \
      case (12) { /* Get Energy Source Health Info */           \
        Name(BU12, Buffer(11){ 0 })                             \
        I2C_READ(ESLT, BU12, 0x04, 1)                           \
        I2C_READ(ETP0, BU12, 0x05, 1)                           \
        I2C_READ(ETP1, BU12, 0x06, 1)                           \
        I2C_READ(ERT0, BU12, 0x07, 1)                           \
        I2C_READ(ERT1, BU12, 0x08, 1)                           \
        return (BU12)                                           \
      }                                                         \
      case (13) { /* Get Energy Source Health Info */           \
        Name(BU13, Buffer(32){ 0 })                             \
        I2C_READ(LSD0, BU13, 0x04, 2)                           \
        I2C_READ(LSD1, BU13, 0x05, 2)                           \
        I2C_READ(LRD0, BU13, 0x08, 2)                           \
        I2C_READ(LRD1, BU13, 0x09, 2)                           \
        I2C_READ(LED0, BU13, 0x0C, 2)                           \
        I2C_READ(LED1, BU13, 0x0D, 2)                           \
        I2C_READ(SOC0, BU13, 0x10, 2)                           \
        I2C_READ(SOC1, BU13, 0x11, 2)                           \
        I2C_READ(ROC0, BU13, 0x14, 2)                           \
        I2C_READ(ROC1, BU13, 0x15, 2)                           \
        I2C_READ(ESC0, BU13, 0x18, 2)                           \
        I2C_READ(ESC1, BU13, 0x19, 2)                           \
        I2C_READ(PCC0, BU13, 0x1C, 2)                           \
        I2C_READ(PCC1, BU13, 0x1D, 2)                           \
        return (BU13)                                           \
      }                                                         \
      case (16) { /* Query Error Injection Status */            \
        Name(BU16, Buffer(8){ 0 })                              \
        return (BU16)                                           \
      }                                                         \
      case (17) { /* Inject Error */                            \
        STAT = 1                                                \
        return (BUFF)                                           \
      }                                                         \
      case (26) { /* Get Firmware Info */                       \
        Name(BU26, Buffer(6){ 0 })                              \
        If(LEqual(Arg3, 0)) {                                   \
          I2C_READ(S0R0, BU26, 0x04, 0)                         \
          I2C_READ(S0R1, BU26, 0x05, 0)                         \
        } ElseIf(LEqual(Arg3, 1)) {                             \
          I2C_READ(S1R0, BU26, 0x04, 0)                         \
          I2C_READ(S1R1, BU26, 0x05, 0)                         \
        } Else {                                                \
          Store(One, Index(BU26, 0))                            \
        }                                                       \
        return (BU26)                                           \
      }                                                         \
      case (31) { /* Set Memory Error Counters */               \
        STAT = 0x00                                             \
        Index(Arg3, 0, DATA)                                    \
        I2C_SET_PAGE(2)                                         \
        Store(Store(BUFF, DEEC), BUFF)                          \
        If(LEqual(STAT, 0x00)) {                                \
          Index(Arg3, 1, DATA)                                  \
          Store(Store(BUFF, DTEC), BUFF)                        \
        }                                                       \
        I2C_SET_PAGE(0)                                         \
        return (BUFF)                                           \
      }                                                         \
      default {                                                 \
        /* Unsupported functions. */                            \
        return (Buffer(4){0x00, 0x00, 0x00, 0x00})              \
      }                                                         \
    }                                                           \
  }                                                             \

//
// AMLFileName, TableSignature, ComplianceRevision, OEMID, TableID, OEMRevision)
//
DefinitionBlock ("Ssdt.aml", "SSDT", 6, EFI_ACPI_MLNX_OEM_ID_STR, EFI_ACPI_MLNX_OEM_TABLE_ID_NVDIMM_STR, EFI_ACPI_MLNX_OEM_REVISION)
{
  External (\_SB.I2C1, DeviceObj)

  Scope (\_SB)
  {
    // I2C SMBus Host Controller
    Device(I2C1) {
      Name(_HID, "MLNXBF03")
      Name(_UID, 1)
      Name(_CCA, 1)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x02804800, 0x00000800)  // Smbus[1]
        Memory32Fixed(ReadWrite, 0x02801220, 0x00000020)  // CauseMaster[1]
        Memory32Fixed(ReadWrite, 0x02801280, 0x00000020)  // CauseSlave[1]
        Memory32Fixed(ReadWrite, 0x02801300, 0x00000010)  // CauseCoalesce
        Memory32Fixed(ReadWrite, 0x02802000, 0x00000100)  // GPIO 0
        Memory32Fixed(ReadWrite, 0x02800358, 0x00000008)  // CorePll
        Interrupt(ResourceConsumer, Edge, ActiveHigh, Shared)
          { BF1_RSH0_DEVICE_TYU_INT }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package () { "bus-freq", 100 },         // Smbus frequency in KHz
          Package () { "profile", "mlnx-bf18" },  // Device profile
        }
      })

      // NVDIMM controller operation region
      OperationRegion(NCTL, GenericSerialBus, 0, 0x100)

      // NVDIMM controller I2C definition
      Name (DIM0, ResourceTemplate() {
          I2CSerialBusV2(0x40,,100000,,"\\_SB.I2C1")
      })
      Name (DIM1, ResourceTemplate() {
          I2CSerialBusV2(0x41,,100000,,"\\_SB.I2C1")
      })
      Name (DIM2, ResourceTemplate() {
          I2CSerialBusV2(0x42,,100000,,"\\_SB.I2C1")
      })
      Name (DIM3, ResourceTemplate() {
          I2CSerialBusV2(0x43,,100000,,"\\_SB.I2C1")
      })
    }

    // NVDIMM Root
    Device (NVDR)
    {
      Name (_HID, "ACPI0012")   // _HID: Hardware ID


      //
      // Operation region and fields for EFI_INFO, which is a memory region
      // shared with ATF. Changes here need to be consistent with the structure
      // definition in ATF.
      //
      OperationRegion(INFO, SystemMemory, 0x88000000, 0x1000)
      // ByteAcc field definition
      Field(INFO, ByteAcc, NoLock, Preserve) {
        Offset(0x8),            // region number
        RNUM, 8,
        Offset(0x800),
        ARFN, 8,                // ARS function
        ARFL, 8,                // ARS flags
        Offset(0x840),
        ARQR, 816               // ARS query output
      }
      // ByteAcc field definition.
      Field(INFO, WordAcc, NoLock, Preserve) {
        Offset(0x842),
        ARST, 16                // ARS ext_status
      }
      // QWordAcc field definition.
      Field(INFO, QWordAcc, NoLock, Preserve) {
        // Header
        JMAG, 64,

        // Table of region & nvdimm.
        Offset(0x10),           // dimm[0].flags
        FLG0, 64,
        Offset(0x18),           // dimm[0].address
        ADR0, 64,
        Offset(0x20),           // dimm[0].size
        SIZ0, 64,
        Offset(0x28),           // dimm[1].flags
        FLG1, 64,
        Offset(0x30),           // dimm[1].address
        ADR1, 64,
        Offset(0x38),           // dimm[1].size
        SIZ1, 64,
        Offset(0x40),           // dimm[2].flags
        FLG2, 64,
        Offset(0x48),           // dimm[2].address
        ADR2, 64,
        Offset(0x50),           // dimm[2].size
        SIZ2, 64,
        Offset(0x58),           // dimm[3].flags
        FLG3, 64,
        Offset(0x60),           // dimm[3].address
        ADR3, 64,
        Offset(0x68),           // dimm[3].size
        SIZ3, 64,

        // ARS structure.
        Offset(0x800),
        FSFL, 64,               // func, status & flags
        STPA, 64,               // start address (PA)
        STLE, 64,               // start length
        Offset(0x858),
        RRPA, 64,               // restart address (PA)
        RRLE, 64                // restart length
      }

      // Operation region and fields of RShim registers.
      OperationRegion(RSHM, SystemMemory, 0x800000, 0x1000)
      Field(RSHM, QWordAcc, NoLock, Preserve) {
        Offset(0x0318),
        RINT, 64
      }

      //
      // Check whether the PA/LEN range overlaps a NVDIMM region.
      // Input:
      //   - Arg0    start PA address
      //   - Arg1    length
      // Output:
      //   DIMM_ID (1 ~ 4)    (found nvdimm region)
      //   ARS_CKNV_NO_REGION (no region found)
      //   ARS_CKNV_ERROR     (no NVDIMM region found)
      //
      Method (CKNV, 2, Serialized) {
            Local2 = RNUM    // Total number of regions
            Local3 = 0       // Flag to indicate a region is found
            Local4 = 0       // NVDIMM handle

            If(Local2 > 0) {
              If((Local3 == 0) && (Arg0 < (ADR0 + SIZ0)) && ((Arg0 + Arg1) >= ADR0)) {
                Local3 = 1
                If(FLG0 & 0x1000) {
                  Local4 = 1
                }
              }
              Local2 = Local2 - 1
            }
            If(Local2 > 0) {
              If((Local3 == 0) && (Arg0 < (ADR1 + SIZ1)) && ((Arg0 + Arg1) >= ADR1)) {
                Local3 = 1
                If(FLG1 & 0x1000) {
                  Local4 = 2
                }
              }
              Local2 = Local2 - 1
            }
            If(Local2 > 0) {
              If((Local3 == 0) && (Arg0 < (ADR2 + SIZ2)) && ((Arg0 + Arg1) >= ADR2)) {
                Local3 = 1
                If(FLG2 & 0x1000) {
                  Local4 = 3
                }
              }
              Local2 = Local2 - 1
            }
            If(Local2 > 0) {
              If((Local3 == 0) && (Arg0 < (ADR3 + SIZ3)) && ((Arg0 + Arg1) >= ADR3)) {
                Local3 = 1
                If(FLG3 & 0x1000) {
                  Local4 = 4
                }
              }
            }

            If(Local3 == 0) {
              return (ARS_CKNV_NO_REGION)
            } ElseIf(Local4 == 0) {
              return (ARS_CKNV_NO_NVDIMM)
            } Else {
              return (Local4)
            }
      }

      // Root Device DSM (based on ACPI spec 6.2)
      //   Arg0: UUID ( 2f10e7a4-9e91-11e4-89d3-123b93f75cba)
      //   Arg1: Revision ID (set to 1)
      //   Arg2: Function Index
      Method (_DSM, 4, Serialized) {
        // Verify UUID for root device DSM (defined in ACPI spec).
        If(LNotEqual(Arg0, ToUUID("2F10E7A4-9E91-11E4-89D3-123B93F75CBA"))) {
          return(Buffer(4){ACPI_STATUS_EINVAL, 0x00, 0x00, 0x00})
        }

        // Verify the revision id.
        If(LNotEqual(Arg1, One)) {
          return(Buffer(4){ACPI_STATUS_EINVAL, 0x00, 0x00, 0x00})
        }

        Switch(Arg2) {
          // Function 0 (supported functions)
          case(Zero) {
            return (Buffer(2){0x3F, 0x00})
          }

          //
          // Query Address Range Scrub (ARS) Capabilities (ACPI_6.2 Section 9.20.7.4)
          // Input Buffer:
          //   - ARS Start SPA Address (8B)
          //   - ARS Length (8B)
          case (ARS_FUNC_QUERY_CAP) {
            Local2 = DeRefOf(Index(Arg3, 0))
            CreateQWordField(Local2, 0x00, AAD1) // ARS Start SPA Address
            CreateQWordField(Local2, 0x08, ALE1) // ARS Length

            // Validate address range.
            Local3 = CKNV(AAD1, ALE1)

            If(Local3 == ARS_CKNV_NO_REGION) {
              // No region found, invalid Input Parameters
              return (Buffer(16){ACPI_STATUS_EINVAL, 0x00, 0x00, 0x00})
            } ElseIf(Local3 == ARS_CKNV_NO_NVDIMM) {
              return (Buffer(16){
                                 // Not NVDIMM, scrub not supported.
                                 ACPI_STATUS_ENOSUPPORT, 0x00, 0x00, 0x00
                                })
            } Else {
             return (Buffer(16){
                                // Persistent Memory (Byte 2, Bit 1)
                                ACPI_STATUS_SUCCESS, 0x00, 0x02, 0x00,
                                // Up to 32 ARS Error Records (6 + 32 * 3) * 8
                                0x30, 0x03, 0x00, 0x00,
                                // Clear Uncorrectable Error Range Length Unit Size
                                0x40, 0x00, 0x00, 0x00
                               })
            }
          }

          //
          // Start Address Range Scrub (ARS) (ACPI_6.2 Section Section 9.20.7.5)
          // Input Buffer:
          //   - ARS Start SPA Address (8B)
          //   - ARS Length (8B)
          //   - Type (2B)
          //   - Flags (1B)
          // Output Buffer:
          //   - Status (2B, 6 : ARS already in progress)
          //   - Extended Status (2B, reserved)
          //   - Estimated Time for Scrub (4B, seconds)
          //
          case (ARS_FUNC_START_ARS) {
            Local2 = DeRefOf(Index(Arg3, 0))
            CreateQWordField(Local2, 0x00, AAD2) // ARS Start SPA Address
            CreateQWordField(Local2, 0x08, ALE2) // ARS Length
            CreateWordField(Local2, 0x10, ATP2)  // Type
            CreateByteField(Local2, 0x12, AFL2)  // Flags

            // Check NVDIMM address range.
            // Only support Persistent Memory Scrub.
            Local3 = CKNV(AAD2, ALE2)
            If((Local3 >= ARS_CKNV_ERROR) || !(ATP2 & 0x02)) {
              // Invalid Input Parameters
              return (Buffer(8){ACPI_STATUS_EINVAL, 0x00, 0x00, 0x00})
            }

            // If set to 1 the firmware shall return data from a previous
            // scrub, if any, without starting a new scrub.
            If(AFL2 & 0x2) {
              // Return success to let the caller to poll last result.
              return (Buffer(8){ACPI_STATUS_SUCCESS, 0x00, 0x00, 0x00})
            }

            // Check if already started.
            If(ARST == ARS_STATUS_INPROGRESS) {
              // Calculate the estimated time in seconds.
              // In ATF, the down-counter is configured to be ~4us. Each
              // down-counter cycle handles one block of memory (4K bytes).
              Local3 = (RRLE >> 12) / 250000
              return (Buffer(8){0x06, 0x00, 0x00, 0x00,
                                (Local3 >> 24) & 0xFF, (Local3 >> 16) & 0xFF,
                                (Local3 >> 8) & 0xFF, Local3 & 0xFF})
            }

            // Fill in the new request.
            ARFN = 2                            // function
            ARST = ARS_STATUS_INPROGRESS        // start
            STPA = AAD2                         // physical address
            STLE = ALE2                         // length
            ARFL = AFL2                         // flags
            RRPA = 0                            // indicate new request
            RRLE = 0

            // Trigger the SWINT0 interrupt.
            RINT = 1

            // Estimate time is 1us per 4K bytes.
            Local3 = (STLE >> 12) / 1000000
            return (Buffer(8){ACPI_STATUS_SUCCESS, 0x00, 0x00, 0x00,
                              (Local3 >> 24) & 0xFF, (Local3 >> 16) & 0xFF,
                              (Local3 >> 8) & 0xFF, Local3 & 0xFF})
          }

          // Query Address Range Scrub (ARS) Status (ACPI_6.2 Section 9.20.7.6)
          case (ARS_FUNC_QUERY_STATUS) {
            If (ARST == ARS_STATUS_NO_ARS) {
              // No ARS performed for current boot
              return (Buffer(4){ACPI_STATUS_SUCCESS, 0x00, 0x02, 0x00})
            }
            If (ARST == ARS_STATUS_INPROGRESS) {
              // ARS in progress
              return (Buffer(4){ACPI_STATUS_SUCCESS, 0x00, 0x01, 0x00})
            }
            return (ARQR)
          }

          // Clear Uncorrectable Error (ACPI_6.2 Section 9.20.7.7)
          case (ARS_FUNC_CLEAR_ERROR) {
            Local2 = DeRefOf(Index(Arg3, 0))
            CreateQWordField(Local2, 0x00, AAD4) // SPA Address Base
            CreateQWordField(Local2, 0x08, ALE4) // Range Length

            // Invalid Input Parameters if not cache-aligned.
            If((AAD4 & 0x3F) != 0 || (ALE4 & 0x3F) != 0) {
              return (Buffer(16){0x02, 0x00, 0x00, 0x00})
            }

            // Don't continue if another operation is still in progres.
            If(ARST == ARS_STATUS_INPROGRESS) {
              return (Buffer(16){ACPI_STATUS_SUCCESS, 0x00, 0x00, 0x00})
            }

            // Check the input range.
            Local3 = CKNV(AAD4, ALE4)
            If (Local3 >= ARS_CKNV_ERROR) {
              return (Buffer(16){ACPI_STATUS_EINVAL, 0x00, 0x00, 0x00})
            }

            // Fill in the new request.
            ARFN = ARS_FUNC_CLEAR_ERROR         // function
            ARST = ARS_STATUS_INPROGRESS        // start
            STPA = AAD2                         // physical address
            STLE = ALE2                         // length

            // Trigger the SWINT0 interrupt.
            RINT = 1

            // This one needs to poll the completion.
            While (ARST == ARS_STATUS_INPROGRESS) {
              Sleep(1)
            }

            // Return the 16B result of the output buffer.
            CreateField (ARQR, 0, 128, ARCL)
            return (ARCL)
          }

          // Translate SPA (ACPI_6.2 Section 9.20.7.6)
          // Input Buffer:
          //   - SPA Address (8B)
          // Output Buffer:
          //   - Status (2B, 6 : ARS already in progress)
          //   - Extended Status (2B, reserved)
          //   - Flags (1B)
          //   - Reserved (3B)
          //   - Translated Length (8B)
          //   - Number of NVDIMMs (4B)
          //   - Translated NVDIMM List (varies)
          //     NFIT Device Handle (4B)
          //     Reserved (4B)
          //     DPA (8B)
          //
          case (ARS_FUNC_TRANS_SPA) {
            Local2 = DeRefOf(Index(Arg3, 0))
            CreateQWordField(Local2, 0x00, AAD5) // ARS Start SPA Address
            Local3 = CKNV(AAD5, 0)
            If (Local3 >= ARS_CKNV_ERROR) {
              return (Buffer(20){ACPI_STATUS_EINVAL, 0x00, 0x00, 0x00})
            }
            Local4 = AAD5
            // Calculate the translated length.
            If (Local3 == 1) {
              Local5 = ADR0 + SIZ0 - AAD5;
            } ElseIf (Local3 == 2) {
              Local5 = ADR1 + SIZ1 - AAD5;
            } ElseIf (Local3 == 3) {
              Local5 = ADR2 + SIZ2 - AAD5;
            } ElseIf (Local3 == 4) {
              Local5 = ADR3 + SIZ3 - AAD5;
            }
            return (Buffer(36){
              0x00, 0x00, 0x00, 0x00, // Status + Extended Status
              0x00, 0x00, 0x00, 0x00, // Flags + Reserved
              (Local5 & 0xFF), ((Local5 >> 8) & 0xFF), // Translated Length
              ((Local5 >> 16) & 0xFF), ((Local5 >> 24) & 0xFF),
              ((Local5 >> 32) & 0xFF), ((Local5 >> 40) & 0xFF),
              ((Local5 >> 48) & 0xFF), ((Local5 >> 56) & 0xFF),
              0x01, 0x00, 0x00, 0x00, // Number of NVDIMMs
              (Local3 & 0xFF), 0x00, 0x00, 0x00,       // Translated NVDIMM List
              0x00, 0x00, 0x00, 0x00,                  // Reserved
              (Local4 & 0xFF), ((Local4 >> 8) & 0xFF), // DPA
              ((Local4 >> 16) & 0xFF), ((Local4 >> 24) & 0xFF),
              ((Local4 >> 32) & 0xFF), ((Local4 >> 40) & 0xFF),
              ((Local4 >> 48) & 0xFF), ((Local4 >> 56) & 0xFF)
            })
          }

          // Not Supported.
          default {
            return (Buffer(4){ACPI_STATUS_ENOSUPPORT, 0x00, 0x00, 0x00})
          }
        }
      }

      // Function 0
      Method (FT00, 0x0, Serialized) {
        // Function 0 (list supported functions)
        // This bitmask needs to be updated if the set of functions
        // in NVD_DSM() macro is changed.
        return (Buffer(4){0xFF, 0x3F, 0x03, 0x84})
      }

      // NVDIMM1
      Device (NVD1)
      {
        Name (_ADR, 1)                  // Address
        NVD_REG_MAP(\_SB.I2C1.DIM0)     // Register definition
        NVD_DSM();                      // DSM methods
      }

      // NVDIMM2
      Device (NVD2)
      {
        Name (_ADR, 2)                  // Address
        NVD_REG_MAP(\_SB.I2C1.DIM1)     // Register definition
        NVD_DSM();                      // DSM methods
      }

      // NVDIMM3
      Device (NVD3)
      {
        Name (_ADR, 3)                  // Address
        NVD_REG_MAP(\_SB.I2C1.DIM2)     // Register definition
        NVD_DSM();                      // DSM methods
      }

      // NVDIMM4
      Device (NVD4)
      {
        Name (_ADR, 4)                  // Address
        NVD_REG_MAP(\_SB.I2C1.DIM3)     // Register definition
        NVD_DSM();                      // DSM methods
      }
    }
  }
}
