#ifndef __MEMCHECK_CMA_H__
#define __MEMCHECK_CMA_H__

int memcheck_cma_createfs(struct proc_dir_entry *parent);
void memcheck_cma_report(char *name, unsigned long total, unsigned long req);
void memcheck_cma_info_show(void);

#endif // MEMCHECK_CMA_H
