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
 * process_1_sensors_handler.c
 *
 *  Created on: Feb 1, 2025
 *      Author: fil
 */

#include "main.h"
#include "A_os_includes.h"
#include "membrane_global_includes.h"

MembraneSensorsArray_TypeDef	MembraneSensorsArray[MAX_LINES][MAX_SENSORS];

extern	uint32_t	Line0_Uart_driver_handle;
extern	uint32_t	Line1_Uart_driver_handle;
extern	uint32_t	Line2_Uart_driver_handle;
extern	uint32_t	Line3_Uart_driver_handle;


uint8_t power_on_serials(void)
{
	HAL_GPIO_WritePin(PW_N_GPIO_Port, PW_N_Pin,GPIO_PIN_RESET);
	MembraneSystem.sensors_status |= SENSORS_POWERED;
	sprintf((char *)MembraneUSB.usb_tx_buf,"Power ON");
	MembraneUSB.usb_tx_buf_len = strlen((char *)MembraneUSB.usb_tx_buf);
	return 0;
}

uint8_t power_off_serials(void)
{
	HAL_GPIO_WritePin(PW_N_GPIO_Port, PW_N_Pin,GPIO_PIN_SET);
	MembraneSystem.sensors_status &= ~SENSORS_POWERED;
	sprintf((char *)MembraneUSB.usb_tx_buf,"Power OFF");
	MembraneUSB.usb_tx_buf_len = strlen((char *)MembraneUSB.usb_tx_buf);
	return 0;
}

void send_command_to_all_lines(uint8_t *packet,uint16_t len)
{
	uart_send(Line0_Uart_driver_handle,packet,len);
	uart_send(Line1_Uart_driver_handle,packet,len);
	uart_send(Line2_Uart_driver_handle,packet,len);
	uart_send(Line3_Uart_driver_handle,packet,len);
}

void send_command_to_single_line(uint8_t *packet,uint16_t len)
{
	switch ( MembraneUSB.parameter1_from_usb )
	{
	case	1	:	uart_send(Line0_Uart_driver_handle,packet,len); break;
	case	2	:	uart_send(Line1_Uart_driver_handle,packet,len); break;
	case	3	:	uart_send(Line2_Uart_driver_handle,packet,len); break;
	case	4	:	uart_send(Line3_Uart_driver_handle,packet,len); break;
	}
}

uint32_t	crc_packet(uint32_t 	*flash_data_ptr,uint32_t flash_data_len)
{
	hcrc.Instance->INIT = 0xffffffff;
	return HAL_CRC_Calculate(&hcrc, flash_data_ptr, flash_data_len);
}



uint8_t send_get_acq_reply(void)
{
uint8_t	line,sensor;
	if (MembraneUSB.parameter1_from_usb <= MAX_LINES)
	{
		if (MembraneUSB.parameter2_from_usb <= MAX_SENSORS)
		{
			line = MembraneUSB.parameter1_from_usb - 1;
			sensor = MembraneUSB.parameter2_from_usb - 1;
			sprintf((char *)MembraneUSB.usb_tx_buf,"%02x %02x %04x %04x %04x %04x",
					MembraneSensorsArray[line][sensor].type,
					MembraneSensorsArray[line][sensor].address,
					MembraneSensorsArray[line][sensor].dac_value,
					MembraneSensorsArray[line][sensor].conductivity_value,
					MembraneSensorsArray[line][sensor].iterations,
					MembraneSensorsArray[line][sensor].temperature
					);
			MembraneUSB.usb_tx_buf_len = strlen((char *)MembraneUSB.usb_tx_buf);
			return 0;
		}
	}
	MembraneUSB.usb_tx_buf_len = 0;
	return 0;
}

void send_datarequest_to_sensors(uint8_t sensor_selector)
{
	MembraneSystem.sensor_scratchbuf[0] = 0;
	MembraneSystem.sensor_scratchbuf[1] = '<';
	MembraneSystem.sensor_scratchbuf[2] = SENSORS_GETACQ_COMMAND;
	MembraneSystem.sensor_scratchbuf[3] = sensor_selector;
	MembraneSystem.sensor_scratchbuf[4] = '>';
	MembraneSystem.sensor_scratchbuf[5] = 0;
	send_command_to_all_lines(MembraneSystem.sensor_scratchbuf,6);
}

