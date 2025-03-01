/*
 * Digital_input.c
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
#include "Digital_input.h"

// Callback function for button press (External IRQ)
void button_callback(external_irq_callback_args_t *p_args)
{
    g_sw_press = true;  // Set the button pressed flag
}

void Digital_input()
{
    fsp_err_t err = FSP_SUCCESS;  // Initialize error status to success

    /* Handle button press */
    if (g_sw_press)
    {
        g_sw_press = false; // Clear the button press flag
        g_app_running = !g_app_running; // Toggle application state

        /* Notify that the user pushbutton is pressed */
        APP_PRINT("\r\nUser Pushbutton Pressed\r\n");

        /* Handle error in pin read operation */
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("** R_IOPORT_PinRead FAILED ** \r\n");
            /* Close External IRQ module */
            icu_deinit();
            APP_ERR_TRAP(err);
        }

        /* Handle error in pin write operation */
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("** R_IOPORT_PinWrite FAILED ** \r\n");
            /* Close External IRQ module */
            icu_deinit();
            APP_ERR_TRAP(err);
        }
    }

    /* If the application is running, start ADC scanning */
    if (g_app_running)
    {
        R_ADC_ScanStart(&g_adc0_ctrl);  // Start continuous ADC scanning

        // Loop continuously while the application is running and the button is not pressed
        while (g_app_running && !g_sw_press)
        {
            ADC();  // Perform ADC operation

            // Handle button press during ADC scanning
            if (g_sw_press)
            {
                g_sw_press = false;    // Clear the button press flag
                g_app_running = false; // Stop application
                break;                 // Exit loop to handle button press
            }
        }
    }
    else
    {
        // Ensure the buzzer is off when not running
        R_IOPORT_PinWrite(&g_ioport_ctrl, BUZZER_PIN, BSP_IO_LEVEL_LOW);
    }
}
