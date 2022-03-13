/********************************************************************************************************
 * @file     app.c
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
 * @date     Sep. 18, 2018
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *			 The information contained herein is confidential and
 *proprietary property of Telink Semiconductor (Shanghai) Co., Ltd. and is
 *available under the terms of Commercial License Agreement between Telink
 *Semiconductor (Shanghai) Co., Ltd. and the licensee in separate contract or
 *the terms described here-in. This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the
 *information in this file under Mutual Non-Disclosure Agreement. NO WARRENTY of
 *ANY KIND is provided.
 *
 *******************************************************************************************************/

#include "application/keyboard/keyboard.h"
#include "application/usbstd/usbkeycode.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "tl_common.h"
#include "vendor/common/blt_common.h"
#include "vendor/common/blt_led.h"
#include "app_config.h"

#define ADV_IDLE_ENTER_DEEP_TIME 60   // 60 s
#define CONN_IDLE_ENTER_DEEP_TIME 60  // 60 s

#define MY_DIRECT_ADV_TMIE 2000000

#define MY_APP_ADV_CHANNEL BLT_ENABLE_ADV_ALL
#define MY_ADV_INTERVAL_MIN ADV_INTERVAL_10MS
#define MY_ADV_INTERVAL_MAX ADV_INTERVAL_35MS

#define MY_RF_POWER_INDEX RF_POWER_P10p46dBm

#define BLE_DEVICE_ADDRESS_TYPE BLE_DEVICE_ADDRESS_PUBLIC

_attribute_data_retention_ own_addr_type_t app_own_address_type =
    OWN_ADDRESS_PUBLIC;

#define RX_FIFO_SIZE 64
#define RX_FIFO_NUM 8

#define TX_FIFO_SIZE 40
#define TX_FIFO_NUM 16

#if 0
	MYFIFO_INIT(blt_rxfifo, RX_FIFO_SIZE, RX_FIFO_NUM);
#else
_attribute_data_retention_ u8 blt_rxfifo_b[RX_FIFO_SIZE * RX_FIFO_NUM] = {0};
_attribute_data_retention_ my_fifo_t blt_rxfifo = {
    RX_FIFO_SIZE, RX_FIFO_NUM, 0, 0, blt_rxfifo_b,
};
#endif

#if 0
	MYFIFO_INIT(blt_txfifo, TX_FIFO_SIZE, TX_FIFO_NUM);
#else
_attribute_data_retention_ u8 blt_txfifo_b[TX_FIFO_SIZE * TX_FIFO_NUM] = {0};
_attribute_data_retention_ my_fifo_t blt_txfifo = {
    TX_FIFO_SIZE, TX_FIFO_NUM, 0, 0, blt_txfifo_b,
};
#endif

//////////////////////////////////////////////////////////////////////////////
//	 Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////
const u8 tbl_advData[] = {
    0x05, 0x09, 'k',  'H',  'I',  'D',
    0x02, 0x01, 0x05,  // BLE limited discoverable mode and BR/EDR not supported
    0x03, 0x19, 0x80, 0x01,  // 384, Generic Remote Control, Generic category
    0x05, 0x02, 0x12, 0x18, 0x0F, 0x18,  // incomplete list of service class
                                         // UUIDs (0x1812, 0x180F)
};

const u8 tbl_scanRsp[] = {
    0x08, 0x09, 'k', 'S', 'a', 'm', 'p', 'l', 'e',
};

_attribute_data_retention_ int device_in_connection_state;

_attribute_data_retention_ u32 advertise_begin_tick;

_attribute_data_retention_ u32 interval_update_tick;

_attribute_data_retention_ u8 sendTerminate_before_enterDeep = 0;

_attribute_data_retention_ u32 latest_user_event_tick;

void app_switch_to_indirect_adv(u8 e, u8 *p, int n)
{
    bls_ll_setAdvParam(MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                       ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type, 0,
                       NULL, MY_APP_ADV_CHANNEL, ADV_FP_NONE);

    bls_ll_setAdvEnable(1);  // must: set adv enable
}

void ble_remote_terminate(u8 e, u8 *p, int n)  //*p is terminate reason
{
    printf("disconnect\n");
    device_in_connection_state = 0;

    if (*p == HCI_ERR_CONN_TIMEOUT) {
    } else if (*p == HCI_ERR_REMOTE_USER_TERM_CONN) {  // 0x13

    } else if (*p == HCI_ERR_CONN_TERM_MIC_FAILURE) {
    } else {
    }

#if (BLE_APP_PM_ENABLE)
    // user has push terminate pkt to ble TX buffer before deepsleep
    if (sendTerminate_before_enterDeep == 1) {
        sendTerminate_before_enterDeep = 2;
    }
#endif

    advertise_begin_tick = clock_time();
}

_attribute_ram_code_ void user_set_rf_power(u8 e, u8 *p, int n)
{
    rf_set_power_level_index(MY_RF_POWER_INDEX);
}

void task_connect(u8 e, u8 *p, int n)
{
    printf("new connection\n");
    //	bls_l2cap_requestConnParamUpdate (8, 8, 19, 200);  // 200mS
    bls_l2cap_requestConnParamUpdate(8, 8, 99, 400);  // 1 S
    //	bls_l2cap_requestConnParamUpdate (8, 8, 149, 600);  // 1.5 S
    //	bls_l2cap_requestConnParamUpdate (8, 8, 199, 800);  // 2 S
    //	bls_l2cap_requestConnParamUpdate (8, 8, 249, 800);  // 2.5 S
    //	bls_l2cap_requestConnParamUpdate (8, 8, 299, 800);  // 3 S

    latest_user_event_tick = clock_time();

    device_in_connection_state = 1;  //

    interval_update_tick = clock_time() | 1;  // none zero
}

