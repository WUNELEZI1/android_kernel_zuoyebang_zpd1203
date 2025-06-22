/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _FK_ACORE_DEFINE_
#define _FK_ACORE_DEFINE_

#include <linux/init.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/slab.h>

#include "../common/fk-audio-mailbox.h"

#define STARTUP_INFO_VER_02

/****************************** macro define begin ************************/

/* max message length is 60 bytes. And ipc protocal header is 8 bytes,
 * message header is 8 bytes. Message payload must less than 44 bytes.
 * Just for non overflow here.
 */
#define CONFIG_RPMSG_MSG_DATA_LEN	(MBX_BLK_SIZE)
#define RSP_MSG_OK					(0)

#define I2S_UNSEL	(0)
#define I2S0_BIT	(1)
#define I2S1_BIT	(2)
#define I2S2_BIT	(3)
#define I2S3_BIT	(4)
#define I2S4_BIT	(5)

#define VAD_I2S_IDX (20)

#define PCM_FMTBIT_16	(16)
#define PCM_FMTBIT_24	(24)
#define PCM_FMTBIT_32	(32)

#define PCM_FMTCHS_2	(2)
#define PCM_FMTCHS_4	(4)
#define PCM_FMTCHS_6	(6)
#define PCM_FMTCHS_8	(8)
#define PCM_FMTCHS_16	(16)

#define MAX_ELEMS_NUM	5

/* channel map */
#define CHAN_MAP_INFO_0_NUM		(8)
#define CHAN_MAP_INFO_0_1_NUM	(16)
#define SLOT_BIT_WIDTH			(4)
#define SLOT_VAL(val, n)		((val & 0xf) << SLOT_BIT_WIDTH * n)

/****************************** macro define end **************************/

/****************************** enum define begin *************************/
enum adsp_scene_ctrl_type {
	ADSP_AUD_SCENE_STARTUP = 0x0,
	ADSP_AUD_SCENE_SHUTDOWN,
	ADSP_AUD_SCENE_TRIGGER,
	ADSP_AUD_CTRL,
	ADSP_VOICE_CTRL,
	ADSP_PARAM_CTRL,
	ADSP_HW_MODULE_CTRL,
	ADSP_DEBUG_CTRL,
	ADSP_SYS_CTRL,
	ADSP_GET_VALUE,
	ADSP_GET_RTC_TIME,
	ADSP_SEND_RTC_TIME,
	ADSP_KWS_CTRL,
	MAX_CTRL_NUM,
};

enum adsp_scene_type {
	ADSP_SCENE_NONE = 0x0,
	ADSP_SCENE_NORMAL_RECORD,
	ADSP_SCENE_OFFLOAD_RECORD,
	ADSP_SCENE_FAST_RECORD,
	ADSP_SCENE_ULTRA_FAST_RECORD,
	ADSP_SCENE_KWS_RECORD,
	ADSP_SCENE_KMUSIC_RECORD,
	ADSP_SCENE_ULTRA_FAST_PLAY,
	ADSP_SCENE_FAST_PLAY,
	ADSP_SCENE_NORMAL_PLAY,
	ADSP_SCENE_OFFLOAD_PLAY,
	ADSP_SCENE_MUSIC_INCALL_PLAY,
	ADSP_SCENE_VOICE_CALL_DL,
	ADSP_SCENE_VOICE_CALL_UL,
	ADSP_SCENE_VOIP_DL,
	ADSP_SCENE_VOIP_UL,
	ADSP_SCENE_HFP,
	ADSP_SCENE_LOOPBACK,
	ADSP_SCENE_HAPTIC,
	ADSP_SCENE_SPATIAL_PLAY,
	ADSP_SCENE_MMAP_PLAY,
	ADSP_SCENE_MMAP_RECORD,
	ADSP_SCENE_KARAOKE_DL,
	ADSP_SCENE_KARAOKE_UL,
	ADSP_SCENE_VOICE_CALL_RECORD,
	ADSP_SCENE_DIRECT_PLAY,
	ADSP_SCENE_HF_DL,
	ADSP_SCENE_HF_UL,
	MAX_SCENE_NUM,
};

