#ifndef __INC_SYMBOS_KERNEL__
#define __INC_SYMBOS_KERNEL__

#include "symbos.h"

#define Multitasking_SoftInterrupt() \
__asm \
	rst 0x30 \
__endasm;

unsigned short Message_Sleep_And_Receive (unsigned char recID, unsigned char sendID, unsigned char *msg) __sdcccall(0);
unsigned char Message_Send (unsigned char sendID, unsigned char recID, unsigned char *msg) __sdcccall(0);
unsigned short Message_Receive (unsigned char recID, unsigned char sendID, unsigned char *msg) __sdcccall(0);
//void Multitasking_SoftInterrupt (void);
unsigned char Multitasking_Add_Process (unsigned char bank, APP_INFO *info, unsigned char prio) __sdcccall(0);
void Multitasking_Delete_Process (unsigned char pID) __sdcccall(0);
unsigned char Multitasking_Add_Timer (unsigned char bank, APP_INFO *info) __sdcccall(0);
void Multitasking_Delete_Timer (unsigned char pID) __sdcccall(0);

#endif //__INC_SYMBOS_KERNEL__
