/* 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Project : A_os
*/
/*
 * membrane_global_includes.h
 *
 *  Created on: Feb 1, 2025
 *      Author: fil
 */

#ifndef MEMBRANE_GLOBAL_INCLUDES_H_
#define MEMBRANE_GLOBAL_INCLUDES_H_

#include "process_1_comm.h"
#include "process_1_usb_handler.h"
#include "process_1_sensors_handler.h"

#include "process_2_flasher.h"

#define		MAX_SENSORS			16
#define		MAX_LINES			4
#define		MAX_BOARDS			4
#define		USB_BUF_LEN			128
#define 	SINGLE_PKT_SIZE		8
#define		SENSORS_NUM			MAX_SENSORS
#define		SENSORS_LINE		MAX_LINES
#define		SENSORS_LINE0_INDEX	0
#define		SENSORS_LINE1_INDEX	1
#define		SENSORS_LINE2_INDEX	2
#define		SENSORS_LINE3_INDEX	3
#define		SENSORS_COMM_TIMEOUT		10

#define		SENSOR_WATER			1
#define		SENSOR_TEMPERATURE		2
#define		SENSOR_NOTPRESENT		0

#define		SENSORS_RX_LEN				(USB_BUF_LEN-4)

#define		PRC1_COMM_PROCESS_ID		1
#define		PRC2_FLASHER_PROCESS_ID		2
#define		PRC1_COMM_MAILBOX_ID		0
#define		PRC1_COMM_MAILBOX_LEN		128
#define		PRC2_FLASHER_MAILBOX_ID		0
#define		PRC2_FLASHER_MAILBOX_LEN	128

#define		SENSORS_UPDATE_MAXRETRY	5
#define		SENSORS_UPDATE_CMD		SENSORS_REPLY_CMD
#define		SENSORS_UPDATE_ADDRESS	SENSORS_REPLY_ADDRESS

#define		SENSORS_UPDATE_LEN		268	// 0<DAC[256 bytes data][4 bytes CRC32]D>0 , crc is on bytes data only
#define		SENSORS_UPDATE_PAYLOAD	256
#define		SENSORS_UPDATE_PKTNUM	240
#define		SENSORS_UPDATE_FULLSIZE	(256*240)

#define		SENSORS_DUMMY_INIT		0
#define		SENSORS_INITIATOR		1
#define		SENSORS_REPLY_CMD		2
#define		SENSORS_REPLY_ADDRESS	3
#define		SENSORS_UPDATE_PKTCNT	4
#define		SENSORS_UPDATE_DATA		5
#define		SENSORS_CRC_HH			(SENSORS_UPDATE_LEN-7)
#define		SENSORS_CRC_HL			(SENSORS_UPDATE_LEN-6)
#define		SENSORS_CRC_LH			(SENSORS_UPDATE_LEN-5)
#define		SENSORS_CRC_LL			(SENSORS_UPDATE_LEN-4)
#define		SENSORS_CLOSING_FLAG	(SENSORS_UPDATE_LEN-3)
#define		SENSORS_TERMINATOR		(SENSORS_UPDATE_LEN-2)
#define		SENSORS_DUMMY_END		(SENSORS_UPDATE_LEN-1)

#define		SENSORS_CRC_HH_SHIFT	24
#define		SENSORS_CRC_HL_SHIFT	16
#define		SENSORS_CRC_LH_SHIFT	8
#define		SENSORS_CRC_LL_SHIFT	0

#define		LINE_BROADCAST			0xff
#define		SENSORS_BROADCAST		0xff
#define		SENSORS_RESERVED		0x00

#define		SENSORS_INITIATOR_CHAR	'<'
#define		SENSORS_TERMINATOR_CHAR	'>'

