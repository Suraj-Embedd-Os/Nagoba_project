
/**
  ******************************************************************************
  * @file           : mfm_modbus.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the serial comm.
  ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODBUS_H
#define __MODBUS_H

 #include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "libModbus.h"

#ifdef __cplusplus
 extern "C" {
#endif

/* Modbus Holding Registers (All floats, 2*16-bit)
*/
#define MB_REG_ADDR_RMS_SENS_I	(40101U)
#define MB_REG_ADDR_TRIP_VALUE	(40103U)
#define MB_REG_ADDR_ALARM_PERCENTAGE	(40105U)
#define MB_REG_ADDR_RMS_DELAY (40107U)
#define MB_REG_ADDR_FAULT_1	(40109U)
#define MB_REG_ADDR_FAULT_2	(40111U)
#define MB_REG_ADDR_FAULT_3	(40113U)
#define MB_REG_ADDR_FAULT_4	(40115U)
#define MB_REG_ADDR_FAULT_5	(40117U)

//Address for Write Registers
#define MB_WREG_ADDR_TRIP_VALUE	(40001U)
#define MB_WREG_ADDR_ALARM_PERCENTAGE	(40003U)
#define MB_WREG_ADDR_RMS_DELAY	(40005U)
#define MB_WREG_ADDR_ADDRESS	(40007U)
#define MB_WREG_ADDR_BAUD	(40009U)
#define MB_WREG_ADDR_PARITY	(40011U)
#define MB_WREG_ADDR_YEAR (40013U)
#define MB_WREG_ADDR_MONTH (40015U)
#define MB_WREG_ADDR_DATE (40017U)
#define MB_WREG_ADDR_HOUR (40019U)
#define MB_WREG_ADDR_MINUTE (40021U)

void modbus_init( uint8_t slvAddr );
uint16_t modbus_getRegValue ( uint32_t regNum );
void modbus_writeMultipleRegValue ( uint32_t regNum , uint16_t value);



#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_H */

/************************ (C) COPYRIGHT Nagoba Electronics *****END OF FILE****/
