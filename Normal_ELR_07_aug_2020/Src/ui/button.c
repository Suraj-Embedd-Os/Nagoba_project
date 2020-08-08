#include "button.h"
#include <string.h>


GPIO_TypeDef* const ButtonPort[NO_OF_BUTTONS] = {

		GPIOB,GPIOA
};
const uint32_t ButtonPin[NO_OF_BUTTONS] = {

GPIO_PIN_2, GPIO_PIN_15
};

uint8_t state_shifter [NO_OF_BUTTONS] = {
2,15
};



 uint16_t btnEventPressedCntr[BUTTON_TOTAL];
 uint8_t btnEventPressed[BUTTON_TOTAL] = {false};
 uint16_t btnEventReleasedCntr[BUTTON_TOTAL];

 uint8_t btnEventReleased[BUTTON_TOTAL] = {false};

 uint16_t btnEventLongPressedCntr[BUTTON_TOTAL];
 uint8_t btnEventLongPressed[BUTTON_TOTAL] = {false};
	
BUTTON_ID btn;
void button_scanEvent ( void )
{
	static bool currState[BUTTON_TOTAL] = {0x0};
	static uint8_t btnState[BUTTON_TOTAL] = {0}; // 0=release, 1=press, b1=prev state, b0=curr state
	/* Determine button press:
	*/	
	if(btn == BUTTON_TOTAL)btn = 0;
	
	if(((ButtonPort[btn]->IDR & ButtonPin[btn])>>state_shifter[btn]) == GPIO_PIN_RESET)
	{
			if(btnEventPressedCntr[btn] > 50) // Press Debounce
		{
			btnEventPressedCntr[btn] = 0;
			currState[btn] = 0x1;
		}
		else
		{
			// 1count = DIGIT_SWITCHING_PERIOD(=1ms)*TOTAL_DIGITS(=12)
			btnEventPressedCntr[btn]++; 
			
		}
	}
	else
	{
		btnEventPressedCntr[btn] = 0;
		
		if(btnEventReleasedCntr[btn] > 10) // Release Debounce
		{
			btnEventReleasedCntr[btn] = 0;
			currState[btn] = 0x0;
		}
		else
		{
			btnEventReleasedCntr[btn]++;
		}
		
	}
	/* Current button state */
	btnState[btn] = ((btnState[btn]<<1) | currState[btn]) & 0x3;
	
	if( btnState[btn] == 0x3 )
	{
		if(btnEventLongPressedCntr[btn] > 200) // Release Debounce
		{
			btnEventLongPressedCntr[btn] = 0;
			btnEventLongPressed[btn] = 1;
		}
		else
		{
			btnEventLongPressedCntr[btn]++;
		}
	}
	else
	{
		btnEventLongPressedCntr[btn] = 0;
		
		if( btnState[btn] == 0x1 )
		{
			btnEventPressed[btn] = true;
		}
		else if( btnState[btn] == 0x2 )
		{
			btnEventReleased[btn] = 1;
		}
		
	}
++btn;
}



/* Status */
bool ui_isButtonNormalPressed( BUTTON_ID btn )
{
	bool buttonNormalPressed = btnEventPressed[btn];
	btnEventPressed[btn] = false;
	return buttonNormalPressed;
}
bool ui_isButtonLongPressed( BUTTON_ID btn )
{
	bool buttonLongPressed = false;
	buttonLongPressed = btnEventLongPressed[btn];
	btnEventLongPressed[btn] = 0x00;
	btnEventPressed[btn] = 0x00;
	return buttonLongPressed;
}

bool ui_isButtonReleased( BUTTON_ID btn )
{
	bool buttonReleased = btnEventReleased[btn];
	btnEventReleased[btn] = false;
	return buttonReleased;
}


void reset_btn( void )
{
memset(btnEventReleased,0,sizeof(btnEventReleased));	
memset(btnEventLongPressed,0,sizeof(btnEventLongPressed));		
}
