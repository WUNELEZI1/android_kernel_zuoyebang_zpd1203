/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_CPU_IDLE_H__
#define __XRING_CPU_IDLE_H__

#define IDLE_DEPTH_MAX 10

enum idle_client {
	IDLE_CLIENT_BOOST,
	IDLE_CLIENT_USER,
	IDLE_CLIENT_MAX,
};

void cpuidle_set_allowed_depth(unsigned int cpu, unsigned int depth, int client);


#endif
