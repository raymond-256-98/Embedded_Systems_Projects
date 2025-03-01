/*
 * Set_Reset_GPIO.c
 *
 *  Created on: 7 August 2024
 *      Author: Amith Raj Nataraj
 */

#include <RTT/SEGGER_RTT.h>
#include "hal_data.h"
#include "Set_Reset_GPIO.h"

/**
 * @brief Generates a SET pulse to initialize the sensor.
 */
void set_pulse(void)
{
    // SET pin goes HIGH
    R_IOPORT_PinWrite(&g_ioport_ctrl, SET_PIN, BSP_IO_LEVEL_HIGH);
    // Maintain HIGH for 5 microseconds
    R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MICROSECONDS);
    // SET pin goes LOW
    R_IOPORT_PinWrite(&g_ioport_ctrl, SET_PIN, BSP_IO_LEVEL_LOW);
}

/**
 * @brief Generates a RESET pulse to reinitialize the sensor.
 */
void reset_pulse(void)
{
    // RESET pin goes HIGH
    R_IOPORT_PinWrite(&g_ioport_ctrl, RESET_PIN, BSP_IO_LEVEL_HIGH);
    // Maintain HIGH for 5 microseconds
    R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MICROSECONDS);
    // RESET pin goes LOW
    R_IOPORT_PinWrite(&g_ioport_ctrl, RESET_PIN, BSP_IO_LEVEL_LOW);
}

/**
 * @brief Performs calibration and filtering for magnetic field data from the sensor.
 */
void set_reset_filtering(void)
{
    // Initialize ADC
    R_ADC_Open(&g_adc0_ctrl, &g_adc0_cfg);
    R_ADC_ScanCfg(&g_adc0_ctrl, &g_adc0_channel_cfg);
    R_ADC_ScanStart(&g_adc0_ctrl);

    // Calibrate the sensor by averaging initial readings
    uint32_t sum_x = 0, sum_y = 0, sum_z = 0;

    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        // Read X, Y, Z-axis data
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_10, &sensor_data_2); // X-Axis
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_17, &sensor_data_3); // Y-Axis
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_4, &sensor_data_1);  // Z-Axis

        // Accumulate the values for averaging
        sum_x += sensor_data_2;
        sum_y += sensor_data_3;
        sum_z += sensor_data_1;

        // Delay between samples
        R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);
    }

    // Calculate the baseline values for X, Y, and Z axes
    basemfd_x_axis = sum_x / NUM_SAMPLES;
    basemfd_y_axis = sum_y / NUM_SAMPLES;
    basemfd_z_axis = sum_z / NUM_SAMPLES;

    // Display calibration results
    SEGGER_RTT_printf(0, "Calibration Complete: X=%d, Y=%d, Z=%d\n\r", basemfd_x_axis, basemfd_y_axis, basemfd_z_axis);

    // Continuous monitoring loop
    while (1)
    {
        // Read ADC values for each axis
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_10, &sensor_data_2); // X-Axis
        SEGGER_RTT_printf(0, "X-Axis: %d\r", sensor_data_2);
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_17, &sensor_data_3); // Y-Axis
        SEGGER_RTT_printf(1, "Y-Axis: %d\r", sensor_data_3);
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_4, &sensor_data_1);  // Z-Axis
        SEGGER_RTT_printf(2, "Z-Axis: %d\r", sensor_data_1);

        // Compensate for baseline values
        calculated_x_axis = sensor_data_2 - basemfd_x_axis;
        calculated_y_axis = sensor_data_3 - basemfd_y_axis;
        calculated_z_axis = sensor_data_1 - basemfd_z_axis;

        // Check for significant changes on each axis
        if (abs(calculated_x_axis) > threshold)
        {
            SEGGER_RTT_printf(0, "Significant Change detected on X-Axis: %d\r", calculated_x_axis);
        }
        else
        {
            SEGGER_RTT_printf(0, "X-Axis: %d\r", calculated_x_axis);
        }

        if (abs(calculated_y_axis) > threshold)
        {
            SEGGER_RTT_printf(0, "Significant Change detected on Y-Axis: %d\r", calculated_y_axis);
        }
        else
        {
            SEGGER_RTT_printf(0, "Y-Axis: %d\r", calculated_y_axis);
        }

        if (abs(calculated_z_axis) > threshold)
        {
            SEGGER_RTT_printf(0, "Significant Change detected on Z-Axis: %d\n\r", calculated_z_axis);
        }
        else
        {
            SEGGER_RTT_printf(0, "Z-Axis: %d\n\r", calculated_z_axis);
        }

        // Trigger the SET pulse
        set_pulse();

        // Wait before triggering RESET
        R_BSP_SoftwareDelay(99000, BSP_DELAY_UNITS_MICROSECONDS); // 99 ms delay to create 1% duty cycle

        // Trigger the RESET pulse
        reset_pulse();

        // Delay before the next cycle
        R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
    }
}