typedef struct
{
	uint8_t 		sensors_status;
	uint8_t 		sensors_max_sensor;
	uint8_t 		line_selector;
	uint8_t 		sensor_selector;
	uint8_t 		sensor_scan_time;
	uint8_t 		sensor_scan_counter;
	uint8_t 		sensor_rxbuf[SENSORS_LINE][SENSORS_RX_LEN];
	uint8_t 		sensor_rxstate;
	uint8_t 		sensor_rxchar[SENSORS_LINE];
	uint32_t 		sensor_rxindex;
	uint32_t 		sensor_rx_count;

	uint8_t			sensor_reply[SENSORS_LINE][SENSORS_NUM][SENSORS_RX_LEN];
	uint8_t			sensor_scratchbuf[SENSORS_UPDATE_LEN];
	uint8_t			sensor_map[SENSORS_LINE][SENSORS_NUM];
	uint8_t			sensor_map_line_index;
	uint8_t			sensor_map_sensor_index;
	uint8_t 		sensor_discovery_line_selector;
	uint8_t 		sensor_discovery_sensor_selector;
	uint8_t 		sensor_scan_selector;
	uint8_t 		external_flash_status;
	/* sensors update section */
	uint32_t 		update_datalen;
	uint8_t			*update_data_src_address;
	uint32_t		update_data_count;
	uint8_t 		update_pkt_counter;
	uint8_t 		update_download_errors;
	uint8_t 		update_line;
	uint8_t 		update_sensor;
	uint8_t 		update_retry;
	uint8_t 		update_result;
	uint8_t 		update_numeration;
	uint16_t 		update_retries;
	uint16_t 		update_timeout;
	uint32_t		update_wait_sensor_setup;
	uint32_t		update_bar_val;
	uint32_t		update_iterations;
	uint32_t		flash_crc;
}MembraneSystem_TypeDef;

/* sensors_status */

#define	SENSORS_PKTSENT			0x01
#define	SENSORS_ON_UPDATE		0x02
#define	SENSORS_RUN_STATE		0x40
#define	SENSORS_POWERED			0x80
/* sensor_rxstate */
#define	SENSORS_WAIT_INITIATOR_CHAR			0
#define	SENSORS_DATA_PHASE					1

/* external_flash_status */
#define	EXTFLASH_VALID			0x80

#define	SENSORS_DOWNLOAD_TIMEOUT	50
/* sensor_rxbuf */
/*
 * sensor packet
 * <D Address Type ScaleFactor Data Temperature>
 */
#define	STARTFLAG_POSITION	0
#define	CMD_POSITION		1
#define	ADDRESS_POSITION	2
#define	TYPE_POSITION		3
#define	DAC_POSITION		4
#define	DATA_POSITION		6
#define	ITERATIONS_POSITION	8
#define	TEMP_POSITION		10
#define	ENDFLAG_POSITION	12


typedef struct
{
	uint8_t 		type;
	uint8_t 		address;
	uint16_t 		dac_value;
	uint16_t 		conductivity_value;
	uint16_t 		iterations;
	uint16_t 		temperature;
}MembraneSensorsArray_TypeDef;

#define	XMODEM_HEADER_AREA		(uint8_t *)0x30000000
#define	XMODEM_HEADER_LEN		(256)
#define	XMODEM_DATA_AREA		(uint8_t *)0x30000100
#define	XMODEM_DATA_LEN			(256*1024)
#define	SENSOR_CODE_AREA		(uint8_t *)0x38000000
#define	SENSOR_CODE_LEN			(64*1024)

typedef struct
{
	uint8_t 		xmodem_md5_status;
	uint32_t 		xmodem_file_len;
	uint8_t 		xmodem_file_name[128];
	uint8_t 		xmodem_md5[32];
	uint8_t 		xmodem_bin_md5[16];
	uint8_t 		xmodem_file_md5[16];
}MembraneFlashHeader_TypeDef;

