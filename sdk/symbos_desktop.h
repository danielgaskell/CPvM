#ifndef __INC_SYMBOS_DESKTOP__
#define __INC_SYMBOS_DESKTOP__

#include "symbos.h"

#define CLIPBOARD_TYPE_TEXT  			1
#define CLIPBOARD_TYPE_GRAPHIC		2
#define CLIPBOARD_TYPE_ITEMLIST		3
#define CLIPBOARD_TYPE_BINARY			4

#define SW_STATUS_CLOSED    			0
#define SW_STATUS_NORMAL    			1
#define SW_STATUS_MAXIMIZED				2
#define SW_STATUS_MINIMIZED 			3

#define SW_STYLE_SHOWICON					0x0001
#define SW_STYLE_RESIZEABLE				0x0002
#define SW_STYLE_SHOWCLOSE				0x0004
#define SW_STYLE_SHOWTOOLBAR			0x0008
#define SW_STYLE_SHOWTITLE				0x0010
#define SW_STYLE_SHOWMENU					0x0020
#define SW_STYLE_SHOWSTATUS				0x0040
#define SW_STYLE_RESERVED1				0x0080
#define SW_STYLE_ADJUSTWIDTH			0x0100
#define SW_STYLE_ADJUSTHEIGTH			0x0200
#define SW_STYLE_NOTTASKBAR				0x0400
#define SW_STYLE_NOTMOVEABLE			0x0800
#define SW_STYLE_SUPERWINDOW			0x1000
#define SW_STYLE_RESERVED2				0x2000
#define	SW_STYLE_RESERVED3				0x4000
#define SW_STYLE_RESERVED4				0x8000

typedef struct
{
	unsigned char numControls;
	unsigned char ownerProcID;
	unsigned short cdr;
	unsigned short crdr;
	unsigned char reserved1[2];
	unsigned char objRet;
	unsigned char objEsc;
	unsigned char reserved2[4];
	unsigned char objFocus;
	unsigned char reserved3;
} SYMBOS_CONTROL_GROUP_DATA_RECORD;

typedef struct
{
	unsigned short id;
	unsigned char type;
	unsigned char bank;
	unsigned short param;
	unsigned short x, y;
	unsigned short w, h;
	unsigned char reserved1[2];
} SYMBOS_CONTROL_DATA_RECORD;

typedef struct
{
	unsigned short x;
	unsigned char xMul;
	unsigned char xDiv;
	unsigned short y;
	unsigned char yMul;
	unsigned char yDiv;
	unsigned short w;
	unsigned char wMul;
	unsigned char wDiv;
	unsigned short h;
	unsigned char hMul;
	unsigned char hDiv;
} SYMBOS_CALCULATION_RULE_DATA_RECORD;

typedef struct
{
	unsigned short flags;
	char* textAddr;
	unsigned short subMenuAddr;
	unsigned short reserved;
} SYMBOS_MENU_DATA_RECORD_ITEM;

typedef struct
{
	unsigned short numItems;
	//SYMBOS_MENU_DATA_RECORD_ITEM *item; // removed - shouldn't be here!
} SYMBOS_MENU_DATA_RECORD;

typedef struct
{
	unsigned char status;
	unsigned short style;
	unsigned char pID;
	unsigned short x, y;
	unsigned short w, h;
	unsigned short offX, offY;
	unsigned short fullW, fullH;
	unsigned short minW, minH;
	unsigned short maxW, maxH;
	unsigned short icon;
	char *titleText;
	char *statusText;
	unsigned short mdr;
	SYMBOS_CONTROL_GROUP_DATA_RECORD *cgdr;
	SYMBOS_CONTROL_GROUP_DATA_RECORD *cgdr_toolbar;
	unsigned short toolbarH;
	unsigned char reserved1[9];
	unsigned char superWindow;
	unsigned char reserved2[140];
} SYMBOS_WINDOW;

unsigned char Window_Open (unsigned char bank, SYMBOS_WINDOW *win) __sdcccall(0);
void Window_Close (unsigned char winID) __sdcccall(0);
void Window_Redraw (unsigned char winID, unsigned char content, unsigned char firstControl) __sdcccall(0);
void Window_Redraw_Menu (unsigned char winID) __sdcccall(0);
void Window_Redraw_Toolbar (unsigned char winID, unsigned char content, unsigned char firstControl) __sdcccall(0);
void Window_Redraw_Title (unsigned char winID) __sdcccall(0);
void Window_Redraw_Statusbar (unsigned char winID) __sdcccall(0);

char Clipboard_Put (unsigned char bank, unsigned char *addr, unsigned int len, unsigned char type) __sdcccall(0);

unsigned char Desktop_GetScreenMode (void) __sdcccall(0);
void Desktop_SetScreenMode (unsigned char mode) __sdcccall(0);
unsigned short Desktop_GetColor (unsigned char ix) __sdcccall(0);
void Desktop_SetColor (unsigned char ix, unsigned short value) __sdcccall(0);
char Desktop_Stop (unsigned int winID) __sdcccall(0);
void Desktop_Continue (void) __sdcccall(0);

#endif
