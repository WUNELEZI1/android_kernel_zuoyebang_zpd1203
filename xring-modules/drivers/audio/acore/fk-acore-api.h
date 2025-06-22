/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _FK_ACORE_API_
#define _FK_ACORE_API_

#include "../asoc/fk-dai-fe.h"

/*********************************** macro define begin **********************************/
#define SMEM_HEAD_SIZE					(32)
#define SMEM_MIN_SIZE					(4096)

#define PCM_PLAYBACK_DIRECT				XR_DIRECT
#define PCM_PLAYBACK_NORMAL_1			XR_MEDIA3
#define PCM_PLAYBACK_NORMAL				XR_MEDIA2
#define PCM_PLAYBACK_LOWLATENCY			XR_MEDIA1
#define PCM_PLAYBACK_ULTRA_LOWLATENCY	XR_MEDIA0

#define PCM_CAPTURE_NORMAL_1			XR_MEDIA3
#define PCM_CAPTURE_NORMAL				XR_MEDIA2
#define PCM_CAPTURE_LOWLATENCY			XR_MEDIA1
#define PCM_CAPTURE_ULTRA_LOWLATENCY	XR_MEDIA0

#define PCM_CAPTURE_VOICE_TRIGGER		XR_LSM
#define PCM_CAPTURE_ASR					XR_ASR

#define CHANNEL_NUM_MAX					(16)

/* set wait_for_avail to 300 ms */
#define PCM_WAIT_TIME					(100)
#define VOLUME_VALUE_MAX				(0x1000)

/*********************************** macro define end **********************************/

/*********************************** enum define begin ***********************************/
enum scene_type {
	SCENE_PCM_PLAYBACK = 0x0,
	SCENE_PCM_CAPTURE,
	SCENE_COMPR_PLAYBACK,
	SCENE_COMPR_CAPTURE,
	SCENE_VOIP_PLAYBACK,
	SCENE_VOIP_CAPTURE,
	SCENE_VOICE_PLAYBACK,
	SCENE_VOICE_CAPTURE,
	SCENE_HPF,
	SCENE_LOOPBACK,
	SCENE_HAPTIC,
	SCENE_SPATIAL,
	SCENE_MMAP_PLAYBACK,
	SCENE_MMAP_CAPTURE,
	SCENE_KARAOKE_PLAYBACK,
	SCENE_KARAOKE_CAPTURE,
	SCENE_INCALL_PLAYBACK,
	SCENE_INCALL_CAPTURE,
	SCENE_HF_PLAYBACK,
	SCENE_HF_CAPTURE,
	SCENE_MAX,
};

enum op_code {
	OP_START = 0x0,
	OP_PAUSE,
	OP_RESUME,
	OP_FLUSH,
	OP_STOP,
	OP_DRAIN,
	OP_NEXT_TRACK,
	OP_REGISTER = 0x20,
	OP_UNREGISTER,
	OP_STARTUP,
	OP_SET_PARAMS,
	OP_GET_PARAMS,
	OP_PLAYBACK_COPY,
	OP_CAPTURE_COPY,
	OP_VOICE_RECORD = 0x40,
	OP_VOICE_PLAY,
	/* indicate adsp poweron state */
	OP_ADSP_POWERON = 0x60,
	/* indicate adsp shutdown state */
	OP_ADSP_SHUTDOWN,
};

/* define parameter type which audio core support */
enum params_type {
	PARAMS_NONE = 0x0,
	PARAMS_SET_DEC_INFO,
	PARAMS_GET_TIMESTAMP,
	PARAMS_GAPLESS_ENABLE,
	PARAMS_GAPLESS_INIT_SAMPS,
	PARAMS_GAPLESS_TRAIL_SAMPS,
	PARAMS_SET_KWS_MODULE_INFO,
	PARAMS_DEL_KWS_MODULE_INFO,
	PARAMS_KWS_EVENT_STATUS,
	PARAMS_KWS_STOP_RECOGNITON,
	PARAMS_KWS_START_RECOGNITON,
	PARAMS_KWS_EC_REF,
	PARAMS_MMAP_HW_PRT,
	PARAMS_SET_VOICE_PCIE_BAR,
	PARAMS_SET_VOICE_BANDWIDTH,
	PARAMS_SET_VT_FCA_FLAG,
	PARAMS_SET_FCA_EVENT,
};