void get_sensors_info(void)
{
	MembraneSystem.sensor_scratchbuf[0] = 0;
	MembraneSystem.sensor_scratchbuf[1] = '<';
	MembraneSystem.sensor_scratchbuf[2] = SENSORS_GETSENSVERINFO;
	MembraneSystem.sensor_scratchbuf[3] = MembraneUSB.parameter2_from_usb;
	MembraneSystem.sensor_scratchbuf[4] = '>';
	MembraneSystem.sensor_scratchbuf[5] = 0;
	send_command_to_single_line(MembraneSystem.sensor_scratchbuf,6);
}

void updater_create_packet(uint8_t address)
{
uint32_t	pktcrc,i;

	MembraneSystem.sensor_scratchbuf[0] = 0;
	MembraneSystem.sensor_scratchbuf[SENSORS_INITIATOR] = '<';
	MembraneSystem.sensor_scratchbuf[SENSORS_UPDATE_CMD] = SENSORS_FLASH;
	MembraneSystem.sensor_scratchbuf[SENSORS_UPDATE_ADDRESS] = address;
	MembraneSystem.sensor_scratchbuf[SENSORS_UPDATE_PKTCNT] = MembraneSystem.update_pkt_counter;
	for(i=0;i<SENSORS_UPDATE_PAYLOAD;i++)
		MembraneSystem.sensor_scratchbuf[i+SENSORS_UPDATE_DATA] = MembraneSystem.update_data_src_address[i + (MembraneSystem.update_pkt_counter*SENSORS_UPDATE_PAYLOAD)];
	MembraneSystem.update_data_count+=SENSORS_UPDATE_PAYLOAD;
	pktcrc = crc_packet((uint32_t *)&MembraneSystem.sensor_scratchbuf[SENSORS_UPDATE_DATA],SENSORS_UPDATE_PAYLOAD);
	MembraneSystem.sensor_scratchbuf[SENSORS_CRC_HH] = pktcrc >> SENSORS_CRC_HH_SHIFT;
	MembraneSystem.sensor_scratchbuf[SENSORS_CRC_HL] = pktcrc >> SENSORS_CRC_HL_SHIFT;
	MembraneSystem.sensor_scratchbuf[SENSORS_CRC_LH] = pktcrc >> SENSORS_CRC_LH_SHIFT;
	MembraneSystem.sensor_scratchbuf[SENSORS_CRC_LL] = pktcrc >> SENSORS_CRC_LL_SHIFT;
	MembraneSystem.sensor_scratchbuf[SENSORS_CLOSING_FLAG] = 'D';
	MembraneSystem.sensor_scratchbuf[SENSORS_TERMINATOR] = '>';
	MembraneSystem.sensor_scratchbuf[SENSORS_TERMINATOR+1] = 0;
}

uint32_t send_update(void)
{
	updater_create_packet(MembraneSystem.update_sensor);
	send_update_packet_to_sensors(MembraneSystem.sensor_scratchbuf,SENSORS_UPDATE_LEN);
	MembraneSystem.update_iterations++;
	if ( MembraneSystem.update_iterations == 12 )
	{
		MembraneSystem.update_iterations = 0;
		MembraneSystem.update_bar_val += 5;
		send_sensor_update_progress(MembraneSystem.update_sensor,MembraneSystem.update_bar_val);
	}
	if ( MembraneSystem.update_pkt_counter >= SENSORS_UPDATE_PKTNUM)
	{
		MembraneSystem.sensors_status &= ~(SENSORS_ON_UPDATE | SENSORS_PKTSENT);
		MembraneSystem.update_iterations = 0;
		MembraneSystem.update_bar_val = 0;
		return 0;
	}
	return 1;
}
void send_update_packet_to_sensors(uint8_t *packet,uint16_t len)
{
	if ((MembraneSystem.sensors_status & SENSORS_POWERED) == SENSORS_POWERED)
	{
		switch(MembraneSystem.update_line)
		{
		case	1	:	uart_send(Line0_Uart_driver_handle,packet,len); break;
		case	2	:	uart_send(Line1_Uart_driver_handle,packet,len); break;
		case	3	:	uart_send(Line2_Uart_driver_handle,packet,len); break;
		case	4	:	uart_send(Line3_Uart_driver_handle,packet,len); break;
		}
	}
}

