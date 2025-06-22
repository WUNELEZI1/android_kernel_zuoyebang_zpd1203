/* SPDX-License-Identifier: GPL-2.0 */
#ifndef XR_SPI_HEADER_H
#define XR_SPI_HEADER_H

#include <linux/bits.h>
#include <linux/completion.h>
#include <linux/debugfs.h>
#include <linux/irqreturn.h>
#include <linux/io.h>
#include <linux/scatterlist.h>
#include <linux/spi/spi-mem.h>
#include <linux/bitfield.h>
#include <linux/pinctrl/consumer.h>
#include <dt-bindings/xring/platform-specific/dw_apb_ssi.h>
#include <dt-bindings/xring/platform-specific/spi_dma_wrap.h>
#include <dt-bindings/xring/platform-specific/spi_dma_regif.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/hwspinlock.h>

#define SPI_DIV_MIN 2
#define SPI_DIV_MAX 65534
#define DW_SPI_ENDIANNESS               (1 << 25)
#define SPI_DMA_EN_B                    BIT(0)
#define SPI_DMA_REGIF_ET_INT_EN_B       0
#define SPI_DMA_REGIF_ET_INT_CLR_B      BIT(0)
#define SPI_DMA_INT_FLAG                0x01
#define SPI_DMA_ERR_MASK_EN_B           (1 << 1)

#define SPI_DMA_GIF_DEC_ERR             (1 << 0)
#define SPI_DMA_GIF_ADDR_ERR            (1 << 1)
#define SPI_DMA_APB_ERR                 (1 << 2)
#define SPI_DMA_FIFO_UD_ERR             (1 << 3)
#define SPI_DMA_FIFO_OV_ERR             (1 << 4)
#define DMA_ADDR_ALIGN_SIZE              64
#define DMA_LENGTH_ALIGN_SIZE            64

#define BUS_MAX_SPEED_HZ 50000000

#define GET_HARDWARE_TIMEOUT  1000

#define RX_BUSY		0
#define RX_BURST_LEVEL	16
#define TX_BUSY		1
#define TX_BURST_LEVEL	16

enum dw_ssi_type {
	SSI_MOTO_SPI = 0,
	SSI_TI_SSP,
	SSI_NS_MICROWIRE,
};

/* Slave spi_transfer/spi_mem_op related */
struct dw_spi_cfg {
	u8 tmode;
	u8 dfs;
	u32 ndf;
	u32 freq;
};

struct dw_spi;
struct dw_spi_dma_ops {
	int (*dma_init)(struct device *dev, struct dw_spi *dws);
	void (*dma_exit)(struct dw_spi *dws);
	int (*dma_setup)(struct dw_spi *dws, struct spi_transfer *xfer);
	bool (*can_dma)(struct spi_controller *master, struct spi_device *spi,
			struct spi_transfer *xfer);
	int (*dma_transfer)(struct dw_spi *dws, struct spi_transfer *xfer);
	void (*dma_stop)(struct dw_spi *dws);
};

enum xr_dma_type {
	INTERNAL_DMA = 0,
	EXTERNAL_DMA = 3,
};

enum spi_pinctrl_state {
	PINCTRL_DEFAULT = 0,
	PINCTRL_SLEEP,
};

enum spi_transfer_state {
	SPI_TRANSFER_UNINITIALIZED = 0,
	SPI_TRANSFER_PREPARED,
	SPI_TRANSFER_ERR,
	SPI_TRANSFER_COMPLETE,
};

struct dw_spi {
	struct spi_controller	*master;

	void __iomem		*regs;
	unsigned long		paddr;
	int			irq;
	u32			fifo_len;	/* depth of the FIFO buffer */
	unsigned int		dfs_offset;     /* CTRLR0 DFS field offset */
	u32			max_mem_freq;	/* max mem-ops bus freq */
	u32			max_freq;	/* max bus freq supported */

	u32			caps;		/* DW SPI capabilities */

