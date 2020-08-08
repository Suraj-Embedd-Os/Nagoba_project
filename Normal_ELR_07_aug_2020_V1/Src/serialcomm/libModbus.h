/*  PetitModbus Version 1.0
 *  Author  :   Firat DEVECI
 *  Date    :   27.07.16
 *  
 *  Ipucu   :
 *  RS485   :	Bu iletisim metodu kullanilirken RX pini pull up yapilmalidir.
 */

#ifndef __PETITMODBUS__H
#define __PETITMODBUS__H

#define NUMBER_OF_OUTPUT_PETITREGISTERS                 300                     // Petit Modbus RTU Slave Output Register Sayisi        
                                                                                // Kullanilacak Register sayisi buradan girilmeli

#define PETITMODBUS_RECEIVE_BUFFER_SIZE                 50                      // Petit Modbus RTU Slave icin kullanilacak buffer boyutu
#define PETITMODBUS_TRANSMIT_BUFFER_SIZE                70
#define PETITMODBUS_RXTX_BUFFER_SIZE                    PETITMODBUS_TRANSMIT_BUFFER_SIZE

#define PETITMODBUS_TIMEOUTTIMER                        250                     // Petit Modbus RTU Slave icin timeout suresi [milisaniye]
extern unsigned char PETITMODBUS_SLAVE_ADDRESS;                                 // Petit Modbus RTU Slave icin adres numarasi [0 to 255]

#define PETITMODBUS_READ_HOLDING_REGISTERS_ENABLED      ( 1 )                   // Kullanilacaksa 1, kullanilmayacaksa 0
#define PETITMODBUSWRITE_SINGLE_REGISTER_ENABLED        ( 0 )                   // Kullanilacaksa 1, kullanilmayacaksa 0
#define PETITMODBUS_WRITE_MULTIPLE_REGISTERS_ENABLED    ( 1 )                   // Kullanilacaksa 1, kullanilmayacaksa 0

extern unsigned char   PetitReceiveBuffer[PETITMODBUS_RECEIVE_BUFFER_SIZE];
extern unsigned char   PetitReceiveCounter;

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

extern void             InitPetitModbus(unsigned char PetitModbusSlaveAddress);
extern unsigned char    PetitSendMessage(void);
extern unsigned char    PetitRxDataAvailable(void);
extern void             ProcessPetitModbus(void);
extern void             HandlePetitModbusReadHoldingRegisters(void);            // ModBus 03
extern void             HandlePetitModbusWriteSingleRegister(void);             // ModBus 06
extern void             HandleMPetitodbusWriteMultipleRegisters(void);          // ModBus 16
extern unsigned char    CheckPetitModbusBufferComplete(void);


#endif
