/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
 * XRing I2C adapter driver.
 *
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/bits.h>
#include <linux/compiler_types.h>
#include <linux/completion.h>
#include <linux/dev_printk.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/types.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio.h>
#include <linux/hwspinlock.h>

#include <dt-bindings/xring/platform-specific/peri_crg.h>
#include <dt-bindings/xring/platform-specific/DW_apb_i2c_header.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/hwlock/ap_spin_lock_id.h>

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)		"XR_I2C: " fmt

#if defined CONFIG_XRING_I2C_DMA
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#endif

#define XR_I2C_IRQ_EXIT		0x0
#define XR_I2C_IRQ_RUNING	0x1
#define XR_I2C_RECOVERING	0x2
#define XR_I2C_TRANS_DONE	0x3

#define I2C_CLK_PREPARE_ENABLE		1
#define I2C_CLK_DISABLE_UNPREPARE	2
#define I2C_CLK_EBABLE			3
#define I2C_CLK_DISABLE			4

#define I2C_WAIT_IRQ_RETRY_NUMS	10
#define I2C_WAIT_IRQ_INTERVAL	5  /* 5ms */

#define XR_I2C_CLK_NUMS		2
#define XR_PCLK_PREFIX_NAME	"pclk"
#define XR_PCLK_PREFIX_LEN	4

#define XR_SUPPORT_SPEED_MODE	4
#define XR_STANDARD_MODE	0
#define XR_FAST_MODE		1
#define XR_FAST_MODE_PLUS	2
#define XR_HIGH_SPEED_MODE	3

#define SCL_SS_FALLING_TIME	300  /* standspeed, scl falling time 300 ns */
#define SDA_SS_FALLING_TIME	300  /* standspeed, sda falling time 300 ns */
#define SCL_FS_FALLING_TIME	280  /* fastmode, scl falling time 280 ns */
#define SDA_FS_FALLING_TIME	280  /* fsatmode, sda falling time 280 ns */
#define SCL_FP_FALLING_TIME	100  /* fsatmode plus, scl falling time 100 ns */
#define SDA_FP_FALLING_TIME	100  /* fsatmode plus, sda falling time 100 ns */
#define SCL_HS_FALLING_TIME	10  /* hgh speed, scl falling time 10 ns */
#define SDA_HS_FALLING_TIME	50  /* hgh speed, sda falling time 50 ns */

#define TO_RX_TX_TL(x)		((x) - 1)  /* rlr > rx_outstanding */
#define TL_TO_FIFO(x)		((x) + 1)

#define XR_FIFO_DEPTH_MASK	0xFF
#define PARAM_TO_FIFO(x)	(((x) & XR_FIFO_DEPTH_MASK) + 1)
#define RETRIES			3
#define XR_I2C_DEFAULT_NR	-1
#define PINCTRL_CONFIG_ERROR	-1
#define PINCTRL_ALREADY_INIT	1

/*
 * I2C DMA Markers
 */
#define USE_DMA			1
#define DMA_BIT_MASK_64		64
#define DMA_BIT_MASK_32		32
#define IP_HAS_DMA		1
#define I2C_DMA_MAX_MSGS	7
#define DMA_THRESHOLD		64  /* TX/RX FIFO 64 Byte */

#define CMD_MATCH_DMA_WIDTH(x, y)	((x) * (y))
#define CMD_DMA_WIDTH_2BYTE(x)		((x) * DMA_SLAVE_BUSWIDTH_2_BYTES)
#define CMD_DMA_WIDTH_4BYTE(x)		((x) * DMA_SLAVE_BUSWIDTH_4_BYTES)

#define XR_I2C_DMA_TRANSFER_TIMEOUT		1000  /* 1000ms */

#define XR_WAIT_NOT_BUSY_TIMEOUT_RANGE_MIN	1100   /* 1100us */
#define XR_WAIT_NOT_BUSY_TIMEOUT_RANGE_MAX	20000  /* 20000us */

