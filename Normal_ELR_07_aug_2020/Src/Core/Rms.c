/*******************************************************************************/

/********************Include header File and preprocessor***********************************/
#include "main.h"
#include "Rms.h"
#include "math.h"
#include "stdbool.h"
#include "string.h"
#include "pt24xx.h"
#include "setup.h"
#define   SQR(va)   ((va)*(va))
#define REMOTE_RESET   ((GPIOA->IDR & GPIO_PIN_6)>>6)      

/*******************************************************************************/

/**************Declaration of Private variable*******************************/
 
uint32_t Trip_Value; 

bool Trip_Relay_1,Trip_Relay_2;

bool Select_Adc_Channel_1_Rms,Select_Adc_Channel_2_Rms;

bool DG_Status=false,Alarm_Set_Flag=false;

uint32_t Rms_Buffer[BUFFER_SIZE];

uint32_t Stored_Trip_Data[5];


Adc  Adc_Rms;
Common_Var  Common;
 Store_Data  Stored;
 
bool relay_trip_status_1=false,relay_trip_status_2=false;	 
	/********/  

extern bool comp_mem;	

bool Pk_Display_Flag=false; /* used for pick up */	


uint32_t __temp_write=1;

 uint32_t Alarm_value; /* alarm */
 
 bool __relay_test=false;  /* used  for test relay  r1 */
 
 extern bool test ;
 
 bool refrence_flag=false;

 uint32_t sqr_sample_buffer[2][10];

 uint32_t  Avg_sqr_rot_sample[2];
 
 uint32_t  __Avg_Value[2];
 
  uint8_t Display_Bar_per=0;
	
	uint8_t __adc_select;
	
	uint16_t delay_trip;
	
	extern bool Write_history_trip;

	uint32_t disp_trip = 0;
	
	extern bool falt_disp;
	
 uint32_t  Calibartion_Factor_Array[No_OF_TIME_CALL]= {300,600,1000,3000,6000,8000,10000,15000,20000,30000};
/***********************************************************************************************/	
 uint32_t _stored_sqr_value[2][No_OF_TIME_CALL];
 
 uint8_t Relay_Voltage_Detection=1;  /* 0 = not ok 1 = ok */
 
/********************************Definition of user function Start**********************************************/

void New_Scaling(float slope,float intercept,uint8_t channel);
 
 
void Start_Check_Scaling(uint32_t __Current_Sqrt, uint8_t Channel);

/***************************************************************************************/

uint8_t __temp=0,Select_Adc=0;
/*Operation On ADC SAMPLE call every 125us */
uint16_t acd[2][160];

inline void  Adc_Sample_Calculations(uint32_t Adc_Sample[],uint8_t Slot)
{
	
	uint8_t Acd_Channel;
	int32_t Ca;
			
	for(Acd_Channel=0;Acd_Channel<TOTAL_ADC_CHANNEL;Acd_Channel++)
	   {
		   Ca= (int32_t)Adc_Sample[Acd_Channel];
		/* Accumulation for Average Calcalculations */
		Common.Adc_Curr_Sample[Acd_Channel] +=  (uint32_t)Adc_Sample[Acd_Channel];
		 
	
		/* Removing Dc_offSEt Value */
		Ca -=  Adc_Rms.Avg_Value[Acd_Channel]; 
		
		/* Accumulation for Average Calcalculations */	
		Common.Adc_Sqr_Curr_Sample[Slot][Acd_Channel]+= SQR(Ca);
	   }

	
		/*Seclection for high current and low current */
      	
	 /* if Adc_Value of Adc channel two ADC2 is gereter than 4000 and less then 100
	  It means it start saturated so select another ADC channel ADC1 for comparing eith triping value */
					if(Adc_Sample[1]>4000 ||Adc_Sample[1] <100)
						 {
						    Select_Adc++;
					   }
					if(__temp>=160)
					   {
						__temp=0;
						Selection_Of_Adc_Channel(Select_Adc);  /* used for Selection of channel for trip value compare*/
							Select_Adc=0;
					   }
  __temp++;
   
	
}
/***************************************************************************************/


