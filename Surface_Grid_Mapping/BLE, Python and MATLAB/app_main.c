/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2019-2022 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/******************************************************************************
* File Name    : app_main.c
* Device(s)    : RA4W1
* Tool-Chain   : e2Studio
* Description  : This is an application file for peripheral role.
*******************************************************************************/

/******************************************************************************
 Includes   <System Includes> , "Project Includes"
*******************************************************************************/
#include <string.h>
#include "r_ble_api.h"
#include "rm_ble_abs.h"
#include "rm_ble_abs_api.h"
#include "discovery/r_ble_disc.h"
#include "gatt_db.h"
#include "profile_cmn/r_ble_servs_if.h"
#include "profile_cmn/r_ble_servc_if.h"
#include "hal_data.h"

#include "../src/ADC.h"
#include "../src/Digital_input.h"
#include "../src/common_utils.h"
#include "../src/Surface_mapping_main.h"

#define BLE_APP_AZURE_RTOS    (1)
#define BLE_APP_FREE_RTOS     (2)

/* This code is needed for using Azure RTOS */
#if (BSP_CFG_RTOS == BLE_APP_AZURE_RTOS)
#include "tx_api.h"

#define  EXECUTE_STACK_SIZE  2048

static TX_SEMAPHORE gs_ble_exe_smpr;
void                *g_ble_event_group_handle = &gs_ble_exe_smpr;
static TX_THREAD    gs_ble_execute_task;
static uint8_t      gs_ble_execute_task_stack[EXECUTE_STACK_SIZE];
static TX_THREAD    *gs_ble_core_task_ptr;
static void         ble_execute_task_func(unsigned long Parameters);
/* This code is needed for using FreeRTOS */
#elif (BSP_CFG_RTOS == BLE_APP_FREE_RTOS)
#if !defined(BLE_CFG_SYNCHRONIZATION_TYPE)
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#define BLE_EVENT_PATTERN   (0x0A0A)
void *g_ble_event_group_handle;
#else /* !defined(BLE_CFG_SYNCHRONIZATION_TYPE) */

#if (BLE_CFG_SYNCHRONIZATION_TYPE == 0)
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define EXECUTE_STACK_SIZE (512)

static SemaphoreHandle_t    gs_ble_exe_smpr;
void                        *g_ble_event_group_handle;
static TaskHandle_t         gs_ble_core_task_ptr;
static TaskHandle_t         gs_ble_execute_task;
static void                 ble_execute_task_func(void *pvParameters);
#elif (BLE_CFG_SYNCHRONIZATION_TYPE == 1)
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#define BLE_EVENT_PATTERN   (0x0A0A)
void *g_ble_event_group_handle;
#endif /* (BLE_CFG_SYNCHRONIZATION_TYPE == x) */
#endif /* !defined(BLE_CFG_SYNCHRONIZATION_TYPE) */
#endif /* (BSP_CFG_RTOS == x) */
#include "r_ble_gapc.h"
#include "r_ble_gaps.h"
#include "r_ble_gats.h"
#include "r_ble_sss.h"

/******************************************************************************
 User file includes
*******************************************************************************/
/* Start user code for file includes. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/******************************************************************************
 User macro definitions
*******************************************************************************/
#define BLE_LOG_TAG "app_main"

/* Queue for Prepare Write Operation. Change if needed. */
#define BLE_GATTS_QUEUE_ELEMENTS_SIZE       (14)
#define BLE_GATTS_QUEUE_BUFFER_LEN          (245)
#define BLE_GATTS_QUEUE_NUM                 (1)

/* Start user code for macro definitions. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/******************************************************************************
 Generated function prototype declarations
*******************************************************************************/
/* Internal functions */
void gap_cb(uint16_t type, ble_status_t result, st_ble_evt_data_t *p_data);
void gatts_cb(uint16_t type, ble_status_t result, st_ble_gatts_evt_data_t *p_data);
void gattc_cb(uint16_t type, ble_status_t result, st_ble_gattc_evt_data_t *p_data);
void vs_cb(uint16_t type, ble_status_t result, st_ble_vs_evt_data_t *p_data);
void disc_comp_cb(uint16_t conn_hdl);
ble_status_t ble_init(void);
void app_main(void);

