/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _OCM_WRAPPER_H__
#define _OCM_WRAPPER_H__

enum ocm_clk_rate_e {
	OCM_CLK_RATE_060V,
	OCM_CLK_RATE_065V,
	OCM_CLK_RATE_070V,
	OCM_CLK_RATE_080V,
	OCM_CLK_RATE_MAX,
};


enum ocm_buf_attr {
	XRING_ACPU = 0,
	XRING_NPU,
	XRING_ISP_DMA,
	XRING_DPU,
	XRING_MID_MAX,
};

struct ocm_buffer {
	unsigned long addr;
	unsigned int size;
};

struct ocm_buf_info {
	int fd;
	unsigned int size;
};

int ocm_clk_set_rate(enum ocm_clk_rate_e rate_e);

unsigned int ocm_clk_get_rate(void);

int ocm_clk_get_stats(void);

int ocm_buf_set_attr(struct ocm_buffer *buf, enum ocm_buf_attr attr);

int ocm_buf_clr_attr(struct ocm_buffer *buf, enum ocm_buf_attr attr);

struct ocm_buffer *ocm_buf_get(int fd);

void ocm_buf_put(int fd);

unsigned int ocm_query_avail_size(void);

int ocm_alloc(struct ocm_buffer *buf);

int ocm_free(struct ocm_buffer *buf);

#endif
