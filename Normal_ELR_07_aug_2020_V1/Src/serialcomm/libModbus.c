#include "serialcomm.h"
#include "libModbus.h"
#include "modbus.h"
#include "setup.h"

/*******************************ModBus Functions*******************************/
#define PETITMODBUS_READ_COILS                  1
#define PETITMODBUS_READ_DISCRETE_INPUTS        2
#define PETITMODBUS_READ_HOLDING_REGISTERS      3
#define PETITMODBUS_READ_INPUT_REGISTERS        4
#define PETITMODBUS_WRITE_SINGLE_COIL           5
#define PETITMODBUS_WRITE_SINGLE_REGISTER       6
#define PETITMODBUS_WRITE_MULTIPLE_COILS        15
#define PETITMODBUS_WRITE_MULTIPLE_REGISTERS    16
/****************************End of ModBus Functions***************************/

#define PETIT_FALSE_FUNCTION                    0
#define PETIT_FALSE_SLAVE_ADDRESS               1
#define PETIT_DATA_NOT_READY                    2
#define PETIT_DATA_READY                        3

#define PETIT_ERROR_CODE_01                     0x01                                    // Function code is not supported
#define PETIT_ERROR_CODE_02                     0x02                                    // Register address is not allowed or write-protected
#define PETIT_ERROR_CODE_03                     0x03                                    // Some data values are out of range, invalid number of register
#define PETIT_ERROR_CODE_06                     0x06                                    // Device can not handle the request at the moment. Repeat the request.
#define PETIT_ERROR_CODE_0B                     0x0B                                    // Error message of the interconnected gateway: No response of the accessed device.

unsigned char PETITMODBUS_SLAVE_ADDRESS         =1;

typedef enum
{
    PETIT_RXTX_IDLE,
    PETIT_RXTX_START,
    PETIT_RXTX_DATABUF,
    PETIT_RXTX_WAIT_ANSWER,
    PETIT_RXTX_TIMEOUT
}PETIT_RXTX_STATE;

typedef struct
{
  unsigned char     Address;
  unsigned char     Function;
  unsigned char     DataBuf[PETITMODBUS_RXTX_BUFFER_SIZE];
  unsigned int      DataLen;
}PETIT_RXTX_DATA;

/**********************Slave Transmit and Receive Variables********************/
PETIT_RXTX_DATA     Petit_Tx_Data;
unsigned int        Petit_Tx_Current              = 0;
unsigned int        Petit_Tx_CRC16                = 0xFFFF;
PETIT_RXTX_STATE    Petit_Tx_State                = PETIT_RXTX_IDLE;
unsigned char       Petit_Tx_Buf[PETITMODBUS_TRANSMIT_BUFFER_SIZE];
unsigned int        Petit_Tx_Buf_Size             = 0;

PETIT_RXTX_DATA     Petit_Rx_Data;
unsigned int        Petit_Rx_CRC16                = 0xFFFF;
PETIT_RXTX_STATE    Petit_Rx_State                = PETIT_RXTX_IDLE;
unsigned char       Petit_Rx_Data_Available       = FALSE;

unsigned char   PetitReceiveBuffer[PETITMODBUS_RECEIVE_BUFFER_SIZE];
unsigned char   PetitReceiveCounter=0;

/****************End of Slave Transmit and Receive Variables*******************/

/*
 * Function Name        : CRC16
 * @param[in]           : Data  - CRC'si hesaplanacak data
 * @param[in/out]       : CRC   - Anlik CRC degeri
 * @How to use          : CRC'si hesaplanacak dizinin degerleri sirasiyla bu
 *                        fonksiyona girmeli, sonuc olarak her seferde CRC
 *                        geri dondurulecek. Baslangic 0xFFFF ile baslamali.
 */
void Petit_CRC16(const unsigned char Data, unsigned int* MIO_CRC)
{
    unsigned int i;

    *MIO_CRC = *MIO_CRC ^(unsigned int) Data;
    for (i = 8; i > 0; i--)
    {
        if (*MIO_CRC & 0x0001)
            *MIO_CRC = (*MIO_CRC >> 1) ^ 0xA001;
        else
            *MIO_CRC >>= 1;
    }
}

/******************************************************************************/

/*
 * Function Name        : DoTx
 * @param[out]          : TRUE
 * @How to use          : Gonderilecek dizi bu fonksiyon ile gonderilir.
 */
