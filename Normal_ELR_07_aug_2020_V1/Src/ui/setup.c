#include "setup.h"
#include "ui.h"
#include "display.h"
#include "pt24xx.h"
#include "serialcomm.h"
#include "libModbus.h"
#include "modbus.h"
#include "stm32f0xx.h"  

/**********************************************************************
MACRO declarations
**********************************************************************/
#define ON (1U)
#define OFF (0U)
#define SETUP_TOTAL_PARAMS	(3U)

/**********************************************************************
structure calling
**********************************************************************/
cfg_t cfg;

/**********************************************************************
function declaration
***********************************************************************/
void display_setup (uint8_t id ,uint8_t MenuId);
static void _save_and_exit ( uint32_t *temp_cfg );
static int32_t _editSetupParam ( uint32_t new_data, uint8_t id );
static void _load_default (void);
uint32_t temp_run( void );
uint32_t digit_4( void );	
void setup_edit_display( uint8_t id_menu, uint8_t id_left, uint8_t id_up );

/**********************************************************************
extern veriables
**********************************************************************/
extern bool test;
extern uint32_t minValue,maxValue;
extern uint16_t delay_trip;
extern bool testing_flag;
extern bool prev_tog_state;
extern bool toggle_Sv_Mv;
extern bool flow_cntl_flag;
extern bool setup_status;

/*********************************************************************
variables
**********************************************************************/
static uint8_t ident = 1,ident2 = 0;
uint8_t menuId,leftId,updId,one_second_counter;
uint32_t temp_cfg[SETUP_TOTAL_PARAMS];
uint8_t save_temp[4]={0,0,0,0};
uint8_t save_3_digit[3]= {0,0,0};
uint8_t error_indicator[SETUP_TOTAL_PARAMS]= {0,0,0};
uint8_t setup_ctrl = 1;
bool mili_flag = false;
bool retrive = false;

/*********************************************************************
@note this function will update the Max tripping value when its edited
			in setting and stores in @EEPROM
**********************************************************************/
void setting_default( void )
{
  cfg.trip_value = maxValue;
  Trip_Value = cfg.trip_value; 
	//Alarm_value=(Trip_Value*9500)/100;	
	PT24xx_write(0x74,(uint32_t *)&cfg.trip_value,sizeof(cfg.trip_value));
}

/*********************************************************************
@note this function will restores the setup values to corresponding 
			variables.
@working checks if informations are stored priviously, if not stores 
			@DEFAULT values.
**********************************************************************/
void setup_init (void)
{			

	PT24xx_read(0x64,(uint32_t *)&ident2,1);	
	
	if(ident==ident2)
	{
		PT24xx_read(0x70,(uint32_t *)&cfg,sizeof(cfg));  
		PT24xx_read(0x56,(uint32_t *)&maxValue,4); 
		PT24xx_read(0x60,(uint32_t *)&minValue,4); 
	}
	else
	{
		_load_default();  
	}
	
	Trip_Value = cfg.trip_value*100; 
	//Alarm_value=(Trip_Value*cfg.set_alarm_value)/100;
	delay_trip = cfg.set_delay*10-100;
}

/*********************************************************************
@note
Structure Define Setup parameters:
1. Possible value(Fixed or Random), current variable pointer
2. Minimum and Maximum of that parameter.
**********************************************************************/
typedef struct 
{
	uint32_t *const currVal; 
	int32_t minVal, maxVal;	
} Setup_t;

/*********************************************************************
@note
array holds Setup parameters Random values and also some texts.
**********************************************************************/
uint32_t setup_arr[5][5]={								
	{DP_TEXT_OFF,DP_TEXT_ON,DP_TEXT_TRIP},
	{150,300,600,750,900},	
	{4800,9600,19200,38400,57600},
	{DP_TEXT_NONE,DP_TEXT_ODD,DP_TEXT_EVEN},
	{50,60,70,80,90},
};

