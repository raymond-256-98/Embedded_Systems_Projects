/*
 * Surface_mapping_main.c
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

/*******************************************************************************************************************//**
 * @brief This is the main function for the Surface Mapping application. It initializes and configures the necessary
 *        peripherals including the ICU, ADC, Timer, and Digital Input.
 **********************************************************************************************************************/
void surface_main()
{
    fsp_err_t err = FSP_SUCCESS;

    /************ ICU Initialization ************/
    /* Initialize External IRQ driver */
    err = icu_init();
    /* Handle error if ICU initialization fails */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** ICU INIT FAILED ** \r\n");
        APP_ERR_TRAP(err);
    }

    /************ ICU Enabling ************/
    /* Enable External IRQ driver */
    err = icu_enable();
    /* Handle error if ICU enabling fails */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** ICU ENABLE FAILED ** \r\n");
        /* Deinitialize ICU module before trapping */
        icu_deinit();
        APP_ERR_TRAP(err);
    }

    /************ Peripheral Configurations ************/
    /* Configure the ADC */
    ADC_configure();

    /* Configure the Timer */
    timer_configure();

    /* Configure the Buzzer GPIO */
    Buzzer_Configure();

    /************ Application Start ************/
    /* Handle the button press for starting the ADC */
    Digital_input();
}
