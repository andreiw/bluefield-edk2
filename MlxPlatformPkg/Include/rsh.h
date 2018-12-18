#ifndef __REGS_RSH_H__
#define __REGS_RSH_H__

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif
#include "rsh_def.h"

#ifndef __ASSEMBLER__


#ifndef __DOXYGEN__

/*
 * MMIO Address Space.
 * The MMIO physical address space for the rshim is described below.  This is
 * a general description of the MMIO space as opposed to a register
 * description
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * This field of the address provides an offset into the
		 * channel whose register space is being accessed.
		 */
		uint64_t offset       : 16;
		/*
		 * Protection level.  Setting to 0 or 1 allows access to all
		 * registers.  Setting to 2 denies access to registers at
		 * level 2.  Setting to 3 denies access to registers at
		 * levels 1 and 2. Other settings are reserved for channel-0.
		 */
		uint64_t prot         : 3;
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * This field of the address selects the channel (device).
		 * Channel-0 is boot. Channel-1 is the common rshim
		 * registers.  Channels 0-15 are used to access the
		 * rshim-attached devices.
		 */
		uint64_t channel      : 4;
		/* Reserved. */
		uint64_t __reserved_1 : 37;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 37;
		uint64_t channel      : 4;
		uint64_t __reserved_0 : 4;
		uint64_t prot         : 3;
		uint64_t offset       : 16;
#endif
	};

	uint64_t word;
} RSH_MMIO_ADDRESS_SPACE_t;


/*
 * Device Info.
 * This register provides general information about the device attached to
 * this port and channel.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Encoded device Type - 33 to indicate rshim */
		uint64_t type         : 12;
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/* Device revision ID. */
		uint64_t device_rev   : 8;
		/* Register format architectural revision. */
		uint64_t register_rev : 4;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/* Instance ID for multi-instantiated devices. */
		uint64_t instance     : 4;
		/* Reserved. */
		uint64_t __reserved_2 : 28;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 28;
		uint64_t instance     : 4;
		uint64_t __reserved_1 : 4;
		uint64_t register_rev : 4;
		uint64_t device_rev   : 8;
		uint64_t __reserved_0 : 4;
		uint64_t type         : 12;
#endif
	};

	uint64_t word;
} RSH_DEV_INFO_t;


/*
 * Device Control.
 * This register provides general device control.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When 1, packets sent on the NDN will be routed x-first.
		 * When 0, packets will be routed y-first.  This setting must
		 * match the setting in the Tiles.  Devices may have
		 * additional interfaces with customized route-order settings
		 * used in addition to or instead of this field.
		 */
		uint64_t ndn_route_order : 1;
		/*
		 * When 1, packets sent on the CDN will be routed x-first.
		 * When 0, packets will be routed y-first.  This setting must
		 * match the setting in the Tiles.  Devices may have
		 * additional interfaces with customized route-order settings
		 * used in addition to or instead of this field.
		 */
		uint64_t cdn_route_order : 1;
		/*
		 * When 1, packets sent on the DDN will be routed x-first.
		 * When 0, packets will be routed y-first.  This setting must
		 * match the setting in the Tiles.  Devices may have
		 * additional interfaces with customized route-order settings
		 * used in addition to or instead of this field.
		 */
		uint64_t ddn_route_order : 1;
		/*
		 * When 1, the ExpCompAck flow will be used on DMA reads
		 * which allows read-data-bypass for lower latency. Must only
		 * be changed if no DMA read traffic is inflight.
		 */
		uint64_t dma_rd_ca_ena   : 1;
		/*
		 * For devices with DMA. When 1, the L3 cache profile will be
		 * forced to L3_PROFILE_VAL. When 0, the L3 profile is
		 * selected by the device.
		 */
		uint64_t l3_profile_ovd  : 1;
		/*
		 * For devices with DMA. L3 cache profile to be used when
		 * L3_PROFILE_OVD is 1.
		 */
		uint64_t l3_profile_val  : 4;
		/* Write response mapping for MMIO slave errors */
		uint64_t wr_slverr_map   : 2;
		/* Write response mapping for MMIO decode errors */
		uint64_t wr_decerr_map   : 2;
		/* Read response mapping for MMIO slave errors */
		uint64_t rd_slverr_map   : 2;
		/* Read response mapping for MMIO decode errors */
		uint64_t rd_decerr_map   : 2;
		/*
		 * When 1, the CDN sync FIFO is allowed to back pressure
		 * until full to avoid retries and improve performance
		 */
		uint64_t cdn_req_buf_ena : 1;
		/* Reserved. */
		uint64_t __reserved_0    : 2;
		/*
		 * For diagnostics only. Block new traffic when WRQ_INFL
		 * count exceeds this threshold
		 */
		uint64_t dma_wrq_hwm     : 8;
		/* For diagnostics only. Adjust packet gather delay on RNF */
		uint64_t gthr_delay_adj  : 4;
		/* Reserved. */
		uint64_t __reserved_1    : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1    : 32;
		uint64_t gthr_delay_adj  : 4;
		uint64_t dma_wrq_hwm     : 8;
		uint64_t __reserved_0    : 2;
		uint64_t cdn_req_buf_ena : 1;
		uint64_t rd_decerr_map   : 2;
		uint64_t rd_slverr_map   : 2;
		uint64_t wr_decerr_map   : 2;
		uint64_t wr_slverr_map   : 2;
		uint64_t l3_profile_val  : 4;
		uint64_t l3_profile_ovd  : 1;
		uint64_t dma_rd_ca_ena   : 1;
		uint64_t ddn_route_order : 1;
		uint64_t cdn_route_order : 1;
		uint64_t ndn_route_order : 1;
#endif
	};

	uint64_t word;
} RSH_DEV_CTL_t;


/* Scratchpad. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Scratchpad. */
		uint64_t scratchpad : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t scratchpad : 64;
#endif
	};

	uint64_t word;
} RSH_SCRATCHPAD_t;


/*
 * Semaphore0.
 * Semaphore
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When read, the current semaphore value is returned and the
		 * semaphore is set to 1.  Bit can also be written to 1 or 0.
		 */
		uint64_t val        : 1;
		/* Reserved. */
		uint64_t __reserved : 63;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 63;
		uint64_t val        : 1;
#endif
	};

	uint64_t word;
} RSH_SEMAPHORE0_t;


/*
 * DMA Status.
 * DMA status information for debug. Unused for devices that do not have DMA.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t rdq_infl_count : 9;
		uint64_t wrq_infl_count : 9;
		/* Reserved. */
		uint64_t __reserved     : 7;
		/* Internal diagnostics status */
		uint64_t wrq_diag_vec   : 39;
#else   /* __BIG_ENDIAN__ */
		uint64_t wrq_diag_vec   : 39;
		uint64_t __reserved     : 7;
		uint64_t wrq_infl_count : 9;
		uint64_t rdq_infl_count : 9;
#endif
	};

	uint64_t word;
} RSH_DMA_STATUS_t;


/* Clock Count. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When 1, the counter is running.  Cleared by HW once count
		 * is complete.  When written with a 1, the count sequence is
		 * restarted.  Counter runs automatically after reset.
		 * Software must poll until this bit is zero, then read the
		 * CLOCK_COUNT register again to get the final COUNT value.
		 */
		uint64_t run        : 1;
		/*
		 * Indicates the number of core clocks that were counted
		 * during 1000 device clock periods.  Result is accurate to
		 * within +/-1 core clock periods.
		 */
		uint64_t count      : 15;
		/* Reserved. */
		uint64_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 48;
		uint64_t count      : 15;
		uint64_t run        : 1;
#endif
	};

	uint64_t word;
} RSH_CLOCK_COUNT_t;


/*
 * Interrupt Setup.
 * Configuration for device interrupts.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Global enable. Must be 1 for any interrupts to be sent. */
		uint64_t gbl_ena      : 1;
		/* Reserved. */
		uint64_t __reserved   : 47;
		/*
		 * Base interrupt number. The interrupt number within the
		 * device will added to this number to form the interrupt
		 * number sent to the GIC.
		 */
		uint64_t base_int_num : 16;
#else   /* __BIG_ENDIAN__ */
		uint64_t base_int_num : 16;
		uint64_t __reserved   : 47;
		uint64_t gbl_ena      : 1;
#endif
	};

	uint64_t word;
} RSH_INT_SETUP_t;


/*
 * Feature Control.
 * Device-specific feature controls.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Feature Control.
		 * Device-specific feature controls.
		 */
		uint64_t feature_ctl : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t feature_ctl : 64;
#endif
	};

	uint64_t word;
} RSH_FEATURE_CTL_t;


/*
 * Credit Control.
 * Provides access to the request-credit counters that control end-to-end
 * flow control between the device and other nodes in the system.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When written with a 1, the associated credit counter will
		 * be updated and the resulting value will be placed in VAL.
		 * This register clears once the access has occurred.
		 * Software must poll UPDATE until is is clear before VAL
		 * will be valid. A counter can be read without modifying it
		 * by setting VAL=0.
		 */
		uint64_t update     : 1;
		/*
		 * When UPDATE is written with a 1, this register determines
		 * which credit counter is accessed. For ring targets, this
		 * is {DIR,Link[3:0],CHANNEL[3:0]}
		 */
		uint64_t nodeid_sel : 11;
		/* Select target block. Not all nodes support all blocks. */
		uint64_t tgt_sel    : 2;
		/*
		 * For nodes with multiple DMA clients, this selects the
		 * client being accessed.
		 */
		uint64_t client_sel : 2;
		/*
		 * When UPDATE is written with a 1, this signed number will
		 * be added to the selected credit counter. The resulting
		 * value is provided in this register.
		 */
		uint64_t val        : 16;
		/* Reserved. */
		uint64_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 32;
		uint64_t val        : 16;
		uint64_t client_sel : 2;
		uint64_t tgt_sel    : 2;
		uint64_t nodeid_sel : 11;
		uint64_t update     : 1;
#endif
	};

	uint64_t word;
} RSH_CRED_CTL_t;


/*
 * SAM Control.
 * Provides access to SAM initialization.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When written with a 1, the associated table will be
		 * accessed. This register clears once the access has
		 * occurred. Software must poll UPDATE until is is clear
		 * before VAL will be valid.
		 */
		uint64_t update     : 1;
		/*
		 * When UPDATE is written with a 1, this register determines
		 * which entry is accessed.
		 */
		uint64_t idx        : 12;
		/* Structure to be accessed. */
		uint64_t tbl_sel    : 3;
		/*
		 * For nodes with multiple DMA clients, this selects the
		 * client being accessed.
		 */
		uint64_t client_sel : 1;
		/*
		 * When 1, the entry will not be updated and VAL will return
		 * the table data.
		 */
		uint64_t read       : 1;
		/*
		 * When UPDATE is written with a 1, this value will be
		 * written to the selected table entry.
		 */
		uint64_t val        : 34;
		/* Reserved. */
		uint64_t __reserved : 12;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 12;
		uint64_t val        : 34;
		uint64_t read       : 1;
		uint64_t client_sel : 1;
		uint64_t tbl_sel    : 3;
		uint64_t idx        : 12;
		uint64_t update     : 1;
#endif
	};

	uint64_t word;
} RSH_SAM_CTL_t;


/*
 * DMA Read Pacer.
 * Control the behavior of dma read pacer
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* When set, enables the dma read pacer */
		uint64_t en               : 1;
		/* Average extra delay for each read request */
		uint64_t per_read_penalty : 3;
		/* The max value of counter in the read pacer */
		uint64_t max_count        : 9;
		/*
		 * Control the ratio of reads with extra read penalty. The
		 * penalty ratio is equal to PENALTY_FRACATION/256
		 */
		uint64_t penalty_fraction : 8;
		/* Reserved. */
		uint64_t __reserved       : 43;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved       : 43;
		uint64_t penalty_fraction : 8;
		uint64_t max_count        : 9;
		uint64_t per_read_penalty : 3;
		uint64_t en               : 1;
#endif
	};

	uint64_t word;
} RSH_DMA_READ_PACE_t;


/*
 * DMA Write Latency.
 * Provides random sample and record of DMA AXI4 write latency
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Contains minimum latency since last clear. */
		uint64_t min_lat       : 15;
		/* Reserved. */
		uint64_t __reserved_0  : 1;
		/* Contains maximum latency since last clear. */
		uint64_t max_lat       : 15;
		/* Reserved. */
		uint64_t __reserved_1  : 1;
		/*
		 * Contains latency of the most recently sampled transaction.
		 */
		uint64_t curr_lat      : 15;
		/* Reserved. */
		uint64_t __reserved_2  : 1;
		/*
		 * When written with a 1, sets MAX_LAT to zero and MIN_LAT to
		 * all 1's.
		 */
		uint64_t clear         : 1;
		/* Contains maximum outstanding requests. */
		uint64_t max_stnd_reqs : 12;
		/* Reserved. */
		uint64_t __reserved_3  : 3;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_3  : 3;
		uint64_t max_stnd_reqs : 12;
		uint64_t clear         : 1;
		uint64_t __reserved_2  : 1;
		uint64_t curr_lat      : 15;
		uint64_t __reserved_1  : 1;
		uint64_t max_lat       : 15;
		uint64_t __reserved_0  : 1;
		uint64_t min_lat       : 15;
#endif
	};

	uint64_t word;
} RSH_DMA_WR_LAT_t;


/*
 * DMA Read Latency.
 * Provides random sample and record of DMA AXI4 read latency
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Contains minimum latency since last clear. */
		uint64_t min_lat       : 15;
		/* Reserved. */
		uint64_t __reserved_0  : 1;
		/* Contains maximum latency since last clear. */
		uint64_t max_lat       : 15;
		/* Reserved. */
		uint64_t __reserved_1  : 1;
		/*
		 * Contains latency of the most recently sampled transaction.
		 */
		uint64_t curr_lat      : 15;
		/* Reserved. */
		uint64_t __reserved_2  : 1;
		/*
		 * When written with a 1, sets MAX_LAT to zero and MIN_LAT to
		 * all 1's.
		 */
		uint64_t clear         : 1;
		/* Contains maximum outstanding requests. */
		uint64_t max_stnd_reqs : 12;
		/* Reserved. */
		uint64_t __reserved_3  : 3;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_3  : 3;
		uint64_t max_stnd_reqs : 12;
		uint64_t clear         : 1;
		uint64_t __reserved_2  : 1;
		uint64_t curr_lat      : 15;
		uint64_t __reserved_1  : 1;
		uint64_t max_lat       : 15;
		uint64_t __reserved_0  : 1;
		uint64_t min_lat       : 15;
#endif
	};

	uint64_t word;
} RSH_DMA_RD_LAT_t;


/*
 * Bandwidth_Control.
 * Limit the bandwidth between the node and the mesh. There are four
 * bandwidth control registers, which respectively control WRITE-CDN,
 * WRITE-DDN, READ-CDN and READ-DDN channel
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* When set, bandwidth control module is enabled */
		uint64_t en               : 1;
		/* The max value of buckets in the bandwidth control module */
		uint64_t max_count        : 9;
		/* Reserved. */
		uint64_t __reserved_0     : 10;
		/*
		 * Token is expressed as fraction format, token per cycle is
		 * equal to TOKEN_NUM/1023
		 */
		uint64_t token_num        : 10;
		/* Extra penalty added for each write/read transcation */
		uint64_t penalty_num      : 3;
		/*
		 * Control the ratio of reads with extra write/read penalty.
		 * The penalty ratio is equal to PENALTY_FRACATION/256
		 */
		uint64_t penalty_fraction : 8;
		/* Reserved. */
		uint64_t __reserved_1     : 23;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1     : 23;
		uint64_t penalty_fraction : 8;
		uint64_t penalty_num      : 3;
		uint64_t token_num        : 10;
		uint64_t __reserved_0     : 10;
		uint64_t max_count        : 9;
		uint64_t en               : 1;
#endif
	};

	uint64_t word;
} RSH_BND_CTL_t;