enum adsp_ctrl_type {
	/* audio ctrl */
	ADSP_AUD_CTRL_VOL = 0x0,
	ADSP_AUD_CTRL_MIXER,
	ADSP_AUD_CTRL_EFFECT,
	ADSP_AUD_CTRL_START,
	ADSP_AUD_CTRL_WRITE,
	ADSP_AUD_CTRL_READ,
	ADSP_AUD_CTRL_PAUSE,
	ADSP_AUD_CTRL_FLUSH,
	ADSP_AUD_CTRL_RESUME,
	ADSP_AUD_CTRL_STOP,
	ADSP_AUD_CTRL_GET_TIMESTAMP,
	ADSP_AUD_CTRL_LAST_FRAME,
	ADSP_AUD_CTRL_SET_GAPLESS,
	ADSP_AUD_CTRL_SET_FRAME_PER_BLK,
	ADSP_AUD_CTRL_SET_NEXT_TRACK,
	ADSP_AUD_CTRL_SEND_TIMESTAMP,
	/* voice ctrl */
	ADSP_VOICE_CTRL_START = 0x20,
	ADSP_VOICE_CTRL_STOP,
	ADSP_VOICE_CTRL_PCIE_BAR,
	ADSP_VOICE_CTRL_SPEECH_ON,
	ADSP_VOICE_CTRL_SPEECH_OFF,
	ADSP_VOICE_CTRL_VOL,
	ADSP_VOICE_CTRL_RECORD,
	ADSP_VOICE_CTRL_MIXER,
	ADSP_VOICE_CTRL_REPLACE,
	ADSP_VOICE_CTRL_UL_MUTE,
	ADSP_VOICE_CTRL_DL_MUTE,
	ADSP_VOICE_CTRL_TONE_GENERATE,
	ADSP_VOICE_CTRL_DTMF_GENERATE,
	ADSP_VOICE_CTRL_SET_SAMPLERATE,
	ADSP_VOICE_CTRL_SET_BANDWIDTH,
	/* ADSP_VOICE_CTRL_WRITE, */
	/* ADSP_VOICE_CTRL_READ, */
	/* KWS CTRL */
	ADSP_KWS_CTRL_LOAD_MODEL = 0x40,
	ADSP_KWS_CTRL_UNLOAD_MODEL,
	ADSP_KWS_CTRL_START_RECOGNITION,
	ADSP_KWS_CTRL_STOP_RECOGNITION,
	ADSP_KWS_CTRL_EVT_REPORT,
	ADSP_KWS_CTRL_READ,
	ADSP_KWS_CTRL_VT_FCA_FLAG,
	ADSP_KWS_CTRL_FCA_EVENT,
	/* parameter ctrl */
	ADSP_PARAM_CTRL_VOICE_ALGO = 0x60,
	ADSP_PARAM_CTRL_AUDIO_ALGO,
	ADSP_PARAM_CTRL_AUDIF_REGS,
	ADSP_PARAM_CTRL_FREE,
	ADSP_PARAM_CTRL_EC,
	ADSP_PARAM_CTRL_DYN_PARAMTER,
	/* HW module ctrl */
	ADSP_SET_AUDIOIF_MODULE = 0x80,
	ADSP_GET_AUDIOIF_MODULE,
	ADSP_SET_DEV_PATH,
	ADSP_SET_DEV_PATH_COMPLETE,
	ADSP_SET_USB_CONNECT,
	ADSP_SET_USB_DISCONNECT,
	/* debug ctrl */
	ADSP_DEBUG_CTRL_DSP_LOG = 0xA0,
	ADSP_DEBUG_CTRL_PCM_DUMP_OPEN,
	ADSP_DEBUG_CTRL_PCM_DUMP_CLOSE,
	ADSP_DEBUG_CTRL_ASK_DSP_WDT,
	ADSP_DEBUG_CTRL_ASK_DSP_ASSERT,
	ADSP_DEBUG_CTRL_DSP_PANIC,
	ADSP_DEBUG_CTRL_DSP_LOG_TELL_AP,
	ADSP_DEBUG_CTRL_PCM_DUMP_TELL_AP,
	ADSP_DEBUG_CTRL_REG_VALUE,
	ADSP_DEBUG_CTRL_REG_VALUE_TELL_AP,
	ADSP_DEBUG_CTRL_UNCACHE_LOG_ENABLE,
	ADSP_DEBUG_CTRL_UNCACHE_MEM_TELL_AP,
	/* system ctrl */
	ADSP_SYS_CTRL_SMARTAMP = 0xC0,
	ADSP_SYS_CTRL_BT_CODEC,
	ADSP_SYS_CTRL_GET_VER,
	ADSP_SYS_CTRL_RCV_VER,
	ADSP_SYS_CTRL_SET_DVFS,
	ADSP_SYS_CTRL_AP_WAKEUP,
	ADSP_SYS_CTRL_TEMP,
};

