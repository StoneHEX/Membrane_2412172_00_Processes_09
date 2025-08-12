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
 * process_1_comm.c
 *
 *  Created on: Feb 1, 2025
 *      Author: fil
 */


#include "main.h"
#include "A_os_includes.h"
#include "membrane_global_includes.h"

uint8_t		packet_assemble_status;
uint8_t 	led_cntr = 0;
uint8_t		to_flashprc_mbx[PRC2_FLASHER_MAILBOX_LEN];
uint8_t 	xmodem_cntr = 10;
uint8_t		xmodem_reply;
uint8_t		nak=X_NAK,ack=X_ACK;

#define	TICK_PRC1	100

USB_Drv_TypeDef	Usb_channel =
{
		.requested_len = USB_BUF_LEN,
		.data = MembraneUSB.usb_rx_buf_rxed,
		.timeout = 10,
		.wakeup_id = WAKEUP_FROM_USB_DEVICE_IRQ,
};
uint32_t	usb_handle;

UART_Drv_TypeDef Line0_Uart_Drv =
{
	.data = &MembraneSystem.sensor_rxchar[SENSORS_LINE0_INDEX],
	.rx_max_len = 1,
	.uart = &huart4,
	.wakeup_id = WAKEUP_FROM_UART4_IRQ,
	.timeout = SENSORS_COMM_TIMEOUT,
	.flags = UART_USES_DMA_TX | UART_USES_DMA_RX | UART_WAKEUP_ON_RXCHAR,
};
uint32_t	Line0_Uart_driver_handle;

UART_Drv_TypeDef Line1_Uart_Drv =
{
	.data = &MembraneSystem.sensor_rxchar[SENSORS_LINE1_INDEX],
	.rx_max_len = 1,
	.uart = &huart5,
	.wakeup_id = WAKEUP_FROM_UART5_IRQ,
	.timeout = SENSORS_COMM_TIMEOUT,
	.flags = UART_USES_DMA_TX | UART_USES_DMA_RX | UART_WAKEUP_ON_RXCHAR,
};
uint32_t	Line1_Uart_driver_handle;

UART_Drv_TypeDef Line2_Uart_Drv =
{
	.data = &MembraneSystem.sensor_rxchar[SENSORS_LINE2_INDEX],
	.rx_max_len = 1,
	.uart = &huart7,
	.wakeup_id = WAKEUP_FROM_UART7_IRQ,
	.timeout = SENSORS_COMM_TIMEOUT,
	.flags = UART_USES_DMA_TX | UART_USES_DMA_RX | UART_WAKEUP_ON_RXCHAR,
};
uint32_t	Line2_Uart_driver_handle;

UART_Drv_TypeDef Line3_Uart_Drv =
{
	.data = &MembraneSystem.sensor_rxchar[SENSORS_LINE3_INDEX],
	.rx_max_len = 1,
	.uart = &huart8,
	.wakeup_id = WAKEUP_FROM_UART8_IRQ,
	.timeout = SENSORS_COMM_TIMEOUT,
	.flags = UART_USES_DMA_TX | UART_USES_DMA_RX | UART_WAKEUP_ON_RXCHAR,
};
uint32_t	Line3_Uart_driver_handle;

void xmodem_check(void)
{
	if (( MembraneUSB.usb_status & USB_XMO_INITIALIZED ) == 0 )
	{
		xmodem_rx_init(MembraneXMODEM.xmodem_data_area,MembraneXMODEM.xmodem_data_len);
		Usb_channel.requested_len = XMODEM_LINE_LEN;
		MembraneUSB.usb_status |= (USB_XMO_INITIALIZED | USB_XMO_POLL);
	}
	else
	{
		if (( MembraneUSB.usb_status & (USB_XMO_INITIALIZED | USB_XMO_POLL) ) == (USB_XMO_INITIALIZED | USB_XMO_POLL))
		{
			xmodem_cntr++;
			if ( xmodem_cntr >= (10 * (100/TICK_PRC1)) )
			{
				usb_send(usb_handle,&nak,1);
				xmodem_cntr = 0;
			}
		}
	}
}

