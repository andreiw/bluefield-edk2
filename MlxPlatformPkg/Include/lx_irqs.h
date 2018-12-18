#ifndef __REGS_LX_INT_H__
#define __REGS_LX_INT_H__

/* LX Interrupts */

/* Software-triggered interrupt */
#define LX_RSH0_SWINT0_INT 32

/* Software-triggered interrupt */
#define LX_RSH0_SWINT1_INT 33

/* Software-triggered interrupt */
#define LX_RSH0_SWINT2_INT 34

/* Software-triggered interrupt */
#define LX_RSH0_SWINT3_INT 35

/* Access attempted from disabled external device */
#define LX_RSH0_DEV_PROT_INT 36

/* Access attempted to non-existent or secured device */
#define LX_RSH0_MMIO_ERR_INT 37

/* Down counter 0 interrupt */
#define LX_RSH0_DCNT0_INT 38

/* Down counter 1 interrupt */
#define LX_RSH0_DCNT1_INT 39

/* Down counter 2 interrupt */
#define LX_RSH0_DCNT2_INT 40

/* Host to tile low watermark reached */
#define LX_RSH0_TM_HTT_LWM_INT 41

/* Host to tile high watermark reached */
#define LX_RSH0_TM_HTT_HWM_INT 42

/* Tile to host low watermark reached */
#define LX_RSH0_TM_TTH_LWM_INT 43

/* Tile to host high watermark reached */
#define LX_RSH0_TM_TTH_HWM_INT 44

/* Host to tile read error - read attempted while fifo empty or disabled */
#define LX_RSH0_TM_HTT_RERR_INT 45

/*
 * Host to tile write error - write attempted while fifo full or
 * disabled
 */
#define LX_RSH0_TM_HTT_WERR_INT 46

/* Tile to host read error - read attempted while fifo empty or disabled */
#define LX_RSH0_TM_TTH_RERR_INT 47

/*
 * Tile to host write error - write attempted while fifo full or
 * disabled
 */
#define LX_RSH0_TM_TTH_WERR_INT 48

/* Reserved */
#define LX_RSH0_RSH_INT_RSVD0_INT 49

/* Reserved */
#define LX_RSH0_RSH_INT_RSVD1_INT 50

/* Reserved */
#define LX_RSH0_RSH_INT_RSVD2_INT 51

/* Power watchdog counter has reached TI Threshold */
#define LX_RSH0_PWR_WDOG_INT 52

/*
 * A request did not target any device in the system, so the mesh
 * routed it to the RSHIM. RSHIM sends a response to requests like
 * this to avoid a system hang.
 */
#define LX_RSH0_RSH_INT_BAD_PA_INT 53

#define LX_RSH0_DEVICE_UART0_INT 54

#define LX_RSH0_DEVICE_UART1_INT 55

#define LX_RSH0_DEVICE_DIAG_UART_INT 56

#define LX_RSH0_DEVICE_YU_INT 57

#define LX_RSH0_DEVICE_RSVD0_INT 58

#define LX_RSH0_DEVICE_RSVD1_INT 59

#define LX_RSH0_DEVICE_RSVD2_INT 60

#define LX_RSH0_DEVICE_TIMER_INT 61

#define LX_RSH0_DEVICE_USB_INT 62

#define LX_RSH0_DEVICE_OOB_INT 63

#define LX_RSH0_DEVICE_MMC_INT 64

#define LX_RSH0_DEVICE_TIMER_EXT_INT 65

#define LX_RSH0_DEVICE_WDOG_NS_INT 66

#define LX_RSH0_DEVICE_WDOG_SEC_INT 67

/* MAC Interrupt. */
#define LX_TH_TP0_MAC_INT 69

/*
 * A request targeted the rshim but the buffer was full so the request
 * was directed to the panic address.
 */
#define LX_TH_TP0_RSH_FULL_ERROR_INT 70

/* A message arrived for the message queue, but it was full. */
#define LX_TH_TP0_MSG_Q_FULL_ERROR_INT 71

/* A message has arrived in the message queue. */
#define LX_TH_TP0_MSG_Q_ARRIVED_INT 72

/*
 * An MMIO request encountered an error.  Error info captured in
 * MMIO_ERROR_INFO.
 */
