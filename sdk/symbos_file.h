#ifndef __INC_SYMBOS_FILE__
#define __INC_SYMBOS_FILE__

#include "symbos.h"

#define File_GetErrorCode() (symbos_msgBuffer[2])

#define FILE_READONLY	0x01
#define FILE_HIDDEN		0x02
#define FILE_SYSTEM		0x04
#define FILE_ARCHIVE	0x20

#define SEEK_SET      0x00
#define SEEK_CUR      0x01
#define SEEK_END      0x02

// Used internally
void File_Call (void) __sdcccall(0);

int File_New (unsigned char bank, char *path, char attrib) __sdcccall(0);
int File_Open (unsigned char bank, char *path) __sdcccall(0);
int File_Close (int handle) __sdcccall(0);
unsigned int File_Read (int handle, unsigned char bank, unsigned char *buffer, unsigned int bytes) __sdcccall(0);
unsigned int File_Write (int handle, unsigned char bank, unsigned char *buffer, unsigned int bytes) __sdcccall(0);
unsigned long File_Seek (int handle, unsigned long pos, unsigned char ref) __sdcccall(0);
int File_GetLine (int handle, unsigned char bank, char *buffer) __sdcccall(0);
unsigned long File_Size (int handle) __sdcccall(0);

#endif