enum adsp_io_buffer_type {
	ADSP_IO_BUF_NONE = 0x0,
	//DL
	ADSP_IO_BUF_AP,
	ADSP_IO_BUF_DA0,
	ADSP_IO_BUF_DA1,
	ADSP_IO_BUF_DA2,
	//UL
	ADSP_IO_BUF_AR,
	ADSP_IO_BUF_AD0,
	ADSP_IO_BUF_AD1,
	ADSP_IO_BUF_AD2,
	ADSP_IO_BUF_AD3,
	ADSP_IO_BUF_AD4,
};

enum codec_type {
	CODEC_NONE = 0x0,
	CODEC_SBC,
	CODEC_LC3,
	CODEC_MSBC,
	CODEC_CVSD,
	CODEC_MP3,
	CODEC_AAC,
	CODEC_AMR,
};

enum loopback_type {
	LP_TYPE_PCM_ALGO = 0x0,
	LP_TYPE_PCM,
	LP_TYPE_COMPRESS,
	LP_TYPE_BT,
	LP_TYPE_MODEM,
	LP_TYPE_VOICE_CALL,
	LP_TYPE_MAX,
};

enum voice_type {
	VOICE_TYPE_NORMAL = 0x0,
	VOICE_TYPE_LOOPBACK,
	VOICE_TYPE_MAX,
};

enum port_dai_codec_type {
	PORT_DAI_CODEC_TYPE_NORMAL = 0x0,
	PORT_DAI_CODEC_TYPE_DUALADC,
	PORT_DAI_CODEC_TYPE_OZO,
	PORT_DAI_CODEC_TYPE_HEADSET,
	PORT_DAI_CODEC_TYPE_VA,
	PORT_DAI_CODEC_TYPE_MAX,
};

/*enum for ec reference */
enum echo_type_t {
	FROM_EC_NONE = 0x0,
	FROM_IIS_RX,
	FROM_AUDIF_ADC,
	FROM_DSP,
	FROM_DSP_IIS_TX,
	FROM_DSP_AUDIF_DAC,
};

enum mixer_ctrl_type {
	MIXER_CTRL_NONE = 0x0,
	CAPTURE_SWITCH,
	CAPTURE_VOLUME,
	PLAYBACK_SWITCH,
	PLAYBACK_VOLUME,
	MIXER_CTRL_MAX,
};

enum speech_band {
	SPEECH_NB,
	SPEECH_WB,
	SPEECH_SWB,
	NUM_SPEECH_BAND,
};

/****************************** enum define end *************************/

/****************************** struct define begin *************************/

struct usb_ctrl_param {
	uint8_t ctrl_type;
	uint32_t pipe;
	uint8_t request;
	uint8_t request_type;
	uint16_t value;
	uint16_t index;
	uint32_t buf;
	uint16_t size;
} __packed;

struct usb_mixer_info {
	uint8_t ctrl_num;
	struct usb_ctrl_param ctrl_msg[MAX_ELEMS_NUM];
};

struct sbc_enc_info {
	uint16_t hci_handle;
	uint16_t l2cap_cid;
	uint16_t a2dp_mtu;
	uint32_t a2dp_bitrate;
	uint8_t channel_mode;
	uint8_t blocks;
	uint8_t subbands;
	uint8_t alloc_method;
	uint16_t min_bitpool;
	uint16_t max_bitpool;
} __packed;