#define LX_TH_TP0_MMIO_ERR_INT 73

/*
 * An ingress packet was not claimed by any map region.  The info for
 * the request is captured in MAP_ERR_STS.
 */
#define LX_TH_TP0_MAP_UNCLAIMED_INT 74

/* A request targeting the rshim violated the maximum read request size. */
#define LX_TH_TP0_RSH_SIZE_ERROR_INT 75

/* A PIO ECAM transaction was malformed (crossed DWD boundary). */
#define LX_TH_TP0_PIO_ECAM_ERR_INT 76

/*
 * A PIO transaction received an error completion from the MAC.  The
 * associated region is stored in TILE_PIO_CPL_ERR_STS.
 */
#define LX_TH_TP0_PIO_CPL_ERR_INT 77

/* An MMIO request violated the config register protection level. */
#define LX_TH_TP0_MMIO_PROT_ERR_INT 78

/* The PUSH_DMA_EVT_CTR wrapped. */
#define LX_TH_TP0_PUSH_DMA_EVT_CTR_INT 79

/* The MAP_EVT_CTR wrapped. */
#define LX_TH_TP0_MAP_EVT_CTR_INT 80

/*
 * Access to disabled PIO region.  Error info captured in
 * TILE_PIO_TIMEOUT_STS.
 */
#define LX_TH_TP0_PIO_DISABLED_INT 81

/*
 * Remote buffer return encountered MMIO error (not applicable to all
 * devices).
 */
#define LX_TH_TP0_REM_MMIO_ERR_INT 82

/* Correctable error message received. */
#define LX_TH_TP0_ERR_MSG_COR_INT 83

/* Non-fatal error message received. */
#define LX_TH_TP0_ERR_MSG_NONFATAL_INT 84

/* Fatal error message received. */
#define LX_TH_TP0_ERR_MSG_FATAL_INT 85

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define LX_TH_TP0_LEVEL_INT0_INT 86

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define LX_TH_TP0_LEVEL_INT1_INT 87

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define LX_TH_TP0_LEVEL_INT2_INT 88

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define LX_TH_TP0_LEVEL_INT3_INT 89

/* MAC Interrupt. */
#define LX_TH_TP1_MAC_INT 91

/*
 * A request targeted the rshim but the buffer was full so the request
 * was directed to the panic address.
 */
#define LX_TH_TP1_RSH_FULL_ERROR_INT 92

/* A message arrived for the message queue, but it was full. */
#define LX_TH_TP1_MSG_Q_FULL_ERROR_INT 93

/* A message has arrived in the message queue. */
#define LX_TH_TP1_MSG_Q_ARRIVED_INT 94

/*
 * An MMIO request encountered an error.  Error info captured in
 * MMIO_ERROR_INFO.
 */
#define LX_TH_TP1_MMIO_ERR_INT 95

/*
 * An ingress packet was not claimed by any map region.  The info for
 * the request is captured in MAP_ERR_STS.
 */
#define LX_TH_TP1_MAP_UNCLAIMED_INT 96

/* A request targeting the rshim violated the maximum read request size. */
#define LX_TH_TP1_RSH_SIZE_ERROR_INT 97

/* A PIO ECAM transaction was malformed (crossed DWD boundary). */
#define LX_TH_TP1_PIO_ECAM_ERR_INT 98

/*
 * A PIO transaction received an error completion from the MAC.  The
 * associated region is stored in TILE_PIO_CPL_ERR_STS.
 */
#define LX_TH_TP1_PIO_CPL_ERR_INT 99

/* An MMIO request violated the config register protection level. */
#define LX_TH_TP1_MMIO_PROT_ERR_INT 100

/* The PUSH_DMA_EVT_CTR wrapped. */
#define LX_TH_TP1_PUSH_DMA_EVT_CTR_INT 101

/* The MAP_EVT_CTR wrapped. */
#define LX_TH_TP1_MAP_EVT_CTR_INT 102

/*
 * Access to disabled PIO region.  Error info captured in
 * TILE_PIO_TIMEOUT_STS.
 */
#define LX_TH_TP1_PIO_DISABLED_INT 103