void send_update_command_to_sensors(void)
{
	MembraneSystem.update_data_src_address = MembraneXMODEM.sensors_code_area;
	MembraneSystem.update_download_errors =	MembraneSystem.update_pkt_counter = 0;
	MembraneSystem.update_data_count = 0;
	MembraneSystem.update_timeout = SENSORS_DOWNLOAD_TIMEOUT;
	MembraneXMODEM.sensors_code_len = SENSOR_CODE_LEN;
	MembraneSystem.update_pkt_counter = 0;
	MembraneSystem.update_wait_sensor_setup = 0;

	MembraneSystem.sensors_status &= ~SENSORS_PKTSENT;
	MembraneSystem.sensors_status |= SENSORS_ON_UPDATE;
	MembraneSystem.update_iterations = 0;
	MembraneSystem.update_bar_val = 0;

	MembraneSystem.sensor_scratchbuf[0] = 0;
	MembraneSystem.sensor_scratchbuf[1] = '<';
	MembraneSystem.sensor_scratchbuf[2] = SENSORS_GO_UPDATMODE;
	MembraneSystem.sensor_scratchbuf[3] = MembraneSystem.update_sensor;
	MembraneSystem.sensor_scratchbuf[4] = '>';
	MembraneSystem.sensor_scratchbuf[5] = 0;
	send_command_to_single_line(MembraneSystem.sensor_scratchbuf,6);
}

void send_write_command_to_sensors(void)
{
	MembraneSystem.sensor_scratchbuf[0] = 0;
	MembraneSystem.sensor_scratchbuf[1] = '<';
	MembraneSystem.sensor_scratchbuf[2] = SENSORS_FLASH_WRITE;
	MembraneSystem.sensor_scratchbuf[3] = MembraneUSB.parameter2_from_usb;
	MembraneSystem.sensor_scratchbuf[4] = '>';
	MembraneSystem.sensor_scratchbuf[5] = 0;
	send_command_to_single_line(MembraneSystem.sensor_scratchbuf,6);
}

/*
 * sensor packet
 * <A Address Type ScaleFactor Data Temperature>
 */

uint16_t	conductivity_value,temperature,dac_value,iterations,message=0;