/*
 * Bandwidth_Control.
 * Limit the bandwidth between the node and the mesh. There are four
 * bandwidth control registers, which respectively control WRITE-CDN,
 * WRITE-DDN, READ-CDN and READ-DDN channel
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* When set, bandwidth control module is enabled */
		uint64_t en               : 1;
		/* The max value of buckets in the bandwidth control module */
		uint64_t max_count        : 9;
		/* Reserved. */
		uint64_t __reserved_0     : 10;
		/*
		 * Token is expressed as fraction format, token per cycle is
		 * equal to TOKEN_NUM/1023
		 */
		uint64_t token_num        : 10;
		/* Extra penalty added for each write/read transcation */
		uint64_t penalty_num      : 3;
		/*
		 * Control the ratio of reads with extra write/read penalty.
		 * The penalty ratio is equal to PENALTY_FRACATION/256
		 */
		uint64_t penalty_fraction : 8;
		/* Reserved. */
		uint64_t __reserved_1     : 23;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1     : 23;
		uint64_t penalty_fraction : 8;
		uint64_t penalty_num      : 3;
		uint64_t token_num        : 10;
		uint64_t __reserved_0     : 10;
		uint64_t max_count        : 9;
		uint64_t en               : 1;
#endif
	};

	uint64_t word;
} RSH_BND_CTL_1_t;


/*
 * Bandwidth_Control.
 * Limit the bandwidth between the node and the mesh. There are four
 * bandwidth control registers, which respectively control WRITE-CDN,
 * WRITE-DDN, READ-CDN and READ-DDN channel
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* When set, bandwidth control module is enabled */
		uint64_t en               : 1;
		/* The max value of buckets in the bandwidth control module */
		uint64_t max_count        : 9;
		/* Reserved. */
		uint64_t __reserved_0     : 10;
		/*
		 * Token is expressed as fraction format, token per cycle is
		 * equal to TOKEN_NUM/1023
		 */
		uint64_t token_num        : 10;
		/* Extra penalty added for each write/read transcation */
		uint64_t penalty_num      : 3;
		/*
		 * Control the ratio of reads with extra write/read penalty.
		 * The penalty ratio is equal to PENALTY_FRACATION/256
		 */
		uint64_t penalty_fraction : 8;
		/* Reserved. */
		uint64_t __reserved_1     : 23;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1     : 23;
		uint64_t penalty_fraction : 8;
		uint64_t penalty_num      : 3;
		uint64_t token_num        : 10;
		uint64_t __reserved_0     : 10;
		uint64_t max_count        : 9;
		uint64_t en               : 1;
#endif
	};

	uint64_t word;
} RSH_BND_CTL_2_t;


/*
 * Bandwidth_Control.
 * Limit the bandwidth between the node and the mesh. There are four
 * bandwidth control registers, which respectively control WRITE-CDN,
 * WRITE-DDN, READ-CDN and READ-DDN channel
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* When set, bandwidth control module is enabled */
		uint64_t en               : 1;
		/* The max value of buckets in the bandwidth control module */
		uint64_t max_count        : 9;
		/* Reserved. */
		uint64_t __reserved_0     : 10;
		/*
		 * Token is expressed as fraction format, token per cycle is
		 * equal to TOKEN_NUM/1023
		 */
		uint64_t token_num        : 10;
		/* Extra penalty added for each write/read transcation */
		uint64_t penalty_num      : 3;
		/*
		 * Control the ratio of reads with extra write/read penalty.
		 * The penalty ratio is equal to PENALTY_FRACATION/256
		 */
		uint64_t penalty_fraction : 8;
		/* Reserved. */
		uint64_t __reserved_1     : 23;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1     : 23;
		uint64_t penalty_fraction : 8;
		uint64_t penalty_num      : 3;
		uint64_t token_num        : 10;
		uint64_t __reserved_0     : 10;
		uint64_t max_count        : 9;
		uint64_t en               : 1;
#endif
	};

	uint64_t word;
} RSH_BND_CTL_3_t;


/*
 * BandWidth Throttle Control.
 * Control Bandwidth throttle algorithm behavior.There are four bandwidth
 * throttle control registers, which respectively control WRITE-CDN,
 * WRITE-DDN, READ-CDN and READ-DDN channel
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* When set, bandwidth throttle algorithm is enabled */
		uint64_t en         : 1;
		/*
		 * The threshold used for determing whether the mesh is
		 * congested or not
		 */
		uint64_t cong_thres : 15;
		/*
		 * Maximum possible value of Bandwith throttle module output
		 */
		uint64_t max        : 10;
		/*
		 * Minimum possible value of Bandwith throttle module output
		 */
		uint64_t min        : 10;
		/*
		 * The ratio between decrement amout and maximum value is
		 * 1/(1 << DEC_PER)
		 */
		uint64_t dec_per    : 5;
		/*
		 * The ratio between increment amout and maximum value is
		 * 1/(1 << INC_PER)
		 */
		uint64_t inc_per    : 5;
		/*
		 * The number of cycles in one monitor window of the algorithm
		 */
		uint64_t win_len    : 18;
#else   /* __BIG_ENDIAN__ */
		uint64_t win_len    : 18;
		uint64_t inc_per    : 5;
		uint64_t dec_per    : 5;
		uint64_t min        : 10;
		uint64_t max        : 10;
		uint64_t cong_thres : 15;
		uint64_t en         : 1;
#endif
	};

	uint64_t word;
} RSH_BND_THRT_CTL_t;


/*
 * BandWidth Throttle Control.
 * Control Bandwidth throttle algorithm behavior.There are four bandwidth
 * throttle control registers, which respectively control WRITE-CDN,
 * WRITE-DDN, READ-CDN and READ-DDN channel
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* When set, bandwidth throttle algorithm is enabled */
		uint64_t en         : 1;
		/*
		 * The threshold used for determing whether the mesh is
		 * congested or not
		 */
		uint64_t cong_thres : 15;
		/*
		 * Maximum possible value of Bandwith throttle module output
		 */
		uint64_t max        : 10;
		/*
		 * Minimum possible value of Bandwith throttle module output
		 */
		uint64_t min        : 10;
		/*
		 * The ratio between decrement amout and maximum value is
		 * 1/(1 << DEC_PER)
		 */
		uint64_t dec_per    : 5;
		/*
		 * The ratio between increment amout and maximum value is
		 * 1/(1 << INC_PER)
		 */
		uint64_t inc_per    : 5;
		/*
		 * The number of cycles in one monitor window of the algorithm
		 */
		uint64_t win_len    : 18;
#else   /* __BIG_ENDIAN__ */
		uint64_t win_len    : 18;
		uint64_t inc_per    : 5;
		uint64_t dec_per    : 5;
		uint64_t min        : 10;
		uint64_t max        : 10;
		uint64_t cong_thres : 15;
		uint64_t en         : 1;
#endif
	};

	uint64_t word;
} RSH_BND_THRT_CTL_1_t;


/*
 * BandWidth Throttle Control.
 * Control Bandwidth throttle algorithm behavior.There are four bandwidth
 * throttle control registers, which respectively control WRITE-CDN,
 * WRITE-DDN, READ-CDN and READ-DDN channel
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* When set, bandwidth throttle algorithm is enabled */
		uint64_t en         : 1;
		/*
		 * The threshold used for determing whether the mesh is
		 * congested or not
		 */
		uint64_t cong_thres : 15;
		/*
		 * Maximum possible value of Bandwith throttle module output
		 */
		uint64_t max        : 10;
		/*
		 * Minimum possible value of Bandwith throttle module output
		 */
		uint64_t min        : 10;
		/*
		 * The ratio between decrement amout and maximum value is
		 * 1/(1 << DEC_PER)
		 */
		uint64_t dec_per    : 5;
		/*
		 * The ratio between increment amout and maximum value is
		 * 1/(1 << INC_PER)
		 */
		uint64_t inc_per    : 5;
		/*
		 * The number of cycles in one monitor window of the algorithm
		 */
		uint64_t win_len    : 18;
#else   /* __BIG_ENDIAN__ */
		uint64_t win_len    : 18;
		uint64_t inc_per    : 5;
		uint64_t dec_per    : 5;
		uint64_t min        : 10;
		uint64_t max        : 10;
		uint64_t cong_thres : 15;
		uint64_t en         : 1;
#endif
	};

	uint64_t word;
} RSH_BND_THRT_CTL_2_t;


/*
 * BandWidth Throttle Control.
 * Control Bandwidth throttle algorithm behavior.There are four bandwidth
 * throttle control registers, which respectively control WRITE-CDN,
 * WRITE-DDN, READ-CDN and READ-DDN channel
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* When set, bandwidth throttle algorithm is enabled */
		uint64_t en         : 1;
		/*
		 * The threshold used for determing whether the mesh is
		 * congested or not
		 */
		uint64_t cong_thres : 15;
		/*
		 * Maximum possible value of Bandwith throttle module output
		 */
		uint64_t max        : 10;
		/*
		 * Minimum possible value of Bandwith throttle module output
		 */
		uint64_t min        : 10;
		/*
		 * The ratio between decrement amout and maximum value is
		 * 1/(1 << DEC_PER)
		 */
		uint64_t dec_per    : 5;
		/*
		 * The ratio between increment amout and maximum value is
		 * 1/(1 << INC_PER)
		 */
		uint64_t inc_per    : 5;
		/*
		 * The number of cycles in one monitor window of the algorithm
		 */
		uint64_t win_len    : 18;
#else   /* __BIG_ENDIAN__ */
		uint64_t win_len    : 18;
		uint64_t inc_per    : 5;
		uint64_t dec_per    : 5;
		uint64_t min        : 10;
		uint64_t max        : 10;
		uint64_t cong_thres : 15;
		uint64_t en         : 1;
#endif
	};

	uint64_t word;
} RSH_BND_THRT_CTL_3_t;


/*
 * Generic performance module configuration register.
 * Set up the configuration of generic performance module
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Write(1) or read(0) control */
		uint64_t wr_rd_b    : 1;
		/* access strobe (pulse) */
		uint64_t strobe     : 1;
		/* Configuration access address offset */
		uint64_t addr       : 3;
		/* Configuration write data */
		uint64_t wdata      : 56;
		/* Reserved. */
		uint64_t __reserved : 3;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 3;
		uint64_t wdata      : 56;
		uint64_t addr       : 3;
		uint64_t strobe     : 1;
		uint64_t wr_rd_b    : 1;
#endif
	};

	uint64_t word;
} RSH_GEN_PERFMON_CONFIG_t;


/*
 * Generic performance module configuration register.
 * Set up the configuration of generic performance module
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Write(1) or read(0) control */
		uint64_t wr_rd_b    : 1;
		/* access strobe (pulse) */
		uint64_t strobe     : 1;
		/* Configuration access address offset */
		uint64_t addr       : 3;
		/* Configuration write data */
		uint64_t wdata      : 56;
		/* Reserved. */
		uint64_t __reserved : 3;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 3;
		uint64_t wdata      : 56;
		uint64_t addr       : 3;
		uint64_t strobe     : 1;
		uint64_t wr_rd_b    : 1;
#endif
	};

	uint64_t word;
} RSH_GEN_PERFMON_CONFIG_1_t;


/*
 * Generic performance module configuration register.
 * Set up the configuration of generic performance module
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Write(1) or read(0) control */
		uint64_t wr_rd_b    : 1;
		/* access strobe (pulse) */
		uint64_t strobe     : 1;
		/* Configuration access address offset */
		uint64_t addr       : 3;
		/* Configuration write data */
		uint64_t wdata      : 56;
		/* Reserved. */
		uint64_t __reserved : 3;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 3;
		uint64_t wdata      : 56;
		uint64_t addr       : 3;
		uint64_t strobe     : 1;
		uint64_t wr_rd_b    : 1;
#endif
	};

	uint64_t word;
} RSH_GEN_PERFMON_CONFIG_2_t;


/*
 * Generic performance module configuration register.
 * Set up the configuration of generic performance module
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Write(1) or read(0) control */
		uint64_t wr_rd_b    : 1;
		/* access strobe (pulse) */
		uint64_t strobe     : 1;
		/* Configuration access address offset */
		uint64_t addr       : 3;
		/* Configuration write data */
		uint64_t wdata      : 56;
		/* Reserved. */
		uint64_t __reserved : 3;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 3;
		uint64_t wdata      : 56;
		uint64_t addr       : 3;
		uint64_t strobe     : 1;
		uint64_t wr_rd_b    : 1;
#endif
	};

	uint64_t word;
} RSH_GEN_PERFMON_CONFIG_3_t;


/*
 * Generic performance value register.
 * The register is used to provide interface where software can read the
 * value of generic performance counter
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Selected generic performance register's value */
		uint64_t val : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t val : 64;
#endif
	};

	uint64_t word;
} RSH_GEN_PERFMVAL_t;


/*
 * Generic performance value register.
 * The register is used to provide interface where software can read the
 * value of generic performance counter
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Selected generic performance register's value */
		uint64_t val : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t val : 64;
#endif
	};

	uint64_t word;
} RSH_GEN_PERFMVAL_1_t;


/*
 * Generic performance value register.
 * The register is used to provide interface where software can read the
 * value of generic performance counter
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Selected generic performance register's value */
		uint64_t val : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t val : 64;
#endif
	};

	uint64_t word;
} RSH_GEN_PERFMVAL_2_t;


/*
 * Generic performance value register.
 * The register is used to provide interface where software can read the
 * value of generic performance counter
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Selected generic performance register's value */
		uint64_t val : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t val : 64;
#endif
	};

	uint64_t word;
} RSH_GEN_PERFMVAL_3_t;


/*
 * Performance Control Register.
 * The performance control register holds global and per accumulator control
 * fields defining the operation of the module on 0/1 data paths
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Filter Match options for accumulator 1 */
		uint64_t fm1          : 3;
		/* Reserved. */
		uint64_t __reserved_0 : 1;
		/*
		 * PERFACC1 match selection for accumulation trigger. This
		 * field is ignored if ACCM1 selects free runing counter mode
		 */
		uint64_t ms1          : 2;
		/* Reserved. */
		uint64_t __reserved_1 : 2;
		/*
		 * Accumulator 1 Mode. This field controls accumulator 1
		 * operation
		 */
		uint64_t accm1        : 3;
		/* Auto-Disable Accumulator 1 */
		uint64_t ad1          : 1;
		/*
		 * Event trigger for PERFACC1 path. This field is ignored if
		 * ACCM1=1
		 */
		uint64_t etrig1       : 2;
		/* Enable Both for accumulator 1 */
		uint64_t eb1          : 1;
		/*
		 * Enable performance accumulator 1 (PERFACC1) or both
		 * accumulators (PERFACC0, PERFACC1), depending on EB1 value.
		 */
		uint64_t en1          : 1;
		/* Filter Match options for accumulator 0 */
		uint64_t fm0          : 3;
		/* Reserved. */
		uint64_t __reserved_2 : 1;
		/*
		 * PERFACC0 match selection for accumulation trigger. This
		 * field is ignored if ACCM0 selects free runing counter mode
		 */
		uint64_t ms0          : 2;
		/* Reserved. */
		uint64_t __reserved_3 : 2;
		/*
		 * Accumulator 0 Mode. This field control accumulator 0
		 * operation
		 */
		uint64_t accm0        : 3;
		/* Auto-Disable Accumulator 0 */
		uint64_t ad0          : 1;
		/*
		 * Event trigger for PERFACC0 path. This field is ignored if
		 * ACCM0=1
		 */
		uint64_t etrig0       : 2;
		/* Enable Both for accumulator 0 */
		uint64_t eb0          : 1;
		/*
		 * Enable performance accumulator 0(PERFACC0) or both
		 * accumulators(PERFACC0, PERFACC1), depending on EB0 value.
		 */
		uint64_t en0          : 1;
		/* Reserved. */
		uint64_t __reserved_4 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_4 : 32;
		uint64_t en0          : 1;
		uint64_t eb0          : 1;
		uint64_t etrig0       : 2;
		uint64_t ad0          : 1;
		uint64_t accm0        : 3;
		uint64_t __reserved_3 : 2;
		uint64_t ms0          : 2;
		uint64_t __reserved_2 : 1;
		uint64_t fm0          : 3;
		uint64_t en1          : 1;
		uint64_t eb1          : 1;
		uint64_t etrig1       : 2;
		uint64_t ad1          : 1;
		uint64_t accm1        : 3;
		uint64_t __reserved_1 : 2;
		uint64_t ms1          : 2;
		uint64_t __reserved_0 : 1;
		uint64_t fm1          : 3;
