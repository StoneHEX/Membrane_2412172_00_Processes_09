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
 * process_2_flasher.c
 *
 *  Created on: Feb 1, 2025
 *      Author: fil
 */

#include "main.h"
#include "A_os_includes.h"
#include "membrane_global_includes.h"

extern	QSPI_HandleTypeDef hqspi;

W25Qxx_Drv_TypeDef W25Qxx_Drv =
{
		.qspi_bus = &hqspi,
		.flags = QSPI_USES_DMA,
		.FlashSize = 128,
		.wakeup_id = WAKEUP_FROM_QSPI_IRQ,
};
uint32_t	w25_handle;

uint8_t		from_commprc_mbx[PRC2_FLASHER_MAILBOX_LEN];
uint32_t	buffer_addr , buffer_len , done = 0,qspi_error=0;
uint32_t	qspi_state = 2;

void parse_mbx_in(void)
{
int pnum;
char cmd;
	bzero((char *)from_commprc_mbx,PRC2_FLASHER_MAILBOX_LEN);
	if ( mbx_receive(PRC1_COMM_MAILBOX_ID,from_commprc_mbx) == 0)
		return;
	pnum = sscanf((char *)from_commprc_mbx,"%c %d %d",&cmd, (int *)&buffer_addr, (int *)&buffer_len);
	if ( pnum == 3 )
	{
		if ( cmd == CONCENTRATOR_WRITE_FLASH)
		{
			qspi_state = 0;
		}
	}
	if ( pnum == 1 )
	{
		if ( cmd == CONCENTRATOR_READ_FLASH)
		{
			qspi_state = 2;
		}

	}
}

#define	DO_MD5	1
#ifdef DO_MD5

void check_md5(void)
{
MembraneFlashHeader_TypeDef	*header_ihex = (MembraneFlashHeader_TypeDef *)MembraneXMODEM.xmodem_area;
	md5( MembraneXMODEM.xmodem_data_area, header_ihex->xmodem_file_len, MembraneFlashHeader.xmodem_file_md5 );
}
#endif // #ifdef DO_MD5

void qspi_sm(void)
{
uint32_t	i;
	switch(qspi_state)
	{
	case 0 :
		qspi_error += qspi_erase_blocks(w25_handle,0,8);
		if (( W25Qxx_Drv.status & QSPI_BUSY ) != QSPI_BUSY )
			qspi_state++;
		break;
	case 1 :
		qspi_error += qspi_write(w25_handle,0,MembraneXMODEM.xmodem_area,MembraneXMODEM.xmodem_len);
		if (( W25Qxx_Drv.status & QSPI_BUSY ) != QSPI_BUSY )
			qspi_state++;
		break;
	case 2 :
		qspi_error += qspi_read(w25_handle,0,MembraneXMODEM.xmodem_area,MembraneXMODEM.xmodem_len);
		if (( W25Qxx_Drv.status & QSPI_BUSY ) != QSPI_BUSY )
			qspi_state++;
		break;
	case 3 :
		qspi_state++;
		break;
	case 4 :
		qspi_state++;
		break;
	case 5 :
		qspi_state++;
		break;
	case 6 :
#ifdef DO_MD5
		check_md5();
#endif // #ifdef DO_MD5
		qspi_state++;
		break;
	case 7 :
		for(i=0;i<0x10000;i++)
			MembraneXMODEM.sensors_code_area[i] = 0xff;
		qspi_state++;
		break;
	case 8:
		qspi_state++;
		break;
	case 9:
		qspi_state++;
		break;
	case 10:
		qspi_state++;
		break;
	case 11 :
		if ( ihex_decode_area(MembraneXMODEM.sensors_code_area,MembraneXMODEM.xmodem_data_area) != 0 )
			MembraneSystem.external_flash_status |= EXTFLASH_VALID;
		qspi_state++;
		break;
	default :
		break;
	}

}

void process_2_flasher(uint32_t process_id)
{
uint32_t	wakeup,flags;

	w25_handle = w25qxx_register(&W25Qxx_Drv);
	create_timer(TIMER_ID_0,100,TIMERFLAGS_FOREVER | TIMERFLAGS_ENABLED);
	while(1)
	{
		wait_event(EVENT_TIMER | EVENT_MBX);
		get_wakeup_flags(&wakeup,&flags);
		if (( wakeup & WAKEUP_FROM_TIMER) == WAKEUP_FROM_TIMER)
		{
			qspi_sm();
		}
		if (( wakeup & WAKEUP_FROM_MBX) == WAKEUP_FROM_MBX)
		{
			parse_mbx_in();
		}

	}
}



