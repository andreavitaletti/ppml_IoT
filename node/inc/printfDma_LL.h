#ifndef __PRINTFDMA_LL_H_
#define __PRINTFDMA_LL_H_

#include "main.h"

/* Definition for PRINTFDMA_USARTx clock resources */
#define PRINTFDMA_USARTx                           USART2
#define PRINTFDMA_USARTx_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE();
#define PRINTFDMA_DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define PRINTFDMA_USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define PRINTFDMA_USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE() 

#define PRINTFDMA_USARTx_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define PRINTFDMA_USARTx_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()

/* Definition for PRINTFDMA_USARTx Pins */
#define PRINTFDMA_USARTx_TX_PIN                    GPIO_PIN_2
#define PRINTFDMA_USARTx_TX_GPIO_PORT              GPIOA
#define PRINTFDMA_USARTx_TX_AF                     GPIO_AF7_USART2
#define PRINTFDMA_USARTx_RX_PIN                    GPIO_PIN_15
#define PRINTFDMA_USARTx_RX_GPIO_PORT              GPIOA
#define PRINTFDMA_USARTx_RX_AF                     GPIO_AF3_USART2

/* Definition for PRINTFDMA_USARTx's DMA */
#define PRINTFDMA_USARTx_TX_DMA_CHANNEL            DMA1_Channel7
#define PRINTFDMA_USARTx_TX_DMA_REQUEST            DMA_REQUEST_2
#define PRINTFDMA_USARTx_RX_DMA_CHANNEL            DMA1_Channel6
#define PRINTFDMA_USARTx_RX_DMA_REQUEST            DMA_REQUEST_2

/* Definition for PRINTFDMA_USARTx's NVIC */
#define PRINTFDMA_USARTx_DMA_TX_IRQn               DMA1_Channel7_IRQn
#define PRINTFDMA_USARTx_DMA_RX_IRQn               DMA1_Channel6_IRQn
#define PRINTFDMA_USARTx_DMA_TX_IRQHandler         DMA1_Channel7_IRQHandler
#define PRINTFDMA_USARTx_DMA_RX_IRQHandler         DMA1_Channel6_IRQHandler
#define PRINTFDMA_USARTx_IRQn                      USART2_IRQn
#define PRINTFDMA_USARTx_IRQHandler                USART2_IRQHandler

/* Definition for PRINTF_DMA_TIMx clock resources */
#define PRINTFDMA_TIMx                           	TIM2
#define PRINTFDMA_TIMx_CLK_ENABLE()              	__HAL_RCC_TIM2_CLK_ENABLE()

/* Definition for PRINTF_DMA_TIMx's NVIC */
#define PRINTFDMA_TIMx_IRQn                      	TIM2_IRQn
#define PRINTFDMA_TIMx_IRQHandler                	TIM2_IRQHandler

/* Size of Trasmission buffer */
#ifndef PRINTFDMA_TXBUFFERSIZE
#define PRINTFDMA_TXBUFFERSIZE                      (1024)
#endif
/* Size of Reception buffer */
#ifndef PRINTFDMA_RXBUFFERSIZE
#define PRINTFDMA_RXBUFFERSIZE                      (1024)
#endif

#endif
