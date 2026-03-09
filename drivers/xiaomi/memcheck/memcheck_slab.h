#ifndef __MEMCHECK_SLAB_H__
#define __MEMCHECK_SLAB_H__

static const char *const slubobj_ignore_list[] = {
	"f2fs_inode_cache",
	"kmemleak_object",
};

void memcheck_slab_info_show(void);
int memcheck_slab_createfs(struct proc_dir_entry *parent);
void memcheck_slab_alloc_node_report(void *object, unsigned long addr, struct kmem_cache *s);

#endif // MEMCHECK_SLAB_H
