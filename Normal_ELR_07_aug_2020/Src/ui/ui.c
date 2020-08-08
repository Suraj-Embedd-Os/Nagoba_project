#include "ui.h"
#include "setup.h"
#include "display.h"
#include "button.h"
#include "Rms.h"
#include "pt24xx.h"
#include "button_new.h"

/***************************************************
Macro Declaration
****************************************************/
#define ON (1U)
#define OFF (0U)

/******************************************************
Function Declaration
******************************************************/
uint8_t InV__( uint8_t flags_s );
void page_update (uint8_t page_row, uint8_t page_col);

/*****************************************************
Variable Declaration
******************************************************/
bool toggle_Sv_Mv = ON;
bool return_flag = OFF;
bool prev_tog_state = ON;
bool test = OFF;
bool min_max1 = true;
bool min_max2 = true;
bool intrpt_hndl;
bool flow_cntl_flag = ON;

uint8_t min_max_counter = 0,min_max2c = 0;
uint8_t cop_idex1 = 0,copy_index2 = 0;
uint8_t return_counter = 0;
uint8_t page_Cur_row = 0, page_Cur_col = 1;
uint8_t sv_mv_counter = 0;

uint16_t record_counter = 0;

uint32_t page,setting,setted;
uint32_t minValue = 0,maxValue = 0;

/*******************************************************
Extern Variables
*******************************************************/
extern bool relay_trip_status_1,relay_trip_status_2;	 
extern bool falt_disp;
extern bool __relay_test;



/*
Tasks:  Execution frequency = 2 KHz
	Switch segments (determine current segment number)
	Request data for current segment 
	Break 16-bit data value in bits and 
	Select current segment and Assign data bits to pins 

Input: 
	Data associated with current segment 
Output: 
	Select current segment (Set associated pin) 
	Assign bit value to associated pins
*/

/*

	Digit Lines:
	A0 : Pb15
	A1 : Pb14
	A2 : Pb13
	A3 : Pb12
	A4 : Pa8	
*/

void ui_init (void)
{	
	// Set default scaling factor
	display_setScalingFactor(DISP_SF_CUR);
}

GPIO_TypeDef* const DigitPort[DISP_NO_DIGI] = {

		GPIOB,GPIOB,GPIOB,GPIOB,GPIOA
};
const uint16_t DigitPin[DISP_NO_DIGI] = {

GPIO_PIN_15, GPIO_PIN_14, GPIO_PIN_13, GPIO_PIN_12, GPIO_PIN_8
};


void ui_periodicProcess( void )
{
	static uint32_t currDigit;

	/* Switch Off previous digit */
	DigitPort[currDigit]->BRR = (uint32_t)DigitPin[currDigit];
	
	/* Determine next digit */
	currDigit++;
	if(currDigit == DISP_NO_DIGI)
	{
		currDigit = 0;
	}
	
	/* Display data */
	display_flashData(currDigit);	
		
	
	/* Switch On current digit */
	DigitPort[currDigit]->BSRR = (uint32_t)DigitPin[currDigit];
	
}

//******************LAST RECORDS************//

const DISP_ID_PARAM pageParam[/*Page Number */] = { 
	DP_FAULT_1,
	DP_FAULT_2,
	DP_FAULT_3,
	DP_FAULT_4,
	DP_FAULT_5,
 };

const uint32_t pageIndicators[/*Page Number */] = { 
	 (DISP_IND_A |DISP_IND_1 ),
	 (DISP_IND_2 |DISP_IND_A ),
	 (DISP_IND_3 |DISP_IND_A ),
	 (DISP_IND_4 |DISP_IND_A ),
	 (DISP_IND_5 |DISP_IND_A ),
};

//*********************main Page*************************// 
DISP_ID_PARAM page_Wr_flgs[5][2] = {
{DP_CUR_LEAKAGE_T,	DP_TRIP_SET_VALUE	},
{DP_FAULT_VALUE,		DP_TEXT_FAULT			},
{DP_TEXT_CBCT,			DP_TEXT_OPEN			},
{DP_TEXT_ON,				DP_TEXT_OFF				},
{DP_TEST,						DP_TEXT_DONE	}
};


uint32_t pageupdIndicators[5][2] 		= { 
{(DISP_IND_A | DISP_IND_MV ),(DISP_IND_SV |	DISP_IND_A)},
{(DISP_IND_A)								,(0x00)},
{(0x00)											,(0x00)},
{(0x00)											,(0x00)},
{(0x00)											,(0x00)},
};



