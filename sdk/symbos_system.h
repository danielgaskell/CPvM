#ifndef __INC_SYMBOS_SYSTEM__
#define __INC_SYMBOS_SYSTEM__

#include "symbos.h"

#define OFD_FILTER_READONLY			0x01
#define OFD_FILTER_HIDDEN				0x02
#define OFD_FILTER_SYSTEM				0x04
#define OFD_FILTER_VOLID				0x08
#define OFD_FILTER_DIRECTORIES	0x10
#define OFD_FILTER_ARCHIVES			0x20
#define OFD_FILTER_NOTHING      0x00

#define OFD_MODE_SAVE           0x40
#define OFD_MODE_FOLDER         0x80

#define SMB_OK						1
#define SMB_YESNO         2
#define SMB_YESNOCANCEL   3
#define SMB_ID_OK         2
#define SMB_ID_YES        3
#define SMB_ID_NO         4
#define SMB_ID_CANCEL     5

typedef struct
{
	char *line1;
	unsigned int pen1;
	char *line2;
	unsigned int pen2;
	char *line3;
	unsigned int pen3;
	unsigned char *icon;
} MESSAGE_BOX;

#define CONTROLPANEL_MAIN					0
#define CONTROLPANEL_DISPLAY			1
#define CONTROLPANEL_TIMEDATE			2

#define PostQuitMessage() Sys_Program_End (GET_APPLICATION_ID())

#define MESSAGEBOX(t) \
{ \
	MESSAGE_BOX var; \
	memset (&var, 0, sizeof(MESSAGE_BOX)); \
	var.line1 = t; \
	var.pen1 = 6; \
	Sys_MessageBox (GET_APPLICATION_BANK(), &var, SMB_OK); \
}

#define Sys_FileDialog_MakePath(p,fn,ext) \
{ \
	strcpy (p, ext); \
	p[3] = 0; \
	strcpy (&p[4], fn); \
}

typedef struct
{
	unsigned char pid;
	unsigned char aid;
} SYS_PROGRAM_INFO;

unsigned char Sys_Program_Run (unsigned char bank, char *path, unsigned char showError, SYS_PROGRAM_INFO *pInfo) __sdcccall(0);
void Sys_Program_End (unsigned char aID) __sdcccall(0);
void Sys_RunDialog (void) __sdcccall(0);
void Sys_ControlPanel (unsigned char subMod) __sdcccall(0);
void Sys_TaskManager (void) __sdcccall(0);
unsigned char Sys_MessageBox (unsigned char bank, MESSAGE_BOX *mb, unsigned char mode) __sdcccall(0);
char Sys_FileDialog (unsigned char bank, char *path, unsigned char filter) __sdcccall(0);
void Sys_GetComputerType (unsigned char *computerType) __sdcccall(0);

#endif