/*
 * Remote buffer return encountered MMIO error (not applicable to all
 * devices).
 */
#define LX_TH_TP1_REM_MMIO_ERR_INT 104

/* Correctable error message received. */
#define LX_TH_TP1_ERR_MSG_COR_INT 105

/* Non-fatal error message received. */
#define LX_TH_TP1_ERR_MSG_NONFATAL_INT 106

/* Fatal error message received. */
#define LX_TH_TP1_ERR_MSG_FATAL_INT 107

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define LX_TH_TP1_LEVEL_INT0_INT 108

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define LX_TH_TP1_LEVEL_INT1_INT 109

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define LX_TH_TP1_LEVEL_INT2_INT 110

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define LX_TH_TP1_LEVEL_INT3_INT 111

/* MAC Interrupt. */
#define LX_TH_HCA0_MAC_INT 113

/*
 * A request targeted the rshim but the buffer was full so the request
 * was directed to the panic address.
 */
#define LX_TH_HCA0_RSH_FULL_ERROR_INT 114

/* A message arrived for the message queue, but it was full. */
#define LX_TH_HCA0_MSG_Q_FULL_ERROR_INT 115

/* A message has arrived in the message queue. */
#define LX_TH_HCA0_MSG_Q_ARRIVED_INT 116

/*
 * An MMIO request encountered an error.  Error info captured in
 * MMIO_ERROR_INFO.
 */
#define LX_TH_HCA0_MMIO_ERR_INT 117

/*
 * An ingress packet was not claimed by any map region.  The info for
 * the request is captured in MAP_ERR_STS.
 */
#define LX_TH_HCA0_MAP_UNCLAIMED_INT 118

/* A request targeting the rshim violated the maximum read request size. */
#define LX_TH_HCA0_RSH_SIZE_ERROR_INT 119

/* A PIO ECAM transaction was malformed (crossed DWD boundary). */
#define LX_TH_HCA0_PIO_ECAM_ERR_INT 120

/*
 * A PIO transaction received an error completion from the MAC.  The
 * associated region is stored in TILE_PIO_CPL_ERR_STS.
 */
#define LX_TH_HCA0_PIO_CPL_ERR_INT 121

/* An MMIO request violated the config register protection level. */
#define LX_TH_HCA0_MMIO_PROT_ERR_INT 122

/* The PUSH_DMA_EVT_CTR wrapped. */
#define LX_TH_HCA0_PUSH_DMA_EVT_CTR_INT 123

/* The MAP_EVT_CTR wrapped. */
#define LX_TH_HCA0_MAP_EVT_CTR_INT 124

/*
 * Access to disabled PIO region.  Error info captured in
 * TILE_PIO_TIMEOUT_STS.
 */
#define LX_TH_HCA0_PIO_DISABLED_INT 125

/*
 * Remote buffer return encountered MMIO error (not applicable to all
 * devices).
 */
#define LX_TH_HCA0_REM_MMIO_ERR_INT 126

/* Correctable error message received. */
#define LX_TH_HCA0_ERR_MSG_COR_INT 127

/* Non-fatal error message received. */
#define LX_TH_HCA0_ERR_MSG_NONFATAL_INT 128

/* Fatal error message received. */
#define LX_TH_HCA0_ERR_MSG_FATAL_INT 129

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define LX_TH_HCA0_LEVEL_INT0_INT 130

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define LX_TH_HCA0_LEVEL_INT1_INT 131

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define LX_TH_HCA0_LEVEL_INT2_INT 132

/*
 * Generic level-sensitive interrupt associted with TRIO_LEVEL_INT
 * register.
 */
#define LX_TH_HCA0_LEVEL_INT3_INT 133

/* L3C functional interrupt, check INTRPT_CAUSE for further information. */
#define LX_TH_MSS0_L3C0_FUNCTIONAL_INT 135

/*
 * L3C RAMS ECC interrupt, check ECC_DOUBLE_ERROR_0/ECC_SINGLE_ERROR_0
 * for further information.
 */
#define LX_TH_MSS0_L3C0_RAMS_ERR_INT 136

/* L3C functional interrupt, check INTRPT_CAUSE for further information. */
#define LX_TH_MSS0_L3C1_FUNCTIONAL_INT 137

