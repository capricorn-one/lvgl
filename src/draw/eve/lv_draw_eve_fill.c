/*
 * lv_draw_eve_fill.c
 *
 *  Created on: 27 mar 2023
 *      Author: juanj
 */
/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_eve.h"
#if LV_USE_DRAW_EVE
#include "lv_eve.h"


/**********************
 * STATIC PROTOTYPES
 **********************/

static void eve_draw_shadow(lv_draw_eve_unit_t * draw_unit, const lv_draw_box_shadow_dsc_t * dsc,
                            const lv_area_t * coords);

static void eve_draw_circle_border(int32_t coord_x1, int32_t coord_y1, int32_t radius, int32_t border, uint32_t color,
                                   uint8_t col_A);
static void eve_draw_rect_border(lv_draw_eve_unit_t * draw_unit, const lv_area_t * outer_area,
                                 const lv_area_t * inner_area,
                                 int32_t rout, int32_t rin, lv_color_t color, lv_opa_t opa);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_eve_fill(lv_draw_eve_unit_t * draw_unit, const lv_draw_fill_dsc_t * dsc, const lv_area_t * coords)
{

    int32_t rad = dsc->radius;
    int32_t bg_w = lv_area_get_width(coords);
    int32_t bg_h = lv_area_get_height(coords);
    int32_t real_radius = LV_MIN3(bg_w / 2, bg_h / 2, rad);

    LV_LOG_TRACE("FILL********");
    LV_LOG_TRACE("x1: %d", draw_unit->base_unit.clip_area->x1);
    LV_LOG_TRACE("y1: %d", draw_unit->base_unit.clip_area->y1);
    LV_LOG_TRACE("X2: %d", draw_unit->base_unit.clip_area->x2);
    LV_LOG_TRACE("y2: %d", draw_unit->base_unit.clip_area->y2);
/*
    LV_LOG("cx1: %d\n", coords->x1);
    LV_LOG("cy1: %d\n", coords->y1);
    LV_LOG("cx2: %d\n", coords->x2);
    LV_LOG("cy2: %d\n", coords->y2);
*/
    eve_scissor(draw_unit->base_unit.clip_area->x1, draw_unit->base_unit.clip_area->y1, draw_unit->base_unit.clip_area->x2, draw_unit->base_unit.clip_area->y2);
    eve_save_context();

    eve_color(dsc->color);
    eve_color_opa(dsc->opa);

    if(bg_w == bg_h && rad == LV_RADIUS_CIRCLE) {
        eve_draw_circle_simple(coords->x1 + (bg_w / 2), coords->y1 + (bg_h / 2), real_radius);
    }
    else {
        eve_draw_rect_simple(coords->x1, coords->y1, coords->x2, coords->y2, real_radius);
    }

    eve_restore_context();

    EVE_end_cmd_burst();
    EVE_execute_cmd();
    EVE_start_cmd_burst();


}


/**********************
 *   STATIC FUNCTIONS
 **********************/



