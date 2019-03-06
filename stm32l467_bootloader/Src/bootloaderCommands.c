/*
 * bootloaderCommands.c
 *
 *  Created on: 5 Mar 2019
 *      Author: emirs
 */


#include "bootloaderCommands.h"

uint8_t blRxBuffer[BL_RX_LEN];


/*Bootloader command handle function prototypes*/
void bootloaderHandleGET_VER_cmd 			(uint8_t *cmd);
void bootloaderHandleGET_HELP_cmd 			(uint8_t *cmd);
void bootloaderHandleGET_CID_cmd 			(uint8_t *cmd);
void bootloaderHandleGET_RDP_cmd 			(uint8_t *cmd);
void bootloaderHandleGO_TO_ADDR_cmd 		(uint8_t *cmd);
void bootloaderHandleFLASH_ERASE_cmd 		(uint8_t *cmd);
void bootloaderHandleMEM_WRITE_cmd 			(uint8_t *cmd);
void bootloaderHandleMEM_READ_cmd 			(uint8_t *cmd);
void bootloaderHandleENDIS_RW_PROTECT_cmd 	(uint8_t *cmd);
void bootloaderHandleSECTOR_STATUS_cmd 		(uint8_t *cmd);
void bootloaderHandleOTP_READ_cmd 			(uint8_t *cmd);
void bootloaderHandleDIS_RW_PROTECT_cmd 	(uint8_t *cmd);

/*Bootloader Flow Handle functions*/
void bootLoaderSendAck 						(uint8_t cmd, uint8_t followLength);
void bootloaderSendNack						(void);
uint8_t bootLoaderVerifyCRC(uint8_t *pData, uint32_t length, uint32_t crcHost);
uint8_t getBootLoaderVersion 				(void);
void bootLoaderUART_writeData 				(uint8_t *pBuffer, uint32_t length);


bool isUserButtonPushed(void)
{
	if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET)
	{
		printMsg("Note: User Button has been pushed\r\n"
				"Executing BootLoader Mode");
		return true;
	}
	else
	{
		printMsg("Note: User Button has not been pushed\r\n"
						"Executing normal application");
		return false;
	}
}

/*****************************************************************************************
 * Brief: Function to jump yo user application
 * Arguments: none
 * Returns: none
 * Assuming the FLASH_SECTOR2_BASE_ADDRESS is where the
 * user application is stored
 *****************************************************************************************/
void bootLoaderJumpToUserApp (void)
{
	/*define a function pointer to hold the adress of the RESET Handler of the user app.*/
	void (*appResetHandler) (void);
	printMsg("BL_DEBUG_MSG: bootLoaderJumpToUserApp \r\n");

	/*configure the MSP by  reading the value from the base address of sector 2 0x0800-8000
	 * 0x0800-8000 adress holds the Main stack pointer
	 * Assume the FLASH_SECTOR2_BASE_ADDRESS is the adress where user application is stored*/
	uint32_t mspValue = *(volatile uint32_t *) FLASH_SECTOR2_BASE_ADDRESS;						//type cast a 32bit unsigned integer and dereference the pinter to get address contents
	printMsg("BL_DEBUG_MSG: MSP value = %#x\r\n", mspValue);

	/*cmsis function to set the Main Stack Pointer Value*/
	__set_MSP(mspValue);

	//SCB->VTOR = FLASH_SECTOR1_BASE_ADDRESS;

	/*Now we fetch the RESET HANDLER ADDRESS of the User APLICATION
	 * from the location FLASH_SECTOR2_BASE_ADDRESS+4*/
	uint32_t resetHandlerAddress = *(volatile uint32_t *) (FLASH_SECTOR2_BASE_ADDRESS + 4);		//type cast a 32bit unsigned integer and dereference the pinter to get address contents

	/*initializee a function pointer with reset handler address from FLASH_SECTOR2_BASE_ADDRESS + 4*/
	appResetHandler = (void *) resetHandlerAddress;

	printMsg("BL_DEBUG_MSG: appResetHandler address = %#x\r\n");
	/*jump to reset handler of the application, user application will now take control of the functionality
	 * the reset handler address wis then put into the PC (Program counter) and the code will be executed from this address */
	appResetHandler();																			//jump to the initialized address of the reset handler

	/*Note: the reset handler of the the application can be found in the start up .s file in this case "startup_stm32l476xx.s line 78
	 * from the reset handler the CopyDataInit , LoopCopyDataInit, FillZerobss, LoopFillZerobss are called
	 * in LoopFillZerobss SystemInit is called, here the vector table is shifted/relocated using the SCB->VTOR
	 * The FLASH_BASE is  is by default 0x0800-0000, we should not chnage this, but we can change the VECT_TAB_OFFSET which is by default 0x00
	 * the VECT_TAB_OFFSET must be a multiple of 0x200 as stated by STM32, the VECT_TAB_OFFSET should be set to 0x8000
	 *
	 * SystemInit can be found in system_stm32l4xx.c*/

}

