// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/list.h>
#include <drm/drm_modes.h>
#include <drm/drm_mode.h>
#include "dp_display.h"
#include "dp_drm.h"
#include "dp_panel.h"

#define DP_DSC_STATUS                         0x20F
#define DP_DSC_CRC_0                          0x262
#define DP_DSC_CRC_STATUS_SIZE                6
#define DP_FEC_STATUS_SIZE                    3

#define MIN_DYNAMIC_METADATA_INFOFRAME_SIZE   4
#define DP_SDP_PAYLOAD_SIZE                   32

#define SINK_IDENTITY_OFFSET                  0x8

#define USE_EXTENDED_TAG                      0x7
#define AUDIO_DATA_BLOCK                      0x1
#define SPEAKER_ALLOCATION_DATA_BLOCK         0x4
#define COLORIMETRY_BLOCK                     0x5
#define HDR_STATIC_METADATA_BLOCK             0x6
#define HDR_DYNAMIC_METADATA_BLOCK            0x7
#define CEA_DATA_BLK_LEN_MASK                 BITS_MASK(0, 5)

#define DYNAMIC_METADATA_TYPE_DOLBY           0x0001
#define DYNAMIC_METADATA_TYPE_HDR10_PLUS      0x0004

#define DP_STATIC_METADATA_REPEAT_TIME        1000
#define DP_DYNAMIC_METADATA_REPEAT_TIME       100

#define cea_data_blk_extended_tag_code(cea_db) (cea_db[1])
#define cea_data_blk_len(cea_db) (((cea_db)[0] & CEA_DATA_BLK_LEN_MASK) + 1)
#define cea_data_blk_tag_code(cea_db) (GET_BITS_VAL((cea_db)[0], 5, 3))

#define for_each_cea_db(cea_blk, i, start, end) \
	for (i = (start); i < (end) && i + cea_data_blk_len(&cea_blk[i]) <= (end); \
			i += cea_data_blk_len(&cea_blk[i]))

static struct dp_panel_video_info default_video_info = {
	.bpc = DP_BPC_8,
	.pixel_format = PIXEL_FORMAT_RGB,
	.colorimetry = DP_COLORIMETRY_BT601,
	.dynamic_range = DYNAMIC_RANGE_CEA,
};

struct tu_calculate_param {
	bool fec_en;
	u32 color_depth;
	u32 pixel_clock_khz;
	enum dp_lane_count lane_count;
	u32 link_rate_mbps;
};

struct init_threshold_calculate_param {
	bool dsc_en;
	bool fec_en;
	u32 link_clk_khz;
	u32 pixel_clock_khz;
	u32 tu_valid_bytes;
	u32 tu_valid_bytes_frac;
	u32 color_depth;
	enum dp_lane_count lane_count;
	enum dp_bpc bpc;
	enum pixel_format pixel_format;
};

static void dp_panel_dump_timing(struct dp_hw_ctrl_video_config *video_config)
{
	struct dp_display_timing *timing = &video_config->timing;

	DP_INFO("  inter|    hsw|    hbp|   hact|    hfp|h_blank|h_total"
			"|    vsw|    vbp|   vact|    vfp|v_blank|v_total\n");
	DP_INFO("%7d|%7d|%7d|%7d|%7d|%7d|%7d|%7d|%7d|%7d|%7d|%7d|%7d\n",
			timing->interlaced,
			timing->h_sync_width,
			timing->h_back_porch,
			timing->h_active,
			timing->h_front_porch,
			timing->h_blank,
			timing->h_total,
			timing->v_sync_width,
			timing->v_back_porch,
			timing->v_active,
			timing->v_front_porch,
			timing->v_blank,
			timing->v_total);

	DP_INFO("  TU |  TU_FRAC |    pixel_clock |         resolution  |    color_depth\n");
	DP_INFO("  %2u |  %7u |    %8dkhz | %4ux%4u@%03d.%03dhz |    %11d\n",
			video_config->tu.valid_bytes,
			video_config->tu.valid_bytes_frac,
			timing->pixel_clock_khz,
			timing->h_active,
			timing->v_active,
			timing->fps_thousand / 1000,
			timing->fps_thousand % 1000,
			timing->color_depth);
}

void dp_panel_set_timing(struct dp_panel *panel, struct drm_display_mode *mode)
{
	struct dp_panel_video_info *video_info;
	struct dp_panel_dsc_info *dsc_info;
	struct dp_display_timing *timing;

	if (!panel || !mode) {
		DP_ERROR("invalid parameter, panel:%p, mode:%p\n", panel, mode);
		return;
	}

	video_info = &panel->video_info;
	dsc_info = &panel->dsc_info;
	timing = &panel->video_info.timing;

	timing->pixel_clock_khz = mode->clock;

	timing->h_active = mode->hdisplay;
	timing->h_front_porch = mode->hsync_start - mode->hdisplay;
	timing->h_sync_width = mode->hsync_end - mode->hsync_start;
	timing->h_back_porch = mode->htotal - mode->hsync_end;
	timing->h_blank = mode->htotal - mode->hdisplay;
	timing->h_total = mode->htotal;

	timing->v_active = mode->vdisplay;
	timing->v_front_porch = mode->vsync_start - mode->vdisplay;
	timing->v_sync_width = mode->vsync_end - mode->vsync_start;
	timing->v_back_porch = mode->vtotal - mode->vsync_end;
	timing->v_blank = mode->vtotal - mode->vdisplay;
	timing->v_total = mode->vtotal;

	timing->interlaced = mode->flags & DRM_MODE_FLAG_INTERLACE ?
			DP_TIMING_INTERLACED : DP_TIMING_PROGRESSIVE;

	if (mode->flags & DRM_MODE_FLAG_PHSYNC)
		timing->h_sync_polarity = DP_TIMING_POLARITY_ACTIVE_HIGH;
	else if (mode->flags & DRM_MODE_FLAG_NHSYNC)
		timing->h_sync_polarity = DP_TIMING_POLARITY_ACTIVE_LOW;
	else
		DP_ERROR("unclear Vsync polarity\n");

	if (mode->flags & DRM_MODE_FLAG_PVSYNC)
		timing->v_sync_polarity = DP_TIMING_POLARITY_ACTIVE_HIGH;
	else if (mode->flags & DRM_MODE_FLAG_NVSYNC)
		timing->v_sync_polarity = DP_TIMING_POLARITY_ACTIVE_LOW;
	else
		DP_ERROR("unclear Vsync polarity\n");

	timing->v_active = (timing->interlaced == 1) ?
			(timing->v_active / 2) : timing->v_active;

	timing->fps_thousand = (u64)timing->pixel_clock_khz * 1000 * 1000 /
			timing->h_total / timing->v_total;

	timing->color_depth = dsc_info->dsc_en ?
			(dsc_info->dsc_config->bits_per_pixel >> 4) :
			dp_panel_get_color_depth(video_info->bpc, video_info->pixel_format);
}