#define XR_WAIT_FOR_COMPLETE_MIN		25   /* 25us */
#define R_WAIT_FOR_COMPLETE_MAX			2000  /* 1000us */

#define XR_I2C_DISABLE_TIMEOUT			100  /* 100 times */
#define XR_I2C_DISABLE_USLEEP_RANGE_MIN		25   /* 25us */
#define XR_I2C_DISABLE_USLEEP_RANGE_MAX		250  /* 250us */

#define GET_DEV_LOCK_TIMEOUT			100  /* 100ms */
#define GET_HARDWARE_LOCK_TIMEOUT		1000  /* 1s */

#define DW_IC_DEFAULT_FUNCTIONALITY (I2C_FUNC_I2C |			\
					I2C_FUNC_SMBUS_BYTE |		\
					I2C_FUNC_SMBUS_BYTE_DATA |	\
					I2C_FUNC_SMBUS_WORD_DATA |	\
					I2C_FUNC_SMBUS_BLOCK_DATA |	\
					I2C_FUNC_SMBUS_I2C_BLOCK)

#define DW_IC_CON_MASTER			BIT(0)
#define DW_IC_CON_SPEED_STD			(1 << 1)
#define DW_IC_CON_SPEED_FAST			(2 << 1)
#define DW_IC_CON_SPEED_HIGH			(3 << 1)
#define DW_IC_CON_SPEED_MASK			GENMASK(2, 1)
#define DW_IC_CON_10BITADDR_SLAVE		BIT(3)
#define DW_IC_CON_10BITADDR_MASTER		BIT(4)
#define DW_IC_CON_RESTART_EN			BIT(5)
#define DW_IC_CON_SLAVE_DISABLE			BIT(6)
#define DW_IC_CON_STOP_DET_IFADDRESSED		BIT(7)
#define DW_IC_CON_TX_EMPTY_CTRL			BIT(8)
#define DW_IC_CON_RX_FIFO_FULL_HLD_CTRL		BIT(9)

#define DW_IC_DATA_CMD_DAT			GENMASK(7, 0)

/*
 * Registers offset
 */
