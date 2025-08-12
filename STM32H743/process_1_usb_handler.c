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
 * process_1_usb_handler.c
 *
 *  Created on: Feb 1, 2025
 *      Author: fil
 */

#include "main.h"
#include "A_os_includes.h"
#include "membrane_global_includes.h"

extern	uint32_t	usb_handle;

uint8_t pack_USB_packet(uint8_t *rx_buf,uint8_t len)
{
uint32_t	i;

	for(i=0;i<len;i++)
	{
		i &= (USB_BUF_LEN-1);
		if (( MembraneUSB.usb_flags & USB_FLAGS_HEADEROK) == 0 )
		{
			if ( rx_buf[i] == '<')
			{
				MembraneUSB.usb_rx_buf[0] = rx_buf[i];
				MembraneUSB.usb_rx_buf_index = 1;
				MembraneUSB.usb_flags |= USB_FLAGS_HEADEROK;
			}
		}
		else
		{
			MembraneUSB.usb_rx_buf[MembraneUSB.usb_rx_buf_index ] = rx_buf[i];
			if ( MembraneUSB.usb_rx_buf[MembraneUSB.usb_rx_buf_index] == '>')
			{
				MembraneUSB.usb_flags |= USB_FLAGS_PKTCOMPLETE;
				MembraneUSB.usb_flags &= ~USB_FLAGS_HEADEROK;
				MembraneUSB.usb_rx_buf_len = MembraneUSB.usb_rx_buf_index;
				MembraneUSB.usb_rx_buf_index = 0;
				return	MembraneUSB.usb_rx_buf_len;
			}
			MembraneUSB.usb_rx_buf_index++;
		}
	}
	return 0;
}

uint8_t decode_USB_packet(uint8_t* Buf)
{
uint16_t	pnum;
char p0;
int	p1,p2,p3;

	pnum = sscanf((char * )Buf,"<%c%d%d%d", &p0, &p1,&p2,&p3);
	switch(pnum)
	{
	case	4:
		MembraneUSB.command_from_usb = p0;
		MembraneUSB.parameter1_from_usb = p1;
		MembraneUSB.parameter2_from_usb = p2;
		MembraneUSB.parameter3_from_usb = p3;
		break;
	case	3:
		MembraneUSB.command_from_usb = p0;
		MembraneUSB.parameter1_from_usb = p1;
		MembraneUSB.parameter2_from_usb = p2;
		break;
	case	2:
		MembraneUSB.command_from_usb = p0;
		MembraneUSB.parameter1_from_usb = p1;
		break;
	case	1:
		MembraneUSB.command_from_usb = p0;
		break;
	default:
		break;
	}
	return pnum;
}

extern	uint8_t	app_name[16];
extern	uint8_t	app_version[16];
extern	uint8_t	a_version[32];