/*
 * L3C RAMS ECC interrupt, check ECC_DOUBLE_ERROR_0/ECC_SINGLE_ERROR_0
 * for further information.
 */
#define LX_TH_MSS0_L3C1_RAMS_ERR_INT 138

/* EMEM_MI ECC SERR interrupt. */
#define LX_TH_MSS0_EMEM_MI_SERR_INT 139

/* EMEM_MI ECC DERR interrupt. */
#define LX_TH_MSS0_EMEM_MI_DERR_INT 140

/* EMEM_MI functional interrupt. */
#define LX_TH_MSS0_EMEM_MI_FUNCTIONAL_INT 141

/* EMEM_MC calibration interrupt. */
#define LX_TH_MSS0_EMEM_MC_CALIB_INT 142

/* Command count for queue 0 on PKA 0 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_0_0_INT 144

/* Command count for queue 1 on PKA 0 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_0_1_INT 145

/* Command count for queue 2 on PKA 0 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_0_2_INT 146

/* Command count for queue 3 on PKA 0 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_0_3_INT 147

/* Result count for queue 0 on PKA 0 is above threshold value. */
#define LX_TH_CR0_RES_CNT_0_0_INT 148

/* Result count for queue 1 on PKA 0 is above threshold value. */
#define LX_TH_CR0_RES_CNT_0_1_INT 149

/* Result count for queue 2 on PKA 0 is above threshold value. */
#define LX_TH_CR0_RES_CNT_0_2_INT 150

/* Result count for queue 3 on PKA 0 is above threshold value. */
#define LX_TH_CR0_RES_CNT_0_3_INT 151

/* PKA Master Controller on PKA 0 requests attention */
#define LX_TH_CR0_PKA_MASTER_0_INT 152

/* TRNG (True Random Number Generator) on PKA 0 requests attention */
#define LX_TH_CR0_TRNG_0_INT 153

/* Command count for queue 0 on PKA 1 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_1_0_INT 154

/* Command count for queue 1 on PKA 1 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_1_1_INT 155

/* Command count for queue 2 on PKA 1 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_1_2_INT 156

/* Command count for queue 3 on PKA 1 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_1_3_INT 157

/* Result count for queue 0 on PKA 1 is above threshold value. */
#define LX_TH_CR0_RES_CNT_1_0_INT 158

/* Result count for queue 1 on PKA 1 is above threshold value. */
#define LX_TH_CR0_RES_CNT_1_1_INT 159

/* Result count for queue 2 on PKA 1 is above threshold value. */
#define LX_TH_CR0_RES_CNT_1_2_INT 160

/* Result count for queue 3 on PKA 1 is above threshold value. */
#define LX_TH_CR0_RES_CNT_1_3_INT 161

/* PKA Master Controller on PKA 1 requests attention */
#define LX_TH_CR0_PKA_MASTER_1_INT 162

/* TRNG (True Random Number Generator) on PKA 1 requests attention */
#define LX_TH_CR0_TRNG_1_INT 163

/* Command count for queue 0 on PKA 2 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_2_0_INT 164

/* Command count for queue 1 on PKA 2 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_2_1_INT 165

/* Command count for queue 2 on PKA 2 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_2_2_INT 166

/* Command count for queue 3 on PKA 2 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_2_3_INT 167

/* Result count for queue 0 on PKA 2 is above threshold value. */
#define LX_TH_CR0_RES_CNT_2_0_INT 168

/* Result count for queue 1 on PKA 2 is above threshold value. */
#define LX_TH_CR0_RES_CNT_2_1_INT 169

/* Result count for queue 2 on PKA 2 is above threshold value. */
#define LX_TH_CR0_RES_CNT_2_2_INT 170

/* Result count for queue 3 on PKA 2 is above threshold value. */
#define LX_TH_CR0_RES_CNT_2_3_INT 171

/* PKA Master Controller on PKA 2 requests attention */
#define LX_TH_CR0_PKA_MASTER_2_INT 172

/* TRNG (True Random Number Generator) on PKA 2 requests attention */
#define LX_TH_CR0_TRNG_2_INT 173