void lv_draw_eve_border(lv_draw_eve_unit_t * draw_unit, const lv_draw_border_dsc_t * dsc, const lv_area_t * coords)
{

    if(dsc->opa <= LV_OPA_MIN) return;
    if(dsc->width == 0) return;
    if(dsc->side == LV_BORDER_SIDE_NONE) return;
    /*
LV_LOG("BORDE********\n");
    LV_LOG("x1: %d\n", draw_unit->base_unit.clip_area->x1);
    LV_LOG("y1: %d\n", draw_unit->base_unit.clip_area->y1);
    LV_LOG("X2: %d\n", draw_unit->base_unit.clip_area->x2);
    LV_LOG("y2: %d\n", draw_unit->base_unit.clip_area->y2);

    LV_LOG("cx1: %d\n", coords->x1);
    LV_LOG("cy1: %d\n", coords->y1);
    LV_LOG("cx2: %d\n", coords->x2);
    LV_LOG("cy2: %d\n", coords->y2);
    LV_LOG("width: %d\n", dsc->width);

    LV_LOG("\n");*/
    int32_t coords_w = lv_area_get_width(coords);
    int32_t coords_h = lv_area_get_height(coords);
    int32_t rout = dsc->radius;
    int32_t short_side = LV_MIN(coords_w, coords_h);
    if(rout > short_side >> 1) rout = short_side >> 1;

    /*Get the inner area*/
    lv_area_t area_inner;
    lv_area_copy(&area_inner, coords);
    area_inner.x1 += ((dsc->side & LV_BORDER_SIDE_LEFT) ? dsc->width : - (dsc->width ));
    area_inner.x2 -= ((dsc->side & LV_BORDER_SIDE_RIGHT) ? dsc->width : - (dsc->width ));
    area_inner.y1 += ((dsc->side & LV_BORDER_SIDE_TOP) ? dsc->width : - (dsc->width ));
    area_inner.y2 -= ((dsc->side & LV_BORDER_SIDE_BOTTOM) ? dsc->width : - (dsc->width ));

    int32_t rin = rout - dsc->width;
    if(rin < 0) rin = 0;

    eve_scissor(coords->x1, coords->y1, coords->x2, coords->y2);

    eve_save_context();

    eve_color(dsc->color);
    eve_color_opa(dsc->opa);

    eve_color_mask(0, 0, 0, 1);
    eve_stencil_func(EVE_ALWAYS, 0, 1);
    eve_stencil_op(EVE_REPLACE, EVE_REPLACE);
    eve_draw_rect_simple(coords->x1, coords->y1, coords->x2, coords->y2, 0);

    eve_blend_func(EVE_ONE, EVE_ZERO);
    eve_draw_rect_simple(area_inner.x1 - 2, area_inner.y1 - 1, area_inner.x2 + 1, area_inner.y2 + 2, rin);

    eve_stencil_func(EVE_ALWAYS, 1, 1);
    eve_stencil_op(EVE_REPLACE, EVE_REPLACE);
    eve_blend_func(EVE_ZERO, EVE_ONE_MINUS_SRC_ALPHA);
    eve_color_opa(255);
    eve_draw_rect_simple(area_inner.x1, area_inner.y1, area_inner.x2, area_inner.y2, rin);

    eve_color_mask(1, 1, 1, 1);

    if (dsc->side == LV_BORDER_SIDE_FULL)
    {
        eve_blend_func(EVE_DST_ALPHA, EVE_ONE_MINUS_DST_ALPHA);
        eve_draw_rect_simple(area_inner.x1, area_inner.y1, area_inner.x2, area_inner.y2, rin);
    }

    eve_stencil_func(EVE_NOTEQUAL, 1, 255);
    eve_blend_func(EVE_SRC_ALPHA, EVE_ONE_MINUS_SRC_ALPHA);

    eve_color_opa(dsc->opa);
    eve_draw_rect_simple(coords->x1, coords->y1, coords->x2, coords->y2, rout);

    eve_restore_context();
    
    EVE_end_cmd_burst();
    EVE_execute_cmd();
    EVE_start_cmd_burst();

}