/***************************************************************************************/


/***************************************************************************************/
/* Selection of adc channel for tripping value compare call every 125us*160 sample */
void Selection_Of_Adc_Channel(uint8_t Adc_Select)
{
		/*Seclection for high current and low current */
	/*if ADC_Select is grether than 10 ,means Adc_saturation is happen so choose another channel*/ 
	   if(Adc_Select>2)
	    {
				/* selection of adc channel for fault  */
		   Select_Adc_Channel_1_Rms=true; /*select for high value of current */ 
			 Select_Adc_Channel_2_Rms=false;
				
	    }
	  else 
	     {
				 
			 Select_Adc_Channel_1_Rms=false;
		   Select_Adc_Channel_2_Rms=true;  /*select for low value of current */ 
	     }
}

/***************************************************************************************/
/*stored adc_accumulated data for further Operation call every 100ms*/

bool history_flag = false;


 void Store_Adc_Data(void)
 {
	 uint8_t Acd_Channel;
	for(Acd_Channel=0;Acd_Channel<TOTAL_ADC_CHANNEL;Acd_Channel++)
	{
		/* clear Accumulation Data  */ 
			Stored.Adc_Curr_Store_Sample[Acd_Channel]=0; 
	   
		/*copy for AVerage_calculations */
		  Stored.Adc_Curr_Store_Sample[Acd_Channel]=Common.Adc_Curr_Sample[Acd_Channel]; 
	    
		  Common.Adc_Curr_Sample[Acd_Channel]=0;
		
		
		 
				/* Average calcularions every time */
			Adc_Rms.Avg_Value[Acd_Channel]= (uint16_t) (Stored.Adc_Curr_Store_Sample[Acd_Channel]*Average_Mul);
		
			 	uint8_t slot;
		/* clear Accumulation Data  */ 
		Stored.Adc_Sqr_Curr_Store_Sample[Acd_Channel]=0;
		for(slot=0;slot<2;slot++)
	     {
	   /*copy for rms And*/
				Stored.Adc_Sqr_Curr_Store_Sample[Acd_Channel]+=Common.Adc_Sqr_Curr_Sample[slot][Acd_Channel]>>NO_OF_BIT_SHIFT;
			
			 Common.Adc_Sqr_Curr_Sample[slot][Acd_Channel]=0;
	     }
		
	}
	
	
		/*
	    voltage divider circuit is 3.3k by 10k
	    at V =7.8 v
	    V_at 3.3 =   V*3.3/(3.3+10) = 1.935 
	          3.3  =  4096
	         1.935 = 2401
	    at V =5     V*3.3/(3.3+10) = 1.240
	
       	    3.3  =  4096
	          1.240 = 1539
	 let V=8v
	*/
	
 }

/***************************************************************************************/

 
 /***************************************************************************************/
 /* Average of Adc_calculation call once */
void Read_Eeprom_Data(void)
{
	  uint8_t Acd_Channel;
	 for(Acd_Channel=0;Acd_Channel<2;Acd_Channel++)
	 {
		// Adc_Rms.Avg_Value[Acd_Channel]= (uint16_t) ((float)VDC_OFFSET*ADC_MAXOUT)  /AN_VREF;
	 }
	
	PT24xx_read(0x0C8,(uint32_t *)_stored_sqr_value,sizeof(_stored_sqr_value));  /* Stored Square data */
}