/*********************************************************************
@note
array holding Possible value(Fixed or Random) of current variable and
Minimum and Maximum of that parameter.
**********************************************************************/
const Setup_t setupParam[/*Page Number */] = {	
	{	(uint32_t *)&cfg.trip_value, 30,30000},
	{	(uint32_t *)&cfg.set_delay, 15, 500 },	
	{ (uint32_t *)&cfg.set_alarm, 0000, 9999 },	 
};

/*********************************************************************
@note array holding indicators of current parameter under editing.
**********************************************************************/
uint32_t setupind[/*Page Number */] = { 
	 ( DISP_IND_A | DISP_IND_1 | DISP_IND_S ),
	 ( DISP_IND_2 | DISP_IND_S ),
	 ( DISP_IND_3 | DISP_IND_Pr | DISP_IND_S),
	 ( DISP_IND_4 | DISP_IND_S),
	 ( DISP_IND_5 | DISP_IND_S ),
	 ( DISP_IND_6 | DISP_IND_S ),
	 ( DISP_IND_6 | DISP_IND_5 | DISP_IND_S),
};

/*********************************************************************
@note array holding scaling factor of current parameter under editing.
**********************************************************************/
DISP_SF scaling_factor[/*Page Number */] = {
	DISP_SF_SETUP,
	DISP_SF_SETUP,
	DISP_NONE_SETUP,
	DISP_NONE_SETUP,
	DISP_NONE_SETUP,
	DISP_SF_NONE,
	DISP_NONE_SETUP,
};

/*********************************************************************
@note { x , y , z , a }
 x = 0 (3 digit) numbers editing	 y = range of variable	 z = some value which you 	 a = selecting text with 3 digit number       
 x = 1 number array 	             	 that you want to vary	 want to select in 					 works only when x = 0	  
 x = 2 text array				             max range 							 setup_arr [ z ] 						 [t001]	<- in display							  
 x = 3 (4 digit) number editing
**********************************************************************/
 uint8_t setupParam_infom[/*Page Number */][4]=
{			
	{3,10,0,0},//set value
	{0,10,0,1},//delay	
	{1,5,4,0},//alarm 
	{2,3,0,0},//set alarm
};

/*********************************************************************
@note
this function takes user inputs like fixed values or Random values and 
stores it in Structure, and EEPROM.
**********************************************************************/
void setup_enter (void)
{
		uint8_t setupId;

//	if(!retrive)
//	{	
	for( setupId=0; setupId < SETUP_TOTAL_PARAMS; setupId++ )
	{
		temp_cfg[setupId] = *setupParam[setupId].currVal;
	} 
//		retrive = true;
// 		menuId = 0;
//	}

//	mili_flag = OFF;

	
if( ui_isButtonNormalPressed(BUTTON_ID_B1)) 
		{
	if(setup_ctrl == 1)
		{
		menuId++;
	if(menuId == SETUP_TOTAL_PARAMS)menuId = 0;	
 	memset(save_temp,0,sizeof(save_temp));	
	memset(save_3_digit,0,sizeof(save_3_digit));				
	leftId = 0;
		}		
		if( setup_ctrl == 2)
		{
			updId++;		
			if(updId== setupParam_infom[menuId][1])updId = 0;
		}
}
		
//if( ui_isButtonNormalPressed(BUTTON_ID_B0)) 
//{
//		if(setup_ctrl == 1)
//		{
//			menuId--;	
//	if(menuId > 200)menuId = SETUP_TOTAL_PARAMS-1;	
// 	memset(save_temp,0,sizeof(save_temp));		
//	memset(save_3_digit,0,sizeof(save_3_digit));				
//	leftId = 0;
//		}		
//		if( setup_ctrl == 2)
//		{
//			updId--;		
//			if( updId > 200)updId = setupParam_infom[menuId][1]-1;
//		}
//}else 


else if( ui_isButtonNormalPressed(BUTTON_ID_B0)) 
{	

	if(setup_ctrl == 2) 
		{
		if(setupParam_infom[menuId][0] == 0)
		{
		leftId++;
		if(leftId > 2)leftId = 0;
 		updId = 0;
		}
		if(setupParam_infom[menuId][0] == 3)
		{
		leftId++;
		if(leftId > 3)leftId = 0;
 		updId = 0;
		}	
		}
	else if(setup_ctrl == 1)
		{
			setup_ctrl = 2;//to enter submenu
		}
				
}
else 		
if( ui_isButtonLongPressed(BUTTON_ID_B0) ) 
		{
		if(setup_ctrl == 1)
		{
		_save_and_exit(temp_cfg);				
		HAL_NVIC_SystemReset();
		}			
		if(setup_ctrl == 2)
		{			
		setup_ctrl = 1;
		setup_status = false;
		updId = 0;
		leftId = 0;
		}
}
	display_setScalingFactor(scaling_factor[menuId]);				

	if(setup_ctrl == 1)
	{	
		display_setup(setupParam_infom[menuId][0],menuId);
	}	
	else if(setup_ctrl == 2 ) 	
	{
		setup_edit_display(menuId, leftId, updId);
	}
}

