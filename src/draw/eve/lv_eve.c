/*
 * lv_eve.c
 *
 *  Created on: 8 jun 2023
 *      Author: juanj
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_eve.h"
#if LV_USE_DRAW_EVE
#include "lv_eve.h"
#include "EVE_commands.h"


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static uint16_t scissor_x = 0;
static uint16_t scissor_y = 0;
static uint16_t scissor_w = 0;
static uint16_t scissor_h = 0;

static DrawingContext ct = {
    .primitive = ZERO_VALUE,
    .color = {0xff, 0xff, 0xff},
    .opa = 255,
    .lineWidth = 1,  //for format(0)
    .pointSize = 1,
    .colorMask = {1, 1, 1, 1},
    .stencilFunc = {EVE_ALWAYS, 0, 255},
    .stencilOp = {EVE_KEEP, EVE_KEEP},
    .blendFunc = {EVE_SRC_ALPHA, EVE_ONE_MINUS_SRC_ALPHA},
    .scx = 0,
    .scy = 0,
    .scw = EVE_HSIZE,
    .sch = EVE_VSIZE
};

static DrawingContext ct_temp;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void eve_save_context(void)
{
    EVE_cmd_dl_burst(DL_SAVE_CONTEXT);
    ct_temp = ct;
}

void eve_restore_context(void)
{
    EVE_cmd_dl_burst(DL_RESTORE_CONTEXT);
    ct = ct_temp;
}


void eve_primitive(uint8_t context)
{
    if(context != ct.primitive && context != ZERO_VALUE) {
        EVE_cmd_dl_burst(DL_BEGIN | context);
        ct.primitive = context;
    }
}


void eve_scissor(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if(x != scissor_x || y != scissor_y || w != scissor_w || h != scissor_h) {
        EVE_cmd_dl_burst(SCISSOR_XY(x, y));
        EVE_cmd_dl_burst(SCISSOR_SIZE(w, h));
        scissor_w = w;
        scissor_h = h;
        scissor_x = x;
        scissor_y = y;
    }

}


void eve_scissor_size(uint16_t w, uint16_t h)
{
    if(w != scissor_w || h != scissor_h) {
        EVE_cmd_dl_burst(SCISSOR_SIZE(w, h));
        scissor_w = w;
        scissor_h = h;
    }
}

void eve_color(lv_color_t color)
{
    if((ct.color.red != color.red) || (ct.color.green != color.green) || (ct.color.blue != color.blue)) {
        EVE_cmd_dl_burst(COLOR_RGB(color.red, color.green, color.blue));
        ct.color = color;
    }
}

void eve_color_mask(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if((ct.colorMask[0] != r) ||
       (ct.colorMask[1] != g) ||
       (ct.colorMask[2] != b) ||
       (ct.colorMask[3] != a)) {

        EVE_cmd_dl_burst(COLOR_MASK(r, g, b, a));
        ct.colorMask[0] = r;
        ct.colorMask[1] = g;
        ct.colorMask[2] = b;
        ct.colorMask[3] = a;
    }
}

void eve_stencil_func(uint8_t func, uint8_t ref, uint8_t mask)
{
    if(func != ct.stencilFunc[0] || ref != ct.stencilFunc[1] || mask != ct.stencilFunc[2]) {

        EVE_cmd_dl_burst(STENCIL_FUNC(func, ref, mask));
        ct.stencilFunc[0] = func;
        ct.stencilFunc[1] = ref;
        ct.stencilFunc[2] = mask;
    }
}

void eve_stencil_op(uint8_t sfail, uint8_t spass)
{
    if(sfail != ct.stencilOp[0] || spass != ct.stencilOp[1]) {
        EVE_cmd_dl_burst(STENCIL_OP(sfail, spass));
        ct.stencilOp[0] = sfail;
        ct.stencilOp[1] = spass;

    }
}

void eve_blend_func(uint8_t src, uint8_t dst)
{
    if(src != ct.blendFunc[0] || dst != ct.blendFunc[1]) {
        EVE_cmd_dl_burst(BLEND_FUNC(src, dst));
        ct.blendFunc[0] = src;
        ct.blendFunc[1] = dst;
    }
}

void eve_color_opa(lv_opa_t opa)
{
    if(opa != ct.opa) {
        EVE_cmd_dl_burst(COLOR_A(opa));
        ct.opa = opa;
    }
}

void eve_line_width(int32_t width)
{
    if(width != ct.lineWidth) {
        EVE_cmd_dl_burst(LINE_WIDTH(width));
        ct.lineWidth = width;
    }
}

void eve_point_size(uint16_t radius)
{
    if(radius != ct.pointSize) {
        EVE_cmd_dl_burst(POINT_SIZE(radius * 16));
        ct.pointSize = radius;
    }
}

void eve_vertex_2f(int16_t x, int16_t y)
{
    EVE_cmd_dl_burst(VERTEX2F(x, y));
}

void eve_draw_circle_simple(int16_t coord_x1, int16_t coord_y1, uint16_t radius_t)
{
    eve_primitive(POINTS);
    eve_point_size(radius_t);
    eve_vertex_2f(coord_x1, coord_y1);
}


void eve_draw_rect_simple(int16_t coord_x1, int16_t coord_y1, int16_t coord_x2, int16_t coord_y2, uint16_t radius)
{
    int16_t adj_pixel = 0;
    eve_primitive(RECTS);
    if(radius > 1) {
        eve_line_width(radius * 16);
        adj_pixel = 1;
    }
    else {
        radius = 0;
        eve_line_width(16);
    }

    eve_vertex_2f(coord_x1 + radius - adj_pixel, coord_y1 + radius - adj_pixel);
    eve_vertex_2f(coord_x2 - radius + adj_pixel, coord_y2 - radius + adj_pixel);
}

void eve_mask_round(int16_t coord_x1, int16_t coord_y1, int16_t coord_x2, int16_t coord_y2, int16_t radius)
{
    eve_color_mask(0, 0, 0, 1);
    EVE_cmd_dl_burst(CLEAR(1, 1, 1));


    eve_draw_rect_simple(coord_x1, coord_y1, coord_x2, coord_y2, radius);
    eve_color_mask(1, 1, 1, 0);
    eve_blend_func(EVE_DST_ALPHA, EVE_ONE_MINUS_DST_ALPHA);
}



/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_USE_DRAW_EVE*/