enum chan_info {
	CHAN_NONE = 0x0,
	CHAN_1,
	CHAN_2,
	CHAN_3,
	CHAN_4,
	CHAN_5,
	CHAN_6,
	CHAN_7,
	CHAN_8,
};

/*********************************** enum define end *************************************/


/*********************************** struct define begin *********************************/
struct voice_pcie_bar {
	uint64_t BAR0_1_addr;
	uint64_t BAR2_3_addr;
};

struct stream_format {
	/* data sample rate */
	uint32_t samples;
	/* channel numbers */
	uint32_t channels;
	/* the number of valid bits for each sample */
	uint32_t bit_width;
	/* codec type */
	uint32_t codec_type;
};

struct dai_format {
	/* data sample rate */
	uint32_t samples;
	/* the number of valid bits for each sample */
	uint32_t bit_width;
	/* channel numbers */
	uint32_t channels;
	/* pcm format */
	uint32_t pcm_fmt;
};

struct channel_map_info {
	/* channel num */
	uint32_t num;
	/* map info */
	uint32_t info[CHANNEL_NUM_MAX];
};

struct kws_module_info {
	uint8_t model_num;
	uint8_t model_id;
	uint32_t model_size;
	uint32_t *model_addr;
};

struct ec_ctrl {
uint8_t ec_type;/*for get ec. default:0,PA:3*/
uint8_t port_id;/*iis port id*/
uint8_t channel;
uint8_t bit_width;
uint32_t sample_rate;
};

union u_params {
	/* common parameter structure define */
	uint32_t value;
	struct stream_format sfmt;
	struct voice_pcie_bar pcie_bar;
	struct kws_module_info module_info;
	struct ec_ctrl kws_ec_ctrl;
};

struct stream_params {
	enum params_type type;
	union u_params params;
};

struct share_mem {
	/* virtual address of share memory */
	void *vir_addr;
	/* physical address of share memory */
	phys_addr_t p_addr;
	/* frame size one block buffer */
	uint32_t periods_size;
	/* block buffer number */
	uint32_t periods_num;
};

struct buf_blk {
	/* data buffer virtual address */
	void *vir_addr;
	/* data buffer physic address */
	phys_addr_t phy_addr;
	/* valid data size */
	uint32_t size;
	/* data transfer result
	 *  0: success
	 *  other: failure
	 */
	uint32_t result;
};

union u_rsp_info {
	/* message process result
	 * 0:      success
	 * other:  fail
	 */
	uint8_t result;
	struct buf_blk buf_info;
	struct stream_format sfmt;
};

struct rsp_payload {
	/* define operation code*/
	enum op_code opCode;
	union u_rsp_info payload;
};


/*********************************** struct define end ***********************************/

/* define callback function for asoc driver */
typedef void (*fk_acore_cb)(uint16_t session_id, void *priv, struct rsp_payload payload);

/*********************************** function define begin *******************************/
char *fk_acore_smem_alloc(struct device *dev, dma_addr_t *phy_addr, size_t size);
void fk_acore_smem_free(struct device *dev, size_t size, void *vaddr, dma_addr_t phy_addr);
int fk_acore_register(uint16_t session_id, fk_acore_cb cb, void *priv);
int fk_acore_unregister(uint16_t session_id);
int fk_acore_startup(uint16_t session_id, struct stream_format fmt, struct share_mem smem);
int fk_acore_set_paths(uint16_t session_id, int port_id, struct dai_format fmt, bool en);
int fk_acore_set_channel_map(uint16_t session_id, int port_id, struct channel_map_info *map_info);
int fk_acore_set_vol(uint16_t session_id, int val);
int fk_acore_set_mute(uint16_t session_id, uint8_t flag);
int fk_acore_set_params(uint16_t session_id, struct stream_params *strm_param);
int fk_acore_get_params(uint16_t session_id, struct stream_params *strm_param);
int fk_acore_trigger(uint16_t session_id, enum op_code op);
int fk_acore_playback_copy(uint16_t session_id, struct buf_blk p_buf);
int fk_acore_capture_copy(uint16_t session_id, struct buf_blk p_buf);


/*********************************** function define end *******************************/

#endif /* _FK_ACORE_API_ */