#endif
	};

	uint64_t word;
} RSH_PERFCTL_t;


/*
 * Performance Event Selection Register.
 * The performance event selection register controls multiplexing logic at
 * the block level
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 16;
		/*
		 * Performance value selection from a group associated with
		 * EVTSEL.
		 */
		uint64_t pvalsel      : 4;
		/*
		 * Module select. This optional field is used in case the
		 * performance module collects performance data from
		 * additional other blocks
		 */
		uint64_t modsel       : 4;
		/* Event select signal */
		uint64_t evtsel       : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 32;
		uint64_t evtsel       : 8;
		uint64_t modsel       : 4;
		uint64_t pvalsel      : 4;
		uint64_t __reserved_0 : 16;
#endif
	};

	uint64_t word;
} RSH_PERFEVT_t;


/*
 * Performance Value Extraction Register.
 * The performance value extraction register extracts two 56-bit values out
 * of PVAL with bitwise offset and width control
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Value 1 LS bit offset within PVAL, indicating the
		 * right-shift amount from PVAL.
		 */
		uint64_t vofs1        : 7;
		/* Reserved. */
		uint64_t __reserved_0 : 1;
		/* Value 1 length */
		uint64_t vlen1        : 6;
		/* Reserved. */
		uint64_t __reserved_1 : 2;
		/*
		 * Value 0 LS bit offset within PVAL, indicating the
		 * right-shift amount from PVAL.
		 */
		uint64_t vofs0        : 7;
		/* Reserved. */
		uint64_t __reserved_2 : 1;
		/* Value 0 length */
		uint64_t vlen0        : 6;
		/* Reserved. */
		uint64_t __reserved_3 : 34;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_3 : 34;
		uint64_t vlen0        : 6;
		uint64_t __reserved_2 : 1;
		uint64_t vofs0        : 7;
		uint64_t __reserved_1 : 2;
		uint64_t vlen1        : 6;
		uint64_t __reserved_0 : 1;
		uint64_t vofs1        : 7;
#endif
	};

	uint64_t word;
} RSH_PERFVALEXT_t;


/*
 * Performance Accumulator 0 Register.
 * This register enable both reading and updating the 32-bit value
 * accumulated by accumulator 0.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Accumulator 0 value */
		uint64_t acc        : 56;
		/* Reserved. */
		uint64_t __reserved : 8;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 8;
		uint64_t acc        : 56;
#endif
	};

	uint64_t word;
} RSH_PERFACC0_t;


/*
 * Performance Accumulator 1 Register.
 * This register enable both reading and updating the 32-bit value
 * accumulated by accumulator 1.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Accumulator 1 value */
		uint64_t acc        : 56;
		/* Reserved. */
		uint64_t __reserved : 8;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 8;
		uint64_t acc        : 56;
#endif
	};

	uint64_t word;
} RSH_PERFACC1_t;


/*
 * Performance Match Value 0 Register.
 * This register provides up to 32-bit match to be compared with extracted
 * value 0 according to PERFCTL register configurations.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Match value to be compared against extracted value 0 */
		uint64_t mval       : 32;
		/* Reserved. */
		uint64_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 32;
		uint64_t mval       : 32;
#endif
	};

	uint64_t word;
} RSH_PERFMVAL0_t;


/*
 * Performance Match Value 1 Register.
 * This register provides up to 32-bit match to be compared with extracted
 * value 1 according to PERFCTL register configurations.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Match value to be compared against extracted value 1 */
		uint64_t mval       : 32;
		/* Reserved. */
		uint64_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 32;
		uint64_t mval       : 32;
#endif
	};

	uint64_t word;
} RSH_PERFMVAL1_t;


/*
 * Rev ID.
 * Tile Revision
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Provides architectural revision of the tile. */
		uint64_t tile_rev_id : 8;
		/*
		 * Chip revision.  The low 4 bits indicate the minor
		 * revision; the high 4 bits indicate the major revision.
		 */
		uint64_t chip_rev_id : 8;
		/* Reserved. */
		uint64_t __reserved  : 48;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved  : 48;
		uint64_t chip_rev_id : 8;
		uint64_t tile_rev_id : 8;
#endif
	};

	uint64_t word;
} RSH_REV_ID_t;


/*
 * Fabric Dimensions.
 * Indicates the size of the Tile Fabric and the location of the rshim.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Indicates the number of Tiles in the Y dimension. */
		uint64_t dim_y      : 4;
		/* Indicates the number of Tiles in the X dimension. */
		uint64_t dim_x      : 4;
		/*
		 * Indicates the location of rshim on the mesh.  If this
		 * register returns 0x00, the rshim location is 1,-1 (or 0x1f
		 * as an encoded mesh coordinate).
		 */
		uint64_t rshim_loc  : 8;
		/*
		 * This field returns the number of cores per Tile (minus 1).
		 * Thus 0 means 1 core per Tile, 1 means 2 cores per Tile etc.
		 */
		uint64_t dim_z      : 4;
		/* Reserved. */
		uint64_t __reserved : 44;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 44;
		uint64_t dim_z      : 4;
		uint64_t rshim_loc  : 8;
		uint64_t dim_x      : 4;
		uint64_t dim_y      : 4;
#endif
	};

	uint64_t word;
} RSH_FABRIC_DIM_t;


/*
 * Error Status.
 * Indicators for various fatal and non-fatal RSH error conditions
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Illegal opcode received on MMIO interface */
		uint64_t mmio_ill_opc : 1;
		/* Reserved. */
		uint64_t __reserved   : 63;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved   : 63;
		uint64_t mmio_ill_opc : 1;
#endif
	};

	uint64_t word;
} RSH_ERROR_STATUS_t;


/*
 * MMIO Error Information.
 * Provides diagnostics information when an MMIO error occurs.  Captured
 * whenever the MMIO_ERR interrupt condition occurs (typically due to size
 * error).
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Source Tile in {x[3:0],y[3:0]} format. */
		uint64_t src        : 8;
		/*
		 * Request size.  0=1B, 1=2B, 2=4B, 3=8B, 4=16B, 5=32B,
		 * 6=48B, 7=64B.
		 */
		uint64_t size       : 4;
		/* Full PA from request. */
		uint64_t pa         : 40;
		/*
		 * Opcode of request.  MMIO supports only MMIO_READ (0x0e)
		 * and MMIO_WRITE (0x0f).  All others are reserved and will
		 * only occur on a misconfigured TLB.
		 */
		uint64_t opc        : 5;
		/* Reserved. */
		uint64_t __reserved : 7;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 7;
		uint64_t opc        : 5;
		uint64_t pa         : 40;
		uint64_t size       : 4;
		uint64_t src        : 8;
#endif
	};

	uint64_t word;
} RSH_MMIO_ERROR_INFO_t;


/* Down Count Current Value. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Current value for the down counter.  Writable by SW.
		 * Decremented by HW based on setup in control register.
		 * When counter is 0 and timer tick occurs, an interrupt is
		 * signaled.
		 */
		uint64_t count      : 48;
		/* Reserved. */
		uint64_t __reserved : 16;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 16;
		uint64_t count      : 48;
#endif
	};

	uint64_t word;
} RSH_DOWN_COUNT_VALUE_t;


/* Down Count Current Value. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Current value for the down counter.  Writable by SW.
		 * Decremented by HW based on setup in control register.
		 * When counter is 0 and timer tick occurs, an interrupt is
		 * signaled.
		 */
		uint64_t count      : 48;
		/* Reserved. */
		uint64_t __reserved : 16;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 16;
		uint64_t count      : 48;
#endif
	};

	uint64_t word;
} RSH_DOWN_COUNT_VALUE_1_t;


/* Down Count Current Value. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Current value for the down counter.  Writable by SW.
		 * Decremented by HW based on setup in control register.
		 * When counter is 0 and timer tick occurs, an interrupt is
		 * signaled.
		 */
		uint64_t count      : 48;
		/* Reserved. */
		uint64_t __reserved : 16;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 16;
		uint64_t count      : 48;
#endif
	};

	uint64_t word;
} RSH_DOWN_COUNT_VALUE_2_t;


/* Down Count Refresh Value. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When COUNT reaches 0, this value is loaded at the next
		 * time interval (defined in the control register).
		 */
		uint64_t refresh    : 48;
		/* Reserved. */
		uint64_t __reserved : 16;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 16;
		uint64_t refresh    : 48;
#endif
	};

	uint64_t word;
} RSH_DOWN_COUNT_REFRESH_VALUE_t;


/* Down Count Refresh Value. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When COUNT reaches 0, this value is loaded at the next
		 * time interval (defined in the control register).
		 */
		uint64_t refresh    : 48;
		/* Reserved. */
		uint64_t __reserved : 16;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 16;
		uint64_t refresh    : 48;
#endif
	};

	uint64_t word;
} RSH_DOWN_COUNT_REFRESH_VALUE_1_t;


/* Down Count Refresh Value. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When COUNT reaches 0, this value is loaded at the next
		 * time interval (defined in the control register).
		 */
		uint64_t refresh    : 48;
		/* Reserved. */
		uint64_t __reserved : 16;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 16;
		uint64_t refresh    : 48;
#endif
	};

	uint64_t word;
} RSH_DOWN_COUNT_REFRESH_VALUE_2_t;


/* Down Count Control. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When 0, the current value is frozen and no interrupts will
		 * be signaled.
		 */
		uint64_t ena          : 1;
		/* Clock mode for the down counter. */
		uint64_t mode         : 2;
		/*
		 * Pin select for external clocking.  Can source 1 of 3 pins.
		 *  This setting only matters if MODE is nonzero.  The
		 * minimum pulse width supported on these pins is 10 ns (50
		 * MHz with a 50% duty cycle).
		 */
		uint64_t pinsel       : 2;
		/* Reserved. */
		uint64_t __reserved_0 : 7;
		/*
		 * Clock divisor.  Number of additional ticks of clock source
		 * to trigger a down count.  Setting to 0 causes the down
		 * counter to decrement every cycle.  Setting to 1 causes the
		 * down counter to decrement every other cycle.  To convert a
		 * 10 MHz source to a 100 KHz down count, for example, the
		 * value would be set to 99(d).
		 */
		uint64_t div          : 20;
		/* Reserved. */
		uint64_t __reserved_1 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 32;
		uint64_t div          : 20;
		uint64_t __reserved_0 : 7;
		uint64_t pinsel       : 2;
		uint64_t mode         : 2;
		uint64_t ena          : 1;
#endif
	};

	uint64_t word;
} RSH_DOWN_COUNT_CONTROL_t;


/* Down Count Control. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When 0, the current value is frozen and no interrupts will
		 * be signaled.
		 */
		uint64_t ena          : 1;
		/* Clock mode for the down counter. */
		uint64_t mode         : 2;
		/*
		 * Pin select for external clocking.  Can source 1 of 3 pins.
		 *  This setting only matters if MODE is nonzero.  The
		 * minimum pulse width supported on these pins is 10 ns (50
		 * MHz with a 50% duty cycle).
		 */
		uint64_t pinsel       : 2;
		/* Reserved. */
		uint64_t __reserved_0 : 7;
		/*
		 * Clock divisor.  Number of additional ticks of clock source
		 * to trigger a down count.  Setting to 0 causes the down
		 * counter to decrement every cycle.  Setting to 1 causes the
		 * down counter to decrement every other cycle.  To convert a
		 * 10 MHz source to a 100 KHz down count, for example, the
		 * value would be set to 99(d).
		 */
		uint64_t div          : 20;
		/* Reserved. */
		uint64_t __reserved_1 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 32;
		uint64_t div          : 20;
		uint64_t __reserved_0 : 7;
		uint64_t pinsel       : 2;
		uint64_t mode         : 2;
		uint64_t ena          : 1;
#endif
	};

	uint64_t word;
} RSH_DOWN_COUNT_CONTROL_1_t;


/* Down Count Control. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When 0, the current value is frozen and no interrupts will
		 * be signaled.
		 */
		uint64_t ena          : 1;
		/* Clock mode for the down counter. */
		uint64_t mode         : 2;
		/*
		 * Pin select for external clocking.  Can source 1 of 3 pins.
		 *  This setting only matters if MODE is nonzero.  The
		 * minimum pulse width supported on these pins is 10 ns (50
		 * MHz with a 50% duty cycle).
		 */
		uint64_t pinsel       : 2;
		/* Reserved. */
		uint64_t __reserved_0 : 7;
		/*
		 * Clock divisor.  Number of additional ticks of clock source
		 * to trigger a down count.  Setting to 0 causes the down
		 * counter to decrement every cycle.  Setting to 1 causes the
		 * down counter to decrement every other cycle.  To convert a
		 * 10 MHz source to a 100 KHz down count, for example, the
		 * value would be set to 99(d).
		 */
		uint64_t div          : 20;
		/* Reserved. */
		uint64_t __reserved_1 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 32;
		uint64_t div          : 20;
		uint64_t __reserved_0 : 7;
		uint64_t pinsel       : 2;
		uint64_t mode         : 2;
		uint64_t ena          : 1;
#endif
	};

	uint64_t word;
} RSH_DOWN_COUNT_CONTROL_2_t;


/*
 * Power Threshold.
 * Control dynamic power management.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Indicates the current value on the PWR_IND pins. */
		uint64_t curr_pwr     : 4;
		/*
		 * When the PWR_IND pins are greater than this value, power
		 * draw is considerred to be HIGH and the core PLL will be
		 * switched to the ALT_CLOCK if ALT_CLOCK_SEL is set to AUTO.
		 */
		uint64_t high         : 4;
		/*
		 * When the PWR_IND pins change to greater than this value,
		 * the PWR_ALARM interrupt will be triggered. The ALARM
		 * interrupt will re-trigger when the value falls to equal
		 * this value.
		 */
		uint64_t alarm        : 4;
		/*
		 * Controls the minimum time to switch from the MAIN clock to
		 * the ALT clock when the PWR_IND pins go above the HIGH
		 * threshold.
		 */
		uint64_t hyst_tmr_a   : 3;
		/* Reserved. */
		uint64_t __reserved_0 : 1;
		/*
		 * Controls the minimum time to switch from the ALT clock to
		 * the MAIN clock when the PWR_IND pins match or go below the
		 * HIGH threshold.
		 */
		uint64_t hyst_tmr_b   : 3;
		/* Reserved. */
		uint64_t __reserved_1 : 45;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 45;
		uint64_t hyst_tmr_b   : 3;
		uint64_t __reserved_0 : 1;
		uint64_t hyst_tmr_a   : 3;
		uint64_t alarm        : 4;
		uint64_t high         : 4;
		uint64_t curr_pwr     : 4;
#endif
	};

	uint64_t word;
} RSH_POWER_THRESHOLD_t;


/*
 * Alternate Clock Selection.
 * Selects the source for the core clock.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t sel        : 2;
		/* Reserved. */
		uint64_t __reserved : 62;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 62;
		uint64_t sel        : 2;
#endif
	};

	uint64_t word;
} RSH_ALT_CLOCK_SEL_t;