typedef struct
{
	uint8_t 		usb_status;
	uint8_t			command_from_usb;
	uint32_t		parameter1_from_usb;
	uint32_t		parameter2_from_usb;
	uint32_t		parameter3_from_usb;
	uint32_t		parameter4_from_usb;
	uint32_t		parameter5_from_usb;
	uint32_t		parameter6_from_usb;
	uint32_t		parameter7_from_usb;
	char			string1_from_usb[32];
	char			string2_from_usb[32];
	uint8_t			usb_rx_buf_rxed[USB_BUF_LEN];
	uint8_t			usb_rx_buf[USB_BUF_LEN];
	uint8_t			usb_rx_buf_len;
	uint8_t			usb_rx_buf_index;
	uint8_t			usb_tx_buf[USB_BUF_LEN];
	uint8_t			usb_tx_buf_len;
	uint8_t 		usb_flags;
	uint8_t 		*xmodem_area;
	uint32_t 		xmodem_len;
	uint8_t 		*xmodem_header_area;
	uint32_t 		xmodem_header_len;
	uint8_t 		*xmodem_data_area;
	uint32_t 		xmodem_data_len;
	uint8_t 		*sensors_area;
	uint8_t 		*sensors_header_area;
	uint32_t 		sensors_header_len;
	uint8_t 		*sensors_code_area;
	uint32_t 		sensors_code_len;
	uint8_t 		external_flash_status;
}MembraneUSB_TypeDef;

/* usb_status */
#define	USB_XMO_PHASE			0x01
#define	USB_XMO_INITIALIZED		0x02
#define	USB_XMO_POLL			0x04

/* usb_flags */
#define	USB_FLAGS_LOGOSENT		0x02
#define	USB_FLAGS_SENDINFO		0x04
#define	USB_FLAGS_SENDDATA		0x08
#define	USB_FLAGS_SENDREPLY		0x10
#define	USB_FLAGS_IHEXOK		0x20
#define	USB_FLAGS_HEADEROK		0x40
#define	USB_FLAGS_PKTCOMPLETE	0x80

typedef struct
{
	uint8_t 		*xmodem_area;
	uint32_t 		xmodem_len;
	uint8_t 		*xmodem_header_area;
	uint32_t 		xmodem_header_len;
	uint8_t 		*xmodem_data_area;
	uint32_t 		xmodem_data_len;
	uint8_t 		*sensors_area;
	uint8_t 		*sensors_header_area;
	uint32_t 		sensors_header_len;
	uint8_t 		*sensors_code_area;
	uint32_t 		sensors_code_len;
}MembraneXMODEM_TypeDef;


#define	USB_STD_PACKET			0x01
#define	USB_IHEX_PACKET			0x02
#define	USB_NO_PACKET			0x80

/* commands */
#define	SENSORS_POWER_ON			'P'
#define	SENSORS_POWER_OFF			'O'
#define	SENSORS_SCAN_COMMAND		'S'
#define	SENSORS_STOP_COMMAND		'H'
#define	SENSORS_GETACQ_COMMAND		'A'
#define	SENSORS_GETSENSVERINFO		'J'
#define	SENSORS_GO_UPDATMODE		'U'
#define	SENSORS_FLASH				'F'
#define	SENSORS_FLASH_WRITE			'W'

#define	CONCENTRATOR_GO_XMO			'h'
#define	CONCENTRATOR_WRITE_FLASH	'f'
#define	CONCENTRATOR_READ_FLASH		'r'
#define	CONCENTRATOR_VERSION		'v'
#define	CONCENTRATOR_FLASH_GETINFO	'g'



extern	MembraneSystem_TypeDef			MembraneSystem;
extern	MembraneUSB_TypeDef				MembraneUSB;
extern	MembraneFlashHeader_TypeDef		MembraneFlashHeader;
extern	MembraneXMODEM_TypeDef			MembraneXMODEM;
extern	MembraneSensorsArray_TypeDef	MembraneSensorsArray[MAX_LINES][MAX_SENSORS];

extern	UART_HandleTypeDef 			huart4;
extern	UART_HandleTypeDef 			huart5;
extern	UART_HandleTypeDef 			huart7;
extern	UART_HandleTypeDef 			huart8;
extern	CRC_HandleTypeDef 			hcrc;



#endif /* MEMBRANE_GLOBAL_INCLUDES_H_ */
