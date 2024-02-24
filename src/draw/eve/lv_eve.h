/*
 * lv_eve.h
 *
 *  Created on: 8 jun 2023
 *      Author: juanj
 */

#ifndef LV_EVE_H_
#define LV_EVE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "lv_draw_eve.h"

#if LV_USE_DRAW_EVE
#include <stdint.h>
#include "EVE.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    ZERO_VALUE,
    BITMAPS = 1UL,      // Bitmap drawing primitive
    POINTS = 2UL,       // Point drawing primitive
    LINES = 3UL,        // Line drawing primitive
    LINE_STRIP = 4UL,   // Line strip drawing primitive
    EDGE_STRIP_R = 5UL, // Edge strip right side drawing primitive
    EDGE_STRIP_L = 6UL, // Edge strip left side drawing primitive
    EDGE_STRIP_A = 7UL, // Edge strip above drawing primitive
    EDGE_STRIP_B = 8UL, // Edge strip below side drawing primitive
    RECTS = 9UL,        // Rectangle drawing primitive
} PrimitiveDraw;


typedef struct {

    PrimitiveDraw primitive;
    lv_color_t color;
    lv_opa_t opa;
    int32_t lineWidth;
    uint16_t pointSize;
    uint8_t colorMask[4];
    uint8_t stencilFunc[3];
    uint8_t stencilOp[2];
    uint8_t blendFunc[2];
    uint16_t scx;
    uint16_t scy;
    uint16_t scw;
    uint16_t sch;

} DrawingContext;

/**********************
 *  GLOBAL PROTOTYPES
 **********************/

extern void eve_save_context(void);
extern void eve_restore_context(void);
extern void eve_scissor(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
extern void eve_primitive(uint8_t context);
extern void eve_color(lv_color_t color);
extern void eve_color_opa(lv_opa_t opa);
extern void eve_line_width(int32_t width);
extern void eve_point_size(uint16_t radius);
extern void eve_vertex_2f(int16_t x, int16_t y);
extern void eve_color_mask(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
extern void eve_stencil_func(uint8_t func, uint8_t ref, uint8_t mask);
extern void eve_stencil_op(uint8_t sfail, uint8_t spass);
extern void eve_blend_func(uint8_t sfail, uint8_t spass);

extern void eve_draw_circle_simple(int16_t coord_x1, int16_t coord_y1, uint16_t radius_t);
extern void eve_draw_rect_simple(int16_t coord_x1, int16_t coord_y1, int16_t coord_x2, int16_t coord_y2,
                                 uint16_t radius);
extern void eve_mask_round(int16_t coord_x1, int16_t coord_y1, int16_t coord_x2, int16_t coord_y2, int16_t radius);

/**********************
 *  EXTERN VARIABLES
 **********************/


/**********************
 *      MACROS
 **********************/

/**********************
 *   INLINE FUNCTIONS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_USE_DRAW_EVE*/


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_EVE_H_ */
