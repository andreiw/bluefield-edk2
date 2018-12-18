#ifndef __REGS_BF1_IRQS_H__
#define __REGS_BF1_IRQS_H__

/* BF1 Interrupts */

#define BF1_RSH0_SWINT0_INT 32

#define BF1_RSH0_SWINT1_INT 33

#define BF1_RSH0_SWINT2_INT 34

#define BF1_RSH0_SWINT3_INT 35

/* Access attempted from disabled external device */
#define BF1_RSH0_DEV_PROT_INT 36

/* Access attempted to non-existent or secured device */
#define BF1_RSH0_MMIO_ERR_INT 37

#define BF1_RSH0_DCNT0_INT 38

#define BF1_RSH0_DCNT1_INT 39

#define BF1_RSH0_DCNT2_INT 40

#define BF1_RSH0_TM_HTT_LWM_INT 41

#define BF1_RSH0_TM_HTT_HWM_INT 42

#define BF1_RSH0_TM_TTH_LWM_INT 43

#define BF1_RSH0_TM_TTH_HWM_INT 44

#define BF1_RSH0_TM_HTT_RERR_INT 45

#define BF1_RSH0_TM_HTT_WERR_INT 46

#define BF1_RSH0_TM_TTH_RERR_INT 47

#define BF1_RSH0_TM_TTH_WERR_INT 48

#define BF1_RSH0_CFG_PROT_VIOL_INT 49

#define BF1_RSH0_PWR_ALARM_INT 50

#define BF1_RSH0_PWR_HIGH_INT 51

#define BF1_RSH0_PWR_WDOG_INT 52

#define BF1_RSH0_VDROOP_TRACE_INT 53

#define BF1_RSH0_DEVICE_UART0_INT 54

#define BF1_RSH0_DEVICE_UART1_INT 55

#define BF1_RSH0_DEVICE_DIAG_UART_INT 56

#define BF1_RSH0_DEVICE_TYU_INT 57

#define BF1_RSH0_DEVICE_TYU_EXT1_UNUSED_INT 58

#define BF1_RSH0_DEVICE_TYU_EXT2_UNUSED_INT 59

#define BF1_RSH0_DEVICE_TYU_EXT3_UNUSED_INT 60

#define BF1_RSH0_DEVICE_TIMER_INT 61

#define BF1_RSH0_DEVICE_USB_INT 62

#define BF1_RSH0_DEVICE_GPIO_INT 63

#define BF1_RSH0_DEVICE_MMC_INT 64

#define BF1_RSH0_DEVICE_TIMER_EXT_INT 65

#define BF1_RSH0_DEVICE_WDOG_NS_INT 66

#define BF1_RSH0_DEVICE_WDOG_SEC_INT 67

/* MAC Interrupt. */
#define BF1_TH_TP0_MAC_INT 69

/*
 * A request targeted the rshim but the buffer was full so the request
 * was directed to the panic address.
 */
#define BF1_TH_TP0_RSH_FULL_ERROR_INT 70

/* A message arrived for the message queue, but it was full. */
#define BF1_TH_TP0_MSG_Q_FULL_ERROR_INT 71

/* A message has arrived in the message queue. */
#define BF1_TH_TP0_MSG_Q_ARRIVED_INT 72

/*
 * An MMIO request encountered an error.  Error info captured in
 * MMIO_ERROR_INFO.
 */
#define BF1_TH_TP0_MMIO_ERR_INT 73

/*
 * An ingress packet was not claimed by any map region.  The info for
 * the request is captured in MAP_ERR_STS.
 */
#define BF1_TH_TP0_MAP_UNCLAIMED_INT 74

/* A request targeting the rshim violated the maximum read request size. */
#define BF1_TH_TP0_RSH_SIZE_ERROR_INT 75

/* A PIO ECAM transaction was malformed (crossed DWD boundary). */
#define BF1_TH_TP0_PIO_ECAM_ERR_INT 76

/*
 * A PIO transaction received an error completion from the MAC.  The
 * associated region is stored in TILE_PIO_CPL_ERR_STS.
 */
#define BF1_TH_TP0_PIO_CPL_ERR_INT 77

/* An MMIO request violated the config register protection level. */
#define BF1_TH_TP0_MMIO_PROT_ERR_INT 78

/* The PUSH_DMA_EVT_CTR wrapped. */
#define BF1_TH_TP0_PUSH_DMA_EVT_CTR_INT 79

/* The MAP_EVT_CTR wrapped. */
#define BF1_TH_TP0_MAP_EVT_CTR_INT 80

/*
 * Access to disabled PIO region.  Error info captured in
 * TILE_PIO_TIMEOUT_STS.
 */
#define BF1_TH_TP0_PIO_DISABLED_INT 81

/*
 * Remote buffer return encountered MMIO error (not applicable to all
 * devices).
 */
#define BF1_TH_TP0_REM_MMIO_ERR_INT 82