struct aac_enc_info {
	uint16_t hci_handle;
	uint16_t l2cap_cid;
	uint16_t a2dp_mtu;
	uint32_t a2dp_bitrate;
	uint32_t profile;
	uint32_t vbr;
} __packed;

union bt_codec_info {
	struct sbc_enc_info sbc_config;
	struct aac_enc_info aac_config;
};

struct bt_codec_port_info {
	uint8_t codec_type;
	uint8_t channels;
	uint8_t bit_width;
	uint32_t sample_rate;
} __packed;

struct bt_codec_config {
	struct bt_codec_port_info port_info;
	union bt_codec_info data;
};

struct ap_dsp_usb_info {
	uint8_t intf_num;
	uint8_t alt_num;
	uint8_t idle_intf_num;
	uint8_t idle_alt_num;
	uint8_t datainterval;
	uint32_t pipe;
	uint16_t clock_id;
	uint8_t protocol;
	uint8_t ctrl_intf;
	uint32_t sync_pipe;
	uint8_t sync_interval;
	uint8_t format;
	uint8_t bits;
	uint8_t attributes;
	/*
	 * 0: capture & playback
	 * 1: capture
	 * 2: playback
	 */
	uint8_t dev_type;
} __packed;

#ifdef STARTUP_INFO_VER_02

struct stream_info {
	uint8_t codec_type;
	uint32_t sample_rate;
	uint8_t channels;
	uint8_t bit_width;
} __packed;

struct port_dai_info {
	uint8_t codec_type;
	uint32_t sample_rate;
	uint8_t channels;
	uint8_t bit_width;
} __packed;

#else

struct codec_info {
	uint8_t codec_type;
	uint32_t sample_rate;
	uint8_t channels;
	uint8_t bit_width;
} __packed;

#endif

/* for pcm stream:
 *    play stream:
 *        r_off: current read block index(frame no), update by dsp.
 *        w_off: current write block index(frame no), update by ap.
 *    capture stream:
 *        r_off: current read block index(frame no), update by ap.
 *        w_off: current write block index(frame no), update by dsp.
 */
struct adsp_pcm_smem_head {
	uint32_t r_off;
	uint32_t w_off;
	uint8_t flag;
	uint32_t len;
} __packed;

struct adsp_share_mem {
	/* ring buffer head info physical address of share memory */
	uint32_t p_addr_head;
	/* physical address of share memory */
	uint32_t p_addr;
	/* size one block buffer, unit byte */
	uint32_t blk_buf_size;
	/* block buffer number */
	uint16_t blk_buf_num;
} __packed;

struct audif_reg_devpath {
	/* bit[0]: un-sel
	 * bit[1]: iis 0
	 * bit[2]: iis 1
	 * bit[3]: iis 2
	 * bit[4]: iis 3
	 * bit[5]: iis 4
	 */
	uint8_t da2_audif_iis_sel;
	uint8_t da1_audif_iis_sel;
	uint8_t da0_audif_iis_sel;
	uint8_t ad4_audif_iis_sel;
	uint8_t ad3_audif_iis_sel;
	uint8_t ad2_audif_iis_sel;
	uint8_t ad1_audif_iis_sel;
	uint8_t ad0_audif_iis_sel;
} __packed;

struct adsp_io_buffer {
	uint8_t dlPathID0;
	uint8_t dlPathID1;
	uint8_t dlPathID2;
	uint8_t ulPathID0;
	uint8_t ulPathID1;
	uint8_t ulPathID2;
	uint8_t ulPathID3;
	uint8_t reservePath;
} __packed;

struct loopback_info {
	uint8_t loopback_mode;
	uint8_t loopback_enc_mode;
} __packed;

#ifdef STARTUP_INFO_VER_02

