/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_eve.h"
#if LV_USE_DRAW_EVE
#include "lv_eve.h"
#include "eve_ram_g.h"
#include "src/font/lv_font_fmt_txt.h"
#include "src/misc/lv_utils.h"
#include "stdlib.h"

/*********************
 *      DEFINES
 *********************/

#if LV_FONT_SIMSUN_16_CJK
    #define MAX_GLYPH_DSC  1414
    #define _MAX_FONT MAX_FONT / 2 /* Reduce the number of allowed fonts to 4 to save memory */
#elif LV_FONT_DEJAVU_16_PERSIAN_HEBREW
    #define MAX_GLYPH_DSC  598
    #define _MAX_FONT MAX_FONT / 2 /* Reduce the number of allowed fonts to 4 to save memory */

#else
    #define MAX_GLYPH_DSC  160
    #define _MAX_FONT MAX_FONT /* Case for montserrat font */
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void lv_draw_eve_letter_cb(lv_draw_unit_t * draw_unit, lv_draw_glyph_dsc_t * glyph_draw_dsc,
                                  lv_draw_fill_dsc_t * fill_draw_dsc, const lv_area_t * fill_area);
static void bitmap_to_evenWidth(uint32_t addr, const uint8_t * img_in, uint8_t * img_out, uint16_t width,
                                uint16_t height);
static int32_t _unicode_list_compare(const void * ref, const void * element);
static uint32_t _get_glyph_dsc_id(const lv_font_t * font, uint32_t letter);
static uint32_t eve_lv_font_to_ramg(const lv_font_t * font_p, uint8_t font_eveId, uint32_t ad);

/**********************
 *  STATIC VARIABLES
 **********************/

static uint32_t addr_font[_MAX_FONT][MAX_GLYPH_DSC]; /* Save the current RAM_G font address */
static const lv_font_t * font_static;

static lv_draw_eve_unit_t * unit = NULL;

/**********************
 *      MACROS
 **********************/

