
#ifndef __RMS_H
#define __RMS_H
                 // Device header
#include "main.h"



#ifdef __cplusplus
 extern "C" {
#endif

#include"stdbool.h"
/***************************************************************************************/

/***********************Write preprocessor directives*************************************************/
#define VDC_OFFSET 							1.65f // DC Offset Voltage
#define AN_VREF							     3.30f	// Analog Reference Voltage
#define ADC_BITS 								12 // ADC bit resolutionn
#define ADC_MAXOUT 							(1<<ADC_BITS)
#define TOTAL_ADC_CHANNEL       (2U)
#define TOTAL_NO_SAMPLE         (800U)
#define NO_OF_BIT_SHIFT          (2U)
//#define QUARTER_NO_OF_SAMPLE   (TOTAL_NO_SAMPLE/4)
#define HALF_NO_OF_SAMPLE       (TOTAL_NO_SAMPLE/2)
//#define THIRD_FOURTH_SAMPLE     (3*QUARTER_NO_OF_SAMPLE)
#define  BUFFER_SIZE            (10U)

#define   ENABLE                 (1)
#define   DISABLE                (0)

#define  No_OF_TIME_CALL          (4U)

     /* first enable  CALIBARTION_EIN_DIS  then  SELECT_CALIBARTION_CHANNEL */  
		 /*****************To start calibartions  *************/
		 
#define        CALIBARTION_EIN_DIS           (0)  /* enable and disable the calibartions */
#define        LOW_CURENT_CALIBARTION        (1)   
#define        HIGH_CURRENT_CALIBARTION      (2)   
#define         AVERAGE_CALIBARTION          (0)   /* selection for average calculations once */

#define      SELECT_CALIBARTION_CHANNEL     (HIGH_CURRENT_CALIBARTION)    /* selection for calibartion  */
 
#define  Average_Mul       ((float)1/(float)TOTAL_NO_SAMPLE)//()
	
#if    (SELECT_CALIBARTION_CHANNEL==LOW_CURENT_CALIBARTION)
#define   MULTIPLICATIONS_FACTORS  (1000U) /* multiplier */
#define   CALIBARATIONS_CURRENT     (6U)   /* calibaration current in 6=0.6 Amp */


#define  CALIBARATIONS_FACTORS  (MULTIPLICATIONS_FACTORS*CALIBARATIONS_CURRENT)

#elif     (SELECT_CALIBARTION_CHANNEL==HIGH_CURRENT_CALIBARTION)
#define   MULTIPLICATIONS_FACTORS  (10000U) /* multiplier */
#define   CALIBARATIONS_CURRENT     (20U)   /* calibaration current in 5=0.5 Amp */
#define  CALIBARATIONS_FACTORS  (MULTIPLICATIONS_FACTORS*CALIBARATIONS_CURRENT)

#endif

#define  S1_Button           ((GPIOB->IDR & GPIO_PIN_2)>>2)
#define  S2_Button           ((GPIOA->IDR & GPIO_PIN_15)>>15)

#define  HIGH          (1U)
#define  LOW           (0U)

#define  Relay_select_1        (1U)
#define  Relay_select_2        (2U)
 
/**************** Adress defintion Start for Store and read data into from EEPROM ********************/
#define SCALING_FACTOR_STORE      0x0004 /* 8 byte store */
#define AVERAGE_VALUE             0x000c  /* 8 byte */
#define LAST_FIVE_TRIP_DATA       0x0014  /* 20 byte */

#define Index_add                 0x002C


/**************************************************************************************************/
/**************User variable Declaration Start***************************************/


extern uint32_t Start_Timmer;

extern uint32_t Trip_Delay_Time;

extern uint32_t Trip_Value; 

extern bool Trip_Value_Status,DG_Status,Alarm_Set_Flag;

extern bool Trip_Relay_1,Trip_Relay_2;

extern bool Button_flag_1,Button_flag_2,Button_flag_3,Button_flag_4;

extern bool IsLongPress_1,IsLongPress_2,IsLongPress_3,IsLongPress_4;

extern uint32_t Long_Press_1,Long_Press_2,Long_Press_3,Long_Press_4;


extern bool Select_Adc_Channel_1_Rms,Select_Adc_Channel_2_Rms;

extern  uint32_t Rms_Buffer[BUFFER_SIZE];

extern  uint32_t Stored_Trip_Data[5];

extern bool relay_trip_status_1,relay_trip_status_2;

extern bool Pk_Display_Flag;

extern uint32_t Eeprom_RW_Status;

extern  uint32_t Alarm_value; /* alarm */

extern bool refrence_flag;

extern uint32_t sqr_sample_buffer[2][10];

extern  uint32_t  Avg_sqr_rot_sample[2];

extern  uint32_t  __Avg_Value[2];

extern bool callibartion_flag,start_calibartions_flag;

extern uint8_t Display_Bar_per;


 extern uint32_t  Calibartion_Factor_Array[No_OF_TIME_CALL];

 extern  float Scaling_Factor_Array[No_OF_TIME_CALL];




 

/**************************************************************************************************/


/**************User variable Declaration Start***************************************/

typedef struct RMS_Sample
{
	uint32_t RMS_Value[2];
	uint16_t Avg_Value[TOTAL_ADC_CHANNEL];
	uint32_t Rms_Value_Disply;
	
}Adc;
 
/***************************************************************************************/

typedef struct Common_Variable
{	
	uint32_t Adc_Curr_Sample[TOTAL_ADC_CHANNEL];
	
	uint32_t Adc_Sqr_Curr_Sample[2][2]; 
	
	float Scaling_Factor[2];
	
}Common_Var;

/***************************************************************************************/

typedef struct Store
{
	
	int32_t Adc_Curr_Store_Sample[TOTAL_ADC_CHANNEL]; 
	
	uint32_t Adc_Sqr_Curr_Store_Sample[2]; 
	
}Store_Data;

/***************************************************************************************/

typedef enum 
{
	FAULT_1 = 0,
	FAULT_2,
	FAULT_3,
	FAULT_4,
	FAULT_5,
	TRIP_VALUE,
	RMS_LEAKAGE_CURRENT,
	FAULT_VALUE
}RMS_READING_t;

/**************************************************************************************************/
/**************Declation of user function***************************************/

extern void Rms_Calculations(void);

extern void Scaling_Calculations(void);

extern void Calibaration(void);

extern void Read_Eeprom_Data(void);

extern void Store_Adc_Data(void);

extern  inline void  Adc_Sample_Calculations(uint32_t Adc_Sample[],uint8_t Slot);

extern void Hard_Init(void);

extern void Trip_Status_Check(void);

extern void  Relay_Set_Reset(uint8_t Relay_select,uint8_t Status);

uint32_t read_value (RMS_READING_t id);

void Button_Scan(void);

void IsLongPress(void);

void Selection_Of_Adc_Channel(uint8_t Adc_Select);

uint32_t Average_Rms_Display(void);

void Refrence_Voltage_Status(void);

void Test_Conditions(void);

extern uint8_t Display_Bar(void);

extern void Twice_calibartion(void);

extern float Return_Scaling(uint32_t Rms,uint8_t index,uint8_t channel);

void Slop_Intercept_Cal(uint8_t channel,uint8_t index);


/**************************************************************************************************/


extern Adc  Adc_Rms;
extern Common_Var  Common;
extern Store_Data  Stored;








#ifdef __cplusplus
}
#endif
#endif

/************************ (C) COPYRIGHT NAGOBA ELECTRONICS  NOV 2019*****END OF FILE****/
