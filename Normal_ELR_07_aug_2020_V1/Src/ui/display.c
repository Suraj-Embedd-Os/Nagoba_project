/******************************************************************************
ELR Display Routines
File Name: display.c
File Descrption: Contains source code for handling display activities which 
is divided in following three layers:
1. Page Management Layer
2. Segment Data Layer
3. Hardware Abstraction Layer
Note: See Header file for function's description
//suh7Shm@
******************************************************************************/
#include "display.h"
#include "button.h"
#include "Rms.h"
#include "ui.h"


/******************************************************************************
@note Macro Declaration
******************************************************************************/
#define INDI_90 (DISP_IND_EE | DISP_IND_DD | DISP_IND_CC | DISP_IND_BB | DISP_IND_AA )
#define INDI_70 (DISP_IND_DD | DISP_IND_CC | DISP_IND_BB | DISP_IND_AA)
#define INDI_50 (DISP_IND_CC | DISP_IND_BB | DISP_IND_AA)
#define INDI_30 (DISP_IND_BB | DISP_IND_AA)
#define INDI_10 (DISP_IND_AA)


/******************************************************************************
@note Static and extern Declaration
******************************************************************************/
static uint8_t DISP_m_IND_DIGITS = 2+_CFG_DECIMAL_SCALER_DIGIT;
static uint8_t DISP_K_IND_DIGITS = 3+_CFG_DECIMAL_SCALER_DIGIT;

extern bool test;
extern bool mili_flag;
extern bool testing_flag;
extern bool falt_disp;
extern bool __relay_test;

/******************************************************************************
@note Variable Declaration 
******************************************************************************/
uint8_t bar_percent;
uint32_t pageVarIndicatorStatus;
const uint8_t *currDP;
bool setup_status = false;
uint32_t bridge_digit = 0;


/******************************************************************************
Data Lines:
	A : PB3
	B : PA15
	C : PF7
	D : PF6
	E : PA1
	F : PA11
	G : PA8
	H(DP) : PB15
	A12(a) : PB14
	A13(b) : PB13
******************************************************************************/
GPIO_TypeDef* const DataPort[] = {
	GPIOB, GPIOB, GPIOB, GPIOB, GPIOB,
	GPIOB, GPIOB, GPIOC, GPIOC, GPIOC, GPIOB
};
const uint16_t DataPin[] = {
	GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, 
	GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15, GPIO_PIN_1
};



/******************************************************************************
decimal point position in the order of number of digits 
******************************************************************************/
const uint8_t dpPos[DISP_TOTAL_SF][13] = {
		{	0,0,0,0,0,2,0,0,0,0,0,0,0},
		{ 2,2,2,3,2,3,3,2,1,3,2,1,0},//
		{ 2,1,1,1,1,3,2,2,2,3,2,1,0},//leakage current
		{ 2,2,2,2,2,2,2,2,2,3,2,1,0},//setup values
		{	0,0,0,0,0,0,0,0,0,0,0,0,0},
}; /* Max. 12 digits in 32-bit */

const uint8_t sf_digits[DISP_TOTAL_SF]= {
		_CFG_SF_BDR,
		_CFG_DECIMAL_SCALER_DIGIT,
		_CFG_SF_LEAKAGE_CURRENT,
		_CFG_SF_LEAKAGE_CURRENT,
		_CFG_SF_LEAKAGE_CURRENT,
	};

const uint32_t indRowBitMask[DISP_NO_DIGI] = {
		( DISP_IND_5 |  DISP_IND_A  | DISP_IND_EE | DISP_IND_Pr),
		( DISP_IND_4 |  DISP_IND_V  | DISP_IND_DD ),
		( DISP_IND_3 |	DISP_IND_M  | DISP_IND_CC ),
		( DISP_IND_2 |  DISP_IND_6  | DISP_IND_BB ),
 		( DISP_IND_MV |  DISP_IND_MINUS | DISP_IND_k | DISP_IND_m | DISP_IND_S | DISP_IND_R1 | DISP_IND_R2 |
     DISP_IND_PK | DISP_IND_1 |  DISP_IND_SV	| DISP_IND_AA),
};

