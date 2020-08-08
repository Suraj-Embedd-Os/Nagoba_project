#ifndef __SETUP_H
#define __SETUP_H


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f0xx_hal.h"
#include "default.h"

#ifdef __cplusplus
 extern "C" {
#endif
	 
/* System Configurations
*/	 
typedef struct SYSTEMCONFIG
{	
	uint32_t check_bit;
	uint32_t trip_value;	
	uint32_t set_delay;
	uint32_t set_alarm_value;
	uint32_t set_alarm;
	uint32_t address;
	uint32_t buad_rate;
	uint32_t parity;
	uint32_t end_bit;
 } cfg_t;
 

 
extern cfg_t cfg;
	


void setup_init (void);
void setup_enter (void);
void setting_default( void );

#ifdef __cplusplus
}
#endif

#endif /* __SETUP_H */

/************************ (C) COPYRIGHT Nagoba Electronics *****END OF FILE****/
