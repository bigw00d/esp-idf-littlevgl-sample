// Tab switcher example

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "lvgl/lvgl.h"
#include "lv_examples/lv_apps/demo/demo.h"
#include "lv_examples/lv_apps/sysmon/sysmon.h"
#include "lv_examples/lv_apps/terminal/terminal.h"
#include "lv_examples/lv_tests/lv_test_objx/lv_test_bar/lv_test_bar.h"
#include "lv_examples/lv_tests/lv_test_theme/lv_test_theme_1.h"
#include "lvgl/src/lv_themes/lv_theme.h"
#include "lv_examples/lv_tutorial/4_themes/lv_tutorial_themes.h"
#include "lv_examples/lv_tests/lv_test_theme/lv_test_theme_2.h"

#include "esp_freertos_hooks.h"

#include "drv/disp_spi.h"
#include "drv/ili9341.h"
#include "drv/tp_spi.h"
#include "drv/xpt2046.h"

#include "lv_font.h"

#define PADDING_RATE 20

LV_IMG_DECLARE(img_bubble_pattern)

static void IRAM_ATTR lv_tick_task(void);

static void tab_switcher(lv_task_t * task);

lv_obj_t * gauge;
// lv_obj_t * tabview;
lv_obj_t * tabv_obj;

typedef struct _message_frame_t
{
		lv_style_t style_line;
		lv_style_t area_bg;
		lv_point_t line_points[5];
} message_frame_t;

void my_tab_test(void);
void create_home_condition_tabview(lv_obj_t *tabview);
void create_gps_condition_tabview(lv_obj_t *tabview);
void create_battery_condition_tabview(lv_obj_t *tabview);
void create_message_frame(message_frame_t *obj, lv_obj_t *tabview, int start_x, int start_y, int width, int height);

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

	my_tab_test(); // start example application
	lv_task_create(tab_switcher, 3000, LV_TASK_PRIO_MID, tabv_obj); // start timer switching tab

	while(1) {
		vTaskDelay(1);
		lv_task_handler();
	}
}

void my_tab_test(void)
{

		lv_theme_t *theme = lv_theme_alien_init(90, NULL);
		lv_theme_set_current(theme);
		tabv_obj = lv_tabview_create(lv_scr_act(), NULL);

		static lv_style_t *pStyle_tv_btn_rel; //button release style
		pStyle_tv_btn_rel = (lv_style_t *)lv_tabview_get_style(tabv_obj, LV_TABVIEW_STYLE_BTN_REL);
		pStyle_tv_btn_rel->body.padding.left   = LV_DPI / PADDING_RATE;
		pStyle_tv_btn_rel->body.padding.right  = LV_DPI / PADDING_RATE;
		pStyle_tv_btn_rel->body.padding.top    = LV_DPI / PADDING_RATE;
		pStyle_tv_btn_rel->body.padding.bottom = LV_DPI / PADDING_RATE;
		pStyle_tv_btn_rel->text.font = &lv_font_roboto_12;

		static lv_style_t indic;
		lv_style_copy(&indic, &lv_style_plain);
		indic.body.main_color = lv_color_hsv_to_rgb(90, 80, 87); //color for theme alien
		indic.body.grad_color = lv_color_hsv_to_rgb(90, 80, 87); //color for theme alien
		indic.body.padding.inner = LV_DPI / 16;
		lv_tabview_set_style(tabv_obj, LV_TABVIEW_STYLE_INDIC, &indic);

		//Add 3 tabs
    lv_obj_t *tab1 = lv_tabview_add_tab(tabv_obj, LV_SYMBOL_HOME);
    lv_obj_t *tab2 = lv_tabview_add_tab(tabv_obj, LV_SYMBOL_BATTERY_FULL);
    lv_obj_t *tab3 = lv_tabview_add_tab(tabv_obj, LV_SYMBOL_GPS);

		create_home_condition_tabview(tab1);
		create_battery_condition_tabview(tab2);
		create_gps_condition_tabview(tab3);

}

static void tab_switcher(lv_task_t * task)
{
    static uint8_t tab = 0;
    tab++;
    if(tab >= 3) tab = 0;
    lv_tabview_set_tab_act(tabv_obj, tab, true);
}

static void IRAM_ATTR lv_tick_task(void)
{
	lv_tick_inc(portTICK_RATE_MS);
}

