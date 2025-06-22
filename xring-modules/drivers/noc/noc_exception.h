/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 *
 * Description: noc exception header
 */

#ifndef __NOC_EXCEPTION_H__
#define __NOC_EXCEPTION_H__

#include <linux/types.h>

struct noc_dma_exception_s;

void set_noc_exception_list(u32 mid, u32 err_code);
void noc_exception_process(void);
void noc_exception_wq_init(void);

#if IS_ENABLED(CONFIG_XRING_NOC_DMA_EXCEPTION)
void noc_dma_exception(struct noc_dma_exception_s *e, const char *mid_name,
			u32 mid, u64 targetaddr, u32 opc);
void noc_dma_process(struct noc_dma_exception_s *e);
int noc_dma_process_init(struct noc_dma_exception_s *e);
int noc_dma_process_exit(struct noc_dma_exception_s *e);
#else
static inline void noc_dma_exception(struct noc_dma_exception_s *e, const char *mid_name,
			u32 mid, u64 targetaddr, u32 opc) {}
static inline void noc_dma_process(struct noc_dma_exception_s *e) {}
static inline int noc_dma_process_init(struct noc_dma_exception_s *e)
{
	return 0;
}
static inline int noc_dma_process_exit(struct noc_dma_exception_s *e)
{
	return 0;
}
#endif

#endif /* __NOC_EXCEPTION_H__ */