/* Correctable error message received. */
#define BF1_TH_TP0_ERR_MSG_COR_INT 83

/* Non-fatal error message received. */
#define BF1_TH_TP0_ERR_MSG_NONFATAL_INT 84

/* Fatal error message received. */
#define BF1_TH_TP0_ERR_MSG_FATAL_INT 85

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define BF1_TH_TP0_LEVEL_INT0_INT 86

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define BF1_TH_TP0_LEVEL_INT1_INT 87

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define BF1_TH_TP0_LEVEL_INT2_INT 88

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define BF1_TH_TP0_LEVEL_INT3_INT 89

/* MAC Interrupt. */
#define BF1_TH_TP1_MAC_INT 91

/*
 * A request targeted the rshim but the buffer was full so the request
 * was directed to the panic address.
 */
#define BF1_TH_TP1_RSH_FULL_ERROR_INT 92

/* A message arrived for the message queue, but it was full. */
#define BF1_TH_TP1_MSG_Q_FULL_ERROR_INT 93

/* A message has arrived in the message queue. */
#define BF1_TH_TP1_MSG_Q_ARRIVED_INT 94

/*
 * An MMIO request encountered an error.  Error info captured in
 * MMIO_ERROR_INFO.
 */
#define BF1_TH_TP1_MMIO_ERR_INT 95

/*
 * An ingress packet was not claimed by any map region.  The info for
 * the request is captured in MAP_ERR_STS.
 */
#define BF1_TH_TP1_MAP_UNCLAIMED_INT 96

/* A request targeting the rshim violated the maximum read request size. */
#define BF1_TH_TP1_RSH_SIZE_ERROR_INT 97

/* A PIO ECAM transaction was malformed (crossed DWD boundary). */
#define BF1_TH_TP1_PIO_ECAM_ERR_INT 98

/*
 * A PIO transaction received an error completion from the MAC.  The
 * associated region is stored in TILE_PIO_CPL_ERR_STS.
 */
#define BF1_TH_TP1_PIO_CPL_ERR_INT 99

/* An MMIO request violated the config register protection level. */
#define BF1_TH_TP1_MMIO_PROT_ERR_INT 100

/* The PUSH_DMA_EVT_CTR wrapped. */
#define BF1_TH_TP1_PUSH_DMA_EVT_CTR_INT 101

/* The MAP_EVT_CTR wrapped. */
#define BF1_TH_TP1_MAP_EVT_CTR_INT 102

/*
 * Access to disabled PIO region.  Error info captured in
 * TILE_PIO_TIMEOUT_STS.
 */
#define BF1_TH_TP1_PIO_DISABLED_INT 103

/*
 * Remote buffer return encountered MMIO error (not applicable to all
 * devices).
 */
#define BF1_TH_TP1_REM_MMIO_ERR_INT 104

/* Correctable error message received. */
#define BF1_TH_TP1_ERR_MSG_COR_INT 105

/* Non-fatal error message received. */
#define BF1_TH_TP1_ERR_MSG_NONFATAL_INT 106

/* Fatal error message received. */
#define BF1_TH_TP1_ERR_MSG_FATAL_INT 107

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define BF1_TH_TP1_LEVEL_INT0_INT 108

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define BF1_TH_TP1_LEVEL_INT1_INT 109

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define BF1_TH_TP1_LEVEL_INT2_INT 110

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define BF1_TH_TP1_LEVEL_INT3_INT 111

/* MAC Interrupt. */
#define BF1_TH_HCA0_MAC_INT 113

/*
 * A request targeted the rshim but the buffer was full so the request
 * was directed to the panic address.
 */
#define BF1_TH_HCA0_RSH_FULL_ERROR_INT 114

/* A message arrived for the message queue, but it was full. */
#define BF1_TH_HCA0_MSG_Q_FULL_ERROR_INT 115

/* A message has arrived in the message queue. */
#define BF1_TH_HCA0_MSG_Q_ARRIVED_INT 116

/*
 * An MMIO request encountered an error.  Error info captured in
 * MMIO_ERROR_INFO.
 */
#define BF1_TH_HCA0_MMIO_ERR_INT 117

/*
 * An ingress packet was not claimed by any map region.  The info for
 * the request is captured in MAP_ERR_STS.
 */
#define BF1_TH_HCA0_MAP_UNCLAIMED_INT 118

/* A request targeting the rshim violated the maximum read request size. */
#define BF1_TH_HCA0_RSH_SIZE_ERROR_INT 119

/* A PIO ECAM transaction was malformed (crossed DWD boundary). */
#define BF1_TH_HCA0_PIO_ECAM_ERR_INT 120

/*
 * A PIO transaction received an error completion from the MAC.  The
 * associated region is stored in TILE_PIO_CPL_ERR_STS.
 */
#define BF1_TH_HCA0_PIO_CPL_ERR_INT 121

/* An MMIO request violated the config register protection level. */
#define BF1_TH_HCA0_MMIO_PROT_ERR_INT 122