/* Reset Control. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When written with the KEY, the chip will be soft reset.
		 * On soft reset, the BREADCRUMB registers will be left
		 * intact allowing reboot software to determine why the chip
		 * was reset.  Using a KEY reduces the likelihood that a
		 * misconfigured boot ROM or other errant write could
		 * accidentally reset the chip.
		 */
		uint64_t reset_chip : 32;
		/*
		 * Used for diagnostics. When set, software writes to
		 * RESET_CHIP or watchdog reset will be blocked from
		 * resetting the ARM complex. The state of the reset request
		 * will be visible in RESET_REQ_PND.
		 */
		uint64_t disable    : 1;
		/*
		 * Used for diagnostics. Indicates that software or a
		 * watchdog timer has requested a reset. Normally this will
		 * reset the device so this bit is not observable unless
		 * RESET_DISABLE is set. Can be cleared by writing with 1.
		 */
		uint64_t req_pnd    : 1;
		/* Reserved. */
		uint64_t __reserved : 30;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 30;
		uint64_t req_pnd    : 1;
		uint64_t disable    : 1;
		uint64_t reset_chip : 32;
#endif
	};

	uint64_t word;
} RSH_RESET_CONTROL_t;


/*
 * Breadcrumb0.
 * Scratchpad register that resets only on hard_rst_l (power on reset).
 * Typically used by software to leave information for the reboot software on
 * software reset.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Breadcrumb0.
		 * Scratchpad register that resets only on hard_rst_l (power
		 * on reset).  Typically used by software to leave
		 * information for the reboot software on software reset.
		 */
		uint64_t breadcrumb0 : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t breadcrumb0 : 64;
#endif
	};

	uint64_t word;
} RSH_BREADCRUMB0_t;


/*
 * Breadcrumb1.
 * Scratchpad register that resets only on hard_rst_l (power on reset).
 * Typically used by software to leave information for the reboot software on
 * software reset.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Breadcrumb1.
		 * Scratchpad register that resets only on hard_rst_l (power
		 * on reset).  Typically used by software to leave
		 * information for the reboot software on software reset.
		 */
		uint64_t breadcrumb1 : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t breadcrumb1 : 64;
#endif
	};

	uint64_t word;
} RSH_BREADCRUMB1_t;


/* Boot Control. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Indicates source for ROM boot on next software reset.
		 * Initialized based on strapping pins on hard_rst_l.  This
		 * register is not reset on software reset.
		 */
		uint64_t boot_mode  : 2;
		/* Reserved. */
		uint64_t __reserved : 62;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 62;
		uint64_t boot_mode  : 2;
#endif
	};

	uint64_t word;
} RSH_BOOT_CONTROL_t;


/* Watchdog Control. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Each bit corresponds to one of the 3 down counters.  When
		 * asserted, if the associated down counter triggers an
		 * interrupt condition, the RESET_CHIP functionality will be
		 * triggered automatically (the interrupt binding doesn't
		 * have to be enabled).
		 */
		uint64_t reset_ena  : 3;
		/* Reserved. */
		uint64_t __reserved : 61;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 61;
		uint64_t reset_ena  : 3;
#endif
	};

	uint64_t word;
} RSH_WATCHDOG_CONTROL_t;


/*
 * Device Protection.
 * Controls device access to rshim services.  When an access is blocked,
 * reads and writes will acknowledged but ignored.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Disable UART0. */
		uint64_t disable_dev_uart0     : 1;
		/* Disable UART1. */
		uint64_t disable_dev_uart1     : 1;
		/* Disable DIAG_UART. */
		uint64_t disable_dev_diag_uart : 1;
		/* Disable TYU. */
		uint64_t disable_dev_tyu       : 1;
		/* Disable TYU_EXT1. */
		uint64_t disable_dev_tyu_ext1  : 1;
		/* Disable TYU_EXT2. */
		uint64_t disable_dev_tyu_ext2  : 1;
		/* Disable TYU_EXT3. */
		uint64_t disable_dev_tyu_ext3  : 1;
		/* Disable TIMER. */
		uint64_t disable_dev_timer     : 1;
		/* Disable USB. */
		uint64_t disable_dev_usb       : 1;
		/* Disable GPIO. */
		uint64_t disable_dev_gpio      : 1;
		/* Disable MMC. */
		uint64_t disable_dev_mmc       : 1;
		/* Disable TIMER_EXT. */
		uint64_t disable_dev_timer_ext : 1;
		/* Disable WDOG_NS. */
		uint64_t disable_dev_wdog_ns   : 1;
		/* Disable WDOG_SEC. */
		uint64_t disable_dev_wdog_sec  : 1;
		/* Reserved. */
		uint64_t __reserved_0          : 18;
		/* Disable TRIO0. */
		uint64_t disable_rci_trio0     : 1;
		/* Disable TRIO1. */
		uint64_t disable_rci_trio1     : 1;
		/* Disable TRIO2. */
		uint64_t disable_rci_trio2     : 1;
		/* Disable USB. */
		uint64_t disable_rci_usb       : 1;
		/* Reserved. */
		uint64_t __reserved_1          : 28;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1          : 28;
		uint64_t disable_rci_usb       : 1;
		uint64_t disable_rci_trio2     : 1;
		uint64_t disable_rci_trio1     : 1;
		uint64_t disable_rci_trio0     : 1;
		uint64_t __reserved_0          : 18;
		uint64_t disable_dev_wdog_sec  : 1;
		uint64_t disable_dev_wdog_ns   : 1;
		uint64_t disable_dev_timer_ext : 1;
		uint64_t disable_dev_mmc       : 1;
		uint64_t disable_dev_gpio      : 1;
		uint64_t disable_dev_usb       : 1;
		uint64_t disable_dev_timer     : 1;
		uint64_t disable_dev_tyu_ext3  : 1;
		uint64_t disable_dev_tyu_ext2  : 1;
		uint64_t disable_dev_tyu_ext1  : 1;
		uint64_t disable_dev_tyu       : 1;
		uint64_t disable_dev_diag_uart : 1;
		uint64_t disable_dev_uart1     : 1;
		uint64_t disable_dev_uart0     : 1;
#endif
	};

	uint64_t word;
} RSH_DEVICE_PROTECTION_t;


/*
 * Device Security Level.
 * Controls device access to rshim registers based on current security level.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Device security level for UART0. */
		uint64_t dev_lvl_uart0     : 2;
		/* Device security level for UART1. */
		uint64_t dev_lvl_uart1     : 2;
		/* Device security level for DIAG_UART. */
		uint64_t dev_lvl_diag_uart : 2;
		/* Device security level for TYU. */
		uint64_t dev_lvl_tyu       : 2;
		/* Device security level for TYU_EXT1. */
		uint64_t dev_lvl_tyu_ext1  : 2;
		/* Device security level for TYU_EXT2. */
		uint64_t dev_lvl_tyu_ext2  : 2;
		/* Device security level for TYU_EXT3. */
		uint64_t dev_lvl_tyu_ext3  : 2;
		/* Device security level for TIMER. */
		uint64_t dev_lvl_timer     : 2;
		/* Device security level for USB. */
		uint64_t dev_lvl_usb       : 2;
		/* Device security level for GPIO. */
		uint64_t dev_lvl_gpio      : 2;
		/* Device security level for MMC. */
		uint64_t dev_lvl_mmc       : 2;
		/* Device security level for TIMER_EXT. */
		uint64_t dev_lvl_timer_ext : 2;
		/* Device security level for WDOG_NS. */
		uint64_t dev_lvl_wdog_ns   : 2;
		/* Device security level for WDOG_SEC. */
		uint64_t dev_lvl_wdog_sec  : 2;
		/* Reserved. */
		uint64_t __reserved_0      : 4;
		/* RCI security level for TRIO0. */
		uint64_t rci_lvl_trio0     : 2;
		/* RCI security level for TRIO1. */
		uint64_t rci_lvl_trio1     : 2;
		/* RCI security level for TRIO2. */
		uint64_t rci_lvl_trio2     : 2;
		/* RCI security level for USB. */
		uint64_t rci_lvl_usb       : 2;
		/* Reserved. */
		uint64_t __reserved_1      : 8;
		/*
		 * Security level of mem-access widget. When 1, the memory
		 * transactions will use NS=0. When 0, memory transactions
		 * will use NS=1.
		 */
		uint64_t mem_acc_lvl       : 1;
		/* Reserved. */
		uint64_t __reserved_2      : 15;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2      : 15;
		uint64_t mem_acc_lvl       : 1;
		uint64_t __reserved_1      : 8;
		uint64_t rci_lvl_usb       : 2;
		uint64_t rci_lvl_trio2     : 2;
		uint64_t rci_lvl_trio1     : 2;
		uint64_t rci_lvl_trio0     : 2;
		uint64_t __reserved_0      : 4;
		uint64_t dev_lvl_wdog_sec  : 2;
		uint64_t dev_lvl_wdog_ns   : 2;
		uint64_t dev_lvl_timer_ext : 2;
		uint64_t dev_lvl_mmc       : 2;
		uint64_t dev_lvl_gpio      : 2;
		uint64_t dev_lvl_usb       : 2;
		uint64_t dev_lvl_timer     : 2;
		uint64_t dev_lvl_tyu_ext3  : 2;
		uint64_t dev_lvl_tyu_ext2  : 2;
		uint64_t dev_lvl_tyu_ext1  : 2;
		uint64_t dev_lvl_tyu       : 2;
		uint64_t dev_lvl_diag_uart : 2;
		uint64_t dev_lvl_uart1     : 2;
		uint64_t dev_lvl_uart0     : 2;
#endif
	};

	uint64_t word;
} RSH_DEVICE_SEC_LVL_t;


/*
 * Device Slave Security Level.
 * Controls the security levels for all of the registers within each slave
 * device.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Register security level for UART0. */
		uint64_t reg_lvl_uart0     : 2;
		/* Register security level for UART1. */
		uint64_t reg_lvl_uart1     : 2;
		/* Register security level for DIAG_UART. */
		uint64_t reg_lvl_diag_uart : 2;
		/* Register security level for TYU. */
		uint64_t reg_lvl_tyu       : 2;
		/* Register security level for TYU_EXT1. */
		uint64_t reg_lvl_tyu_ext1  : 2;
		/* Register security level for TYU_EXT2. */
		uint64_t reg_lvl_tyu_ext2  : 2;
		/* Register security level for TYU_EXT3. */
		uint64_t reg_lvl_tyu_ext3  : 2;
		/* Register security level for TIMER. */
		uint64_t reg_lvl_timer     : 2;
		/* Register security level for USB. */
		uint64_t reg_lvl_usb       : 2;
		/* Register security level for GPIO. */
		uint64_t reg_lvl_gpio      : 2;
		/* Register security level for MMC. */
		uint64_t reg_lvl_mmc       : 2;
		/* Register security level for TIMER_EXT. */
		uint64_t reg_lvl_timer_ext : 2;
		/* Register security level for WDOG_NS. */
		uint64_t reg_lvl_wdog_ns   : 2;
		/* Register security level for WDOG_SEC. */
		uint64_t reg_lvl_wdog_sec  : 2;
		/* Reserved. */
		uint64_t __reserved        : 36;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved        : 36;
		uint64_t reg_lvl_wdog_sec  : 2;
		uint64_t reg_lvl_wdog_ns   : 2;
		uint64_t reg_lvl_timer_ext : 2;
		uint64_t reg_lvl_mmc       : 2;
		uint64_t reg_lvl_gpio      : 2;
		uint64_t reg_lvl_usb       : 2;
		uint64_t reg_lvl_timer     : 2;
		uint64_t reg_lvl_tyu_ext3  : 2;
		uint64_t reg_lvl_tyu_ext2  : 2;
		uint64_t reg_lvl_tyu_ext1  : 2;
		uint64_t reg_lvl_tyu       : 2;
		uint64_t reg_lvl_diag_uart : 2;
		uint64_t reg_lvl_uart1     : 2;
		uint64_t reg_lvl_uart0     : 2;
#endif
	};

	uint64_t word;
} RSH_DEVICE_SLV_SEC_LVL_t;


/*
 * Scratch buffer control.
 * Controls R/W access to scratch buffer.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Index into the 128 word scratch buffer.   Increments
		 * automatically on write or read to SCRATCH_BUF_DAT.
		 */
		uint64_t idx        : 7;
		/* Reserved. */
		uint64_t __reserved : 57;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 57;
		uint64_t idx        : 7;
#endif
	};

	uint64_t word;
} RSH_SCRATCH_BUF_CTL_t;


/*
 * Scratch buffer data.
 * Read/Write data for scratch buffer
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Scratch buffer data.
		 * Read/Write data for scratch buffer
		 */
		uint64_t scratch_buf_dat : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t scratch_buf_dat : 64;
#endif
	};

	uint64_t word;
} RSH_SCRATCH_BUF_DAT_t;


/*
 * Byte Access Control.
 * Provides 8-byte access to rshim registers using 1B, 2B, or 4B accesses.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Byte-index into the 8-byte wdat/rdat/addr register for
		 * access.  Increments automatically on access to the
		 * BYTE_ACC_WDAT, BYTE_ACC_RDAT, or BYTE_ACC_ADDR registers.
		 */
		uint64_t byte_ptr   : 3;
		/*
		 * Size of access.  This controls how much the BYTE_PTR
		 * increments by on each BYTE_ACC_WDAT/RDAT access.
		 */
		uint64_t size       : 2;
		/*
		 * When asserted, an access is still pending.  Attempting to
		 * access the WDAT/RDAT/ADDR registers when an access is
		 * pending has unpredictable results.
		 */
		uint64_t pending    : 1;
		/*
		 * When written with a 1, a read to BYTE_ACC_ADDR is
		 * triggered and the resulting data is placed into
		 * BYTE_ACC_RDAT.
		 */
		uint64_t read_trig  : 1;
		/* Reserved. */
		uint64_t __reserved : 57;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 57;
		uint64_t read_trig  : 1;
		uint64_t pending    : 1;
		uint64_t size       : 2;
		uint64_t byte_ptr   : 3;
#endif
	};

	uint64_t word;
} RSH_BYTE_ACC_CTL_t;


/*
 * Byte Access Write Data.
 * 8 bytes of write data written when BYTE_ACC_CTL.BYTE_PTR wraps back to
 * zero.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * The 8-bytes in this register are updated based on
		 * BYTE_ACC_CTL.BYTE_PTR and SIZE.  Each time this register
		 * is written, the BYTE_ACC_CTL.BYTE_PTR is incremented based
		 * on BYTE_ACC_CTL.SIZE.  When the most significant byte of
		 * this register is written, the register at address
		 * BYTE_ACC_ADDR is written using the data in this register.
		 */
		uint64_t dat : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t dat : 64;
#endif
	};

	uint64_t word;
} RSH_BYTE_ACC_WDAT_t;


/*
 * Byte Access Read Data.
 * 8 bytes of read data captured when READ_TRIGGER is written with a 1.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Each time this register is read, the BYTE_ACC_CTL.BYTE_PTR
		 * is incremented based on BYTE_ACC_CTL.SIZE.  The bytes
		 * returned will be shifted into the least significant
		 * BYTE_ACC_CTL.SIZE bytes based on the current value of
		 * BYTE_ACC_CTL.BYTE_PTR.
		 */
		uint64_t dat : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t dat : 64;
#endif
	};

	uint64_t word;
} RSH_BYTE_ACC_RDAT_t;


/*
 * Byte Access Register Address.
 * Register address used for read/write accesses triggered by
 * BYTE_ACC_WDAT/RDAT.  The BYTE_ACC_CTL.BYTE_PTR is used and auto
 * incremented when writing or reading this register.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 3;
		/*
		 * This field selects the 8-byte register to be accessed
		 * within the channel specified by CHANNEL.
		 */
		uint64_t regnum       : 13;
		/*
		 * This field of the address selects the channel (device).
		 * Channel-0 is the common rshim registers.  Channels 1-15
		 * are used to access the rshim-attached devices.
		 */
		uint64_t channel      : 4;
		/* Reserved. */
		uint64_t __reserved_1 : 44;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 44;
		uint64_t channel      : 4;
		uint64_t regnum       : 13;
		uint64_t __reserved_0 : 3;
#endif
	};

	uint64_t word;
} RSH_BYTE_ACC_ADDR_t;