/***************************************************************************************/
 /* Calibaration for calculate scaling factors  call once */
 uint8_t __temp253=0,__just=0;
 
 void Twice_calibartion(void)
 {
	 uint8_t Acd_Channel;
	 for(Acd_Channel=0;Acd_Channel<2;Acd_Channel++)
	     {
	    /*add 10 times adc data  means 800*10 no of sample */
	 	 sqr_sample_buffer[Acd_Channel][__temp253] =  sqrt(Stored.Adc_Sqr_Curr_Store_Sample[Acd_Channel]);
		 
	     }
                __temp253++;	 
            if(__temp253==10)
							       {
								 __temp253=0;
								      }
 }
 /***************************************************************************************/

void Calibaration(void)
{
	 uint8_t Acd_Channel;
	
	  #if CALIBARTION_EIN_DIS
								 
	     for(Acd_Channel=0;Acd_Channel<2;Acd_Channel++)
	       {
								Avg_sqr_rot_sample[Acd_Channel]=0;
						uint8_t __temp123;
						for(__temp123=0;__temp123<10;__temp123++)
						   {
							   Avg_sqr_rot_sample[Acd_Channel]+=sqr_sample_buffer[Acd_Channel][__temp123];
								 sqr_sample_buffer[Acd_Channel][__temp123]=0;
						   }
						     Avg_sqr_rot_sample[Acd_Channel]/=10;
							 
							 /* stored average of 10 sqare data for scaling factor calculations */ 
							 _stored_sqr_value[Acd_Channel][__just]=Stored.Adc_Sqr_Curr_Store_Sample[Acd_Channel];
						   
							 
					  }
				  __just++;
				 	 if(	__just>No_OF_TIME_CALL-1)
           {
	         PT24xx_write(0x0C8,(uint32_t *)_stored_sqr_value,sizeof(_stored_sqr_value));
           }
			
							
	 #endif
	
	
			 if(AVERAGE_CALIBARTION)
			 PT24xx_write(AVERAGE_VALUE,(uint32_t *)Adc_Rms.Avg_Value,8);
	
}

/***************************************************************************************/
		

/*Rms_calulation_of current  call every after 100ms */
uint8_t _buffer_count=0;

void Rms_Calculations(void)
{
    	uint32_t __Current_Sqrt=0;
	     uint8_t Acd_Channel;
      for(Acd_Channel=0;Acd_Channel<2;Acd_Channel++)
	     {
				 	Adc_Rms.RMS_Value[Acd_Channel]= (uint32_t) (sqrt(Stored.Adc_Sqr_Curr_Store_Sample[Acd_Channel]))* Common.Scaling_Factor[Acd_Channel];
				 
				if(Adc_Rms.RMS_Value[Acd_Channel]<200)  /* less than 20 mili amp display zero  */
				 	Adc_Rms.RMS_Value[Acd_Channel] =0;
    	 }
     /* stored into buffer for average  rms display  */			 
			 if(Select_Adc_Channel_1_Rms==true)
			 {
				    Rms_Buffer[_buffer_count++]=Adc_Rms.RMS_Value[0];
				  __Current_Sqrt=Stored.Adc_Sqr_Curr_Store_Sample[0];
				 
				Start_Check_Scaling(__Current_Sqrt,0); //first adc channel
			 }
			 else if (Select_Adc_Channel_2_Rms==true)
			 {
				    Rms_Buffer[_buffer_count++]=Adc_Rms.RMS_Value[1]; /*  40 gain */
				  __Current_Sqrt=Stored.Adc_Sqr_Curr_Store_Sample[1]; 
				 
				 Start_Check_Scaling(__Current_Sqrt,1); // 2nd Adc channel
			 }
			 
			 if(_buffer_count>=BUFFER_SIZE)
			 {
				 _buffer_count=0;
				 
			 }
			 
			 
			 /***********************************************/
			 
}
/***************************************************************************************/
 
 void Slop_Intercept_Cal(uint8_t channel,uint8_t index)
 {
	 
	 /* equetion of liner line between two point is:	 
	  y=mx+b;
	 where m=slop;b=intercept at y axis
	  slop m =(y2-y1)/(x2-x1) 
	 intercept b =y-slop*x
	 y=slope*x+intercept;  where y= scaling factor ,x= square root of adc voltage value 
	 */
	 uint32_t x1= sqrt(_stored_sqr_value[channel][index]); // x1=first point x2=next point
	 uint32_t x2= sqrt(_stored_sqr_value[channel][index+1]);
	 
	  float  y1  =  (float)Calibartion_Factor_Array[index]/x1;  //y1=first point calibaration factor
	  float  y2  =  (float)Calibartion_Factor_Array[index+1]/x2;  //y2=second point calibaration factor
	 
	 /* slop =(y2-y1)/(x2-x1) */
	  float Slope =  (float)(y2-y1)/(x2-x1);                 
	  float Intercept= y1- Slope*x1;                        
	 Common.Scaling_Factor[channel] =(float)x1*Slope+Intercept;//(y1+y2)/2;
	 
 }
 
 /***************************************************************************************/
