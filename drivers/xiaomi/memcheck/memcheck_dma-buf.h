#ifndef __MEMCHECK_ION_H__
#define __MEMCHECK_ION_H__


int memcheck_dmabuf_createfs(struct proc_dir_entry *parent);
void memcheck_dmabuf_info_show(void);
unsigned long memcheck_dmabuf_get_total(void);

#endif /* _MEMCHECK_ION_H */