/*
 * Boot FIFO.
 * Reads pop one word from the FIFO. If the FIFO is empty, the read will
 * return unpredictable data and the FIFO state will not be affected. Writes
 * push one entry. If full, writes will be dropped (MMIO) or flow-control the
 * associated interface.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Boot FIFO.
		 * Reads pop one word from the FIFO. If the FIFO is empty,
		 * the read will return unpredictable data and the FIFO state
		 * will not be affected. Writes push one entry. If full,
		 * writes will be dropped (MMIO) or flow-control the
		 * associated interface.
		 */
		uint64_t boot_fifo_data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t boot_fifo_data : 64;
#endif
	};

	uint64_t word;
} RSH_BOOT_FIFO_DATA_t;


/*
 * Boot FIFO Count.
 * Indicates the number of words currently in the boot FIFO. The boot FIFO
 * can hold up to 512 8-byte words.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Boot FIFO Count.
		 * Indicates the number of words currently in the boot FIFO.
		 * The boot FIFO can hold up to 512 8-byte words.
		 */
		uint64_t boot_fifo_count : 10;
		/* Reserved. */
		uint64_t __reserved      : 54;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved      : 54;
		uint64_t boot_fifo_count : 10;
#endif
	};

	uint64_t word;
} RSH_BOOT_FIFO_COUNT_t;


/*
 * Memory Access Setup.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Cacheing attributes for transcation */
		uint64_t attr       : 4;
		/* Reserved. */
		uint64_t __reserved : 60;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 60;
		uint64_t attr       : 4;
#endif
	};

	uint64_t word;
} RSH_MEM_ACC_SETUP_t;


/*
 * Memory Access Control.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Physical address for access. The address must be naturally
		 * aligned based on the SIZE being sent. Unaligned transfers
		 * are not supported.
		 */
		uint64_t address      : 40;
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/*
		 * When 1, operation is a write. When 0, operation is a read
		 */
		uint64_t write        : 1;
		/* Reserved. */
		uint64_t __reserved_1 : 3;
		/* Encoded transaction size */
		uint64_t size         : 3;
		/* Reserved. */
		uint64_t __reserved_2 : 8;
		/*
		 * When written with a 1, an access will be triggerred. Bit
		 * will clear once access been sent. The MEM_ACC_RSP_CNT will
		 * increment once the access has completed and a response has
		 * been received.
		 */
		uint64_t send         : 1;
#else   /* __BIG_ENDIAN__ */
		uint64_t send         : 1;
		uint64_t __reserved_2 : 8;
		uint64_t size         : 3;
		uint64_t __reserved_1 : 3;
		uint64_t write        : 1;
		uint64_t __reserved_0 : 8;
		uint64_t address      : 40;
#endif
	};

	uint64_t word;
} RSH_MEM_ACC_CTL_t;


/*
 * Memory Access Data.
 * When MEM_ACC_CTL is used to perform a read, this register contains the
 * result data. On a write, this contains the data to be written. Data must
 * be packed into the LSBs. For example, a 2-byte write will always have an
 * even address and the data will always be in bits [16:0] of the first
 * MEM_ACC_DATA word for both reads and writes. Thus the 2nd MEM_ACC_DATA
 * word is only used for writes larger than 8-bytes etc.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t data : 64;
#endif
	};

	uint64_t word;
} RSH_MEM_ACC_DATA_t;


/*
 * Memory Access Data.
 * When MEM_ACC_CTL is used to perform a read, this register contains the
 * result data. On a write, this contains the data to be written. Data must
 * be packed into the LSBs. For example, a 2-byte write will always have an
 * even address and the data will always be in bits [16:0] of the first
 * MEM_ACC_DATA word for both reads and writes. Thus the 2nd MEM_ACC_DATA
 * word is only used for writes larger than 8-bytes etc.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t data : 64;
#endif
	};

	uint64_t word;
} RSH_MEM_ACC_DATA_1_t;


/*
 * Memory Access Data.
 * When MEM_ACC_CTL is used to perform a read, this register contains the
 * result data. On a write, this contains the data to be written. Data must
 * be packed into the LSBs. For example, a 2-byte write will always have an
 * even address and the data will always be in bits [16:0] of the first
 * MEM_ACC_DATA word for both reads and writes. Thus the 2nd MEM_ACC_DATA
 * word is only used for writes larger than 8-bytes etc.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t data : 64;
#endif
	};

	uint64_t word;
} RSH_MEM_ACC_DATA_2_t;


/*
 * Memory Access Data.
 * When MEM_ACC_CTL is used to perform a read, this register contains the
 * result data. On a write, this contains the data to be written. Data must
 * be packed into the LSBs. For example, a 2-byte write will always have an
 * even address and the data will always be in bits [16:0] of the first
 * MEM_ACC_DATA word for both reads and writes. Thus the 2nd MEM_ACC_DATA
 * word is only used for writes larger than 8-bytes etc.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t data : 64;
#endif
	};

	uint64_t word;
} RSH_MEM_ACC_DATA_3_t;


/*
 * Memory Access Data.
 * When MEM_ACC_CTL is used to perform a read, this register contains the
 * result data. On a write, this contains the data to be written. Data must
 * be packed into the LSBs. For example, a 2-byte write will always have an
 * even address and the data will always be in bits [16:0] of the first
 * MEM_ACC_DATA word for both reads and writes. Thus the 2nd MEM_ACC_DATA
 * word is only used for writes larger than 8-bytes etc.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t data : 64;
#endif
	};

	uint64_t word;
} RSH_MEM_ACC_DATA_4_t;


/*
 * Memory Access Data.
 * When MEM_ACC_CTL is used to perform a read, this register contains the
 * result data. On a write, this contains the data to be written. Data must
 * be packed into the LSBs. For example, a 2-byte write will always have an
 * even address and the data will always be in bits [16:0] of the first
 * MEM_ACC_DATA word for both reads and writes. Thus the 2nd MEM_ACC_DATA
 * word is only used for writes larger than 8-bytes etc.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t data : 64;
#endif
	};

	uint64_t word;
} RSH_MEM_ACC_DATA_5_t;


/*
 * Memory Access Data.
 * When MEM_ACC_CTL is used to perform a read, this register contains the
 * result data. On a write, this contains the data to be written. Data must
 * be packed into the LSBs. For example, a 2-byte write will always have an
 * even address and the data will always be in bits [16:0] of the first
 * MEM_ACC_DATA word for both reads and writes. Thus the 2nd MEM_ACC_DATA
 * word is only used for writes larger than 8-bytes etc.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t data : 64;
#endif
	};

	uint64_t word;
} RSH_MEM_ACC_DATA_6_t;


/*
 * Memory Access Data.
 * When MEM_ACC_CTL is used to perform a read, this register contains the
 * result data. On a write, this contains the data to be written. Data must
 * be packed into the LSBs. For example, a 2-byte write will always have an
 * even address and the data will always be in bits [16:0] of the first
 * MEM_ACC_DATA word for both reads and writes. Thus the 2nd MEM_ACC_DATA
 * word is only used for writes larger than 8-bytes etc.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t data : 64;
#endif
	};

	uint64_t word;
} RSH_MEM_ACC_DATA_7_t;


/*
 * Memory Access Response Count.
 * Counts number of responses received.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t val        : 8;
		/* Reserved. */
		uint64_t __reserved : 56;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 56;
		uint64_t val        : 8;
#endif
	};

	uint64_t word;
} RSH_MEM_ACC_RSP_CNT_t;


/*
 * Boot Record Tile Disable.
 * This register reflects the value of the Boot Record bits used to disable
 * tile clusters and home nodes in the TILE_STATUS register. In BlueField
 * there are a maximum of 8 Tiles; the reserved bits are for future expansion.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * A '1' indicates that the cluster in the Tile with the
		 * corresponding bit position is disabled.
		 */
		uint64_t cluster      : 8;
		/* Reserved. */
		uint64_t __reserved_0 : 24;
		/*
		 * A '1' indicates that the Home Node in the Tile with the
		 * corresponding bit position is disabled.
		 */
		uint64_t hnf          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t hnf          : 8;
		uint64_t __reserved_0 : 24;
		uint64_t cluster      : 8;
#endif
	};

	uint64_t word;
} RSH_BOOT_RECORD_TILE_DISABLE_t;


/*
 * eFuse Tile Disable.
 * This register reflects the value of the eFuses used to disable tile
 * clusters and home nodes in the TILE_STATUS register. In BlueField there
 * are a maximum of 8 Tiles; the reserved bits are for future expansion.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * A '1' indicates that the cluster in the Tile with the
		 * corresponding bit position is disabled.
		 */
		uint64_t cluster      : 8;
		/* Reserved. */
		uint64_t __reserved_0 : 24;
		/*
		 * A '1' indicates that the Home Node in the Tile with the
		 * corresponding bit position is disabled.
		 */
		uint64_t hnf          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t hnf          : 8;
		uint64_t __reserved_0 : 24;
		uint64_t cluster      : 8;
#endif
	};

	uint64_t word;
} RSH_EFUSE_TILE_DISABLE_t;


/*
 * Feature Disable.
 * SKU specific information; used by BL1 code. This register resets only on
 * hard reset and maintains its state after a software reset. It is
 * initialized based on the configuration of the product SKU which is
 * contained in eFuses.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* The value is used by BL1 code. */
		uint64_t value      : 48;
		/* Reserved. */
		uint64_t __reserved : 16;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 16;
		uint64_t value      : 48;
#endif
	};

	uint64_t word;
} RSH_EFUSE_FEATURE_t;


/*
 * Tile Disable.
 * Disables HNFs/clusters, and is used in functions such as coresight and the
 * pci bridge setup broadcast. This register resets only on hard reset and
 * maintains its state after a software reset.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * One disable bit per tile (bit 0 = linear tile id 0, bit 1
		 * = linear tile id 1, etc).
		 */
		uint64_t hnf          : 8;
		/* Reserved. */
		uint64_t __reserved_0 : 24;
		/*
		 * One disable bit per tile (bit 0 = linear tile id 0, bit 1
		 * = linear tile id 1, etc).
		 */
		uint64_t cluster      : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t cluster      : 8;
		uint64_t __reserved_0 : 24;
		uint64_t hnf          : 8;
#endif
	};

	uint64_t word;
} RSH_TILE_DISABLE_t;


/*
 * Software Interrupt.
 * Used to trigger interrupts via register write.  Each bit corresponds to a
 * single SWINT interrupt which will be triggered when written with a 1.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Software Interrupt.
		 * Used to trigger interrupts via register write.  Each bit
		 * corresponds to a single SWINT interrupt which will be
		 * triggered when written with a 1.
		 */
		uint64_t swint      : 4;
		/* Reserved. */
		uint64_t __reserved : 60;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 60;
		uint64_t swint      : 4;
#endif
	};

	uint64_t word;
} RSH_SWINT_t;


/*
 * Pseudo-Rand.
 * Provides a pseudo-random 64-bit value in the range of [1..(2**64)-1] using
 * a 64-bit LFSR (taps are 64,63,61,60).
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Pseudo-Rand.
		 * Provides a pseudo-random 64-bit value in the range of
		 * [1..(2**64)-1] using a 64-bit LFSR (taps are 64,63,61,60).
		 */
		uint64_t pseudo_rand : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t pseudo_rand : 64;
#endif
	};

	uint64_t word;
} RSH_PSEUDO_RAND_t;


/* Pseudo-Rand Control. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When 1, the RSHIM_P_RAND value advances once each time it
		 * is read.  When 0, it is free running.
		 */
		uint64_t p_rand_mode : 1;
		/* Reserved. */
		uint64_t __reserved  : 63;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved  : 63;
		uint64_t p_rand_mode : 1;
#endif
	};

	uint64_t word;
} RSH_PSEUDO_RAND_CONTROL_t;


/*
 * Uptime.
 * Provides core_refclk cycle count since last reset (either hard or soft).
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Uptime.
		 * Provides core_refclk cycle count since last reset (either
		 * hard or soft).
		 */
		uint64_t uptime     : 56;
		/* Reserved. */
		uint64_t __reserved : 8;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 8;
		uint64_t uptime     : 56;
#endif
	};

	uint64_t word;
} RSH_UPTIME_t;


/*
 * Uptime.
 * Provides core_refclk cycle count since last hard_reset.  This register is
 * not reset on a software reset.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Uptime.
		 * Provides core_refclk cycle count since last hard_reset.
		 * This register is not reset on a software reset.
		 */
		uint64_t uptime_por : 56;
		/* Reserved. */
		uint64_t __reserved : 8;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 8;
		uint64_t uptime_por : 56;
#endif
	};

	uint64_t word;
} RSH_UPTIME_POR_t;


/*
 * Boot RAM Control.
 * This register provides indirect access to the 512KB boot RAM.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * 8-byte word index into the boot RAM. This field is
		 * incremented to the next 8-byte word automatically on a
		 * read or write to the BOOT_RAM_DATA register.
		 */
		uint64_t idx        : 16;
		/* Reserved. */
		uint64_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 48;
		uint64_t idx        : 16;
#endif
	};

	uint64_t word;
} RSH_BOOT_RAM_CTL_t;


/*
 * Boot RAM Data.
 * This register provides read/write access to the boot RAM at the address
 * specified in BOOT_RAM_CTL.IDX.  IDX is automatically incremented on any
 * access.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Boot RAM Data.
		 * This register provides read/write access to the boot RAM
		 * at the address specified in BOOT_RAM_CTL.IDX.  IDX is
		 * automatically incremented on any access.
		 */
		uint64_t boot_ram_data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t boot_ram_data : 64;
#endif
	};

	uint64_t word;
} RSH_BOOT_RAM_DATA_t;


/*
 * Boot RAM Window.
 * This register controls the physical address mapping of the boot ROM and
 * RAM regions.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When 0, the boot ROM will be mapped to offset 0. When 1,
		 * the boot RAM will be mapped to offset 0.   This register
		 * is reset by hard_rst_l but not software reset.
		 */
		uint64_t ctl        : 1;
		/* Reserved. */
		uint64_t __reserved : 63;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 63;
		uint64_t ctl        : 1;
#endif
	};

	uint64_t word;
} RSH_BOOT_RAM_WINDOW_t;


/*
 * GPIO Mode Control.
 * Device specific mode control for GPIO pins.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Enables alternate MMC card. */
		uint64_t mmc_alt_card     : 1;
		/*
		 * Enables 4-bit alternate MMC card. MMC_ALT_CARD must also
		 * be 1.
		 */
		uint64_t mmc_alt_card4    : 1;
		/* Enables SW control over DDR_SAVE0 instead of HW FSM */
		uint64_t ddr_save0_sw_ctl : 1;
		/* Enables SW control over DDR_SAVE1 instead of HW FSM */
		uint64_t ddr_save1_sw_ctl : 1;
		/* Reserved. */
		uint64_t __reserved       : 60;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved       : 60;
		uint64_t ddr_save1_sw_ctl : 1;
		uint64_t ddr_save0_sw_ctl : 1;
		uint64_t mmc_alt_card4    : 1;
		uint64_t mmc_alt_card     : 1;
#endif
	};

	uint64_t word;
} RSH_BF1_GPIO_MODE_t;


/*
 * TileMonitor Host to Tile Data.
 * Provides read/write access to the HostToTile FIFO.  When written, the
 * TileMonitor HostToTile FIFO will be written with the associated data and
 * the WPTR will be advanced.  When read, the entry at the head of the FIFO
 * will be returned and the RPTR will be advanced.
 *
 * When empty, reads will have no effect.  Behavior on an empty read is
 * device dependent.  Tile software will receive all 1's on an empty read.
 *
 * When full, writes will be ignored.  Behavior on a full write is device
 * dependent.
 *
 * For Gx9/16/36/72 devices, this FIFO may only be used when PG.AUTO_SEND is
 * zero (packet generator not in boot mode).
 *
 * The FIFO may be drained by reading TM_HOST_TO_TILE_STS.COUNT entries.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * TileMonitor Host to Tile Data.
		 * Provides read/write access to the HostToTile FIFO.  When
		 * written, the TileMonitor HostToTile FIFO will be written
		 * with the associated data and the WPTR will be advanced.
		 * When read, the entry at the head of the FIFO will be
		 * returned and the RPTR will be advanced.
		 *
		 * When empty, reads will have no effect.  Behavior on an
		 * empty read is device dependent.  Tile software will
		 * receive all 1's on an empty read.
		 *
		 * When full, writes will be ignored.  Behavior on a full
		 * write is device dependent.
		 *
		 * For Gx9/16/36/72 devices, this FIFO may only be used when
		 * PG.AUTO_SEND is zero (packet generator not in boot mode).
		 *
		 * The FIFO may be drained by reading
		 * TM_HOST_TO_TILE_STS.COUNT entries.
		 */
		uint64_t tm_host_to_tile_data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t tm_host_to_tile_data : 64;