struct peri_sel_ul_path {
	/*
	 * Audif value:
	 *		0 : i2s0
	 *		1 : i2s1
	 *		2 : i2s2
	 *		3 : i2s3
	 *		4 : i2s4
	 */
	uint8_t l_i2s_idx;
	/*
	 * Audif i2s0:
	 *		bit[0] : left
	 *		bit[1] : right
	 * TDM:
	 *		bit[0~7] : ch0~7
	 *		other:invalid
	 */
	uint8_t l_i2s_chn;
	uint8_t r_i2s_idx;
	uint8_t r_i2s_chn;
	/*
	 * DSP i2s5 : only support i2s mode, connect to modem
	 */
	uint8_t i2s5_slot;
	/*
	 * DSP i2s6 : only support i2s mode, connect to wifi
	 */
	uint8_t i2s6_slot;
	/*
	 * DSP i2s7/8 : support TDM 16 slot mode, connect to SmartPA
	 */
	uint16_t i2s7_slot;
	uint16_t i2s8_slot;
	/*
	 * DSP i2s9 : only support i2s mode, connect to codec
	 */
	uint8_t i2s9_slot;
	/*
	 * vad i2s:
	 *     connect to modem/wifi/smartPA/codec
	 */
	uint8_t i2s10_slot;
	/*
	 * channel map info
	 * fill ch_map_info0 only one ul port used.
	 * fill ch_map_info0&ch_map_info1 when multi ul port used.
	 * when used multi port, the smaller seriel port info set
	 * ch_map_info0, the bigger seriel port info set ch_map_info1.
	 *
	 * bit[0~3]: slot0 channel map info
	 * bit[4~7]: slot1 channel map info
	 * bit[8~11]: slot2 channel map info
	 * bit[12~15]: slot3 channel map info
	 * bit[16~19]: slot4 channel map info
	 * bit[20~23]: slot5 channel map info
	 * bit[24~27]: slot6 channel map info
	 * bit[28~31]: slot7 channel map info
	 */
	uint32_t ch_map_info0;
	uint32_t ch_map_info1;
} __packed;

struct peri_sel_dl_path {
	/*
	 * Audif I2S0:
	 *    bit[0] : left
	 *    bit[1] : right
	 * TDM:
	 *    bit[0~7] : ch0~7
	 *    other:invalid
	 */
	uint8_t i2s0_slot;
	uint8_t i2s1_slot;
	uint8_t i2s2_slot;
	uint8_t i2s3_slot;
	uint8_t i2s4_slot;
	uint8_t i2s5_slot;
	uint8_t i2s6_slot;
	uint16_t i2s7_slot;
	uint16_t i2s8_slot;
	uint8_t i2s9_slot;
	uint8_t mi2s0_slot;
	/*
	 * channel map info
	 *    fill ch_map_info0 only one dl port used.
	 *    fill ch_map_info0&ch_map_info1 when multi dl port used.
	 *    when used multi port, the smaller seriel port info set
	 *    ch_map_info0, the bigger seriel port info set ch_map_info1.
	 */
	uint32_t ch_map_info0;
	uint32_t ch_map_info1;
} __packed;

enum audio_dev_port_id_t {
	PORT_ID_AUDIFIIS0,
	PORT_ID_AUDIFIIS1,
	PORT_ID_AUDIFIIS2,
	PORT_ID_AUDIFIIS3,
	PORT_ID_AUDIFIIS4,
	PORT_ID_IIS5,
	PORT_ID_IIS6,
	PORT_ID_IIS7,
	PORT_ID_IIS8,
	PORT_ID_IIS9,
	PORT_ID_MIIS0,
	PORT_ID_USB,
	PORT_ID_VAD,
	PORT_ID_MAX,
};

struct adsp_dev_port_info {
	/* shift audio_dev_id_t
	 * eg: port_id = 101b, config AUDIFIIS0/2
	 */
	uint32_t port_id;
	/* 1: enable port
	 * 0: disable port
	 */
	uint8_t enable_flag;
	struct port_dai_info dai_info;
	struct peri_sel_ul_path ul_peri_sel;
	struct peri_sel_dl_path dl_peri_sel;
	struct ap_dsp_usb_info usb_info;
	struct bt_codec_config bt_config;
} __packed;

struct adsp_scene_startup_info {
	struct stream_info strm_info;
	struct adsp_share_mem smem;
	struct adsp_dev_port_info port_info;
} __packed;

struct adsp_scene_lp_startup_info {
	uint16_t lp_type;
	struct loopback_info lp_info;
	uint32_t ul_port_id;
	uint32_t dl_port_id;
	uint32_t ul_port_ch;
	struct port_dai_info ul_dai_info;
	struct port_dai_info dl_dai_info;
	struct peri_sel_ul_path ul_peri_sel;
	struct peri_sel_dl_path dl_peri_sel;
} __packed;

