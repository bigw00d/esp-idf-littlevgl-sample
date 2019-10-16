/**
 * @file lv_templ.h
 *
 */

#ifndef ILI9341_H
#define ILI9341_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define DISP_BUF_SIZE (LV_HOR_RES_MAX * 40)
// #define ILI9341_DC   21
// #define ILI9341_RST  18
// #define ILI9341_BCKL 5
#define ILI9341_DC   17
#define ILI9341_RST  16
#define ILI9341_BCKL 4 //dummy

// if text/images are backwards, try setting this to 1
// #define ILI9341_INVERT_DISPLAY 0 //-:daiki
#define ILI9341_INVERT_DISPLAY 1 //+:daiki

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void ili9341_init(void);
void ili9341_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*ILI9341_H*/