/* The PUSH_DMA_EVT_CTR wrapped. */
#define BF1_TH_HCA0_PUSH_DMA_EVT_CTR_INT 123

/* The MAP_EVT_CTR wrapped. */
#define BF1_TH_HCA0_MAP_EVT_CTR_INT 124

/*
 * Access to disabled PIO region.  Error info captured in
 * TILE_PIO_TIMEOUT_STS.
 */
#define BF1_TH_HCA0_PIO_DISABLED_INT 125

/*
 * Remote buffer return encountered MMIO error (not applicable to all
 * devices).
 */
#define BF1_TH_HCA0_REM_MMIO_ERR_INT 126

/* Correctable error message received. */
#define BF1_TH_HCA0_ERR_MSG_COR_INT 127

/* Non-fatal error message received. */
#define BF1_TH_HCA0_ERR_MSG_NONFATAL_INT 128

/* Fatal error message received. */
#define BF1_TH_HCA0_ERR_MSG_FATAL_INT 129

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define BF1_TH_HCA0_LEVEL_INT0_INT 130

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define BF1_TH_HCA0_LEVEL_INT1_INT 131

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define BF1_TH_HCA0_LEVEL_INT2_INT 132

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define BF1_TH_HCA0_LEVEL_INT3_INT 133

/* L3C functional interrupt, check INTRPT_CAUSE for further information. */
#define BF1_TH_MSS0_L3C0_FUNCTIONAL_INT 135

/*
 * L3C RAMS ECC interrupt, check ECC_DOUBLE_ERROR_0/ECC_SINGLE_ERROR_0
 * for further information.
 */
#define BF1_TH_MSS0_L3C0_RAMS_ERR_INT 136

/* L3C functional interrupt, check INTRPT_CAUSE for further information. */
#define BF1_TH_MSS0_L3C1_FUNCTIONAL_INT 137

/*
 * L3C RAMS ECC interrupt, check ECC_DOUBLE_ERROR_0/ECC_SINGLE_ERROR_0
 * for further information.
 */
#define BF1_TH_MSS0_L3C1_RAMS_ERR_INT 138

/* EMEM_MI ECC SERR interrupt. */
#define BF1_TH_MSS0_EMEM_MI_SERR_INT 139

/* EMEM_MI ECC DERR interrupt. */
#define BF1_TH_MSS0_EMEM_MI_DERR_INT 140

/* EMEM_MI functional interrupt. */
#define BF1_TH_MSS0_EMEM_MI_FUNCTIONAL_INT 141

#define BF1_TH_MSS0_TBD0_INT 142

/* L3C functional interrupt, check INTRPT_CAUSE for further information. */
#define BF1_TH_MSS1_L3C0_FUNCTIONAL_INT 144

/*
 * L3C RAMS ECC interrupt, check ECC_DOUBLE_ERROR_0/ECC_SINGLE_ERROR_0
 * for further information.
 */
#define BF1_TH_MSS1_L3C0_RAMS_ERR_INT 145

/* L3C functional interrupt, check INTRPT_CAUSE for further information. */
#define BF1_TH_MSS1_L3C1_FUNCTIONAL_INT 146

/*
 * L3C RAMS ECC interrupt, check ECC_DOUBLE_ERROR_0/ECC_SINGLE_ERROR_0
 * for further information.
 */
#define BF1_TH_MSS1_L3C1_RAMS_ERR_INT 147

/* EMEM_MI ECC SERR interrupt. */
#define BF1_TH_MSS1_EMEM_MI_SERR_INT 148

/* EMEM_MI ECC DERR interrupt. */
#define BF1_TH_MSS1_EMEM_MI_DERR_INT 149

/* EMEM_MI functional interrupt. */
#define BF1_TH_MSS1_EMEM_MI_FUNCTIONAL_INT 150

#define BF1_TH_MSS1_TBD0_INT 151

/* Command count for queue 0 on PKA 0 is below threshold value. */
#define BF1_CRYPTO0_CMD_CNT_0_0_INT 153

/* Command count for queue 1 on PKA 0 is below threshold value. */
#define BF1_CRYPTO0_CMD_CNT_0_1_INT 154

/* Command count for queue 2 on PKA 0 is below threshold value. */
#define BF1_CRYPTO0_CMD_CNT_0_2_INT 155

/* Command count for queue 3 on PKA 0 is below threshold value. */
#define BF1_CRYPTO0_CMD_CNT_0_3_INT 156

/* Result count for queue 0 on PKA 0 is above threshold value. */
#define BF1_CRYPTO0_RES_CNT_0_0_INT 157

/* Result count for queue 1 on PKA 0 is above threshold value. */
#define BF1_CRYPTO0_RES_CNT_0_1_INT 158

/* Result count for queue 2 on PKA 0 is above threshold value. */
#define BF1_CRYPTO0_RES_CNT_0_2_INT 159

