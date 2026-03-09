#ifndef POWERINSIGHT_IOCTL_H
#define POWERINSIGHT_IOCTL_H

#include <linux/ioctl.h>
#include <linux/sched.h>

#define PREFIX_LEN 32
#define NAME_LEN (PREFIX_LEN + TASK_COMM_LEN)
#define MAX_STAT_ENTRIES 1000

#define POWERINSIGHT_DIR_IOC(dir, type, nr, param) \
    _IOC(dir, POWERINSIGHT_IOC_##type, nr, sizeof(param))

#define POWERINSIGHT_CPU_DIR_IOC(dir, nr, param) \
    POWERINSIGHT_DIR_IOC(dir, CPU, nr, param)
#define POWERINSIGHT_GPU_DIR_IOC(dir, nr, param) \
    POWERINSIGHT_DIR_IOC(dir, GPU, nr, param)
#define POWERINSIGHT_UTILS_DIR_IOC(dir, nr, param) \
	POWERINSIGHT_DIR_IOC(dir, UTILS, nr, param)

enum powerinsight_ioctl_type_t {
	POWERINSIGHT_IOC_GPU = 0,
	POWERINSIGHT_IOC_CPU = 1,
	POWERINSIGHT_IOC_UTILS = 2,
};

struct powerinsight_cputime {
	uid_t uid;
	pid_t pid;
	unsigned long long time;
	unsigned long long power;
	unsigned char cmdline;
	char name[NAME_LEN];
} __packed;

struct powerinsight_cpu_stats {
	int count;
	struct powerinsight_cputime entries[MAX_STAT_ENTRIES];
};

struct ioctl_memfd_request {
    void __user *user_ptr; // mmap 映射地址
};

#endif // POWERINSIGHT_IOCTL_H