#define DW_IC_CON		DW_APB_I2C_IC_CON
#define DW_IC_TAR		DW_APB_I2C_IC_TAR
#define DW_IC_SAR		DW_APB_I2C_IC_SAR
#define DW_IC_DATA_CMD		DW_APB_I2C_IC_DATA_CMD
#define DW_IC_SS_SCL_HCNT	DW_APB_I2C_IC_SS_SCL_HCNT
#define DW_IC_SS_SCL_LCNT	DW_APB_I2C_IC_SS_SCL_LCNT
#define DW_IC_FS_SCL_HCNT	DW_APB_I2C_IC_FS_SCL_HCNT
#define DW_IC_FS_SCL_LCNT	DW_APB_I2C_IC_FS_SCL_LCNT
#define DW_IC_HS_SCL_HCNT	DW_APB_I2C_IC_HS_SCL_HCNT
#define DW_IC_HS_SCL_LCNT	DW_APB_I2C_IC_HS_SCL_LCNT
#define DW_IC_INTR_STAT		DW_APB_I2C_IC_INTR_STAT
#define DW_IC_INTR_MASK		DW_APB_I2C_IC_INTR_MASK
#define DW_IC_RAW_INTR_STAT	DW_APB_I2C_IC_RAW_INTR_STAT
#define DW_IC_RX_TL		DW_APB_I2C_IC_RX_TL
#define DW_IC_TX_TL		DW_APB_I2C_IC_TX_TL
#define DW_IC_CLR_INTR		DW_APB_I2C_IC_CLR_INTR
#define DW_IC_CLR_RX_UNDER	DW_APB_I2C_IC_CLR_RX_UNDER
#define DW_IC_CLR_RX_OVER	DW_APB_I2C_IC_CLR_RX_OVER
#define DW_IC_CLR_TX_OVER	DW_APB_I2C_IC_CLR_TX_OVER
#define DW_IC_CLR_RD_REQ	DW_APB_I2C_IC_CLR_RD_REQ
#define DW_IC_CLR_TX_ABRT	DW_APB_I2C_IC_CLR_TX_ABRT
#define DW_IC_CLR_RX_DONE	DW_APB_I2C_IC_CLR_RX_DONE
#define DW_IC_CLR_ACTIVITY	DW_APB_I2C_IC_CLR_ACTIVITY
#define DW_IC_CLR_STOP_DET	DW_APB_I2C_IC_CLR_STOP_DET
#define DW_IC_CLR_START_DET	DW_APB_I2C_IC_CLR_START_DET
#define DW_IC_CLR_GEN_CALL	DW_APB_I2C_IC_CLR_GEN_CALL
#define DW_IC_ENABLE		DW_APB_I2C_IC_ENABLE
#define DW_IC_STATUS		DW_APB_I2C_IC_STATUS
#define DW_IC_TXFLR		DW_APB_I2C_IC_TXFLR
#define DW_IC_RXFLR		DW_APB_I2C_IC_RXFLR
#define DW_IC_SDA_HOLD		DW_APB_I2C_IC_SDA_HOLD
#define DW_IC_TX_ABRT_SOURCE	DW_APB_I2C_IC_TX_ABRT_SOURCE
#define DW_IC_ENABLE_STATUS	DW_APB_I2C_IC_ENABLE_STATUS
#define DW_IC_COMP_PARAM_1	DW_APB_I2C_IC_COMP_PARAM_1
#define DW_IC_COMP_VERSION	DW_APB_I2C_IC_COMP_VERSION
#define DW_IC_SDA_HOLD_MIN_VERS	DW_APB_I2C_IC_COMP_VERSION_REGISTERRESETVALUE
#define DW_IC_COMP_TYPE		DW_APB_I2C_IC_COMP_TYPE
#define DW_IC_COMP_TYPE_VALUE	DW_APB_I2C_IC_COMP_TYPE_REGISTERRESETVALUE
#define XR_IC_COMP_TYPE_MASK_32	DW_APB_I2C_IC_COMP_TYPE_REGISTERRESETMASK
#define XR_IC_COMP_TYPE_MASK_16	0x0000FFFF

#define IC_SCL_STUCK_AT_LOW_TIMEOUT	DW_APB_I2C_IC_SCL_STUCK_AT_LOW_TIMEOUT

#if defined CONFIG_XRING_I2C_DMA
#define XR_IC_DMA_CR		DW_APB_I2C_IC_DMA_CR
#define XR_IC_DMA_TDLR		DW_APB_I2C_IC_DMA_TDLR
#define XR_IC_DMA_RDLR		DW_APB_I2C_IC_DMA_RDLR
#define XR_IC_DMA_CR_RDMAE	BIT(DW_APB_I2C_IC_DMA_CR_RDMAE_BITADDRESSOFFSET)
#define XR_IC_DMA_CR_TDMAE	BIT(DW_APB_I2C_IC_DMA_CR_TDMAE_BITADDRESSOFFSET)
#endif

#define DW_IC_INTR_RX_UNDER	BIT(0)
#define DW_IC_INTR_RX_OVER	BIT(1)
#define DW_IC_INTR_RX_FULL	BIT(2)
#define DW_IC_INTR_TX_OVER	BIT(3)
#define DW_IC_INTR_TX_EMPTY	BIT(4)
#define DW_IC_INTR_RD_REQ	BIT(5)
#define DW_IC_INTR_TX_ABRT	BIT(6)
#define DW_IC_INTR_RX_DONE	BIT(7)
#define DW_IC_INTR_ACTIVITY	BIT(8)
#define DW_IC_INTR_STOP_DET	BIT(9)
#define DW_IC_INTR_START_DET	BIT(10)
#define DW_IC_INTR_GEN_CALL	BIT(11)
#define DW_IC_INTR_RESTART_DET	BIT(12)
#define XR_INTR_SCL_STUCK_LOW	BIT(14)