/* Result count for queue 3 on PKA 0 is above threshold value. */
#define BF1_CRYPTO0_RES_CNT_0_3_INT 160

/* PKA Master Controller on PKA 0 requests attention */
#define BF1_CRYPTO0_PKA_MASTER_0_INT 161

/* TRNG (True Random Number Generator) on PKA 0 requests attention */
#define BF1_CRYPTO0_TRNG_0_INT 162

/* One or more PKA RAM(s) on PKA 0 had a parity error */
#define BF1_CRYPTO0_PARITY_ERROR_0_INT 163

/* Command count for queue 0 on PKA 1 is below threshold value. */
#define BF1_CRYPTO0_CMD_CNT_1_0_INT 164

/* Command count for queue 1 on PKA 1 is below threshold value. */
#define BF1_CRYPTO0_CMD_CNT_1_1_INT 165

/* Command count for queue 2 on PKA 1 is below threshold value. */
#define BF1_CRYPTO0_CMD_CNT_1_2_INT 166

/* Command count for queue 3 on PKA 1 is below threshold value. */
#define BF1_CRYPTO0_CMD_CNT_1_3_INT 167

/* Result count for queue 0 on PKA 1 is above threshold value. */
#define BF1_CRYPTO0_RES_CNT_1_0_INT 168

/* Result count for queue 1 on PKA 1 is above threshold value. */
#define BF1_CRYPTO0_RES_CNT_1_1_INT 169

/* Result count for queue 2 on PKA 1 is above threshold value. */
#define BF1_CRYPTO0_RES_CNT_1_2_INT 170

/* Result count for queue 3 on PKA 1 is above threshold value. */
#define BF1_CRYPTO0_RES_CNT_1_3_INT 171

/* PKA Master Controller on PKA 1 requests attention */
#define BF1_CRYPTO0_PKA_MASTER_1_INT 172

/* TRNG (True Random Number Generator) on PKA 1 requests attention */
#define BF1_CRYPTO0_TRNG_1_INT 173

/* One or more PKA RAM(s) on PKA 1 had a parity error */
#define BF1_CRYPTO0_PARITY_ERROR_1_INT 174

/* Command count for queue 0 on PKA 0 is below threshold value. */
#define BF1_CRYPTO1_CMD_CNT_0_0_INT 176

/* Command count for queue 1 on PKA 0 is below threshold value. */
#define BF1_CRYPTO1_CMD_CNT_0_1_INT 177

/* Command count for queue 2 on PKA 0 is below threshold value. */
#define BF1_CRYPTO1_CMD_CNT_0_2_INT 178

/* Command count for queue 3 on PKA 0 is below threshold value. */
#define BF1_CRYPTO1_CMD_CNT_0_3_INT 179

/* Result count for queue 0 on PKA 0 is above threshold value. */
#define BF1_CRYPTO1_RES_CNT_0_0_INT 180

/* Result count for queue 1 on PKA 0 is above threshold value. */
#define BF1_CRYPTO1_RES_CNT_0_1_INT 181

/* Result count for queue 2 on PKA 0 is above threshold value. */
#define BF1_CRYPTO1_RES_CNT_0_2_INT 182

/* Result count for queue 3 on PKA 0 is above threshold value. */
#define BF1_CRYPTO1_RES_CNT_0_3_INT 183

/* PKA Master Controller on PKA 0 requests attention */
#define BF1_CRYPTO1_PKA_MASTER_0_INT 184

/* TRNG (True Random Number Generator) on PKA 0 requests attention */
#define BF1_CRYPTO1_TRNG_0_INT 185

/* One or more PKA RAM(s) on PKA 0 had a parity error */
#define BF1_CRYPTO1_PARITY_ERROR_0_INT 186

/* Command count for queue 0 on PKA 1 is below threshold value. */
#define BF1_CRYPTO1_CMD_CNT_1_0_INT 187

/* Command count for queue 1 on PKA 1 is below threshold value. */
#define BF1_CRYPTO1_CMD_CNT_1_1_INT 188

/* Command count for queue 2 on PKA 1 is below threshold value. */
#define BF1_CRYPTO1_CMD_CNT_1_2_INT 189

/* Command count for queue 3 on PKA 1 is below threshold value. */
#define BF1_CRYPTO1_CMD_CNT_1_3_INT 190

/* Result count for queue 0 on PKA 1 is above threshold value. */
#define BF1_CRYPTO1_RES_CNT_1_0_INT 191

/* Result count for queue 1 on PKA 1 is above threshold value. */
#define BF1_CRYPTO1_RES_CNT_1_1_INT 192

/* Result count for queue 2 on PKA 1 is above threshold value. */
#define BF1_CRYPTO1_RES_CNT_1_2_INT 193

/* Result count for queue 3 on PKA 1 is above threshold value. */
#define BF1_CRYPTO1_RES_CNT_1_3_INT 194

/* PKA Master Controller on PKA 1 requests attention */
#define BF1_CRYPTO1_PKA_MASTER_1_INT 195

