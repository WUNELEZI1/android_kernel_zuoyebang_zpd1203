/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _FK_ACORE_
#define _FK_ACORE_

#include <linux/init.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/types.h>
#include <linux/slab.h>

#include "fk-acore-api.h"
#include "fk-acore-define.h"
#include "../adsp/adsp_boot_driver.h"

#include "../common/fk-audio-ipc.h"
#include "../common/fk-audio-log.h"

//#define SIMULATE_DEBUG 1
#define FK_AUDIO_USB_OFFLOAD 1

//#define EVENT_WAKEUP_TIMEOUT
#define EVENT_WAKEUP_TIMEOUT 0xc8

#ifndef EOK
#define EOK	(0)
#endif

/****************************** macro define begin ************************/
#define WAIT_TIMEOUT		(200)
#define WAIT_TIMEOUT_SHORT	(10)

#define SAMPLERATE_8K		(8000)
#define SAMPLERATE_12K		(12000)
#define SAMPLERATE_16K		(16000)
#define SAMPLERATE_24K		(24000)
#define SAMPLERATE_32K		(32000)
#define SAMPLERATE_48K		(48000)

#define PCM_FORMAT_8BIT		(8)
#define PCM_FORMAT_16BIT	(16)
#define PCM_FORMAT_24BIT	(24)
#define PCM_FORMAT_32BIT	(32)

#define CHANNEL_MONO		(1)
#define CHANNEL_STEREO		(2)

#define FRAMES_PER_BLOCK_DEFAULT	(1)

#define BE_DEV_NUM_MAX	(4)
/****************************** macro define end **************************/


/****************************** enum define begin *************************/
enum stream_ins {
	STRM_INS_1 = 0x0,
	STRM_INS_2,
	STRM_INS_3,
	STRM_INS_4,
	STRM_INS_MAX,
};

enum stream_dir {
	STRM_DIR_PLAYBACK = 0x0,
	STRM_DIR_CAPTURE,
	STRM_DIR_DUAL,
	STRM_DIR_MAX,
};

enum acore_stream_state {
	ACORE_STATE_IDLE = 0x0,
	ACORE_STATE_PREPARE,
	ACORE_STATE_PAUSE,
	ACORE_STATE_RUN,
	ACORE_STATE_MAX,
};

/****************************** enum define end *************************/


/****************************** struct define begin ************************/
#ifdef SIMULATE_DEBUG
struct cmd_cb_list_node {
	struct list_head cmd_list;
	struct xring_aud_msg msg;
	void *priv;
};
#endif


struct acore_stream {
	struct list_head strm_list;
	uint16_t sess_id;
	atomic_t init_flag;
	spinlock_t strm_lock;

	/* indicate scene type for adsp */
	enum adsp_scene_type adsp_sc_t;
	/* indicate instance id for adsp scene */
	uint8_t adsp_ins_id;

	/* ultra data lock*/
	struct mutex cap_lock;
	struct mutex play_lock;

	/* callback function and private parameters */
	fk_acore_cb cb;
	void *priv;

	/* fe info */
	int fe_flag;
	struct stream_format fmt;
	/* TODO: VOICE record and music incall */

	/* be info */
	int be_flag;
	int port_num;
	int port_info[BE_DEV_NUM_MAX];
	/* last operation port id */
	int port_id;
	struct dai_format dai_fmt;
	/* in&out port for bidirectional scene */
	int in_be_flag;
	int out_be_flag;
	int in_port_id;
	int out_port_id;
	struct dai_format in_dai_fmt;
	struct dai_format out_dai_fmt;
	/* channel map info
	 *    fill chan_map_info0 only one ul port used.
	 *    fill chan_map_info0&chan_map_info1 when multi ul port used.
	 *    when used multi port, the smaller seriel port info set
	 *    chan_map_info0, the bigger seriel port info set chan_map_info1.
	 *        bit[0~3]: slot0 channel map info
	 *        bit[4~7]: slot1 channel map info
	 *        bit[8~11]: slot2 channel map info
	 *        bit[12~15]: slot3 channel map info
	 *        bit[16~19]: slot4 channel map info
	 *        bit[20~23]: slot5 channel map info
	 *        bit[24~27]: slot6 channel map info
	 *        bit[28~31]: slot7 channel map info
	 */
	uint32_t chan_map_info0;
	uint32_t chan_map_info1;