#define DW_IC_INTR_DEFAULT_MASK		(DW_IC_INTR_RX_FULL | \
					 DW_IC_INTR_TX_ABRT | \
					 DW_IC_INTR_STOP_DET)
#define DW_IC_INTR_MASTER_MASK		(DW_IC_INTR_DEFAULT_MASK | \
					 DW_IC_INTR_TX_EMPTY)
#define DW_IC_INTR_SLAVE_MASK		(DW_IC_INTR_DEFAULT_MASK | \
					 DW_IC_INTR_RX_DONE | \
					 DW_IC_INTR_RX_UNDER | \
					 DW_IC_INTR_RD_REQ)

#if defined CONFIG_XRING_I2C_DMA
#define XR_DMA_INTR_MASK		(DW_IC_INTR_TX_ABRT | \
					 DW_IC_INTR_STOP_DET)
#endif

#define DW_IC_STATUS_ACTIVITY		BIT(0)
#define DW_IC_STATUS_TFNF		BIT(1)
#define DW_IC_STATUS_TFE		BIT(2)
#define DW_IC_STATUS_RFNF		BIT(3)
#define DW_IC_STATUS_RFF		BIT(4)
#define DW_IC_STATUS_MASTER_ACTIVITY	BIT(5)
#define DW_IC_STATUS_SLAVE_ACTIVITY	BIT(6)

#define DW_IC_SDA_HOLD_RX_SHIFT		16
#define DW_IC_SDA_HOLD_RX_MASK		GENMASK(23, 16)

#define DW_IC_ERR_TX_ABRT	0x1

#define DW_IC_TAR_10BITADDR_MASTER	BIT(12)

#define DW_IC_COMP_PARAM_1_SPEED_MODE_HIGH	(BIT(2) | BIT(3))
#define DW_IC_COMP_PARAM_1_SPEED_MODE_MASK	GENMASK(3, 2)

/*
 * status codes
 */
#define STATUS_IDLE			0x0
#define STATUS_WRITE_IN_PROGRESS	0x1
#define STATUS_READ_IN_PROGRESS		0x2

/*
 * operation modes
 */
#define DW_IC_MASTER		0
#define DW_IC_SLAVE		1

/*
 * Hardware abort codes from the DW_IC_TX_ABRT_SOURCE register
 *
 * Only expected abort codes are listed here
 * refer to the datasheet for the full list
 */
#define ABRT_7B_ADDR_NOACK	0
#define ABRT_10ADDR1_NOACK	1
#define ABRT_10ADDR2_NOACK	2
#define ABRT_TXDATA_NOACK	3
#define ABRT_GCALL_NOACK	4
#define ABRT_GCALL_READ		5
#define ABRT_SBYTE_ACKDET	7
#define ABRT_SBYTE_NORSTRT	9
#define ABRT_10B_RD_NORSTRT	10
#define ABRT_MASTER_DIS		11
#define ARB_LOST		12
#define ABRT_SLAVE_FLUSH_TXFIFO	13
#define ABRT_SLAVE_ARBLOST	14
#define ABRT_SLAVE_RD_INTX	15

