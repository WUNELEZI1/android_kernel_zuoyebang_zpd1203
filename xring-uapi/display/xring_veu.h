#ifndef _XRING_VEU_H_
#define _XRING_VEU_H_

#include <linux/types.h>

#define VEU_STRIPE_NUM_MAX 40

enum VEU_COLOR_SPACE {
    BT601_LIMITED = 0,
    BT601_FULL,
    BT709_LIMITED,
    BT709_FULL,
    P3_LIMITED,
    P3_FULL,
    BT2020_LIMITED,
    BT2020_FULL,
    COLOR_SPACE_NUM,
};

enum VEU_TRANSFORM {
    VEU_TRANSFORM_NONE = 0x0,
    VEU_TRANSFORM_FLIP_H= 0x1,
    VEU_TRANSFORM_FLIP_V = 0x2,
    VEU_TRANSFORM_ROT_90 = 0x4,
    VEU_TRANSFORM_ROT_180 = VEU_TRANSFORM_FLIP_H | VEU_TRANSFORM_FLIP_V,
    VEU_TRANSFORM_ROT_270 = VEU_TRANSFORM_ROT_180 | VEU_TRANSFORM_ROT_90,
    VEU_TRANSFORM_H_90 = VEU_TRANSFORM_FLIP_H | VEU_TRANSFORM_ROT_90,
    VEU_TRANSFORM_V_90 = VEU_TRANSFORM_FLIP_V | VEU_TRANSFORM_ROT_90,
};

struct veu_rect_ltrb_float {
    float left;
    float top;
    float right;
    float bottom;
};

struct veu_rect_float {
    float x;
    float y;
    float w;
    float h;
};

struct veu_rect_ltrb {
    int left;
    int top;
    int right;
    int bottom;
};

struct veu_rect {
    int x;
    int y;
    int w;
    int h;
};

enum TILE_MODE {
    SCAN_MODE = 0,
    TILE_MODE,
};

/**
 * srcrect: layer input size after crop
 * dstrect: layer output size
 * sharefd: fd for layerbuffer
 * planeoffset1: u/v plane offset for yuv raw format
 * planeoffset2: u/v plane offset for yuv raw format
 * stride0: stride for rgb raw format & y plane
 * stride1: stride for u/v plane
 * afbcused: layer format is afbc or not
 * afbclayout: afbc superblock size
 * afbcTileMode: afbc tile header mode
 * afbcYuvTransform: afbc yuv transform enable
 * usage: usage for layerbuffer allocation
 * imgwidth: original img width
 * imgheight: original img width
 * format: layer format
 * colorspace: layer colorspace, bt709 default
 * transform: rot&flip mode
 * alpha: unused right now
 * bufsize: bufsize for layerbuffer
 */
struct veu_layer {
    struct veu_rect_ltrb src_rect;
    struct veu_rect_ltrb dst_rect;

    int sharefd;
    uint64_t plane_offset1;
    uint64_t plane_offset2;
    uint32_t stride0;
    uint32_t stride1;
    uint32_t afbc_used;
    uint32_t afbc_layout;
    enum TILE_MODE afbc_tile_mode;
    uint32_t afbc_yuvtransform;

    uint32_t img_width;
    uint32_t img_height;

    uint32_t format;
    uint32_t color_space;
    uint32_t transform;

    uint64_t buf_size;

    bool vpu_crop;
    bool dither_enable;
    bool is_pattern_dither;
};


#endif /* _XRING_VEU_H_ */
