#ifndef __MEMCHECK_ACCOUNT_H__
#define __MEMCHECK_ACCOUNT_H__

int memcheck_createfs(void);
void memcheck_warn_alloc_show_mem(void);
void memcheck_memstat_report(void);
void memcheck_page_slow_report(gfp_t *gfp_mask, unsigned int order, u64 stime,
								unsigned long pages_reclaimed, int retry_loop_count);


#endif /* _MEMCHECK_ACCOUNT_H */
