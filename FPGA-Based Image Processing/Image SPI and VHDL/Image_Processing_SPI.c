#include <stdio.h>
#include "xil_printf.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xil_cache.h"
#include "xil_exception.h"
#include "platform.h"
#include "xparameters.h"
#include "xscugic.h"
#include "xspi.h"
#include "Convolution_Multiplier.h"
#include "Image_Processing_SPI.h"

// Assigning the Input Image Values to the below Address
u8 *Input_Image = (u8 *)Image_Storage_Address;
// Storing the Output Image Values to the below Address
u8 *Output_Image = (u8 *)New_Image_Storage_Address;

// Defining the Storage Addresses for the Input and Output Image
// The Images are stored in the below Addresses in the DDR
#define Image_Storage_Address     0x0000300000
#define New_Image_Storage_Address 0x0000500000

// Function to perform 2-D Convolution Operation on an Input Image for a 3X3 Kernel
void convolution(u8 *Input_Image, u8 *Output_Image, float kernel[Filter_Rows_Three][Filter_Columns_Three])
{
	// Calculating the New Dimensions of the Output Image After Convolution
    int New_Width = Image_Width - (Filter_Columns_Three - 1);
    int New_Height = Image_Height - (Filter_Rows_Three - 1);
    int y, x, Kernel_Rows, Kernel_Columns;

    // Line Buffer to store a Three Lines of the Image for Efficient Processing
    u8 line_buffer[Filter_Columns_Three][Image_Width];

    // Initialising the Line Buffer with the First Three Rows of the Input Image
    for (Kernel_Rows = 0; Kernel_Rows < Filter_Rows_Three; Kernel_Rows++)
    {
        for (Kernel_Columns = 0; Kernel_Columns < Image_Width; Kernel_Columns++)
        {
            line_buffer[Kernel_Rows][Kernel_Columns] = Input_Image[(Kernel_Rows * Image_Width) + Kernel_Columns];
        }
    }

    // Iterating over each Row of the Input Image
    for (y = 0; y < New_Height; y++)
    {
    	// Updating the Line Buffer with a New Row of the Input Image by Shifting the Existing Rows upwards
        for (Kernel_Rows = 0; Kernel_Rows < Filter_Rows_Three - 1; Kernel_Rows++)
        {
            for (Kernel_Columns = 0; Kernel_Columns < Image_Width; Kernel_Columns++)
            {
                line_buffer[Kernel_Rows][Kernel_Columns] = line_buffer[Kernel_Rows + 1][Kernel_Columns];
            }
        }

        // Reading a New Row into Line Buffer from the Input Image
        for (Kernel_Columns = 0; Kernel_Columns < Image_Width; Kernel_Columns++)
        {
            line_buffer[Filter_Rows_Three - 1][Kernel_Columns] = Input_Image[((y + Filter_Rows_Three - 1) * Image_Width) + Kernel_Columns];
        }

        // Iterating over each Column of the Input Image
        for (x = 0; x < New_Width; x++)
        {
        	// Variable to Accumulate the Sum after Multiplication of the Image Data and Kernel Coefficients
            float Sum = 0;

            // Iterating over each Row of the Kernel
            for (Kernel_Rows = 0; Kernel_Rows < Filter_Rows_Three; Kernel_Rows++)
            {
            	// Iterating over each Column of the Kernel
                for (Kernel_Columns = 0; Kernel_Columns < Filter_Columns_Three; Kernel_Columns++)
                {
                	// Procuring the Current Kernel Value and the corresponding Image Pixel Value from the Line Buffer
                    float Kernel_Value = kernel[Kernel_Rows][Kernel_Columns];
                    float Image_Value  = line_buffer[Kernel_Rows][x + Kernel_Columns];

                    // Performing the Multiplication Operation using the Hardware Multiplier
                    // Writing the Kernel Value into Slave Register - 1
                    CONVOLUTION_MULTIPLIER_mWriteReg(MULTIPLIER_BASE_ADDRESS, CONVOLUTION_MULTIPLIER_S00_AXI_SLV_REG1_OFFSET, Kernel_Value);
                    // Writing the Image Pixel Value into Slave Register - 0
                    CONVOLUTION_MULTIPLIER_mWriteReg(MULTIPLIER_BASE_ADDRESS, CONVOLUTION_MULTIPLIER_S00_AXI_SLV_REG0_OFFSET, Image_Value);

                    // Storing the Product of the Kernel Value and the Image Pixel Value into a Variable
                    // The Product is stored in Slave Register - 2. The below Function reads Product Value from that Register
                    u8 Product = CONVOLUTION_MULTIPLIER_mReadReg(MULTIPLIER_BASE_ADDRESS, CONVOLUTION_MULTIPLIER_S00_AXI_SLV_REG2_OFFSET);

                    // Accumulating the Sum of the Products over each Iteration to eventually obtain the Convoluted Value
                    Sum += Product;
                }
            }
            // Limiting the Convoluted Value to fit within the 8-Bit Range
            Sum = Sum > 255 ? 255 : Sum < 0 ? 0 : Sum;
            // Storing the Convoluted Value into the Output Image Array
            Output_Image[(y + (Filter_Rows_Three / 2)) * Image_Width + (x + (Filter_Columns_Three / 2))] = (u8)Sum;
        }
    }
}