	enum acore_stream_state state;
	enum stream_dir strm_dir;

	/* share buffer block info */
	atomic_t put_block_no;
	atomic_t rel_block_no;
	/* Remaining data */
	int cur_len;
	int total_len;

	/* data pipeline bypass dsp */
	bool bypass_dsp;
	/* loopback type */
	enum loopback_type lp_type;

	/* share memory info */
	struct share_mem smem;
	/* audif path info */
	struct adsp_io_buffer adsIOBuffer;
	struct audif_reg_devpath audifRegDevpath;

	/* operation flag */
	atomic_t write_flag;
	atomic_t start_flag;
	atomic_t stop_flag;
	atomic_t flush_flag;
	atomic_t resume_flag;
	atomic_t last_frame;

	wait_queue_head_t cmd_wait_que;
	atomic_t startup_wait;
	atomic_t start_wait;
	atomic_t stop_wait;
	atomic_t shutdown_wait;
	atomic_t timestamp_wait;
	atomic_t event_wait;
	atomic_t start_rec_wait;
	atomic_t stop_rec_wait;
	atomic_t vt_fca_flag_wait;

	/* ultra scene: data process resource */
	/* data list head */
	struct list_head strm_data_list;
	struct buf_blk cur_buf;

	/* compress playback scene */
	uint32_t cur_timestamp;
	uint32_t use_gapless;
	uint32_t initial_samples;
	uint32_t trailing_samples;

	/* voice call pcie bar */
	struct voice_pcie_bar_info bar_info;
	bool speech_status;

	/*kws scene*/
	uint32_t det_event;

	/*mmap offset*/
	uint32_t mmap_offset_paddr;
	void *mmap_offset_vaddr;

	int audif_clk_status;
	bool work_on;
};

struct adsp_vote_info {
	struct list_head vote_info_list;
	uint16_t sess_id;
	uint16_t pu_vote_flag;
	uint32_t pu_vote_cnt;
	uint64_t pu_vote_tstamp;
	uint32_t pd_vote_cnt;
	uint64_t pd_vote_tstamp;
};

struct acore_client {
	int init_flag;
	/* define audio client register numbers */
	atomic_t ac_num;
	/* define adsp vote register numbers */
	atomic_t vote_num;
	struct list_head vote_list;
	struct mutex vote_lock;

	/* audio stream list */
	struct list_head ac_strm_list;
	struct mutex list_lock;

	/* audio device route table */
	void *route_table;
	/* audio device route number */
	uint32_t items_num;

	struct dentry *dbg_fs_root;
	bool backdoor_boot;

	/* frame number one block on compress capture scene */
	int frames_per_blk;

#ifdef ABANDON_FRAME_VER
	int abandon_frame_count;
#endif

	bool gapless_mode;

	int usb_conn;

#ifdef SIMULATE_DEBUG
		/* simulate define */
		struct task_struct *cb_thrd_fn;
		struct list_head cmd_cb_list;
#endif

	struct adsp_ssr_action *ssr_act;
	atomic_t ssr_flag;
};

/****************************** struct define end **************************/

/****************************** function define begin **********************/
void fk_acore_msg_handle(void *msg, int len, void *priv);