void create_home_condition_tabview(lv_obj_t *tabview)
{
	// temperature gauge
	static lv_style_t style;
	lv_style_copy(&style, &lv_style_pretty_color);
	// style.body.main_color = LV_COLOR_GRAY; // Line color at the beginning/ gray
	style.body.main_color = LV_COLOR_GRAY; // Line color at the beginning/ gray
	style.body.grad_color = LV_COLOR_WHITE; // Line color at the end/ gray
	style.body.padding.left = 5; // Scale line length/
	style.body.padding.inner = 8; // Scale label padding/
	style.body.border.color = LV_COLOR_GRAY; // Needle middle circle color/
	style.line.width = 3;
	style.line.color = LV_COLOR_RED; // Line color after the critical value/
	style.text.color = LV_COLOR_GRAY; //lv_color_hex3(0x333);
	style.text.font = &lv_font_roboto_12;
	style.text.opa = LV_OPA_COVER; // no transparent

	static lv_color_t colorArray2[1];
	colorArray2[0] = LV_COLOR_GRAY; // lv_color_hex3(0x333); // transparent(alien theme back ground)
	lv_obj_t * gauge1 = lv_gauge_create(tabview, NULL);
	lv_obj_set_size(gauge1,120,120);
	lv_obj_set_pos(gauge1,20,50);
	lv_gauge_set_range(gauge1,-20,50);
	lv_gauge_set_style(gauge1, LV_GAUGE_STYLE_MAIN, &style);
	lv_gauge_set_critical_value(gauge1,30);
	lv_gauge_set_needle_count(gauge1,1,colorArray2);
	lv_gauge_set_value(gauge1,0,25);
	static lv_style_t label_style1;
	lv_style_copy(&label_style1, &lv_style_plain);
	label_style1.text.font = &lv_font_roboto_28;
	label_style1.text.color = LV_COLOR_WHITE;
	lv_obj_t * label1 =  lv_label_create(tabview, NULL);
	lv_label_set_text(label1, "25  C");
	lv_label_set_style(label1, LV_LABEL_STYLE_MAIN,  &label_style1);
	lv_obj_align(label1, gauge1, LV_ALIGN_CENTER, 0, 45);

	static lv_style_t label_style2;
	lv_style_copy(&label_style2, &lv_style_plain);
	label_style2.text.font = &lv_font_roboto_12;
	label_style2.text.color = LV_COLOR_WHITE;
	lv_obj_t * label2 =  lv_label_create(tabview, NULL);
	lv_label_set_text(label2, "o");
	lv_label_set_style(label2, LV_LABEL_STYLE_MAIN,  &label_style2);
	lv_obj_align(label2, gauge1, LV_ALIGN_CENTER, 7, 40);

	// humidity gauge

	// outline arc
	static lv_style_t style_arc;
	lv_style_copy(&style_arc, &lv_style_plain);
	style_arc.line.color = LV_COLOR_GRAY;
	style_arc.line.width = 4;
	lv_obj_t * arc1 = lv_arc_create(tabview, NULL);
	lv_obj_set_style(arc1, &style_arc);
	lv_arc_set_angles(arc1, 0, 360);
  lv_obj_set_size(arc1, 110, 110);
	lv_obj_set_pos(arc1, 170, 50);
  // real value arc
	static lv_style_t style_arc2;
	lv_style_copy(&style_arc2, &lv_style_plain);
	style_arc2.line.color = lv_color_hex(0x00EBC1); // lv_color_hsv_to_rgb(90, 80, 87);
	style_arc2.line.width = 8;
	lv_obj_t * arc2 = lv_arc_create(tabview, arc1);
	lv_obj_set_style(arc2, &style_arc2);
	lv_arc_set_angles(arc2, 0, 180); //from 90 to 270 degree(all:360)
	lv_obj_set_size(arc2, 115, 115);
	lv_obj_align(arc2, arc1, LV_ALIGN_CENTER, 0, 0);
	// value label
	static lv_style_t label_style3;
	lv_style_copy(&label_style3, &lv_style_plain);
	label_style3.text.font = &lv_font_roboto_28;
	label_style3.text.color = LV_COLOR_WHITE;
	lv_obj_t * label3 =  lv_label_create(tabview, NULL);
	lv_label_set_text(label3, "50%");
	lv_label_set_style(label3, LV_LABEL_STYLE_MAIN,  &label_style3);
	lv_obj_align(label3, arc1, LV_ALIGN_CENTER, 0, 0);

}