//********************min_max Ranges**************************//
uint16_t minValues[3] = {
300,
3000,
10000,
};
uint32_t maxValues[6] = {
3000,
30000,
80000,	
120000,
150000,
300000,	
};




/*
sets the MINIMUX and MAXIMUM values for
tripping values
*/
//void min_max( void )
//{ 
//	
//	display_setScalingFactor(DISP_SF_CUR);
//	
//if(min_max1 == false &&  min_max2 == false)	 
//{
//			HAL_NVIC_SystemReset();
//}

//	
//if(ui_isButtonLongPressed(BUTTON_ID_B3)){
//min_max_counter++;
//if(min_max_counter > 5)
//{	
//	minValue = minValues[copy_index2]/100;
//	display_rowNumeric(minValues[copy_index2],(DISP_IND_A|DISP_IND_SV)); 
//	PT24xx_write(0x60,(uint32_t *)&minValue,4);
//	setting_default();
//	min_max_counter = 0;
//	min_max1 = false;
//	}
//}	  
//if(ui_isButtonLongPressed(BUTTON_ID_B2))
//{
//	min_max2c++;
//if(min_max2c > 5)
//{		
//	maxValue = maxValues[cop_idex1]/100;	
//	display_rowNumeric(maxValues[cop_idex1],(DISP_IND_A|DISP_IND_SV)); 	
//	PT24xx_write(0x56,(uint32_t *)&maxValue,4);
//	setting_default();
//	min_max2c = 0;		
//	min_max2 = false;

//}
//}


//if(min_max2 && ui_isButtonReleased(BUTTON_ID_B2) == true)
//	 {
//		setted++;
//  	if(setted == 6)setted = 0;	 	
//		display_rowNumeric(maxValues[setted],DISP_IND_A); 
//		cop_idex1 = setted;
//		min_max2c = 0;		 
//}
// 

//if(min_max1 && ui_isButtonReleased(BUTTON_ID_B3) == true)
//{		
//	setting++;
//	if(setting == 3)setting = 0;
//	display_rowNumeric(minValues[setting],DISP_IND_A); 
//	min_max_counter = 0;
//	copy_index2 = setting;
//}
//}

/*
togle state read from EEPROM when 
power is turn on
*/
uint8_t check_bit2 = 0;

void mv_sv_read ( void )
{
	uint8_t check_bit1 = 1;
	PT24xx_read(0xB8,(uint32_t *)&check_bit2,sizeof(check_bit2));
	if(check_bit1 == check_bit2)
		{
			toggle_Sv_Mv = ON;
			prev_tog_state  = ON;
		}
		else
			{
				toggle_Sv_Mv = OFF;
				prev_tog_state  = OFF;
			}
}


/*
these function executed every 2 seconds

here interupts flags are reference, falt, relay_test, what ever condtion is toggle 
On or OFF when these flags are setted or resetted display will starts to toggle 
between interupts texts on Display with respect to the priority given by if and else if.

this function is executed when toogle is on, (if toggle_sv_mv glag is TRUE),
else processor will execute whem interupt flags are TRUE
*/
void interupt_state( void )
{
	intrpt_hndl = OFF;
	
if(refrence_flag && !test)
	{
		page_Cur_row = 2;
		intrpt_hndl = ON;
	}
	else if(falt_disp && !test)//falt occures
	{
		page_Cur_row = 1;
		intrpt_hndl = ON;
	}
	else if(__relay_test && !test)//relay test
	{
		page_Cur_row = 4;
		intrpt_hndl = ON;
	}	
	else if(toggle_Sv_Mv)// normal toggle of sv_mv
	{
		page_Cur_row = 0;
	}


	if(toggle_Sv_Mv || intrpt_hndl)
	{
	page_Cur_col = InV__(page_Cur_col);
	
	page_update(page_Cur_row,page_Cur_col);
	}

}


/*
this function will control the other two button action by checking where 
interupt flags are FALSE or not,
if all flags FALSE then last two BUTTONS will works 
*/
bool btn_state( void )
{
if(!refrence_flag && !falt_disp && !__relay_test)
	return ON;
else 
	return OFF;
}