static void dp_panel_dump_dsc_caps(struct dp_panel *panel)
{
	struct dsc_caps_sink *dsc_caps = &panel->sink_caps.dsc_caps;
	u8 i;

	DP_DEBUG("============== DSC SINK CAPABILITIES ==============\n");
	DP_DEBUG("dsc_supported: %d\n", dsc_caps->dsc_supported);
	DP_DEBUG("rate_buffer_size: %d kbytes\n", dsc_caps->rate_buffer_size_kbytes);
	DP_DEBUG("dsc_version_minor: %d\n", dsc_caps->dsc_version_minor);
	DP_DEBUG("dsc_version_major: %d\n", dsc_caps->dsc_version_major);
	DP_DEBUG("max_slice_width: %d\n", dsc_caps->max_slice_width);
	for (i = 0; i < dsc_caps->slice_counts_size; i++)
		DP_DEBUG("slice_counts_size: %d\n", dsc_caps->slice_counts[i]);
	DP_DEBUG("max_line_buffer_depth: %d\n", dsc_caps->max_line_buffer_depth);
	DP_DEBUG("block_predication_supported: %d\n", dsc_caps->block_predication_supported);
	DP_DEBUG("rgb_supported: %d\n", dsc_caps->rgb_supported);
	for (i = 0; i < dsc_caps->bpcs_size; i++)
		DP_DEBUG("bpcs: %d\n", dsc_caps->bpcs[i]);
	DP_DEBUG("==================================================\n");
}

static void dp_panel_dump_hdr_caps(struct dp_panel *panel, u8 feature_enum_dpcd)
{
	struct dp_hdr_caps_sink *hdr_caps = &panel->sink_caps.hdr_caps;
	struct edid *edid = panel->edid;

	DP_DEBUG("============== HDR SINK CAPABILITIES ==============\n");
	DP_DEBUG("hdr caps valid: %d\n", hdr_caps->caps_valid);
	DP_DEBUG("eotf supported: 0x%02x\n", hdr_caps->eotf_supported);
	DP_DEBUG("static metadata type supported: 0x%02x\n",
			hdr_caps->static_metadata_type_supported);
	DP_DEBUG("max cll: %d\n", hdr_caps->max_cll);
	DP_DEBUG("max_fall: %d\n", hdr_caps->max_fall);
	DP_DEBUG("min_cll: %d\n", hdr_caps->min_cll);
	DP_DEBUG("hdr_dolby_supported: %d\n", hdr_caps->hdr_dolby_supported);
	DP_DEBUG("hdr_dolby_metadata_version: 0x%02x\n",
			hdr_caps->hdr_dolby_metadata_version);
	DP_DEBUG("hdr10_plus_supported: %d\n", hdr_caps->hdr10_plus_supported);
	DP_DEBUG("hdr10_plus_metadata_version: 0x%02x\n",
			hdr_caps->hdr10_plus_metadata_version);

	DP_DEBUG("dprx vsc_ext_cea_sdp_supported: %d\n",
			!!(feature_enum_dpcd & DP_VSC_EXT_CEA_SDP_SUPPORTED));
	DP_DEBUG("dprx vsc_ext_cea_sdp_chaining_supported: %d\n",
			!!(feature_enum_dpcd & DP_VSC_EXT_CEA_SDP_CHAINING_SUPPORTED));

	if (edid) {
		DP_DEBUG("SINK CHROMATICITY COORDINATES (units of 0.001)\n");
		DP_DEBUG("red: (%lu, %lu)\n",
				((edid->red_x << 2) | GET_BITS_VAL(edid->red_green_lo, 6, 2)) * 1000 / 1024,
				((edid->red_y << 2) | GET_BITS_VAL(edid->red_green_lo, 4, 2)) * 1000 / 1024);
		DP_DEBUG("green: (%lu, %lu)\n",
				((edid->green_x << 2) | GET_BITS_VAL(edid->red_green_lo, 2, 2)) * 1000 / 1024,
				((edid->green_y << 2) | GET_BITS_VAL(edid->red_green_lo, 0, 2)) * 1000 / 1024);
		DP_DEBUG("blue: (%lu, %lu)\n",
				((edid->blue_x << 2) | GET_BITS_VAL(edid->blue_white_lo, 6, 2)) * 1000 / 1024,
				((edid->blue_y << 2) | GET_BITS_VAL(edid->blue_white_lo, 4, 2)) * 1000 / 1024);
		DP_DEBUG("white point: (%lu, %lu)\n",
				((edid->white_x << 2) | GET_BITS_VAL(edid->blue_white_lo, 2, 2)) * 1000 / 1024,
				((edid->white_y << 2) | GET_BITS_VAL(edid->blue_white_lo, 0, 2)) * 1000 / 1024);
	}

	DP_DEBUG("SINK COLORIMETRY\n");
	DP_DEBUG("colorimetry_supported: 0x%02x\n", hdr_caps->colorimetry_supported);
	DP_DEBUG("max_metadata_size: %d\n", hdr_caps->max_metadata_size);

	DP_DEBUG("==================================================\n");
}

static void __maybe_unused dp_panel_dump_fec_status(struct dp_panel *panel)
{
	u8 bytes[AUX_MAX_BYTES_SIZE];
	int i;

	DP_INFO("DPCD FEC status\n");
	drm_dp_dpcd_readb(&panel->aux->base, DP_FEC_CONFIGURATION, &bytes[0]);
	DP_INFO("dpcd addr:0x%04x, value:0x%02x", DP_FEC_CONFIGURATION, bytes[0]);

	drm_dp_dpcd_read(&panel->aux->base, DP_FEC_STATUS, bytes,
			DP_FEC_STATUS_SIZE);
	for (i = 0; i < DP_FEC_STATUS_SIZE; i++)
		DP_INFO("dpcd addr:0x%04x, value:0x%02x", DP_FEC_STATUS + i, bytes[i]);
}

static void __maybe_unused dp_panel_dump_crc_status(struct dp_panel *panel)
{
	u8 crc_dpcd[AUX_MAX_BYTES_SIZE];
	u8 i;

	if (drm_dp_dpcd_read(&panel->aux->base, DP_DSC_CRC_0,
			crc_dpcd, DP_DSC_CRC_STATUS_SIZE) < DP_DSC_CRC_STATUS_SIZE)
		return;

	DP_INFO("DPCD CRC status\n");
	for (i = 0; i < DP_DSC_CRC_STATUS_SIZE; i++)
		DP_INFO("dpcd addr:0x%04x, value:0x%02x\n",
				DP_DSC_CRC_0 + i, crc_dpcd[i]);
}

