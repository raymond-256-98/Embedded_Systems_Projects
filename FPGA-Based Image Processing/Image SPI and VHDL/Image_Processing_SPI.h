/*
 * Image_Processing_SPI.h
 *
 *  Created on: 4 Feb 2024
 *      Author: sxh512
 */

#ifndef SRC_IMAGE_PROCESSING_SPI_H_
#define SRC_IMAGE_PROCESSING_SPI_H_

/************************************************* Type Definitions ************************************************/

// Defining the Dimensions of the Image
#define Image_Height 512
#define Image_Width  512

// Defining the Values for the Filter Size Selection
#define Three_By_Three 3
#define Five_By_Five   5
#define Seven_By_Seven 7

// Defining the Filter Size for a 3X3 Filter
#define Filter_Rows_Three    3
#define Filter_Columns_Three 3

// Defining the Filter Size for a 3X3 Filter
#define Filter_Rows_Five    5
#define Filter_Columns_Five 5

// Defining the Filter Size for a 3X3 Filter
#define Filter_Rows_Seven    7
#define Filter_Columns_Seven 7

// Defining the Values for the Filter Type Selection
#define Sharpen_Filter 1
#define Motion_Filter  2
#define Outline_Filter 3

// Defining the Image Size
#define Image_Size Image_Height * Image_Width

// Defining the Storage Addresses for the Input and Output Image
// The Images are stored in the below Addresses in the DDR
#define Image_Storage_Address     0x0000300000
#define New_Image_Storage_Address 0x0000500000

// Defining the Base Address of the Convolution Multiplier
#define MULTIPLIER_BASE_ADDRESS XPAR_CONVOLUTION_MULTIPLIER_0_S00_AXI_BASEADDR

/*******************************************************************************************************************/

/*********************************************** Variable Definitions **********************************************/

// Variable for the Loop Condition in the Case Statements
int iterator = 1;

// Variable to store the Filter Size Input
int Filter_Size;
// Variable to store the Filter Type Input
int Filter_Type;

// Array to Read the Input Image Data stored in the DDR
u8 ReadBuffer[Image_Size];

// Array to Store the Output Image Data stored in the DDR
// To send the STM32F303RE via SPI Interface
unsigned char WriteBuffer[Image_Size];

float brightness = 0.0;

float previous_brightness = 0.0;

volatile int SPI_TransferInProgress;
int SPI_Error_Count;

// Creating an Instance for the SPI Device
static XSpi SpiInstance;

// Creating an Instance for the Interrupt Controller
XScuGic IntcInstance;

/******************************************************************************************************************/

/*********************************************** Function Prototypes **********************************************/

// Function to perform Image Processing on an Image with a 3X3 Filter
void convolution(u8 *Input_Image, u8 *Output_Image, float kernel[Filter_Rows_Three][Filter_Columns_Three]);
// Function to perform Image Processing on an Image with a 5X5 Filter
void convolution_Five(u8 *Input_Image, u8 *Output_Image, float kernel[Filter_Rows_Five][Filter_Columns_Five]);
// Function to perform Image Processing on an Image with a 7X7 Filter
void convolution_Seven(u8 *Input_Image, u8 *Output_Image, float kernel[Filter_Rows_Seven][Filter_Columns_Seven]);
// Function to Initialise and Configure the SPI Device
void SPI_Initialisation(void);
// Function to Initialise and Configure the Interrupt Controller
void SPI_Interrupt_Initialisation(void);
// Function to Configure and run the SPI Device
void SPI_Configurations(void);
// Interrupt Handler Function to check if the Image Data is Transfered
void SpiIntrHandler(void *CallBackRef, u32 StatusEvent, u32 ByteCount);
// Function to Display the Values stored in the Read and Write Buffers
void display_buffers(void);
// Function to clear all the Data stored in the Read and Write Buffers
void clear_SPI_buffers(void);
// Function to Transfer the Output Image Data via SPI into the STM32F303RE
float read_current_brightness(XSpi *SpiInstance);
// Function to create delays. Input in the Millisecond Level
int sleep(unsigned int milli_seconds);

/******************************************************************************************************************/

#endif /* SRC_IMAGE_PROCESSING_SPI_H_ */