/******************************************************************************
 * Brief: The function in which the bootloader reads Ccommands sent by the host
 * Arguments: none
 * Returns: none
 * *****************************************************************************/
void bootloaderUARTreadData (void)
{

}

void debugUART2init (void)
{
	uint8_t receiveLenght = 0;
	while(1)
	{
		memset(blRxBuffer,0,200);
		/*Here we will read and decode the commands from the host*/
		/*First read only one byte from the host, which is the Lenght byte*/
		HAL_UART_Receive(C_UART, blRxBuffer, SINGLE_BYTE, HAL_MAX_DELAY);
		receiveLenght = blRxBuffer[0];
		/*Next we will use the receiveLenght from the first variable to receive the rest of the bytes*/
		HAL_UART_Receive(C_UART, &blRxBuffer[1], receiveLenght, HAL_MAX_DELAY);

		/*use tre switch statement to decode the command*/
		switch(blRxBuffer[1])
		{
			case BL_GET_VER:
				bootloaderHandleGET_VER_cmd(blRxBuffer);
				break;
			case BL_GET_HELP:
				bootloaderHandleGET_HELP_cmd(blRxBuffer);
				break;
			case BL_GET_CID:
				bootloaderHandleGET_CID_cmd(blRxBuffer);
				break;
			case BL_GET_RDP_STATUS:
				bootloaderHandleGET_RDP_cmd(blRxBuffer);
				break;
			case BL_GO_TO_ADDR:
				bootloaderHandleGO_TO_ADDR_cmd(blRxBuffer);
				break;
			case BL_FLASH_ERASE:
				bootloaderHandleFLASH_ERASE_cmd(blRxBuffer);
				break;
			case BL_MEM_WRITE:
				bootloaderHandleMEM_WRITE_cmd(blRxBuffer);
				break;
			case BL_EN_R_W_PROTECT:
				bootloaderHandleENDIS_RW_PROTECT_cmd(blRxBuffer);
				break;
			case BL_MEM_READ:
				bootloaderHandleMEM_READ_cmd(blRxBuffer);
				break;
			case BL_READ_SECTOR_STATUS:
				bootloaderHandleSECTOR_STATUS_cmd(blRxBuffer);
				break;
			case BL_OTP_READ:
				bootloaderHandleOTP_READ_cmd(blRxBuffer);
				break;
			case BL_DIS_R_W_PROTECT:
				bootloaderHandleDIS_RW_PROTECT_cmd(blRxBuffer);
				break;
			default:
				printMsg("BL_DEBUG_MSG:Invalid command doe received from host! \r\n");
				break;
		}
	}
}

/***********************************************************
 * Brief: Function Handles BL_GET_VER command
 * Arguments: 8 bit  pointer
 * Returns: None
 **********************************************************/
void bootloaderHandleGET_VER_cmd 			(uint8_t *cmd)
{
	uint8_t bootLoaderVersion = 0;
	printMsg("BL_DEBUG_MSG:Bootloader handle GET_VER cmd \r\n");

	//total lenght of the command packet
	uint32_t cmdPacketLength = cmd[0] + 1;											//first byte of this command is the length to follow = 5
																					//5+1 = 6 = total bytes in the buffer

	//extract Host CRC																//the last 4 bytes of the cmd buffer are the user crc
																					//we do not need to calculate CRC ofr these bytes,
																					//we just compare them to our calculated CRC
																					//-4 is for the 4 bytesof CRC = 2 bytes to be calculated
	uint32_t hostCRC = *((uint32_t * ) (cmd + cmdPacketLength -4));					//here we move the starting address to the 2nd (position 3) of the buffer
																					//typecast the parentisized as 32 bit pointer, then derefference to get data
	//bootLoaderVerifyCRC must return a zero value for the CRC to be correct
	//uint8_t bootLoaderVerifyCRC(uint8_t *pData, uint32_t length, uint32_t crcHost)
	if(!bootLoaderVerifyCRC(&cmd[0], (cmdPacketLength -4), hostCRC))				//length = 2
	{
		//successful checksum
		printMsg("BL_DEBUG_MSG:checksum correct! \r\n");
		//send Ack
		bootLoaderSendAck(cmd[0], SINGLE_BYTE);
		//Get bootloader version
		bootLoaderVersion = getBootLoaderVersion();
		//Print debug text
		printMsg("BL_DEBUG_MSG:Bootloader Version: %d\t%#x \r\n", bootLoaderVersion, bootLoaderVersion);
		//write version to host
		bootLoaderUART_writeData(&bootLoaderVersion,1);
	}
	else
	{
		//unsuccessful checksum
		printMsg("BL_DEBUG_MSG:checksum failure! \r\n");
		//Send Nack
		bootloaderSendNack();
	}
}