#else

struct ap_usb_msg {
	/* audio_usb_cmd */
	uint16_t cmd_id;
	/* length of data */
	uint16_t data_len;
	/* data physical address */
	uint32_t data_addr;
} __packed;

struct peri_sel_ul_path {
	uint8_t l_i2s_idx;
	uint8_t l_i2s_chn;
	uint8_t r_i2s_idx;
	uint8_t r_i2s_chn;
	uint8_t i2s5_slot;
	uint8_t i2s6_slot;
	uint16_t i2s7_slot;
	uint16_t i2s8_slot;
	uint8_t i2s9_slot;
	uint8_t i2s10_slot;
	struct ap_usb_msg usb_msg;
} __packed;

struct peri_sel_dl_path {
	uint8_t i2s0_slot;
	uint8_t i2s1_slot;
	uint8_t i2s2_slot;
	uint8_t i2s3_slot;
	uint8_t i2s4_slot;
	uint8_t i2s5_slot;
	uint8_t i2s6_slot;
	uint16_t i2s7_slot;
	uint16_t i2s8_slot;
	uint8_t i2s9_slot;
	uint8_t mi2s0_slot;
	struct ap_usb_msg usb_msg;
} __packed;

union adsp_peri_sel {
	struct peri_sel_ul_path ulPeriSel;
	struct peri_sel_dl_path dlPeriSel;
};

struct adsp_scene_startup_info {
	struct codec_info codecInfo;
	struct adsp_share_mem smem;
	struct adsp_io_buffer adsIOBuffer;
	union adsp_peri_sel peri_sel;
} __packed;

struct adsp_scene_lp_startup_info {
	struct codec_info codecInfo;
	/* lpTpye: loopback type.
	 *     0: pcm mode
	 *     1: pcm mode without algorithm.
	 *     2: compress mode.
	 *     3: bt mode.
	 */
	uint16_t lpType;
	struct loopback_info lpInfo;
	struct peri_sel_ul_path ulPeriSel;
	struct peri_sel_dl_path dlPeriSel;
} __packed;

#endif

struct adsp_data_buf_info {
	uint32_t phy_addr;
	uint32_t len;
	uint32_t frame_no;
} __packed;

struct rtc_time_s {
	uint8_t rtc_year;
	uint8_t rtc_month;
	uint8_t rtc_day;
	uint8_t rtc_hour;
	uint8_t rtc_min;
	uint8_t rtc_sec;
} __packed;

struct adsp_last_buf_info {
	uint32_t write_offset;
	uint32_t data_len;
} __packed;

struct gapless_info {
	/* gapless mode state */
	uint32_t gapless_mode;
	/* encoder padding */
	uint32_t initial_samples;
	/* encoder delay */
	uint32_t trailing_samples;
};

struct voice_pcie_bar_info {
	uint64_t BAR0_1_addr;
	uint64_t BAR2_3_addr;
	uint8_t  voice_type;
} __packed;

struct xring_aud_msg_header {
	uint8_t scene_id;
	uint8_t ins_id;
	uint8_t cmd_id;
	uint8_t func_id;
	uint16_t len;
	/* indicate message result
	 *		0: success
	 *		other: fail
	 */
	uint8_t result;
	/* FK-AUDIO_MAILBOX begin */
	/* adsp_source_type */
	uint8_t msg_source;
	/* FK-AUDIO_MAILBOX end */
} __packed;

struct xring_aud_msg {
	struct xring_aud_msg_header header;
	uint8_t data[CONFIG_RPMSG_MSG_DATA_LEN];
} __packed;

struct dsp_assert_info {
	uint8_t tcb_name[24];
	uint32_t int_status;
	uint32_t scene_flag;
	uint32_t stackoverflow;
	uint32_t version;
	uint32_t exccause;
	uint32_t excvaddr;
	uint32_t excpc;
	uint32_t reserve0;
	uint32_t reserve1;
	uint32_t reserve2;
} __packed;

/****************************** struct define end ***************************/

#endif /* _FK_ACORE_DEFINE_ */
