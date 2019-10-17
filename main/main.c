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

#include "esp_freertos_hooks.h"

#include "drv/disp_spi.h"
#include "drv/ili9341.h"
#include "drv/tp_spi.h"
#include "drv/xpt2046.h"

#include "lv_font.h"

static void IRAM_ATTR lv_tick_task(void);

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


void my_test_temperature(void)
{

		// bar

		static lv_style_t bar_bg;
    lv_style_copy(&bar_bg, &lv_style_pretty);
    bar_bg.body.main_color = LV_COLOR_GRAY;

    static lv_style_t bar_indic;
    lv_style_copy(&bar_indic, &lv_style_pretty);
    bar_indic.body.main_color = LV_COLOR_RED;
    bar_indic.body.grad_color = LV_COLOR_MAROON;
    bar_indic.body.shadow.color = LV_COLOR_RED;
    // bar_indic.body.shadow.width = 20;
    // bar_indic.body.padding.top = 10;       /*Set the padding around the indicator*/
    // bar_indic.body.padding.bottom = 3;
    // bar_indic.body.padding.left = 3;
    // bar_indic.body.padding.right = 10;

    lv_obj_t * bar3 = lv_bar_create(lv_disp_get_scr_act(NULL), NULL);
    lv_obj_set_size(bar3, 25, 100);
		lv_obj_set_pos(bar3, 50, 50);
		lv_bar_set_value(bar3, 75, false); // bar, new_value, LV_ANIM_ON/OFF
		lv_bar_set_style(bar3, LV_BAR_STYLE_BG, &bar_bg);
		lv_bar_set_style(bar3, LV_BAR_STYLE_INDIC, &bar_indic);

     /*Create a Label*/
    lv_obj_t * label3 =  lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label3, "20 C");
    lv_obj_align(label3, bar3, LV_ALIGN_OUT_BOTTOM_MID, 0, 20); //voffset:20

		// gauge

		// static lv_color_t colorArray[2] ;
    // colorArray[0]=LV_COLOR_RED;
    // colorArray[1]=LV_COLOR_GREEN;
		static lv_color_t colorArray[1] ;
		colorArray[0]=LV_COLOR_BLACK;

    gauge=lv_gauge_create(lv_scr_act(), NULL);

    lv_obj_set_size(gauge,120,120);
    lv_obj_set_pos(gauge,150,50);

    lv_gauge_set_range(gauge,0,100);
    lv_gauge_set_critical_value(gauge,90);

    // lv_gauge_set_needle_count(gauge,2,colorArray);
    // lv_gauge_set_value(gauge,0,10);
    // lv_gauge_set_value(gauge,1,20);
		lv_gauge_set_needle_count(gauge,1,colorArray);
    lv_gauge_set_value(gauge,0,88);

     /*Create a Label*/
    lv_obj_t * label1 =  lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label1, "88%");
    lv_obj_align(label1, gauge, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
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

	// demo_create(); //-:daiki
	// sysmon_create(); //+:daiki
	// terminal_create(); //+:daiki
	// my_create(); //+:daiki
	// lv_test_lmeter_1(); //+:daiki
	// demoLmeter(); //+:daiki
	// demoGauge(); //+:daiki
	// my_test_bar_1(); //+:daiki
	my_test_temperature(); //+:daiki

	while(1) {
		vTaskDelay(1);
		lv_task_handler();
	}
}

static void IRAM_ATTR lv_tick_task(void)
{
	lv_tick_inc(portTICK_RATE_MS);
}
