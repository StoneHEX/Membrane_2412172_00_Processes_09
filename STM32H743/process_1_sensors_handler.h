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
 * process_1_sensors_handler.h
 *
 *  Created on: Feb 1, 2025
 *      Author: fil
 */

#ifndef PROCESS_1_SENSORS_HANDLER_H_
#define PROCESS_1_SENSORS_HANDLER_H_

// packet replies
#define	PKT_CRC_ERROR			'C'
#define	PKT_SEQ_ERROR			'N'
#define	PKT_NOK_ERROR			'E'
#define	PKT_OK					'Y'

extern	uint8_t 	power_on_serials(void);
extern	uint8_t 	power_off_serials(void);
extern	uint8_t 	send_get_acq_reply(void);
extern	void 		send_datarequest_to_sensors(uint8_t sensor_selector);
extern	void 		get_sensors_info(void);
extern	void 		check_sensors_flash(void);
extern	void 		send_update_packet_to_sensors(uint8_t *packet,uint16_t len);
extern	void 		send_update_command_to_sensors(void);
extern	void 		send_write_command_to_sensors(void);
extern	void 		updater_create_packet(uint8_t address);
extern	uint32_t 	send_update(void);
extern	void 		get_sensors_info(void);
extern	uint32_t 	send_update_clear_packet(void);


#endif /* PROCESS_1_SENSORS_HANDLER_H_ */
