/*
 * Digital_input.h
 *
 *  Created on: Aug 7, 2024
 *      Author: Umesh
 */

#ifndef DIGITAL_INPUT_H_
#define DIGITAL_INPUT_H_

/**
 * @brief Function to manage digital input and application state.
 *
 * This function handles the button press, toggles the application running state,
 * and initiates ADC scanning if the application is running.
 */
void Digital_input();

/**
 * @brief Flag to manage the application's running state.
 *
 * This flag is set to `true` when the application is running and `false` when it is stopped.
 */
static volatile bool g_app_running = false; // Flag to manage application state

#endif /* DIGITAL_INPUT_H_ */