/* Command count for queue 0 on PKA 3 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_3_0_INT 174

/* Command count for queue 1 on PKA 3 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_3_1_INT 175

/* Command count for queue 2 on PKA 3 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_3_2_INT 176

/* Command count for queue 3 on PKA 3 is below threshold value. */
#define LX_TH_CR0_CMD_CNT_3_3_INT 177

/* Result count for queue 0 on PKA 3 is above threshold value. */
#define LX_TH_CR0_RES_CNT_3_0_INT 178

/* Result count for queue 1 on PKA 3 is above threshold value. */
#define LX_TH_CR0_RES_CNT_3_1_INT 179

/* Result count for queue 2 on PKA 3 is above threshold value. */
#define LX_TH_CR0_RES_CNT_3_2_INT 180

/* Result count for queue 3 on PKA 3 is above threshold value. */
#define LX_TH_CR0_RES_CNT_3_3_INT 181

/* PKA Master Controller on PKA 3 requests attention */
#define LX_TH_CR0_PKA_MASTER_3_INT 182

/* TRNG (True Random Number Generator) on PKA 3 requests attention */
#define LX_TH_CR0_TRNG_3_INT 183

/* Command count for queue 0 on PKA 0 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_0_0_INT 185

/* Command count for queue 1 on PKA 0 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_0_1_INT 186

/* Command count for queue 2 on PKA 0 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_0_2_INT 187

/* Command count for queue 3 on PKA 0 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_0_3_INT 188

/* Result count for queue 0 on PKA 0 is above threshold value. */
#define LX_TH_CR1_RES_CNT_0_0_INT 189

/* Result count for queue 1 on PKA 0 is above threshold value. */
#define LX_TH_CR1_RES_CNT_0_1_INT 190

/* Result count for queue 2 on PKA 0 is above threshold value. */
#define LX_TH_CR1_RES_CNT_0_2_INT 191

/* Result count for queue 3 on PKA 0 is above threshold value. */
#define LX_TH_CR1_RES_CNT_0_3_INT 192

/* PKA Master Controller on PKA 0 requests attention */
#define LX_TH_CR1_PKA_MASTER_0_INT 193

/* TRNG (True Random Number Generator) on PKA 0 requests attention */
#define LX_TH_CR1_TRNG_0_INT 194

/* Command count for queue 0 on PKA 1 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_1_0_INT 195

/* Command count for queue 1 on PKA 1 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_1_1_INT 196

/* Command count for queue 2 on PKA 1 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_1_2_INT 197

/* Command count for queue 3 on PKA 1 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_1_3_INT 198

/* Result count for queue 0 on PKA 1 is above threshold value. */
#define LX_TH_CR1_RES_CNT_1_0_INT 199

/* Result count for queue 1 on PKA 1 is above threshold value. */
#define LX_TH_CR1_RES_CNT_1_1_INT 200

/* Result count for queue 2 on PKA 1 is above threshold value. */
#define LX_TH_CR1_RES_CNT_1_2_INT 201

/* Result count for queue 3 on PKA 1 is above threshold value. */
#define LX_TH_CR1_RES_CNT_1_3_INT 202

/* PKA Master Controller on PKA 1 requests attention */
#define LX_TH_CR1_PKA_MASTER_1_INT 203

/* TRNG (True Random Number Generator) on PKA 1 requests attention */
#define LX_TH_CR1_TRNG_1_INT 204

/* Command count for queue 0 on PKA 2 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_2_0_INT 205

/* Command count for queue 1 on PKA 2 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_2_1_INT 206

/* Command count for queue 2 on PKA 2 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_2_2_INT 207

/* Command count for queue 3 on PKA 2 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_2_3_INT 208

/* Result count for queue 0 on PKA 2 is above threshold value. */
#define LX_TH_CR1_RES_CNT_2_0_INT 209

/* Result count for queue 1 on PKA 2 is above threshold value. */
#define LX_TH_CR1_RES_CNT_2_1_INT 210

/* Result count for queue 2 on PKA 2 is above threshold value. */
#define LX_TH_CR1_RES_CNT_2_2_INT 211

/* Result count for queue 3 on PKA 2 is above threshold value. */
#define LX_TH_CR1_RES_CNT_2_3_INT 212