// Function to perform 2-D Convolution Operation on an Input Image for a 5X5 Kernel
void convolution_Five(u8 *Input_Image, u8 *Output_Image, float kernel[Filter_Rows_Five][Filter_Columns_Five])
{
	// Calculating the New Dimensions of the Output Image After Convolution
    int New_Width = Image_Width - (Filter_Columns_Five - 1);
    int New_Height = Image_Height - (Filter_Rows_Five - 1);
    int y, x, Kernel_Rows, Kernel_Columns;

    // Line Buffer to store a Five Lines of the Image for Efficient Processing
    u8 line_buffer[Filter_Columns_Five][Image_Width];

    // Initialising the Line Buffer with the First Five Rows of the Input Image
    for (Kernel_Rows = 0; Kernel_Rows < Filter_Rows_Five; Kernel_Rows++)
    {
        for (Kernel_Columns = 0; Kernel_Columns < Image_Width; Kernel_Columns++)
        {
            line_buffer[Kernel_Rows][Kernel_Columns] = Input_Image[(Kernel_Rows * Image_Width) + Kernel_Columns];
        }
    }

    // Iterating over each Row of the Input Image
    for (y = 0; y < New_Height; y++)
    {
    	// Updating the Line Buffer with a New Row of the Input Image by Shifting the Existing Rows upwards
        for (Kernel_Rows = 0; Kernel_Rows < Filter_Rows_Five - 1; Kernel_Rows++)
        {
            for (Kernel_Columns = 0; Kernel_Columns < Image_Width; Kernel_Columns++)
            {
                line_buffer[Kernel_Rows][Kernel_Columns] = line_buffer[Kernel_Rows + 1][Kernel_Columns];
            }
        }

        // Reading a New Row into Line Buffer from the Input Image
        for (Kernel_Columns = 0; Kernel_Columns < Image_Width; Kernel_Columns++)
        {
            line_buffer[Filter_Rows_Five - 1][Kernel_Columns] = Input_Image[((y + Filter_Rows_Five - 1) * Image_Width) + Kernel_Columns];
        }

        // Iterating over each Column of the Input Image
        for (x = 0; x < New_Width; x++)
        {
        	// Variable to Accumulate the Sum after Multiplication of the Image Data and Kernel Coefficients
            float Sum = 0;

            // Iterating over each Row of the Kernel
            for (Kernel_Rows = 0; Kernel_Rows < Filter_Rows_Five; Kernel_Rows++)
            {
            	// Iterating over each Column of the Kernel
                for (Kernel_Columns = 0; Kernel_Columns < Filter_Columns_Five; Kernel_Columns++)
                {
                	// Procuring the Current Kernel Value and the corresponding Image Pixel Value from the Line Buffer
                    float Kernel_Value = kernel[Kernel_Rows][Kernel_Columns];
                    float Image_Value  = line_buffer[Kernel_Rows][x + Kernel_Columns];

                    // Performing the Multiplication Operation using the Hardware Multiplier
                    // Writing the Kernel Value into Slave Register - 1
                    CONVOLUTION_MULTIPLIER_mWriteReg(MULTIPLIER_BASE_ADDRESS, CONVOLUTION_MULTIPLIER_S00_AXI_SLV_REG1_OFFSET, Kernel_Value);
                    // Writing the Image Pixel Value into Slave Register - 0
                    CONVOLUTION_MULTIPLIER_mWriteReg(MULTIPLIER_BASE_ADDRESS, CONVOLUTION_MULTIPLIER_S00_AXI_SLV_REG0_OFFSET, Image_Value);

                    // Storing the Product of the Kernel Value and the Image Pixel Value into a Variable
                    // The Product is stored in Slave Register - 2. The below Function reads Product Value from that Register
                    u8 Product = CONVOLUTION_MULTIPLIER_mReadReg(MULTIPLIER_BASE_ADDRESS, CONVOLUTION_MULTIPLIER_S00_AXI_SLV_REG2_OFFSET);

                    // Accumulating the Sum of the Products over each Iteration to eventually obtain the Convoluted Value
                    Sum += Product;
                }
            }
            // Limiting the Convoluted Value to fit within the 8-Bit Range
            Sum = Sum > 255 ? 255 : Sum < 0 ? 0 : Sum;
            // Storing the Convoluted Value into the Output Image Array
            Output_Image[(y + (Filter_Rows_Five / 2)) * Image_Width + (x + (Filter_Columns_Five / 2))] = (u8)Sum;
        }
    }
}

