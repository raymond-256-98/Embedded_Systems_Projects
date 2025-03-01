/*
 * Digital_output.c
 *
 *  Created on: Aug 7, 2024
 *      Author: Umesh
 */

#include "hal_data.h"
#include "common_utils.h"
#include "icu_ep.h"
#include "Surface_mapping_main.h"
#include "ADC.h"
#include "Digital_input.h"
#include "Digital_output.h"

/**
 * @brief Configures the GPIO pin for the buzzer.
 *
 * This function initializes the GPIO pin associated with the buzzer,
 * setting it as an output. If the initialization fails, an error message
 * is printed using the APP_ERR_PRINT macro.
 */
void Buzzer_Configure()
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize GPIO for Buzzer */
    err = R_IOPORT_PinCfg(&g_ioport_ctrl, BUZZER_PIN, IOPORT_CFG_PORT_DIRECTION_OUTPUT);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("Buzzer GPIO Initialization Failed\r\n");
    }
}
