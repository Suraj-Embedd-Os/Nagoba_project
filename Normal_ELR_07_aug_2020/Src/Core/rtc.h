#ifndef __rtc_h
#define __rtc_h


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>



#ifdef __cplusplus
 extern "C" {
#endif

#define rtc_w (0U) // to write time for rtc
#define RTC_ADDR_HOUR			(0x04)
#define RTC_ADDR_MINUTE		(0x03)
#define RTC_ADDR_SECOND		(0x02)

typedef struct RTCTIME
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	
} rtctime_t;
	
void rtc_init  ( void *i2cPort, const uint8_t devAddr);	
rtctime_t* rtc_getCurrentTime (void);
#if(rtc_w)
void rtc_write(int hour,int min,int sec,int day,int week,int month,int year);
#endif
uint8_t DEC2BCD(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // __rtc_h


