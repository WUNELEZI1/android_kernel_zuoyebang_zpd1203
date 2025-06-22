/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __NOC_H__
#define __NOC_H__

#include <linux/clk.h>
#include <linux/interrupt.h>

/*logging register val*/
#define ERRCLR		0x1
#define FAULTEN		0x1
#define IRQ_VALID       1
#define MID_MASK	0xff
#define MAINBUS_MID_MASK	0xff00
#define MAINBUS_MID_SHIFT	0x8
#define SRCATTR_MASK	0xF800000
#define SRCATTR_SHIFT	23
#define MID_MIN		0x1
#define MID_MAX		0xDF
#define ERRCODE_MAX	0x6
#define OPC_MAX		0x9
#define TRACE_ARG_NUM	6

/* packet probe global register */
#define NOC_MAINCTL		0x08
#define NOC_CFGCTL		0x0c
#define NOC_FILTERLUT		0x14
#define FILTERLUT_VAL		0xffffffff
#define FILTERLUT_FILTER_VAL	0xaaaa
#define ROUTEIDMASK		0x84
#define ROUTEIDMASK_VAL		0
#define WINDOWSIZE_VAL		0x3f
#define WINDOWSIZE		0x90
#define OPC			0x9c
#define OPC_VAL			0xf
#define STATUS			0xa0
#define STATUS_VAL		0x3
#define LENGTH			0xa4
#define	LENGTH_VAL		0xf
#define SE_MASL_VAL		0
#define SE_MASK			0x98
#define	URGENCY			0xa8
#define URGENCY_VAL		0
#define USERMID			0xac
#define USERMID_VAL		0x0
#define USERMASK		0xb0
#define USERMASK_VAL		0x0

#define MAIN_TRACE_SIZE		0x4000
#define LPIS_TRACE_SIZE		0x1000

#define ROUTEIDBASE	0x80
#define ADDRBASE_LOW	0x88
#define ADDRBASE_HIGH	0x8c
#define SE_BASE	0x94

/* MAINCTL */
#define ERREN		1
#define TRACE_DISABLE	0
#define TRACEEN		0x2
#define PAYLOADEN	0x4
#define GLOBALEN	0x1

#define BUS_NOIDLE	0
#define NOMASK	0xffff0000
#define MASK_L	0x8
#define MASK_H	0xc
#define EXC0_RAW_ST	0x4
#define MASK_SIZE	0x100

#define MAIN_BUS_NUM	0
#define MAIN_CPU_NUM	1
#define MAIN_GPU_NUM	2
#define SYSCFG0_NUM	4
#define SYSCFG1_NUM	5
#define SYSCFG2_NUM	6
#define SYSCFG3_NUM	7
#define PERI_NUM	8
#define XRSE_NUM	11
#define CPU_NUM		12
#define LPIS1_NUM	15
#define	LPIS0_NUM	16
#define RESERVE_ADD	0xec00b000
#define RESERVE_ADD_SIZE	0x100

#define	ISP_BW_SHIFT	0x700
#define VENC_LATENCY_SHIFT	0x700
#define DPU_LATENCY_SHIFT	0x700
#define MAIN_BUS_PORT(port_id)	(0x2000 + port_id * 0x400)
#define LPIS_BUS_PORT(port_id)	(0x800 + port_id * 0x400)
#define FILTER_LEN	120
#define SMC_OK		0
#define DDR_32G    0x800000000
#define DDR_33G    0x840000000
#define INITLOCAL_MASK 0xFFFFFFFFF
#define SE_BASE_VALUE	0
#define SE_MASK_VALUE	0
#define ADDR_SHIFT	32
#define MAIN_BUS_ROUTEIDMASK	0x1FFF800
#define LPIS_BUS_ROUTEIDMASK	0xFFF00
#define DDR_32G	0x800000000
#define DDR_33G	0x840000000
#define INITLOCAL_MASK	0xFFFFFFFFF

enum errlog6_num {
	ERRLOG6_MAIN_BUS = 0,
	ERRLOG6_MAIN_BUS_CPU,
	ERRLOG6_MAIN_BUS_GPU,
	ERRLOG6_CPU_BUS
};

enum errlog4_num {
	ERRLOG4_SYSCFG0 = 0,
	ERRLOG4_SYSCFG1,
	ERRLOG4_SYSCFG2,
	ERRLOG4_SYSCFG3,
	ERRLOG4_SYS0
};

#if IS_ENABLED(CONFIG_XRING_NOC_DMA_EXCEPTION)
#include <linux/kfifo.h>
#include <soc/xring/dmd.h>

#define DMD_FIFO_SIZE 8

struct noc_dma_info {
	const char *name;
	u32 mid;
	u64 targetaddr;
	u32 opc;
};

struct noc_dma_exception_s {
	struct workqueue_struct *wq;
	struct work_struct work;
	struct dmd_client *dmd_client;
	DECLARE_KFIFO(msg_fifo, struct noc_dma_info, DMD_FIFO_SIZE);
	spinlock_t lock;
	bool is_inited;
};
#endif