static void eve_draw_shadow(lv_draw_eve_unit_t * draw_unit, const lv_draw_box_shadow_dsc_t * dsc,
                            const lv_area_t * coords)
{
#if 0
    /*Check whether the shadow is visible*/
    if(dsc->shadow_width == 0) return;
    if(dsc->shadow_opa <= LV_OPA_MIN) return;

    if(dsc->shadow_width == 1 && dsc->shadow_spread <= 0 &&
       dsc->shadow_offset_x == 0 && dsc->shadow_offset_y == 0) {
        return;
    }

    /*Calculate the rectangle which is blurred to get the shadow in `shadow_area`*/
    lv_area_t core_area;
    core_area.x1 = coords->x1  + dsc->shadow_offset_x - dsc->shadow_spread;
    core_area.x2 = coords->x2  + dsc->shadow_offset_x + dsc->shadow_spread;
    core_area.y1 = coords->y1  + dsc->shadow_offset_y - dsc->shadow_spread;
    core_area.y2 = coords->y2  + dsc->shadow_offset_y + dsc->shadow_spread;

    /*Calculate the bounding box of the shadow*/
    lv_area_t shadow_area;
    shadow_area.x1 = core_area.x1 - dsc->shadow_width / 2 - 1;
    shadow_area.x2 = core_area.x2 + dsc->shadow_width / 2 + 1;
    shadow_area.y1 = core_area.y1 - dsc->shadow_width / 2 - 1;
    shadow_area.y2 = core_area.y2 + dsc->shadow_width / 2 + 1;

    lv_opa_t opa = dsc->shadow_opa;
    if(opa > LV_OPA_MAX) opa = LV_OPA_COVER;

    /*Get clipped draw area which is the real draw area.
     *It is always the same or inside `shadow_area`*/
    lv_area_t draw_area;
    if(!_lv_area_intersect(&draw_area, &shadow_area, draw_unit->clip_area)) return;

    /*Consider 1 px smaller bg to be sure the edge will be covered by the shadow*/
    lv_area_t bg_area;
    lv_area_copy(&bg_area, coords);
    lv_area_increase(&bg_area, -1, -1);

    /*Get the clamped radius*/
    int32_t r_bg = dsc->radius;
    int32_t short_side = LV_MIN(lv_area_get_width(&bg_area), lv_area_get_height(&bg_area));
    if(r_bg > short_side >> 1) r_bg = short_side >> 1;

    /*Get the clamped radius*/
    int32_t r_sh = dsc->radius;
    short_side = LV_MIN(lv_area_get_width(&core_area), lv_area_get_height(&core_area));
    if(r_sh > short_side >> 1) r_sh = short_side >> 1;


    /*Get how many pixels are affected by the blur on the corners*/
    int32_t corner_size = dsc->shadow_width  + r_sh;

    eve_color(dsc->shadow_color);
    uint32_t opa_steps = 255 / (dsc->shadow_width * 2);

    for(int steps = 1; steps <= dsc->shadow_width; steps += 3) {

        //EVE_cmd_dl_burst(COLOR_A(opa_steps));
        eve_color_opa(opa_steps);
        Eve_draw_rect_simple(shadow_area.x1 + steps, shadow_area.y1 + steps, shadow_area.x2 - steps, shadow_area.y2 - steps,
                             r_sh + dsc->shadow_width / 2);

    }


#endif

}

static void eve_draw_circle_border(int32_t coord_x1, int32_t coord_y1, int32_t radius, int32_t border, uint32_t color,
                                   uint8_t col_A)
{

    int16_t innerRadius = radius - border;

    // Use local rendering context, bypass ESD display list functions.
    eve_primitive(POINTS);
    EVE_cmd_dl_burst(DL_COLOR_RGB | color);
    EVE_cmd_dl_burst(COLOR_A(col_A));


    // Outer reset
    eve_color_mask(0, 0, 0, 1);
    EVE_cmd_dl_burst(STENCIL_FUNC(EVE_ALWAYS, 0, 1));
    EVE_cmd_dl_burst(STENCIL_OP(EVE_REPLACE, EVE_REPLACE));
    eve_draw_circle_simple(coord_x1, coord_y1, radius);

    // Inner alpha quantity
    EVE_cmd_dl_burst(BLEND_FUNC(EVE_ONE, EVE_ZERO));
    eve_draw_circle_simple(coord_x1, coord_y1, innerRadius + 2); //verificar ? 2 pixel

    // Inner alpha edge mask
    EVE_cmd_dl_burst(STENCIL_FUNC(EVE_ALWAYS, 1, 1));
    EVE_cmd_dl_burst(STENCIL_OP(EVE_REPLACE, EVE_REPLACE));
    EVE_cmd_dl_burst(BLEND_FUNC(EVE_ZERO, EVE_ONE_MINUS_SRC_ALPHA));
    EVE_cmd_dl_burst(COLOR_A(255));
    eve_draw_circle_simple(coord_x1, coord_y1, innerRadius);

    // Inner color, outer rect stencil mask
    eve_color_mask(1, 1, 1, 1);
    EVE_cmd_dl_burst(BLEND_FUNC(EVE_DST_ALPHA, EVE_ONE_MINUS_DST_ALPHA));
    eve_draw_circle_simple(coord_x1, coord_y1, innerRadius);

    // Outer rect
    EVE_cmd_dl_burst(STENCIL_FUNC(EVE_NOTEQUAL, 1, 255));
    EVE_cmd_dl_burst(BLEND_FUNC(EVE_SRC_ALPHA, EVE_ONE_MINUS_SRC_ALPHA));
    EVE_cmd_dl_burst(COLOR_A(col_A));
    eve_draw_circle_simple(coord_x1, coord_y1, radius);

    // xxx EVE_cmd_dl_burst(DL_RESTORE_CONTEXT);


}

#endif /*LV_USE_DRAW_EVE*/

