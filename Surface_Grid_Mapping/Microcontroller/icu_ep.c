/***********************************************************************************************************************
 * File Name    : icu_ep.c
 * Description  : Contains function definitions for ICU (Interrupt Controller Unit) module.
 **********************************************************************************************************************/
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
 * Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/

#include "common_utils.h"
#include "icu_ep.h"

/*******************************************************************************************************************//**
 * @addtogroup icu_ep
 * @{
 **********************************************************************************************************************/

/* Boolean flag to determine if the switch is pressed or not. */
volatile bool g_sw_press = false;

/*******************************************************************************************************************//**
 * @brief       This function initializes the ICU (Interrupt Controller Unit) module.
 * @param[IN]   None
 * @retval      FSP_SUCCESS    Successfully opened the ICU module.
 * @retval      Other          An error occurred during the opening of the ICU module.
 **********************************************************************************************************************/
fsp_err_t icu_init(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Open ICU module */
    err = R_ICU_ExternalIrqOpen(&g_external_irq1_ctrl, &g_external_irq1_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* ICU Open failure message */
        APP_ERR_PRINT("\r\n** R_ICU_ExternalIrqOpen API FAILED **\r\n");
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief       This function enables the external interrupt for the specified channel.
 * @param[IN]   None
 * @retval      FSP_SUCCESS    Successfully enabled the ICU module.
 * @retval      Other          An error occurred during enabling of the ICU module.
 **********************************************************************************************************************/
fsp_err_t icu_enable(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Enable ICU module */
    err = R_ICU_ExternalIrqEnable(&g_external_irq1_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* ICU Enable failure message */
        APP_ERR_PRINT("\r\n** R_ICU_ExternalIrqEnable API FAILED **\r\n");
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief       This function closes the ICU module before the project ends or an error trap occurs.
 * @param[IN]   None
 * @retval      None
 **********************************************************************************************************************/
void icu_deinit(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close ICU module */
    err = R_ICU_ExternalIrqClose(&g_external_irq1_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* ICU Close failure message */
        APP_ERR_PRINT("\r\n** R_ICU_ExternalIrqClose API FAILED **\r\n");
    }
}

/*******************************************************************************************************************//**
 * @brief      User-defined callback function for handling external IRQ (Interrupt Request).
 * @param[IN]  p_args    Pointer to external IRQ callback arguments.
 * @retval     None
 **********************************************************************************************************************/
void irq_ep_callback(external_irq_callback_args_t *p_args)
{
    /* Check if the interrupt is from the expected channel */
    if (USER_SW_IRQ_NUMBER == p_args->channel)
    {
        g_sw_press = true;  // Set the switch press flag to true
    }
}

/*******************************************************************************************************************//**
 * @} (end addtogroup icu_ep)
 **********************************************************************************************************************/