uint8_t packet_assemble(uint8_t line)
{
	if ( MembraneSystem.sensor_rxstate == SENSORS_WAIT_INITIATOR_CHAR)
	{
		if(MembraneSystem.sensor_rxchar[line] == SENSORS_INITIATOR_CHAR)
		{
			bzero(MembraneSystem.sensor_rxbuf[line],16);
			MembraneSystem.sensor_rxbuf[line][0] = SENSORS_INITIATOR_CHAR;
			MembraneSystem.sensor_rxstate = SENSORS_DATA_PHASE;
			MembraneSystem.sensor_rxindex = 1;
			return 1;
		}
	}
	else
	{
		MembraneSystem.sensor_rxbuf[line][MembraneSystem.sensor_rxindex] = MembraneSystem.sensor_rxchar[line];
		if ( MembraneSystem.sensor_rxindex == 1)
		{
			switch(MembraneSystem.sensor_rxbuf[line][1])
			{
			case SENSORS_GETACQ_COMMAND :
				MembraneSystem.sensor_rx_count = 11;
				break;
			case SENSORS_GO_UPDATMODE :
				MembraneSystem.sensor_rx_count = 7;
				break;
			case SENSORS_FLASH :
				MembraneSystem.sensor_rx_count = 7;
				break;
			case SENSORS_GETSENSVERINFO :
				MembraneSystem.sensor_rx_count = 64;
				break;
			default:
				MembraneSystem.sensor_rxstate = SENSORS_WAIT_INITIATOR_CHAR;
			}
			MembraneSystem.sensor_rxindex ++;
			return 1;
		}
		else if ( MembraneSystem.sensor_rxindex == 2)
		{
			if (MembraneSystem.sensor_rxbuf[line][ADDRESS_POSITION] != MembraneSystem.sensor_selector)
				MembraneSystem.sensor_rxstate = SENSORS_WAIT_INITIATOR_CHAR;
			MembraneSystem.sensor_rxindex ++;
			return 1;
		}
		else
		{
			if (( MembraneSystem.sensor_rxbuf[line][MembraneSystem.sensor_rxindex] == SENSORS_TERMINATOR_CHAR) && ( MembraneSystem.sensor_rxindex > 4 ))
			{
				/* running section */
				if (( MembraneSystem.sensors_status & SENSORS_RUN_STATE ) == SENSORS_RUN_STATE )
				{
					if ( MembraneSystem.sensor_rxbuf[line][CMD_POSITION] == SENSORS_GETACQ_COMMAND)
					{
						MembraneSensorsArray[line][MembraneSystem.sensor_selector-1].type = MembraneSystem.sensor_rxbuf[line][TYPE_POSITION];
						MembraneSensorsArray[line][MembraneSystem.sensor_selector-1].address = MembraneSystem.sensor_rxbuf[line][ADDRESS_POSITION];
						dac_value = (MembraneSystem.sensor_rxbuf[line][DAC_POSITION] << 8 ) | MembraneSystem.sensor_rxbuf[line][DAC_POSITION+1];
						conductivity_value = (MembraneSystem.sensor_rxbuf[line][DATA_POSITION] << 8 ) | MembraneSystem.sensor_rxbuf[line][DATA_POSITION+1];
						iterations = (MembraneSystem.sensor_rxbuf[line][ITERATIONS_POSITION] << 8 ) | MembraneSystem.sensor_rxbuf[line][ITERATIONS_POSITION+1];
						temperature = (MembraneSystem.sensor_rxbuf[line][TEMP_POSITION] << 8 ) | MembraneSystem.sensor_rxbuf[line][TEMP_POSITION+1];
						MembraneSensorsArray[line][MembraneSystem.sensor_selector-1].dac_value = dac_value;
						MembraneSensorsArray[line][MembraneSystem.sensor_selector-1].conductivity_value = conductivity_value;
						MembraneSensorsArray[line][MembraneSystem.sensor_selector-1].iterations = iterations;
						MembraneSensorsArray[line][MembraneSystem.sensor_selector-1].temperature = temperature;
						MembraneSystem.sensor_rxstate = SENSORS_WAIT_INITIATOR_CHAR;
						return 0;
					}
				}
				/* flash section */
				else if ( MembraneSystem.sensor_rxbuf[line][CMD_POSITION] == SENSORS_GO_UPDATMODE)
				{
					MembraneSystem.update_result = MembraneSystem.sensor_rxbuf[line][DAC_POSITION];
					MembraneSystem.update_numeration = MembraneSystem.sensor_rxbuf[line][DATA_POSITION];
					MembraneSystem.sensor_rxstate = SENSORS_WAIT_INITIATOR_CHAR;
					return 0;
				}
				else if ( MembraneSystem.sensor_rxbuf[line][CMD_POSITION] == SENSORS_FLASH)
				{
					if (( MembraneSystem.sensors_status & SENSORS_ON_UPDATE ) == SENSORS_ON_UPDATE )
					{
						MembraneSystem.update_result = MembraneSystem.sensor_rxbuf[line][DAC_POSITION];
						MembraneSystem.update_numeration = MembraneSystem.sensor_rxbuf[line][DATA_POSITION];
						MembraneSystem.sensor_rxstate = SENSORS_WAIT_INITIATOR_CHAR;
						return 0;
					}
				}
				else if ( MembraneSystem.sensor_rxbuf[line][CMD_POSITION] == SENSORS_GETSENSVERINFO)
				{
					MembraneSystem.sensor_rxstate = SENSORS_WAIT_INITIATOR_CHAR;
					send_sensor_version(MembraneSystem.sensor_rxbuf[line][ADDRESS_POSITION],MembraneSystem.sensor_rxbuf[line][TYPE_POSITION],(char *)&MembraneSystem.sensor_rxbuf[line][DATA_POSITION]);
					return 0;
				}
			}
			MembraneSystem.sensor_rxindex ++;
		}
	}
	return 1;
}

