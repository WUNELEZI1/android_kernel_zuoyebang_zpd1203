// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/kernel.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <trace/hooks/cpuinfo.h>

static const char * const features[] = {" evtstrm", " dgh bti ecv afp wfxt"};
static size_t feature_lengths[ARRAY_SIZE(features)];
static size_t num_features = ARRAY_SIZE(features);

static size_t cpuinfo_remove_features(char *str)
{
	char *end = str + strlen(str);
	char *current_pos = str;

	while (current_pos < end) {
		for (size_t i = 0; i < num_features; ++i) {
			if (strncmp(current_pos, features[i], feature_lengths[i]) == 0) {
				char *feature_end = current_pos + feature_lengths[i];

				memmove(current_pos, feature_end, end - feature_end);
				end -= feature_lengths[i];
				current_pos--;
				break;
			}
		}

		current_pos++;
	}

	*end = '\0';
	return end - str;
}

static void cpuinfo_show_hook(void *unused, struct seq_file *m)
{
	if (m->count > 4096)
		m->count = cpuinfo_remove_features(m->buf);
}

void cpuinfo_trim_init(void)
{
	for (size_t i = 0; i < num_features; ++i)
		feature_lengths[i] = strlen(features[i]);

	register_trace_android_rvh_cpuinfo_c_show(cpuinfo_show_hook, NULL);
}