	u32			reg_io_width;	/* DR I/O width in bytes */
	u16			bus_num;
	u16			num_cs;		/* supported slave numbers */
	void (*set_cs)(struct spi_device *spi, bool enable);

	/* Current message transfer state info */
	void			*tx;
	unsigned int		tx_len;
	void			*rx;
	unsigned int		rx_len;
	u8			buf[SPI_BUF_SIZE];
	int			dma_mapped;
	u8			n_bytes;	/* current is a 1/2 bytes op */
	irqreturn_t		(*transfer_handler)(struct dw_spi *dws);
	u32			current_freq;	/* frequency in hz */
	u32			cur_rx_sample_dly;
	u32			def_rx_sample_dly_ns;

	/* Custom memory operations */
	struct spi_controller_mem_ops mem_ops;

	/* DMA info */
	struct dma_chan		*txchan;
	u32			txburst;
	struct dma_chan		*rxchan;
	u32			rxburst;
	u32			dma_sg_burst;
	unsigned long		dma_chan_busy;
	dma_addr_t		dma_addr; /* phy address of the Data register */
	const struct dw_spi_dma_ops *dma_ops;
	struct completion	dma_completion;

#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs;
	struct debugfs_regset32 regset;
#endif
	struct device *dev;
	u32     dma_type;
	dma_addr_t   rx_dma;
	int      dma_complete_irq;
	struct hwspinlock *spi_hwspin_lock;
	spinlock_t lock;
	enum spi_transfer_state transfer_state;
	u32 hardware_lock;
	bool is_spi_dma;
	struct pinctrl *spi_pinctrl;
	struct pinctrl_state *pinctrl_state_default;
	struct pinctrl_state *pinctrl_state_sleep;
	u32 crg_div_min;
	u32 crg_div_max;
	bool is_fpga;
	u32			wait_timeout;
	int (*suspend)(struct device *dev, struct dw_spi *dws);
	int (*resume)(struct device *dev, struct dw_spi *dws);
	int (*runtime_suspend)(struct device *dev, struct dw_spi *dws);
	int (*runtime_resume)(struct device *dev, struct dw_spi *dws);
};

struct dw_spi_mmio {
	struct dw_spi  dws;
	struct clk     *clk;
	struct clk     *pclk;
	struct clk     *clk_div;
	void           *priv;
	struct reset_control *rstc;
	struct reset_control *prstc;
};

static inline u32 dw_readl(struct dw_spi *dws, u32 offset)
{
	return __raw_readl(dws->regs + offset);
}

static inline void dw_writel(struct dw_spi *dws, u32 offset, u32 val)
{
	__raw_writel(val, dws->regs + offset);
}

static inline u32 dw_read_io_reg(struct dw_spi *dws, u32 offset)
{
	switch (dws->reg_io_width) {
	case 2:
		return readw_relaxed(dws->regs + offset);
	case 4:
	default:
		return readl_relaxed(dws->regs + offset);
	}
}

static inline void dw_write_io_reg(struct dw_spi *dws, u32 offset, u32 val)
{
	switch (dws->reg_io_width) {
	case 2:
		writew_relaxed(val, dws->regs + offset);
		break;
	case 4:
	default:
		writel_relaxed(val, dws->regs + offset);
		break;
	}
}

static inline void spi_enable_chip(struct dw_spi *dws, int enable)
{
	dw_writel(dws, DW_SPI_SSIENR, (enable ? 1 : 0));
}

/* Disable IRQ bits */
static inline void spi_mask_intr(struct dw_spi *dws, u32 mask)
{
	u32 new_mask;

	new_mask = dw_readl(dws, DW_SPI_IMR) & ~mask;
	dw_writel(dws, DW_SPI_IMR, new_mask);
}

/* Enable IRQ bits */
static inline void spi_umask_intr(struct dw_spi *dws, u32 mask)
{
	u32 new_mask;

	new_mask = dw_readl(dws, DW_SPI_IMR) | mask;
	dw_writel(dws, DW_SPI_IMR, new_mask);
}

