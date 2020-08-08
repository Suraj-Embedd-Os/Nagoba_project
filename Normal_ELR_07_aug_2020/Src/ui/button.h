
/**
  ******************************************************************************
  * @file           : MB85RCxx.h
  * @brief          : Header for MB85RCxx.c file.
  *                   This file contains the common defines of the stoarge.
  ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BUTTON_H
#define __BUTTON_H


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "disp_comn_vari.h"
#include "stm32f0xx_hal.h"

#ifdef __cplusplus
 extern "C" {
#endif
	 
/* Button Configiurations
*/

#define NO_OF_BUTTONS		(2)



/* Push Buttons
*/
typedef enum {
	
	BUTTON_ID_B0=0,
	BUTTON_ID_B1,
//	BUTTON_ID_B2,
//	BUTTON_ID_B3,
	
	BUTTON_TOTAL,
	
} BUTTON_ID;


void button_scanEvent ( void );
void reset_btn( void );


bool ui_isButtonNormalPressed( BUTTON_ID btn );
bool ui_isButtonLongPressed( BUTTON_ID btn );
bool ui_isButtonReleased( BUTTON_ID btn );


		
#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_H */

/************************ (C) COPYRIGHT Nagoba Electronics *****END OF FILE****/
