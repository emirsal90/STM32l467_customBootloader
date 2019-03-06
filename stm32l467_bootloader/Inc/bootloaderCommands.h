/*
 * bootloaderCommands.h
 *
 *  Created on: 5 Mar 2019
 *      Author: emirs
 */

#ifndef BOOTLOADERCOMMANDS_H_
#define BOOTLOADERCOMMANDS_H_

#include "stm32l4xx_hal.h"
#include "stdbool.h"
#include "main.h"
#include "comPort.h"

extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;
extern CRC_HandleTypeDef hcrc;

#define SINGLE_BYTE				0x01u
#define ZERO					0
#define BYTE_LENGHT				0xFFu
#define BL_RX_LEN				0xC8u

/*Bootloader commands*/
#define BL_GET_VER 				0x51		/*Bootloader version number (1byte) This command is used to read the bootloader version from the MCU*/
#define BL_GET_HELP 			0x52		/*All supported Command codes (10 bytes) This command is used to know what are the commands supported by the bootloader*/
#define BL_GET_CID				0x53		/*Chip identification number (2 bytes) This command is used to read the MCU chip identification number*/
#define BL_GET_RDP_STATUS 		0x54		/*Returns the FLASH Read Protection level. (1byte) This command is used to read the FLASH Read Protection level.*/
#define BL_GO_TO_ADDR			0x55		/*Success or Error Code (1byte) This command is used to jump bootloader to specified address.*/
#define BL_FLASH_ERASE  		0x56		/*Success or Error Code (1byte) This command is used to mass erase or sector erase of the user flash*/
#define BL_MEM_WRITE 			0x57		/*Success or Error Code (1byte) This command is used to write data in to different memories of the MCU*/
#define BL_EN_R_W_PROTECT 		0x58		/*Success or Error Code (1byte) This command is used to enable read/write protect on different sectors of the user flash*/
#define BL_MEM_READ				0x59		/*Memory contents of length asked by the host. This command is used to read data from different memories of the microcontroller.
											TODO: This is left as exercise for the student*/
#define BL_READ_SECTOR_STATUS 	0x5A		/*All sector status (2bytes) This command is used to read all the sector protection status.*/
#define BL_OTP_READ				0x5B		/*OTP contents This command is used to read the OTP contents. TODO: This is left as exercise for the student*/
#define BL_DIS_R_W_PROTECT		0x5C		/*Success or Error Code (1byte)This command is used to disable read/write protection on different sectors of the user flash .
											This command takes the protection status to default state.*/

/*Flash Addresses*/
#define FLASH_SECTOR1_BASE_ADDRESS 		0x08000000u
#define FLASH_SECTOR2_BASE_ADDRESS 		0x08008000u

/*STM Datasheet defines*/
#define BL_ACK					0xA5u
#define BL_NACK					0X7Fu

#define VERIFY_CRC_SUCCESS 		0
#define VERIFY_CRC_FAIL			1u

//version = 1.0
#define BL_VERSION				0x10


/*BootLoader Function Prototypes*/
bool isUserButtonPushed			(void);
void bootloaderUARTreadData 	(void);
void bootLoaderJumpToUserApp	(void);
void debugUART2init 			(void);




#endif /* BOOTLOADERCOMMANDS_H_ */
