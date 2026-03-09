#ifndef __MEMCHECK_GPUMEM_H__
#define __MEMCHECK_GPUMEM_H__

int memcheck_gpumem_createfs(struct proc_dir_entry *parent);
void register_memcheck_kgsl_memstat(uint64_t (*cb)(const char *name));
void memcheck_gpumem_info_show(void);
void memcheck_gpumem_hook_meminfo(struct seq_file *s);
unsigned long memcheck_kgsl_get_total(void);

#endif // MEMCHECK_GPUMEM_H
