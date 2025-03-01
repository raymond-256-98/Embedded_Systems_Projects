/*
 * Set_Reset_GPIO.h
 *
 *  Created on: 7 August 2024
 *      Author: Amith Raj Nataraj
 */

#ifndef SET_RESET_GPIO_H_
#define SET_RESET_GPIO_H_

// Number of samples for calibration
#define NUM_SAMPLES 20

// Global variables for storing sensor data and baseline magnetic field values
uint16_t sensor_data_1, sensor_data_2, sensor_data_3;    // Raw ADC values for X, Y, Z axes
int16_t basemfd_x_axis = 0, basemfd_y_axis = 0, basemfd_z_axis = 0; // Baseline magnetic field values

// Threshold for detecting significant changes in the magnetic field
uint16_t threshold = 46;  // Corresponds to a 0.01 Gauss change

// Variables for storing calculated magnetic field differences from baseline
int16_t calculated_x_axis, calculated_y_axis, calculated_z_axis;

// Function prototypes
void set_pulse(void);              // Function to generate a SET pulse
void reset_pulse(void);            // Function to generate a RESET pulse
void set_reset_filtering(void);    // Function to perform SET/RESET operation and filtering

#endif /* SET_RESET_GPIO_H_ */