/* TRNG (True Random Number Generator) on PKA 1 requests attention */
#define BF1_CRYPTO1_TRNG_1_INT 196

/* One or more PKA RAM(s) on PKA 1 had a parity error */
#define BF1_CRYPTO1_PARITY_ERROR_1_INT 197

/* Secure combined interrupt */
#define BF1_SMMU0_COMB_IRPT_S_INT 199

/* Non-secure combined interrupt */
#define BF1_SMMU0_COMB_IRPT_NS_INT 200

/* Non-secure context interrupts for context 0 */
#define BF1_SMMU0_CXT_IRPT_0_INT 201

/* Non-secure context interrupts for context 1 */
#define BF1_SMMU0_CXT_IRPT_1_INT 202

/* Non-secure context interrupts for context 2 */
#define BF1_SMMU0_CXT_IRPT_2_INT 203

/* Non-secure context interrupts for context 3 */
#define BF1_SMMU0_CXT_IRPT_3_INT 204

/* Non-secure context interrupts for context 4 */
#define BF1_SMMU0_CXT_IRPT_4_INT 205

/* Non-secure context interrupts for context 5 */
#define BF1_SMMU0_CXT_IRPT_5_INT 206

/* Non-secure context interrupts for context 6 */
#define BF1_SMMU0_CXT_IRPT_6_INT 207

/* Non-secure context interrupts for context 7 */
#define BF1_SMMU0_CXT_IRPT_7_INT 208

/* Non-secure context interrupts for context 8 */
#define BF1_SMMU0_CXT_IRPT_8_INT 209

/* Non-secure context interrupts for context 9 */
#define BF1_SMMU0_CXT_IRPT_9_INT 210

/* Non-secure context interrupts for context 10 */
#define BF1_SMMU0_CXT_IRPT_10_INT 211

/* Non-secure context interrupts for context 11 */
#define BF1_SMMU0_CXT_IRPT_11_INT 212

/* Non-secure context interrupts for context 12 */
#define BF1_SMMU0_CXT_IRPT_12_INT 213

/* Non-secure context interrupts for context 13 */
#define BF1_SMMU0_CXT_IRPT_13_INT 214

/* Non-secure context interrupts for context 14 */
#define BF1_SMMU0_CXT_IRPT_14_INT 215

/* Non-secure context interrupts for context 15 */
#define BF1_SMMU0_CXT_IRPT_15_INT 216

/* Non-secure context interrupts for context 16 */
#define BF1_SMMU0_CXT_IRPT_16_INT 217

/* Non-secure context interrupts for context 17 */
#define BF1_SMMU0_CXT_IRPT_17_INT 218

/* Non-secure context interrupts for context 18 */
#define BF1_SMMU0_CXT_IRPT_18_INT 219

/* Non-secure context interrupts for context 19 */
#define BF1_SMMU0_CXT_IRPT_19_INT 220

/* Non-secure context interrupts for context 20 */
#define BF1_SMMU0_CXT_IRPT_20_INT 221

/* Non-secure context interrupts for context 21 */
#define BF1_SMMU0_CXT_IRPT_21_INT 222

/* Non-secure context interrupts for context 22 */
#define BF1_SMMU0_CXT_IRPT_22_INT 223

/* Non-secure context interrupts for context 23 */
#define BF1_SMMU0_CXT_IRPT_23_INT 224

/* Non-secure context interrupts for context 24 */
#define BF1_SMMU0_CXT_IRPT_24_INT 225

/* Non-secure context interrupts for context 25 */
#define BF1_SMMU0_CXT_IRPT_25_INT 226

/* Non-secure context interrupts for context 26 */
#define BF1_SMMU0_CXT_IRPT_26_INT 227

/* Non-secure context interrupts for context 27 */
#define BF1_SMMU0_CXT_IRPT_27_INT 228

/* Non-secure context interrupts for context 28 */
#define BF1_SMMU0_CXT_IRPT_28_INT 229

/* Non-secure context interrupts for context 29 */
#define BF1_SMMU0_CXT_IRPT_29_INT 230

/* Non-secure context interrupts for context 30 */
#define BF1_SMMU0_CXT_IRPT_30_INT 231

/* Non-secure context interrupts for context 31 */
#define BF1_SMMU0_CXT_IRPT_31_INT 232

/* Non-secure context interrupts for context 32 */
#define BF1_SMMU0_CXT_IRPT_32_INT 233

/* Non-secure context interrupts for context 33 */
#define BF1_SMMU0_CXT_IRPT_33_INT 234

/* Non-secure context interrupts for context 34 */
#define BF1_SMMU0_CXT_IRPT_34_INT 235

/* Non-secure context interrupts for context 35 */
#define BF1_SMMU0_CXT_IRPT_35_INT 236

/* Non-secure context interrupts for context 36 */
#define BF1_SMMU0_CXT_IRPT_36_INT 237