#endif
	};

	uint64_t word;
} RSH_TM_HOST_TO_TILE_DATA_t;


/*
 * TileMonitor Host to Tile Status.
 * Provides status of the HostToTile TileMonitor FIFO.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Current entry count. */
		uint64_t count      : 9;
		/* Reserved. */
		uint64_t __reserved : 55;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 55;
		uint64_t count      : 9;
#endif
	};

	uint64_t word;
} RSH_TM_HOST_TO_TILE_STS_t;


/*
 * TileMonitor Host to Tile Control.
 * Provides control over FIFO interrupts.  Note that the HWM/LWM interrupts
 * trigger continuously as long as the FIFO is is at or above/below the
 * associated water mark.  Thus these interrupts are typically used in
 * INT_BIND.MODE=0 and should be cleared by SW after the condition has been
 * handled (FIFO filled/drained as appropriate).
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Level at which to trigger the low water mark interrupt. */
		uint64_t lwm          : 8;
		/* Level at which to trigger the high water mark interrupt. */
		uint64_t hwm          : 8;
		/* Reserved. */
		uint64_t __reserved_0 : 16;
		/*
		 * Provides size of the HostToTile FIFO in terms of 8-byte
		 * entries.
		 */
		uint64_t max_entries  : 9;
		/* Reserved. */
		uint64_t __reserved_1 : 23;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 23;
		uint64_t max_entries  : 9;
		uint64_t __reserved_0 : 16;
		uint64_t hwm          : 8;
		uint64_t lwm          : 8;
#endif
	};

	uint64_t word;
} RSH_TM_HOST_TO_TILE_CTL_t;


/*
 * TileMonitor tile to host Data.
 * Provides read/write access to the TileToHost FIFO.  When written, the
 * TileMonitor TileToHost FIFO will be written with the associated data and
 * the WPTR will be advanced.  When read, the entry at the head of the FIFO
 * will be returned and the RPTR will be advanced.
 *
 * When empty, reads will have no effect.  Behavior on an empty read is
 * device dependent.  Tile software will receive all 1's on an empty read.
 *
 * When full, writes will be ignored.  Behavior on a full write is device
 * dependent.
 *
 * For Gx9/16/36/72 devices, this FIFO may only be used when PG.AUTO_SEND is
 * zero (packet generator not in boot mode).
 *
 * The FIFO may be drained by reading TM_TILE_TO_HOST_STS.COUNT entries.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * TileMonitor tile to host Data.
		 * Provides read/write access to the TileToHost FIFO.  When
		 * written, the TileMonitor TileToHost FIFO will be written
		 * with the associated data and the WPTR will be advanced.
		 * When read, the entry at the head of the FIFO will be
		 * returned and the RPTR will be advanced.
		 *
		 * When empty, reads will have no effect.  Behavior on an
		 * empty read is device dependent.  Tile software will
		 * receive all 1's on an empty read.
		 *
		 * When full, writes will be ignored.  Behavior on a full
		 * write is device dependent.
		 *
		 * For Gx9/16/36/72 devices, this FIFO may only be used when
		 * PG.AUTO_SEND is zero (packet generator not in boot mode).
		 *
		 * The FIFO may be drained by reading
		 * TM_TILE_TO_HOST_STS.COUNT entries.
		 */
		uint64_t tm_tile_to_host_data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t tm_tile_to_host_data : 64;
#endif
	};

	uint64_t word;
} RSH_TM_TILE_TO_HOST_DATA_t;


/*
 * TileMonitor tile to host status.
 * Provides status of the TileToHost TileMonitor FIFO.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Current entry count. */
		uint64_t count      : 9;
		/* Reserved. */
		uint64_t __reserved : 55;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 55;
		uint64_t count      : 9;
#endif
	};

	uint64_t word;
} RSH_TM_TILE_TO_HOST_STS_t;


/*
 * TileMonitor tile to host Control.
 * Provides control over FIFO interrupts.  Note that the HWM/LWM interrupts
 * trigger continuously as long as the FIFO is is at or above/below the
 * associated water mark.  Thus these interrupts are typically used in
 * INT_BIND.MODE=0 and should be cleared by SW after the condition has been
 * handled (FIFO filled/drained as appropriate).
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Level at which to trigger the low water mark interrupt. */
		uint64_t lwm          : 8;
		/* Level at which to trigger the high water mark interrupt. */
		uint64_t hwm          : 8;
		/* Reserved. */
		uint64_t __reserved_0 : 16;
		/*
		 * Provides size of the TileToHost FIFO in terms of 8-byte
		 * entries.
		 */
		uint64_t max_entries  : 9;
		/* Reserved. */
		uint64_t __reserved_1 : 23;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 23;
		uint64_t max_entries  : 9;
		uint64_t __reserved_0 : 16;
		uint64_t hwm          : 8;
		uint64_t lwm          : 8;
#endif
	};

	uint64_t word;
} RSH_TM_TILE_TO_HOST_CTL_t;


/* Scratchpad. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Scratchpad. */
		uint64_t scratchpad1 : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t scratchpad1 : 64;
#endif
	};

	uint64_t word;
} RSH_SCRATCHPAD1_t;


/* Scratchpad. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Scratchpad. */
		uint64_t scratchpad2 : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t scratchpad2 : 64;
#endif
	};

	uint64_t word;
} RSH_SCRATCHPAD2_t;


/* Scratchpad. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Scratchpad. */
		uint64_t scratchpad3 : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t scratchpad3 : 64;
#endif
	};

	uint64_t word;
} RSH_SCRATCHPAD3_t;


/* Scratchpad. */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Scratchpad. */
		uint64_t scratchpad4 : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t scratchpad4 : 64;
#endif
	};

	uint64_t word;
} RSH_SCRATCHPAD4_t;


/*
 * Secure Boot Mode.
 * Indicates status about the boot mode.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Secure Boot Mode. */
		uint64_t secure       : 2;
		/*
		 * Secure Boot test enable. This field can be written to
		 * enable testing of secure boot functions. This field is
		 * ORed with the chip state, which is set by eFuses and is
		 * readable in SECURE field of this register. Once a bit of
		 * this field is written to 1 it can not be written back to
		 * 0; only a hard reset will reset it.
		 */
		uint64_t test         : 2;
		/* Reserved. */
		uint64_t __reserved_0 : 3;
		/*
		 * Indicates that the initial key download from eFuses is
		 * completed. Keys are not valid prior to this bit being set.
		 */
		uint64_t init_done    : 1;
		/*
		 * Indicates status of secure boot. This field is used by
		 * boot software, not used by HW.
		 */
		uint64_t boot_status  : 4;
		/* Reserved. */
		uint64_t __reserved_1 : 52;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 52;
		uint64_t boot_status  : 4;
		uint64_t init_done    : 1;
		uint64_t __reserved_0 : 3;
		uint64_t test         : 2;
		uint64_t secure       : 2;
#endif
	};

	uint64_t word;
} RSH_SB_MODE_t;


/*
 * Secure Boot Key Access.
 * Controls access to keys.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Controls key updates to SB_KEY_n and SB_KEY_VLD KEY_n
		 * field and corresponding eFuses. Bits correspond to key 3,
		 * 2, 1, 0. Write-1-to-set (once set can only be cleared by
		 * hard reset).
		 */
		uint64_t write_disable       : 4;
		/* Reserved. */
		uint64_t __reserved_0        : 4;
		/*
		 * Enables SB_KEY_VLD and KEY values to be reloaded from
		 * eFuses after a key is written (by writing SB_KEY_WRITE_CTL
		 * with STEP value of END).
		 */
		uint64_t efuse_reread_enable : 1;
		/* Reserved. */
		uint64_t __reserved_1        : 55;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1        : 55;
		uint64_t efuse_reread_enable : 1;
		uint64_t __reserved_0        : 4;
		uint64_t write_disable       : 4;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_ACCESS_t;


/*
 * Secure Boot Key Valid.
 * Provides information about which version of each key is valid.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Valid bits for Key 0 versions, loaded from eFuses after a
		 * hard reset. Lower 4 bits are a thermometer code
		 *         indicating key programming has started for key n;
		 * 0000 = none, 0001 = version 0, 0011 = version 1,
		 *         0111 = version 2, 1111 = version 3. Upper 4 bits
		 * are a thermometer code indicating key programming has
		 * completed for
		 *         key n (same encodings as the start bits). This
		 * allows for detection of an interruption in the progamming
		 * process which
		 *         has left the key partially programmed (and thus
		 * invalid). The process is to burn the eFuse for the new key
		 * start bit,
		 *         burn the key eFuses, then burn the eFuse for the
		 * new key complete bit. For example 0000_0000: no key valid,
		 * 0001_0001:
		 *         key version 0 valid, 0011_0011: key 1 version
		 * valid, 0011_0111: key version 2 started programming but
		 * did not complete,
		 *         etc. The most recent key for which both start and
		 * complete bit is set is loaded. On soft reset, this register
		 *         is not modified.
		 */
		uint64_t key_0      : 8;
		/* Valid bits for Key 1 versions, same encoding as Key 0. */
		uint64_t key_1      : 8;
		/* Valid bits for Key 2 versions, same encoding as Key 0. */
		uint64_t key_2      : 8;
		/* Valid bits for Key 3 versions, same encoding as Key 0. */
		uint64_t key_3      : 8;
		/* Reserved. */
		uint64_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 32;
		uint64_t key_3      : 8;
		uint64_t key_2      : 8;
		uint64_t key_1      : 8;
		uint64_t key_0      : 8;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_VLD_t;


/*
 * Key 0.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_0_t;


/*
 * Key 0.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_0_1_t;


/*
 * Key 0.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_0_2_t;


/*
 * Key 0.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_0_3_t;


/*
 * Key 0.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_0_4_t;


/*
 * Key 0.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_0_5_t;


/*
 * Key 0.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_0_6_t;


/*
 * Key 0.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_0_7_t;


/*
 * Key 1.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_1_t;


/*
 * Key 1.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_1_1_t;


/*
 * Key 1.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_1_2_t;


/*
 * Key 1.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_1_3_t;


/*
 * Key 1.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_1_4_t;


/*
 * Key 1.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_1_5_t;


/*
 * Key 1.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_1_6_t;


/*
 * Key 1.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_1_7_t;


/*
 * Key 2.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_2_t;


/*
 * Key 2.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_2_1_t;


/*
 * Key 2.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_2_2_t;


/*
 * Key 2.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_2_3_t;


/*
 * Key 2.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_2_4_t;


/*
 * Key 2.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_2_5_t;


/*
 * Key 2.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_2_6_t;


/*
 * Key 2.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_2_7_t;


/*
 * Key 3.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_3_t;


/*
 * Key 3.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_3_1_t;


/*
 * Key 3.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_3_2_t;


/*
 * Key 3.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_3_3_t;


/*
 * Key 3.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_3_4_t;


/*
 * Key 3.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_3_5_t;


/*
 * Key 3.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_3_6_t;


/*
 * Key 3.
 * Key
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* 64-bit segment of key record. */
		uint64_t sb_key : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t sb_key : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_3_7_t;


/*
 * Secure Boot Key Write Control.
 * Controls updates to keys. Note that writes will not be done if
 * SB_KEY_ACCESS WRITE_DISABLE bit is 1. The STEP field determines if key
 * data or key valid information is written
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Write 1 to initiate key write using the accompanying
		 * register fields. Bit will clear upon completion of key
		 * write and can be polled.
		 */
		uint64_t go           : 1;
		/* Controls which step of the update process is being done. */
		uint64_t step         : 2;
		/* Reserved. */
		uint64_t __reserved_0 : 5;
		/*
		 * Which 64-bit segment of the key to write. 0=[63:0],
		 * 1=[127:64], etc. This field is only used when STEP field
		 * is DATA, not START or END.
		 */
		uint64_t segment      : 3;
		/* Reserved. */
		uint64_t __reserved_1 : 5;
		/* Which version of the key to write. */
		uint64_t version      : 2;
		/* Reserved. */
		uint64_t __reserved_2 : 2;
		/* Which key to write. */
		uint64_t key          : 2;
		/* Reserved. */
		uint64_t __reserved_3 : 42;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_3 : 42;
		uint64_t key          : 2;
		uint64_t __reserved_2 : 2;
		uint64_t version      : 2;
		uint64_t __reserved_1 : 5;
		uint64_t segment      : 3;
		uint64_t __reserved_0 : 5;
		uint64_t step         : 2;
		uint64_t go           : 1;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_WRITE_CTL_t;


/*
 * Secure Boot Key Write Control.
 * Data value for updates to keys. This is not used if the STEP value in
 * SB_KEY_WRITE_CTL is START or END.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Data to write to the KEY/VERSION/SEGMENT specified in
		 * SB_KEY_WRITE_CTL.
		 */
		uint64_t data : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t data : 64;
#endif
	};

	uint64_t word;
} RSH_SB_KEY_WRITE_DATA_t;


/*
 * Life Cycle Enable.
 * This register must be written with a special value to enable updates to
 * Life Cycle eFuses, which is done by writing to LIFE_CYCLE_UPDATE register.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Writing the KEY enables updates to Life Cycle state;
		 * writing any other value disbles updates to Life Cycle
		 * state.
		 */
		uint64_t value : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t value : 64;
#endif
	};

	uint64_t word;
} RSH_LIFE_CYCLE_ENABLE_t;


/*
 * Life Cycle Update.
 * Writes to this register change the life cycle state, which is stored in
 * eFuses. The proper value must be written to LIFE_CYCLE_ENABLE register to
 * enable writes to this register
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Write 1 to initiate writing the value in NEW_STATE field
		 * to eFuses. This bit will clear upon completion of write
		 * and can be polled.
		 */
		uint64_t go           : 1;
		/* Reserved. */
		uint64_t __reserved_0 : 3;
		/* New life cycle state to store in eFuse. */
		uint64_t new_state    : 2;
		/* Reserved. */
		uint64_t __reserved_1 : 1;
		/*
		 * Indicates if the write was allowed. Valid after GO bit is
		 * 0.
		 */
		uint64_t status       : 1;
		/* Reserved. */
		uint64_t __reserved_2 : 56;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 56;
		uint64_t status       : 1;
		uint64_t __reserved_1 : 1;
		uint64_t new_state    : 2;
		uint64_t __reserved_0 : 3;
		uint64_t go           : 1;
#endif
	};

	uint64_t word;
} RSH_LIFE_CYCLE_UPDATE_t;