void Start_Check_Scaling(uint32_t __Current_Sqrt, uint8_t Channel)
{
		uint8_t index;
	  for(index=0;index<No_OF_TIME_CALL;index++)
	   { 
				if(__Current_Sqrt >= _stored_sqr_value[Channel][index] && __Current_Sqrt < _stored_sqr_value[Channel][index+1])
				{
					Slop_Intercept_Cal(Channel,index);
					return;
				}
				if(Channel==1)
				{
				  if(__Current_Sqrt<_stored_sqr_value[1][0])
				 {
					
					 Common.Scaling_Factor[1]= (float)Calibartion_Factor_Array[0]/sqrt(_stored_sqr_value[1][0]); //less than minimum current
					 return;
				 }
			 }
				if(Channel==0)
				{
			     if(__Current_Sqrt>_stored_sqr_value[0][No_OF_TIME_CALL-1])
				  {
					 Common.Scaling_Factor[0]= (float)Calibartion_Factor_Array[No_OF_TIME_CALL-1]/sqrt(_stored_sqr_value[0][No_OF_TIME_CALL-1]); //greater than maximum current
						return;
				  }
    	}
		}
}
/***************************************************************************************/

/* checking the refrence volatge call in while loop*/
void Refrence_Voltage_Status(void)
{
	  if(Adc_Rms.Avg_Value[1] < 500)
	  {
		/* Trip the relay */
			Relay_Set_Reset(Relay_select_1,HIGH);
			Relay_Set_Reset(Relay_select_2,HIGH);
			  Trip_Relay_1=true;  /*Reset the staus of realy */
				 refrence_flag=true;
			Adc_Rms.RMS_Value[0] =0;
			Adc_Rms.RMS_Value[1] =0;
			//Display_Bar_per=0;
			
	  }
		else if(Adc_Rms.Avg_Value[1] < 500)
	  {
		/* Trip the relay */
			Relay_Set_Reset(Relay_select_1,HIGH);
			Relay_Set_Reset(Relay_select_2,HIGH);
		  	Trip_Relay_1=true;
			 refrence_flag=true;
			Adc_Rms.RMS_Value[0] =0;
			Adc_Rms.RMS_Value[1] =0;
			//Display_Bar_per=0;
	  }
		else
			 refrence_flag=false;
			
		
}

/***************************************************************************************/

/***************************************************************************************/