/******************************************************************************
 User function prototype declarations
*******************************************************************************/
/* Start user code for function prototype declarations. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/******************************************************************************
 Generated global variables
*******************************************************************************/
/* GAP Service UUID */
static uint8_t GAPC_UUID[] = { 0x00, 0x18 };
/* Service discovery parameters */
static st_ble_disc_entry_t gs_disc_entries[] =
{
    {
        .p_uuid     = GAPC_UUID,
        .uuid_type  = BLE_GATT_16_BIT_UUID_FORMAT,
        .serv_cb    = R_BLE_GAPC_ServDiscCb,
    },
};
/* Advertising Data */
static uint8_t gs_advertising_data[] =
{
    /* Flags */
    0x02, /**< Data Size */
    0x01, /**< Data Type */
    ( 0x06 ), /**< Data Value */

    /* Complete List of 128-bit Service Class UUIDs */
    0x11, /**< Data Size */
    0x07, /**< Data Type */
    0xeb, 0x27, 0x39, 0x9f, 0x4d, 0x6d, 0x3c, 0x8d, 0xba, 0x4c, 0xe4, 0x2a, 0x38, 0xc3, 0xe0, 0xbb, /**< Data Value */

    /* Complete Local Name */
    0x08, /**< Data Size */
    0x09, /**< Data Type */
    0x41, 0x44, 0x43, 0x5f, 0x42, 0x4c, 0x45, /**< Data Value */
};

ble_abs_legacy_advertising_parameter_t g_ble_advertising_parameter =
{
    .p_peer_address             = NULL,       ///< Peer address.
    .fast_advertising_interval  = 0x00000020, ///< Fast advertising interval. 20.0(ms)
    .fast_advertising_period    = 0x1770,     ///< Fast advertising period. 60,000(ms)
    .slow_advertising_interval  = 0x00000320, ///< Slow advertising interval. 500.0(ms)
    .slow_advertising_period    = 0x0000,     ///< Slow advertising period.
    .p_advertising_data         = gs_advertising_data,             ///< Advertising data. If p_advertising_data is specified as NULL, advertising data is not set.
    .advertising_data_length    = ARRAY_SIZE(gs_advertising_data), ///< Advertising data length (in bytes).
    .advertising_filter_policy  = BLE_ABS_ADVERTISING_FILTER_ALLOW_ANY, ///< Advertising Filter Policy.
    .advertising_channel_map    = ( BLE_GAP_ADV_CH_37 | BLE_GAP_ADV_CH_38 | BLE_GAP_ADV_CH_39 ), ///< Channel Map.
    .own_bluetooth_address_type = BLE_GAP_ADDR_RAND, ///< Own Bluetooth address type.
    .own_bluetooth_address      = { 0 },
};

/* GATT server callback parameters */
ble_abs_gatt_server_callback_set_t gs_abs_gatts_cb_param[] =
{
    {
        .gatt_server_callback_function = gatts_cb,
        .gatt_server_callback_priority = 1,
    },
    {
        .gatt_server_callback_function = NULL,
    }
};

/* GATT client callback parameters */
ble_abs_gatt_client_callback_set_t gs_abs_gattc_cb_param[] =
{
    {
        .gatt_client_callback_function = gattc_cb,
        .gatt_client_callback_priority = 1,
    },
    {
        .gatt_client_callback_function = NULL,
    }
};

/* GATT server Prepare Write Queue parameters. Define BLE_APP_PREPARE_WRITE_DISABLE macro to reduce memory usage. */
#if (BLE_APP_PREPARE_WRITE_DISABLE == 0)
static st_ble_gatt_queue_elm_t  gs_queue_elms[BLE_GATTS_QUEUE_ELEMENTS_SIZE];
static uint8_t gs_buffer[BLE_GATTS_QUEUE_BUFFER_LEN];
static st_ble_gatt_pre_queue_t gs_queue[BLE_GATTS_QUEUE_NUM] = {
    {
        .p_buf_start = gs_buffer,
        .buffer_len  = BLE_GATTS_QUEUE_BUFFER_LEN,
        .p_queue     = gs_queue_elms,
        .queue_size  = BLE_GATTS_QUEUE_ELEMENTS_SIZE,
    }
};
#endif /* BLE_APP_PREPARE_WRITE_DISABLE */

/* Connection handle */
uint16_t g_conn_hdl;