/* PKA Master Controller on PKA 2 requests attention */
#define LX_TH_CR1_PKA_MASTER_2_INT 213

/* TRNG (True Random Number Generator) on PKA 2 requests attention */
#define LX_TH_CR1_TRNG_2_INT 214

/* Command count for queue 0 on PKA 3 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_3_0_INT 215

/* Command count for queue 1 on PKA 3 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_3_1_INT 216

/* Command count for queue 2 on PKA 3 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_3_2_INT 217

/* Command count for queue 3 on PKA 3 is below threshold value. */
#define LX_TH_CR1_CMD_CNT_3_3_INT 218

/* Result count for queue 0 on PKA 3 is above threshold value. */
#define LX_TH_CR1_RES_CNT_3_0_INT 219

/* Result count for queue 1 on PKA 3 is above threshold value. */
#define LX_TH_CR1_RES_CNT_3_1_INT 220

/* Result count for queue 2 on PKA 3 is above threshold value. */
#define LX_TH_CR1_RES_CNT_3_2_INT 221

/* Result count for queue 3 on PKA 3 is above threshold value. */
#define LX_TH_CR1_RES_CNT_3_3_INT 222

/* PKA Master Controller on PKA 3 requests attention */
#define LX_TH_CR1_PKA_MASTER_3_INT 223

/* TRNG (True Random Number Generator) on PKA 3 requests attention */
#define LX_TH_CR1_TRNG_3_INT 224

/* Event Queue Secure */
#define LX_SMMU0_EVENT_Q_IRPT_S_INT 226

/* Event Queue Non-secure */
#define LX_SMMU0_EVENT_Q_IRPT_NS_INT 227

/* Sync complete Non-secure */
#define LX_SMMU0_CMD_SYNC_IRPT_NS_INT 228

/* Sync complete Secure */
#define LX_SMMU0_CMD_SYNC_IRPT_S_INT 229

/* Global Non-secure */
#define LX_SMMU0_GLOBAL_IRPT_NS_INT 230

/* Global Secure */
#define LX_SMMU0_GLOBAL_IRPT_S_INT 231

/* Reliability, Availability, Serviceability */
#define LX_SMMU0_RAS_IRPT_INT 232

/* PMU */
#define LX_SMMU0_PMU_IRPT_INT 233

/* PRI Queue Non-secure */
#define LX_SMMU0_PRI_Q_IRPT_NS_INT 234

#define LX_TH_HE_MSS0_TBD0_INT 257

/* Error indicator for an L2 RAM double-bit ECC error */
#define LX_BF_TILE_A720_INTERRIRQ_INT 917

/*
 * Error indicator for an CHI write transaction with a write response
 * error condition.
 */
#define LX_BF_TILE_A720_EXTERRIRQ_INT 918

/* Error indicator for Home Node */
#define LX_TH_TILE_HNF0_HNFIRQ_INT 919

/* Error indicator for an L2 RAM double-bit ECC error */
#define LX_BF_TILE_A721_INTERRIRQ_INT 920

/*
 * Error indicator for an CHI write transaction with a write response
 * error condition.
 */
#define LX_BF_TILE_A721_EXTERRIRQ_INT 921

/* Error indicator for Home Node */
#define LX_TH_TILE_HNF1_HNFIRQ_INT 922

/* Error indicator for an L2 RAM double-bit ECC error */
#define LX_BF_TILE_A722_INTERRIRQ_INT 923

/*
 * Error indicator for an CHI write transaction with a write response
 * error condition.
 */
#define LX_BF_TILE_A722_EXTERRIRQ_INT 924

/* Error indicator for Home Node */
#define LX_TH_TILE_HNF2_HNFIRQ_INT 925

/* Error indicator for an L2 RAM double-bit ECC error */
#define LX_BF_TILE_A723_INTERRIRQ_INT 926

/*
 * Error indicator for an CHI write transaction with a write response
 * error condition.
 */
#define LX_BF_TILE_A723_EXTERRIRQ_INT 927

/* Error indicator for Home Node */
#define LX_TH_TILE_HNF3_HNFIRQ_INT 928

#endif /* !defined(__REGS_LX_INT_H__) */
