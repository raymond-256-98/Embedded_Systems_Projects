/*
 * ADC.c
 *
 *  Created on: Aug 7, 2024
 *      Author: Umesh
 */

#include "hal_data.h"
#include "common_utils.h"
#include "icu_ep.h"
#include "Surface_mapping_main.h"
#include "ADC.h"

/* Global variables to store magnetic field values for X, Y, and Z axes */
int Magnetic_Field_Value1[20] = {0};
int Magnetic_Field_Value2[20] = {0};
int Magnetic_Field_Value3[20] = {0};

/* Function to configure and initialize the ADC module */
void ADC_configure()
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize ADC */
    err = R_ADC_Open(&g_adc0_ctrl, &g_adc0_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("ADC Initialization Failed\r\n");
    }

    /* Configure ADC channels */
    err = R_ADC_ScanCfg(&g_adc0_ctrl, &g_adc0_channel_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("ADC Channel Configuration Failed\r\n");
    }
}

/* Function to read ADC values and store them in global arrays */
void ADC()
{
    uint16_t sensor_data_1;  // X-Axis sensor data
    uint16_t sensor_data_2;  // Y-Axis sensor data
    uint16_t sensor_data_3;  // Z-Axis sensor data

    int ADC_counter = 0;  // Counter to track the number of readings

    /* Loop to read ADC values 20 times */
    while (ADC_counter < 20)
    {
        /* Read sensor data from ADC Channel 4 (X-Axis) */
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_4, &sensor_data_1);
        APP_PRINT("Magnetic Sensor Data X-Axis Reading: %d\r\n", sensor_data_1);

        /* Read sensor data from ADC Channel 10 (Y-Axis) */
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_10, &sensor_data_2);
        APP_PRINT("Magnetic Sensor Data Y-Axis Reading: %d\r\n", sensor_data_2);

        /* Read sensor data from ADC Channel 17 (Z-Axis) */
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_17, &sensor_data_3);
        APP_PRINT("Magnetic Sensor Data Z-Axis Reading: %d\r\n", sensor_data_3);

        APP_PRINT("\n");

        /* Store the read values into global arrays */
        Magnetic_Field_Value1[ADC_counter] = sensor_data_1;
        Magnetic_Field_Value2[ADC_counter] = sensor_data_2;
        Magnetic_Field_Value3[ADC_counter] = sensor_data_3;

        /* Increment the counter and delay for next sampling */
        ADC_counter++;
        R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);  // Adjust delay for ADC sampling rate
    }

    /* Print completion messages */
    APP_PRINT("All 3 ADC Channels transmitted 20 ADC Values each!\r\n");
    APP_PRINT("All ADC Values Notified by the BLE Module!\r\n");
}

