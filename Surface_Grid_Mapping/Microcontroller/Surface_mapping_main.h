/*
 * Surface_mapping_main.h
 *
 *  Created on: Aug 7, 2024
 *      Author: Umesh
 */

#ifndef SURFACE_MAPPING_MAIN_H_
#define SURFACE_MAPPING_MAIN_H_

/*******************************************************************************************************************//**
 * @brief Entry point for the Surface Mapping application. Initializes and configures peripherals and manages the
 *        application flow.
 **********************************************************************************************************************/
void surface_main(void);

/*******************************************************************************************************************//**
 * @brief Global Variables
 *        - g_timer_finished: Indicates whether the timer has finished its operation.
 *        - g_sw_press: Indicates whether the button has been pressed.
 **********************************************************************************************************************/
static volatile bool g_timer_finished = false;  ///< Flag to manage timer completion state
static volatile bool g_sw_press = false;        ///< Flag to manage button press state

/*******************************************************************************************************************//**
 * @brief Pin Definitions
 *        - BUZZER_PIN: Pin assigned for controlling the buzzer.
 *        - BUTTON_PIN: Pin assigned for the user button.
 **********************************************************************************************************************/
#define BUZZER_PIN    (BSP_IO_PORT_04_PIN_09)  ///< Buzzer GPIO pin
#define BUTTON_PIN    (BSP_IO_PORT_04_PIN_02)  ///< User button GPIO pin

#endif /* SURFACE_MAPPING_MAIN_H_ */
