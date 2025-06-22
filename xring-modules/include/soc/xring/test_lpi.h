// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __TEST_LPI_H__
#define __TEST_LPI_H__

#if IS_ENABLED(CONFIG_XRING_DEBUG)
bool lpi_is_enabled(unsigned int cpu);
bool its_command_is_valid(void);
bool all_operations_are_in_process(unsigned int cpu);
#endif

#endif
