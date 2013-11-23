#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "stm32f4xx_conf.h"
#include "FreeRTOS.h"
#include "list.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

// Private function prototypes
void blink1();
void blink2();
void blink3();
void blink3();
void vTaskPB();
void init();
void STM_EVAL_PBInit( void );

// Global variables
static xSemaphoreHandle xTestSemaphore = NULL;

int main(void) {
	init();
	STM_EVAL_PBInit();

	vSemaphoreCreateBinary(xTestSemaphore);

	xTaskCreate(blink1, (const signed char *)"LED1", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate(blink2, (const signed char *)"LED2", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate(blink3, (const signed char *)"LED3", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate(vTaskPB, (const signed char *)"PB_test", configMINIMAL_STACK_SIZE, NULL, 1, NULL );

	vTaskStartScheduler();

	for(;;) {

	}

	return 0;
}

void blink1() {
	for(;;) {
		GPIO_SetBits(GPIOD, GPIO_Pin_12);
		vTaskDelay(200);
		GPIO_ResetBits(GPIOD, GPIO_Pin_12);
		vTaskDelay(200);
	}
}

void blink2() {
	for(;;) {
		GPIO_SetBits(GPIOD, GPIO_Pin_13);
		vTaskDelay(500);
		GPIO_ResetBits(GPIOD, GPIO_Pin_13);
		vTaskDelay(500);
	}
}


void blink3() {
	for(;;) {
		GPIO_SetBits(GPIOD, GPIO_Pin_14);
		vTaskDelay(1000);
		GPIO_ResetBits(GPIOD, GPIO_Pin_14);
		vTaskDelay(1000);
	}
}

void vTaskPB( void ) {
	xSemaphoreTake( xTestSemaphore, 0 );
	for( ;; )
	{
		xSemaphoreTake( xTestSemaphore, portMAX_DELAY );
		GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
	}
}


void init() {
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DAC_InitTypeDef  DAC_InitStructure;

	// ---------- SysTick timer -------- //
	if (SysTick_Config(SystemCoreClock / 1000)) {
		// Capture error
		while (1){};
	}

	// GPIOD Periph clock enable
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// Configure PD12, PD13, PD14 and PD15 in output pushpull mode
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	// Configure PA1 (push button) in input mode
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// ------ UART ------ //

	// Clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// IO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	// Conf
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART2, &USART_InitStructure);

	// Enable
	USART_Cmd(USART2, ENABLE);


	// ---------- DAC ---------- //

	// Clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// Configuration
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	// IO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Enable DAC Channel1
	DAC_Cmd(DAC_Channel_1, ENABLE);

	// Set DAC Channel1 DHR12L register
	DAC_SetChannel1Data(DAC_Align_12b_R, 0);
}

/**
 *   @brief  Configures Button GPIO and EXTI Line.
 * @param  Button: Specifies the Button to be configured.
 *   This parameter can be one of following parameters:   
 *     @arg BUTTON_WAKEUP: Wakeup Push Button
 *     @arg BUTTON_TAMPER: Tamper Push Button  
 *     @arg BUTTON_KEY: Key Push Button 
 *     @arg BUTTON_RIGHT: Joystick Right Push Button 
 *     @arg BUTTON_LEFT: Joystick Left Push Button 
 *     @arg BUTTON_UP: Joystick Up Push Button 
 *     @arg BUTTON_DOWN: Joystick Down Push Button
 *     @arg BUTTON_SEL: Joystick Sel Push Button
 * @param  Button_Mode: Specifies Button mode.
 *   This parameter can be one of following parameters:   
 *     @arg BUTTON_MODE_GPIO: Button will be used as simple IO 
 *     @arg BUTTON_MODE_EXTI: Button will be connected to EXTI line with interrupt
 *                     generation capability  
 * @retval None
 */

void STM_EVAL_PBInit( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure Button pin as input */
	/*GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(BUTTON_PORT[Button], &GPIO_InitStructure);*/

	/* Connect Button EXTI Line to Button GPIO Pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

	/* Configure Button EXTI line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set Button EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
}

void EXTI0_IRQHandler( void ) {
	long lHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xTestSemaphore, &lHigherPriorityTaskWoken);
	EXTI_ClearITPendingBit( EXTI_Line0 );
	portEND_SWITCHING_ISR(lHigherPriorityTaskWoken);
}
