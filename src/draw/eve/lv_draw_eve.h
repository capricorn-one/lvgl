/*
 * lv_draw_eve.h
 *
 *  Created on: 3 dic 2023
 *      Author: juanj
 */

#ifndef LV_DRAW_EVE_H_
#define LV_DRAW_EVE_H_


#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw.h"
#if LV_USE_DRAW_EVE

#include "../../misc/lv_area.h"
#include "../../misc/lv_color.h"

/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_draw_unit_t base_unit;
    lv_draw_task_t * task_act;
    void * eve;
} lv_draw_eve_unit_t;


/**********************
 * GLOBAL PROTOTYPES
 **********************/

extern void lv_draw_eve_init(void);

extern void lv_draw_eve_layer(lv_draw_eve_unit_t * draw_unit, const lv_draw_image_dsc_t * draw_dsc,
                              const lv_area_t * coords);

extern void lv_draw_eve_image(lv_draw_eve_unit_t * draw_unit, const lv_draw_image_dsc_t * draw_dsc,
                              const lv_area_t * coords);

extern void lv_draw_eve_fill(lv_draw_eve_unit_t * draw_unit, const lv_draw_fill_dsc_t * dsc, const lv_area_t * coords);

extern void lv_draw_eve_border(lv_draw_eve_unit_t * draw_unit, const lv_draw_border_dsc_t * dsc,
                               const lv_area_t * coords);

extern void lv_draw_eve_line(lv_draw_eve_unit_t * draw_unit, const lv_draw_line_dsc_t * dsc);

extern void lv_draw_eve_label(lv_draw_eve_unit_t * draw_unit, const lv_draw_label_dsc_t * dsc,
                              const lv_area_t * coords);

extern void lv_draw_eve_arc(lv_draw_eve_unit_t * draw_unit, const lv_draw_arc_dsc_t * dsc, const lv_area_t * coords);

extern void lv_draw_eve_triangle(lv_draw_eve_unit_t * draw_unit, const lv_draw_triangle_dsc_t * dsc);

/**********************
 *      MACROS
 **********************/

#define DEGREES(n) ((65536UL * (n)) / 3600)
#define F16(x)     ((int32_t)((x) * 65536L))


#endif /*LV_USE_DRAW_EVE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif


#endif /* LV_DRAW_EVE_H_ */