/* checking the manualyy reset and set press  call in while loop*/
void Test_Conditions(void)
{
	    if(S4_Button!=true && test !=true)
			{
				/* Manually Trip press  s1 button*/
				  //Relay_Set_Reset(Relay_select_1,HIGH);
				   GPIOF->ODR |=GPIO_PIN_6;
				  //Trip_Value_Status=false;
					Pk_Display_Flag=false;   /* reset pk */
				__relay_test=true;        /* used in relay testing condition only */
				  //relay_trip_status_1=true;   /* used for showing relay status  */ 
			}
			else if((S3_Button!=true && test !=true) || REMOTE_RESET)  /* REMOTE_RESET is when dg pin set all relay should be reset */
			{
				__relay_test=false;    /* used in relay testing condition only */
				  relay_trip_status_1=false;
				  relay_trip_status_2=false;
				  Start_Timmer=0;  /* restart the timmer */
				  Trip_Relay_1=false;  /*Reset the staus of realy */
				  Trip_Relay_2=false;   /*Reset the staus of realy */
					history_flag = false;
				
				    falt_disp=false;

				/* Manually Reset press s2 button*/
          Relay_Set_Reset(Relay_select_1,LOW);
				/* Clear Alarm */
				Relay_Set_Reset(Relay_select_2,LOW);
			}				
}
/***************************************************************************************/

/***************************************************************************************/
/*Average_Rms_Display calculation for display Only */
uint32_t Average_Rms_Display(void)
{
	uint8_t temp;
	 Adc_Rms.Rms_Value_Disply=0;
	for(temp=0;temp<BUFFER_SIZE;temp++)
	{
		/* add rms_buffer */
	  Adc_Rms.Rms_Value_Disply+=Rms_Buffer[temp];
	}
	Adc_Rms.Rms_Value_Disply /=10; /* calculate average rms */
	return Adc_Rms.Rms_Value_Disply;
}


/***************************************************************************************/
/* For setting and resetting Relay functions*/
 void Relay_Set_Reset(uint8_t Relay_select,uint8_t Status)
 {
	 /*****Set relay  pin high**********************************/
	 	if(Status==HIGH && Relay_select==Relay_select_1 )
	   {
			 //if(S4_Button!=true && test !=true)
			    /* used to hide the pk */
	     GPIOF->ODR |=GPIO_PIN_6;
			 relay_trip_status_1=true;    /* used for rl relay status   only */
			 //relay_trip_status_2=false;   /* used for r2relay status    only */
			 Pk_Display_Flag=false;   /* pk display status */
	   }
	  else if(Status==LOW &&  Relay_select==Relay_select_1)
	   {
		  GPIOF->ODR &=~GPIO_PIN_6;
			 relay_trip_status_1=false;  /* used for rl relay status   only */
	   }
	 
	if(Status==HIGH && Relay_select==Relay_select_2 )
	  {
	 GPIOF->ODR |=GPIO_PIN_7;
			relay_trip_status_2=true; /* used for r2 relay status    only   used for display   */
	  }
	 else if(Status==LOW && Relay_select==Relay_select_2)
	   {
		 GPIOF->ODR &=~GPIO_PIN_7;
			 relay_trip_status_2=false; /* used for display  only */
	   }
 }
 /***************************************************************************************/
 
