#ifndef __pt24xx_H
#define __pt24xx_H


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "stm32f0xx_hal.h"

#ifdef __cplusplus
 extern "C" {
#endif

void  PT24xx_init (void *i2cPort, const uint8_t devAddr );
int  PT24xx_read( 	 uint16_t byte_address, 
								 uint32_t *data, 
								 int32_t length );
int  PT24xx_write(  uint16_t byte_address, 
										 uint32_t *data, 
										 int32_t length );
		
#ifdef __cplusplus
}
#endif

#endif /* __PT24xxxx_H */
										