const uint8_t indRowBitPos[DISP_NO_DIGI] = {
		0,4,7,10,13
};

const uint8_t digBCDMap[] = {	
		0x3F,0x06,0x5B,0x4F,0x66,
		0x6D,0x7D,0x07,0x7F,0x6F
};

static uint16_t flashData[DISP_NO_DIGI] = { 
							/* Initial display message */
						0x00,0x7c,0x7d,0x54,0x00 						
};


/*
Input:	Any integral number and
				Digit array address
Output: Number of digits in given number, 4 or more	
Useful for putting decimal point
* 123456 ->[1][2][3][4]
* 12345 ->[1][2][3][4]
* 1234 ->[1][2][3][4]
* 123 ->[0][1][2][3]
* 12 ->[0][0][1][2]
* 1 ->[0][0][0][1]

Hardcoded for  6-digit,DISP_DIGIT_SIZE,
can be made configurable but will add more arithmetic operations 
Note: making this function generic for handling any number of digits will 
waste cpu cycle, if those extra digits are not needed at all.
*/

uint32_t _convertDecimalToDigitArr ( uint32_t number, uint8_t *digArr )
{
	uint32_t	digCnt = 0;
	uint32_t	num;
	
	/* Convert given number to 4-digit number, if it is */
	while(number > 9999)
	{
		number /= 10;	// discard least significant digits 
		digCnt++;
	}
	num = number;
	while(num > 0)
	{
		num /= 10;	// discard least significant digits 
		digCnt++;
	}

	
	/* Break 4-digit number to digits and store in given byte array location */
	if( number > 999 )
	{
		digArr[3] = (uint8_t)(number/1000);	// 3456/1000=3
		number -= (uint32_t)(1000*digArr[3]);	// 3456-3000=456
	}
	else
	{
		digArr[3] = 0;
	}
	
	if( number > 99 )
	{
		digArr[2] = (uint8_t)(number/100);	// 456/100=4
		number -= (uint32_t)(100*digArr[2]);	// 456-400=34
	}
	else
	{
		digArr[2] = 0;
	}
	
	if( number > 9 )
	{
		digArr[1] = (uint8_t)(number/10);	// 56/10=5
		number -= (uint32_t)(10*digArr[1]);	//	56-50=6
	}
	else
	{
		digArr[1] = 0;
	}
	
	digArr[0] = (uint8_t)number;	//	6
	
	return digCnt;
}


uint32_t _convertDtoArr ( uint32_t number, uint8_t *digArr )
{
	uint32_t	digCnt = 0;
	uint32_t	num;
	
	/* Convert given number to 4-digit number, if it is */
	while(number > 999)
	{
		number /= 10;	// discard least significant digits 
		digCnt++;
	}
	num = number;
	while(num > 0)
	{
		num /= 10;	// discard least significant digits 
		digCnt++;
	}

	
	if( number > 99 )
	{
		digArr[2] = (uint8_t)(number/100);	// 456/100=4
		number -= (uint32_t)(100*digArr[2]);	// 456-400=34
	}
	else
	{
		digArr[2] = 0;
	}
	
	if( number > 9 )
	{
		digArr[1] = (uint8_t)(number/10);	// 56/10=5
		number -= (uint32_t)(10*digArr[1]);	//	56-50=6
	}
	else
	{
		digArr[1] = 0;
	}
	
	digArr[0] = (uint8_t)number;	//	6
	
	return digCnt;
}	


 


static uint32_t _getIndicatorStatus ( uint8_t digit, uint32_t indicatorStatus )
{	

		indicatorStatus &= indRowBitMask[digit];
		indicatorStatus >>= indRowBitPos[digit];
		/* first digit, where DP is replaced with one of the indicators */
		if(digit == 0)
		{
			indicatorStatus <<= 7;
		}
		else if(digit == 4)
    {
     indicatorStatus = indicatorStatus;
    }
		else
		{
			indicatorStatus <<= 8;
		}
		
	return indicatorStatus;
}