/*Checking The Trip_Status call in while loop */ 
 
 void Trip_Status_Check(void)
 {
	 	uint8_t _temp_t;
	 /* if Relay 1 is Trip  and not in test condtions Then write trip value into  eeprom */
	  
	 if(relay_trip_status_1 && !__relay_test  )
	   {
	   if(!history_flag)
	   	{
			if(Select_Adc_Channel_1_Rms)
			{ 
			
				for(_temp_t=4;_temp_t>0;_temp_t--)
           			Stored_Trip_Data[_temp_t]=	Stored_Trip_Data[_temp_t-1];
				        Stored_Trip_Data[0]=Adc_Rms.RMS_Value[0]; /* stored 5 record for display*/
						    disp_trip = Adc_Rms.RMS_Value[0]; }
				else if(Select_Adc_Channel_2_Rms)
				{ 
					for(_temp_t=4;_temp_t>0;_temp_t--)
           			Stored_Trip_Data[_temp_t]=	Stored_Trip_Data[_temp_t-1];
					Stored_Trip_Data[0]=Adc_Rms.RMS_Value[1]; /* stored 5 record for display*/
							  disp_trip = Adc_Rms.RMS_Value[1]; 
				}
				 
		   PT24xx_write(LAST_FIVE_TRIP_DATA,(uint32_t *)Stored_Trip_Data,20);
				 
	 		 }
	 history_flag = true;
		}
		 		
	
  if(Select_Adc_Channel_1_Rms	==true)
	{	
   /*Checking the Alarm Percentage Set value */
		 
		  if(cfg.set_alarm)
			{
		if(Alarm_value<=Adc_Rms.RMS_Value[0])
		  {
			  Alarm_Set_Flag=true;
		  }
    else 
		  {
         Alarm_Set_Flag=false;
			  //relay_trip_status_2=false;

		   }	
	    }		
			 /*check trip_value*/
		 if(Adc_Rms.RMS_Value[0]>Trip_Value)
			 {
				  Pk_Display_Flag=true;  /* used for pk display */
				 Trip_Value_Status=true;   /* used for start timmer and trip relay display */
				  
				    /* stored 5 record for display*/
			 }
			 else if(!__relay_test)
			 {
				  relay_trip_status_1=false; /* used for rl display  only */
					Trip_Value_Status=false;
			 }
   }
	
	else if(Select_Adc_Channel_2_Rms==true)
   	{
			/*Checking the Alarm Percentage Set value */
            if(cfg.set_alarm)	
						{							
								if(Alarm_value<=Adc_Rms.RMS_Value[1])
									{
										Alarm_Set_Flag=true;
									}
								 else 
		           {
							 Alarm_Set_Flag=false;
						   //relay_trip_status_2=false;
	        	   }
						 }							 
						 /*check trip_value*/
		       if(Adc_Rms.RMS_Value[1]>Trip_Value)
	           {
							 Pk_Display_Flag=true;  /* used for pk display */
		           Trip_Value_Status=true;  /* used for start timmer and trip relay display */
							 
	           }
	         else if(!__relay_test)
			     {
				    relay_trip_status_1=false; /* used for rl display  only */
					  Trip_Value_Status=false;
			    }
		
	  }
	 if(Start_Timmer>delay_trip)
	 {
		 /*set Relay Status to Trip relay */
		 Trip_Relay_1=true;
		
	 }
	 else
	 {
		 Trip_Relay_1=false;
	 }
	 
	  
 }
 
 /***************************************************************************************/
 
 
 /***************************************************************************************/
 uint8_t Display_Bar(void)
  {
	  if(Select_Adc_Channel_1_Rms)
		{
			Display_Bar_per = (uint8_t)((Adc_Rms.RMS_Value[0]*100)/Trip_Value);
		}
		else if(Select_Adc_Channel_2_Rms)
		{
			Display_Bar_per = (uint8_t)((Adc_Rms.RMS_Value[1]*100)/Trip_Value);
		}
	 return Display_Bar_per;
  }
 /***************************************************************************************/
	
/***************************************************************************************/

 
/***************************************************************************************/

 /***************************************************************************************/
/* Displaying and other purpose used functions */
uint32_t read_value (RMS_READING_t id)
{
	uint32_t current_value;
switch(id)
{
	case FAULT_1 : current_value = Stored_Trip_Data[0];
		break;
	case FAULT_2 : current_value = Stored_Trip_Data[1];
		break;
	case FAULT_3	: current_value = Stored_Trip_Data[2];
		break;
	case FAULT_4 : current_value = Stored_Trip_Data[3];
		break;
	case FAULT_5: current_value = Stored_Trip_Data[4];
		break;
	case TRIP_VALUE : current_value = Trip_Value;
	break;
	case  RMS_LEAKAGE_CURRENT:  current_value = (uint32_t)Average_Rms_Display();
		break;
	case  FAULT_VALUE:  current_value = (uint32_t)disp_trip;
		break;	
}

  return current_value;
}


/**************************************************************************************************/


/******************************************************************************************************/

/************************ (C) COPYRIGHT NAGOBA ELECTRONICS NOV 2019 *****END OF FILE****/