/* Non-secure context interrupts for context 37 */
#define BF1_SMMU0_CXT_IRPT_37_INT 238

/* Non-secure context interrupts for context 38 */
#define BF1_SMMU0_CXT_IRPT_38_INT 239

/* Non-secure context interrupts for context 39 */
#define BF1_SMMU0_CXT_IRPT_39_INT 240

/* Non-secure context interrupts for context 40 */
#define BF1_SMMU0_CXT_IRPT_40_INT 241

/* Non-secure context interrupts for context 41 */
#define BF1_SMMU0_CXT_IRPT_41_INT 242

/* Non-secure context interrupts for context 42 */
#define BF1_SMMU0_CXT_IRPT_42_INT 243

/* Non-secure context interrupts for context 43 */
#define BF1_SMMU0_CXT_IRPT_43_INT 244

/* Non-secure context interrupts for context 44 */
#define BF1_SMMU0_CXT_IRPT_44_INT 245

/* Non-secure context interrupts for context 45 */
#define BF1_SMMU0_CXT_IRPT_45_INT 246

/* Non-secure context interrupts for context 46 */
#define BF1_SMMU0_CXT_IRPT_46_INT 247

/* Non-secure context interrupts for context 47 */
#define BF1_SMMU0_CXT_IRPT_47_INT 248

/* Non-secure context interrupts for context 48 */
#define BF1_SMMU0_CXT_IRPT_48_INT 249

/* Non-secure context interrupts for context 49 */
#define BF1_SMMU0_CXT_IRPT_49_INT 250

/* Non-secure context interrupts for context 50 */
#define BF1_SMMU0_CXT_IRPT_50_INT 251

/* Non-secure context interrupts for context 51 */
#define BF1_SMMU0_CXT_IRPT_51_INT 252

/* Non-secure context interrupts for context 52 */
#define BF1_SMMU0_CXT_IRPT_52_INT 253

/* Non-secure context interrupts for context 53 */
#define BF1_SMMU0_CXT_IRPT_53_INT 254

/* Non-secure context interrupts for context 54 */
#define BF1_SMMU0_CXT_IRPT_54_INT 255

/* Non-secure context interrupts for context 55 */
#define BF1_SMMU0_CXT_IRPT_55_INT 256

/* Non-secure context interrupts for context 56 */
#define BF1_SMMU0_CXT_IRPT_56_INT 257

/* Non-secure context interrupts for context 57 */
#define BF1_SMMU0_CXT_IRPT_57_INT 258

/* Non-secure context interrupts for context 58 */
#define BF1_SMMU0_CXT_IRPT_58_INT 259

/* Non-secure context interrupts for context 59 */
#define BF1_SMMU0_CXT_IRPT_59_INT 260

/* Non-secure context interrupts for context 60 */
#define BF1_SMMU0_CXT_IRPT_60_INT 261

/* Non-secure context interrupts for context 61 */
#define BF1_SMMU0_CXT_IRPT_61_INT 262

/* Non-secure context interrupts for context 62 */
#define BF1_SMMU0_CXT_IRPT_62_INT 263

/* Non-secure context interrupts for context 63 */
#define BF1_SMMU0_CXT_IRPT_63_INT 264

/* Non-secure context interrupts for context 64 */
#define BF1_SMMU0_CXT_IRPT_64_INT 265

/* Non-secure context interrupts for context 65 */
#define BF1_SMMU0_CXT_IRPT_65_INT 266

/* Non-secure context interrupts for context 66 */
#define BF1_SMMU0_CXT_IRPT_66_INT 267

/* Non-secure context interrupts for context 67 */
#define BF1_SMMU0_CXT_IRPT_67_INT 268

/* Non-secure context interrupts for context 68 */
#define BF1_SMMU0_CXT_IRPT_68_INT 269

/* Non-secure context interrupts for context 69 */
#define BF1_SMMU0_CXT_IRPT_69_INT 270

/* Non-secure context interrupts for context 70 */
#define BF1_SMMU0_CXT_IRPT_70_INT 271

/* Non-secure context interrupts for context 71 */
#define BF1_SMMU0_CXT_IRPT_71_INT 272

/* Non-secure context interrupts for context 72 */
#define BF1_SMMU0_CXT_IRPT_72_INT 273

/* Non-secure context interrupts for context 73 */
#define BF1_SMMU0_CXT_IRPT_73_INT 274

/* Non-secure context interrupts for context 74 */
#define BF1_SMMU0_CXT_IRPT_74_INT 275

/* Non-secure context interrupts for context 75 */
#define BF1_SMMU0_CXT_IRPT_75_INT 276

/* Non-secure context interrupts for context 76 */
#define BF1_SMMU0_CXT_IRPT_76_INT 277

/* Non-secure context interrupts for context 77 */
#define BF1_SMMU0_CXT_IRPT_77_INT 278

/* Non-secure context interrupts for context 78 */
#define BF1_SMMU0_CXT_IRPT_78_INT 279