void task_conn_update_req(u8 e, u8 *p, int n) {}

void task_conn_update_done(u8 e, u8 *p, int n) {}

void user_init_normal(void)
{
    // random number generator must be initiated here( in the beginning of
    // user_init_nromal) when deepSleep retention wakeUp, no need initialize
    // again
    random_generator_init();  // this is must

    ////////////////// BLE stack initialization
    ///////////////////////////////////////
    u8 mac_public[6];
    u8 mac_random_static[6];
    blc_initMacAddress(CFG_ADR_MAC, mac_public, mac_random_static);

#if (BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_PUBLIC)
    app_own_address_type = OWN_ADDRESS_PUBLIC;
#elif (BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_RANDOM_STATIC)
    app_own_address_type = OWN_ADDRESS_RANDOM;
    blc_ll_setRandomAddr(mac_random_static);
#endif

    ////// Controller Initialization  //////////
    blc_ll_initBasicMCU();                  // mandatory
    blc_ll_initStandby_module(mac_public);  // mandatory
    // adv module: mandatory for BLE slave,
    blc_ll_initAdvertising_module(mac_public);
    // connection module  mandatory for BLE slave/master
    blc_ll_initConnection_module();
    // slave module: mandatory for BLE slave,
    blc_ll_initSlaveRole_module();
    // blc_ll_initPowerManagement_module();  // pm module: optional

    ////// Host Initialization  //////////
    // gap initialization
    blc_gap_peripheral_init();
    extern void my_att_init();
    // gatt initialization
    my_att_init();
    // l2cap initialization
    blc_l2cap_register_handler(blc_l2cap_packet_receive);

    // Smp Initialization may involve flash write/erase(when one sector stores
    // too much information,
    //   is about to exceed the sector threshold, this sector must be erased,
    //   and all useful information should re_stored) , so it must be done after
    //   battery check
#if (BLE_REMOTE_SECURITY_ENABLE)
    blc_smp_peripheral_init();
#else
    blc_smp_setSecurityLevel(No_Security);
#endif

    ///////////////////// USER application initialization ///////////////////
    bls_ll_setAdvData((u8 *)tbl_advData, sizeof(tbl_advData));
    bls_ll_setScanRspData((u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));

    ////////////////// config adv packet /////////////////////
#if (BLE_REMOTE_SECURITY_ENABLE)
    u8 bond_number =
        blc_smp_param_getCurrentBondingDeviceNumber();  // get bonded device
                                                        // number
    smp_param_save_t bondInfo;
    if (bond_number)  // at least 1 bonding device exist
    {
        bls_smp_param_loadByIndex(bond_number - 1,
                                  &bondInfo);  // get the latest bonding device
                                               // (index: bond_number-1 )
    }

    if (bond_number)  // set direct adv
    {
        // set direct adv
        u8 status = bls_ll_setAdvParam(
            MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
            ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, app_own_address_type,
            bondInfo.peer_addr_type, bondInfo.peer_addr, MY_APP_ADV_CHANNEL,
            ADV_FP_NONE);
        if (status != BLE_SUCCESS) {
            write_reg8(0x40002, 0x11);
            while (1)
                ;
        }  // debug: adv setting err

        // it is recommended that direct adv only last for several seconds, then
        // switch to indirect adv
        bls_ll_setAdvDuration(MY_DIRECT_ADV_TMIE, 1);
        bls_app_registerEventCallback(BLT_EV_FLAG_ADV_DURATION_TIMEOUT,
                                      &app_switch_to_indirect_adv);

    } else  // set indirect adv
#endif
    {
        u8 status = bls_ll_setAdvParam(MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                                       ADV_TYPE_CONNECTABLE_UNDIRECTED,
                                       app_own_address_type, 0, NULL,
                                       MY_APP_ADV_CHANNEL, ADV_FP_NONE);
        if (status != BLE_SUCCESS) {
            write_reg8(0x40002, 0x11);
            while (1)
                ;
        }  // debug: adv setting err
    }

    bls_ll_setAdvEnable(1);  // adv enable

    // set rf power index, user must set it after every suspend wakeup, cause
    // relative setting will be reset in suspend
    user_set_rf_power(0, 0, 0);
    bls_app_registerEventCallback(BLT_EV_FLAG_SUSPEND_EXIT, &user_set_rf_power);

    // ble event call back
	 // it is recommended that direct adv only last for several seconds, then
        // switch to indirect adv
	bls_ll_setAdvDuration(MY_DIRECT_ADV_TMIE, 1);
	bls_app_registerEventCallback(BLT_EV_FLAG_ADV_DURATION_TIMEOUT,
									&app_switch_to_indirect_adv);
    bls_app_registerEventCallback(BLT_EV_FLAG_CONNECT, &task_connect);
    bls_app_registerEventCallback(BLT_EV_FLAG_TERMINATE, &ble_remote_terminate);

    bls_app_registerEventCallback(BLT_EV_FLAG_CONN_PARA_REQ,
                                  &task_conn_update_req);
    bls_app_registerEventCallback(BLT_EV_FLAG_CONN_PARA_UPDATE,
                                  &task_conn_update_done);

    ///////////////////// Power Management initialization///////////////////
    bls_pm_setSuspendMask(SUSPEND_DISABLE);

    advertise_begin_tick = clock_time();
}

void main_loop(void) { blt_sdk_main_loop(); }
