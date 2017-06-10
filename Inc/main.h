#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_audio.h"
#include <stdio.h>
#include "stm32f4xx_it.h"
#include "waveplayer.h"

#include "waverecorder.h"
 
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment this define to disable repeat feature */
/* #define PLAY_REPEAT_DISABLED */

/* Defines for the Audio used commands */
#define CMD_PLAY           ((uint32_t)0x00)
#define CMD_RECORD         ((uint32_t)0x01)
#define CMD_STOP           ((uint32_t)0x02)

/* Defines for LEDs lighting */
#define LED3_TOGGLE      0x03  /* Toggle LED3 */
#define LED4_TOGGLE      0x04  /* Toggle LED4 */
#define LED6_TOGGLE      0x06  /* Toggle LED6 */
#define LEDS_OFF         0x07  /* Turn OFF all LEDs */
#define STOP_TOGGLE      0x00  /* Stop LED Toggling */
                                                                                    
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Error_Handler(void);

/* Definition for USARTx clock resources */
#define USARTx                           USART2
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE();
#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE() 

#define USARTx_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_2
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF7_USART2
#define USARTx_RX_PIN                    GPIO_PIN_3
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_AF                     GPIO_AF7_USART2

/* Definition for USARTx's DMA */
#define USARTx_TX_DMA_CHANNEL            DMA_CHANNEL_4
#define USARTx_TX_DMA_STREAM             DMA1_Stream6
#define USARTx_RX_DMA_CHANNEL            DMA_CHANNEL_4
#define USARTx_RX_DMA_STREAM             DMA1_Stream5

/* Definition for USARTx's NVIC */
#define USARTx_DMA_TX_IRQn               DMA1_Stream6_IRQn
#define USARTx_DMA_RX_IRQn               DMA1_Stream5_IRQn
#define USARTx_DMA_TX_IRQHandler         DMA1_Stream6_IRQHandler
#define USARTx_DMA_RX_IRQHandler         DMA1_Stream5_IRQHandler
#define USARTx_IRQn                      USART2_IRQn
#define USARTx_IRQHandler                USART2_IRQHandler

/* Size of Transmission buffer */
#define TXBUFFERSIZE                     (COUNTOF(aTxBuffer) - 1)
/* Size of Reception buffer */
#define RXBUFFERSIZE                     TXBUFFERSIZE
  
/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
