#include "stm32f3xx.h"                  // Device header	
#include <stdio.h>


char uart_data;
char * guardar;
volatile uint32_t msTicks = 0; 
volatile uint32_t msTicks2 = 0; 
char buf[7];
int temp = 0;
char nuevo = 0;
char escribiendo = 0;
char nLetra = 0;
char letra = 0;

//OpenLCD command characters
#define SPECIAL_COMMAND 254  //Magic number for sending a special command
#define SETTING_COMMAND 0x7C //124, |, the pipe character: The command to change settings: baud, lines, width, backlight, splash, etc

#define DISPLAY_ADDRESS1 0x72 //This is the default address of the OpenLCD
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYCONTROL 0x08

#define CLEAR_COMMAND 0x2D					//45, -, the dash character: command to clear and home the display
#define SET_RGB_COMMAND 0x2B				//43, +, the plus character: command to set backlight RGB value

#define LCD_SETDDRAMADDR 0x80

void clk_config(void) {
	RCC->CFGR |= 0xE<<18;
	FLASH->ACR |= 0x2;
  RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY));	
	// SW<-0x02 (PLL as System Clock), HCLK not divided, PPRE1<-0x4 (APB1 <- HCLK/2), APB2 not divided
	RCC->CFGR |= 0x402;
	while (!(RCC->CFGR & RCC_CFGR_SWS_PLL));
	SystemCoreClockUpdate();
}

void UsartInit(void){
	//RX. PA10
	//TX. PA9
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	//PA10 	
	GPIOA->MODER |= (2<<20);
	//PA9
	GPIOA->MODER |= (2<<18);
	GPIOA->AFR[1] |= (7<<4); //PA9 como TX
	GPIOA->AFR[1] |= (7<<8); //PA10 como RX
	//115200, 8, N, 1
	//Vel transmisión, 8 bits de datos, No paridad , 1 start bit
	//El Baud rate se calcula (velocidad de reloj)/(Velocidad de transmición) (64M / 115200 * 2) = 278
	USART1->BRR |= 278;
	USART1->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;
	USART1->CR1 |= USART_CR1_UE;
	NVIC_EnableIRQ(USART1_IRQn);
}

void PWM_SetDutyCycle(float dc_ms){
	if(dc_ms == 0)
		TIM3->CCR1 = 0;
	TIM3->CCR1 = dc_ms;
}


void USART1_Send(char c){
	//Buscar En USART_ISR. Si TXE está en 1 puedo transimit datos.4
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = c;
}

void USART1_putString(char* string){
	while(*string){
		USART1_Send(*string);
		string++;
	}
}

void USART1_IRQHandler(void){
	if(USART_ISR_NE){
		USART1->ICR = 0xFFFF;
	}
	
	if(USART_ISR_RXNE){
		if(uart_data != USART1->RDR){
			uart_data = USART1->RDR;
			nuevo = 1;
		}
	}
}

void PWM_Init(){
	float periodo = 0.0f;
	/*
	TIM3_CH1 AF2  // PB4
	*/
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	GPIOB->MODER &= ~(3<<8);
	GPIOB->MODER |= (2<<8);
	GPIOB->AFR[0] |= (2<<16);
	TIM3->PSC = 31; //8MHz/(31+1) = 2MHz
	TIM3->ARR = 1000; // Compare Value
	TIM3->CCR1 = 0; //DC 0%
	TIM3->CCMR1 |= (0b110<<4) | (1<<3);  //PWM Mode 1, Preload Enable
	TIM3->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN; //Counter enable, autoreload enable
	TIM3->EGR |= TIM_EGR_UG;  //Count reinit
	TIM3->CCER |= TIM_CCER_CC1E; //Polarity, CC1 Enable
	TIM3->BDTR |= TIM_BDTR_MOE; //Main output enable
}

void LDRInit(void){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	
	// PA2 como Entrada Analogica ADC1_IN3
	GPIOA->MODER |= (3<<4);
	
	// Conversiones continuas y 12 bits
	ADC1->CFGR |= ADC_CFGR_CONT + ADC_CFGR_OVRMOD; //+ ADC_CFGR_RES_0;
	// 1 conversión canal 3
	ADC1->SQR1 = 0;
	ADC1->SQR1 |= (3<<6);
	
	// HCLK/4
	ADC1_2_COMMON->CCR |= (3<<16);
	// Habilita interrupciones al finalizar una conversion
	ADC1->IER |= ADC_IER_EOC;
	// ADC enable
	ADC1->CR |= ADC_CR_ADEN + ADC_CR_ADSTART; 
	NVIC_EnableIRQ(ADC1_2_IRQn);
}