/*
 * CoreSight System-level DebugAPB.
 * System-level access to drive R/W transactions on the DebugAPB bus. SW must
 * not alter the contents of this register after initiating a DebugAPB
 * transaction until after GO has cleared.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Write 1 to initiate APB transaction using the accompanying
		 * register fields. Bit will clear upon completion of APB
		 * operation and can be polled. For read operations, read
		 * data is valid only after deassertion.
		 */
		uint64_t go     : 1;
		/*
		 * Defines the APB operation (RnW). A value of 0 indicates a
		 * write, 1 indicates a read.
		 */
		uint64_t action : 1;
		/*
		 * Address of APB target. For reading the top-level ROM
		 * Table, ADDR[28:10] = 0 and ADDR[9:0] select ROM Table row.
		 * Otherwise, ADDR[28] must be set to 1 due to coresight ip
		 * setup, ADDR[27:24]=LinearTileId+1 (add of 1 accommodates
		 * offset from ROM base address when accessing DebugAPB via
		 * the JTAG/SW interface.) *Note, the bit ranges defined in
		 * this description are the bits within the address field of
		 * this register. CORESIGHT_SYS_DBGAPB_CTL.ADDR[28:0]
		 * corresponds to apb address [30:2]. apb address [31] is
		 * supplied by the hardware depending on whether the request
		 * comes from a core.
		 */
		uint64_t addr   : 29;
		/*
		 * APB Slave error. Indicates that APB slave returned an
		 * error for the operation and validity of read data is
		 * unknown if asserted.  Not valid for writes, valid for read
		 * after deassertion of GO bit.
		 */
		uint64_t err    : 1;
		/*
		 * APB data. Contains data to be write/read data for
		 * write/read operations, respectively.  Validity of read
		 * data is governed by GO bit.
		 */
		uint64_t data   : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t data   : 32;
		uint64_t err    : 1;
		uint64_t addr   : 29;
		uint64_t action : 1;
		uint64_t go     : 1;
#endif
	};

	uint64_t word;
} RSH_CORESIGHT_SYS_DBGAPB_CTL_t;


/*
 * CoreSight Authentication Controls.
 * These controls enable invasive/noninvasive and secure/nonsecure debug.
 * Used in the DAP, CTI, and coresight components including those in the A72s.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Secure privileged non-invasive debug enable (0=not
		 * enabled, 1=enabled). Initialized to 0 for a secure chip,
		 * and 1 for a non-secure chip.
		 */
		uint64_t spniden    : 1;
		/*
		 * Secure privileged invasive debug enable (0=not enabled,
		 * 1=enabled). Initialized to 0 for a secure chip, and 1 for
		 * a non-secure chip.
		 */
		uint64_t spiden     : 1;
		/*
		 * Non-invasive debug enable (0=not enabled, 1=enabled).
		 * Initialized to 0 for a secure chip, and 1 for a non-secure
		 * chip.
		 */
		uint64_t niden      : 1;
		/*
		 * Invasive debug enable (0=not enabled, 1=enabled).
		 * Initialized to 0 for a secure chip, and 1 for a non-secure
		 * chip.
		 */
		uint64_t dbgen      : 1;
		/*
		 * HNF debug enable (0=not enabled, 1=enabled). Initialized
		 * to 0 for a secure chip, and 1 for a non-secure chip.
		 */
		uint64_t hnfdbgen   : 1;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t hnfdbgen   : 1;
		uint64_t dbgen      : 1;
		uint64_t niden      : 1;
		uint64_t spiden     : 1;
		uint64_t spniden    : 1;
#endif
	};

	uint64_t word;
} RSH_CORESIGHT_AUTH_CTL_t;


/*
 * Miscellaneous Node Configuration.
 * Miscellaneous Node Configuration.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Limit the concurrent number of DVMOPs, used for debug
		 * only. Values 0, 1, 2, 3 mean 4, 2, 2, 3 DVMOPs. Note that
		 * a minimum of 2
		 *      is required to allow DVMOp(Sync) to make progress.
		 */
		uint64_t dvmop_limit : 2;
		/* Reserved. */
		uint64_t __reserved  : 62;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved  : 62;
		uint64_t dvmop_limit : 2;
#endif
	};

	uint64_t word;
} RSH_MN_CFG_t;


/*
 * Miscellaneous Node Debug.
 * Internal Debug information from MN.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Debug information */
		uint64_t info : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t info : 64;
#endif
	};

	uint64_t word;
} RSH_MN_DBG_t;


/*
 * Patrol Scrubber Control.
 * Define the behavior of patrol scrubber control module. Patrol scrubber
 * will periodically issue "zero byte" write requests to the message, to
 * avoid the loss the infomration in the memory
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Enable patrol scrubber */
		uint64_t en           : 1;
		/* Number of address ranges */
		uint64_t num_addr_rng : 4;
		/*
		 * Frequency of issuing requests - the number of cycles
		 * between two write requests.
		 */
		uint64_t freq         : 18;
		/* Reserved. */
		uint64_t __reserved   : 41;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved   : 41;
		uint64_t freq         : 18;
		uint64_t num_addr_rng : 4;
		uint64_t en           : 1;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_CTL_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_1_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_2_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_3_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_4_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_5_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_6_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_7_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_8_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_9_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_10_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_11_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_12_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_13_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_14_t;


/*
 * Patrol Scrubber Address Range register.
 * Address range of patrol scrubber module. (Note: Patrol scrubber uses
 * physical address). NUM_ADDR_RNG in PATRL_SCRB_CTL defines the exact number
 * of address ranges used for patrol scrubber. Two PATRL_SCRB_RNG defines an
 * address range, the first one is the base address and the other one is the
 * end address.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 6;
		/* Address value */
		uint64_t val          : 34;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t val          : 34;
		uint64_t __reserved_0 : 6;
#endif
	};

	uint64_t word;
} RSH_PATRL_SCRB_RNG_15_t;


/*
 * Rom Bist Control Register.
 * Register contains control bits to specify the mode(bist, diagnostic or
 * functional), start address, number of words and read margin settings for
 * the rom bist logic.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* ROM BIST Enable */
		uint64_t be         : 1;
		/* Starting Address for rom bist operation */
		uint64_t sa         : 13;
		/*
		 * Number of words to be read for bist operation. A value of
		 * zero means read all the words in the rom
		 */
		uint64_t nw         : 13;
		/* Read Margin setting to use for bist operation */
		uint64_t rm         : 3;
		/* Passing signature value */
		uint64_t crc        : 32;
		/*
		 * ROM passing signature select. Set to "1" to enable the ROM
		 * BIST logic to use the CRC value provided in the CRC field
		 * of this register as the check value.
		 */
		uint64_t crc_en     : 1;
		/* Reserved. */
		uint64_t __reserved : 1;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 1;
		uint64_t crc_en     : 1;
		uint64_t crc        : 32;
		uint64_t rm         : 3;
		uint64_t nw         : 13;
		uint64_t sa         : 13;
		uint64_t be         : 1;
#endif
	};

	uint64_t word;
} RSH_BIST_CTL_REG_t;


/*
 * Rom Bist Status Register.
 * Register contains status bits to indicate the end of crc generateion.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* When high indicates bist operation is running */
		uint64_t busy       : 1;
		/*
		 * ROM BIST Fail indication. Field is valid only when DONE is
		 * asserted
		 */
		uint64_t fail       : 1;
		/*
		 * Calculated CRC value. Field is valid only when DONE is
		 * asserted
		 */
		uint64_t crc_calc   : 32;
		/* Reserved. */
		uint64_t __reserved : 30;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 30;
		uint64_t crc_calc   : 32;
		uint64_t fail       : 1;
		uint64_t busy       : 1;
#endif
	};

	uint64_t word;
} RSH_BIST_STS_REG_t;


/*
 * Rom Bist Diagnostic Port Register.
 * Register contains controls for diagnostic mode.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Diagnostic Mode Enable */
		uint64_t de         : 1;
		/* Diagnostic Mode Address */
		uint64_t da         : 13;
		/* Diagnostic Mode Read Margin */
		uint64_t drm        : 3;
		/* Reserved. */
		uint64_t __reserved : 47;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 47;
		uint64_t drm        : 3;
		uint64_t da         : 13;
		uint64_t de         : 1;
#endif
	};

	uint64_t word;
} RSH_BIST_DIAG_PORT_REG_t;


/*
 * Rom Bist Diagnostic Data Register.
 * Register contains the msb of data read from diagnostic mode.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Upper 8 bytes of 128 bit data read in the diagnostic mode
		 */
		uint64_t drd : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t drd : 64;
#endif
	};

	uint64_t word;
} RSH_BIST_DIAG_DATA_MSB_REG_t;


/*
 * Rom Bist Diagnostic Data Register.
 * Register contains the lsb of data read from diagnostic mode.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Lower 8 bytes of 128 bit data read in the diagnostic mode
		 */
		uint64_t drd : 64;
#else   /* __BIG_ENDIAN__ */
		uint64_t drd : 64;
#endif
	};

	uint64_t word;
} RSH_BIST_DIAG_DATA_LSB_REG_t;


/*
 * Power Watchdog Pulse Width Low.
 * Power Watchdog Pulse Width Low - duration.  This register can only be
 * updated when RSH_PWR_WDOG_CTL.LOCK = 0.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * The power watchdog must assert SAVE_n low to the NVDIMMs
		 * for at least 1us. This register specifies the number of
		 * rshim cycles SAVE_n is held low before releasing.
		 */
		uint64_t duration   : 16;
		/* Reserved. */
		uint64_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 48;
		uint64_t duration   : 16;
#endif
	};

	uint64_t word;
} RSH_PWR_WDOG_PW_LOW_t;


/*
 * Power Watchdog TI Threshold.
 * Power Watchdog Threshold for Interrupt. This register can only be updated
 * when RSH_PWR_WDOG_CTL.LOCK = 0.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Power watchdog timer begins incrementing up from zero when
		 * any power_good indication = 0. When timer reaches this
		 * value and enable is set, an interrupt is generated to all
		 * cores. Expected to be much lower than TF THRESH value.
		 */
		uint64_t thresh     : 20;
		/* Interrupt enable. */
		uint64_t enable     : 1;
		/* Reserved. */
		uint64_t __reserved : 43;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 43;
		uint64_t enable     : 1;
		uint64_t thresh     : 20;
#endif
	};

	uint64_t word;
} RSH_PWR_WDOG_TI_t;


/*
 * Power Watchdog TF Threshold.
 * Power Watchdog Threshold for Flush.  This register can only be updated
 * when RSH_PWR_WDOG_CTL.LOCK = 0.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Power watchdog timer begins incrementing up from zero when
		 * any power_good indication = 0. When timer reaches this
		 * value and enable is set, autonomous flush is triggered.
		 * Recommended to be at least ~5ms less than specified power
		 * supply holdup time.
		 */
		uint64_t thresh     : 20;
		/* Flush enable. */
		uint64_t enable     : 1;
		/* Reserved. */
		uint64_t __reserved : 43;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 43;
		uint64_t enable     : 1;
		uint64_t thresh     : 20;
#endif
	};

	uint64_t word;
} RSH_PWR_WDOG_TF_t;


/*
 * Power Watchdog Control.
 * Overall control settings for the power watchdog hardware.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When the lock bit is set to 1, attempted writes to the
		 * RSH_PWR_WDOG_PW_LOW, RSH_PWR_WDOG_TI, and RSH_PWR_WDOG_TF
		 * registers will be ignored.
		 */
		uint64_t lock             : 1;
		/*
		 * When set to 1, the power watchdog hardware will not
		 * participate in the ADR sequence. Interrupt will still be
		 * sent if PWR_WDOG_TI_ENABLE=1. Flush/reset/SAVE will be
		 * driven by software via the SW_ADR_SEQ registers regardless
		 * of the value of PWR_WDOG_TF_ENABLE.
		 */
		uint64_t sw_override      : 1;
		/*
		 * When 0, a pulse from the arm watchdog will immediately
		 * initiate the hardware ADR sequence. The power watchdog
		 * will ack the arm watchdog after
		 * RSH_PWR_WDOG_PW_LOW.DURATION is met. When 1, a pulse from
		 * the arm watchdog will not initiate the ADR sequence and
		 * the power watchdog will ack the arm watchdog immediately.
		 */
		uint64_t arm_wdog_disable : 1;
		/*
		 * When 1, mss 0 is disabled and will not respond to the ADR
		 * sequence. Hardware will behave as if mss0 has asserted
		 * memc_bkp_done or emi_flush_done.
		 */
		uint64_t mss0_disabled    : 1;
		/*
		 * When 1, mss 1 is disabled and will not respond to the ADR
		 * sequence. Hardware will behave as if mss1 has asserted
		 * memc_bkp_done or emi_flush_done.
		 */
		uint64_t mss1_disabled    : 1;
		/* Reserved. */
		uint64_t __reserved       : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved       : 59;
		uint64_t mss1_disabled    : 1;
		uint64_t mss0_disabled    : 1;
		uint64_t arm_wdog_disable : 1;
		uint64_t sw_override      : 1;
		uint64_t lock             : 1;
#endif
	};

	uint64_t word;
} RSH_PWR_WDOG_CTL_t;


/*
 * Software-Controlled ADR Sequence for MSS0.
 * Allows for software to control the Async DRAM Refresh sequence, instead of
 * the PWR_WDOG hardware.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * If RSH_PWR_WDOG_CTL.SW_OVERRIDE is set: Send flush command
		 * to L30 and EMI0.
		 */
		uint64_t flush_l3_emi : 1;
		/*
		 * If RSH_PWR_WDOG_CTL.SW_OVERRIDE is set: Send backup
		 * command to MEMC0.
		 */
		uint64_t bkp_memc     : 1;
		/*
		 * If RSH_PWR_WDOG_CTL.SW_OVERRIDE is set: Assert (low)
		 * SAVE_N to NVDIMMs 0.
		 */
		uint64_t save_n       : 1;
		/* Reserved. */
		uint64_t __reserved   : 61;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved   : 61;
		uint64_t save_n       : 1;
		uint64_t bkp_memc     : 1;
		uint64_t flush_l3_emi : 1;
#endif
	};

	uint64_t word;
} RSH_SW_ADR_SEQ_MSS0_t;


/*
 * Software-Controlled ADR Sequence for MSS1.
 * Allows for software to control the Async DRAM Refresh sequence, instead of
 * the PWR_WDOG hardware.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * If RSH_PWR_WDOG_CTL.SW_OVERRIDE is set: Send flush command
		 * to L31 and EMI1.
		 */
		uint64_t flush_l3_emi : 1;
		/*
		 * If RSH_PWR_WDOG_CTL.SW_OVERRIDE is set: Send backup
		 * command to MEMC1.
		 */
		uint64_t bkp_memc     : 1;
		/*
		 * If RSH_PWR_WDOG_CTL.SW_OVERRIDE is set: Assert (low)
		 * SAVE_N to NVDIMMs 1.
		 */
		uint64_t save_n       : 1;
		/* Reserved. */
		uint64_t __reserved   : 61;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved   : 61;
		uint64_t save_n       : 1;
		uint64_t bkp_memc     : 1;
		uint64_t flush_l3_emi : 1;
#endif
	};

	uint64_t word;
} RSH_SW_ADR_SEQ_MSS1_t;