static void __maybe_unused dp_panel_dump_dsc_status(struct dp_panel *panel)
{
	struct drm_dp_aux *aux = &panel->aux->base;
	u8 status;

	DP_DEBUG("DPCD DSC status\n");

	if (drm_dp_dpcd_readb(aux, DP_DSC_ENABLE, &status) < 1)
		return;

	DP_DEBUG("dpcd addr:0x%0x, value:0x%0x\n", DP_DSC_ENABLE, status);

	if (drm_dp_dpcd_readb(aux, DP_DSC_STATUS, &status) < 1)
		return;

	DP_DEBUG("dpcd addr:0x%0x, value:0x%0x\n", DP_DSC_STATUS, status);
	DP_DEBUG("dpcd RC-buffer under run:%d\n", !!(status & BIT(0)));
	DP_DEBUG("dpcd RC-buffer overflow:%d\n", !!(status & BIT(1)));
	DP_DEBUG("dpcd chunk length error:%d\n", !!(status & BIT(2)));

	/* clear dsc status */
	drm_dp_dpcd_writeb(aux, DP_DSC_STATUS, 0xFF);
}

static u8 dp_panel_sink_supported_slice_counts(u8 dsc_dpcd[SINK_DSC_CAP_SIZE],
		u8 slice_counts[SINK_DSC_CAP_SLICE_COUNT_SIZE])
{
	u8 num_slice_counts = 0;
	u8 byte = dsc_dpcd[DP_DSC_SLICE_CAP_2 - DP_DSC_SUPPORT];

	if (byte & DP_DSC_24_PER_DP_DSC_SINK)
		slice_counts[num_slice_counts++] = 24;
	if (byte & DP_DSC_20_PER_DP_DSC_SINK)
		slice_counts[num_slice_counts++] = 20;
	if (byte & DP_DSC_16_PER_DP_DSC_SINK)
		slice_counts[num_slice_counts++] = 16;

	byte = dsc_dpcd[DP_DSC_SLICE_CAP_1 - DP_DSC_SUPPORT];
	if (byte & DP_DSC_12_PER_DP_DSC_SINK)
		slice_counts[num_slice_counts++] = 12;
	if (byte & DP_DSC_10_PER_DP_DSC_SINK)
		slice_counts[num_slice_counts++] = 10;
	if (byte & DP_DSC_8_PER_DP_DSC_SINK)
		slice_counts[num_slice_counts++] = 8;
	if (byte & DP_DSC_6_PER_DP_DSC_SINK)
		slice_counts[num_slice_counts++] = 6;
	if (byte & DP_DSC_4_PER_DP_DSC_SINK)
		slice_counts[num_slice_counts++] = 4;
	if (byte & DP_DSC_2_PER_DP_DSC_SINK)
		slice_counts[num_slice_counts++] = 2;
	if (byte & DP_DSC_1_PER_DP_DSC_SINK)
		slice_counts[num_slice_counts++] = 1;

	return num_slice_counts;
}

static const u32 DSC_THROUGHPUT_TABLE[] = {
	0, 340, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 170,
};

static void dp_panel_update_dsc_caps(struct dp_panel *panel)
{
	struct dsc_caps_sink *dsc_caps = &panel->sink_caps.dsc_caps;
	u8 *dsc_dpcd = panel->dsc_dpcd;
	/* in units of KB */
	u8 block_size_table[] = {1, 4, 16, 64};
	u32 block_size, block_count;
	u8 throughput_0_idx;
	u8 idx;

	dsc_caps->dsc_supported = !!(dsc_dpcd[DP_DSC_SUPPORT - DP_DSC_SUPPORT] &
			DP_DSC_DECOMPRESSION_IS_SUPPORTED);

	if (!dsc_caps->dsc_supported)
		return;

	dsc_caps->dsc_version_minor = GET_BITS_VAL(
			dsc_dpcd[DP_DSC_REV - DP_DSC_SUPPORT], 4, 4);
	dsc_caps->dsc_version_major = GET_BITS_VAL(
			dsc_dpcd[DP_DSC_REV - DP_DSC_SUPPORT], 0, 4);
	dsc_caps->block_predication_supported =
			dsc_dpcd[DP_DSC_BLK_PREDICTION_SUPPORT - DP_DSC_SUPPORT] &
			DP_DSC_BLK_PREDICTION_IS_SUPPORTED;
	dsc_caps->max_slice_width =
			dsc_dpcd[DP_DSC_MAX_SLICE_WIDTH - DP_DSC_SUPPORT] *
			DP_DSC_SLICE_WIDTH_MULTIPLIER;
	dsc_caps->rgb_supported =
			dsc_dpcd[DP_DSC_DEC_COLOR_FORMAT_CAP - DP_DSC_SUPPORT] &
			DP_DSC_RGB;

	idx = dsc_dpcd[DP_DSC_RC_BUF_BLK_SIZE - DP_DSC_SUPPORT] & 0x3;
	block_size = block_size_table[idx];
	block_count = dsc_dpcd[DP_DSC_RC_BUF_SIZE - DP_DSC_SUPPORT] + 1;
	dsc_caps->rate_buffer_size_kbytes = block_size * block_count;

	dsc_caps->max_line_buffer_depth = drm_dp_dsc_sink_line_buf_depth(dsc_dpcd);
	dsc_caps->slice_counts_size = dp_panel_sink_supported_slice_counts(
			dsc_dpcd, dsc_caps->slice_counts);
	dsc_caps->bpcs_size = drm_dp_dsc_sink_supported_input_bpcs(dsc_dpcd,
			dsc_caps->bpcs);

	throughput_0_idx = dsc_dpcd[DP_DSC_PEAK_THROUGHPUT - DP_DSC_SUPPORT] &
			DP_DSC_THROUGHPUT_MODE_0_MASK;
	dsc_caps->throughput_0_MP = DSC_THROUGHPUT_TABLE[throughput_0_idx];
}

static void dp_panel_update_link_caps(struct dp_panel *panel)
{
	struct dp_link_caps_sink *link_caps = &panel->sink_caps.link_caps;
	u8 *dpcd_caps = panel->dpcd_caps;

	link_caps->max_lane_count = drm_dp_max_lane_count(dpcd_caps);
	link_caps->max_link_rate = dp_link_rate_to_dptx_code(dpcd_caps[DP_MAX_LINK_RATE]);
	link_caps->fec_supported = drm_dp_sink_supports_fec(panel->fec_dpcd);
	link_caps->ssc_supported = drm_dp_max_downspread(dpcd_caps);
	link_caps->tps3_supported = drm_dp_tps3_supported(dpcd_caps);
	link_caps->tps4_supported = drm_dp_tps4_supported(dpcd_caps);
	link_caps->enhance_frame_supported = drm_dp_enhanced_frame_cap(dpcd_caps);
}

