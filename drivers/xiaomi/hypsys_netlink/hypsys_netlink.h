#ifndef HYPSYS_NETLINK_H
#define HYPSYS_NETLINK_H

// #define pr_fmt(fmt) "hypsys_netlink: "fmt

#define HYPSYS_MAX_BUF_SIZE 512

#define FORMAT_COMMA_BRACE 2	// {}
#define FORMAT_COMMA_SIZE 1	// ,
#define FORMAT_COLON_SIZE 1	// :
#define FORMAT_QUOTES_SIZE 2 // ""

#define hypsys_err(fmt, ...) \
	pr_err(KBUILD_MODNAME "[%s %d]: " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define hypsys_info(fmt, ...) \
	pr_info(KBUILD_MODNAME "[%s %d]: " fmt, __func__, __LINE__, ##__VA_ARGS__)

/**
 * CMA_REPORT: cma alloction failure
 * LOWMEM_REPORT: lowmem alloction failure
 * MEMSTAT_REPORT: memstat
*/
enum report_type {
	CMA_REPORT = 1,
	PAGESLOW_REPORT,
	MEMSTAT_REPORT,
	SLAB_REPORT,
	PMEM_REPORT,
	MAX_REPORT,
};

struct report_event {
	struct list_head head;
	enum report_type msg_type;
	unsigned int msg_len;
	char *msg_buf;
};

struct report_event *hypsys_event_alloc(enum report_type type);
int hypsys_event_add_str(struct report_event *event,
							const char *key, const char *value);
int hypsys_event_add_int(struct report_event *event,
							const char *key, const long value);
bool hypsys_event_report(struct report_event *event);
void hypsys_event_destroy(struct report_event *event);

#endif // HYPSYS_NETLINK_H