/* 2. Prepare Display Flash Data for given row
*/
void display_clearRow(  uint32_t pageIndicatorStatus )
{
	uint8_t digit;
	
 pageVarIndicatorStatus &= ~(DISP_IND_m  | DISP_IND_k);	
	
	for( digit=0; digit<DISP_NO_DIGI; digit++ )
	{
		flashData[digit] = _getIndicatorStatus( digit,pageIndicatorStatus );
	}
}


void display_setScalingFactor( DISP_SF sf )
{
	currDP = &dpPos[sf][0];
		DISP_K_IND_DIGITS = 4 + sf_digits[sf];
		DISP_m_IND_DIGITS = 2 + sf_digits[sf];
}


void display_rowNumeric ( int32_t Number, uint32_t pageFixIndicatorStatus  ){
	uint8_t numDigits,num_DP,
					digArr[DISP_DIGITS],
					digit;
		uint32_t indicatorStatus;

	// Clear variable indicators
	pageVarIndicatorStatus &= ~( DISP_IND_m  | DISP_IND_k | DISP_IND_PK | DISP_IND_R2 | DISP_IND_R1 | INDI_90);	

	/* Convert parameters into digit array */
	numDigits = _convertDecimalToDigitArr ( Number, digArr );
	
	/* Convert real time percentage of measuring and trip value */	
	bar_percent = Display_Bar();

	/* m/k Indicator */
	if( numDigits <= DISP_m_IND_DIGITS  && mili_flag)
	{
	pageVarIndicatorStatus	|=	DISP_IND_m; 	
	}
	else if( numDigits > DISP_K_IND_DIGITS )
	{
	pageVarIndicatorStatus	|=	 DISP_IND_k;
	}
	
	/* pk/r1/r1 Indicator */
  if(Pk_Display_Flag && !Trip_Relay_1)
	{
	pageVarIndicatorStatus	|= DISP_IND_PK;
	}	
	if(relay_trip_status_2)
	{
	pageVarIndicatorStatus	|= DISP_IND_R2;
	}
	if(falt_disp || __relay_test || relay_trip_status_1)
	{
	pageVarIndicatorStatus	|= DISP_IND_R1;
	}
	
	/* bar graph Indicator  test flag is to neglect thsi if system in setup*/
if(!test)
{
	mili_flag = true;

	if(bar_percent >= 90 )
	{
	pageVarIndicatorStatus |= INDI_90 ;	 			
	}
	else if(bar_percent >= 70)
	{
	pageVarIndicatorStatus |= INDI_70;
	}
	else if(bar_percent >= 50)
	{
	pageVarIndicatorStatus |= INDI_50 ;
	}
	else if(bar_percent >= 30)
	{
	pageVarIndicatorStatus |= INDI_30 ;
 	}
	else if(bar_percent >= 10)
	{
	pageVarIndicatorStatus |= INDI_10;
	}
}
	
		num_DP = numDigits;

	 if(numDigits < 4) numDigits = 4;	
	/* Map digits with BCD and apply indicator's data to 
		prepare Data to Flash on ports for each digit segment 
		in given row */
	for( digit=0; digit<DISP_NO_DIGI; digit++ )
	{
		/* add additional indicator's status for current digit */
		indicatorStatus = _getIndicatorStatus( digit, pageFixIndicatorStatus | pageVarIndicatorStatus );

		/* first digit, where DP is replaced with one of the indicators */
		if(digit != 0 )
		{
			/* Check for decimal point */
			if( digit == currDP[num_DP] )
			{
				indicatorStatus |= (1<<7);
			}
		}
		
		/* Assign BCD value
		*/
		flashData[digit] = 0x00;
		
		if( digit < numDigits && digit < DISP_DIGITS  )
		{
			flashData[digit] = digBCDMap[digArr[digit]];	 
		
			flashData[digit] |= indicatorStatus;
		}
		/* fifth digit indicators are updated here
		*/
		else
		{	
		flashData[DISP_DIGITS] |= indicatorStatus; 
		}	
	}
}