#define DW_IC_TX_ABRT_7B_ADDR_NOACK		BIT(ABRT_7B_ADDR_NOACK)
#define DW_IC_TX_ABRT_10ADDR1_NOACK		BIT(ABRT_10ADDR1_NOACK)
#define DW_IC_TX_ABRT_10ADDR2_NOACK		BIT(ABRT_10ADDR2_NOACK)
#define DW_IC_TX_ABRT_TXDATA_NOACK		BIT(ABRT_TXDATA_NOACK)
#define DW_IC_TX_ABRT_GCALL_NOACK		BIT(ABRT_GCALL_NOACK)
#define DW_IC_TX_ABRT_GCALL_READ		BIT(ABRT_GCALL_READ)
#define DW_IC_TX_ABRT_SBYTE_ACKDET		BIT(ABRT_SBYTE_ACKDET)
#define DW_IC_TX_ABRT_SBYTE_NORSTRT		BIT(ABRT_SBYTE_NORSTRT)
#define DW_IC_TX_ABRT_10B_RD_NORSTRT		BIT(ABRT_10B_RD_NORSTRT)
#define DW_IC_TX_ABRT_MASTER_DIS		BIT(ABRT_MASTER_DIS)
#define DW_IC_TX_ARB_LOST			BIT(ARB_LOST)
#define DW_IC_RX_ABRT_SLAVE_RD_INTX		BIT(ABRT_SLAVE_RD_INTX)
#define DW_IC_RX_ABRT_SLAVE_ARBLOST		BIT(ABRT_SLAVE_ARBLOST)
#define DW_IC_RX_ABRT_SLAVE_FLUSH_TXFIFO	BIT(ABRT_SLAVE_FLUSH_TXFIFO)

#define DW_IC_TX_ABRT_NOACK		(DW_IC_TX_ABRT_7B_ADDR_NOACK | \
					 DW_IC_TX_ABRT_10ADDR1_NOACK | \
					 DW_IC_TX_ABRT_10ADDR2_NOACK | \
					 DW_IC_TX_ABRT_TXDATA_NOACK | \
					 DW_IC_TX_ABRT_GCALL_NOACK)


#define XR_RX_FIFO_DEPTH_REG_OPS \
		DW_APB_I2C_IC_COMP_PARAM_1_RX_BUFFER_DEPTH_BITADDRESSOFFSET
#define XR_TX_FIFO_DEPTH_REG_OPS \
		DW_APB_I2C_IC_COMP_PARAM_1_TX_BUFFER_DEPTH_BITADDRESSOFFSET

#if defined CONFIG_XRING_I2C_DMA

struct dw_i2c_dev;

struct xr_i2c_timing_info {
	u32		scl_def_val;
	u32		sda_def_val;
	u8		speed_mode;
	bool		using_default;
};

struct dma_rx_msgs {
	u32			index;
	u8			*dma_buf;
};

struct xr_exteranl_dma {
	struct dma_chan		*tx_fifo_chan;
	struct dma_chan		*rx_fifo_chan;

	struct scatterlist	tx_data_cmd_sg;
	struct scatterlist	rx_data_sg[I2C_DMA_MAX_MSGS];

	u32			dma_tx_msg_idx;
	u32			dma_rx_msg_idx;

	u16			*dma_tx_buf;
	struct dma_rx_msgs	dma_rx_msgs[I2C_DMA_MAX_MSGS];

	struct dma_slave_config	tx_config;
	struct dma_slave_config	rx_config;

	struct dma_async_tx_descriptor	*rx_desc;
	struct dma_async_tx_descriptor	*tx_desc;

	u8			using_rx_dma;
};
#endif

struct clk;
struct device;
struct reset_control;

