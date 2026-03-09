#ifndef __MEMCHECK_PROCESS_MEM_H__
#define __MEMCHECK_PROCESS_MEM_H__

void memcheck_top_pmem_info(unsigned int topN);
int memcheck_pmem_createfs(struct proc_dir_entry *parent);

#endif // MEMCHECK_PROCESS_MEM_H