#define get_nibble_1(w) ((uint8_t) ((w) >> 4))
#define get_nibble_2(w) ((uint8_t) ((w) & 0xf))


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_eve_label(lv_draw_eve_unit_t * draw_unit, const lv_draw_label_dsc_t * dsc, const lv_area_t * coords)
{

    if(dsc->opa <= LV_OPA_MIN)
        return;
    unit = draw_unit;
    
    eve_scissor(draw_unit->base_unit.clip_area->x1, draw_unit->base_unit.clip_area->y1, draw_unit->base_unit.clip_area->x2, draw_unit->base_unit.clip_area->y2);
    eve_save_context();
    eve_primitive(BITMAPS);

    uint16_t width = draw_unit->base_unit.clip_area->x2 - draw_unit->base_unit.clip_area->x1;
    uint16_t height = draw_unit->base_unit.clip_area->y2 - draw_unit->base_unit.clip_area->y1;

    EVE_cmd_dl_burst(BITMAP_SIZE_H(width, height));
    EVE_cmd_dl_burst(BITMAP_LAYOUT_H(width, height));
    
    //LV_LOG("%s\r\n ", dsc->text);
    //draw_unit->target_layer->user_data = (lv_font_t *)dsc->font;
    font_static = dsc->font;
    lv_draw_label_iterate_characters(&unit->base_unit, dsc, coords, lv_draw_eve_letter_cb);
    eve_restore_context();
    EVE_end_cmd_burst();
    EVE_execute_cmd();
    EVE_start_cmd_burst();

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static int32_t _unicode_list_compare(const void * ref, const void * element)
{
    return ((int32_t)(*(uint16_t *)ref)) - ((int32_t)(*(uint16_t *)element));
}

static uint32_t _get_glyph_dsc_id(const lv_font_t * font, uint32_t letter)
{
    if(letter == '\0') return 0;

    lv_font_fmt_txt_dsc_t * fdsc = (lv_font_fmt_txt_dsc_t *)font->dsc;

    uint16_t i;
    for(i = 0; i < fdsc->cmap_num; i++) {

        /*Relative code point*/
        uint32_t rcp = letter - fdsc->cmaps[i].range_start;
        if(rcp >= fdsc->cmaps[i].range_length) continue;
        uint32_t glyph_id = 0;
        if(fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY) {
            glyph_id = fdsc->cmaps[i].glyph_id_start + rcp;
        }
        else if(fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL) {
            const uint8_t * gid_ofs_8 = fdsc->cmaps[i].glyph_id_ofs_list;
            glyph_id = fdsc->cmaps[i].glyph_id_start + gid_ofs_8[rcp];
        }
        else if(fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_SPARSE_TINY) {
            uint16_t key = rcp;
            uint16_t * p = _lv_utils_bsearch(&key, fdsc->cmaps[i].unicode_list, fdsc->cmaps[i].list_length,
                                             sizeof(fdsc->cmaps[i].unicode_list[0]), _unicode_list_compare);

            if(p) {
                lv_uintptr_t ofs = p - fdsc->cmaps[i].unicode_list;
                glyph_id = fdsc->cmaps[i].glyph_id_start + ofs;
            }
        }
        else if(fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_SPARSE_FULL) {
            uint16_t key = rcp;
            uint16_t * p = _lv_utils_bsearch(&key, fdsc->cmaps[i].unicode_list, fdsc->cmaps[i].list_length,
                                             sizeof(fdsc->cmaps[i].unicode_list[0]), _unicode_list_compare);

            if(p) {
                lv_uintptr_t ofs = p - fdsc->cmaps[i].unicode_list;
                const uint16_t * gid_ofs_16 = fdsc->cmaps[i].glyph_id_ofs_list;
                glyph_id = fdsc->cmaps[i].glyph_id_start + gid_ofs_16[ofs];
            }
        }

        return glyph_id;
    }

    return 0;

}



static void lv_draw_eve_letter_cb(lv_draw_unit_t * draw_unit, lv_draw_glyph_dsc_t * glyph_draw_dsc,
                                  lv_draw_fill_dsc_t * fill_draw_dsc, const lv_area_t * fill_area)
{

    if(fill_draw_dsc && fill_area) {
        /* draw UNDERLINE and STRIKETHROUGH */
        eve_draw_rect_simple(fill_area->x1, fill_area->y1, fill_area->x2, fill_area->y2, 0);
    }

    if(glyph_draw_dsc == NULL)
        return;  //Important

    const lv_font_t * font_in_use = font_static; //(lv_font_t *)

    lv_font_fmt_txt_dsc_t * font_dsc = (lv_font_fmt_txt_dsc_t *) font_in_use->dsc;

    uint32_t letter = (int32_t)draw_unit->target_layer->user_data;
    if(letter > '~') {
        letter = _get_glyph_dsc_id(font_in_use, letter);
    }
    else {
        /* Use this method for fast ID accses */
        letter = (letter - font_dsc->cmaps->range_start) + font_dsc->cmaps->glyph_id_start;
    }


    uint16_t g_box_w = font_dsc->glyph_dsc[letter].box_w + 1; /* + 1 Normalize Width*/
    uint16_t g_box_h = font_dsc->glyph_dsc[letter].box_h;
    uint8_t bpp = 0;

    const uint8_t * font_src = (const uint8_t *)font_in_use; /* font location pointer */

    uint32_t font_eveId;
    font_eveId = find_ramg_font(font_src);

    if(font_eveId == NOT_FOUND_BLOCK) { /* If the font is not yet loaded in ramG, load it */
        uint32_t free_ramg_block = next_free_ramg_block(TYPE_FONT);
        uint32_t start_addr_ramg = get_ramg_ptr();

        uint32_t total_font_size = eve_lv_font_to_ramg(font_in_use, free_ramg_block,
                                                       start_addr_ramg); /* load font to ramG (The block Id is updated in this function) */

        update_ramg_block(free_ramg_block, (uint8_t *)font_src, start_addr_ramg, total_font_size);
        font_eveId = free_ramg_block;

    }


    eve_color_opa(glyph_draw_dsc->opa);
    eve_color(glyph_draw_dsc->color);

    EVE_cmd_dl_burst(BITMAP_SOURCE(addr_font[font_eveId][letter]));

    switch(font_dsc->bpp) {
        case 1 :
            bpp = EVE_L1;
            break;
        case 4 :
            bpp = EVE_L4;
            break;
        case 8 :
            bpp = EVE_L8;
            break;
        default :
            break;
    }

    EVE_cmd_dl_burst(BITMAP_SIZE(EVE_NEAREST, EVE_BORDER, EVE_BORDER, g_box_w, g_box_h));
    EVE_cmd_dl_burst(BITMAP_LAYOUT(bpp, g_box_w / 2, g_box_h));

    eve_vertex_2f(glyph_draw_dsc->letter_coords->x1, glyph_draw_dsc->letter_coords->y1);

    EVE_end_cmd_burst();
    EVE_execute_cmd();
    EVE_start_cmd_burst();

}


/**********************
 *   STATIC FUNCTIONS
 **********************/

static uint32_t eve_lv_font_to_ramg(const lv_font_t * font_p, uint8_t font_eveId, uint32_t ad)
{

    EVE_end_cmd_burst();
    const uint8_t * map_p;
    uint32_t addr = ad;
    lv_font_fmt_txt_dsc_t * font_dsc = (lv_font_fmt_txt_dsc_t *) font_p->dsc;

    //uint8_t temp_buff[(font_p->line_height * font_p->line_height * font_dsc->bpp) / 8];
    uint8_t * temp_buff = lv_malloc(font_p->line_height * font_p->line_height * font_dsc->bpp); /*Extra mem*/
    uint32_t cmap_max = font_dsc->cmap_num;
    uint32_t range_max = 0;
    uint32_t range_init = 0;
    uint32_t cmap;
    for(cmap = 0; cmap < cmap_max; ++cmap) {

        range_init = font_dsc->cmaps[cmap].glyph_id_start;
        range_max = font_dsc->cmaps[cmap].range_length + range_init;

        if(font_dsc->cmaps[cmap].unicode_list || font_dsc->cmaps[cmap].glyph_id_ofs_list) {
            range_max = font_dsc->cmaps[cmap].list_length + range_init;

        }

        uint32_t id;
        for(id = range_init; id < range_max; ++id) { /*Iterate through each character range*/

            uint32_t uni = font_dsc->glyph_dsc[id].bitmap_index;

            map_p = &font_dsc->glyph_bitmap[uni];

            uint8_t g_box_w = font_dsc->glyph_dsc[id].box_w;
            uint8_t g_box_h = font_dsc->glyph_dsc[id].box_h;

            if(font_dsc->glyph_dsc[id].box_w != 0 && g_box_h != 0) {

                addr_font[font_eveId][id] = addr; /*Store the address of the glyph in the address table*/

                uint32_t buffer_size;// = (g_box_w * g_box_h) / 2; /*Calculate buffer size based on glyph dimensions*/
                if(g_box_w % 2 != 0) {
                    bitmap_to_evenWidth(0, map_p, temp_buff, g_box_w, g_box_h); /*Adjust bitmap width to even width if necessary*/
                    buffer_size = ((g_box_w + 1) * g_box_h) / 2;

                    EVE_memWrite_sram_buffer(addr, temp_buff, buffer_size);
                }
                else {
                    buffer_size = (g_box_w * g_box_h) / 2; /*Calculate buffer size based on glyph dimensions*/
                    EVE_memWrite_flash_buffer(addr, map_p, buffer_size); /*Write glyph bitmap to EVE memory*/
                }

                addr += buffer_size;

            }
        }
    }
    lv_free(temp_buff);
    EVE_start_cmd_burst();  /*Start command burst*/

    return addr;

}

static void bitmap_to_evenWidth(uint32_t addr, const uint8_t * img_in, uint8_t * img_out, uint16_t width,
                                uint16_t height)
{
    uint32_t addr_it = addr;
    uint8_t nibble_1;
    uint8_t nibble_2;
    uint8_t key = 0;
    uint16_t j = 0;

    /* Iterate through each row of the bitmap*/
    for(int i = 0; i <= height; i++) {
        /* Iterate through each byte of the row*/
        for(int var = 0; var < (width / 2); ++var) {
            /*Get the two nibbles from the current byte*/
            if(key == 0) {
                nibble_1 = get_nibble_1(img_in[addr_it]);
                nibble_2 = get_nibble_2(img_in[addr_it]);
            }
            if(key == 1) {
                nibble_1 = get_nibble_2(img_in[addr_it - 1]);
                nibble_2 = get_nibble_1(img_in[addr_it]);
            }

            /*Combine the nibbles and assign the result to the output byte*/
            img_out[j] = (nibble_1 << 4) | nibble_2;

            addr_it++;
            j++;
        }

        /*If the width is odd, process the last remaining nibble*/
        if(width % 2 >= 1) {
            img_out[j] =
                (key == 0) ?
                (get_nibble_1(img_in[addr_it])) << 4 | 0x0 : (get_nibble_2(img_in[addr_it - 1])) << 4 | 0x0;
            key = (key == 0) ? 1 : 0;
            addr_it += (key == 1) ? 1 : 0;
            j++;
        }
    }
}


#endif /*LV_USE_DRAW_EVE*/

