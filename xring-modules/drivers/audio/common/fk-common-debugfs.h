/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _FK_COMMON_DEBUGFS_
#define _FK_COMMON_DEBUGFS_


#include <linux/types.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/string.h>

#include "../acore/fk-acore.h"
#include "../asoc/codec/xring_codec/madera/mfd/madera.h"
#include "fk-audio-tool-core.h"
#include "fk-audio-tool-user.h"

int audio_debugfs_init(struct acore_client *ac);
void audio_debugfs_deinit(struct acore_client *ac);
extern struct dump_blk dump_blk_g;

#endif //_FK_COMMON_DEBUGFS_