/******************************************************************************
@note
takes current setup values and compaires with its predefine stored min,max values
if current value crosses the range it sets corresponding error bit to high so 
display will show error with respect to id

dividing min,max by 100 to get them to 3 digit for comparition,

@Input: 
new_data	 holds the 32bit data from user using setup
id			 	 holds the parameter id which is to be compiared
******************************************************************************/
static int32_t _editSetupParam ( uint32_t new_data, uint8_t id )
{
	uint32_t currVal =	new_data;

	if(id == 0)
	{
	if( currVal < minValue){currVal =  maxValue; error_indicator[id] = 1;}
	if( currVal > maxValue){currVal =  maxValue; error_indicator[id] = 1;}
	else {error_indicator[id] = 0;}
	}
	else
		{
	if( currVal < setupParam[id].minVal){ currVal = setupParam[id].minVal;error_indicator[id] = 1;}
	if( currVal > setupParam[id].maxVal){ currVal = setupParam[id].maxVal;error_indicator[id] = 1;}
	else {error_indicator[id] = 0;}
		}
	return currVal; 
}

/******************************************************************************
@note
 takes the edited array and stores them in structure, these values then stored in EEPROM
 a check bit and end bit is added to ensure the storing of EEPROM

 ident is a another check bit setted to 1 for compairing used to compair while system rested for
 check the EEPROM stored value 
******************************************************************************/
static void _save_and_exit ( uint32_t *new_cfg )
{
	cfg.check_bit 				=  1;
	cfg.trip_value 				= (uint32_t)new_cfg[0];
	cfg.set_delay 				= (uint32_t)new_cfg[1];
	cfg.set_alarm 				= (uint32_t)new_cfg[3];
	cfg.end_bit						= 0;
	ident = 1;
	
	PT24xx_write(0x64,(uint32_t *)&ident,1);
	PT24xx_write(0x70,(uint32_t *)&cfg,sizeof(cfg));

}

/******************************************************************************
@note
 this funcation stores the DEFAULT setted values to corrensponding parameters
 an ident is rested and stored in EEPROM for compaire it while, 
 this bit 0 system takes default values  
******************************************************************************/
static void _load_default (void)
{
	cfg.check_bit 				=  1;
	cfg.trip_value 					= DEFAULT_SET_VALUE;
	cfg.set_delay 					= DEFAULT_DELAY;	
	cfg.set_alarm 					= DEFAULT_ALARM_STATUS;
	cfg.end_bit						= 0;
//	minValue = 3;
//	maxValue = 3000;
	ident = 0;	
	PT24xx_write(0x64,(uint32_t *)&ident,1);
//	PT24xx_write(0x60,(uint32_t *)&minValue,4);
//	PT24xx_write(0x56,(uint32_t *)&maxValue,4);
}

/******************************************************************************
@note
takes the global declared unsigned int 8 bit save_3_digit[] array data of 3 indivisual 
digits and forms it into 32bit single data and returns that.
******************************************************************************/
uint32_t temp_run( void )
{
	uint32_t d1,d2,data;	
	d1=save_3_digit[2]*100;
	d2=d1;
	d1=save_3_digit[1]*10;
	d2+=d1;
	d1=save_3_digit[0]*1;
	d2+=d1;	
	data=d2;
	return data;
}