uint8_t parse_USB_packet(uint8_t* Buf)
{
int		pnum,p1;
uint8_t ret_val = 1;
uint8_t i,j;
	MembraneUSB.usb_tx_buf_len = 0;
	switch(MembraneUSB.command_from_usb)
	{
	/* Data & info commands to sensors */
	case SENSORS_POWER_ON :
		ret_val = power_on_serials();
		break;
	case SENSORS_POWER_OFF :
		ret_val = power_off_serials();
		break;
	case SENSORS_SCAN_COMMAND:
		MembraneSystem.sensors_status |= SENSORS_RUN_STATE;
		MembraneSystem.sensors_max_sensor = MembraneUSB.parameter1_from_usb;
		if (( MembraneSystem.sensors_max_sensor == 0 ) || ( MembraneSystem.sensors_max_sensor > MAX_SENSORS ))
			MembraneSystem.sensors_max_sensor = 8;
		sprintf((char *)MembraneUSB.usb_tx_buf,"SCAN");
		MembraneUSB.usb_tx_buf_len = strlen((char *)MembraneUSB.usb_tx_buf);
		ret_val = 0;
		break;
	case SENSORS_STOP_COMMAND:
		MembraneSystem.sensors_status &= ~SENSORS_RUN_STATE;
		sprintf((char *)MembraneUSB.usb_tx_buf,"HALT");
		MembraneUSB.usb_tx_buf_len = strlen((char *)MembraneUSB.usb_tx_buf);
		ret_val = 0;
		break;
	case SENSORS_GETACQ_COMMAND:
		ret_val = send_get_acq_reply();
		break;
	case SENSORS_GETSENSVERINFO:
		MembraneSystem.line_selector = MembraneUSB.parameter1_from_usb;
		MembraneSystem.sensor_selector = MembraneUSB.parameter2_from_usb;
		get_sensors_info();
		break;
	case CONCENTRATOR_GO_XMO:
		pnum = sscanf((char * )Buf,"<h %d %s %s", &p1, MembraneFlashHeader.xmodem_file_name, MembraneFlashHeader.xmodem_md5);
		if ( pnum == 3 )
		{
			for(i=0,j=0;i<31;i+=2,j++)
				MembraneFlashHeader.xmodem_bin_md5[j]= A_hex_to_byte(MembraneFlashHeader.xmodem_md5[i],MembraneFlashHeader.xmodem_md5[i+1]);
			MembraneFlashHeader.xmodem_file_len = p1;
			memcpy(MembraneXMODEM.xmodem_header_area,(uint8_t *)&MembraneFlashHeader,sizeof(MembraneFlashHeader_TypeDef));
			MembraneUSB.usb_status |= USB_XMO_PHASE;
		}
		break;
	/* Flash commands to sensors */
	case	SENSORS_FLASH	: /* F <l> <s>: Send flash command to sensors */
		if (( MembraneSystem.sensors_status & (SENSORS_POWERED | SENSORS_RUN_STATE) ) == SENSORS_POWERED ) // check if powered and stopped
		{
			sprintf((char *)MembraneUSB.usb_tx_buf,"Sending download command to line %d sensor %d",(int )MembraneSystem.update_line , (int )MembraneSystem.update_sensor);
			MembraneUSB.usb_tx_buf_len = strlen((char *)MembraneUSB.usb_tx_buf);
			MembraneSystem.update_retry = 0;
			MembraneSystem.update_line = MembraneUSB.parameter1_from_usb;
			MembraneSystem.update_sensor = MembraneUSB.parameter2_from_usb;
			MembraneSystem.line_selector = MembraneUSB.parameter1_from_usb;
			MembraneSystem.sensor_selector = MembraneUSB.parameter2_from_usb;
			send_update_command_to_sensors();
			ret_val = 0;
		}
		break;
	case	SENSORS_FLASH_WRITE	: /* W  <l> <s>: Send Write_data command */
		MembraneSystem.update_line = MembraneUSB.parameter1_from_usb;
		MembraneSystem.update_sensor = MembraneUSB.parameter2_from_usb;
		sprintf((char *)MembraneUSB.usb_tx_buf,"Sending write command to line %d sensor %d",(int )MembraneSystem.update_line , (int )MembraneSystem.update_sensor);
		MembraneUSB.usb_tx_buf_len = strlen((char *)MembraneUSB.usb_tx_buf);
		send_write_command_to_sensors();
		ret_val = 0;
		break;
	/* Data & info commands to concentrator */
	case CONCENTRATOR_VERSION:
		sprintf((char *)MembraneUSB.usb_tx_buf,"%s %s %s",app_name , app_version, a_version);
		MembraneUSB.usb_tx_buf_len = strlen((char *)MembraneUSB.usb_tx_buf);
		ret_val = 0;
		break;
	}
	if ( ret_val == 0 )
	{
		if ( MembraneUSB.usb_tx_buf_len )
			usb_send(usb_handle,MembraneUSB.usb_tx_buf,MembraneUSB.usb_tx_buf_len);
	}
	return 0;
}

void send_sensor_version(uint8_t address,uint8_t type , char *packet)
{
	bzero(MembraneUSB.usb_tx_buf,USB_BUF_LEN);
	sprintf((char *)MembraneUSB.usb_tx_buf,"Sensor %d type %d %s",address,type,packet);
	MembraneUSB.usb_tx_buf_len = strlen((char *)MembraneUSB.usb_tx_buf);
	usb_send(usb_handle,MembraneUSB.usb_tx_buf,MembraneUSB.usb_tx_buf_len-1);
}

void send_sensor_update_progress(uint8_t address, uint32_t pkt_num)
{
	bzero(MembraneUSB.usb_tx_buf,USB_BUF_LEN);
	sprintf((char *)MembraneUSB.usb_tx_buf,"%d",(int )pkt_num);
	MembraneUSB.usb_tx_buf_len = strlen((char *)MembraneUSB.usb_tx_buf);
	usb_send(usb_handle,MembraneUSB.usb_tx_buf,MembraneUSB.usb_tx_buf_len);
}