unsigned char Petit_DoSlaveTX(void)
{ 
		sercomm_transmit(Petit_Tx_Buf,Petit_Tx_Buf_Size);

    Petit_Tx_Buf_Size = 0;
    return TRUE;
}

/******************************************************************************/

/*
 * Function Name        : SendMessage
 * @param[out]          : TRUE/FALSE
 * @How to use          : Hazirlanan mesaji gonder komutu bu fonksiyon ile verilir.
 */
unsigned char PetitSendMessage(void)
{
    if (Petit_Tx_State != PETIT_RXTX_IDLE)
        return FALSE;

    Petit_Tx_Current  =0;
    Petit_Tx_State    =PETIT_RXTX_START;

    return TRUE;
}

/******************************************************************************/

/*
 * Function Name        : HandleModbusError
 * @How to use          : ModBus master'a hata mesaji gonderilir.
 */
void HandlePetitModbusError(char ErrorCode)
{
    // Initialise the output buffer. The first byte in the buffer says how many registers we have read
    Petit_Tx_Data.Function    = ErrorCode | 0x80;
    Petit_Tx_Data.Address     = PETITMODBUS_SLAVE_ADDRESS;
    Petit_Tx_Data.DataLen     = 0;
    PetitSendMessage();
}

/******************************************************************************/

/*
 * Function Name        : HandleModbusReadHoldingRegisters
 * @How to use          : Modbus function 03 - Read holding registers
 */
#if PETITMODBUS_READ_HOLDING_REGISTERS_ENABLED > 0
void HandlePetitModbusReadHoldingRegisters(void)
{
    // Holding registers are effectively numerical outputs that can be written to by the host.
    // They can be control registers or analogue outputs.
    // We potientially have one - the pwm output value
    unsigned int    Petit_StartAddress        = 0;
    unsigned int    Petit_NumberOfRegisters   = 0;
    unsigned int    Petit_i                   = 0;

    // The message contains the requested start address and number of registers
    Petit_StartAddress        = ((unsigned int) (Petit_Rx_Data.DataBuf[0]) << 8) + (unsigned int) (Petit_Rx_Data.DataBuf[1]);
    Petit_NumberOfRegisters   = ((unsigned int) (Petit_Rx_Data.DataBuf[2]) << 8) + (unsigned int) (Petit_Rx_Data.DataBuf[3]);

    if((Petit_StartAddress+Petit_NumberOfRegisters)>NUMBER_OF_OUTPUT_PETITREGISTERS)
        HandlePetitModbusError(PETIT_ERROR_CODE_03);
    else
    {
        // Initialise the output buffer. The first byte in the buffer says how many registers we have read
        Petit_Tx_Data.Function    = PETITMODBUS_READ_HOLDING_REGISTERS;
        Petit_Tx_Data.Address     = PETITMODBUS_SLAVE_ADDRESS;
        Petit_Tx_Data.DataLen     = 1;
        Petit_Tx_Data.DataBuf[0]  = 0;

        for (Petit_i = 0; Petit_i < Petit_NumberOfRegisters; Petit_i++)
        {
            unsigned short Petit_CurrentData = modbus_getRegValue(Petit_StartAddress+Petit_i);//PetitRegisters[Petit_StartAddress+Petit_i].ActValue;

            Petit_Tx_Data.DataBuf[Petit_Tx_Data.DataLen]        = (unsigned char) ((Petit_CurrentData & 0xFF00) >> 8);
            Petit_Tx_Data.DataBuf[Petit_Tx_Data.DataLen + 1]    = (unsigned char) (Petit_CurrentData & 0xFF);
            Petit_Tx_Data.DataLen                        += 2;
            Petit_Tx_Data.DataBuf[0]                      = Petit_Tx_Data.DataLen - 1;
        }

        PetitSendMessage();
    }
}
#endif

/******************************************************************************/

/*
 * Function Name        : HandleModbusReadInputRegisters
 * @How to use          : Modbus function 06 - Write single register
 */
