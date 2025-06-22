/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_EDR_IPC_H__
#define __XRING_EDR_IPC_H__

struct EdrBufInfo {
	uint32_t size;
	uint64_t hwaddr;
};

struct TimeZone {
	uint32_t month;
	uint32_t day;
	uint32_t hour;
};

struct EdrFaultInject {
	uint8_t  cmd;
	uint8_t  res;
	uint8_t sensorid;
	uint8_t interfaceid;
	char   param[12];
};

struct EdrIpcAp2Isp {
	uint64_t msgid;
	uint32_t opcode;
	union {
		struct EdrBufInfo buf_info;
		struct TimeZone edrtimezone;
		struct EdrFaultInject faultinject;
		int loglevel;
	};
};

#define EDR_CMD_NR		    (0x18)
#define EDR_CMD_COREDUMP	    _IOW('E', EDR_CMD_NR + 0, struct EdrIpcAp2Isp)
#define EDR_CMD_EXIT		    _IOW('E', EDR_CMD_NR + 1, struct EdrIpcAp2Isp)
#define EDR_CMD_LOGDONE		    _IOW('E', EDR_CMD_NR + 2, struct EdrIpcAp2Isp)
#define EDR_CMD_FAULT_INJECT_CANCEL _IOW('E', EDR_CMD_NR + 3, struct EdrIpcAp2Isp)
#define EDR_CMD_TIMEZONE	    _IOW('E', EDR_CMD_NR + 4, struct EdrIpcAp2Isp)
#define EDR_CMD_DUMP_MDR_LOG	    _IOW('E', EDR_CMD_NR + 5, char *)
#define EDR_CMD_FW_LOG_LEVEL	    _IOW('E', EDR_CMD_NR + 6, struct EdrIpcAp2Isp)

//define OPCODE，if have new add here
#define EDR_MSG_OPCODE_COREDUMP   0x1000
#define EDR_MSG_OPCODE_RAMTRACE_START	0x1001
#define EDR_MSG_OPCODE_RAMTRACE_STOP	0x1002
#define EDR_MSG_OPCODE_FAULT_INJECT_CANCEL 0x1007

//add for edr exit
#define EDR_EXIT                0x1234
#define EDR_POWER_ON            0x2345
#define EDR_FAULT_INJECT_ACK    0x3456

void edr_ipc_rcv(void);
void edr_ipc_exit(void);
int edr_ipc_send(void *data, unsigned int len);

#endif
