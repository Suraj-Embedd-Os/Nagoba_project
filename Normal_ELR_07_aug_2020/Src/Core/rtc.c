#include "rtc.h"
#include "main.h"
void *rtcPort;
static uint8_t rtcDevAddr;
static rtctime_t rtctime;

uint8_t send_data[7];

void rtc_init  ( void *i2cPort, const uint8_t devAddr)
{
	rtcPort = i2cPort;
	rtcDevAddr = devAddr;
}



uint8_t DEC2BCD(uint8_t data)
{
	return (data /10)<<4 | (data%10);
}
/**/
rtctime_t* rtc_getCurrentTime (void)
{
	uint16_t rdAddr = rtcDevAddr | 0x1;
	
	//For Writing to RTC : 0xA2
	//For Reading from RTC : 0xA3
	// Reading hour from RTC address : seconds = 0x02, minutes = 0x03, hour = 0x04
	HAL_I2C_Mem_Read(rtcPort,rdAddr,RTC_ADDR_HOUR,I2C_MEMADD_SIZE_8BIT,&rtctime.hour,1,1);	
	HAL_I2C_Mem_Read(rtcPort,rdAddr,RTC_ADDR_MINUTE,I2C_MEMADD_SIZE_8BIT,&rtctime.minute,1,1);	
	HAL_I2C_Mem_Read(rtcPort,rdAddr,RTC_ADDR_SECOND,I2C_MEMADD_SIZE_8BIT,&rtctime.second,1,1);	
	
	return (&rtctime);

}

#if(rtc_w)
void rtc_write(int hour,int min,int sec,int day,int week,int month,int year)
{
	send_data[0] = DEC2BCD(sec);													//seconds (0 - 59)
	send_data[1] = DEC2BCD(min);													//minutes (0 - 59)
	send_data[2] = DEC2BCD(hour);													//hours (0 - 23)
	send_data[3] = DEC2BCD(day);													//day (1 - 31)
	send_data[4] = DEC2BCD(week);													//weekday(0 - 6)
	send_data[5] = DEC2BCD(month);													//month (1 - 12)
	send_data[6] = DEC2BCD(year);													// year ( 0 - 99)
	
	HAL_I2C_Mem_Write(rtcPort,rtcDevAddr,0x02,I2C_MEMADD_SIZE_8BIT,send_data,7,900);
}
#endif