#if PETITMODBUSWRITE_SINGLE_REGISTER_ENABLED > 0
void HandlePetitModbusWriteSingleRegister(void)
{
    // Write single numerical output
    unsigned int    Petit_Address   = 0;
    unsigned int    Petit_Value     = 0;
    unsigned char   Petit_i         = 0;
		unsigned int Petit_CurrentData;

    // The message contains the requested start address and number of registers
    Petit_Address   = ((unsigned int) (Petit_Rx_Data.DataBuf[0]) << 8) + (unsigned int) (Petit_Rx_Data.DataBuf[1]);
    Petit_Value     = ((unsigned int) (Petit_Rx_Data.DataBuf[2]) << 8) + (unsigned int) (Petit_Rx_Data.DataBuf[3]);

    // Initialise the output buffer. The first byte in the buffer says how many registers we have read
    Petit_Tx_Data.Function    = PETITMODBUS_WRITE_SINGLE_REGISTER;
    Petit_Tx_Data.Address     = PETITMODBUS_SLAVE_ADDRESS;
    Petit_Tx_Data.DataLen     = 4;

    if(Petit_Address>=NUMBER_OF_OUTPUT_PETITREGISTERS)
        HandlePetitModbusError(PETIT_ERROR_CODE_02);
    else
    {
//        PetitRegisters[Petit_Address].ActValue=Petit_Value;
				Petit_CurrentData = (Petit_Rx_Data.DataBuf[2] * 256) + Petit_Rx_Data.DataBuf[3];
					modbus_writeRegValue(Petit_Address+Petit_i, Petit_CurrentData);
        // Output data buffer is exact copy of input buffer
        for (Petit_i = 0; Petit_i < 4; ++Petit_i)			
            Petit_Tx_Data.DataBuf[Petit_i] = Petit_Rx_Data.DataBuf[Petit_i];
    }

    PetitSendMessage();
}
#endif

/******************************************************************************/

/*
 * Function Name        : HandleModbusWriteMultipleRegisters
 * @How to use          : Modbus function 16 - Write multiple registers
 */
#if PETITMODBUS_WRITE_MULTIPLE_REGISTERS_ENABLED > 0
void HandleMPetitodbusWriteMultipleRegisters(void)
{
    // Write single numerical output
    unsigned int    Petit_StartAddress            =0;
    unsigned int    Petit_NumberOfRegisters       =0;
    unsigned char   Petit_i                       =0;
    unsigned int		Petit_Value                   =0;

    // The message contains the requested start address and number of registers
    Petit_StartAddress        = ((unsigned int) (Petit_Rx_Data.DataBuf[0]) << 8) + (unsigned int) (Petit_Rx_Data.DataBuf[1]);
    Petit_NumberOfRegisters   = ((unsigned int) (Petit_Rx_Data.DataBuf[2]) << 8) + (unsigned int) (Petit_Rx_Data.DataBuf[3]);

    if((Petit_StartAddress+Petit_NumberOfRegisters)>NUMBER_OF_OUTPUT_PETITREGISTERS)
        HandlePetitModbusError(PETIT_ERROR_CODE_02);
    else
    {
        // Initialise the output buffer. The first byte in the buffer says how many outputs we have set
        Petit_Tx_Data.Function    = PETITMODBUS_WRITE_MULTIPLE_REGISTERS;
        Petit_Tx_Data.Address     = PETITMODBUS_SLAVE_ADDRESS;
        Petit_Tx_Data.DataLen     = 4;
        Petit_Tx_Data.DataBuf[0]  = Petit_Rx_Data.DataBuf[0];
        Petit_Tx_Data.DataBuf[1]  = Petit_Rx_Data.DataBuf[1];
        Petit_Tx_Data.DataBuf[2]  = Petit_Rx_Data.DataBuf[2];
        Petit_Tx_Data.DataBuf[3]  = Petit_Rx_Data.DataBuf[3];

        // Output data buffer is exact copy of input buffer
        for (Petit_i = 0; Petit_i <Petit_NumberOfRegisters; Petit_i++)
        {
            Petit_Value=(Petit_Rx_Data.DataBuf[5+2*Petit_i]<<8)+(Petit_Rx_Data.DataBuf[6+2*Petit_i]);
						modbus_writeMultipleRegValue(Petit_StartAddress+Petit_i,Petit_Value);
					//            PetitRegisters[Petit_StartAddress+Petit_i].ActValue=Petit_Value;
        }

        PetitSendMessage();
    }
}
#endif

/******************************************************************************/

/*
 * Function Name        : RxDataAvailable
 * @return              : Datalar hazirsa TRUE
 *                        Datalar hazir degilse FALSE
 */
unsigned char Petit_RxDataAvailable(void)
{
    unsigned char Result    = Petit_Rx_Data_Available;
    
    Petit_Rx_Data_Available       = FALSE;

    return Result;
}

/******************************************************************************/

