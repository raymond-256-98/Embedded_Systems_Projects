/*
 * Digital_output.h
 *
 *  Created on: Aug 7, 2024
 *      Author: Umesh
 */

#ifndef DIGITAL_OUTPUT_H_
#define DIGITAL_OUTPUT_H_

/**
 * @brief Configures the GPIO pin for the buzzer.
 *
 * This function initializes the GPIO pin associated with the buzzer,
 * setting it as an output.
 */
void Buzzer_Configure();

/**
 * @brief Controls the buzzer output.
 *
 * This function manages the buzzer's behavior, such as turning it on or off,
 * depending on the application's requirements.
 */
void Buzzer();

#endif /* DIGITAL_OUTPUT_H_ */
