/*
 * Timer.c
 *
 *  Created on: Aug 7, 2024
 *      Author: Umesh
 */

#include "hal_data.h"
#include "common_utils.h"
#include "icu_ep.h"
#include "Surface_mapping_main.h"
#include "ADC.h"
#include "Timer.h"

/*******************************************************************************************************************//**
 * @brief Timer callback function. This function is triggered when the timer reaches the end of its cycle.
 *
 * @param[in] p_args  Pointer to the timer callback arguments.
 **********************************************************************************************************************/
void timer_callback(timer_callback_args_t *p_args)
{
    if (p_args->event == TIMER_EVENT_CYCLE_END)
    {
        g_timer_finished = true;  ///< Set the timer finished flag when the cycle ends.
    }
}

/*******************************************************************************************************************//**
 * @brief Configures and starts the General Purpose Timer (GPT).
 **********************************************************************************************************************/
void timer_configure()
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize Timer */
    err = R_GPT_Open(&g_timer3_ctrl, &g_timer3_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("Timer Initialization Failed\r\n");  ///< Error message for timer initialization failure.
        return;
    }

    /* Start Timer */
    err = R_GPT_Start(&g_timer3_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("Timer Start Failed\r\n");  ///< Error message for timer start failure.
    }
}