void  _setupNumberText (int32_t Number, uint32_t pageFixIndicatorStatus , uint8_t text){
	
	uint8_t numDigits,num_DP,
					digArr[DISP_DIGITS],
					digit;
		uint32_t indicatorStatus;
		uint8_t arr[]= {0x00,BCD_T,0x00};

		
	pageVarIndicatorStatus &= ~( DISP_IND_k | DISP_IND_m );	
	
	/* Convert parameters into digit array */
	numDigits = _convertDtoArr ( Number, digArr );

	/* K Indicator */
	if( numDigits > 4 )
	{
	pageVarIndicatorStatus	|=	DISP_IND_k; 	
	}

		num_DP = numDigits;

	 if(numDigits < 4) numDigits = 4;	
	/* Map digits with BCD and apply indicator's data to 
		prepare Data to Flash on ports for each digit segment 
		in given row */
	for( digit=0; digit<DISP_NO_DIGI; digit++ )
	{
		/* add additional indicator's status for current digit */
		indicatorStatus = _getIndicatorStatus( digit, pageFixIndicatorStatus | pageVarIndicatorStatus );

		/* last digit, where DP is replaced with one of the indicators */
		if(digit != 0 )
		{
			/* Check for decimal point */
			if( digit == currDP[num_DP] )
			{
				indicatorStatus |= (1<<7);
			}
		}
		
		/* Assign BCD value
		*/

		flashData[digit] = 0x00;
		
if( digit < numDigits && digit < DISP_DIGITS  )
		{
	if(digit == 3)
		{
		flashData[digit] = arr[text];

		flashData[digit] |= indicatorStatus;
		}
	else 
		{
		flashData[digit] = digBCDMap[digArr[digit]];	 
		
		flashData[digit] |= indicatorStatus;
		}
		}
else
		{	
		flashData[DISP_DIGITS] |= indicatorStatus;
		}		
	}
}


/* For texts, read characters one by one and map to corresponding hex value 
*/
const uint8_t textString[][DISP_DIGITS] = {
{BCD_NONE,BCD_B,BCD_G, BCD_N}, 
{BCD_P,BCD_I,BCD_R, BCD_T}, 
{BCD_NONE,BCD_L,BCD_A, BCD_C}, 
{BCD_NONE,BCD_T,BCD_E, BCD_S}, 
{BCD_NONE,BCD_D,BCD_N, BCD_E}, 
{BCD_e,BCD_m,BCD_i, BCD_T}, 
{BCD_e,BCD_t,BCD_a, BCD_d}, 
{BCD_s,BCD_r,BCD_d, BCD_A}, 
{BCD_d,BCD_a,BCD_u, BCD_b}, 
{BCD_i,BCD_r,BCD_a, BCD_p}, 
{BCD_y,BCD_l,BCD_e, BCD_d}, 
{BCD_e,BCD_n,BCD_O, BCD_n}, 
{BCD_NONE,BCD_d,BCD_d, BCD_O}, 
{BCD_n,BCD_e,BCD_v, BCD_E}, 
{BCD_MINUS,BCD_r,BCD_E, BCD_MINUS}, 
{BCD_MINUS,BCD_N,BCD_O, BCD_MINUS}, 
{BCD_NONE,BCD_F,BCD_F, BCD_O}, 
{BCD_T,BCD_L,BCD_A, BCD_F}, 
{BCD_M,BCD_L,BCD_C, BCD_R}, 
{BCD_R,BCD_A,BCD_L, BCD_A}, 
{BCD_t,BCD_s,BCD_e, BCD_t}, 
{BCD_t,BCD_C,BCD_b, BCD_C},
{BCD_n,BCD_E,BCD_p, BCD_O},
{BCD_E,BCD_n,BCD_O, BCD_d}
};
 