/* Non-secure context interrupts for context 79 */
#define BF1_SMMU0_CXT_IRPT_79_INT 280

/* Non-secure context interrupts for context 80 */
#define BF1_SMMU0_CXT_IRPT_80_INT 281

/* Non-secure context interrupts for context 81 */
#define BF1_SMMU0_CXT_IRPT_81_INT 282

/* Non-secure context interrupts for context 82 */
#define BF1_SMMU0_CXT_IRPT_82_INT 283

/* Non-secure context interrupts for context 83 */
#define BF1_SMMU0_CXT_IRPT_83_INT 284

/* Non-secure context interrupts for context 84 */
#define BF1_SMMU0_CXT_IRPT_84_INT 285

/* Non-secure context interrupts for context 85 */
#define BF1_SMMU0_CXT_IRPT_85_INT 286

/* Non-secure context interrupts for context 86 */
#define BF1_SMMU0_CXT_IRPT_86_INT 287

/* Non-secure context interrupts for context 87 */
#define BF1_SMMU0_CXT_IRPT_87_INT 288

/* Non-secure context interrupts for context 88 */
#define BF1_SMMU0_CXT_IRPT_88_INT 289

/* Non-secure context interrupts for context 89 */
#define BF1_SMMU0_CXT_IRPT_89_INT 290

/* Non-secure context interrupts for context 90 */
#define BF1_SMMU0_CXT_IRPT_90_INT 291

/* Non-secure context interrupts for context 91 */
#define BF1_SMMU0_CXT_IRPT_91_INT 292

/* Non-secure context interrupts for context 92 */
#define BF1_SMMU0_CXT_IRPT_92_INT 293

/* Non-secure context interrupts for context 93 */
#define BF1_SMMU0_CXT_IRPT_93_INT 294

/* Non-secure context interrupts for context 94 */
#define BF1_SMMU0_CXT_IRPT_94_INT 295

/* Non-secure context interrupts for context 95 */
#define BF1_SMMU0_CXT_IRPT_95_INT 296

/* Non-secure context interrupts for context 96 */
#define BF1_SMMU0_CXT_IRPT_96_INT 297

/* Non-secure context interrupts for context 97 */
#define BF1_SMMU0_CXT_IRPT_97_INT 298

/* Non-secure context interrupts for context 98 */
#define BF1_SMMU0_CXT_IRPT_98_INT 299

/* Non-secure context interrupts for context 99 */
#define BF1_SMMU0_CXT_IRPT_99_INT 300

/* Non-secure context interrupts for context 100 */
#define BF1_SMMU0_CXT_IRPT_100_INT 301

/* Non-secure context interrupts for context 101 */
#define BF1_SMMU0_CXT_IRPT_101_INT 302

/* Non-secure context interrupts for context 102 */
#define BF1_SMMU0_CXT_IRPT_102_INT 303

/* Non-secure context interrupts for context 103 */
#define BF1_SMMU0_CXT_IRPT_103_INT 304

/* Non-secure context interrupts for context 104 */
#define BF1_SMMU0_CXT_IRPT_104_INT 305

/* Non-secure context interrupts for context 105 */
#define BF1_SMMU0_CXT_IRPT_105_INT 306

/* Non-secure context interrupts for context 106 */
#define BF1_SMMU0_CXT_IRPT_106_INT 307

/* Non-secure context interrupts for context 107 */
#define BF1_SMMU0_CXT_IRPT_107_INT 308

/* Non-secure context interrupts for context 108 */
#define BF1_SMMU0_CXT_IRPT_108_INT 309

/* Non-secure context interrupts for context 109 */
#define BF1_SMMU0_CXT_IRPT_109_INT 310

/* Non-secure context interrupts for context 110 */
#define BF1_SMMU0_CXT_IRPT_110_INT 311

/* Non-secure context interrupts for context 111 */
#define BF1_SMMU0_CXT_IRPT_111_INT 312

/* Non-secure context interrupts for context 112 */
#define BF1_SMMU0_CXT_IRPT_112_INT 313

/* Non-secure context interrupts for context 113 */
#define BF1_SMMU0_CXT_IRPT_113_INT 314

/* Non-secure context interrupts for context 114 */
#define BF1_SMMU0_CXT_IRPT_114_INT 315

/* Non-secure context interrupts for context 115 */
#define BF1_SMMU0_CXT_IRPT_115_INT 316

/* Non-secure context interrupts for context 116 */
#define BF1_SMMU0_CXT_IRPT_116_INT 317

/* Non-secure context interrupts for context 117 */
#define BF1_SMMU0_CXT_IRPT_117_INT 318

/* Non-secure context interrupts for context 118 */
#define BF1_SMMU0_CXT_IRPT_118_INT 319

/* Non-secure context interrupts for context 119 */
#define BF1_SMMU0_CXT_IRPT_119_INT 320

/* Non-secure context interrupts for context 120 */
#define BF1_SMMU0_CXT_IRPT_120_INT 321

