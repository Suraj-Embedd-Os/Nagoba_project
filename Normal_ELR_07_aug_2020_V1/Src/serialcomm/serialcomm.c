

#include "serialcomm.h"
#include "libModbus.h"


#define SERCOMM_RECEIVE_BUFFER_SIZE  PETITMODBUS_RECEIVE_BUFFER_SIZE
void *sercommPort;
void *sercommRxDMAChannel;
uint8_t *rxDMABuffer;
uint32_t rxDMABufferSize;


void sercomm_init ( void *uartPort, void *uartRxDMAChannel)
{
	sercommPort = uartPort;
	sercommRxDMAChannel = uartRxDMAChannel;
	rxDMABuffer = PetitReceiveBuffer;
	rxDMABufferSize = SERCOMM_RECEIVE_BUFFER_SIZE;
	
	
#if (DEVICE_PART_NUMBER == STM32F030Cx8)
	HAL_GPIO_WritePin(DE_PORT, DE_PIN, GPIO_PIN_RESET); // DE Disable
#endif
	// RX request
	HAL_HalfDuplex_EnableReceiver((UART_HandleTypeDef *)sercommPort);
	HAL_UART_Receive_DMA((UART_HandleTypeDef *)sercommPort, rxDMABuffer, rxDMABufferSize);
}
void sercomm_transmit( const uint8_t *txData, const uint32_t txSize )
{

	HAL_HalfDuplex_EnableTransmitter((UART_HandleTypeDef *)sercommPort);
#if (DEVICE_PART_NUMBER == STM32F030Cx8)
	HAL_GPIO_WritePin(DE_PORT, DE_PIN, GPIO_PIN_SET); // DE Enable
#endif

	HAL_UART_Transmit((UART_HandleTypeDef *)sercommPort, (uint8_t *)txData, txSize, 0xffff);
	
#if (DEVICE_PART_NUMBER == STM32F030Cx8)
	HAL_GPIO_WritePin(DE_PORT, DE_PIN, GPIO_PIN_RESET); // DE Disable
#endif
	HAL_HalfDuplex_EnableReceiver((UART_HandleTypeDef *)sercommPort);
}
void sercomm_receive( uint8_t *rxData, const uint32_t rxSize )
{
	HAL_UART_Transmit((UART_HandleTypeDef *)sercommPort, rxData, rxSize, 0xffff);
	HAL_UART_Receive_DMA((UART_HandleTypeDef *)sercommPort, rxDMABuffer, rxDMABufferSize);
}

uint32_t sercomm_getRxCount ( void )
{
	return (rxDMABufferSize - __HAL_DMA_GET_COUNTER((DMA_HandleTypeDef *)sercommRxDMAChannel));
}

void sercomm_resetRxDmaChannel ( void )
{
	DMA_HandleTypeDef *rxDmaCh = (DMA_HandleTypeDef *)sercommRxDMAChannel;
	/* Disable the channel */
	rxDmaCh->Instance->CCR &= ~DMA_CCR_EN;
	/* Clear all flags */
	rxDmaCh->DmaBaseAddress->IFCR  = (DMA_FLAG_GL1 << rxDmaCh->ChannelIndex);
	/* Configure DMA Channel data length */
	rxDmaCh->Instance->CNDTR = (uint32_t)rxDMABufferSize;

	/* Enable the Peripheral */
	rxDmaCh->Instance->CCR |= DMA_CCR_EN;
	/* Change the DMA state*/
	rxDmaCh->State = HAL_DMA_STATE_READY; 
}