static u8 *dp_panel_find_cea_extension_block(struct edid *edid)
{
	u8 *cea_blk;
	u8 i;

	if (edid == NULL || edid->extensions == 0) {
		DP_INFO("There is no extension block in EDID\n");
		return NULL;
	}

	for (i = 1; i <= edid->extensions; i++) {
		cea_blk = (u8 *)edid + 128 * i;
		if (cea_blk[0] == 0x02)
			return cea_blk;
	}

	DP_INFO("There is no CEA extension block in EDID\n");
	return NULL;
}

static void dp_panel_parse_cta_audio_data_blk(u8 *cea_db,
		struct dp_audio_cta_blk *blk)
{
	u8 len = cea_data_blk_len(cea_db);
	u8 i;

	if (len > CTA_AUDIO_DATA_BLOCK_SIZE || len < 3) {
		DP_INFO("invalid cta audio data block size (%d)\n", len - 1);
		return;
	}

	memcpy(blk->audio_data_blk, cea_db, len);
	blk->audio_data_blk_valid = true;

	DP_DEBUG("cta audio data block:\n");
	for (i = 0; i < len; i++)
		DP_DEBUG("0x%02x\n", cea_db[i]);
}

static void dp_panel_parse_cta_speaker_allocation_data_blk(u8 *cea_db,
		struct dp_audio_cta_blk *blk)
{
	u8 len = cea_data_blk_len(cea_db);
	u8 i;

	if (len != CTA_SPEAKER_ALLOCATION_DATA_BLOCK_SIZE) {
		DP_INFO("invalid cta speaker allocation data block size (%d)\n", len - 1);
		return;
	}

	memcpy(blk->spk_alloc_data_blk, cea_db, len);
	blk->spk_alloc_data_blk_valid = true;

	DP_DEBUG("cta speaker allocation data block:\n");
	for (i = 0; i < len; i++)
		DP_DEBUG("0x%02x\n", cea_db[i]);
}

static void dp_panel_parse_edid_static_metadata(u8 *cea_db,
		struct dp_hdr_caps_sink *hdr_caps)
{
	u8 len = cea_data_blk_len(cea_db);

	hdr_caps->caps_valid = true;
	hdr_caps->eotf_supported = cea_db[2];
	hdr_caps->static_metadata_type_supported = cea_db[3];

	if (len >= 5)
		hdr_caps->max_cll = cea_db[4];
	if (len >= 6)
		hdr_caps->max_fall = cea_db[5];
	if (len >= 7)
		hdr_caps->min_cll = cea_db[6];
}

static void dp_panel_parse_edid_dynamic_metadata(u8 *cea_db,
		struct dp_hdr_caps_sink *hdr_caps)
{
	u8 len = cea_data_blk_len(cea_db);
	u16 metadata_type;
	u8 type_len;
	u8 i;

	for (i = 2; i < len; i += (type_len + 1)) {
		type_len = cea_db[i];
		if (type_len < 2)
			goto error;

		metadata_type = 0;
		metadata_type = MERGE_BITS(metadata_type, cea_db[i + 1], 0, 8);
		metadata_type = MERGE_BITS(metadata_type, cea_db[i + 2], 8, 16);

		switch (metadata_type) {
		case DYNAMIC_METADATA_TYPE_DOLBY:
			hdr_caps->hdr_dolby_supported = true;
			hdr_caps->hdr_dolby_metadata_version = cea_db[i+3];
			break;
		case DYNAMIC_METADATA_TYPE_HDR10_PLUS:
			hdr_caps->hdr10_plus_supported = true;
			hdr_caps->hdr10_plus_metadata_version = cea_db[i+3];
			break;
		default:
			break;
		}
	}

	return;

error:
	hdr_caps->hdr10_plus_metadata_version = 0;
	hdr_caps->hdr10_plus_supported = false;
	hdr_caps->hdr_dolby_metadata_version = 0;
	hdr_caps->hdr_dolby_supported = false;
}

static void dp_panel_parse_colorimetry(u8 *cea_db,
		struct dp_hdr_caps_sink *hdr_caps)
{
	u8 len = cea_data_blk_len(cea_db);

	if (len != 0x04)
		return;

	if ((cea_db[2] & BIT(0)) || (cea_db[2] & BIT(2)))
		hdr_caps->colorimetry_supported |= DP_COLORIMETRY_BT601;
	if (cea_db[2] & BIT(1))
		hdr_caps->colorimetry_supported |= DP_COLORIMETRY_BT709;
	if (cea_db[2] & BIT(4))
		hdr_caps->colorimetry_supported |= DP_COLORIMETRY_ADOBE_RGB;
	if ((cea_db[2] & BIT(5)) || (cea_db[2] & BIT(6)) || (cea_db[2] & BIT(7)))
		hdr_caps->colorimetry_supported |= DP_COLORIMETRY_BT2020;
	if (cea_db[3] & BIT(7))
		hdr_caps->colorimetry_supported |= DP_COLORIMETRY_DCIP3;
}

static void dp_panel_parse_edid_cta(struct dp_panel *panel)
{
	struct dp_audio_cta_blk *audio_blk = &panel->sink_caps.audio_blk;
	struct dp_hdr_caps_sink *hdr_caps = &panel->sink_caps.hdr_caps;
	struct edid *edid = panel->edid;
	u8 *cea_blk, *cea_db;
	u8 start, end, tag_code;
	u32 i;

	cea_blk = dp_panel_find_cea_extension_block(edid);
	if (!cea_blk)
		return;

	audio_blk->basic_audio_supported = cea_blk[3] & BIT(6);

	start = 4;
	end = cea_blk[2];
	for_each_cea_db(cea_blk, i, start, end) {
		cea_db = &cea_blk[i];
		tag_code = cea_data_blk_tag_code(cea_db);
		if (tag_code != USE_EXTENDED_TAG) {
			switch (tag_code) {
			case AUDIO_DATA_BLOCK:
				dp_panel_parse_cta_audio_data_blk(cea_db, audio_blk);
				break;
			case SPEAKER_ALLOCATION_DATA_BLOCK:
				dp_panel_parse_cta_speaker_allocation_data_blk(cea_db, audio_blk);
				break;
			default:
				break;
			}
		} else {
			switch (cea_data_blk_extended_tag_code(cea_db)) {
			case HDR_STATIC_METADATA_BLOCK:
				dp_panel_parse_edid_static_metadata(cea_db, hdr_caps);
				break;
			case HDR_DYNAMIC_METADATA_BLOCK:
				dp_panel_parse_edid_dynamic_metadata(cea_db, hdr_caps);
				break;
			case COLORIMETRY_BLOCK:
				dp_panel_parse_colorimetry(cea_db, hdr_caps);
				break;
			default:
				break;
			}
		}
	}
}