void ADC1_2_IRQHandler(void){
	temp = ADC1->DR;
}

uint8_t espera = 20;

void I2C_Write(char slave, char bytes){
	// AutoEND, Reload, 2 bytes
	I2C1->CR2 = 0x00;
	I2C1->CR2 = I2C_CR2_AUTOEND | (bytes<<16) | slave << 1 | I2C_CR2_START;
}

void I2C_Transmit(char data){
	while(!(I2C1->ISR& I2C_ISR_TXIS));
	I2C1->TXDR = data;
	while(!(I2C1->ISR& I2C_ISR_TXE));
}

void SysTick_Handler(void){
	if(nLetra == 0 || escribiendo == 1){
		msTicks++;
		escribiendo = 1;
		if(msTicks == espera + 4){
			USART1_putString("\r\n\r\n\r\n");
			USART1_putString("from machine import Pin, UART\r\n");
		}else if(msTicks == espera + 8){
			USART1_putString("import ujson, json, network, time\r\n");
		}else if(msTicks == espera + 12){
			USART1_putString("import urequests as requests\r\n");
		}else if(msTicks == espera + 16){
			USART1_putString("uart = UART(1, baudrate=115200)\r\n");
			USART1_putString("URLetra = 'http://www.mechanode.com.gt/PDAE/GetK8Letter.php?nLetra='\r\n");
		}else if(msTicks == espera + 20){
			USART1_putString("URLGet = 'http://www.mechanode.com.gt/PDAE/getK8Command.php'\r\n");
			USART1_putString("URLSend = 'http://www.mechanode.com.gt/PDAE/K8SendCommand.php?command='\r\n");
		}else if(msTicks == espera + 24){
			USART1_putString("ssid = 'Galaxy'\r\n");
			USART1_putString("gc.enable()\r\n");
		}else if(msTicks == espera + 28){
			USART1_putString("password = 'hola1234'\r\n");
		}else if(msTicks == espera + 32){
			USART1_putString("station = network.WLAN(network.STA_IF)\r\n");
		}else if(msTicks == espera + 36){
			USART1_putString("station.active(True)\r\n");
		}else if(msTicks == espera + 40){
			USART1_putString("station.connect(ssid, password)\r\n");
		}else if(msTicks == espera + 44){
			USART1_putString("uart.write('z')\r\n");
		}else if(msTicks == espera + 48){
			USART1_putString("response = requests.get(URLGet)\r\n");
		}else if(msTicks == espera + 52){
			USART1_putString("uart.write(response.text)\r\n");
			USART1_putString("response.close()\r\n");
		}else if(msTicks == espera + 56){
			USART1_putString("response = requests.get(URLSend + '");
			sprintf(buf, "%d\n", temp);
			USART1_putString(buf);
			USART1_putString("')\r\n");
		}else if(msTicks == espera + 60){
			USART1_putString("response.close()\r\n");
			USART1_putString("gc.collect()\r\n");
			msTicks = espera + 45;
			escribiendo = 0;
		}
	}else{
		msTicks2++;
		if(nLetra < 17){
			if(msTicks2 == 4){
				USART1_putString("response = requests.get(URLetra + '");
			}else if(msTicks2 == 6){
				sprintf(buf, "%d\n", nLetra);
				USART1_putString(buf);
			}else if(msTicks2 == 8){
				USART1_putString("')\r\n");
			}else if(msTicks2 == 10){
				USART1_putString("uart.write(response.text)\r\n");
				USART1_putString("response.close()\r\n");
			}else if(msTicks2 == 14){
				/* begin */
				I2C1->CR2 = 0x00;
				I2C1->CR2 = I2C_CR2_AUTOEND | (1<<16) | DISPLAY_ADDRESS1 << 1 | I2C_CR2_START;
				/* transmit */
				while(!(I2C1->ISR& I2C_ISR_TXIS));
				I2C1->TXDR = uart_data;
				while(!(I2C1->ISR& I2C_ISR_TXE));
				msTicks2 = 2;
				uart_data = ' ';
				nLetra++;
			}
		}else{
			if(msTicks2 == 10)
				nLetra = 0;
				uart_data = '&';
		}
	}
}