/* Non-secure context interrupts for context 121 */
#define BF1_SMMU0_CXT_IRPT_121_INT 322

/* Non-secure context interrupts for context 122 */
#define BF1_SMMU0_CXT_IRPT_122_INT 323

/* Non-secure context interrupts for context 123 */
#define BF1_SMMU0_CXT_IRPT_123_INT 324

/* Non-secure context interrupts for context 124 */
#define BF1_SMMU0_CXT_IRPT_124_INT 325

/* Non-secure context interrupts for context 125 */
#define BF1_SMMU0_CXT_IRPT_125_INT 326

/* Non-secure context interrupts for context 126 */
#define BF1_SMMU0_CXT_IRPT_126_INT 327

/* Non-secure context interrupts for context 127 */
#define BF1_SMMU0_CXT_IRPT_127_INT 328

/* Performance counter for TRIO TBU 0 */
#define BF1_SMMU0_PERF_IRPT_0_INT 329

/* Performance counter for TRIO TBU 1 */
#define BF1_SMMU0_PERF_IRPT_1_INT 330

/* Performance counter for TRIO TBU 2 */
#define BF1_SMMU0_PERF_IRPT_2_INT 331

/* Global Non-secure fault interrupt */
#define BF1_SMMU0_GBL_FLT_IRPT_NS_INT 332

/* Global Secure fault interrupt */
#define BF1_SMMU0_GBL_FLT_IRPT_S_INT 333

#define BF1_TH_HE_MSS0_TBD0_INT 356

/* Error indicator for an L2 RAM double-bit ECC error */
#define BF1_BF_TILE_A720_INTERRIRQ_INT 917

/*
 * Error indicator for an CHI write transaction with a write response
 * error condition.
 */
#define BF1_BF_TILE_A720_EXTERRIRQ_INT 918

/* Error indicator for Home Node */
#define BF1_TH_TILE_HNF0_HNFIRQ_INT 919

/* Error indicator for an L2 RAM double-bit ECC error */
#define BF1_BF_TILE_A721_INTERRIRQ_INT 920

/*
 * Error indicator for an CHI write transaction with a write response
 * error condition.
 */
#define BF1_BF_TILE_A721_EXTERRIRQ_INT 921

/* Error indicator for Home Node */
#define BF1_TH_TILE_HNF1_HNFIRQ_INT 922

/* Error indicator for an L2 RAM double-bit ECC error */
#define BF1_BF_TILE_A722_INTERRIRQ_INT 923

/*
 * Error indicator for an CHI write transaction with a write response
 * error condition.
 */
#define BF1_BF_TILE_A722_EXTERRIRQ_INT 924

/* Error indicator for Home Node */
#define BF1_TH_TILE_HNF2_HNFIRQ_INT 925

/* Error indicator for an L2 RAM double-bit ECC error */
#define BF1_BF_TILE_A723_INTERRIRQ_INT 926

/*
 * Error indicator for an CHI write transaction with a write response
 * error condition.
 */
#define BF1_BF_TILE_A723_EXTERRIRQ_INT 927

/* Error indicator for Home Node */
#define BF1_TH_TILE_HNF3_HNFIRQ_INT 928

/* Error indicator for an L2 RAM double-bit ECC error */
#define BF1_BF_TILE_A724_INTERRIRQ_INT 929

/*
 * Error indicator for an CHI write transaction with a write response
 * error condition.
 */
#define BF1_BF_TILE_A724_EXTERRIRQ_INT 930

/* Error indicator for Home Node */
#define BF1_TH_TILE_HNF4_HNFIRQ_INT 931

/* Error indicator for an L2 RAM double-bit ECC error */
#define BF1_BF_TILE_A725_INTERRIRQ_INT 932

/*
 * Error indicator for an CHI write transaction with a write response
 * error condition.
 */
#define BF1_BF_TILE_A725_EXTERRIRQ_INT 933

/* Error indicator for Home Node */
#define BF1_TH_TILE_HNF5_HNFIRQ_INT 934

/* Error indicator for an L2 RAM double-bit ECC error */
#define BF1_BF_TILE_A726_INTERRIRQ_INT 935

/*
 * Error indicator for an CHI write transaction with a write response
 * error condition.
 */
#define BF1_BF_TILE_A726_EXTERRIRQ_INT 936

/* Error indicator for Home Node */
#define BF1_TH_TILE_HNF6_HNFIRQ_INT 937

/* Error indicator for an L2 RAM double-bit ECC error */
#define BF1_BF_TILE_A727_INTERRIRQ_INT 938

/*
 * Error indicator for an CHI write transaction with a write response
 * error condition.
 */
#define BF1_BF_TILE_A727_EXTERRIRQ_INT 939

/* Error indicator for Home Node */
#define BF1_TH_TILE_HNF7_HNFIRQ_INT 940

#endif /* !defined(__REGS_BF1_IRQS_H__) */