struct noc_dev {
	struct device		*dev;
	struct noc_platdata	*parseddata;
	int	irq;
	int	id;
	void __iomem	*regs[29];
	void __iomem	*trace_regs[2];
	void __iomem    *idle_regs[29];
	struct noc_timeout	*timeout;
	struct noc_errlog	*errlog_reg;
	char	*bufs;
	struct clk	*clk;
	u32	bus_array;
	u32	bus_num;
	u32	bus_info_max;
	u32	bus_intr_exc0;
	u32	remap_ddrstartl;
	u32	remap_ddrstarth;
	u64     remap_ddrstart;
	u32	remap_ddrendl;
	u32	remap_ddrendh;
	u64     remap_ddrend;
	u32	ddr_maxl;
	u32	ddr_maxh;
	u64     ddr_max;
	u32     main_bus_routeidmask;
	u32     lpis_bus_routeidmask;
	u32	main_bus_trace;
	u32	lpis_bus_trace;
	struct work_struct	work;
	void __iomem	*err_irq_reg;
	u32	tmc_etr_sts;
	u32	no_log4_num;
	u32	log6_num;
	int	main_bus_info_count;
	int	mid_max;
	u32     main_bus_info[3];
	u32	no_errlog4_bus[7];
	u32	errlog6_bus[7];
	u32	bus_idle_addr[29];
	u32	bus_idle_mask[29];
	u32	idle_shift[29];
#if IS_ENABLED(CONFIG_XRING_NOC_DMA_EXCEPTION)
	struct noc_dma_exception_s noc_dma_s;
#endif
};

struct statistic {
	void *isp_bus_base;
	void *venc_bus_base;
	void *dpu_bus_base;
};

enum trace_bus_num {
	MAIN_NUM = 0,
	LPIS_NUM = 1
};

enum lpis_bus_port {
	LPIS_PORT1 = 0,
	LPIS_PORT0 = 1
};

enum main_bus_port {
	MAIN_P0_R = 0,
	MAIN_P1_R = 1,
	MAIN_P2_R = 2,
	MAIN_P3_R = 3,
	MAIN_P0_W = 4,
	MAIN_P1_W = 5,
	MAIN_P2_W = 6,
	MAIN_P3_W = 7
};

struct noc_errlog {
	u32 errvld;
	u32 errlog0;
	u32 errlog1;
	u32 errlog2;
	u32 errlog3;
	u32 errlog4;
	u32 errlog5;
	u32 errlog6;
	u32 errlog7;
};

struct noc_platdata {
	u32 init_num;
	u32 target_num;
	u32 sub_num;
	u32 sub_array;

	u64 init_flow;
	u64 target_flow;
	u32 subrange;
	u64 target_addr;
	u64 init_addr;

	u32 reg_add;
	char *bus_name[29];
	char *init_flow_string;
	char *targ_flow_string;
	u32 errcode_num;
	u32 mid_num;
	char *mid_name[MID_MAX];
	u32 cpuid;
	u32 opc;
	char *errcode[20];
};

typedef struct bus_type_s {
	enum lpis_bus_port lpis_bus_port;
	enum main_bus_port main_bus_port;
} bus_type_t;

typedef struct noc_trace_s {
	enum trace_bus_num bus_num;
	u32 bus_port;
	u32 routeidmask;
	u32 noc_bus_num;
} noc_trace_t;

typedef struct init_info_s {
	u32 init_shift;
	u32 init_mask;
} init_info_t;

typedef struct target_info_s {
	u32 target_shift;
	u32 target_mask;
} target_info_t;

typedef struct sub_info_s {
	u32 sub_shift;
	u32 sub_mask;
} sub_info_t;

typedef struct mid_info_s {
	u32 mid_shift;
	u32 mid_mask;
} mid_info_t;

typedef struct bus_info_s {
	char *init_flow_string;
	u32 init_flow;
	char *targ_flow_string;
	u32 targ_flow;
	u32 targ_sub;
	u64 init_addr;
	u64 targ_addr;
} bus_info_t;

struct trace_filter_s {
	enum trace_bus_num bus_num;
	u32 port_id;
	u32 payload_flags;
	u32 routeid_base;
	u64 start_addr;
	u64 end_addr;
};

struct routeid_s {
	u32 init_id;
	u32 target_id;
	u32 sub_id;
	u32 bus_info_max;
};

extern const bus_info_t *bus_info[];
extern sub_info_t sub_info[];
extern target_info_t target_info[];
extern init_info_t init_info[];
extern mid_info_t mid_info[];
extern u32 g_info_size[];
struct noc_dev *get_noc(void);
void *get_isp_bus_base(void);
void *get_venc_bus_base(void);
void *get_dpu_bus_base(void);
struct noc_errlog *noc_getdata(void);
int noc_trace_init(void);
long get_targ_addr(struct noc_dev *noc_dev, int bus_num, struct routeid_s *routeid,
		u32 *bus_info_index);
#ifdef CONFIG_XRING_NOC_DDR_DEBUG
int noc_addr_debug_init(void);
bool check_noc_panic_enable(void);
#endif

#endif /* __NOC_H__ */