void I2C_Config(void){
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	GPIOB->MODER |= (2<<12) | (2<<14);
	GPIOB->AFR[0] |= (4<<24) | (4<<28);
	
	// fbus = (64MHz/2), fi2c = 400kHz 
	I2C1->TIMINGR |= (0x2<<8) | (0x0B<<0);
	// fbus = 8MHz , fi2c = 100kHz 
	//I2C1->TIMINGR |= (1<<28) | (0x4<<20) | (0x2<<16) |(0xf<<8) | (0x13<<0);
	I2C1->CR1 |= I2C_CR1_PE;
}


int main(void){
	
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN + RCC_AHBENR_ADC12EN;
	GPIOB->MODER &= ~(3<<6);
	GPIOB->MODER |= (1<<6);
	GPIOB->MODER &= ~(3<<10);
	GPIOB->MODER |= (1<<10);
	
	GPIOB->MODER &= ~(3<<8);
	GPIOB->MODER |= (1<<8);
	clk_config();
	
	SysTick_Config(SystemCoreClock/4); // Interrupt each 250 ms
	
	UsartInit();
	LDRInit();
	PWM_Init();
	I2C_Config();
	
	while(1){
		if(nuevo){
			nuevo = 0;
			if(nLetra == 0){
				switch(uart_data){
					case 'z' : GPIOB->ODR |= 1<<3; break;
					case 0x6C : GPIOB->ODR &= ~(1<<3); break;
					case 'a' : PWM_SetDutyCycle(0); break;
					case 'b' : PWM_SetDutyCycle(50); break;
					case 'c' : PWM_SetDutyCycle(150); break;
					case 'd' : PWM_SetDutyCycle(300); break;
					case 'e' : PWM_SetDutyCycle(600); break;
					case 'f' : PWM_SetDutyCycle(1000); break;
					case 'r' : 
						I2C_Write(DISPLAY_ADDRESS1, 5);
						I2C_Transmit(SETTING_COMMAND);
						I2C_Transmit(SET_RGB_COMMAND); 	
						I2C_Transmit(255);
						I2C_Transmit(0); 	
						I2C_Transmit(0); 	
						break;
					case 'g' : 
						I2C_Write(DISPLAY_ADDRESS1, 5);
						I2C_Transmit(SETTING_COMMAND);
						I2C_Transmit(SET_RGB_COMMAND); 	
						I2C_Transmit(0);
						I2C_Transmit(255); 	
						I2C_Transmit(0); 	
						break;
					case 'v' : 
						I2C_Write(DISPLAY_ADDRESS1, 5);
						I2C_Transmit(SETTING_COMMAND);
						I2C_Transmit(SET_RGB_COMMAND); 	
						I2C_Transmit(0);
						I2C_Transmit(0); 	
						I2C_Transmit(255); 	
						break;
					case 'w' : 
						I2C_Write(DISPLAY_ADDRESS1, 5);
						I2C_Transmit(SETTING_COMMAND);
						I2C_Transmit(SET_RGB_COMMAND); 	
						I2C_Transmit(255);
						I2C_Transmit(255); 	
						I2C_Transmit(255); 	
						break;
					case 'o' : 
						I2C_Write(DISPLAY_ADDRESS1, 5);
						I2C_Transmit(SETTING_COMMAND);
						I2C_Transmit(SET_RGB_COMMAND); 	
						I2C_Transmit(0);
						I2C_Transmit(0); 	
						I2C_Transmit(0); 	
						break;
					case '&':
						nLetra = 1;
						msTicks2 = 0;
						//int row_offsets[] = {0x00, 0x40, 0x14, 0x54}
						I2C_Write(DISPLAY_ADDRESS1, 2);
						I2C_Transmit(SPECIAL_COMMAND | 0 + 0); 	
						I2C_Transmit(LCD_SETDDRAMADDR | 0 + 0); 	
						break;
				}
			}
		}
	}
}