/**
 * struct dw_i2c_dev - private i2c-designware data
 * @dev: driver model device node
 * @map: IO registers map
 * @sysmap: System controller registers map
 * @base: IO registers pointer
 * @ext: Extended IO registers pointer
 * @cmd_complete: tx completion indicator
 * @clk: input reference clock
 * @pclk: clock required to access the registers
 * @slave: represent an I2C slave device
 * @cmd_err: run time hadware error code
 * @msgs: points to an array of messages currently being transferred
 * @msgs_num: the number of elements in msgs
 * @msg_write_idx: the element index of the current tx message in the msgs
 *	array
 * @tx_buf_len: the length of the current tx buffer
 * @tx_buf: the current tx buffer
 * @msg_read_idx: the element index of the current rx message in the msgs
 *	array
 * @rx_buf_len: the length of the current rx buffer
 * @rx_buf: the current rx buffer
 * @msg_err: error status of the current transfer
 * @status: i2c master status, one of STATUS_*
 * @abort_source: copy of the TX_ABRT_SOURCE register
 * @irq: interrupt number for the i2c master
 * @adapter: i2c subsystem adapter node
 * @slave_cfg: configuration for the slave device
 * @tx_fifo_depth: depth of the hardware tx fifo
 * @rx_fifo_depth: depth of the hardware rx fifo
 * @rx_outstanding: current master-rx elements in tx fifo
 * @timings: bus clock frequency, SDA hold and other timings
 * @sda_hold_time: SDA hold value
 * @ss_hcnt: standard speed HCNT value
 * @ss_lcnt: standard speed LCNT value
 * @fs_hcnt: fast speed HCNT value
 * @fs_lcnt: fast speed LCNT value
 * @fp_hcnt: fast plus HCNT value
 * @fp_lcnt: fast plus LCNT value
 * @hs_hcnt: high speed HCNT value
 * @hs_lcnt: high speed LCNT value
 * @acquire_lock: function to acquire a hardware lock on the bus
 * @release_lock: function to release a hardware lock on the bus
 * @shared_with_punit: true if this bus is shared with the SoCs PUNIT
 * @disable: function to disable the controller
 * @disable_int: function to disable all interrupts
 * @init: function to initialize the I2C hardware
 * @mode: operation mode - DW_IC_MASTER or DW_IC_SLAVE
 * @suspended: set to true if the controller is suspended
 *
 * HCNT and LCNT parameters can be used if the platform knows more accurate
 * values than the one computed based only on the input clock frequency.
 * Leave them to be %0 if not used.
 */
struct dw_i2c_dev {
	struct device		*dev;
	struct regmap		*map;
	struct regmap		*sysmap;
	void __iomem		*base;
	void __iomem		*ext;
	struct completion	cmd_complete;
	struct clk		*clk;
	struct clk		*pclk;
	struct reset_control	*rst;
	struct i2c_client	*slave;
	u32			(*get_clk_rate_khz)(struct dw_i2c_dev *dev);
	int			cmd_err;
	struct i2c_msg		*msgs;
	int			msgs_num;
	int			rx_msgs_num;
	int			msg_write_idx;
	u32			tx_buf_len;
	u8			*tx_buf;
	int			msg_read_idx;
	u32			rx_buf_len;
	u8			*rx_buf;
	int			msg_err;
	unsigned int		status;
	u32			abort_source;
	int			irq;
	u32			flags;
	struct i2c_adapter	adapter;
	u32			functionality;
	u32			master_cfg;
	u32			slave_cfg;
	unsigned int		tx_fifo_depth;
	unsigned int		rx_fifo_depth;
	int			rx_outstanding;
	struct i2c_timings	timings;
	u32			sda_hold_time;
	u16			ss_hcnt;
	u16			ss_lcnt;
	u16			fs_hcnt;
	u16			fs_lcnt;
	u16			fp_hcnt;
	u16			fp_lcnt;
	u16			hs_hcnt;
	u16			hs_lcnt;
	int			(*acquire_lock)(void);
	void			(*release_lock)(void);
	bool			shared_with_punit;
	void			(*disable)(struct dw_i2c_dev *dev);
	void			(*disable_int)(struct dw_i2c_dev *dev);
	int			(*init)(struct dw_i2c_dev *dev);
	int			(*set_sda_hold_time)(struct dw_i2c_dev *dev);
	int			mode;
	struct i2c_bus_recovery_info rinfo;
	bool			suspended;
	struct pinctrl		*pinctrl;
	int			pinctrl_flag;
	struct mutex		lock;
#if defined CONFIG_XRING_I2C_DESIGNWARE
	int			ip_has_dma;
	u32			tx_cmd_intr_count;
	u32			rx_data_intr_count;
	u32			intr_status;
#if defined CONFIG_XRING_I2C_DMA
	resource_size_t		ip_paddr;
	dma_addr_t		fifo_dma_addr; /* phy address of the i2c IC_DATA_CMD register */
	u8			using_dma;
	struct xr_exteranl_dma	*exteranl_dma;
	u32			cmd_total_len;
	u32			rx_data_len;
	u32			cmd_dma_buf_size;
	struct completion	dma_complete;
	u32			xr_i2c_dma_en_cr;
#endif
	struct reset_control	*prst;
	bool			asic;
	volatile u32		irq_running;
	struct hwspinlock	*hwspin_lock;
	spinlock_t		irq_running_lock;
#endif

#if defined CONFIG_XRING_I2C_DEBUGFS
	struct dentry *xr_i2c_dentry;
#endif
};