void create_gps_condition_tabview(lv_obj_t *tabview)
{
	// gps satellite frame
	static message_frame_t frameObj1;
	create_message_frame(&frameObj1, tabview, 5, 5, 165, 190); //tabview, start_x, start_y, width, height

	// gps satellite
	static lv_style_t style;
	lv_style_copy(&style, &lv_style_pretty_color);
	// style.body.main_color = LV_COLOR_GRAY; // Line color at the beginning/ gray
	style.body.main_color = LV_COLOR_GRAY; // Line color at the beginning/ gray
	style.body.grad_color = LV_COLOR_GRAY; // Line color at the end/ gray
	style.body.padding.left = 2; // Scale line length/
	style.body.padding.inner = 8; // Scale label padding/
	style.body.border.color = LV_COLOR_GRAY; // Needle middle circle color/
	style.line.width = 2;
	style.line.color = LV_COLOR_GRAY; // Line color after the critical value/
	style.text.color = lv_color_hex3(0x333);
	// style.text.opa = LV_OPA_TRANSP; //Object's opacity (0-255)
	style.text.opa = LV_OPA_COVER;

	int start_gauge_x = 20; //horizontal position
	int start_gauge_y = 30; //vertical position
	static lv_color_t needle_colors[] = {
		LV_COLOR_GRAY, LV_COLOR_GRAY, LV_COLOR_GRAY, LV_COLOR_GRAY, LV_COLOR_GRAY, LV_COLOR_GRAY,
		LV_COLOR_GRAY, LV_COLOR_GRAY, LV_COLOR_GRAY, LV_COLOR_GRAY, LV_COLOR_GRAY, LV_COLOR_GRAY
	};
	lv_obj_t * gauge1 = lv_gauge_create(tabview, NULL);
	lv_gauge_set_scale(gauge1, 360, 36*2, 0); // point interval
	lv_gauge_set_style(gauge1, LV_GAUGE_STYLE_MAIN, &style);
	lv_gauge_set_critical_value(gauge1, 360);
	lv_gauge_set_needle_count(gauge1, 12, needle_colors); // 12 needles
	lv_gauge_set_range(gauge1, 0, 360);
	lv_obj_align(gauge1, NULL, LV_ALIGN_IN_TOP_LEFT, start_gauge_x, start_gauge_y);
	lv_obj_set_size(gauge1, 150, 150);
	uint8_t i;
	for(i = 0; i < 12; i++)
	{
		lv_gauge_set_value(gauge1, i, 30*i); // set value needle[i]
	}

	lv_obj_t * gauge2 = lv_gauge_create(tabview, NULL);
	lv_gauge_set_scale(gauge2, 360, 36, 0); // point interval
	lv_gauge_set_style(gauge2, LV_GAUGE_STYLE_MAIN, &style);
	lv_gauge_set_critical_value(gauge2, 360);
	lv_gauge_set_needle_count(gauge2, 1, needle_colors); // 1 needles
	lv_gauge_set_range(gauge2, 0, 360);
	lv_obj_align(gauge2, NULL, LV_ALIGN_IN_TOP_LEFT, start_gauge_x+(150/2)*(1.00-0.66), start_gauge_y+(150/2)*(1.00-0.66));
	lv_obj_set_size(gauge2, 150*0.66, 150*0.66);

	lv_obj_t * gauge3 = lv_gauge_create(tabview, NULL);
	lv_gauge_set_scale(gauge3, 360, 36/2, 0); // point interval
	lv_gauge_set_style(gauge3, LV_GAUGE_STYLE_MAIN, &style);
	lv_gauge_set_critical_value(gauge3, 360);
	lv_gauge_set_needle_count(gauge3, 1, needle_colors); // 1 needles
	lv_gauge_set_range(gauge3, 0, 360);
	lv_obj_align(gauge3, NULL, LV_ALIGN_IN_TOP_LEFT, start_gauge_x+(150/2)*(1.00-0.33), start_gauge_y+(150/2)*(1.00-0.33));
	lv_obj_set_size(gauge3, 150*0.33, 150*0.33);

	lv_obj_t * label_north =  lv_label_create(tabview, NULL);
	lv_label_set_text(label_north, "N");
	lv_obj_align(label_north, gauge1, LV_ALIGN_OUT_TOP_MID, 0, 0);

	/*Create a style for the LED*/
	static lv_style_t style_led;
	lv_style_copy(&style_led, &lv_style_pretty_color);
	style_led.body.radius = LV_RADIUS_CIRCLE;
	style_led.body.main_color = lv_color_hsv_to_rgb(90, 80, 87);
	style_led.body.grad_color = lv_color_hsv_to_rgb(90, 80, 87);
	style_led.body.border.width = 0;
	style_led.body.shadow.width = 0;

	// satellite1
	lv_obj_t * led1  = lv_led_create(tabview, NULL);
	lv_obj_set_style(led1, &style_led);
	lv_obj_align(led1, NULL, LV_ALIGN_IN_TOP_LEFT, 50, 50);
	lv_obj_set_size(led1, (LV_DPI / 6), (LV_DPI / 6));
	lv_led_on(led1);

	lv_obj_t * gps_symbol1 = lv_label_create(tabview, NULL);
	lv_label_set_text(gps_symbol1, "2");
	lv_obj_align(gps_symbol1, led1, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

	// satellite2
	static lv_style_t style_led2;
	lv_style_copy(&style_led2, &style_led);
	style_led2.body.main_color = LV_COLOR_YELLOW;
	style_led2.body.grad_color = LV_COLOR_YELLOW;
	lv_obj_t * led2  = lv_led_create(tabview, NULL);
	lv_obj_set_style(led2, &style_led2);
	lv_obj_align(led2, NULL, LV_ALIGN_IN_TOP_LEFT, 75, 150);
	lv_obj_set_size(led2, (LV_DPI / 6), (LV_DPI / 6));
	lv_led_on(led2);
	lv_obj_t * gps_symbol2 = lv_label_create(tabview, NULL);
	lv_label_set_text(gps_symbol2, "16");
	lv_obj_align(gps_symbol2, led2, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

	// satellite3
	static lv_style_t style_led3;
	lv_style_copy(&style_led3, &style_led);
	style_led3.body.main_color = LV_COLOR_ORANGE;
	style_led3.body.grad_color = LV_COLOR_ORANGE;
	lv_obj_t * led3  = lv_led_create(tabview, NULL);
	lv_obj_set_style(led3, &style_led3);
	lv_obj_align(led3, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 75);
	lv_obj_set_size(led3, (LV_DPI / 6), (LV_DPI / 6));
	lv_led_on(led3);

	lv_obj_t * gps_symbol3 = lv_label_create(tabview, NULL);
	lv_label_set_text(gps_symbol3, "30");
	lv_obj_align(gps_symbol3, led3, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

	// snr histogram frame
	static message_frame_t frameObj2;
	create_message_frame(&frameObj2, tabview, 185, 100, 125, 100); //tabview, start_x, start_y, width, height

	// style (snr bar)
	static lv_style_t bar_bg;
	lv_style_copy(&bar_bg, &lv_style_pretty);
	bar_bg.body.main_color = lv_color_hex(0x012E43);
	static lv_style_t bar_indic;
	lv_style_copy(&bar_indic, &lv_style_pretty);
	bar_indic.body.main_color = lv_color_hsv_to_rgb(90, 80, 87); //color for theme alien
	bar_indic.body.grad_color = lv_color_hsv_to_rgb(90, 80, 87); //color for theme alien
	bar_indic.body.shadow.width = 0;
	bar_indic.body.padding.top = 0;
	bar_indic.body.padding.bottom = 0;
	bar_indic.body.padding.left = 0;
	bar_indic.body.padding.right = 0;

	// satellite1 (snr bar)
	lv_obj_t * bar1 = lv_bar_create(tabview, NULL);
	lv_obj_set_size(bar1, 20, 40);
	lv_obj_set_pos(bar1, 200, 140);
	lv_bar_set_value(bar1, 85, false); // bar, new_value, LV_ANIM_ON/OFF
	lv_bar_set_style(bar1, LV_BAR_STYLE_BG, &bar_bg);
	lv_bar_set_style(bar1, LV_BAR_STYLE_INDIC, &bar_indic);

	// satellite1(snr label)
	lv_obj_t * label1_1 =  lv_label_create(tabview, NULL);
	lv_label_set_text(label1_1, "2");
	lv_obj_align(label1_1, bar1, LV_ALIGN_OUT_BOTTOM_MID, 0, 5); //voffset:5
	lv_obj_t * label1_2 =  lv_label_create(tabview, NULL);
	lv_label_set_text(label1_2, "36");
	lv_obj_align(label1_2, bar1, LV_ALIGN_OUT_TOP_MID, 0, 5); //voffset:5

	// style (snr bar)
	static lv_style_t bar_indic2;
	lv_style_copy(&bar_indic2, &bar_indic);
	bar_indic2.body.main_color = LV_COLOR_YELLOW; //color for theme alien
	bar_indic2.body.grad_color = LV_COLOR_YELLOW; //color for theme alien

	// satellite2 (snr bar)
	lv_obj_t * bar2 = lv_bar_create(tabview, NULL);
	lv_obj_set_size(bar2, 20, 40);
	lv_obj_set_pos(bar2, 230, 140);
	lv_bar_set_value(bar2, 60, false); // bar, new_value, LV_ANIM_ON/OFF
	lv_bar_set_style(bar2, LV_BAR_STYLE_BG, &bar_bg);
	lv_bar_set_style(bar2, LV_BAR_STYLE_INDIC, &bar_indic2);

	// satellite2(snr label)
	lv_obj_t * label2_1 =  lv_label_create(tabview, NULL);
	lv_label_set_text(label2_1, "16");
	lv_obj_align(label2_1, bar2, LV_ALIGN_OUT_BOTTOM_MID, 0, 5); //voffset:5
	lv_obj_t * label2_2 =  lv_label_create(tabview, NULL);
	lv_label_set_text(label2_2, "20");
	lv_obj_align(label2_2, bar2, LV_ALIGN_OUT_TOP_MID, 0, 5); //voffset:5

	// style (snr bar)
	static lv_style_t bar_indic3;
	lv_style_copy(&bar_indic3, &bar_indic);
	bar_indic3.body.main_color = LV_COLOR_ORANGE; //color for theme alien
	bar_indic3.body.grad_color = LV_COLOR_ORANGE; //color for theme alien

	// satellite3 (snr bar)
	lv_obj_t * bar3 = lv_bar_create(tabview, NULL);
	lv_obj_set_size(bar3, 20, 40);
	lv_obj_set_pos(bar3, 260, 140);
	lv_bar_set_value(bar3, 40, false); // bar, new_value, LV_ANIM_ON/OFF
	lv_bar_set_style(bar3, LV_BAR_STYLE_BG, &bar_bg);
	lv_bar_set_style(bar3, LV_BAR_STYLE_INDIC, &bar_indic3);

	// satellite3(snr label)
	lv_obj_t * label3_1 =  lv_label_create(tabview, NULL);
	lv_label_set_text(label3_1, "30");
	lv_obj_align(label3_1, bar3, LV_ALIGN_OUT_BOTTOM_MID, 0, 5); //voffset:5
	lv_obj_t * label3_2 =  lv_label_create(tabview, NULL);
	lv_label_set_text(label3_2, "12");
	lv_obj_align(label3_2, bar3, LV_ALIGN_OUT_TOP_MID, 0, 5); //voffset:5

	// inview frame
	static message_frame_t frameObj3;
	create_message_frame(&frameObj3, tabview, 190, 15, 125, 70); //tabview, start_x, start_y, width, height
	// inview label
	static lv_style_t label_style3;
	lv_style_copy(&label_style3, &lv_style_plain);
	label_style3.text.font = &lv_font_roboto_16;
	label_style3.text.color = LV_COLOR_WHITE;
	lv_obj_t * label3 =  lv_label_create(tabview, NULL);
	lv_label_set_text(label3, "Accuracy[m]");
	lv_label_set_style(label3, LV_LABEL_STYLE_MAIN,  &label_style3);
	lv_obj_set_pos(label3, 200, 30);

	static lv_style_t label_style4;
	lv_style_copy(&label_style4, &label_style3);
	label_style4.text.font = &lv_font_roboto_28;
	lv_obj_t * label4 =  lv_label_create(tabview, NULL);
	lv_label_set_text(label4, "--");
	lv_label_set_style(label4, LV_LABEL_STYLE_MAIN,  &label_style4);
	lv_obj_align(label4, label3, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

}

void create_message_frame(message_frame_t *obj, lv_obj_t *tabview, int start_x, int start_y, int width, int height)
{
	// frame area
	lv_style_copy(&(obj->area_bg), &lv_style_pretty);
	obj->area_bg.body.main_color = lv_color_hex(0x012E43);
	obj->area_bg.body.grad_color = lv_color_hex(0x012E43);

	lv_obj_t * area1 = lv_bar_create(tabview, NULL);
	lv_obj_set_size(area1, width, height);
	lv_obj_align(area1, NULL, LV_ALIGN_IN_TOP_LEFT, start_x-5, start_y);
	lv_bar_set_style(area1, LV_BAR_STYLE_BG, &(obj->area_bg));
	lv_bar_set_value(area1, 0, false); // bar, new_value, LV_ANIM_ON/OFF

	// frame line
	obj->line_points[0].x = start_x;
	obj->line_points[0].y = start_y;
	obj->line_points[1].x = start_x+width;
	obj->line_points[1].y = start_y;
	obj->line_points[2].x = start_x+width;
	obj->line_points[2].y = start_y+height;
	obj->line_points[3].x = start_x;
	obj->line_points[3].y = start_y+height-2;
	obj->line_points[4].x = start_x-2;
	obj->line_points[4].y = start_y;

	lv_style_copy(&(obj->style_line), &lv_style_plain);
	(obj->style_line).line.color = lv_color_hex(0x02CDFF); //frame color
	(obj->style_line).line.width = 2;
	(obj->style_line).line.rounded = 1;
	lv_obj_t * line1;
	line1 = lv_line_create(tabview, NULL);
	lv_line_set_points(line1, obj->line_points, 5);     /*Set the points*/
	lv_line_set_style(line1, LV_LINE_STYLE_MAIN, &(obj->style_line));
	lv_obj_align(line1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
}

void create_battery_condition_tabview(lv_obj_t *tabview)
{

	// value frame
	static message_frame_t frameObj;
	create_message_frame(&frameObj, tabview, 40, 60, 200, 100); //tabview, start_x, start_y, width, height

	// battery level bar
	static lv_style_t style_batt_bar;
	lv_style_copy(&style_batt_bar, &lv_style_pretty);
	style_batt_bar.body.main_color = lv_color_hex(0x005F7B);
	style_batt_bar.body.grad_color = lv_color_hex(0x005F7B);
	static lv_style_t batt_bar_indic;
	lv_style_copy(&batt_bar_indic, &lv_style_pretty);
	batt_bar_indic.body.main_color = lv_color_hex(0x02CDFF);
	batt_bar_indic.body.grad_color = lv_color_hex(0x02CDFF);
	batt_bar_indic.body.shadow.color = LV_COLOR_RED;
	batt_bar_indic.body.shadow.width = 0;
	batt_bar_indic.body.padding.top = 0;       /*Set the padding around the indicator*/
	batt_bar_indic.body.padding.bottom = 0;
	batt_bar_indic.body.padding.left = 0;
	batt_bar_indic.body.padding.right = 0;
	lv_obj_t * batt_bar = lv_bar_create(tabview, NULL);
	lv_obj_set_size(batt_bar, 150, 30);
	lv_obj_align(batt_bar, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 120);
	lv_bar_set_style(batt_bar, LV_BAR_STYLE_BG, &style_batt_bar);
	lv_bar_set_style(batt_bar, LV_BAR_STYLE_INDIC, &batt_bar_indic);
	lv_bar_set_value(batt_bar, 80, false); // bar, new_value, LV_ANIM_ON/O

	// battery level label
	static lv_style_t batt_lv_style; //battery level style
	static lv_style_t batt_lv_style2; //battery value style
	lv_style_copy(&batt_lv_style, &lv_style_btn_rel);
	batt_lv_style.text.font = &lv_font_roboto_28;
	lv_style_copy(&batt_lv_style, &lv_style_btn_rel);
	lv_style_copy(&batt_lv_style2, &batt_lv_style);
	batt_lv_style.text.color = lv_color_hex(0x00D2FF); //LV_COLOR_CYAN;
	batt_lv_style2.text.color = lv_color_hex(0x00D2FF); //LV_COLOR_CYAN;
	lv_obj_t * label_batt_level =  lv_label_create(tabview, NULL);
	lv_label_set_text(label_batt_level, "BATTERY LEVEL");
	lv_label_set_align(label_batt_level,LV_LABEL_ALIGN_CENTER);
	lv_obj_align(label_batt_level, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 80);
	lv_label_set_style(label_batt_level, LV_CONT_STYLE_MAIN,  &batt_lv_style);
	lv_obj_t * label_level_value =  lv_label_create(tabview, NULL);
	lv_label_set_text(label_level_value, " 88%");
	lv_obj_align(label_level_value, label_batt_level, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	lv_label_set_style(label_level_value, LV_CONT_STYLE_MAIN,  &batt_lv_style2);
}
