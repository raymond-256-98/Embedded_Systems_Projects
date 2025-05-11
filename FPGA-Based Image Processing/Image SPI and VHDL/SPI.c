#include "stm32f303xe.h"
#include "SPI.h"

// Function to Setup the GPIO Pins for SPI1
void SPI_Pin_Setup(void)
{
	// Setting PA5, PA6 and PA7 to Alternate Function Mode	
	// PA5
	GPIOA->MODER |= (1U << 11);
	GPIOA->MODER &= ~(1U << 10);
	
	// PA6
	GPIOA->MODER |= (1U << 13);
	GPIOA->MODER &= ~(1U << 12);	
	
	// PA7
	GPIOA->MODER |= (1U << 15);
	GPIOA->MODER &= ~(1U << 14);	
	
	// Setting PA5, PA6 and PA7 as an Alternate Function Type to SPI1
	// PA5
	GPIOA->AFR[0] |= (1U << 20);
	GPIOA->AFR[0] &= ~(1U << 21);
	GPIOA->AFR[0] |= (1U << 22);
	GPIOA->AFR[0] &= ~(1U << 23);
	
	// PA6
	GPIOA->AFR[0] |= (1U << 24);
	GPIOA->AFR[0] &= ~(1U << 25);
	GPIOA->AFR[0] |= (1U << 26);
	GPIOA->AFR[0] &= ~(1U << 27);	
	
	// PA7
	GPIOA->AFR[0] |= (1U << 28);
	GPIOA->AFR[0] &= ~(1U << 29);
	GPIOA->AFR[0] |= (1U << 30);
	GPIOA->AFR[0] &= ~(1U << 31);	
}

// Function to Initialise the SPI Interface
 void SPI_Initialisation(void)
{
		// Enabling the Clock Access to the APB2 Bus
		RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
   
    // Function to Setup the GPIO Pins for SPI1
    SPI_Pin_Setup();
   
    // Function to Setup the Control Register to Configure the SPI Interface
    SPI_Control_Register_Setup();
}

// Function to Setup the Control Register to Configure the SPI Interface
void SPI_Control_Register_Setup(void)
{
	// Setting the fpclk value to fpclk/2
	// fpclk = 72 MHz
	SPI1->CR1 &= ~(1U<<3);
	SPI1->CR1 &= ~(1U<<4);
	SPI1->CR1 &= ~(1U<<5);

	// Setting the CPOL = 1 and CPHA = 1
	SPI1->CR1 |= SPI_CR1_CPOL;
	SPI1->CR1 |= SPI_CR1_CPHA;

	// Setting up the SPI in Receive Only Mode
	SPI1->CR1 |= SPI_CR1_RXONLY;
	
	// MSB First
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST;
	
	// Setting up the STM32F303RE as a Main
	SPI1->CR1 |= SPI_CR1_MSTR;
	
	// Enable 8-Bit Data format
	SPI1->CR2 |= (0x07 << SPI_CR2_DS_Pos);

	// Enable Software Sub Managment
	SPI1->CR1 |= SPI_CR1_SSM;
	// Internally Select the Sub
	SPI1->CR1 |= SPI_CR1_SSI;
	
	// Enable the SPI1 Module
	SPI1->CR1 |= SPI_CR1_SPE;
}

// Function to Receive the Output Image Data from the Zedboard
void data_receiver(uint8_t* transmitted_data, uint8_t* received_data, uint32_t size)
{
    for (uint32_t i = 0; i < size; ++i)
    {
			// Setting the Sub Select to Low
			GPIOA->ODR &= ~GPIO_ODR_9;
				
				// Transmitting temporary 8-Bit Data to the Data Register
        SPI1->DR = (uint16_t)(transmitted_data[i] << 8);
        
			 // Loop to Wait unitl the Data is Received
				while(!(SPI1-> SR & SPI_SR_RXNE));
 
        // Storing the Data from the Data Register into an Array
        received_data[i] = (uint8_t)SPI1->DR;

				// Setting the Sub Select to High
        GPIOA->ODR |= GPIO_ODR_9;      
    }         
}
