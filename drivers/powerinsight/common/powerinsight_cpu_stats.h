#ifndef POWERINSIGHT_CPU_STATS_H
#define POWERINSIGHT_CPU_STATS_H
#include <linux/sched.h>

void powerinsight_proc_cputime_init(void);
void powerinsight_proc_cputime_exit(void);
long powerinsight_ioctl_cpu(unsigned int cmd, void __user *argp);

#endif // POWERINSIGHT_CPU_STATS_H
