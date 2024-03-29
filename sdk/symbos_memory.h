#ifndef __INC_SYMBOS_MEMORY__
#define __INC_SYMBOS_MEMORY__

#include "symbos.h"

typedef struct
{
	unsigned char bank;
	unsigned int len;
} MEMORY;

#define MEMORY_CODE 			0
#define MEMORY_DATA				1
#define MEMORY_TRANSFER		2
#define MEMORY_ANY				-1

#define MEMORY_GETBANK(m) (((MEMORY *)((unsigned int)m - sizeof(MEMORY)))->bank)
#define MEMORY_GETLEN(m) (((MEMORY *)((unsigned int)m - sizeof(MEMORY)))->len)

void Banking_Copy (unsigned char bankSrc, unsigned int src, unsigned char bankDst, unsigned int dst, unsigned int len) __sdcccall(0);
unsigned short Banking_ReadWord (unsigned char bank, void *addr) __sdcccall(0);
void Banking_WriteWord (unsigned char bank, void *addr, unsigned short word) __sdcccall(0);
unsigned char Banking_ReadByte (unsigned char bank, void *addr) __sdcccall(0);
void Banking_WriteByte (unsigned char bank, void *addr, unsigned char byte) __sdcccall(0);

unsigned long Memory_Summary (void) __sdcccall(0);
void *Memory_New (unsigned int len, unsigned char type) __sdcccall(0);
void Memory_Delete (void *data) __sdcccall(0);
void Memory_Copy (void *dst, void *src, unsigned int len) __sdcccall(0);

#endif