int fk_acore_get_scene_id(uint16_t session_id);
int fk_acore_get_ins_id(uint16_t session_id);
int fk_acore_get_stream_dir(uint16_t session_id);
struct acore_stream *fk_acore_stream_get(uint16_t session_id);
struct acore_stream *fk_acore_stream_get_by_scene(uint8_t scene_id, uint8_t ins_id);
struct acore_stream *fk_acore_stream_alloc(uint16_t session_id);
int fk_acore_stream_prepare(struct acore_stream *ac_strm);
void fk_acore_stream_idle(struct acore_stream *ac_strm);
int fk_acore_stream_free(struct acore_stream *ac_strm);

int fk_acore_stream_restartup(struct acore_stream *ac_strm);
int fk_acore_stream_startup(struct acore_stream *ac_strm);
int fk_acore_stream_shutdown(struct acore_stream *ac_strm);
int fk_acore_stream_trigger_start(struct acore_stream *ac_strm);
int fk_acore_stream_trigger_stop(struct acore_stream *ac_strm);
int fk_acore_stream_trigger_pause(struct acore_stream *ac_strm);
int fk_acore_stream_trigger_resume(struct acore_stream *ac_strm);
int fk_acore_stream_trigger_flush(struct acore_stream *ac_strm);
int fk_acore_stream_trigger_drain(struct acore_stream *ac_strm);
int fk_acore_stream_trigger_next_track(struct acore_stream *ac_strm);
int fk_acore_stream_data_send(struct acore_stream *ac_strm, struct buf_blk p_buf);
int fk_acore_stream_data_recv(struct acore_stream *ac_strm, struct buf_blk p_buf);

#ifdef STARTUP_INFO_VER_02
int fk_acore_port_set(struct acore_stream *ac_strm, void *info, bool en);
int fk_acore_peri_sel_set(struct acore_stream *ac_strm, void *info);
int fk_acore_add_path_config(struct acore_stream *ac_strm);
int fk_acore_path_update(struct acore_stream *ac_strm, bool en);
#else
int fk_acore_peri_sel_set(struct acore_stream *ac_strm, void *startup_info);
#endif

int fk_acore_audioIf_iobuf_reg_set(struct acore_stream *ac_strm, struct adsp_io_buffer *io_buf,
	struct audif_reg_devpath *audif_reg);
int fk_acore_audioIf_reg_set(struct acore_stream *ac_strm, enum adsp_io_buffer_type io_buf_t,
	struct audif_reg_devpath *audif_reg);

int fk_acore_get_timestamp_info(struct acore_stream *ac_strm,
	uint32_t *timestamp);
int fk_acore_set_frames_info(struct acore_stream *ac_strm);
int fk_acore_get_frames_info(void);
int fk_acore_kws_load_module(struct acore_stream *ac_strm, struct kws_module_info *module_info);
int fk_acore_kws_unload_module(struct acore_stream *ac_strm, struct kws_module_info *module_info);
int fk_acore_kws_event_status(struct acore_stream *ac_strm, uint32_t *event_status);
int fk_acore_kws_stop_rec(struct acore_stream *ac_strm, struct kws_module_info *module_info);
int fk_acore_kws_start_rec(struct acore_stream *ac_strm, struct kws_module_info *module_info);
int fk_acore_kws_ec_ref(struct acore_stream *ac_strm, struct ec_ctrl *kws_ec_ref);
int fk_acore_speech_bandwidth(struct acore_stream *ac_strm, enum speech_band band);
int fk_acore_set_volume(struct acore_stream *ac_strm, int volume);
int fk_acore_set_mute_flag(struct acore_stream *ac_strm, uint8_t mute);
int fk_acore_set_vt_fca_flag(struct acore_stream *ac_strm, uint8_t flag);
int fk_acore_set_fca_event(struct acore_stream *ac_strm);
int fk_acore_message_send(struct xring_aud_msg *msg);
int fk_aocre_common_message_send(struct xring_aud_msg *msg);
int fk_acore_set_dsp_temp(int temp);
uint32_t fk_acore_port_id_get(int port_id);

void *fk_acore_get_acore_client(void);

int fk_acore_init(void);
void fk_acore_exit(void);

/****************************** function define end ************************/


#endif //_FK_ACORE_