// Function to perform 2-D Convolution Operation on an Input Image for a 7X7 Kernel
void convolution_Seven(u8 *Input_Image, u8 *Output_Image, float kernel[Filter_Rows_Seven][Filter_Columns_Seven])
{
	// Calculating the New Dimensions of the Output Image After Convolution
    int New_Width = Image_Width - (Filter_Columns_Seven - 1);
    int New_Height = Image_Height - (Filter_Rows_Seven - 1);
    int y, x, Kernel_Rows, Kernel_Columns;

    // Line Buffer to store a Seven Lines of the Image for Efficient Processing
    u8 line_buffer[Filter_Columns_Seven][Image_Width];

    // Initialising the Line Buffer with the First Seven Rows of the Input Image
    for (Kernel_Rows = 0; Kernel_Rows < Filter_Rows_Seven; Kernel_Rows++)
    {
        for (Kernel_Columns = 0; Kernel_Columns < Image_Width; Kernel_Columns++)
        {
            line_buffer[Kernel_Rows][Kernel_Columns] = Input_Image[(Kernel_Rows * Image_Width) + Kernel_Columns];
        }
    }

    // Iterating over each Row of the Input Image
    for (y = 0; y < New_Height; y++)
    {
    	// Updating the Line Buffer with a New Row of the Input Image by Shifting the Existing Rows upwards
    	for (Kernel_Rows = 0; Kernel_Rows < Filter_Rows_Seven - 1; Kernel_Rows++)
        {
            for (Kernel_Columns = 0; Kernel_Columns < Image_Width; Kernel_Columns++)
            {
                line_buffer[Kernel_Rows][Kernel_Columns] = line_buffer[Kernel_Rows + 1][Kernel_Columns];
            }
        }

    	// Reading a New Row into Line Buffer from the Input Image
        for (Kernel_Columns = 0; Kernel_Columns < Image_Width; Kernel_Columns++)
        {
            line_buffer[Filter_Rows_Seven - 1][Kernel_Columns] = Input_Image[((y + Filter_Rows_Seven - 1) * Image_Width) + Kernel_Columns];
        }

        // Iterating over each Column of the Input Image
        for (x = 0; x < New_Width; x++)
        {
        	// Variable to Accumulate the Sum after Multiplication of the Image Data and Kernel Coefficients
            float Sum = 0;

            // Iterating over each Row of the Kernel
            for (Kernel_Rows = 0; Kernel_Rows < Filter_Rows_Seven; Kernel_Rows++)
            {
            	// Iterating over each Column of the Kernel
                for (Kernel_Columns = 0; Kernel_Columns < Filter_Columns_Seven; Kernel_Columns++)
                {
                	// Procuring the Current Kernel Value and the corresponding Image Pixel Value from the Line Buffer
                    float Kernel_Value = kernel[Kernel_Rows][Kernel_Columns];
                    float Image_Value  = line_buffer[Kernel_Rows][x + Kernel_Columns];

                    // Performing the Multiplication Operation using the Hardware Multiplier
                    // Writing the Kernel Value into Slave Register - 1
                    CONVOLUTION_MULTIPLIER_mWriteReg(MULTIPLIER_BASE_ADDRESS, CONVOLUTION_MULTIPLIER_S00_AXI_SLV_REG1_OFFSET, Kernel_Value);
                    // Writing the Image Pixel Value into Slave Register - 0
                    CONVOLUTION_MULTIPLIER_mWriteReg(MULTIPLIER_BASE_ADDRESS, CONVOLUTION_MULTIPLIER_S00_AXI_SLV_REG0_OFFSET, Image_Value);

                    // Storing the Product of the Kernel Value and the Image Pixel Value into a Variable
                    // The Product is stored in Slave Register - 2. The below Function reads Product Value from that Register
                    u8 Product = CONVOLUTION_MULTIPLIER_mReadReg(MULTIPLIER_BASE_ADDRESS, CONVOLUTION_MULTIPLIER_S00_AXI_SLV_REG2_OFFSET);

                    // Accumulating the Sum of the Products over each Iteration to eventually obtain the Convoluted Value
                    Sum += Product;
                }
            }
            // Limiting the Convoluted Value to fit within the 8-Bit Range
            Sum = Sum > 255 ? 255 : Sum < 0 ? 0 : Sum;
            // Storing the Convoluted Value into the Output Image Array
            Output_Image[(y + (Filter_Rows_Seven / 2)) * Image_Width + (x + (Filter_Columns_Seven / 2))] = (u8)Sum;
        }
    }
}

