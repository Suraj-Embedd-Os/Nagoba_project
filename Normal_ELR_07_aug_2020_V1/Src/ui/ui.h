
/**
  ******************************************************************************
  * @file           : mfm_ui.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the ui.
  ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MFM_UI_H
#define __MFM_UI_H


#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>


#include "stm32f0xx_hal.h"
#include "display.h"
#include "button.h"
#include "disp_comn_vari.h"





void ui_init (void);
void ui_periodicProcess( void );
void ui_display_clearRow ( DISP_ROW_ID row );
void ui_display_numeric (  int32_t number );
void ui_display_text ( DISP_ID_PARAM id );
void ui_pageManager( void );
void min_max( void );
void mv_sv_read ( void );
void interupt_state( void );



#ifdef __cplusplus
}
#endif

#endif // __io_h

/************************ (C) COPYRIGHT Nagoba Electronics *****END OF FILE****/
