/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _FK_COMMON_CTRL_
#define _FK_COMMON_CTRL_

#include <sound/soc.h>
#include "../acore/fk-acore-define.h"
#include "../acore/fk-acore-api.h"

int fk_loopback_type_query(void);
int fk_audif_fmtbit_query(void);
int fk_audio_tdm_fmtchs_query(void);
int fk_audif_fmtchs_query(void);
int fk_voice_type_query(void);
int fk_codec_type_query(void);
int fk_bt_config_query(struct bt_codec_config *bt_config);
int fk_ec_ref_ch_query(void);
int fk_ec_ref_bit_format_query(void);
int fk_ec_ref_sample_rate_query(void);
int fk_ec_ref_type_query(void);
int fk_ec_type_from_val(int ec_val);
int fk_adsp_reset_done_query(void);
void fk_adsp_reset_done_set(int adsp_fixed);
void fk_device_type_set(int device_type);

void fk_common_controls_add(struct snd_soc_component *component);

#endif //_FK_COMMON_CTRL_