static bool check_static_metadata_infoframe(
		struct dp_hdr_caps_sink *hdr_caps, struct dp_hdr_static_metadata *data)
{
	if (!(hdr_caps->eotf_supported & data->eotf)) {
		DP_ERROR("unsupported eotf: %u, caps: %u\n",
				data->eotf, hdr_caps->eotf_supported);
		return false;
	}

	return true;
}

static bool check_dynamic_metadata_infoframe(
		struct dp_hdr_caps_sink *hdr_caps, struct dp_hdr_dynamic_metadata *data)
{
	if (data->size <= MIN_DYNAMIC_METADATA_INFOFRAME_SIZE) {
		DP_ERROR("invalid metadata size %u\n", data->size);
		return false;
	}

	if (data->size > hdr_caps->max_metadata_size) {
		DP_ERROR("invalid metadata size %u, caps: %u\n",
				data->size, hdr_caps->max_metadata_size);
		return false;
	}

	if (data->data[1] != 0) {
		DP_ERROR("wrong dynamic metadata format\n");
		return false;
	}

	if (data->data[0] == 0x01 && !hdr_caps->hdr_dolby_supported) {
		DP_ERROR("unsupported hdr dolby metadata\n");
		return false;
	}

	if (data->data[0] == 0x04 && !hdr_caps->hdr10_plus_supported) {
		DP_ERROR("unsupported hdr 10 plus metadata\n");
		return false;
	}

	return true;
}

static void dp_panel_dump_sdp(struct dp_sdp_packet *sdp, u8 count)
{
	u32 i, j;

	for (i = 0; i < count; i++) {
		DP_DEBUG("sdp [%d]:\n", i);
		DP_DEBUG("header:0x%08x\n", sdp[i].header);
		for (j = 0; j < 8; j++)
			DP_DEBUG("payload[%d]:0x%08x\n", j, sdp[i].payload[j]);
	}
}

static void dp_panel_send_static_metadata(struct dp_panel *panel,
		struct dp_hdr_static_metadata *metadata)
{
	struct dp_hw_sctrl *hw_sctrl = panel->hw_sctrl;
	struct dp_sdp_packet sdp;
	u8 eotf_code;

	switch (metadata->eotf) {
	case DP_EOTF_TRADITIONAL_GAMMA_SDR:
		eotf_code = 0;
		break;
	case DP_EOTF_TRADITIONAL_GAMMA_HDR:
		eotf_code = 1;
		break;
	case DP_EOTF_PQ:
		eotf_code = 2;
		break;
	case DP_EOTF_HLG:
		eotf_code = 3;
		break;
	default:
		eotf_code = 4;
	}

	sdp.header = MERGE_32(0x13 << 2, 0x1D, 0x87, 0x00);

	sdp.payload[0] = MERGE_32(0x00, eotf_code, 0x1A, 0x01);
	sdp.payload[1] = MERGE_32(
		GET_BITS_VAL(metadata->display_primaries_red_y, 8, 8),
		GET_BITS_VAL(metadata->display_primaries_red_y, 0, 8),
		GET_BITS_VAL(metadata->display_primaries_red_x, 8, 8),
		GET_BITS_VAL(metadata->display_primaries_red_x, 0, 8)
	);
	sdp.payload[2] = MERGE_32(
		GET_BITS_VAL(metadata->display_primaries_green_y, 8, 8),
		GET_BITS_VAL(metadata->display_primaries_green_y, 0, 8),
		GET_BITS_VAL(metadata->display_primaries_green_x, 8, 8),
		GET_BITS_VAL(metadata->display_primaries_green_x, 0, 8)
	);
	sdp.payload[3] = MERGE_32(
		GET_BITS_VAL(metadata->display_primaries_blue_y, 8, 8),
		GET_BITS_VAL(metadata->display_primaries_blue_y, 0, 8),
		GET_BITS_VAL(metadata->display_primaries_blue_x, 8, 8),
		GET_BITS_VAL(metadata->display_primaries_blue_x, 0, 8)
	);
	sdp.payload[4] = MERGE_32(
		GET_BITS_VAL(metadata->white_point_y, 8, 8),
		GET_BITS_VAL(metadata->white_point_y, 0, 8),
		GET_BITS_VAL(metadata->white_point_x, 8, 8),
		GET_BITS_VAL(metadata->white_point_x, 0, 8)
	);
	sdp.payload[5] = MERGE_32(
		GET_BITS_VAL(metadata->min_display_luminance, 8, 8),
		GET_BITS_VAL(metadata->min_display_luminance, 0, 8),
		GET_BITS_VAL(metadata->max_display_luminance, 8, 8),
		GET_BITS_VAL(metadata->max_display_luminance, 0, 8)
	);
	sdp.payload[6] = MERGE_32(
		GET_BITS_VAL(metadata->max_fall, 8, 8),
		GET_BITS_VAL(metadata->max_fall, 0, 8),
		GET_BITS_VAL(metadata->max_cll, 8, 8),
		GET_BITS_VAL(metadata->max_cll, 0, 8)
	);
	sdp.payload[7] = 0;
	dp_panel_dump_sdp(&sdp, 1);
	hw_sctrl->ops->send_ext_sdp(&hw_sctrl->hw, &sdp, 1);
}

static void dp_panel_send_dynamic_metadata(struct dp_panel *panel,
		struct dp_hdr_dynamic_metadata *metadata)
{
	struct dp_hw_sctrl *hw_sctrl = panel->hw_sctrl;
	struct dp_sdp_packet sdp[DP_MAX_EXT_SDP_COUNT];
	u8 byte0, byte1, byte2, byte3;
	u32 i = 0, count;

	memset((void *)sdp, 0, sizeof(struct dp_sdp_packet) * DP_MAX_EXT_SDP_COUNT);

	while (i < metadata->size) {
		byte0 = i < metadata->size ? metadata->data[i] : 0;
		i++;
		byte1 = i < metadata->size ? metadata->data[i] : 0;
		i++;
		byte2 = i < metadata->size ? metadata->data[i] : 0;
		i++;
		byte3 = i < metadata->size ? metadata->data[i] : 0;

		sdp[i / 32].payload[(i % 32) / 4] = MERGE_32(byte3, byte2, byte1, byte0);
		i++;
	}

	count = (i - 1) / 32 + 1;
	/* set field packet_sequence of sdps */
	for (i = 0; i < count; i++)
		sdp[i].header = MERGE_32(i, 0xC0, 0x21, 0x00);

