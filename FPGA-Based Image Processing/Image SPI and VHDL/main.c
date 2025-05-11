/*
	PA5 - CLK
	PA6 - MISO
	PA7 - MOSI
	PA9 - Sub Select (SS)
*/

#include "stm32f303xe.h"
#include "SPI.h"

void device_setup(void);
void delay_ms(uint32_t time);
void EXTI1_IRQHandler(void);
void TIM4_IRQHandler(void);

int main(void)
{
	device_setup();
	
	// Transmit Sample data to the SPI Data Register
	uint8_t transmitted_data[100] = {0};
	// Receive the Output Image Data from the SPI Data Register
  uint8_t received_data[100];
  
	// Function to Setup the SPI Interface fo rthe STM32F303RE
	SPI_Initialisation();
  
	// Function to Receive Data from the Zedboard
  data_receiver(transmitted_data, received_data, 100);
	
	return 0;
}
	
	// Delay Function
	void delay_ms(uint32_t time)
	{
		volatile uint32_t a=0;
		//Loop "time" amount of times
		for(uint32_t i = 0; i < (10000*time); i++)
		{
			a = a+1;
		}
	}
	
	// Interrupt Handler Fucntion
	void EXTI1_IRQHandler(void)
	{
		//Check if interrupt status flag has been sent
		if(( EXTI->PR &  EXTI_PR_PR1))
		{
			RCC->AHBENR |= RCC_AHBENR_GPIOAEN;  // Enable the GPIO A clock in RCC register
			GPIOA->MODER |= (1UL << 18);
			GPIOA->MODER &= ~(1UL << 19);
			EXTI->PR |= (1UL << 1); // Write a '1' here to clear!
		}
	}
	
	// Timer Interrupt Handler
	void TIM4_IRQHandler(void) 
	{
		//Check if interrupt status flag has been sent
		if(TIM4->SR & TIM_SR_UIF)
		{ 
			TIM4->SR &= ~TIM_SR_UIF; //(1 << 4UL);
		}
	}

	//Function to enable CPU clock running at 36MHz
	//APB1 Clock is 36MHz, HSE Clock Domain is 72MHz
	void device_setup(void) 
	{
		//Enable HSE (High Speed External Clock) - 8MHz ext. clock
		RCC->CR |= (1UL << 16UL);
		//Wait for HSE clock to be stable
		while(!(RCC->CR & (1UL << 17UL))) {}

		//Configure PLL to upscale HSE to 36MHz

		//Set PLL prescaller to devide by 2 - 4MHz
		//Clear bits
		RCC->CFGR2 &= ~(3UL << 15UL);
		RCC->CFGR2 |= (1UL << 0UL);

		//Enable HSE as PLL source and multiply by 9 (9*4 = 36)
		RCC->CFGR |= (1UL << 16UL);
		RCC->CFGR |= (7UL << 18UL);

		//Enable PLL to active
		RCC->CR |= (1UL << 24UL);
		//Wait for the PLL clock to be stable
		while(!(RCC->CR & (1U << 25U))) {}

		//Select PLL as main clock source
		RCC->CFGR &= ~(3UL << 0UL);
		RCC->CFGR |= (2UL << 0UL);

		//Wait until system clock is stable
		while ((RCC->CFGR & (3UL << 2UL)) != (1UL << 3UL)) {}

		//Update system clock in system config
		SystemCoreClockUpdate();
	}
