/********************************************************************************************************
 * @file     main.c
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
 * @date     Sep. 18, 2018
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *			 The information contained herein is confidential and proprietary property of Telink
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in.
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************************************/

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "vendor/common/user_config.h"
#include "vendor/common/blt_soft_timer.h"
#include "drivers.h"
#include "drivers/8258/gpio_8258.h"
#include "app_config.h"
#include "drivers.h"
#include "drivers/8258/gpio_8258.h"

#include "tl_common.h"
#include "stack/ble/ble.h"
#include "vendor/common/user_config.h"
#include "vendor/common/blt_soft_timer.h"

extern void user_init_normal();
extern void user_init_deepRetn();

extern void main_loop (void);


_attribute_ram_code_ void irq_handler(void)
{

	irq_blt_sdk_handler ();

}


int gpio_test1(void)
{
    gpio_toggle(GPIO_PA1);
    return 0;
}


_attribute_ram_code_ int main (void)    //must run in ramcode
{

	DBG_CHN0_LOW;   //debug

	blc_pm_select_internal_32k_crystal();

	cpu_wakeup_init();

	int deepRetWakeUp = pm_is_MCU_deepRetentionWakeup();  //MCU deep retention wakeUp

	rf_drv_init(RF_MODE_BLE_1M);

	gpio_init( !deepRetWakeUp );  //analog resistance will keep available in deepSleep mode, so no need initialize again

#if (CLOCK_SYS_CLOCK_HZ == 16000000)
	clock_init(SYS_CLK_16M_Crystal);
#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
	clock_init(SYS_CLK_24M_Crystal);
#endif

	blc_app_loadCustomizedParameters();  //load customized freq_offset cap value

	if( deepRetWakeUp ){
		user_init_deepRetn ();
	}
	else{
		user_init_normal ();
	}

	gpio_set_func(GPIO_PA1, AS_GPIO);
    gpio_set_output_en(GPIO_PA1, 1);

	blt_soft_timer_add(&gpio_test1, 500000); //7ms <-> 17ms

    irq_enable();


	while (1) {
#if (MODULE_WATCHDOG_ENABLE)
		wd_clear(); //clear watch dog
#endif
		main_loop ();
		blt_soft_timer_process(MAINLOOP_ENTRY);
	}
}