/*
 * Power Watchdog Status.
 * Power watchdog status registers.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Current value of POWER_GOOD. */
		uint64_t power_good_state           : 1;
		/* Current value nvdimm_save_req from ARM watchdog. */
		uint64_t arm_wdog_req_state         : 1;
		/*
		 * The power watchdog counter is counting, and its value is
		 * less than the TI Threshold.
		 */
		uint64_t wdog_counting_lt_ti        : 1;
		/*
		 * The power watchdog counter is counting, and its value is
		 * between the TI and TF Thresholds.
		 */
		uint64_t wdog_counting_gt_ti_lt_tf  : 1;
		/* The hardware-controlled ADR Sequence is now in progress. */
		uint64_t wdog_hw_adr_seq_in_prog    : 1;
		/* Status of MSS0 L3,EMI flush request. */
		uint64_t mss0_l3_emi_flush_req      : 1;
		/* Status of MSS0 L3,EMI flush done. */
		uint64_t mss0_l3_emi_flush_done     : 1;
		/* Status of MSS0 MEMC backup request. */
		uint64_t mss0_memc_bkp_req          : 1;
		/* Status of MSS0 MEMC backup done. */
		uint64_t mss0_memc_bkp_done         : 1;
		/* Status of SAVE0 req to nvdimms. */
		uint64_t save_n_0                   : 1;
		/* The hardware ADR state machine is DONE for mss0. */
		uint64_t adr_seq0_done              : 1;
		/* Status of MSS1 L3,EMI flush request. */
		uint64_t mss1_l3_emi_flush_req      : 1;
		/* Status of MSS1 L3,EMI flush done. */
		uint64_t mss1_l3_emi_flush_done     : 1;
		/* Status of MSS1 MEMC backup request. */
		uint64_t mss1_memc_bkp_req          : 1;
		/* Status of MSS1 MEMC backup done. */
		uint64_t mss1_memc_bkp_done         : 1;
		/* Status of SAVE1 req to nvdimms. */
		uint64_t save_n_1                   : 1;
		/* The hardware ADR state machine is DONE for mss1. */
		uint64_t adr_seq1_done              : 1;
		/* An ack was sent to the arm wdog. */
		uint64_t arm_wdog_ack_sent          : 1;
		/* The number of times the TI threshold was reached. */
		uint64_t ti_thresh_reached_count    : 12;
		/*
		 * Captures number of cycles of the latest power failure
		 * where power came back on before TF threshold was met.
		 */
		uint64_t latest_short_pbad_duration : 20;
		/* Reserved. */
		uint64_t __reserved                 : 14;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved                 : 14;
		uint64_t latest_short_pbad_duration : 20;
		uint64_t ti_thresh_reached_count    : 12;
		uint64_t arm_wdog_ack_sent          : 1;
		uint64_t adr_seq1_done              : 1;
		uint64_t save_n_1                   : 1;
		uint64_t mss1_memc_bkp_done         : 1;
		uint64_t mss1_memc_bkp_req          : 1;
		uint64_t mss1_l3_emi_flush_done     : 1;
		uint64_t mss1_l3_emi_flush_req      : 1;
		uint64_t adr_seq0_done              : 1;
		uint64_t save_n_0                   : 1;
		uint64_t mss0_memc_bkp_done         : 1;
		uint64_t mss0_memc_bkp_req          : 1;
		uint64_t mss0_l3_emi_flush_done     : 1;
		uint64_t mss0_l3_emi_flush_req      : 1;
		uint64_t wdog_hw_adr_seq_in_prog    : 1;
		uint64_t wdog_counting_gt_ti_lt_tf  : 1;
		uint64_t wdog_counting_lt_ti        : 1;
		uint64_t arm_wdog_req_state         : 1;
		uint64_t power_good_state           : 1;
#endif
	};

	uint64_t word;
} RSH_PWR_WDOG_STATUS_t;


/*
 * Tile Enable Status.
 * This register shows the status of the tiles (does not actually
 * enable/disable tiles) and is used in functions such as coresight and the
 * pci bridge setup broadcast.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * One bit per tile (bit 0 = linear tile id 0, bit 1 = linear
		 * tile id 1, etc). Set up when system comes out of reset
		 * based on lrhc, TILE_DISABLE register, efuses, and boot
		 * record.
		 */
		uint64_t hnf_ena      : 8;
		/* Reserved. */
		uint64_t __reserved_0 : 24;
		/*
		 * One bit per tile (bit 0 = linear tile id 0, bit 1 = linear
		 * tile id 1, etc). Set up when system comes out of reset
		 * based on lrhc, TILE_DISABLE register, efuses, and boot
		 * record.
		 */
		uint64_t cluster_ena  : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 24;
		uint64_t cluster_ena  : 8;
		uint64_t __reserved_0 : 24;
		uint64_t hnf_ena      : 8;
#endif
	};

	uint64_t word;
} RSH_TILE_STATUS_t;


/*
 * Trio Enable Status.
 * This register shows the status of the trios(does not actually
 * enable/disable trios) and is used in functions such as the pci bridge
 * setup broadcast.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* One bit per trio. All enabled by default. */
		uint64_t ena        : 3;
		/* Reserved. */
		uint64_t __reserved : 61;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 61;
		uint64_t ena        : 3;
#endif
	};

	uint64_t word;
} RSH_TRIO_STATUS_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_1_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_2_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_3_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_4_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_5_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_6_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_7_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_8_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_9_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_10_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_11_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_12_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_13_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_14_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_15_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_16_t;


/*
 * PCIe Bridge Copy Bus Numbers.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 8;
		/* Secondary Bus Number */
		uint64_t sec          : 8;
		/* Subordinate Bus Number */
		uint64_t sub          : 8;
		/* Reserved. */
		uint64_t __reserved_1 : 40;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1 : 40;
		uint64_t sub          : 8;
		uint64_t sec          : 8;
		uint64_t __reserved_0 : 8;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_BUS_NUM_17_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_1_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_2_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_3_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_4_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_5_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_6_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_7_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_8_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_9_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_10_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_11_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_12_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_13_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_14_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_15_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_16_t;


/*
 * PCIe Bridge Copy for non-prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for
		 * non-prefetchable memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for
		 * non-prefetchable memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_NON_PF_17_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_1_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_2_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_3_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_4_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_5_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_6_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_7_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_8_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_9_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_10_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_11_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_12_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_13_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_14_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_15_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_16_t;


/*
 * PCIe Bridge Copy for prefetchable base/limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0 : 4;
		/*
		 * Addr[31:20] of 1MB aligned base address for prefetchable
		 * memory space
		 */
		uint64_t base_addr    : 12;
		/* Reserved. */
		uint64_t __reserved_1 : 4;
		/*
		 * Addr[31:20] of 1MB aligned limit address for prefetchable
		 * memory space
		 */
		uint64_t lim_addr     : 12;
		/* Reserved. */
		uint64_t __reserved_2 : 32;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_2 : 32;
		uint64_t lim_addr     : 12;
		uint64_t __reserved_1 : 4;
		uint64_t base_addr    : 12;
		uint64_t __reserved_0 : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_17_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_1_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_2_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_3_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_4_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_5_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_6_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_7_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_8_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_9_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_10_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_11_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_12_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_13_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_14_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_15_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_16_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable base.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned base address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_BASE_17_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_1_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_2_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_3_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_4_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_5_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_6_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_7_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_8_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_9_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_10_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_11_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_12_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_13_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_14_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_15_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_16_t;


/*
 * PCIe Bridge Copy for upper bits of prefetchable limit.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. One register per bridge.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * Addr[36:32] of 1MB aligned limit address for prefetchable
		 * memory space.
		 */
		uint64_t addr       : 5;
		/* Reserved. */
		uint64_t __reserved : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 59;
		uint64_t addr       : 5;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_PF_UPPER_LIMIT_17_t;


/*
 * PCIe Bridge Copy for root control register.
 * RSHIM will copy these values to corresponding downstream bridge if
 * COPY_ENA=1. This register copied to all 3 trio instances.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/* Reserved. */
		uint64_t __reserved_0  : 4;
		/* CRS Software Visibility Enable. */
		uint64_t crs_sw_vis_en : 1;
		/* Reserved. */
		uint64_t __reserved_1  : 59;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved_1  : 59;
		uint64_t crs_sw_vis_en : 1;
		uint64_t __reserved_0  : 4;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_ROOT_CTL_t;


/*
 * PCIe Bridge Copy Control Register.
 * Control bits for the RSHIM pcie bridge copy logic.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * The RSHIM pci bridge copy logic is enabled. Regardless of
		 * the value of COPY_ENA, any new write to the RSHIM
		 * BRIDGE_COPY reigsters will be completed and marked
		 * pending. When COPY_ENA=1 any pending writes will be
		 * automatically copied by hardware. When COPY_ENA=0 all
		 * pending writes will be held in the rshim registers, and
		 * not copied until COPY_ENA is set to 1 again.
		 */
		uint64_t copy_ena   : 1;
		/* Reserved. */
		uint64_t __reserved : 63;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 63;
		uint64_t copy_ena   : 1;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_CTL_t;


/*
 * PCIe Bridge Copy Status Register.
 * Status bits for the RSHIM pcie bridge copy logic.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When a write arrives at any copy register, hardware resets
		 * COPY_IDLE to 0. Hardware sets to 1 when all copies have
		 * been sent from rshim and all write acks have been
		 * received. This value is correct regardless of the state of
		 * the ENA field.
		 */
		uint64_t copy_idle  : 1;
		/* Reserved. */
		uint64_t __reserved : 63;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 63;
		uint64_t copy_idle  : 1;
#endif
	};

	uint64_t word;
} RSH_PCI_BRIDGE_COPY_STAT_t;


/*
 * Voltage Droop Control.
 * Control basic functionality of voltage droop monitor.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When 0, the voltage droop monitor will be held in reset.
		 * Typically changes to the configuration are applied with
		 * RESET_OVD=0 then a second UPDATE is done with RESET_OVD=1.
		 */
		uint64_t reset_ovd          : 1;
		/* clock gate enable to delay paths and filter */
		uint64_t clken              : 1;
		/*
		 * clock phase mux enable. When 0, bypass clock phase mux and
		 * pass core clock through
		 */
		uint64_t cpmen              : 1;
		/*
		 * fine-tune adjustment of delay path capture clock latency.
		 * 5b10000 is nominal 0. Range is +/- ~300ps, depending on
		 * corner
		 */
		uint64_t capture_time       : 5;
		/* time interval for ds/dt */
		uint64_t delta_t            : 2;
		/*
		 * small: smoother state_out estimate, large q -> more noisy
		 * but higher gain
		 */
		uint64_t filter_q           : 5;
		/*
		 * second order exponential control on gain,  lower => better
		 * tracking in noisy environments
		 */
		uint64_t filter_l           : 3;
		/* log2 gain on error sq. going into R */
		uint64_t filter_e           : 3;
		/*
		 * if the variance is larger than this, apply high_pk_margin
		 * to freq_ctl threshold calculation
		 */
		uint64_t filter_high_pk     : 4;
		/*
		 * margin applied to the low threshold for each frequency
		 * control level. Suggested 2. increase if the stretched
		 * clock tends to be too fast in practice.
		 */
		uint64_t low_thresh_margin  : 3;
		/*
		 * margin applied to the high side. Make > low margin for
		 * hysteresis
		 */
		uint64_t high_thresh_margin : 3;
		/*
		 * additional margin to apply to threshold calculation when
		 * variance > filter_high_pk
		 */
		uint64_t high_pk_margin     : 4;
		/*
		 * if calibration is disabled, this sets the needed passing
		 * delay paths for fastest clock output
		 */
		uint64_t max_tgt_ovd        : 7;
		/*
		 * sets how many delay paths are between each frequency
		 * control level
		 */
		uint64_t ctl_step_size      : 3;
		/*
		 * 00 = high freq (500-700ps cycle @ 1V),  delay/stage target
		 * = 4ps, start of range = 293ps.  01 = 700-1112ps cycle,
		 * delay/stage target = 8ps, start of range = 293ps. 10 =
		 * 1112 - 1932ps cycle, delay/stage target = 16ps, start of
		 * range = 293ps. 11 = 2312 - 3336ps cycle, delay/stage
		 * target = 16ps, start of range = 1800ps
		 */
		uint64_t freq_sel           : 2;
		/*
		 * optional bypass of certain registers. Recommend to set =
		 * to freq_sel
		 */
		uint64_t pipe_bypass        : 2;
		/*
		 * when high, set the target and filter_high_pk to a bit
		 * above the state_out and state_variance of the end of the
		 * calibration period. Also enable a calibration period for
		 * some cycles after reset_l deasserts.
		 */
		uint64_t calibrate_ena      : 1;
		/* the number of cycles to spend in calibration after reset */
		uint64_t calibrate_cycles   : 10;
		/*
		 * when high, pass the delay chain output through a
		 * synchronizer
		 */
		uint64_t sync_chains        : 1;
		/* 0 = divide by 2, 1 = clock phase mux */
		uint64_t cpm_type           : 1;
		/*
		 * Enable the voltage droop monitor and clock stretch. When
		 * 0, the core clock is bypassed around the monitor.
		 */
		uint64_t ena                : 1;
		/*
		 * When written with a 1, configuration of voltage droop
		 * monitor will be updated. Software must poll until this bit
		 * is zero before sending anohter update
		 */
		uint64_t update             : 1;
#else   /* __BIG_ENDIAN__ */
		uint64_t update             : 1;
		uint64_t ena                : 1;
		uint64_t cpm_type           : 1;
		uint64_t sync_chains        : 1;
		uint64_t calibrate_cycles   : 10;
		uint64_t calibrate_ena      : 1;
		uint64_t pipe_bypass        : 2;
		uint64_t freq_sel           : 2;
		uint64_t ctl_step_size      : 3;
		uint64_t max_tgt_ovd        : 7;
		uint64_t high_pk_margin     : 4;
		uint64_t high_thresh_margin : 3;
		uint64_t low_thresh_margin  : 3;
		uint64_t filter_high_pk     : 4;
		uint64_t filter_e           : 3;
		uint64_t filter_l           : 3;
		uint64_t filter_q           : 5;
		uint64_t delta_t            : 2;
		uint64_t capture_time       : 5;
		uint64_t cpmen              : 1;
		uint64_t clken              : 1;
		uint64_t reset_ovd          : 1;
#endif
	};

	uint64_t word;
} RSH_VDROOP_CTL_t;


/*
 * Voltage Droop Trace Control.
 * Control the trace buffer and triggers.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t enable                : 1;
		uint64_t trigger_val           : 7;
		uint64_t trigger_sel           : 2;
		uint64_t trigger_type          : 1;
		uint64_t trace_window_post_cnt : 10;
		uint64_t skip_samples          : 10;
		/* Reserved. */
		uint64_t __reserved            : 33;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved            : 33;
		uint64_t skip_samples          : 10;
		uint64_t trace_window_post_cnt : 10;
		uint64_t trigger_type          : 1;
		uint64_t trigger_sel           : 2;
		uint64_t trigger_val           : 7;
		uint64_t enable                : 1;
#endif
	};

	uint64_t word;
} RSH_VDROOP_TRACE_CTL_t;


/*
 * Voltage Droop Trace Status.
 * Used to extract trace data from the voltage droop monitor
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t trace_complete : 1;
		/*
		 * Indicates that sample is valid. When written with a 1,
		 * next sample will be captured.
		 */
		uint64_t sample_vld     : 1;
		uint64_t vec            : 40;
		/* Reserved. */
		uint64_t __reserved     : 22;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved     : 22;
		uint64_t vec            : 40;
		uint64_t sample_vld     : 1;
		uint64_t trace_complete : 1;
#endif
	};

	uint64_t word;
} RSH_VDROOP_TRACE_STS_t;


/*
 * Voltage Droop Stats.
 * Used to extract stats data from the voltage droop monitor
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		uint64_t mode       : 1;
		uint64_t rst        : 1;
		/*
		 * Indicates that stat counter is valid. When written with a
		 * 1, next counter will be captured.
		 */
		uint64_t sample_vld : 1;
		uint64_t vec        : 50;
		/* Reserved. */
		uint64_t __reserved : 11;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 11;
		uint64_t vec        : 50;
		uint64_t sample_vld : 1;
		uint64_t rst        : 1;
		uint64_t mode       : 1;
#endif
	};

	uint64_t word;
} RSH_VDROOP_STAT_STS_t;


/*
 * Clock Count.
 * Count cclks in relation to rclk.
 */

__extension__
typedef union {
	struct {
#ifndef __BIG_ENDIAN__
		/*
		 * When 1, the counter is running.  Cleared by HW once count
		 * is complete.  When written with a 1, the count sequence is
		 * restarted.  Counter runs automatically after reset.
		 * Software must poll until this bit is zero, then read the
		 * CLOCK_COUNT register again to get the final COUNT value.
		 */
		uint64_t run        : 1;
		/*
		 * Indicates the number of core clocks that were counted
		 * during 1000 device clock periods.  Result is accurate to
		 * within +/-1 core clock periods.
		 */
		uint64_t count      : 15;
		/* Reserved. */
		uint64_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
		uint64_t __reserved : 48;
		uint64_t count      : 15;
		uint64_t run        : 1;
#endif
	};

	uint64_t word;
} RSH_CCLK_CLOCK_COUNT_t;

#endif /* !defined(__DOXYGEN__) */

#endif /* !defined(__ASSEMBLER__) */

#endif /* !defined(__REGS_RSH_H__) */