/*
 * This disables the SPI controller, interrupts, clears the interrupts status
 * and CS, then re-enables the controller back. Transmit and receive FIFO
 * buffers are cleared when the device is disabled.
 */
static inline void spi_reset_chip(struct dw_spi *dws)
{
	spi_enable_chip(dws, 0);
	spi_mask_intr(dws, 0xff);
	dw_readl(dws, DW_SPI_ICR);
	dw_writel(dws, DW_SPI_SER, 0);
	spi_enable_chip(dws, 1);
}

static inline void spi_shutdown_chip(struct dw_spi *dws)
{
	spi_enable_chip(dws, 0);
	dw_writel(dws, DW_SPI_BAUDR, 0);
}

static inline void xr_spi_dma_iowrite64(struct dw_spi *dws,
			u32 reg, dma_addr_t val)
{
	dw_writel(dws, reg, lower_32_bits(val));
	dw_writel(dws, reg + 4, upper_32_bits(val));
}

static inline void xr_spi_dma_enable(struct dw_spi *dws)
{
	dw_writel(dws, SPI_DMA_REGIF_SPI_DMA_EN, SPI_DMA_EN_B);
}

static inline void xr_spi_dma_irq_enable(struct dw_spi *dws)
{
	dw_writel(dws, SPI_DMA_REGIF_INT_SPI_DMA_IRQ_INT_MASK,
		SPI_DMA_REGIF_ET_INT_EN_B);
}

static inline void xr_spi_dma_irq_disable(struct dw_spi *dws)
{
	dw_writel(dws, SPI_DMA_REGIF_INT_SPI_DMA_IRQ_INT_MASK,
		~SPI_DMA_REGIF_ET_INT_EN_B);
}

static inline void xr_spi_dma_irq_clear(struct dw_spi *dws)
{
	dw_writel(dws, SPI_DMA_REGIF_INT_SPI_DMA_IRQ_INT_RAW,
		SPI_DMA_REGIF_ET_INT_CLR_B);
}

static inline u32 xr_spi_dma_irq_read(struct dw_spi *dws)
{
	return dw_readl(dws, SPI_DMA_REGIF_INT_SPI_DMA_IRQ_INT_STATUS);
}

int spi_runtime_resume(struct device *dev);
int spi_runtime_suspend(struct device *dev);
int dw_spi_check_status(struct dw_spi *dws, bool raw);
int dw_spi_add_host(struct device *dev, struct dw_spi *dws);
#if IS_ENABLED(CONFIG_XRING_SPI)
int dw_spi_update_config(struct dw_spi *dws, struct spi_device *spi, struct dw_spi_cfg *cfg);
#endif
int dw_spi_setup(struct spi_device *spi);
int dw_spi_prepare_transfer_hardware(struct spi_master *master);
int dw_spi_unprepare_transfer_hardware(struct spi_master *master);
void dw_spi_set_cs(struct spi_device *spi, bool enable);
void dw_spi_remove_host(struct dw_spi *dws);
void spi_hw_init(struct device *dev, struct dw_spi *dws);
void dw_spi_cleanup(struct spi_device *spi);
void dw_spi_handle_err(struct spi_controller *master,
		struct spi_message *msg);
irqreturn_t dw_spi_transfer_handler(struct dw_spi *dws);
void dw_spi_dma_setup_generic(struct dw_spi *dws);
int dw_spi_dma_wait_rx_done(struct dw_spi *dws);
bool dw_spi_can_dma(struct spi_controller *master,
			   struct spi_device *spi, struct spi_transfer *xfer);
int dw_spi_dma_submit_rx(struct dw_spi *dws, struct scatterlist *sgl,
				unsigned int nents);
int xr_spi_add_host(struct device *dev, struct dw_spi *dws);
irqreturn_t xr_spi_dma_irq_handler(int irq, void *dev_id);
void xr_spi_dma_setup_generic(struct dw_spi *dws);
extern int xr_tui_resource_release(struct spi_device *spidev);
extern int xr_tui_resource_request(struct spi_device *spidev);

#endif /* XR_SPI_HEADER_H */