// Function to Initialise the SPI Peripheral
void SPI_Initialisation()
{
	int Status;
	XSpi_Config *SPI_ConfigPtr;
	SPI_ConfigPtr = XSpi_LookupConfig(XPAR_PS7_QSPI_0_DEVICE_ID);
	if (SPI_ConfigPtr == NULL)
			return XST_DEVICE_NOT_FOUND;

	Status = XSpi_CfgInitialize(&SpiInstance, SPI_ConfigPtr, SPI_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
}

// Function to Initialise the Interrupt Controller for SPI
void SPI_Interrupt_Initialisation()
{
	int Status;
	XScuGic_Config *IntcConfig;

	IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);

	if (NULL == IntcConfig)
		return XST_FAILURE;

	Status = XScuGic_CfgInitialize(&IntcInstance, IntcConfig, IntcConfig->CpuBaseAddress);

	if (Status != XST_SUCCESS)
		return XST_FAILURE;
}

// Function to Configure SPI Settings and Perform Tests
void SPI_Configurations()
{
	int Status;
	XSpi_Config *SPI_ConfigPtr;
	XScuGic_Config *IntcConfig;
	// Initialise the SPI driver so that it is ready to use.
		SPI_ConfigPtr = XSpi_LookupConfig(XPAR_PS7_QSPI_0_DEVICE_ID);
		if (SPI_ConfigPtr == NULL) { return XST_DEVICE_NOT_FOUND; }
		Status = XSpi_CfgInitialize(&SpiInstance, SPI_ConfigPtr, SPI_ConfigPtr->BaseAddress);
		if (Status != XST_SUCCESS) { return XST_FAILURE; }

		// Reset the SPI peripheral
		XSpi_Reset(&SpiInstance);

		// Initialise the Interrupt controller so that it is ready to use.
		IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
		if (NULL == IntcConfig) { return XST_FAILURE; }
		Status = XScuGic_CfgInitialize(&IntcInstance, IntcConfig, IntcConfig->CpuBaseAddress);
		if (Status != XST_SUCCESS) { return XST_FAILURE; }

		// Initialise exceptions on the ARM processor
		Xil_ExceptionInit();

		// Connect the interrupt controller interrupt handler to the hardware interrupt handling logic in the processor.
		Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, &IntcInstance);


		// Connect a device driver handler that will be called when an interrupt
		// for the device occurs, the device driver handler performs the
		// specific interrupt processing for the device.
		Status = XScuGic_Connect(&IntcInstance, XPAR_FABRIC_AXI_QUAD_SPI_0_IP2INTC_IRPT_INTR, (Xil_ExceptionHandler)XSpi_InterruptHandler, (void *)&SpiInstance);
		if (Status != XST_SUCCESS) { return Status; }


		// Enable the interrupt for the SPI peripheral.
		XScuGic_Enable(&IntcInstance, XPAR_FABRIC_AXI_QUAD_SPI_0_IP2INTC_IRPT_INTR);

		// Enable interrupts in the Processor.
		Xil_ExceptionEnable();


		// Perform a self-test to ensure that the hardware was built correctly.
		Status = XSpi_SelfTest(&SpiInstance);
		if (Status != XST_SUCCESS) { return XST_FAILURE; }

		printf("Digilent ALS PMOD test\n\r\n\r");

		// Run loopback test only in case of standard SPI mode.
		if (SpiInstance.SpiMode != XSP_STANDARD_MODE) { return XST_SUCCESS; }

		// Setup the handler for the SPI that will be called from the interrupt
		// context when an SPI status occurs, specify a pointer to the SPI
		// driver instance as the callback reference so the handler is able to
		// access the instance data.
		XSpi_SetStatusHandler(&SpiInstance, &SpiInstance, (XSpi_StatusHandler)SpiIntrHandler);


		// Set the SPI device to the correct mode for this application
		printf("Setting the SPI device into Main mode...");
		Status = XSpi_SetOptions(&SpiInstance, XSP_MASTER_OPTION + XSP_MANUAL_SSELECT_OPTION + XSP_CLK_PHASE_1_OPTION);
		if (Status != XST_SUCCESS) { return XST_FAILURE; }
		printf("DONE.\n\r");

		// Select the SPI Sub.  This asserts the correct SS bit on the SPI bus
		XSpi_SetSlaveSelect(&SpiInstance, 0x01);


		// Start the SPI driver so that interrupts and the device are enabled.
		printf("Starting the SPI driver, enabling interrupts and the device...");
		XSpi_Start(&SpiInstance);
		printf("DONE.\n\r");

		printf("\n\r\n\r");

		// Clear the SPI read and write buffers
		clear_SPI_buffers();

		// If you were using a more complex SPI device that requires configuration
		// data, then this might happen here ..


		// An endless loop which reads and displays the current brightness
		while(1) {
			brightness = read_current_brightness(&SpiInstance);

			// Check to see if the brightness is different from the last reading.
			// Only update the display on the UART if it is different.
			if (previous_brightness != brightness) {
				printf("Brightness = %3.4f\n\r", brightness);
				previous_brightness = brightness;
			}
		}


		// Disable and disconnect the interrupt system.
		XScuGic_Disconnect(&IntcInstance, XPAR_FABRIC_AXI_QUAD_SPI_0_IP2INTC_IRPT_INTR);

		return XST_SUCCESS;
}