uint8_t xmodem_get_file(void)
{
	MembraneUSB.usb_status &= ~USB_XMO_POLL;
	xmodem_reply = xmodem_rx_line_parser(MembraneUSB.usb_rx_buf_rxed);
	switch(xmodem_reply)
	{
	case	X_NAK:
		usb_send(usb_handle,&nak,1);
		break;
	case	X_EOT:
		usb_send(usb_handle,&ack,1);
		Usb_channel.requested_len = USB_BUF_LEN;
		return 0;
		break;
	case	X_ACK:
		usb_send(usb_handle,&ack,1);
		break;
	default:
		usb_send(usb_handle,&nak,1);
		break;
	}
	return 1;
}

void process_led(void)
{
	switch(led_cntr)
	{
	case (7 * (100/TICK_PRC1)) :
	case (9 * (100/TICK_PRC1)) :
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin,GPIO_PIN_RESET);
		break;
	default :
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin,GPIO_PIN_SET);
		break;
	}
	led_cntr++;
	if ( led_cntr >= (10 * (100/TICK_PRC1)) )
		led_cntr = 0;
}

void process_1_comm(uint32_t process_id)
{
uint32_t	wakeup,flags;
uint8_t		usb_reply;
uint8_t		reply_time;

	MembraneXMODEM.xmodem_area = XMODEM_HEADER_AREA;
	MembraneXMODEM.xmodem_len = XMODEM_HEADER_LEN + XMODEM_DATA_LEN;
	MembraneXMODEM.xmodem_header_area = XMODEM_HEADER_AREA;
	MembraneXMODEM.xmodem_header_len = XMODEM_HEADER_LEN;
	MembraneXMODEM.xmodem_data_area = XMODEM_DATA_AREA;
	MembraneXMODEM.xmodem_data_len = XMODEM_DATA_LEN;
	MembraneXMODEM.sensors_code_area = SENSOR_CODE_AREA;
	MembraneXMODEM.sensors_code_len = SENSOR_CODE_LEN;

	bzero(MembraneXMODEM.xmodem_header_area,MembraneXMODEM.xmodem_header_len);

	usb_handle = usb_device_driver_register(&Usb_channel);
	Line0_Uart_driver_handle = uart_register(&Line0_Uart_Drv);
	Line1_Uart_driver_handle = uart_register(&Line1_Uart_Drv);
	Line2_Uart_driver_handle = uart_register(&Line2_Uart_Drv);
	Line3_Uart_driver_handle = uart_register(&Line3_Uart_Drv);
	uart_start_receive(Line0_Uart_driver_handle);
	uart_start_receive(Line1_Uart_driver_handle);
	uart_start_receive(Line2_Uart_driver_handle);
	uart_start_receive(Line3_Uart_driver_handle);

	create_timer(TIMER_ID_0,TICK_PRC1,TIMERFLAGS_FOREVER | TIMERFLAGS_ENABLED);

	MembraneSystem.sensor_selector = 0;

	while(1)
	{
		wait_event(EVENT_TIMER | EVENT_USB_DEVICE_IRQ | EVENT_UART4_IRQ | EVENT_UART5_IRQ | EVENT_UART7_IRQ | EVENT_UART8_IRQ |EVENT_MBX);
		get_wakeup_flags(&wakeup,&flags);

		if (( wakeup & WAKEUP_FROM_TIMER) == WAKEUP_FROM_TIMER)
		{
			process_led();
			if ((MembraneSystem.sensors_status & SENSORS_POWERED) == SENSORS_POWERED)
			{
				if ((MembraneSystem.sensors_status & SENSORS_RUN_STATE) == SENSORS_RUN_STATE)
				{
					if (MembraneSystem.sensor_selector > MembraneSystem.sensors_max_sensor)
						MembraneSystem.sensor_selector = 1;
					else
						MembraneSystem.sensor_selector ++;
					send_datarequest_to_sensors(MembraneSystem.sensor_selector);
				}
				else
				{
					if (( MembraneSystem.sensors_status & SENSORS_ON_UPDATE ) == SENSORS_ON_UPDATE )
					{
						reply_time = 0;
						switch(MembraneSystem.update_result)
						{
						case PKT_OK :
							MembraneSystem.update_retries = 0;
							break;
						case PKT_SEQ_ERROR :
							MembraneSystem.update_retries++;
							if ( MembraneSystem.update_retries > 5)
							{
								MembraneSystem.sensors_status &= ~(SENSORS_ON_UPDATE | SENSORS_PKTSENT);
								MembraneSystem.update_retries = 0;
								send_sensor_update_progress(MembraneSystem.update_sensor,1000);
							}
							else
								MembraneSystem.update_pkt_counter = MembraneSystem.update_numeration+1;
							reply_time = 5;
							break;
						case PKT_NOK_ERROR :
						case PKT_CRC_ERROR :
						default :
							MembraneSystem.update_retries++;
							if ( MembraneSystem.update_retries > 5)
							{
								MembraneSystem.sensors_status &= ~(SENSORS_ON_UPDATE | SENSORS_PKTSENT);
								MembraneSystem.update_retries = 0;
								send_sensor_update_progress(MembraneSystem.update_sensor,1000);
							}
							else
							{
								if ( MembraneSystem.update_pkt_counter )
									MembraneSystem.update_pkt_counter--;
							}
							reply_time = 5;
							break;
						}
						if ( reply_time )
							reply_time--;
						else
						{
							if ( MembraneSystem.update_result )
							{
								MembraneSystem.update_result = 0;
								send_update();
								MembraneSystem.update_pkt_counter++;
							}
						}
					}
				}
			}

			if (( MembraneUSB.usb_status & USB_XMO_PHASE ) == USB_XMO_PHASE )
			{
				xmodem_check();
			}
		}

		if (( wakeup & WAKEUP_FROM_USB_DEVICE_IRQ) == WAKEUP_FROM_USB_DEVICE_IRQ)
		{
			if (( MembraneUSB.usb_status & USB_XMO_PHASE ) == USB_XMO_PHASE )
			{
				if ( xmodem_get_file() == 0 )
				{
					MembraneUSB.usb_status &= ~(USB_XMO_PHASE | USB_XMO_INITIALIZED | USB_XMO_POLL);
					sprintf((char * )to_flashprc_mbx,"%c %d %d",CONCENTRATOR_WRITE_FLASH,(int )MembraneXMODEM.xmodem_data_area,(int )xmodem_rx_get_rxed_amount());
					mbx_send(PRC2_FLASHER_PROCESS_ID,PRC2_FLASHER_MAILBOX_ID,to_flashprc_mbx,strlen((char *)to_flashprc_mbx));
					MembraneUSB.usb_status &= ~USB_XMO_PHASE;
				}
			}
			else
			{
				usb_reply = pack_USB_packet(MembraneUSB.usb_rx_buf_rxed,usb_get_rx_len(usb_handle));
				if ( usb_reply )
				{
					if ( decode_USB_packet(MembraneUSB.usb_rx_buf) )
						parse_USB_packet(MembraneUSB.usb_rx_buf);
				}
			}
		}

		if (( wakeup & Line0_Uart_Drv.wakeup_id) == Line0_Uart_Drv.wakeup_id)
		{
			if (( flags & WAKEUP_FLAGS_UART_RX) == WAKEUP_FLAGS_UART_RX )
				packet_assemble_status = packet_assemble(0);
		}
		if (( wakeup & Line1_Uart_Drv.wakeup_id) == Line1_Uart_Drv.wakeup_id)
		{
			if (( flags & WAKEUP_FLAGS_UART_RX) == WAKEUP_FLAGS_UART_RX )
				packet_assemble_status = packet_assemble(1);
		}
		if (( wakeup & Line2_Uart_Drv.wakeup_id) == Line2_Uart_Drv.wakeup_id)
		{
			if (( flags & WAKEUP_FLAGS_UART_RX) == WAKEUP_FLAGS_UART_RX )
				packet_assemble_status = packet_assemble(2);
		}
		if (( wakeup & Line3_Uart_Drv.wakeup_id) == Line3_Uart_Drv.wakeup_id)
		{
			if (( flags & WAKEUP_FLAGS_UART_RX) == WAKEUP_FLAGS_UART_RX )
				packet_assemble_status = packet_assemble(3);
		}
	}
}

