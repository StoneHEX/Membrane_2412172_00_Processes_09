/*
 * processes_table.c
 *
 *  Created on: Sep 13, 2023
 *      Author: fil
 */


#include "main.h"
#include "A_os_includes.h"

#include "membrane_global_includes.h"

VERSIONING	uint8_t	app_name[16] 		= "i.con C";
VERSIONING	uint8_t	app_version[16] 	= "1.0.0";
VERSIONING	uint8_t	a_version[32] 		= A_OS_VERSION;

extern	void process_1_comm(uint32_t process_id);	//This is process2
extern	void process_2_flasher(uint32_t process_id);	//This is process2
extern	void process_3(uint32_t process_id);	//This is process3
extern	void process_4(uint32_t process_id);	//This is process4 of the application


MembraneSystem_TypeDef		MembraneSystem;
MembraneUSB_TypeDef			MembraneUSB;
MembraneFlashHeader_TypeDef	MembraneFlashHeader;
MembraneXMODEM_TypeDef		MembraneXMODEM;

USRprcs_t	UserProcesses[USR_PROCESS_NUMBER] =
{
		{
				.user_process = process_1_comm,
				.stack_size = 2048,
		},
		{
				.user_process = process_2_flasher,
				.stack_size = 2048,
		},
		{
				.user_process = process_3,
				.stack_size = 256,
		},
		{
				.user_process = process_4,
				.stack_size = 256,
		}
};