/******************************************************************************
@note
takes the global declared unsigned int 8 bit save_temp[] array data of 4 indivisual 
digits and forms it into 32bit single data and returns that.
******************************************************************************/
uint32_t digit_4( void )
{
	uint32_t d1,d2,data;	
	d1=save_temp[3]*1000;
	d2=d1;
	d1=save_temp[2]*100;
	d2+=d1;
	d1=save_temp[1]*10;
	d2+=d1;
	d1=save_temp[0]*1;
	d2+=d1;	
	data=d2;
	return data;
}

/******************************************************************************
@note
Displays the Previous Paramaters On display and Current Saved DATA with respect to 
its Id.
******************************************************************************/
void display_setup (uint8_t id ,uint8_t MenuId)
{
 if(id == 2)		
	{	
		display_rowText((DISP_ID_PARAM)setup_arr[setupParam_infom[MenuId][2]][temp_cfg[MenuId]],
					setupind[MenuId]);
	}
else if(id == 1 || id == 3)
	{	
		if(error_indicator[MenuId] == 1)
		{
			display_rowText((DISP_ID_PARAM)DP_CT_ERROR,
					setupind[MenuId]);
			one_second_counter++;
		if(one_second_counter > 2)
		{
		error_indicator[MenuId] = 0;
		one_second_counter = 0;
		}
		}
else
		{
			error_indicator[MenuId] = 0;
			display_rowNumeric((int32_t)temp_cfg[MenuId],
						setupind[MenuId]);
		}
	}
	
else 
	{	
		if(error_indicator[MenuId] == 1)
		{
			display_rowText((DISP_ID_PARAM)DP_CT_ERROR,
					setupind[MenuId]);
					one_second_counter++;
		if(one_second_counter > 2)
		{
		error_indicator[MenuId] = 0;
		one_second_counter = 0;
		}
		}
else
		{
			error_indicator[MenuId] = 0;
		_setupNumberText((int32_t)temp_cfg[MenuId],
						setupind[MenuId],setupParam_infom[MenuId][3] );
		}
	}
}

/******************************************************************************
@note
shows current editing values or test in Display and stores to temp_arr[].
@takes
parameter Id, left_ value, up_down_value
******************************************************************************/
void setup_edit_display( uint8_t id_menu, uint8_t id_left, uint8_t id_up )
{
if(setupParam_infom[id_menu][0] == 0 )	//3 digit numbers 
		{					
			save_3_digit[2-id_left] = id_up;
			
			_setupNumberText(temp_run(),setupind[id_menu],
						setupParam_infom[id_menu][3]);
					temp_cfg[id_menu] = _editSetupParam(temp_run(),id_menu);		
			
			segment_skipper(2-id_left);			
					
	}
				
if(setupParam_infom[id_menu][0] == 1 ) // number array
		{				
			display_rowNumeric(setup_arr[setupParam_infom[id_menu][2]][id_up],
					setupind[id_menu]);
			temp_cfg[id_menu] = setup_arr[setupParam_infom[id_menu][2]][id_up];
					segment_skipper(5);			
		}
				
if(setupParam_infom[id_menu][0] == 2 )	// text array
		{
			display_rowText((DISP_ID_PARAM)setup_arr[setupParam_infom[id_menu][2]][id_up],
						setupind[id_menu]);
					temp_cfg[id_menu] = id_up;
						segment_skipper(5);	
			}
	
if(setupParam_infom[id_menu][0] == 3 )	//4 digit numbers 
		{	
		save_temp[3-id_left]=id_up;		
		display_rowNumeric(digit_4(),setupind[id_menu]);
			temp_cfg[id_menu] = _editSetupParam(digit_4(),id_menu);				
				segment_skipper(3-id_left);			
				
	}
}

/************************ (C) COPYRIGHT Nagoba Electronics *****END OF FILE****/