/*
 * Function Name        : CheckRxTimeout
 * @return              : Zaman asimi olmussa TRUE
 *                        Zaman asimi olmamissa FALSE

unsigned char Petit_CheckRxTimeout(void)
{
    // A return value of true indicates there is a timeout    
    if (PetitModbusTimerValue>= PETITMODBUS_TIMEOUTTIMER)
    {
        PetitModbusTimerValue   =0;
        PetitReceiveCounter     =0;
        return TRUE;
    }

    return FALSE;
}
 */
/******************************************************************************/

/*
 * Function Name        : CheckBufferComplete
 * @return              : Alim kesmesinden bilgiler dogru gelmise   DATA_READY
 *                        Alim kesmesinden slave adresi yanlissa    FALSE_SLAVE_ADDRESS
 *                        Alim kesmesinden bilgiler daha aliniyorsa DATA_NOT_READY
 *                        Alim kesmesinden fonksiyon yanlissa       FALSE_FUNCTION
 */
unsigned char CheckPetitModbusBufferComplete(void)
{
    int PetitExpectedReceiveCount=0;

    if(PetitReceiveCounter>4)
    {
        if(PetitReceiveBuffer[0]==PETITMODBUS_SLAVE_ADDRESS)
        {
            if(PetitReceiveBuffer[1]==0x01 || PetitReceiveBuffer[1]==0x02 || PetitReceiveBuffer[1]==0x03 || PetitReceiveBuffer[1]==0x04 || PetitReceiveBuffer[1]==0x05 || PetitReceiveBuffer[1]==0x06)  // RHR
            {
                PetitExpectedReceiveCount    =8;
            }
            else if(PetitReceiveBuffer[1]==0x0F || PetitReceiveBuffer[1]==0x10)
            {
                PetitExpectedReceiveCount=PetitReceiveBuffer[6]+9;
            }
            else
            {
                PetitReceiveCounter=0;
                return PETIT_FALSE_FUNCTION;   // beklenmeyen fonksiyon
            }
        }
        else
        {
            PetitReceiveCounter=0;
            return PETIT_FALSE_SLAVE_ADDRESS;   // beklenmeyen adres
        }
    }
    else
        return PETIT_DATA_NOT_READY;       // Cevap bitmedi

    if(PetitReceiveCounter==PetitExpectedReceiveCount)
    {
        return PETIT_DATA_READY;       // cevap bitti.
    }

    return PETIT_DATA_NOT_READY;
}

/******************************************************************************/

/*
 * Function Name        : RxRTU
 * @How to use          : Surekli dongu icerisinde kalmali. Gonderilecek data
 *                        hazirsa bu data gonderilir.
 */
void Petit_RxRTU(void)
{
    unsigned char   Petit_i;
    unsigned char   Petit_ReceiveBufferControl=0;

		PetitReceiveCounter = sercomm_getRxCount();
    Petit_ReceiveBufferControl    =CheckPetitModbusBufferComplete();

    if(Petit_ReceiveBufferControl==PETIT_DATA_READY)
    {
        Petit_Rx_Data.Address               =PetitReceiveBuffer[0];
        Petit_Rx_CRC16                      = 0xffff;
        Petit_CRC16(Petit_Rx_Data.Address, &Petit_Rx_CRC16);
        Petit_Rx_Data.Function              =PetitReceiveBuffer[1];
        Petit_CRC16(Petit_Rx_Data.Function, &Petit_Rx_CRC16);

        Petit_Rx_Data.DataLen=0;

        for(Petit_i=2;Petit_i<PetitReceiveCounter;Petit_i++)
            Petit_Rx_Data.DataBuf[Petit_Rx_Data.DataLen++]=PetitReceiveBuffer[Petit_i];

        Petit_Rx_State =PETIT_RXTX_DATABUF;

        PetitReceiveCounter=0;
    }

    //Petit_CheckRxTimeout();

    if ((Petit_Rx_State == PETIT_RXTX_DATABUF) && (Petit_Rx_Data.DataLen >= 2))
    {
        // Finish off our CRC check
        Petit_Rx_Data.DataLen -= 2;
        for (Petit_i = 0; Petit_i < Petit_Rx_Data.DataLen; ++Petit_i)
        {
            Petit_CRC16(Petit_Rx_Data.DataBuf[Petit_i], &Petit_Rx_CRC16);
        }
        
        if (((unsigned int) Petit_Rx_Data.DataBuf[Petit_Rx_Data.DataLen] + ((unsigned int) Petit_Rx_Data.DataBuf[Petit_Rx_Data.DataLen + 1] << 8)) == Petit_Rx_CRC16)
        {
            // Valid message!
            Petit_Rx_Data_Available = TRUE;
					
					sercomm_resetRxDmaChannel();
        }

        Petit_Rx_State = PETIT_RXTX_IDLE;
    }
		
		
}