	/* set the bit middle_of_chaining of the last sdp to be 0 */
	sdp[count - 1].header &= ~BIT(23);
	dp_panel_dump_sdp(&sdp[0], count);
	hw_sctrl->ops->send_ext_sdp(&hw_sctrl->hw, sdp, count);
}

int dp_panel_config_hdr_commit(struct dp_panel *panel,
		struct dp_connector_state *dp_conn_state)
{
	struct dp_hw_sctrl *hw_sctrl;
	struct hdr_state *hdr_state;
	unsigned long flag = 0;

	if (!panel || !dp_conn_state) {
		DP_ERROR("invalid parameters, %pK, %pK\n", panel, dp_conn_state);
		return -EINVAL;
	}

	hw_sctrl = panel->hw_sctrl;
	hdr_state = &panel->hdr_state;

	spin_lock_irqsave(&hdr_state->lock, flag);

	switch (dp_conn_state->metadata.type) {
	case DP_STATIC_METADATA:
		if (!check_static_metadata_infoframe(&panel->sink_caps.hdr_caps,
				&dp_conn_state->static_metadata)) {
			DP_ERROR("invalid static metadata\n");
			goto error;
		}

		if (!hdr_state->ext_sdp_on) {
			DP_DEBUG("power on ext_sdp module\n");
			hdr_state->ext_sdp_on = true;
			hw_sctrl->ops->power_manage_ext_sdp(&hw_sctrl->hw, true);
		}

		hw_sctrl->ops->suspend_ext_sdp(&hw_sctrl->hw);
		DP_DEBUG("send static metadata\n");
		dp_panel_send_static_metadata(panel, &dp_conn_state->static_metadata);

		hdr_state->power_off_flag = (dp_conn_state->static_metadata.eotf ==
				DP_EOTF_TRADITIONAL_GAMMA_SDR);
		hdr_state->repeat_times = DP_STATIC_METADATA_REPEAT_TIME;

		break;
	case DP_DYNAMIC_METADATA_HDR10_PLUS:
		if (!check_dynamic_metadata_infoframe(&panel->sink_caps.hdr_caps,
				&dp_conn_state->dynamic_metadata)) {
			DP_ERROR("invalid dynamic metadata\n");
			goto error;
		}

		if (!hdr_state->ext_sdp_on) {
			DP_DEBUG("power on ext_sdp module\n");
			hdr_state->ext_sdp_on = true;
			hw_sctrl->ops->power_manage_ext_sdp(&hw_sctrl->hw, true);
		}

		hw_sctrl->ops->suspend_ext_sdp(&hw_sctrl->hw);
		DP_DEBUG("send dynamic metadata\n");
		dp_panel_send_dynamic_metadata(panel, &dp_conn_state->dynamic_metadata);

		hdr_state->power_off_flag = false;
		hdr_state->repeat_times = DP_DYNAMIC_METADATA_REPEAT_TIME;

		break;
	default:
		DP_ERROR("unknown metadata type\n");
	}

	spin_unlock_irqrestore(&hdr_state->lock, flag);
	return 0;
error:
	spin_unlock_irqrestore(&hdr_state->lock, flag);
	return -EINVAL;
}

void dp_panel_send_metadata_intr_handler(struct dp_panel *panel)
{
	struct dp_hw_sctrl *hw_sctrl;
	struct hdr_state *hdr_state;
	unsigned long flag = 0;

	if (!panel) {
		DP_ERROR("invalid parameters, %pK\n", panel);
		return;
	}

	hw_sctrl = panel->hw_sctrl;
	hdr_state = &panel->hdr_state;

	spin_lock_irqsave(&hdr_state->lock, flag);

	if (hdr_state->ext_sdp_on) {
		if (hdr_state->repeat_times >= 0)
			hdr_state->repeat_times--;

		if (hdr_state->repeat_times == 0) {
			if (hdr_state->power_off_flag) {
				hdr_state->ext_sdp_on = false;
				DP_DEBUG("power off ext_sdp\n");
				hw_sctrl->ops->power_manage_ext_sdp(&hw_sctrl->hw, false);
			} else {
				DP_DEBUG("suspend ext_sdp\n");
				hw_sctrl->ops->suspend_ext_sdp(&hw_sctrl->hw);
			}
		}
	}

	spin_unlock_irqrestore(&hdr_state->lock, flag);
}

static void dp_panel_enable_dsc(struct dp_panel *panel, bool enable)
{
	struct drm_dp_aux *aux = &panel->aux->base;
	struct dp_hw_ctrl *hw_ctrl = panel->hw_ctrl;

	if (!panel->sink_caps.dsc_caps.dsc_supported) {
		DP_INFO("DSC is not supported\n");
		return;
	}

	if (drm_dp_dpcd_writeb(aux, DP_DSC_ENABLE, enable) < 1)
		DP_WARN("failed to write DPCD DSC ENABLE\n");

	hw_ctrl->ops->enable_dsc(&hw_ctrl->hw, enable);
	DP_INFO("DP DSC is %s\n", enable ? "enabled" : "disabled");
}

static int __maybe_unused dp_panel_get_modes(struct dp_panel *panel,
		struct drm_connector *connector)
{
	if (panel->edid) {
		drm_connector_update_edid_property(connector, panel->edid);

		return drm_add_edid_modes(connector, panel->edid);
	}

	drm_connector_update_edid_property(connector, NULL);

	return 0;
}

static u32 dp_panel_t1_calculate(enum dp_lane_count lane_count,
		enum dp_bpc bpc, enum pixel_format pixel_format)
{
	u32 t1;

	switch (bpc) {
	case DP_BPC_6:
		t1 = (4 * 1000 / 9) * lane_count;
		break;
	case DP_BPC_8:
		if (pixel_format == PIXEL_FORMAT_YCBCR422)
			t1 = (1000 / 2) * lane_count;
		else if (pixel_format == PIXEL_FORMAT_YONLY)
			t1 = lane_count * 1000;
		else
			t1 = (3000 / 16) * lane_count;
		break;
	case DP_BPC_10:
		if (pixel_format == PIXEL_FORMAT_YCBCR422)
			t1 = (2000 / 5) * lane_count;
		else if (pixel_format == PIXEL_FORMAT_YONLY)
			t1 = (4000 / 5) * lane_count;
		else
			t1 = (4000 / 15) * lane_count;
		break;
	case DP_BPC_12:
		if (pixel_format == PIXEL_FORMAT_YCBCR422)
			t1 = (1000 / 3) * lane_count;
		else if (pixel_format == PIXEL_FORMAT_YONLY)
			t1 = (2000 / 3) * lane_count;
		else
			t1 = (2000 / 9) * lane_count;
		break;
	case DP_BPC_16:
		if (pixel_format == PIXEL_FORMAT_YONLY)
			t1 = (1000 / 2) * lane_count;
		else
			t1 = (1000 / 4) * lane_count;
		break;
	}

