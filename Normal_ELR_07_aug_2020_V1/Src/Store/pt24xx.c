#include "pt24xx.h"
#include "stdlib.h"
#include "string.h"



static void *PT24xxPort;
static uint8_t PT24xxDevAddr;

uint8_t wr_Ind, rd_Ind;
void PT24xx_init (void *i2cPort, const uint8_t devAddr )
{
	PT24xxPort = i2cPort;
	PT24xxDevAddr = devAddr;
}

int PT24xx_read( 	uint16_t byte_address, 
										 uint32_t *data, 
										 int32_t length )
{
	uint16_t i2c_address = PT24xxDevAddr | 0x01;
	
	//read
	while(length>0)
	{
		wr_Ind = HAL_I2C_Mem_Read(PT24xxPort,i2c_address,byte_address,1,(uint8_t *)data,4,HAL_MAX_DELAY);
		 HAL_Delay(4);
			length = length - 4;
		byte_address= byte_address+4;
			data++;
	}
	
	if(length <= 0)
		return 0;
	else 
		return -1;
	
 
}

int PT24xx_write( uint16_t byte_address, 
										uint32_t *data, 
										int32_t length )
{ 		
	// Write
	while(length>0)
	{
		wr_Ind = HAL_I2C_Mem_Write(PT24xxPort,PT24xxDevAddr,byte_address,1,(uint8_t *)data,4,HAL_MAX_DELAY);
		 HAL_Delay(4);
			length = length - 4;
		byte_address= byte_address +4;
			data++;
	}
	
	if(length <= 0)
		return 0;
	else 
		return -1;
 
}			
