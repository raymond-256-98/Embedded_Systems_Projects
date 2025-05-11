#include "stm32f303xe.h"

void SPI_Initialisation(void);
void SPI_Pin_Setup(void);
void SPI_Control_Register_Setup(void);
void data_receiver(uint8_t* transmitted_data, uint8_t* received_data, uint32_t size);
