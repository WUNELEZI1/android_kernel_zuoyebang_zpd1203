#ifndef __MEMCHECK_VMALLOC_H__
#define __MEMCHECK_VMALLOC_H__


void memcheck_vmalloc_info_show(void);
int memcheck_vmalloc_createfs(struct proc_dir_entry *parent);

#endif // MEMCHECK_VMALLOC_H
