#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <net/snmp.h>

#include "ftun.h"

#ifdef CONFIG_PROC_FS
struct linux_ftun_mib *ftun_statistics;

static const struct snmp_mib ftun_mib_list[] = {
	SNMP_MIB_ITEM("FtunCurrTxSw", LINUX_MIB_FTUNCURRTXSW),
	SNMP_MIB_ITEM("FtunCurrRxSw", LINUX_MIB_FTUNCURRRXSW),
	SNMP_MIB_ITEM("FtunTxSw", LINUX_MIB_FTUNTXSW),
	SNMP_MIB_ITEM("FtunRxSw", LINUX_MIB_FTUNRXSW),
	SNMP_MIB_ITEM("FtunDecryptError", LINUX_MIB_FTUNDECRYPTERROR),
	SNMP_MIB_ITEM("FtunDecryptRetry", LINUX_MIB_FTUNDECRYPTRETRY),
	SNMP_MIB_ITEM("FtunRxNoPadViolation", LINUX_MIB_FTUNRXNOPADVIOL),
	SNMP_MIB_SENTINEL
};

static int ftun_statistics_seq_show(struct seq_file *seq, void *v)
{
	unsigned long buf[LINUX_MIB_FTUNMAX] = {};
	// struct net *net = seq->private;
	int i;

	snmp_get_cpu_field_batch(buf, ftun_mib_list, ftun_statistics);
	for (i = 0; ftun_mib_list[i].name; i++)
		seq_printf(seq, "%-32s\t%lu\n", ftun_mib_list[i].name, buf[i]);

	return 0;
}
#endif  /* CONFIG_PROC_FS */

int __net_init ftun_proc_init(struct net *net)
{
#ifdef CONFIG_PROC_FS
	ftun_statistics = alloc_percpu(struct linux_ftun_mib);
	if (!ftun_statistics)
		return -ENOMEM;

	if (!proc_create_net_single("ftun_stat", 0444, net->proc_net,
				    ftun_statistics_seq_show, NULL))
		goto err_free_stats;
#endif /* CONFIG_PROC_FS */

	return 0;

err_free_stats:
	free_percpu(ftun_statistics);
	return -ENOMEM;
}

void __net_exit ftun_proc_fini(struct net *net)
{
	remove_proc_entry("ftun_stat", net->proc_net);
	free_percpu(ftun_statistics);
}