/******************************************************************************
 User global variables
*******************************************************************************/
/* Start user code for global variables. Do not edit comment generated here */

volatile bool adc_ready = false;

/* End user code. Do not edit comment generated here */

/******************************************************************************
 Generated function definitions
*******************************************************************************/
/******************************************************************************
 * Function Name: gap_cb
 * Description  : Callback function for GAP API.
 * Arguments    : uint16_t type -
 *                  Event type of GAP API.
 *              : ble_status_t result -
 *                  Event result of GAP API.
 *              : st_ble_vs_evt_data_t *p_data - 
 *                  Event parameters of GAP API.
 * Return Value : none
 ******************************************************************************/
void gap_cb(uint16_t type, ble_status_t result, st_ble_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for GAP callback function common process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    switch(type)
    {
        case BLE_GAP_EVENT_STACK_ON:
        {
            /* Get BD Address for Advertising */
            R_BLE_VS_GetBdAddr(BLE_VS_ADDR_AREA_REG, BLE_GAP_ADDR_RAND);
        } break;

        case BLE_GAP_EVENT_CONN_IND:
        {
            if (BLE_SUCCESS == result)
            {
                /* Store connection handle */
                st_ble_gap_conn_evt_t *p_gap_conn_evt_param = (st_ble_gap_conn_evt_t *)p_data->p_param;
                g_conn_hdl = p_gap_conn_evt_param->conn_hdl;

                APP_PRINT("Connected! Starting Pairing...\r\n");

                st_ble_gap_pairing_param_t pairing_param = {
                    .iocap         = BLE_GAP_IOCAP_NOINPUT_NOOUTPUT,
                    .mitm          = BLE_GAP_SEC_MITM_BEST_EFFORT,
                    .sec_conn_only = BLE_GAP_LESC_PAIRING,
                    .bonding       = BLE_GAP_BONDING,
                    .loc_key_dist  = BLE_GAP_KEY_DIST_ENCKEY,
                    .rem_key_dist  = BLE_GAP_KEY_DIST_ENCKEY,
                };

                ble_status_t status = R_BLE_GAP_StartPairing(g_conn_hdl);
                if(BLE_SUCCESS != status)
                {
                    APP_PRINT("Failed to Pair the Devices: %d\r\n", status);
                }

                st_ble_gap_set_phy_param_t phy_param = {
                        .tx_phys  = BLE_GAP_SET_PHYS_HOST_PREF_2M,
                        .rx_phys  = BLE_GAP_SET_PHYS_HOST_PREF_2M
                };

                status = R_BLE_GAP_SetPhy(g_conn_hdl, &phy_param);
                if(BLE_SUCCESS != status)
                {
                    APP_PRINT("Failed to Set PHY to 2 Mbps: %d\r\n", status);
                }
                else
                {
                    APP_PRINT("PHY Set to 2 Mbps Successfully!\r\n");
                }

            }
            else
            {
                /* Restart advertising when connection failed */
                /* Include header file that contained BLE Abstraction (rm_ble_abs) module instance, when application work on RTOS. */
                APP_PRINT("Connection Failed, Restarting Advertising...\r\n");
                RM_BLE_ABS_StartLegacyAdvertising(&g_ble_abs0_ctrl, &g_ble_advertising_parameter);
            }
        } break;

        case BLE_GAP_EVENT_PAIRING_REQ:
        {
            // Handle Pairing Request
            APP_PRINT("Pairing Request Received. Accepting the Request...\r\n");
            st_ble_gap_pairing_req_evt_t  *pairing_req = (st_ble_gap_pairing_req_evt_t *)p_data->p_param;
            R_BLE_GAP_ReplyPairing(pairing_req->conn_hdl, BLE_GAP_PAIRING_ACCEPT);
        } break;

        case BLE_GAP_EVENT_PAIRING_COMP:
        {
            // Handle Pairing Complete Event
            if(result == BLE_SUCCESS)
            {
                APP_PRINT("Pairing Completed Successfully!\r\n");
            }
            else
            {
                APP_PRINT("Pairing Failed with Error: %d\r\n", result);
            }
        } break;

        case BLE_GAP_EVENT_DISCONN_IND:
        {
            /* Restart advertising when disconnected */
            g_conn_hdl = BLE_GAP_INVALID_CONN_HDL;
            /* Include header file that contained BLE Abstraction (rm_ble_abs) module instance, when application work on RTOS. */
            RM_BLE_ABS_StartLegacyAdvertising(&g_ble_abs0_ctrl, &g_ble_advertising_parameter);
        } break;

        case BLE_GAP_EVENT_CONN_PARAM_UPD_REQ:
        {
            /* Send connection update response with value received on connection update request */
            st_ble_gap_conn_upd_req_evt_t *p_conn_upd_req_evt_param = (st_ble_gap_conn_upd_req_evt_t *)p_data->p_param;

            st_ble_gap_conn_param_t conn_updt_param = {
                .conn_intv_min = p_conn_upd_req_evt_param->conn_intv_min,
                .conn_intv_max = p_conn_upd_req_evt_param->conn_intv_max,
                .conn_latency  = p_conn_upd_req_evt_param->conn_latency,
                .sup_to        = p_conn_upd_req_evt_param->sup_to,
                .min_ce_length = 0xFFFF,
                .max_ce_length = 0xFFFF,
            };

            R_BLE_GAP_UpdConn(p_conn_upd_req_evt_param->conn_hdl,
                              BLE_GAP_CONN_UPD_MODE_RSP,
                              BLE_GAP_CONN_UPD_ACCEPT,
                              &conn_updt_param);
        } break;

        /* Hint: Add cases of GAP event macros defined as BLE_GAP_XXX */
        /* Start user code for GAP callback function event process. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }

    /* Start user code for GAP callback function closing process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/******************************************************************************
 * Function Name: gatts_cb
 * Description  : Callback function for GATT Server API.
 * Arguments    : uint16_t type -
 *                  Event type of GATT Server API.
 *              : ble_status_t result -
 *                  Event result of GATT Server API.
 *              : st_ble_gatts_evt_data_t *p_data - 
 *                  Event parameters of GATT Server API.
 * Return Value : none
 ******************************************************************************/
void gatts_cb(uint16_t type, ble_status_t result, st_ble_gatts_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for GATT Server callback function common process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    R_BLE_SERVS_GattsCb(type, result, p_data);
    switch(type)
    {
        /* Hint: Add cases of GATT Server event macros defined as BLE_GATTS_XXX */
        /* Start user code for GATT Server callback function event process. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }

    /* Start user code for GATT Server callback function closing process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/******************************************************************************
 * Function Name: gattc_cb
 * Description  : Callback function for GATT Client API.
 * Arguments    : uint16_t type -
 *                  Event type of GATT Client API.
 *              : ble_status_t result -
 *                  Event result of GATT Client API.
 *              : st_ble_gattc_evt_data_t *p_data - 
 *                  Event parameters of GATT Client API.
 * Return Value : none
 ******************************************************************************/
void gattc_cb(uint16_t type, ble_status_t result, st_ble_gattc_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for GATT Client callback function common process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    R_BLE_SERVC_GattcCb(type, result, p_data);
    switch(type)
    {
        case BLE_GATTC_EVENT_CONN_IND:
        {
            /* Start discovery operation after connection established */
            R_BLE_DISC_Start(p_data->conn_hdl, gs_disc_entries, ARRAY_SIZE(gs_disc_entries), disc_comp_cb);
        } break;

        /* Hint: Add cases of GATT Client event macros defined as BLE_GATTC_XXX */
        /* Start user code for GATT Client callback function event process. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }

    /* Start user code for GATT Client callback function closing process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/******************************************************************************
 * Function Name: vs_cb
 * Description  : Callback function for Vendor Specific API.
 * Arguments    : uint16_t type -
 *                  Event type of Vendor Specific API.
 *              : ble_status_t result -
 *                  Event result of Vendor Specific API.
 *              : st_ble_vs_evt_data_t *p_data - 
 *                  Event parameters of Vendor Specific API.
 * Return Value : none
 ******************************************************************************/
void vs_cb(uint16_t type, ble_status_t result, st_ble_vs_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for vender specific callback function common process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    R_BLE_SERVS_VsCb(type, result, p_data);
    switch(type)
    {
        case BLE_VS_EVENT_GET_ADDR_COMP:
        {
            /* Start advertising when BD address is ready */
            st_ble_vs_get_bd_addr_comp_evt_t * get_address = (st_ble_vs_get_bd_addr_comp_evt_t *)p_data->p_param;
            memcpy(g_ble_advertising_parameter.own_bluetooth_address, get_address->addr.addr, BLE_BD_ADDR_LEN);
            /* Include header file that contained BLE Abstraction (rm_ble_abs) module instance, when application work on RTOS. */
            RM_BLE_ABS_StartLegacyAdvertising(&g_ble_abs0_ctrl, &g_ble_advertising_parameter);
        } break;

        /* Hint: Add cases of vender specific event macros defined as BLE_VS_XXX */
        /* Start user code for vender specific callback function event process. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }

    /* Start user code for vender specific callback function closing process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/******************************************************************************
 * Function Name: gaps_cb
 * Description  : Callback function for GAP Service server feature.
 * Arguments    : uint16_t type -
 *                  Event type of GAP Service server feature.
 *              : ble_status_t result -
 *                  Event result of GAP Service server feature.
 *              : st_ble_servs_evt_data_t *p_data - 
 *                  Event parameters of GAP Service server feature.
 * Return Value : none
 ******************************************************************************/
static void gaps_cb(uint16_t type, ble_status_t result, st_ble_servs_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for GAP Service Server callback function common process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    switch(type)
    {
        /* Hint: Add cases of GAP Service server events defined in e_ble_gaps_event_t */
        /* Start user code for GAP Service Server callback function event process. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }

    /* Start user code for GAP Service Server callback function closing process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/******************************************************************************
 * Function Name: gats_cb
 * Description  : Callback function for GATT Service server feature.
 * Arguments    : uint16_t type -
 *                  Event type of GATT Service server feature.
 *              : ble_status_t result -
 *                  Event result of GATT Service server feature.
 *              : st_ble_servs_evt_data_t *p_data - 
 *                  Event parameters of GATT Service server feature.
 * Return Value : none
 ******************************************************************************/
static void gats_cb(uint16_t type, ble_status_t result, st_ble_servs_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for GATT Service Server callback function common process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    switch(type)
    {
        /* Hint: Add cases of GATT Service server events defined in e_ble_gats_event_t */
        /* Start user code for GATT Service Server callback function event process. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }

    /* Start user code for GATT Service Server callback function closing process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/******************************************************************************
 * Function Name: sss_cb
 * Description  : Callback function for Sensor Service server feature.
 * Arguments    : uint16_t type -
 *                  Event type of Sensor Service server feature.
 *              : ble_status_t result -
 *                  Event result of Sensor Service server feature.
 *              : st_ble_servs_evt_data_t *p_data - 
 *                  Event parameters of Sensor Service server feature.
 * Return Value : none
 ******************************************************************************/
static void sss_cb(uint16_t type, ble_status_t result, st_ble_servs_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for Sensor Service Server callback function common process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    switch(type)
    {
        /* Hint: Add cases of Sensor Service server events defined in e_ble_sss_event_t */
        /* Start user code for Sensor Service Server callback function event process. Do not edit comment generated here */

        case BLE_SSS_EVENT_SC_READ_REQ:
        {
            uint16_t sensor_value = 0;
            R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_4, &sensor_value);
            R_BLE_SSS_SetSc(&sensor_value);
        } break;

        default:
            break;

        /* End user code. Do not edit comment generated here */
    }

    /* Start user code for Sensor Service Server callback function closing process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/******************************************************************************
 * Function Name: gapc_cb
 * Description  : Callback function for GAP Service client feature.
 * Arguments    : uint16_t type -
 *                  Event type of GAP Service client feature.
 *              : ble_status_t result -
 *                  Event result of GAP Service client feature.
 *              : st_ble_servc_evt_data_t *p_data - 
 *                  Event parameters of GAP Service client feature.
 * Return Value : none
 ******************************************************************************/
static void gapc_cb(uint16_t type, ble_status_t result, st_ble_servc_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for GAP Service Client callback function common process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    switch(type)
    {
        /* Hint: Add cases of GAP Service client events defined in e_ble_gapc_event_t */
        /* Start user code for GAP Service Client callback function event process. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }

    /* Start user code for GAP Service Client callback function closing process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}
/******************************************************************************
 * Function Name: disc_comp_cb
 * Description  : Callback function for Service Discovery.
 * Arguments    : none
 * Return Value : none
 ******************************************************************************/
void disc_comp_cb(uint16_t conn_hdl)
{
    /* Hint: Input process such as GATT operation */
    /* Start user code for Discovery Complete callback function. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}
/******************************************************************************
 * Function Name: ble_init
 * Description  : Initialize BLE and profiles.
 * Arguments    : none
 * Return Value : BLE_SUCCESS - SUCCESS
 *                BLE_ERR_INVALID_OPERATION -
 *                    Failed to initialize BLE or profiles.
 ******************************************************************************/
ble_status_t ble_init(void)
{
    ble_status_t status;
    fsp_err_t err;

    /* Initialize BLE */
    /* Include header file that contained BLE Abstraction (rm_ble_abs) module instance, when application work on RTOS. */
    err = RM_BLE_ABS_Open(&g_ble_abs0_ctrl, &g_ble_abs0_cfg);
    if (FSP_SUCCESS != err)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Start user code for global value initialization. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    /* Initialize GATT Database */
    status = R_BLE_GATTS_SetDbInst(&g_gatt_db_table);
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Initialize GATT server */
    status = R_BLE_SERVS_Init();
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Initialize GATT client */
    status = R_BLE_SERVC_Init();
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Set Prepare Write Queue. Define BLE_APP_PREPARE_WRITE_DISABLE macro to reduce memory usage. */
#if (BLE_APP_PREPARE_WRITE_DISABLE == 0)
    R_BLE_GATTS_SetPrepareQueue(gs_queue, BLE_GATTS_QUEUE_NUM);
#endif /* BLE_APP_PREPARE_WRITE_DISABLE */

    /* Initialize GATT Discovery Library */
    status = R_BLE_DISC_Init();
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Initialize GAP Service server API */
    status = R_BLE_GAPS_Init(gaps_cb);
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Initialize GATT Service server API */
    status = R_BLE_GATS_Init(gats_cb);
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Initialize Sensor Service server API */
    status = R_BLE_SSS_Init(sss_cb);
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Initialize GAP Service client API */
    status = R_BLE_GAPC_Init(gapc_cb);
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Start user code for user function initialization. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    return status;
}

/* Timer callback function for ADC readiness */
void adc_timer_callback(timer_callback_args_t *p_args)
{
    adc_ready = true;
}

/******************************************************************************
 * Function Name: app_main
 * Description  : Application main function with main loop
 * Arguments    : none
 * Return Value : none
 ******************************************************************************/
void app_main(void)
{
#if (BSP_CFG_RTOS == BLE_APP_FREE_RTOS)

#if !defined(BLE_CFG_SYNCHRONIZATION_TYPE)
    /* Create Event Group */
    g_ble_event_group_handle = (void *)xEventGroupCreate();
    assert(g_ble_event_group_handle);
#else /* !defined(BLE_CFG_SYNCHRONIZATION_TYPE) */
#if (BLE_CFG_SYNCHRONIZATION_TYPE == 0)
    /* Create Semaphore */
    gs_ble_exe_smpr = xSemaphoreCreateBinary();
    assert(gs_ble_exe_smpr);
    g_ble_event_group_handle = (void *)gs_ble_exe_smpr;
#elif (BLE_CFG_SYNCHRONIZATION_TYPE == 1)
    /* Create Event Group */
    g_ble_event_group_handle = (void *)xEventGroupCreate();
    assert(g_ble_event_group_handle);
#endif /* (BLE_CFG_SYNCHRONIZATION_TYPE == x) */
#endif /* !defined(BLE_CFG_SYNCHRONIZATION_TYPE) */
#endif /* (BSP_CFG_RTOS == BLE_APP_FREE_RTOS) */

    /* Initialize BLE */
    ble_status_t initialize_result = ble_init();
    if(BLE_SUCCESS != initialize_result)
    {
        APP_PRINT("BLE Initialisation Failed\r\n");
        while (1);
    }
    else
    {
        APP_PRINT("BLE Initialised\r\n");
    }

    int i = 0;
    while(i != 20)
    {
        R_BLE_Execute();

        for(i = 0; i <= 20; i++)
        {
            R_BLE_Execute();

            // Send First Notification
            ble_status_t notify_status_1 = R_BLE_SSS_NotifySc(g_conn_hdl, (uint16_t *)&Magnetic_Field_Value1[i]);
            if (notify_status_1 != BLE_SUCCESS)
            {
                APP_PRINT("Waiting for Button User Interface to Read from ADC Channel 4\r\n");
                APP_PRINT("X-Axis Magnetic Field Value not Notified\r\n");
                if(notify_status_1 == BLE_ERR_INVALID_STATE)
                {
                    APP_PRINT("Invalid State: Check if the Notifications are enabled on the Client Side\r\n");
                }
            }

            // Send Second Notification
            ble_status_t notify_status_2 = R_BLE_SSS_NotifySc(g_conn_hdl, (uint16_t *)&Magnetic_Field_Value2[i]);
            if (notify_status_2 != BLE_SUCCESS)
            {
                APP_PRINT("Waiting for Button User Interface to Read from ADC Channel 10\r\n");
                APP_PRINT("Y-Axis Magnetic Field Value not Notified\r\n");
                if(notify_status_2 == BLE_ERR_INVALID_STATE)
                {
                    APP_PRINT("Invalid State: Check if the Notifications are enabled on the Client Side\r\n");
                }
            }

            // Send Third Notification
            ble_status_t notify_status_3 = R_BLE_SSS_NotifySc(g_conn_hdl, (uint16_t *)&Magnetic_Field_Value3[i]);
            if (notify_status_3 != BLE_SUCCESS)
            {
                APP_PRINT("Waiting for Button User Interface to Read from ADC Channel 17\r\n");
                APP_PRINT("Z-Axis Magnetic Field Value not Notified\r\n");
                if(notify_status_3 == BLE_ERR_INVALID_STATE)
                {
                    APP_PRINT("Invalid State: Check if the Notifications are enabled on the Client Side\r\n");
                }
            }
            APP_PRINT("\n");
        }

        surface_main();
    }

    RM_BLE_ABS_Close(&g_ble_abs0_ctrl);

/* When this BLE application works on the Azure RTOS */
#if (BSP_CFG_RTOS == BLE_APP_AZURE_RTOS)

    /* Create Semaphore */
    tx_semaphore_create(&gs_ble_exe_smpr, "BLE_CORE_TASK_SEMAPHOR", TX_NO_INHERIT);

    /* Get Own thread handle */
    gs_ble_core_task_ptr = tx_thread_identify();

    /* Create BLE Execute Task */
    tx_thread_create(&gs_ble_execute_task, (CHAR*) "BLE_EXECUTE_TASK", ble_execute_task_func, (ULONG) NULL,
                            &gs_ble_execute_task_stack, EXECUTE_STACK_SIZE, 1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);

#elif (BSP_CFG_RTOS == BLE_APP_FREE_RTOS)
#if defined(BLE_CFG_SYNCHRONIZATION_TYPE)
#if (BLE_CFG_SYNCHRONIZATION_TYPE == 0)
    /* Get Current Task handle */
    gs_ble_core_task_ptr = xTaskGetCurrentTaskHandle();

    /* Create Execute Task */
    xTaskCreate(ble_execute_task_func, "execute_task", EXECUTE_STACK_SIZE, NULL, configMAX_PRIORITIES-1, &gs_ble_execute_task);
#endif /* (BLE_CFG_SYNCHRONIZATION_TYPE == x) */
#endif /* defined(BLE_CFG_SYNCHRONIZATION_TYPE) */
#endif /* (BSP_CFG_RTOS == x) */

    /* Hint: Input process that should be done before main loop such as calling initial function or variable definitions */
    /* Start user code for process before main loop. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    /* main loop */
    while (1)
    {
        /* Hint: Input process that should be done before BLE_Execute in main loop */
        /* Start user code for process before BLE_Execute. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */

#if (BSP_CFG_RTOS == BLE_APP_AZURE_RTOS)
        /* When this BLE application works on the Azure RTOS */
        if(0 != R_BLE_IsTaskFree())
        {
            tx_thread_suspend(gs_ble_core_task_ptr);
        }
        else
        {
            tx_semaphore_put(&gs_ble_exe_smpr);
        }
#elif (BSP_CFG_RTOS == BLE_APP_FREE_RTOS)
        /* When this BLE application works on the FreeRTOS */
#if !defined(BLE_CFG_SYNCHRONIZATION_TYPE) || (BLE_CFG_SYNCHRONIZATION_TYPE == 1)
        /* Process BLE Event */
        R_BLE_Execute();

        if(0 != R_BLE_IsTaskFree())
        {
            /* If the BLE Task has no operation to be processed, it transits block state until the event from RF transceiver occurs. */
            xEventGroupWaitBits((EventGroupHandle_t)g_ble_event_group_handle,
                                (EventBits_t)BLE_EVENT_PATTERN,
                                pdTRUE,
                                pdFALSE,
                                portMAX_DELAY);
        }
#else /* !defined(BLE_CFG_SYNCHRONIZATION_TYPE) || (BLE_CFG_SYNCHRONIZATION_TYPE == 1) */
#if (BLE_CFG_SYNCHRONIZATION_TYPE == 0)
        if(0 != R_BLE_IsTaskFree())
        {
            vTaskSuspend(NULL);
        }
        else
        {
            xSemaphoreGive(gs_ble_exe_smpr);
        }

#endif /* (BLE_CFG_SYNCHRONIZATION_TYPE == x) || (BLE_CFG_SYNCHRONIZATION_TYPE == 1) */
#endif /* !defined(BLE_CFG_SYNCHRONIZATION_TYPE) */
#else /* (BSP_CFG_RTOS == x) */
        /* Process BLE Event */
        R_BLE_Execute();
#endif /* (BSP_CFG_RTOS == x) */

        /* Hint: Input process that should be done during main loop such as calling processing functions */
        /* Start user code for process during main loop. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }

    /* Hint: Input process that should be done after main loop such as calling closing functions */
    /* Start user code for process after main loop. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

#if (BSP_CFG_RTOS == BLE_APP_AZURE_RTOS)
    tx_thread_terminate(&gs_ble_execute_task);
    tx_thread_delete(&gs_ble_execute_task);
#elif (BSP_CFG_RTOS == BLE_APP_FREE_RTOS)
#if !defined (BLE_CFG_SYNCHRONIZATION_TYPE)
    vEventGroupDelete((EventGroupHandle_t)g_ble_event_group_handle);
#else /* !defined (BLE_CFG_SYNCHRONIZATION_TYPE) */
#if (BLE_CFG_SYNCHRONIZATION_TYPE == 0)
    vTaskDelete(gs_ble_execute_task);
#elif (BLE_CFG_SYNCHRONIZATION_TYPE == 1)
    vEventGroupDelete((EventGroupHandle_t)g_ble_event_group_handle);
#endif /* (BLE_CFG_SYNCHRONIZATION_TYPE == x) */
#endif /* !defined (BLE_CFG_SYNCHRONIZATION_TYPE) */
#endif /* (BSP_CFG_RTOS == x) */

    /* Terminate BLE */
    /* Include header file that contained BLE Abstraction (rm_ble_abs) module instance, when application work on RTOS. */
    RM_BLE_ABS_Close(&g_ble_abs0_ctrl);
}

/* When this BLE application works on the Azure RTOS */
#if (BSP_CFG_RTOS == BLE_APP_AZURE_RTOS)
static void ble_execute_task_func(unsigned long Parameters)
{
    FSP_PARAMETER_NOT_USED(Parameters);
    while(1)
    {
        tx_semaphore_get(&gs_ble_exe_smpr, TX_WAIT_FOREVER);
        while(0 == R_BLE_IsTaskFree())
            R_BLE_Execute();

        tx_thread_resume(gs_ble_core_task_ptr);
    }
}
#else /* (BSP_CFG_RTOS == x) */
#if defined (BLE_CFG_SYNCHRONIZATION_TYPE)
#if (BLE_CFG_SYNCHRONIZATION_TYPE == 0)
static void ble_execute_task_func(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);

    while(1)
    {
        xSemaphoreTake(gs_ble_exe_smpr, portMAX_DELAY);
        while(0 == R_BLE_IsTaskFree())
            R_BLE_Execute();

        vTaskResume(gs_ble_core_task_ptr);
    }
}
#endif /* (BLE_CFG_SYNCHRONIZATION_TYPE == 0) */
#endif /* defined(BLE_CFG_SYNCHRONIZATION_TYPE) */
#endif /* (BSP_CFG_RTOS == x) */

/******************************************************************************
 User function definitions
*******************************************************************************/
/* Start user code for function definitions. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

