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
 * process_1_usb_handler.h
 *
 *  Created on: Feb 1, 2025
 *      Author: fil
 */

#ifndef PROCESS_1_USB_HANDLER_H_
#define PROCESS_1_USB_HANDLER_H_

extern	uint8_t pack_USB_packet(uint8_t *rx_buf,uint8_t len);
extern	uint8_t decode_USB_packet(uint8_t* Buf);
extern	uint8_t parse_USB_packet(uint8_t* Buf);
extern	void 	send_sensor_version(uint8_t address,uint8_t type , char *packet);
extern	void 	send_sensor_update_progress(uint8_t address, uint32_t pkt_num);

#endif /* PROCESS_1_USB_HANDLER_H_ */