#define ACCESS_NO_IRQ_SUSPEND	BIT(1)

int i2c_dw_init_regmap(struct dw_i2c_dev *dev);
u32 i2c_dw_scl_hcnt(u32 ic_clk, u32 tSYMBOL, u32 tf, int cond, int offset);
u32 i2c_dw_scl_lcnt(u32 ic_clk, u32 tLOW, u32 tf, int offset);
int i2c_dw_set_sda_hold(struct dw_i2c_dev *dev);
unsigned long i2c_dw_clk_rate(struct dw_i2c_dev *dev);
int i2c_dw_prepare_clk(struct dw_i2c_dev *dev, u8 flag);
int i2c_dw_acquire_lock(struct dw_i2c_dev *dev);
void i2c_dw_release_lock(struct dw_i2c_dev *dev);
int i2c_dw_wait_bus_not_busy(struct dw_i2c_dev *dev);
int i2c_dw_handle_tx_abort(struct dw_i2c_dev *dev);
int i2c_dw_set_fifo_size(struct dw_i2c_dev *dev);
u32 i2c_dw_func(struct i2c_adapter *adap);
void i2c_dw_disable(struct dw_i2c_dev *dev);
void i2c_dw_disable_int(struct dw_i2c_dev *dev);
int i2c_pinsctrl(struct dw_i2c_dev *dev, const char *name);
int i2c_irq_lock_clk_disable(struct dw_i2c_dev *dev, u32 state);
int i2c_irq_lock(struct dw_i2c_dev *dev, u32 state);
int xr_i2c_runtime_suspend(struct device *dev);
int xr_i2c_runtime_resume(struct device *dev);

static inline void __i2c_dw_enable(struct dw_i2c_dev *dev)
{
	regmap_write(dev->map, DW_IC_ENABLE, 1);
}

static inline void __i2c_dw_disable_nowait(struct dw_i2c_dev *dev)
{
	regmap_write(dev->map, DW_IC_ENABLE, 0);
}

void __i2c_dw_disable(struct dw_i2c_dev *dev);

int i2c_dw_configure_master(struct dw_i2c_dev *dev);
int i2c_dw_probe_master(struct dw_i2c_dev *dev);

static inline int i2c_dw_probe_lock_support(struct dw_i2c_dev *dev) { return 0; }
int i2c_dw_validate_speed(struct dw_i2c_dev *dev);


#if defined CONFIG_XRING_I2C_DMA
int xr_i2c_dma_init(struct dw_i2c_dev *dev);
void xr_i2c_release_dma(struct dw_i2c_dev *dev);

int xr_dma_xfer(struct dw_i2c_dev *dev);
int i2c_dma_unmap(struct dw_i2c_dev *dev);
int terminate_dma_transfer(struct dw_i2c_dev *dev);
#endif

#if defined CONFIG_XRING_I2C_DEBUGFS
#define DEBUGFS_CHAN_NAME_LEN		24
#define DEBUGFS_FILE_MASK		0644

int xr_i2c_debugfs_init(struct dw_i2c_dev *dev);
void xr_i2c_cleanup_debugfs(struct dw_i2c_dev *dev);
int i2c_hardware_spinlock(struct dw_i2c_dev *dev);
#endif
