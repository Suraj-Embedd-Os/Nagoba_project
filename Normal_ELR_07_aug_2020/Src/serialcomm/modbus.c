
#include "Rms.h"
#include "modbus.h"
#include "setup.h"
#include "pt24xx.h"

float par_val;
int8_t enter_setup = 0;
uint8_t save_cfg;
uint32_t original_value;
extern uint32_t minValue,maxValue;

//cfg.set_delay
//Trip_Value = xxx; 
//Alarm_value=(Trip_Value*xxxx)/100;
void modbus_init( uint8_t slvAddr )
{
	InitPetitModbus(slvAddr);
}

uint16_t modbus_getRegValue ( uint32_t regNum )
{
	uint32_t *ptrFlt;
	uint32_t fltVal;
	
	//MFM_FMD_t* mfmd = meter_getFMDHandle();
	
	regNum += 40001;
							
				switch ( regNum )
				{	
						
					//WRITE REGISTERS
								case MB_WREG_ADDR_TRIP_VALUE:
								case MB_WREG_ADDR_TRIP_VALUE+1:
									fltVal =	cfg.trip_value;									
								break;
									
								case MB_WREG_ADDR_ALARM_PERCENTAGE:
								case MB_WREG_ADDR_ALARM_PERCENTAGE+1:
									fltVal =	cfg.set_alarm_value;
								break;
									
								case MB_WREG_ADDR_RMS_DELAY:
								case MB_WREG_ADDR_RMS_DELAY+1:
										fltVal = cfg.set_delay;
								break;
								
								case MB_WREG_ADDR_ADDRESS:
								case MB_WREG_ADDR_ADDRESS+1:
									fltVal = cfg.address;
								break;
									
								case MB_WREG_ADDR_BAUD:
								case MB_WREG_ADDR_BAUD+1:
										fltVal = cfg.buad_rate;
								break;
									
								case MB_WREG_ADDR_PARITY:
								case MB_WREG_ADDR_PARITY+1:
										fltVal = cfg.parity;
								break;
					
					
					
					
					
											// Group 1: VAP (RMS Voltage/Amps, Power Factor)
											case MB_REG_ADDR_RMS_SENS_I:
											case (MB_REG_ADDR_RMS_SENS_I+1):
												fltVal = (uint32_t)read_value(RMS_LEAKAGE_CURRENT);
												break;
											case MB_REG_ADDR_TRIP_VALUE:
											case (MB_REG_ADDR_TRIP_VALUE+1):
												fltVal = (uint32_t)read_value(TRIP_VALUE);
												break;
											case MB_REG_ADDR_ALARM_PERCENTAGE: 
											case (MB_REG_ADDR_ALARM_PERCENTAGE+1):
												fltVal = (uint32_t) cfg.set_alarm_value;
												break;
											case MB_REG_ADDR_RMS_DELAY:
											case (MB_REG_ADDR_RMS_DELAY+1):
												fltVal = (uint32_t)cfg.set_delay;
												break; 
											case MB_REG_ADDR_FAULT_1: 
											case (MB_REG_ADDR_FAULT_1+1):
												fltVal = (uint32_t)read_value(FAULT_1);
												break;
											case MB_REG_ADDR_FAULT_2:
											case (MB_REG_ADDR_FAULT_2+1):
												fltVal = (uint32_t)read_value(FAULT_2);
												break;
											case MB_REG_ADDR_FAULT_3:
											case (MB_REG_ADDR_FAULT_3+1):
												fltVal = (uint32_t) read_value(FAULT_3);
												break;
											case MB_REG_ADDR_FAULT_4: 
											case (MB_REG_ADDR_FAULT_4+1):
												fltVal = (uint32_t)read_value(FAULT_4);
												break;  
											case MB_REG_ADDR_FAULT_5: 
											case (MB_REG_ADDR_FAULT_5+1):
												fltVal = (uint32_t)read_value(FAULT_5);
												break;
												
											default:
												break;
}
						if(regNum > MB_WREG_ADDR_PARITY + 2 && regNum < MB_REG_ADDR_RMS_SENS_I)
						{								
										fltVal = 0;						
						}
				
	ptrFlt = (uint32_t *)&fltVal;
	
	
	if( regNum & 0x1 )	// Upper 16-bit first (Odd register number)
	{
		return (uint16_t)(( *ptrFlt & 0xffff0000)>>16 );
	}
	else	// Lower 16-bit first (Even register number)
	{
		return (uint16_t)( *ptrFlt & 0x0000ffff );
	}
	//return (uint16_t)fltVal;
		
}


void modbus_writeMultipleRegValue ( uint32_t regNum , uint16_t value)
{
	regNum += 40001;
								switch ( regNum )
								{
								case MB_WREG_ADDR_TRIP_VALUE:
									original_value = value;
								break;
								case MB_WREG_ADDR_TRIP_VALUE+1:
									original_value = (original_value * 10000) + value;
									if(original_value >= minValue && original_value <= maxValue)
										cfg.trip_value = original_value;
									else 
										cfg.trip_value = DEFAULT_SET_VALUE;
								break;
									
								case MB_WREG_ADDR_ALARM_PERCENTAGE:
									original_value = value;
								break;
								case MB_WREG_ADDR_ALARM_PERCENTAGE+1:
									original_value = (original_value * 10000) + value;
									if(original_value >= 50 && original_value <= 80)
										cfg.set_alarm_value = original_value;
									else 
										cfg.set_alarm_value = DEFAULT_ALARM;
								break;
									
								case MB_WREG_ADDR_RMS_DELAY:
									original_value = value;
								break;
								case MB_WREG_ADDR_RMS_DELAY+1:
									original_value = (original_value * 10000) + value;
								if(original_value == 150 || original_value == 300 || original_value ==600 || original_value == 750 || 
									original_value == 900|| original_value == 1200 || original_value ==1500 || original_value == 1650 ||
								original_value == 1800 || original_value == 2100 || original_value == 2250 || original_value == 2400||
								original_value == 2700 || original_value == 2850 || original_value ==3000)
										cfg.set_delay = original_value;
									else 
										cfg.set_delay = DEFAULT_DELAY;
								break;
								
								case MB_WREG_ADDR_ADDRESS:
									original_value = value;
								break;
								case MB_WREG_ADDR_ADDRESS+1:
									original_value = (original_value * 10000) + value;
								if(original_value > 0 && original_value < 226)					
										cfg.address = original_value;
								else
									cfg.address = DEFAULT_MOD_ADRRES;
								break;
									
								case MB_WREG_ADDR_BAUD:
									original_value = value;
								break;
								case MB_WREG_ADDR_BAUD+1:
									original_value = (original_value * 10000) + value;
								if(original_value == 4800 || original_value == 9600 || original_value == 19200 || original_value == 38400)
										cfg.buad_rate 	= original_value;
									else
										cfg.buad_rate 	= DEFAULT_BUAD_RATE;
								break;
									
								case MB_WREG_ADDR_PARITY:
									original_value = value;
								break;
								case MB_WREG_ADDR_PARITY+1:
									original_value = (original_value * 10000) + value;
								if(original_value > 0 && original_value < 3)
										cfg.parity 	= original_value;
									else
										cfg.parity 	= DEFAULT_PARITY;
								break;
								
								default:
												break;
								}
}

	
/************************ (C) COPYRIGHT Nagoba Electronics *****END OF FILE****/