/******************************************************************************/

/*
 * Function Name        : TxRTU
 * @How to use          : Gonderim bilgisi hazirsa bu fonksiyon ile gonderim yapilir.
 */
void Petit_TxRTU(void)
{
    Petit_Tx_CRC16                =0xFFFF;
    Petit_Tx_Buf_Size             =0;
    Petit_Tx_Buf[Petit_Tx_Buf_Size++]   =Petit_Tx_Data.Address;
    Petit_CRC16(Petit_Tx_Data.Address, &Petit_Tx_CRC16);
    Petit_Tx_Buf[Petit_Tx_Buf_Size++]   =Petit_Tx_Data.Function;
    Petit_CRC16(Petit_Tx_Data.Function, &Petit_Tx_CRC16);

    for(Petit_Tx_Current=0; Petit_Tx_Current < Petit_Tx_Data.DataLen; Petit_Tx_Current++)
    {
        Petit_Tx_Buf[Petit_Tx_Buf_Size++]=Petit_Tx_Data.DataBuf[Petit_Tx_Current];
        Petit_CRC16(Petit_Tx_Data.DataBuf[Petit_Tx_Current], &Petit_Tx_CRC16);
    }
    
    Petit_Tx_Buf[Petit_Tx_Buf_Size++] = Petit_Tx_CRC16 & 0x00FF;
    Petit_Tx_Buf[Petit_Tx_Buf_Size++] =(Petit_Tx_CRC16 & 0xFF00) >> 8;

		Petit_DoSlaveTX();

    Petit_Tx_State    =PETIT_RXTX_IDLE;
}

/******************************************************************************/

/*
 * Function Name        : ProcessModbus
 * @How to use          : ModBus cekirdegi. Bu fonksiyon ana dongu icerisinde
 *                        cagrilmali.
 */
void ProcessPetitModbus(void)
{
	if (Petit_Rx_Data.Address == PETITMODBUS_SLAVE_ADDRESS)
	{
    if (Petit_Tx_State != PETIT_RXTX_IDLE)                                                  // Gonderim datalari hazirsa gonderim baslatilir
        Petit_TxRTU();
	}

    Petit_RxRTU();                                                                    // Alim fonksiyonu surekli isletilir

    if (Petit_RxDataAvailable())                                                      // Data alimi tamamlanmissa bu foksiyona girilir
    {
        if (Petit_Rx_Data.Address == PETITMODBUS_SLAVE_ADDRESS)                                   // Data bizim icin mi?
        {
            switch (Petit_Rx_Data.Function)                                           // Data bizim icinse hangi fonksiyon isletilecek?
            {
                #if PETITMODBUS_READ_HOLDING_REGISTERS_ENABLED > 0
                case PETITMODBUS_READ_HOLDING_REGISTERS:    {   HandlePetitModbusReadHoldingRegisters();        break;  }
                #endif
                #if PETITMODBUSWRITE_SINGLE_REGISTER_ENABLED > 0
                case PETITMODBUS_WRITE_SINGLE_REGISTER:     {   HandlePetitModbusWriteSingleRegister();         break;  }
                #endif
                #if PETITMODBUS_WRITE_MULTIPLE_REGISTERS_ENABLED > 0
                case PETITMODBUS_WRITE_MULTIPLE_REGISTERS:  {   HandleMPetitodbusWriteMultipleRegisters();      break;  }
                #endif
                default:                                    {   HandlePetitModbusError(PETIT_ERROR_CODE_06);    break;  }
            }
        }
    }
}

/******************************************************************************/

/*
 * Function Name        : InitPetitModbus
 * @How to use          : Petite ModBus slave calismasi icin donanimsal ilklendirmeleri.
 */
void InitPetitModbus(unsigned char PetitModbusSlaveAddress)
{
    if(PetitModbusSlaveAddress<=0 && PetitModbusSlaveAddress>255)
        PetitModbusSlaveAddress=1;

    PETITMODBUS_SLAVE_ADDRESS    =PetitModbusSlaveAddress;
    
}

/******************************************************************************/