	return t1;
}

static u32 dp_panel_calculate_init_threshold(
		struct init_threshold_calculate_param *param)
{
	u32 t1 = 0, t2 = 0, t3 = 0;
	u8 fec_slot_cnt = 0;
	u8 slot_cnt = 0;
	u32 init_threshold;

	if (param->dsc_en) {
		t1 = (8 * 1000 / param->color_depth) * param->lane_count;

		t2 = param->link_clk_khz * 2 * 1000 / param->pixel_clock_khz;

		fec_slot_cnt = param->fec_en ? (param->lane_count == 1 ? 13 : 7) : 0;
		slot_cnt = param->tu_valid_bytes + fec_slot_cnt;
		slot_cnt = param->tu_valid_bytes_frac > 0 ? slot_cnt + 1 : slot_cnt;
		while (slot_cnt % 4 != 0)
			slot_cnt++;

		t3 = slot_cnt;

		init_threshold = (u64)(t1) * t2 * t3 / (1000 * 1000);

		DP_DEBUG("t1:%d, t2:%d, t3:%d, t1*t2*t3:%d\n",
				t1, t2, t3, init_threshold);
		if (init_threshold < 32 && param->tu_valid_bytes <= 8)
			init_threshold = 32;

		return init_threshold;
	}

	t1 = dp_panel_t1_calculate(param->lane_count, param->bpc, param->pixel_format);

	t2 = param->link_clk_khz * 2 * 1000 / param->pixel_clock_khz;
	t2 = param->pixel_format == PIXEL_FORMAT_YCBCR420 ? t2 * 2 : t2;

	fec_slot_cnt = param->fec_en ? ((param->lane_count == 1) ? 13 : 7) : 0;
	slot_cnt = param->tu_valid_bytes  + fec_slot_cnt;
	slot_cnt = (param->tu_valid_bytes_frac > 0) ? slot_cnt + 1 : slot_cnt;
	t3 = slot_cnt;

	/* fraction, stored with 1000*1000 times, need to be divided here */
	init_threshold = (u64)(t1) * t2 * t3 / (1000 * 1000);

	DP_DEBUG("t1:%d, t2:%d, t3:%d, init_threshold:%d\n",
			t1, t2, t3, init_threshold);
	if (init_threshold <= 16 || param->tu_valid_bytes < 10)
		init_threshold = 40;

	return init_threshold;
}

u32 dp_panel_get_color_depth(enum dp_bpc bpc, enum pixel_format pixel_format)
{
	u32 color_depth;

	if (bpc == DP_BPC_6)
		return 18;

	switch (pixel_format) {
	case PIXEL_FORMAT_YCBCR420:
		color_depth = bpc / 2 * 3;
		break;
	case PIXEL_FORMAT_YCBCR422:
		color_depth = bpc * 2;
		break;
	case PIXEL_FORMAT_YONLY:
		color_depth = bpc;
		break;
	default:
		color_depth = bpc * 3;
	}

	return color_depth;
}

static int dp_panel_calculate_tu(struct tu_calculate_param *param,
		struct dp_hw_ctrl_tu_config *tu)
{
	u32 valid_bytes, valid_bytes_frac;
	u32 ts;

	/**
	 * example:
	 * pixel_clk=80000khz; color_depth=30; lane_count=4; rate=2700(Mbps)
	 *
	 * the variable ts:
	 * ts = (pixel_clk * color_depth / 8) / (lane_count * rate / 10) * 64
	 *    = (pixel_clk * color_depth * 8) / (lane_count * rate / 10)
	 * ts = (30 * 80000(k) * 8) / (4 * 2700(M) / 10) = 17777
	 * the variable valid_bytes:
	 * valid_bytes = ts / 1000 = 17
	 *
	 * real pixel_symbol_rate: (80M * 30 / 8)
	 * real link_symbol_rate: (4 * 2.7G) / 10
	 * the real valid_bytes = pixel_symbol_rate / link_symbol_rate * 64 = 17.777
	 */

	if (param->fec_en)
		ts = ((u64)(param->color_depth * param->pixel_clock_khz * 80) * 1000) /
				((u64)(param->lane_count * param->link_rate_mbps) * (1000 - 24));
	else
		ts = (param->color_depth * param->pixel_clock_khz * 80) /
				(param->lane_count * param->link_rate_mbps);

	valid_bytes  = ts / 1000;
	valid_bytes_frac = ts / 100 - valid_bytes * 10;

	if (valid_bytes > 64 || (valid_bytes == 64 && valid_bytes_frac > 0)) {
		DP_ERROR("tu calculation error, valid bytes:%d, valid bytes frac:%d\n",
				valid_bytes, valid_bytes_frac);
		return -EINVAL;
	}

	tu->valid_bytes = valid_bytes;
	tu->valid_bytes_frac = valid_bytes_frac;
	DP_DEBUG("tu:%u, tu_frac:%u", valid_bytes, valid_bytes_frac);

	return 0;
}

static int dp_panel_get_edid(struct dp_panel *panel, struct drm_connector *conn)
{
	struct drm_dp_aux *aux = &panel->aux->base;

	if (panel->edid != NULL)
		kfree(panel->edid);

	panel->edid = drm_get_edid(conn, &aux->ddc);

	return (panel->edid == NULL) ? -EPROTO : 0;
}