static void _showText(  const uint8_t text[], uint32_t pageIndicatorStatus ) 
{
	uint8_t digit;
	uint32_t indicatorStatus;
	

	// Clear variable indicators
	pageVarIndicatorStatus &= ~( DISP_IND_m | DISP_IND_k | DISP_IND_PK | DISP_IND_R2 | DISP_IND_R1 | INDI_90 );	
 
	/* Convert real time percentage of measuring and trip value */	
	bar_percent = Display_Bar();
	
	/* r1/r2 Indicator */
	if(relay_trip_status_2)
	{
	pageVarIndicatorStatus	|= DISP_IND_R2;
	}
	if(falt_disp || __relay_test || relay_trip_status_1)
	{
	pageVarIndicatorStatus	|= DISP_IND_R1;
	}
  if(Pk_Display_Flag && !Trip_Relay_1)
	{
	pageVarIndicatorStatus	|= DISP_IND_PK;
	}	
	/* bar graph Indicator  test flag is to neglect thsi if system in setup*/	
	if(!test)
	{
	mili_flag = true;

	if(bar_percent >= 90 )
	{
	pageVarIndicatorStatus |= INDI_90 ;	 			
	}
	else if(bar_percent >= 70)
	{
	pageVarIndicatorStatus |= INDI_70;
	}
	else if(bar_percent >= 50)
	{
	pageVarIndicatorStatus |= INDI_50 ;
	}
	else if(bar_percent >= 30)
	{
	pageVarIndicatorStatus |= INDI_30 ;
 	}
	else if(bar_percent >= 10)
	{
	pageVarIndicatorStatus |= INDI_10;
	}
}
		
	for( digit=0; digit<DISP_NO_DIGI; digit++ )
	{
		/*add additional indicator's status for current digit */
		indicatorStatus = _getIndicatorStatus(digit, pageIndicatorStatus | pageVarIndicatorStatus);
			
		flashData[digit] = 0x00;

		if(digit < DISP_DIGITS)
			{
				if( text[digit] != NULL && digit < DISP_DIGITS)
					{
						flashData[digit] = text[digit];
					}
					else /* keep blank */
						{
							flashData[digit] = 0x00;
						}		
						/* Add indicator's status */
						flashData[digit] |= indicatorStatus;
			}
			else
				{
					flashData[DISP_DIGITS] |= indicatorStatus;	
				}
	 }
}




void display_rowText ( DISP_ID_PARAM disp_id, uint32_t pageIndicatorStatus  ){
	disp_id -= DP_TEXT_NGB; /* Remove offset, set base index = 0 */
	
	_showText( textString[disp_id], pageIndicatorStatus );
}
	

void segment_skipper(uint32_t digiter){

	setup_status = true;
	
	if(digiter == 5)
				{
					testing_flag = true;
				}
				else 
					{	
						bridge_digit = digiter;
						testing_flag = false;
					}
}



void display_flashData(  uint32_t digit1 )
{
	uint8_t dl;
	uint32_t flsData = flashData[digit1];	
	
	for( dl=0; dl<DISP_DATA_LINE_SIZE; dl++ )
	{
		if( (flsData & (0x1<<dl)) )
		{
			DataPort[dl]->BRR = (uint32_t)DataPin[dl];
		}
		else
		{
			DataPort[dl]->BSRR = (uint32_t)DataPin[dl];
		}
	}
}


void segment_on( void )
{
if(setup_status)
{
if(testing_flag)
{	
flashData[0] &= 0x780;
flashData[1] &= 0x780;
flashData[2] &= 0x780;
flashData[3] &= 0x780;
}
else 
{
	flashData[bridge_digit] &= 0x780;
}}
else 
{
setup_status = false;
}
}
