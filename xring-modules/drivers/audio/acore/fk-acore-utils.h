/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _FK_ACORE_UTILS_
#define _FK_ACORE_UTILS_

#include "fk-acore.h"

#define AUDIO_USB_CONN_SESS		(0x8001)
#define AUDIO_TOOL_CONN_SESS	(0x8002)
#define AUDIO_COMMON_CONN_SESS	(0x8003)

enum {
	FK_BE_PORT_RX = 0,
	FK_BE_PORT_TX,
};

int fk_acore_adsp_codec_caps_check(struct acore_stream *ac_strm);
int fk_acore_get_adsp_codec_type(int format_type);
int fk_acore_get_bytes_per_sample(struct acore_stream *ac_strm);
int fk_compr_block_size_get(struct acore_stream *ac_strm, uint16_t frames_perblk);
int fk_compr_frames_perblk_get(struct acore_stream *ac_strm, uint32_t size);

int fk_acore_periods_size_check(struct acore_stream *ac_strm, uint32_t periods_size);

void fk_be_item_table_update(void *route_table, uint32_t items_num);
int fk_stream_port_info_update(struct acore_stream *ac_strm);
int fk_port_id_get_by_session(uint16_t session_id);
int fk_port_id_get_by_sess_dir(uint16_t session_id, uint16_t dir);
int fk_adsp_vote_register(uint16_t session_id);
int fk_adsp_vote_unregister(uint16_t session_id);
int fk_adsp_vote_reset(void);
int fk_adsp_vote_info_query(void);
int fk_adsp_vote_config(int flag);
bool fk_adsp_vote_config_query(void);
int fk_adsp_vote(bool enable);
int fk_adsp_power_status_get(void);


#endif //_FK_ACORE_UTILS_
