/* SPI Master example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "lvgl/lvgl.h"
#include "lv_examples/lv_apps/demo/demo.h"
#include "lv_examples/lv_apps/sysmon/sysmon.h" //+:daiki
#include "lv_examples/lv_apps/terminal/terminal.h" //+:daiki
#include "lv_examples/lv_tests/lv_test_objx/lv_test_bar/lv_test_bar.h" //+:daiki
#include "lv_examples/lv_tests/lv_test_theme/lv_test_theme_1.h" //+:daiki
#include "lvgl/src/lv_themes/lv_theme.h" //+:daiki
#include "lv_examples/lv_tutorial/4_themes/lv_tutorial_themes.h" //+:daiki
#include "lv_examples/lv_tests/lv_test_theme/lv_test_theme_2.h" //+:daiki

#include "esp_freertos_hooks.h"

#include "drv/disp_spi.h"
#include "drv/ili9341.h"
#include "drv/tp_spi.h"
#include "drv/xpt2046.h"

#include "lv_font.h"

static void IRAM_ATTR lv_tick_task(void);

static void tab_switcher(lv_task_t * task);

void my_create() //+:daiki
{
	/*Create a style for the line meter*/
	static lv_style_t style_lmeter;
	lv_style_copy(&style_lmeter, &lv_style_pretty_color);
	style_lmeter.line.width = 2;
	style_lmeter.line.color = LV_COLOR_SILVER;
	style_lmeter.body.main_color = lv_color_hex(0x91bfed);         /*Light blue*/
	style_lmeter.body.grad_color = lv_color_hex(0x04386c);         /*Dark blue*/
	style_lmeter.body.padding.left = 16;                           /*Line length*/

	/*Create a line meter */
	lv_obj_t * lmeter;
	lmeter = lv_lmeter_create(lv_scr_act(), NULL);
	lv_lmeter_set_range(lmeter, 0, 100);                   /*Set the range*/
	lv_lmeter_set_value(lmeter, 80);                       /*Set the current value*/
	lv_lmeter_set_scale(lmeter, 240, 31);                  /*Set the angle and number of lines*/
	lv_lmeter_set_style(lmeter, LV_LMETER_STYLE_MAIN, &style_lmeter);           /*Apply the new style*/
	lv_obj_set_size(lmeter, 150, 150);
	lv_obj_align(lmeter, NULL, LV_ALIGN_CENTER, 0, 0);

	// vTaskDelay(5000 / portTICK_PERIOD_MS); // 1000ms
	// lv_lmeter_set_value(lmeter, 20);                       /*Set the current value*/
}

