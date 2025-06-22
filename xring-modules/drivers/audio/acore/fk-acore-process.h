/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _FK_ACORE_PROCESS_
#define _FK_ACORE_PROCESS_

#include <linux/list.h>

#include "fk-acore.h"
#include "fk-acore-api.h"

/* data node structure */
struct data_list_node {
	struct list_head data_list;
	struct buf_blk p_buf;
};

/******************************* function define begin ***********************************/
void fk_acore_process_startup(struct acore_stream *ac_strm);

void fk_acore_process_start(struct acore_stream *ac_strm);
void fk_acore_process_stop(struct acore_stream *ac_strm);

int fk_acore_playback_buf_put(struct acore_stream *ac_strm, struct buf_blk p_buf);
int fk_acore_playback_data_flush(struct acore_stream *ac_strm);
void fk_acore_playback_data_trigger(void *priv);

int fk_acore_capture_buf_put(struct acore_stream *ac_strm, struct buf_blk p_buf);
int fk_acore_capture_data_flush(struct acore_stream *ac_strm);
void fk_acore_capture_data_trigger(void *priv);

int fk_acore_create_data_proc(struct acore_stream *ac_strm);
int fk_acore_destroy_data_proc(struct acore_stream *ac_strm);

/******************************* function define end *************************************/

#endif //_FK_ACORE_PROCESS_