/***********************************************************
 * Brief: Function Handles BL_GET_HELP command
 * Arguments: 8 bit  pointer
 * Returns: None
 **********************************************************/
void bootloaderHandleGET_HELP_cmd 			(uint8_t *cmd)
{

}

/***********************************************************
 * Brief: Function Handles BL_GET_CID command
 * Arguments: 8 bit  pointer
 * Returns: None
 **********************************************************/
void bootloaderHandleGET_CID_cmd 			(uint8_t *cmd)
{

}

/***********************************************************
 * Brief: Function Handles BL_GET_RDP command
 * Arguments: 8 bit  pointer
 * Returns: None
 **********************************************************/
void bootloaderHandleGET_RDP_cmd 			(uint8_t *cmd)
{

}

/***********************************************************
 * Brief: Function Handles BL_GO_TO_ADDR command
 * Arguments: 8 bit  pointer
 * Returns: None
 **********************************************************/
void bootloaderHandleGO_TO_ADDR_cmd 		(uint8_t *cmd)
{

}

/***********************************************************
 * Brief: Function Handles BL_FLASH_ERASE command
 * Arguments: 8 bit  pointer
 * Returns: None
 **********************************************************/
void bootloaderHandleFLASH_ERASE_cmd 		(uint8_t *cmd)
{

}

/***********************************************************
 * Brief: Function Handles BL_MEM_WRITE command
 * Arguments: 8 bit  pointer
 * Returns: None
 **********************************************************/
void bootloaderHandleMEM_WRITE_cmd 			(uint8_t *cmd)
{

}

/***********************************************************
 * Brief: Function Handles BL_EN_R_W_PROTECT command
 * Arguments: 8 bit  pointer
 * Returns: None
 **********************************************************/
void bootloaderHandleENDIS_RW_PROTECT_cmd 	(uint8_t *cmd)
{

}

/***********************************************************
 * Brief: Function Handles SECTOR_STATUS command
 * Arguments: 8 bit  pointer
 * Returns: None
 **********************************************************/
void bootloaderHandleSECTOR_STATUS_cmd 		(uint8_t *cmd)
{

}

/***********************************************************
 * Brief: Function Handles BL_OTP_READ command
 * Arguments: 8 bit  pointer
 * Returns: None
 **********************************************************/
void bootloaderHandleOTP_READ_cmd 			(uint8_t *cmd)
{

}

/***********************************************************
 * Brief: Function Handles BL_DIS_RW_PROTECT command
 * Arguments: 8 bit  pointer
 * Returns: None
 **********************************************************/
void bootloaderHandleDIS_RW_PROTECT_cmd 	(uint8_t *cmd)
{

}

/***********************************************************
 * Brief: Function Handles Sending Ack to host
 * Arguments: 8 bit command integer, 8 bit follow length
 * Returns: None
 **********************************************************/
void bootLoaderSendAck (uint8_t cmd, uint8_t followLength)
{
	//2 bytes sent: 1. Ack 2. Lenght value
	uint8_t ackBuff[2] = {0};
	ackBuff[0] = BL_ACK;
	ackBuff[0] = followLength;
	HAL_UART_Transmit(C_UART, ackBuff, 2, HAL_MAX_DELAY);
}

/***********************************************************
 * Brief: Function Handles Sending Nack to host
 * Arguments: None
 * Returns: None
 **********************************************************/
void bootloaderSendNack(void)
{
	uint8_t nack = BL_NACK;
	HAL_UART_Transmit(C_UART, nack, 2, HAL_MAX_DELAY);
}

/***********************************************************
 * Brief: Function Handles CRC calculation and verification
 * Arguments:	pointer to 8 bit data buffer,
 * 				32 bit unsigned integer for length
 * Returns: 8 bit unsigned integer
 **********************************************************/
uint8_t bootLoaderVerifyCRC(uint8_t *pData, uint32_t length, uint32_t crcHost)
{
	uint32_t userCRC = 0xff;
	for(uint32_t i = 0; length; i++)
	{
		uint32_t iData = pData[i];
		userCRC = HAL_CRC_Accumulate(&hcrc, &iData, 1);
	}

	if(userCRC == crcHost)
		return VERIFY_CRC_SUCCESS;
	return VERIFY_CRC_FAIL;
}

/***********************************************************
 * Brief: Function Handles Bootloader version fetch
 * Arguments: None
 * Returns: 8 bit unsigned integer for BL version
 **********************************************************/
uint8_t getBootLoaderVersion (void)
{
	return(uint8_t) BL_VERSION;
}

/***********************************************************
 * Brief: Function Handles Reply back to the host
 * Arguments: None
 * Returns: 8 bit unsigned integer for BL version
 **********************************************************/
void bootLoaderUART_writeData (uint8_t *pBuffer, uint32_t length)
{
	HAL_UART_Transmit(C_UART, pBuffer, length, HAL_MAX_DELAY);
}