/*
this function executed every 0.5 Seconds to update the page if toggle is OFF,
or last two BUTTONS PRESSED.
checks the toggle is OFF,
BUTTON 0 =  is pressed to change page from sv\mv.
BUTTON 1 = is pressed to show the last 5 RECORDS.
BOTH BUTTON = is pressed will system enter to setup 
*/
void ui_pageManager( void )
{
	if(toggle_Sv_Mv == OFF && flow_cntl_flag == ON && intrpt_hndl == OFF)
		{
			page_update(0,page_Cur_col);	
		}
		if( btn_state() )
			{
				if(test ||	ui_isButtonLongPressed(BUTTON_ID_B0))
					{
						if(!test )
							{
								page_Cur_col = InV__(page_Cur_col);	
							}
							record_counter = 0;
							sv_mv_counter++;
							if(test)
								{
									//setup_enter();
									sv_mv_counter = 0;	
									toggle_Sv_Mv = OFF;
									flow_cntl_flag = OFF;
								}
								if( ui_isButtonLongPressed(BUTTON_ID_B1) )
									{
										test = ON;
										toggle_Sv_Mv = OFF;
										flow_cntl_flag = OFF;
									}
					} 
		}
			/*BUTTON 1 for last trip RECORDS*/
		if( btn_state() )
			{
				if(ui_isButtonNormalPressed(BUTTON_ID_B1))
					{
						if(page == 5)
							{ 
								page= 0;
								if(prev_tog_state  == ON)
									toggle_Sv_Mv = ON;
								if(toggle_Sv_Mv == OFF)
									flow_cntl_flag = ON;
							}
							return_flag = ON;
							toggle_Sv_Mv = OFF;
							flow_cntl_flag = OFF;
							display_rowNumeric(read_value((RMS_READING_t)pageParam[page]),
							pageIndicators[page]);
							page++;	
							return_counter = 0;
					}
			}

/* to avoid the BUTTON 0 Debounce */
if(	sv_mv_counter > 4)
	{
		if(toggle_Sv_Mv)
			{
				toggle_Sv_Mv = OFF;
				page_update(3,1);
				check_bit2 = 0;
				prev_tog_state = OFF;
				PT24xx_write(0xB8,(uint32_t *)&check_bit2,sizeof(check_bit2));
			}
			else
				{
					toggle_Sv_Mv = ON;
					page_update(3,0);
					check_bit2 = 1;
					prev_tog_state = ON;
					PT24xx_write(0xB8,(uint32_t *)&check_bit2,sizeof(check_bit2));
				}
				sv_mv_counter  = 0;
	}
	else
		{
			record_counter++;
			if(record_counter > 2)
				{
					sv_mv_counter  = 0;
					record_counter = 0;
					ui_isButtonLongPressed(BUTTON_ID_B0);
					ui_isButtonLongPressed(BUTTON_ID_B1);
				}
				if(return_flag == ON)
					{
						return_counter++; 
					}
					if(return_counter > 8)
						{
							return_flag = OFF;
							return_counter = 0;	
							if(prev_tog_state  == ON)
								toggle_Sv_Mv = ON;							
							flow_cntl_flag = ON;

						}
		}
}

/*
this function will show the Current page Contents in Display.
*/

void page_update (uint8_t page_row, uint8_t page_col)
	{
		/* it checks the the input is TEXT or NUMBER */
		if(page_Wr_flgs[page_row][page_col] > DISP_ID_TOTAL_NUMBER_TEXT)
		{
			display_rowText(page_Wr_flgs[page_row][page_col],(pageupdIndicators[page_row][page_col] )
			);
		}
		else
		{
			display_rowNumeric(read_value((RMS_READING_t)page_Wr_flgs[page_row][page_col]),
			pageupdIndicators[page_row][page_col]);
		}
}


/*	
used to display number or text nor clear the screen, and function Inv is used 
to toggle the input
*/

void ui_display_clearRow ( DISP_ROW_ID row )
{
	display_clearRow( 0x0 );
}
void ui_display_numeric ( int32_t number )
{
	display_rowNumeric( number, 0x0);
}
void ui_display_text (  DISP_ID_PARAM id )
{
	display_rowText(  id, 0x0);
}
uint8_t InV__( uint8_t flags_s )
{
if(flags_s)
	return OFF;
else
	return ON;

}

/************************ (C) COPYRIGHT Nagoba Electronics *****END OF FILE****/