static int dp_panel_get_caps(struct dp_panel *panel, struct drm_connector *conn)
{
	struct dp_panel_caps *caps = &panel->sink_caps;
	struct drm_dp_aux *aux = &panel->aux->base;
	u8 feature_enum_dpcd = 0;

	if (dp_panel_get_edid(panel, conn)) {
		DP_ERROR("failed to get edid\n");
		return -EPROTO;
	}

	/* read dpcd caps field */
	if (drm_dp_read_dpcd_caps(aux, panel->dpcd_caps)) {
		DP_ERROR("failed to read dpcd caps\n");
		return -EPROTO;
	}

	if (drm_dp_read_lttpr_common_caps(aux, panel->dpcd_caps,
			panel->lttpr_common_dpcd)) {
		DP_WARN("failed to read lttpr common caps\n");
		memset(panel->lttpr_common_dpcd, 0, DP_LTTPR_COMMON_CAP_SIZE);
	}

	if (drm_dp_dpcd_readb(aux, DP_FEC_CAPABILITY, &panel->fec_dpcd) < 1) {
		DP_ERROR("failed to read sink fec capabilities\n");
		return -EPROTO;
	}

	if (drm_dp_dpcd_read(aux, DP_DSC_SUPPORT, panel->dsc_dpcd,
			SINK_DSC_CAP_SIZE) < SINK_DSC_CAP_SIZE) {
		DP_ERROR("failed to read sink dsc capabilities\n");
		return -EPROTO;
	}

	if (drm_dp_dpcd_readb(aux, DP_DPRX_FEATURE_ENUMERATION_LIST,
			&feature_enum_dpcd) < 1) {
		DP_ERROR("failed to read sink feature_enumeration_list\n");
		return -EPROTO;
	}

	/* update sink caps */
	caps->dpcd_version = panel->dpcd_caps[DP_DPCD_REV];
	/* dsc caps */
	dp_panel_update_dsc_caps(panel);

	/* link caps */
	dp_panel_update_link_caps(panel);

	/* audio, hdr caps */
	if (panel->edid)
		dp_panel_parse_edid_cta(panel);

	if ((feature_enum_dpcd & DP_VSC_EXT_CEA_SDP_SUPPORTED) == 0) {
		caps->hdr_caps.hdr10_plus_metadata_version = 0;
		caps->hdr_caps.hdr10_plus_supported = false;
		caps->hdr_caps.hdr_dolby_metadata_version = 0;
		caps->hdr_caps.hdr_dolby_supported = false;
	}

	caps->max_ext_sdp_cnt =
			feature_enum_dpcd & DP_VSC_EXT_CEA_SDP_CHAINING_SUPPORTED ?
			DP_MAX_EXT_SDP_COUNT : 1;
	caps->hdr_caps.max_metadata_size = caps->max_ext_sdp_cnt * DP_SDP_PAYLOAD_SIZE;

	if (!caps->link_caps.fec_supported)
		DP_INFO("sink doesn`t support fec\n");

	if (caps->dsc_caps.dsc_supported)
		dp_panel_dump_dsc_caps(panel);
	else
		DP_INFO("sink doesn`t support dsc\n");

	if (caps->hdr_caps.caps_valid)
		dp_panel_dump_hdr_caps(panel, feature_enum_dpcd);
	else
		DP_INFO("sink doesn`t support hdr\n");

	dp_connector_update_hdr_caps_property(conn);

	return 0;
}

static void dp_panel_clear_hdr_state(struct dp_panel *panel)
{
	panel->hdr_state.ext_sdp_on = false;
	panel->hdr_state.power_off_flag = false;
	panel->hdr_state.repeat_times = 0;
}

static void dp_panel_clear_video_info(struct dp_panel *panel)
{
	memcpy(&panel->video_info, &default_video_info, sizeof(default_video_info));
	memset(&panel->dsc_info, 0, sizeof(panel->dsc_info));

	dp_panel_clear_hdr_state(panel);
}

static void dp_panel_config_video(struct dp_panel *panel)
{
	struct dp_panel_video_info *video_info = &panel->video_info;
	struct dp_panel_link_info *link_info = &video_info->link_info;
	struct dp_panel_dsc_info *dsc_info = &panel->dsc_info;
	struct dp_hw_ctrl *hw_ctrl = panel->hw_ctrl;
	struct init_threshold_calculate_param init_threshold_param;
	struct dp_hw_ctrl_video_config video_config;
	struct tu_calculate_param tu_param;
	bool fec_en = link_info->fec_en;
	bool dsc_en = dsc_info->dsc_en;

	dp_panel_clear_hdr_state(panel);

	/**
	 * dsc_en is determined by sink dsc support, fec support is necessary for
	 * dsc support, it`s guaranteed by sink device. Here we don`t need to
	 * check if fec is enabled.
	 */
	dp_panel_enable_dsc(panel, dsc_en);

	/* construct hardware parameters */
	video_config.dsc_en = dsc_en;
	video_config.dsc_config = dsc_en ? dsc_info->dsc_config : NULL;
	video_config.link_clk_khz = link_info->link_clk_khz;
	video_config.timing = video_info->timing;
	video_config.msa.bpc = video_info->bpc;
	video_config.msa.colorimetry = video_info->colorimetry;
	video_config.msa.dynamic_range = video_info->dynamic_range;
	video_config.msa.pixel_format = video_info->pixel_format;
	video_config.enhance_frame_en = drm_dp_enhanced_frame_cap(panel->dpcd_caps);

	tu_param.fec_en = fec_en;
	tu_param.color_depth = video_info->timing.color_depth;
	tu_param.pixel_clock_khz = video_info->timing.pixel_clock_khz;
	tu_param.lane_count = link_info->lane_count;
	tu_param.link_rate_mbps = link_info->link_rate_mbps;
	dp_panel_calculate_tu(&tu_param, &video_config.tu);

	init_threshold_param.fec_en = fec_en;
	init_threshold_param.dsc_en = dsc_en;
	init_threshold_param.link_clk_khz = link_info->link_clk_khz;
	init_threshold_param.lane_count = link_info->lane_count;
	init_threshold_param.pixel_clock_khz = video_info->timing.pixel_clock_khz;
	init_threshold_param.bpc = video_info->bpc;
	init_threshold_param.pixel_format = video_info->pixel_format;
	init_threshold_param.tu_valid_bytes = video_config.tu.valid_bytes;
	init_threshold_param.tu_valid_bytes_frac = video_config.tu.valid_bytes_frac;
	init_threshold_param.color_depth = video_info->timing.color_depth;
	video_config.init_threshold = dp_panel_calculate_init_threshold(
			&init_threshold_param);

	hw_ctrl->ops->config_video(&hw_ctrl->hw, &video_config);

	dp_panel_dump_timing(&video_config);
}

static struct dp_panel_funcs dp_panel_funcs = {
	.get_caps                   = dp_panel_get_caps,
	.config_video               = dp_panel_config_video,
	.clear_video_info           = dp_panel_clear_video_info,
	.set_timing                 = dp_panel_set_timing,
};

int dp_panel_init(struct dp_display *display, struct dp_panel **panel)
{
	struct dp_panel *panel_priv;

	panel_priv = kzalloc(sizeof(*panel_priv), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(panel_priv))
		return -ENOMEM;

	panel_priv->aux        = display->aux;
	panel_priv->hw_ctrl    = display->hw_modules.hw_ctrl;
	panel_priv->hw_sctrl   = display->hw_modules.hw_sctrl;
	panel_priv->funcs      = &dp_panel_funcs;

	spin_lock_init(&panel_priv->hdr_state.lock);

	memcpy(&panel_priv->video_info, &default_video_info, sizeof(default_video_info));

	*panel = panel_priv;

	return 0;
}

void dp_panel_deinit(struct dp_panel *panel)
{
	kfree(panel);
}
