#include "button_new.h"
#include "button.h"

#define button_1 ((GPIOB->IDR & GPIO_PIN_2)>>2)
#define button_2 ((GPIOA->IDR & GPIO_PIN_15)>>15) 
#define button_3 ((GPIOA->IDR & GPIO_PIN_11)>>11)
#define button_4 ((GPIOA->IDR & GPIO_PIN_5)>>5)





uint16_t btnEventPressedCnt[BUTTON_TOTAL] = {0,0,0,0};
uint16_t btnlongPressedCntr[BUTTON_TOTAL] = {0,0,0,0};
static bool normal_btn_arr[BUTTON_TOTAL] = {0x0,0x0,0x0,0x0};
//static bool long_btn_arr[BUTTON_TOTAL] = {0x0,0x0,0x0,0x0};
uint16_t btnrestPressedCntr[BUTTON_TOTAL] = {0,0,0,0};
uint16_t btnrest_longPressedCntr[BUTTON_TOTAL] = {0,0,0,0};



bool normal_press_1( void )
{
if(button_1)
	{
	if(btnEventPressedCnt[0] > 150) // Press Debounce
		{
			btnEventPressedCnt[0] = 0;
			normal_btn_arr[0] = 0x1;
		}
		else
		{
			btnEventPressedCnt[0]++; 	
		}
	}
	else
	{
	btnEventPressedCnt[0] = 0;
	btnrestPressedCntr[0]++;
	
	if(btnrestPressedCntr[0] > 50)
	{
	normal_btn_arr[0] = 0x0;
	btnrestPressedCntr[0] =0;
	}
	}
return normal_btn_arr[0];
}




bool normal_press_2( void )
{
if(button_2)
	{
			if(btnEventPressedCnt[1] > 150) // Press Debounce
		{
			btnEventPressedCnt[1] = 0;
			normal_btn_arr[1] = 0x1;
		}
		else
		{
			btnEventPressedCnt[1]++; 	
		}
	}
	else
	{
	btnEventPressedCnt[1] = 0;
	btnrestPressedCntr[1]++;
	if(btnrestPressedCntr[1] > 150)
	{
	normal_btn_arr[1] = 0x0;
	btnrestPressedCntr[1] =0;
	}	
	}
return normal_btn_arr[1];	
}

bool normal_press_3( void )
{
bool status = 0x0;
if(button_3)
	{
			if(btnEventPressedCnt[2] > 150) // Press Debounce
		{
			btnEventPressedCnt[2] = 0;
			status = 0x1;
		}
		else
		{
			btnEventPressedCnt[2]++; 	
		}
	}
	else
	{
	btnEventPressedCnt[2] = 0;
	}
return status;
	
}
bool normal_press_4( void )
{
bool status = 0x0;
if(button_4)
	{
			if(btnEventPressedCnt[3] > 150) // Press Debounce
		{
			btnEventPressedCnt[3] = 0;
			status = 0x1;
		}
		else
		{
			btnEventPressedCnt[3]++; 	
		}
	}
	else
	{
	btnEventPressedCnt[3] = 0;
	}
return status;
}



bool long_press_1( void )
{
bool status = 0x0;
if(button_1)
	{
			if(btnlongPressedCntr[0] > 2000) // Press Debounce
		{
			btnlongPressedCntr[0] = 0;
			status = 0x1;
		}
		else
		{
			btnlongPressedCntr[0]++; 	
		}
	}
	else
	{
	btnlongPressedCntr[0] = 0;
	}
return status;
}

bool long_press_2( void )
{
bool status = 0x0;
if(button_2)
	{
			if(btnlongPressedCntr[1] > 2000) // Press Debounce
		{
			btnlongPressedCntr[1] = 0;
			status = 0x1;
		}
		else
		{
			btnlongPressedCntr[1]++; 	
		}
	}
	else
	{
	btnlongPressedCntr[1] = 0;
	}
return status;
}


