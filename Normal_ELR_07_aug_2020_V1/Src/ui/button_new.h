
/**
  ******************************************************************************
  * @file           : mfm_ui.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the ui.
  ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BUTTON_NEW_H
#define __BUTTON_NEW_H


#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>
#include "disp_comn_vari.h"
#include "stm32f0xx_hal.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>


/* Push Buttons
*/
typedef enum {
	
	BUTTON_B0=0,
	BUTTON_B1,
	BUTTON_B2,
	BUTTON_B3,
	
	BUTTON_TOT,
	
} BUTTON_IDs;


bool normal_press_1( void );
bool normal_press_2( void );
bool normal_press_3( void );
bool normal_press_4( void );
bool long_press_1( void );
bool long_press_2( void );








#ifdef __cplusplus
}
#endif

#endif // __io_h

/************************ (C) COPYRIGHT Nagoba Electronics *****END OF FILE****/