// Handler for SPI Interrupts
void SpiIntrHandler(void *CallBackRef, u32 StatusEvent, u32 ByteCount)
{
	if (StatusEvent == XST_SPI_TRANSFER_DONE)
		SPI_TransferInProgress = FALSE;
	else
	{
		printf("\n\r\n\r ** SPI ERROR **\n\r\n\r");
		SPI_Error_Count++;
	}
}

// Display the SPI Read and Write Buffers
void display_buffers(void)
{
	int i;

	printf("----------------------------------------------------------\n\r");
	for(i=0; i<Image_Size; i++)
	{
		printf("Index 0x%02X  -->  Write = 0x%02X  |  Read = 0x%02X\n\r", i, WriteBuffer[i], ReadBuffer[i]);
	}
	printf("----------------------------------------------------------\n\r");
}

// Clear SPI Read and Write Buffers
void clear_SPI_buffers(void)
{
	int SPI_Count;

	// Initialising the Write Buffer and Read Buffer to Zero
	for (SPI_Count = 0; SPI_Count < Image_Size; SPI_Count++)
	{
		WriteBuffer[SPI_Count] = 0;
		ReadBuffer[SPI_Count] = 0;
	}
}

float read_current_brightness(XSpi *SpiInstance)
{
	u8 Brightness_LSB = 0;
	u8 Brightness_MSB = 0;
	float Brightness_float = 0;
	int Status;

	// Clear the SPI read and write buffers
	clear_SPI_buffers();

	// Put the commands (if needed) for the SPI device in the write buffer
	for( int i=0 ; i<=Image_Size; i++)
	{
		WriteBuffer[i] = Output_Image[i];
		printf("Output Image Pixel Data %d is Transmitted\n\r", i);
		SPI_TransferInProgress = TRUE;
		Status = XSpi_Transfer(SpiInstance, WriteBuffer, NULL, 1);
		while(SPI_TransferInProgress) {

		};
		sleep (10000);
	}

	Brightness_MSB = ReadBuffer[0] << 8;
	Brightness_LSB = ReadBuffer[1];

	// Extract the brightness value from data received
	// and account for the 4-bit offset
	Brightness_float = ( Brightness_MSB + Brightness_LSB ) >> 4;

	// When debugging display buffer content
	// display_buffers();

	// Clear the SPI read and write buffers
	clear_SPI_buffers();

	return Brightness_float;
}