void lv_test_lmeter_1(void)
{
    /* Create a default object*/
    lv_obj_t * lmeter1 = lv_lmeter_create(lv_disp_get_scr_act(NULL), NULL);
    lv_obj_set_pos(lmeter1, 10, 10);
    lv_lmeter_set_value(lmeter1, 25);

    /*Copy the previous line meter and set smaller size for it*/
    lv_obj_t * lmeter2 = lv_lmeter_create(lv_disp_get_scr_act(NULL), lmeter1);
		lv_lmeter_set_value(lmeter2, 50);
    lv_obj_set_size(lmeter2, LV_DPI / 2,  LV_DPI / 2);
    lv_obj_align(lmeter2, lmeter1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    /*Create a styled line meter*/
    static lv_style_t style3;
    lv_style_copy(&style3, &lv_style_pretty);
    style3.body.main_color = LV_COLOR_GREEN;
    style3.body.grad_color = LV_COLOR_RED;
    style3.body.padding.left = 4;
    style3.body.border.color = LV_COLOR_GRAY;      /*Means the needle middle*/
    style3.line.width = 2;
    style3.line.color = LV_COLOR_SILVER;

    lv_obj_t * lmeter3 = lv_lmeter_create(lv_disp_get_scr_act(NULL), lmeter1);
    lv_obj_align(lmeter3, lmeter1, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    lv_obj_set_style(lmeter3, &style3);
    lv_lmeter_set_scale(lmeter3, 270, 41);
    lv_lmeter_set_range(lmeter3, -100, 100);
    lv_lmeter_set_value(lmeter3, 75);

    /*Copy the modified 'lmeter3' and set a smaller size for it*/
    lv_obj_t * lmeter4 = lv_lmeter_create(lv_disp_get_scr_act(NULL), lmeter3);
    lv_obj_set_size(lmeter4, 60, 60);
		lv_lmeter_set_value(lmeter4, 100);
    lv_obj_align(lmeter4, lmeter3, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

}

lv_obj_t * lmeter;
lv_obj_t * label_for_lmeter;

void demoLmeter(void)
{
    static lv_style_t style;
    lv_style_copy(&style, &lv_style_pretty);
    style.body.main_color = LV_COLOR_GREEN;
    style.body.grad_color = LV_COLOR_RED;
    // style.body.padding.hor = 20;
    style.body.border.color= LV_COLOR_GRAY; /*Means the needle middle*/
    style.line.width = 4;
    style.line.color = LV_COLOR_SILVER;

    lmeter=lv_lmeter_create(lv_scr_act(), NULL);
    lv_obj_set_style(lmeter, &style);
    lv_obj_set_size(lmeter,120,120);
    // lv_obj_set_pos(lmeter,10+250+20,100);
		lv_obj_set_pos(lmeter,10,50);

    lv_lmeter_set_range(lmeter,0,100);
    lv_lmeter_set_value(lmeter,25);

    static lv_style_t style1;
    lv_style_copy(&style1, &lv_style_plain);
    // style1.text.font = &lv_font_dejavu_40;
		style1.text.font = &lv_font_roboto_28;

    label_for_lmeter =  lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label_for_lmeter, "25");
    lv_label_set_align(label_for_lmeter,LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label_for_lmeter, lmeter, LV_ALIGN_CENTER, 0, 0);
    // lv_label_set_style(label_for_lmeter, &style1);
		lv_label_set_style(label_for_lmeter, LV_CONT_STYLE_MAIN,  &style1);

    /*Create Label*/
    lv_obj_t * label1 =  lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label1, "Lmeter");
    lv_obj_align(label1, lmeter, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
}

lv_obj_t * gauge;
void demoGauge(void)
{
    static lv_color_t colorArray[2] ;
    colorArray[0]=LV_COLOR_RED;
    colorArray[1]=LV_COLOR_GREEN;


    gauge=lv_gauge_create(lv_scr_act(), NULL);

    lv_obj_set_size(gauge,120,120);
    lv_obj_set_pos(gauge,10,50);

    lv_gauge_set_range(gauge,10,10+5*20);
    lv_gauge_set_critical_value(gauge,90);

    lv_gauge_set_needle_count(gauge,2,colorArray);
    lv_gauge_set_value(gauge,0,10);
    lv_gauge_set_value(gauge,1,20);

     /*Create a Label*/
    lv_obj_t * label1 =  lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label1, "Gauge");
    lv_obj_align(label1, gauge, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
}

void my_test_bar_1(void)
{
    /* Create a default object*/
    lv_obj_t * bar1 = lv_bar_create(lv_disp_get_scr_act(NULL), NULL);
    lv_obj_set_pos(bar1, 10, 10);
    lv_bar_set_value(bar1, 25, false); // bar, new_value, LV_ANIM_ON/OFF

    /* Modify size and position, range and set to 75 % */
    lv_obj_t * bar2 = lv_bar_create(lv_disp_get_scr_act(NULL), NULL);
    lv_obj_set_size(bar2, 100, 25);
    lv_obj_align(bar2, bar1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_bar_set_range(bar2, 0, 100);
    lv_bar_set_value(bar2, 50, false);

    /* Copy 'bar2' but set its size to be vertical (indicator at 75%)*/
    lv_obj_t * bar3 = lv_bar_create(lv_disp_get_scr_act(NULL), bar2);
    lv_obj_set_size(bar3, 25, 100);
    lv_obj_align(bar3, bar2, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
		lv_bar_set_value(bar3, 75, false); // bar, new_value, LV_ANIM_ON/OFF


    /* Copy 'bar2' and set new style for it
     * (like 'bar2' on its left but dark bg, thin red indicator with big light)*/
    static lv_style_t bar_bg;
    lv_style_copy(&bar_bg, &lv_style_pretty);
    bar_bg.body.main_color = LV_COLOR_BLACK;

    static lv_style_t bar_indic;
    lv_style_copy(&bar_indic, &lv_style_pretty);
    bar_indic.body.main_color = LV_COLOR_RED;
    bar_indic.body.grad_color = LV_COLOR_MAROON;
    bar_indic.body.shadow.color = LV_COLOR_RED;
    bar_indic.body.shadow.width = 20;
    bar_indic.body.padding.top = 10;       /*Set the padding around the indicator*/
    bar_indic.body.padding.bottom = 3;
    bar_indic.body.padding.left = 3;
    bar_indic.body.padding.right = 10;

    lv_obj_t * bar4 = lv_bar_create(lv_disp_get_scr_act(NULL), bar2);
    lv_obj_align(bar4, bar2, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    lv_bar_set_style(bar4, LV_BAR_STYLE_BG, &bar_bg);
    lv_bar_set_style(bar4, LV_BAR_STYLE_INDIC, &bar_indic);
		lv_bar_set_value(bar4, 100, false); // bar, new_value, LV_ANIM_ON/OFF

    /* Copy 'bar4' but set its size to be vertical*/
    lv_obj_t * bar5 = lv_bar_create(lv_disp_get_scr_act(NULL), bar4);
    lv_obj_set_size(bar5, 25, 100);
		lv_bar_set_value(bar5, 0, false); // bar, new_value, LV_ANIM_ON/OFF
    lv_obj_align(bar5, bar4, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
}

// refer : https://tr.pinterest.com/5eyyah/uiux-littlevgl/
void my_test_theme(void)
{
	//example 1
	// lv_theme_t * th = lv_theme_night_init(210, &lv_font_dejavu_40);
	// lv_theme_t * th = lv_theme_night_init(210, &lv_font_roboto_28);
	// lv_theme_t * th = lv_theme_night_init(210, &lv_font_roboto_16);
	// lv_theme_set_current(th);

	//example 2
	// /*Create a default slider*/
	// lv_obj_t *slider = lv_slider_create(lv_scr_act(), NULL);
	// lv_slider_set_value(slider, 70,  LV_ANIM_OFF);
	// lv_obj_set_pos(slider, 10, 10);
	//
	// /*Initialize the alien theme with a reddish hue*/
	// lv_theme_t *th = lv_theme_alien_init(10, NULL);
	//
	// /*Create a new slider and apply the themes styles*/
	// slider = lv_slider_create(lv_scr_act(), NULL);
	// lv_slider_set_value(slider, 70,  LV_ANIM_OFF);
	// lv_obj_set_pos(slider, 10, 50);

	// example2

	// lv_theme_t *theme = lv_theme_zen_init(10, NULL);
	lv_theme_t *theme = lv_theme_alien_init(10, NULL);
	lv_theme_set_current(theme);

	// base object
	lv_obj_t * obj1;
	obj1 = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_size(obj1, 320, 240);
	// lv_style_plain_color.text.color      = lv_color_make(0xf0, 0xf0, 0xf0);
	// lv_style_plain_color.image.color     = lv_color_make(0xf0, 0xf0, 0xf0);
	// lv_style_plain_color.line.color      = lv_color_make(0xf0, 0xf0, 0xf0);
	// lv_style_plain_color.body.main_color = lv_color_make(0x55, 0x96, 0xd8);
	// lv_style_plain_color.body.grad_color = lv_style_plain_color.body.main_color;
	lv_style_plain_color.body.main_color = LV_COLOR_BLACK;
	lv_style_plain_color.body.grad_color = LV_COLOR_BLACK;
	lv_obj_set_style(obj1, &lv_style_plain_color);
	lv_obj_align(obj1, NULL, LV_ALIGN_CENTER, 0, 0);

	/* Create the background for the whole project */
	lv_obj_t * background = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_size(background, 320, 240);
	lv_obj_set_pos(background, 0, 0);
}

// #define LV_COLOR_WHITE LV_COLOR_MAKE(0xFF, 0xFF, 0xFF)
// #define LV_COLOR_SILVER LV_COLOR_MAKE(0xC0, 0xC0, 0xC0)
// #define LV_COLOR_GRAY LV_COLOR_MAKE(0x80, 0x80, 0x80)
// #define LV_COLOR_BLACK LV_COLOR_MAKE(0x00, 0x00, 0x00)
// #define LV_COLOR_RED LV_COLOR_MAKE(0xFF, 0x00, 0x00)
// #define LV_COLOR_MAROON LV_COLOR_MAKE(0x80, 0x00, 0x00)
// #define LV_COLOR_YELLOW LV_COLOR_MAKE(0xFF, 0xFF, 0x00)
// #define LV_COLOR_OLIVE LV_COLOR_MAKE(0x80, 0x80, 0x00)
// #define LV_COLOR_LIME LV_COLOR_MAKE(0x00, 0xFF, 0x00)
// #define LV_COLOR_GREEN LV_COLOR_MAKE(0x00, 0x80, 0x00)
// #define LV_COLOR_CYAN LV_COLOR_MAKE(0x00, 0xFF, 0xFF)
// #define LV_COLOR_AQUA LV_COLOR_CYAN
// #define LV_COLOR_TEAL LV_COLOR_MAKE(0x00, 0x80, 0x80)
// #define LV_COLOR_BLUE LV_COLOR_MAKE(0x00, 0x00, 0xFF)
// #define LV_COLOR_NAVY LV_COLOR_MAKE(0x00, 0x00, 0x80)
// #define LV_COLOR_MAGENTA LV_COLOR_MAKE(0xFF, 0x00, 0xFF)
// #define LV_COLOR_PURPLE LV_COLOR_MAKE(0x80, 0x00, 0x80)
// #define LV_COLOR_ORANGE LV_COLOR_MAKE(0xFF, 0xA5, 0x00)

#define PADDING_RATE 20

void lv_ex_tabview_1(void)
{
		lv_theme_t *theme = lv_theme_alien_init(90, NULL);
		lv_theme_set_current(theme);

#if 0
		// see basic_init() at lv_theme_alien.c
		static lv_style_t style_tv_btn_rel; //button release style

    lv_style_copy(&style_tv_btn_rel, &lv_style_btn_rel);
    style_tv_btn_rel.text.font = &lv_font_roboto_16;
    /*The vertical padding of the released button is applied to all buttons*/
		style_tv_btn_rel.body.padding.left   = LV_DPI / PADDING_RATE;
    style_tv_btn_rel.body.padding.right  = LV_DPI / PADDING_RATE;
    style_tv_btn_rel.body.padding.top    = LV_DPI / PADDING_RATE;
    style_tv_btn_rel.body.padding.bottom = LV_DPI / PADDING_RATE;

		style_tv_btn_rel.body.main_color     = lv_color_hex3(0x222); //lv_color_make(0x76, 0xa2, 0xd0);
    style_tv_btn_rel.body.grad_color     = lv_color_hex3(0x222); //lv_color_make(0x19, 0x3a, 0x5d);
    style_tv_btn_rel.body.radius         = 0; //LV_DPI / 15;
    // style_tv_btn_rel.body.padding.left   = LV_DPI / 4;
    // style_tv_btn_rel.body.padding.right  = LV_DPI / 4;
    // style_tv_btn_rel.body.padding.top    = LV_DPI / 6;
    // style_tv_btn_rel.body.padding.bottom = LV_DPI / 20;
    // style_tv_btn_rel.body.padding.inner  = LV_DPI / 10;
    style_tv_btn_rel.body.border.color   = LV_COLOR_GRAY; //lv_color_make(0x0b, 0x19, 0x28);
    style_tv_btn_rel.body.border.width   = LV_DPI / 50 >= 1 ? LV_DPI / 50 : 1;
    style_tv_btn_rel.body.border.opa     = LV_OPA_10; //LV_OPA_70;
    style_tv_btn_rel.body.shadow.color   = LV_COLOR_GRAY;
    style_tv_btn_rel.body.shadow.width   = 0;
    style_tv_btn_rel.text.color          = lv_color_hex3(0xDDD); //lv_color_make(0xff, 0xff, 0xff);
    style_tv_btn_rel.image.color         = lv_color_hex3(0xDDD); //lv_color_make(0xff, 0xff, 0xff);

		/*Create a Tab view object*/
    lv_obj_t *tabview;
    tabview = lv_tabview_create(lv_scr_act(), NULL);
		lv_tabview_set_style(tabview, LV_TABVIEW_STYLE_BTN_REL, &style_tv_btn_rel);
#else
		/*Create a Tab view object*/
		lv_obj_t *tabview;
    tabview = lv_tabview_create(lv_scr_act(), NULL);

		static lv_style_t *pStyle_tv_btn_rel; //button release style
		pStyle_tv_btn_rel = lv_tabview_get_style(tabview, LV_TABVIEW_STYLE_BTN_REL);
		pStyle_tv_btn_rel->body.padding.left   = LV_DPI / PADDING_RATE;
    pStyle_tv_btn_rel->body.padding.right  = LV_DPI / PADDING_RATE;
    pStyle_tv_btn_rel->body.padding.top    = LV_DPI / PADDING_RATE;
    pStyle_tv_btn_rel->body.padding.bottom = LV_DPI / PADDING_RATE;

		static lv_style_t indic;
		lv_style_copy(&indic, &lv_style_plain);
    indic.body.main_color = LV_COLOR_ORANGE;
    indic.body.grad_color = LV_COLOR_ORANGE;
    indic.body.padding.inner = LV_DPI / 16;
		lv_tabview_set_style(tabview, LV_TABVIEW_STYLE_INDIC, &indic);

		// static lv_style_t *pStyle_tv_indi; //button release style
		// pStyle_tv_indi = lv_tabview_get_style(tabview, LV_TABVIEW_STYLE_INDIC);
		// // pStyle_tv_indi->body.padding.inner   = LV_DPI / 8;
		// pStyle_tv_indi->body.main_color = LV_COLOR_ORANGE;
    // pStyle_tv_indi->body.grad_color = LV_COLOR_ORANGE;
    // pStyle_tv_indi->body.padding.inner = LV_DPI / 16;
		// lv_tabview_set_style(tabview, LV_TABVIEW_STYLE_INDIC, pStyle_tv_indi);

#endif

		// lv_coord_t hres = lv_disp_get_hor_res(NULL);
    // lv_coord_t vres = lv_disp_get_ver_res(NULL);
		//
    // // Tabview 1
    // tabview = lv_tabview_create(lv_disp_get_scr_act(NULL), NULL);
    // lv_obj_set_size(tabview, hres / 2 - 10, vres / 2 - 10);
    // lv_obj_align(tabview, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    // lv_tabview_set_btns_pos(tabview, LV_TABVIEW_BTNS_POS_TOP);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Tab 3");

    /*Add content to the tabs*/
    lv_obj_t * label = lv_label_create(tab1, NULL);
    lv_label_set_text(label, "This the first tab\n\n"
                             "If the content\n"
                             "of a tab\n"
                             "become too long\n"
                             "the it \n"
                             "automatically\n"
                             "become\n"
                             "scrollable.");

    label = lv_label_create(tab2, NULL);
    lv_label_set_text(label, "Second tab");

    label = lv_label_create(tab3, NULL);
    lv_label_set_text(label, "Third tab");
}

void lv_test_tabview_1(void)
{
    lv_coord_t hres = lv_disp_get_hor_res(NULL);
    lv_coord_t vres = lv_disp_get_ver_res(NULL);

    /* Default object. It will be an empty tab view*/
    lv_obj_t * tv1 = lv_tabview_create(lv_disp_get_scr_act(NULL), NULL);
    lv_tabview_add_tab(tv1, "First");
    lv_tabview_add_tab(tv1, "Second");
    lv_obj_set_size(tv1, hres / 2 - 10, vres / 2 - 10);

    /*Copy the first tabview and add some texts*/
    lv_obj_t * tv2 = lv_tabview_create(lv_disp_get_scr_act(NULL), tv1);
    lv_obj_align(tv2, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

    lv_obj_t * tab = lv_tabview_get_tab(tv2, 0);
    lv_obj_t * label = lv_label_create(tab, NULL);
    lv_label_set_text(label, "This is\n\n\nA long text\n\n\ntext\n\n\non the\n\n\nsecond\n\n\ntab\n\n\nto see\n\n\nthe scrolling");

    tab = lv_tabview_get_tab(tv2, 1);
    label = lv_label_create(tab, NULL);
    lv_label_set_text(label, "This is the second tab");


    /*Create styles*/
    static lv_style_t bg;
    static lv_style_t sb;
    static lv_style_t btns_bg;
    static lv_style_t tab_bg;
    static lv_style_t rel;
    static lv_style_t pr;
    static lv_style_t tgl_rel;
    static lv_style_t tgl_pr;
    static lv_style_t indic;

    lv_style_copy(&btns_bg, &lv_style_plain_color);
    lv_style_copy(&tab_bg, &lv_style_pretty_color);
    lv_style_copy(&bg, &lv_style_pretty_color);
    lv_style_copy(&sb, &lv_style_pretty);
    lv_style_copy(&btns_bg, &lv_style_transp_fit);
    lv_style_copy(&rel, &lv_style_plain);
    lv_style_copy(&pr, &lv_style_plain);
    lv_style_copy(&tgl_rel, &lv_style_plain);
    lv_style_copy(&tgl_pr, &lv_style_plain);
    lv_style_copy(&indic, &lv_style_plain);

    rel.body.main_color = LV_COLOR_SILVER;
    pr.body.main_color = LV_COLOR_GRAY;
    tgl_rel.body.main_color = LV_COLOR_RED;
    tgl_pr.body.main_color = LV_COLOR_MAROON;
    indic.body.main_color = LV_COLOR_ORANGE;
    indic.body.grad_color = LV_COLOR_ORANGE;
    indic.body.padding.inner = LV_DPI / 16;
    tab_bg.body.main_color = LV_COLOR_SILVER;
    tab_bg.body.grad_color = LV_COLOR_GREEN;
    tab_bg.text.color = LV_COLOR_YELLOW;

    /*Apply the styles*/
    lv_obj_t * tv3 = lv_tabview_create(lv_disp_get_scr_act(NULL), tv2);
    lv_obj_align(tv3, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    lv_tabview_set_style(tv3, LV_TABVIEW_STYLE_BG, &bg);
    lv_tabview_set_style(tv3, LV_TABVIEW_STYLE_BTN_BG, &btns_bg);
    lv_tabview_set_style(tv3, LV_TABVIEW_STYLE_BTN_REL, &rel);
    lv_tabview_set_style(tv3, LV_TABVIEW_STYLE_BTN_PR, &pr);
    lv_tabview_set_style(tv3, LV_TABVIEW_STYLE_BTN_TGL_REL, &tgl_rel);
    lv_tabview_set_style(tv3, LV_TABVIEW_STYLE_BTN_TGL_PR, &tgl_pr);
    lv_tabview_set_style(tv3, LV_TABVIEW_STYLE_INDIC, &indic);

    tab = lv_tabview_get_tab(tv3, 0);
    lv_page_set_style(tab, LV_PAGE_STYLE_BG, &tab_bg);
    lv_page_set_style(tab, LV_PAGE_STYLE_SB, &sb);
    label = lv_label_create(tab, NULL);
    lv_label_set_text(label, "This is\n\n\nA long text\n\n\ntext\n\n\non the\n\n\nsecond\n\n\ntab\n\n\nto see\n\n\nthe scrolling");

    tab = lv_tabview_get_tab(tv3, 1);
    label = lv_label_create(tab, NULL);
    lv_label_set_text(label, "This is the second tab");

    /*Copy the styles tab view*/
    lv_obj_t * tv4 = lv_tabview_create(lv_disp_get_scr_act(NULL), tv3);
    lv_obj_align(tv4, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
}

// void alien_theme_test()
// {
//   lv_obj_t * test = lv_obj_create(lv_scr_act(), NULL);
// 	lv_obj_set_size(test , LV_HOR_RES, LV_VER_RES);
// 	lv_theme_t *theme = lv_theme_alien_init(100, nullptr);
//
// 	lv_obj_t * tv = lv_tabview_create(lv_scr_act(), NULL);
// 	lv_obj_t * tab1 = lv_tabview_add_tab(tv, "Write");
// 	lv_obj_t * tab2 = lv_tabview_add_tab(tv, "List");
// 	lv_obj_t * tab3 = lv_tabview_add_tab(tv, "Chart");
// 	lv_page_set_sb_mode(tab1, LV_SB_MODE_OFF);
// }

LV_IMG_DECLARE(img_bubble_pattern)

lv_obj_t *tabview;

void my_test_temperature(void)
{

		// // wallpaper
		// lv_coord_t hres = lv_disp_get_hor_res(NULL);
		// lv_coord_t vres = lv_disp_get_ver_res(NULL);
		//
		// lv_obj_t * wp = lv_img_create(lv_disp_get_scr_act(NULL), NULL);
		// lv_img_set_src(wp, &img_bubble_pattern);
		// lv_obj_set_width(wp, hres * 4);
		// lv_obj_set_protect(wp, LV_PROTECT_POS);

		// base object
		// lv_obj_t * obj1;
		// obj1 = lv_obj_create(lv_scr_act(), NULL);
		// lv_obj_set_size(obj1, 100, 50);
		// lv_obj_set_style(obj1, &lv_style_plain_color);
		// lv_obj_align(obj1, NULL, LV_ALIGN_CENTER, -60, -30);

		// bar

		lv_theme_t *theme = lv_theme_alien_init(90, NULL);
		lv_theme_set_current(theme);
		/*Create a Tab view object*/
		tabview = lv_tabview_create(lv_scr_act(), NULL);

		static lv_style_t *pStyle_tv_btn_rel; //button release style
		pStyle_tv_btn_rel = lv_tabview_get_style(tabview, LV_TABVIEW_STYLE_BTN_REL);
		pStyle_tv_btn_rel->body.padding.left   = LV_DPI / PADDING_RATE;
		pStyle_tv_btn_rel->body.padding.right  = LV_DPI / PADDING_RATE;
		pStyle_tv_btn_rel->body.padding.top    = LV_DPI / PADDING_RATE;
		pStyle_tv_btn_rel->body.padding.bottom = LV_DPI / PADDING_RATE;

		static lv_style_t indic;
		lv_style_copy(&indic, &lv_style_plain);
		indic.body.main_color = lv_color_hsv_to_rgb(90, 80, 87); //color for theme alien
		indic.body.grad_color = lv_color_hsv_to_rgb(90, 80, 87); //color for theme alien
		indic.body.padding.inner = LV_DPI / 16;
		lv_tabview_set_style(tabview, LV_TABVIEW_STYLE_INDIC, &indic);

		/*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Tab 3");

    /*Add content to the tabs*/
    // lv_obj_t * label = lv_label_create(tab1, NULL);
    // lv_label_set_text(label, "This the first tab\n\n"
    //                          "If the content\n"
    //                          "of a tab\n"
    //                          "become too long\n"
    //                          "the it \n"
    //                          "automatically\n"
    //                          "become\n"
    //                          "scrollable.");
		//
    // label = lv_label_create(tab2, NULL);
    // lv_label_set_text(label, "Second tab");
		//
    // label = lv_label_create(tab3, NULL);
    // lv_label_set_text(label, "Third tab");

		static lv_style_t bar_bg;
    lv_style_copy(&bar_bg, &lv_style_pretty);
    bar_bg.body.main_color = LV_COLOR_GRAY;

    static lv_style_t bar_indic;
    lv_style_copy(&bar_indic, &lv_style_pretty);
    bar_indic.body.main_color = LV_COLOR_RED;
    bar_indic.body.grad_color = LV_COLOR_MAROON;
    bar_indic.body.shadow.color = LV_COLOR_RED;

    lv_obj_t * bar3 = lv_bar_create(tab1, NULL);
    lv_obj_set_size(bar3, 25, 100);
		lv_obj_set_pos(bar3, 50, 50);
		lv_bar_set_value(bar3, 75, false); // bar, new_value, LV_ANIM_ON/OFF
		lv_bar_set_style(bar3, LV_BAR_STYLE_BG, &bar_bg);
		lv_bar_set_style(bar3, LV_BAR_STYLE_INDIC, &bar_indic);

		 /*Create a Label*/
    lv_obj_t * label3 =  lv_label_create(tab1, NULL);
    lv_label_set_text(label3, "20 C");
    lv_obj_align(label3, bar3, LV_ALIGN_OUT_BOTTOM_MID, 0, 20); //voffset:20

		// gauge

		static lv_color_t colorArray[1] ;
		colorArray[0]=LV_COLOR_BLACK;

    gauge=lv_gauge_create(tab1, NULL);

    lv_obj_set_size(gauge,120,120);
    lv_obj_set_pos(gauge,150,50);

    lv_gauge_set_range(gauge,0,100);
    lv_gauge_set_critical_value(gauge,90);

		lv_gauge_set_needle_count(gauge,1,colorArray);
    lv_gauge_set_value(gauge,0,88);

     /*Create a Label*/
    lv_obj_t * label1 =  lv_label_create(tab1, NULL);
    lv_label_set_text(label1, "88%");
    lv_obj_align(label1, gauge, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

		// static lv_style_t bar_bg;
    // lv_style_copy(&bar_bg, &lv_style_pretty);
    // bar_bg.body.main_color = LV_COLOR_GRAY;
		//
    // static lv_style_t bar_indic;
    // lv_style_copy(&bar_indic, &lv_style_pretty);
    // bar_indic.body.main_color = LV_COLOR_RED;
    // bar_indic.body.grad_color = LV_COLOR_MAROON;
    // bar_indic.body.shadow.color = LV_COLOR_RED;
		//
    // lv_obj_t * bar3 = lv_bar_create(lv_disp_get_scr_act(NULL), NULL);
    // lv_obj_set_size(bar3, 25, 100);
		// lv_obj_set_pos(bar3, 50, 50);
		// lv_bar_set_value(bar3, 75, false); // bar, new_value, LV_ANIM_ON/OFF
		// lv_bar_set_style(bar3, LV_BAR_STYLE_BG, &bar_bg);
		// lv_bar_set_style(bar3, LV_BAR_STYLE_INDIC, &bar_indic);
		//
    //  /*Create a Label*/
    // lv_obj_t * label3 =  lv_label_create(lv_scr_act(), NULL);
    // lv_label_set_text(label3, "20 C");
    // lv_obj_align(label3, bar3, LV_ALIGN_OUT_BOTTOM_MID, 0, 20); //voffset:20
		//
		// // gauge
		//
		// static lv_color_t colorArray[1] ;
		// colorArray[0]=LV_COLOR_BLACK;
		//
    // gauge=lv_gauge_create(lv_scr_act(), NULL);
		//
    // lv_obj_set_size(gauge,120,120);
    // lv_obj_set_pos(gauge,150,50);
		//
    // lv_gauge_set_range(gauge,0,100);
    // lv_gauge_set_critical_value(gauge,90);
		//
		// lv_gauge_set_needle_count(gauge,1,colorArray);
    // lv_gauge_set_value(gauge,0,88);
		//
    //  /*Create a Label*/
    // lv_obj_t * label1 =  lv_label_create(lv_scr_act(), NULL);
    // lv_label_set_text(label1, "88%");
    // lv_obj_align(label1, gauge, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
}

void app_main()
{
	lv_init();

	disp_spi_init();
	ili9341_init();

#if ENABLE_TOUCH_INPUT
	tp_spi_init();
	xpt2046_init();
#endif

  static lv_color_t buf1[DISP_BUF_SIZE];
  static lv_color_t buf2[DISP_BUF_SIZE];
  static lv_disp_buf_t disp_buf;
  lv_disp_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);

	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.flush_cb = ili9341_flush;
	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);

  // Set TOUCH_SUPPORT on drv\component.mk to 1 if
  // your board have touch support
#if ENABLE_TOUCH_INPUT
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.read_cb = xpt2046_read;
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  lv_indev_drv_register(&indev_drv);
#endif

	esp_register_freertos_tick_hook(lv_tick_task);

  // start example application
	// demo_create(); //-:daiki
	// sysmon_create(); //+:daiki
	// terminal_create(); //+:daiki
	// my_create(); //+:daiki
	// lv_test_lmeter_1(); //+:daiki
	// demoLmeter(); //+:daiki
	// demoGauge(); //+:daiki
	// my_test_bar_1(); //+:daiki
	// lv_test_theme_1(lv_theme_night_init(15, NULL)); //+:daiki
	// my_test_theme(); //+:daiki
	// lv_tutorial_themes(); //+:daiki
	// lv_test_tabview_1(); //+:daiki
	// lv_ex_tabview_1(); //+:daiki
	// lv_test_theme_2(); //+:daiki
	my_test_temperature(); //+:daiki
		lv_task_create(tab_switcher, 3000, LV_TASK_PRIO_MID, tabview); //+:daiki

	while(1) {
		vTaskDelay(1);
		lv_task_handler();
	}
}

static void tab_switcher(lv_task_t * task)
{
    static uint8_t tab = 0;
    lv_obj_t * tv = task->user_data;
    tab++;
    if(tab >= 3) tab = 0;
    lv_tabview_set_tab_act(tabview, tab, true);
}

static void IRAM_ATTR lv_tick_task(void)
{
	lv_tick_inc(portTICK_RATE_MS);
}
