/*
 * ADC.h
 *
 *  Created on: Aug 7, 2024
 *      Author: Umesh
 */

#ifndef ADC_H_
#define ADC_H_

/* Function Prototypes */

/**
 * @brief Configures and initializes the ADC module.
 *
 * This function sets up the ADC channels and prepares the ADC for data collection.
 */
void ADC_configure();

/**
 * @brief Reads ADC values and stores them in global arrays.
 *
 * This function reads the ADC channels for the X, Y, and Z axes, processes the data,
 * and stores it in the global arrays for further use.
 */
void ADC();

/* Global Arrays to Store Magnetic Field Values */
extern int Magnetic_Field_Value1[20];  // X-Axis magnetic field values
extern int Magnetic_Field_Value2[20];  // Y-Axis magnetic field values
extern int Magnetic_Field_Value3[20];  // Z-Axis magnetic field values

#endif /* ADC_H_ */
