#ifndef __serialcomm_h
#define __serialcomm_h


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "stm32f0xx_hal.h"

#ifdef __cplusplus
 extern "C" {
#endif


#define DE_PORT	(GPIOA)
#define DE_PIN	(GPIO_PIN_12)	 
	 
	 
void sercomm_init ( void *uartPort,
										void *uartRxDMAChannel );
void sercomm_transmit( const uint8_t *txData, const uint32_t txSize );
void sercomm_receive( uint8_t *rxData, const uint32_t rxSize );
uint32_t sercomm_getRxCount ( void );
void sercomm_resetRxDmaChannel ( void );	 
#ifdef __cplusplus
}
#endif

#endif // __serialcomm_h