// Function to provide a Software Delay
int sleep( unsigned int milli_seconds )
{
	// A long int is needed here, because we need to
	// count up to very high numbers!
	unsigned long int wcount = 0;
	for (wcount=0; wcount<(milli_seconds*1000); ++wcount)
	{
		// This is embedded assembler code that makes
		// the processor sleep for one clock cycle
		asm ("nop");
	}
}

int main(void)
{
	// Statements to obtain the Filter Size from the User
    printf("Enter the Size of the Filter:\n");
    printf("Enter 3 for a 3X3 Filter:\n");
    printf("Enter 5 for a 5X5 Filter:\n");
    printf("Enter 7 for a 7X7 Filter:\n");

    scanf("%d\n\r", &Filter_Size);

    printf("Your Selected Filter Size is: %d\n\r", Filter_Size);

	// Statements to obtain the Filter Type from the User for a 3X3 Filter
	printf("What Type of Filter do you want?\n");
	printf("Enter 1 for a Sharpen Filter\n");
	printf("Enter 2 for a Motion Filter\n");
	printf("Enter 3 for a Outline Filter\n");
	printf("Enter any number for an Identity Filter\n");

	scanf("%d\n\r",&Filter_Type);
	printf("The Filter Type is : %d\n", Filter_Type);

    switch (Filter_Size)
    {
    	case Three_By_Three:

			   switch (Filter_Type)
			   {
				   case Sharpen_Filter:
					   while(iterator > 0)
					   {
						   // Filter Coefficients for a Sharpen Filter for a 3X3 Kernel
						   float kernel[Filter_Rows_Three][Filter_Columns_Three] =
						   {
								   { 0, -1,  0},
								   {-1,  5, -1},
								   { 0, -1,  0}
						   };
						   convolution((u8 *)Input_Image, (u8 *)Output_Image, kernel);
						   iterator--;
					   }
					   iterator = 1;
				   break;

				   case Motion_Filter:
					   while(iterator > 0)
					   {
						   // Filter Coefficients for a Motion Blur Filter for a 3X3 Kernel
						   float kernel[Filter_Rows_Three][Filter_Columns_Three] =
						   {
								   {1, 0, 0},
								   {0, 1, 0},
								   {0, 0, 1}
						   };
						   convolution((u8 *)Input_Image, (u8 *)Output_Image, kernel);
						   iterator--;
					   }
					   iterator = 1;
					break;

					case Outline_Filter:
						while(iterator > 0)
						{
							// Filter Coefficients for an Outline Filter for a 3X3 Kernel
							float kernel[Filter_Rows_Three][Filter_Columns_Three] =
							{
									{-1, -1, -1},
									{-1,  8, -1},
									{-1, -1, -1}
							};
							convolution((u8 *)Input_Image, (u8 *)Output_Image, kernel);
							iterator--;
						}
						iterator = 1;
					break;

					default:
						while(iterator > 0)
						{
							// Filter Coefficients for an Identity Filter for a 3X3 Kernel
							float kernel[Filter_Rows_Three][Filter_Columns_Three] =
							{
									{0, 0, 0},
									{0, 1, 0},
									{0, 0, 0}
							};
							convolution((u8 *)Input_Image, (u8 *)Output_Image, kernel);
							iterator--;
						}
						iterator = 1;
				 }
			   break;

	   case Five_By_Five:

		   switch (Filter_Type)
		   {
			   case Sharpen_Filter:

				   while(iterator > 0)
				   {
					   // Filter Coefficients for a Sharpen Filter for a 5X5 Kernel
					   float kernel[Filter_Rows_Five][Filter_Columns_Five] =
					   {
							   {-1, -1, -1, -1, -1},
							   {-1, -1, -1, -1, -1},
							   {-1, -1, 24, -1, -1},
							   {-1, -1, -1, -1, -1},
							   {-1, -1, -1, -1, -1}
					   };
					   convolution_Five((u8 *)Input_Image, (u8 *)Output_Image, kernel);
					   iterator--;
				   }
				   iterator = 1;
				break;

			   case Motion_Filter:
				   while(iterator > 0)
				   {
					   // Filter Coefficients for a Motion Blur Filter for a 5X5 Kernel
					   float kernel[Filter_Rows_Five][Filter_Columns_Five] =
					   {
							   {1, 0, 0, 0, 0},
							   {0, 1, 0, 0, 0},
							   {0, 0, 1, 0, 0},
							   {0, 0, 0, 1, 0},
							   {0, 0, 0, 0, 1}
					   };
					   convolution_Five((u8 *)Input_Image, (u8 *)Output_Image, kernel);
					   iterator--;
				   }
				   iterator = 1;
				break;

			   case Outline_Filter:
				   while(iterator > 0)
				   {
					   // Filter Coefficients for an Outline Filter for a 5X5 Kernel
					   float kernel[Filter_Rows_Five][Filter_Columns_Five] =
					   {
							   {-1, -1, -1, -1, -1},
							   {-1,  0,  0,  0, -1},
							   {-1,  0, 24,  0, -1},
							   {-1,  0,  0,  0, -1},
							   {-1, -1, -1, -1, -1}
					   };
					   convolution_Five((u8 *)Input_Image, (u8 *)Output_Image, kernel);
					   iterator--;
				   }
				   iterator = 1;
				break;

			   default:
				   while(iterator > 0)
				   {
					   // Filter Coefficients for an Identity Filter for a 5X5 Kernel
					   float kernel[Filter_Rows_Five][Filter_Columns_Five] =
					   {
							   {0, 0, 0, 0, 0},
							   {0, 0, 0, 0, 0},
							   {0, 0, 1, 0, 0},
							   {0, 0, 0, 0, 0},
							   {0, 0, 0, 0, 0}
					   };
					   convolution_Five((u8 *)Input_Image, (u8 *)Output_Image, kernel);
					   iterator--;
				   }
				   iterator = 1;
			 }

		   break;

	   case Seven_By_Seven:

		   switch (Filter_Type)
		   {
			   case Sharpen_Filter:
				   while(iterator > 0)
				   {
					   // Filter Coefficients for a Sharpen Filter for a 7X7 Kernel
					   float kernel[Filter_Rows_Seven][Filter_Rows_Seven] =
					   {
							   { 0, -1, -1, -1, -1, -1,  0},
							   {-1,  2,  2,  2,  2,  2, -1},
							   {-1,  2,  4,  4,  4,  2, -1},
							   {-1,  2,  4, 12,  4,  2, -1},
							   {-1,  2,  4,  4,  4,  2, -1},
							   {-1,  2,  2,  2,  2,  2, -1},
							   { 0, -1, -1, -1, -1, -1,  0}
					   };
					   convolution_Seven((u8 *)Input_Image, (u8 *)Output_Image, kernel);
					   iterator--;
				   }
				   iterator = 1;
				break;

				case Motion_Filter:
				   while(iterator > 0)
				   {
					   // Filter Coefficients for a Motion Blur Filter for a 7X7 Kernel
					   float kernel[Filter_Rows_Seven][Filter_Rows_Seven] =
					   {
							   {1,0,0,0,0,0,1},
							   {0,1,0,0,0,1,0},
							   {0,0,1,0,1,0,0},
							   {0,0,0,1,0,0,0},
							   {0,0,1,0,1,0,0},
							   {0,1,0,0,0,1,0},
							   {1,0,0,0,0,0,1}
					   };
					   convolution_Seven((u8 *)Input_Image, (u8 *)Output_Image, kernel);
					   iterator--;
				   }
				   iterator = 1;
				break;

				case Outline_Filter:
					while(iterator > 0)
					{
						// Filter Coefficients for an Outline Filter for a 7X7 Kernel
						float kernel[Filter_Rows_Seven][Filter_Rows_Seven] =
						{
								{0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 1, 0, 0, 0},
								{0, 0, 1,-3, 1, 0, 0},
								{0, 0, 0, 1, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0}
						};
						convolution_Seven((u8 *)Input_Image, (u8 *)Output_Image, kernel);
						iterator--;
					}
					iterator = 1;
				break;

				default:
					while(iterator > 0)
					{
						// Filter Coefficients for an Identity Filter for a 7X7 Kernel
						float kernel[Filter_Rows_Seven][Filter_Rows_Seven] =
						{
								{0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0},
								{0,0,0,1,0,0,0},
								{0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0}
						};
						convolution_Seven((u8 *)Input_Image, (u8 *)Output_Image, kernel);
						iterator--;
					}
					iterator = 1;
		   }

	   break;

		 default:
			 // Default Statement for the Wrong Filter Choice
			 printf("Sorry! Not a valid Filter Number\n\r");
	     break;
    }

//    // Function to Initialise and Configure the SPI Device
//	SPI_Initialisation();
//
//	// Resetting the Entire SPI Peripheral
//	XSpi_Reset(&SpiInstance);
//
//	// Function to Initialise and Configure the Interrupt Controller
//	SPI_Interrupt_Initialisation();
//
//	// Initialising the Exceptions on the ARM Processor
//	Xil_ExceptionInit();

	// Function to Configure and run the SPI Peripheral
	SPI_Configurations();
}
