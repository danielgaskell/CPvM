#ifndef __INC_SYMBOS_DEVICE__
#define __INC_SYMBOS_DEVICE__

#include "symbos.h"

typedef struct
{
	unsigned char second;
	unsigned char hour;
	unsigned char minute; // FIXED - D.E.G. 2024-01-21 - hour/minute was backwards from how it's saved by Device_TimeGet()
	unsigned char day;
	unsigned char month;
	unsigned char timeZone;
	unsigned int year;
} TIME;

#define KEY_X_UP				136
#define KEY_X_DOWN			137
#define KEY_X_LEFT  		138
#define KEY_X_RIGHT 		139
#define KEY_X_F0				140
#define KEY_X_F1				141
#define KEY_X_F2				142
#define KEY_X_F3				143
#define KEY_X_F4				144
#define KEY_X_F5				145
#define KEY_X_F6				146
#define KEY_X_F7				147
#define KEY_X_F8				148
#define KEY_X_F9				149
#define KEY_X_FN				150
#define KEY_ALT_ARROBA	151 // ALT + @
#define KEY_ALT_A				152
#define KEY_ALT_B				153
#define KEY_ALT_C				154
#define KEY_ALT_D				155
#define KEY_ALT_E				156
#define KEY_ALT_F				157
#define KEY_ALT_G				158
#define KEY_ALT_H				159
#define KEY_ALT_I				160
#define KEY_ALT_J				161
#define KEY_ALT_K				162
#define KEY_ALT_L				163
#define KEY_ALT_M				164
#define KEY_ALT_N				165
#define KEY_ALT_O				166
#define KEY_ALT_P				167
#define KEY_ALT_Q				168
#define KEY_ALT_R				169
#define KEY_ALT_S				170
#define KEY_ALT_T				171
#define KEY_ALT_U				172
#define KEY_ALT_V				173
#define KEY_ALT_W				174
#define KEY_ALT_X				175
#define KEY_ALT_Y				176
#define KEY_ALT_Z				177
#define KEY_ALT_0				178
#define KEY_ALT_1				179
#define KEY_ALT_2				180
#define KEY_ALT_3				181
#define KEY_ALT_4				182
#define KEY_ALT_5				183
#define KEY_ALT_6				184
#define KEY_ALT_7				185
#define KEY_ALT_8				186
#define KEY_ALT_9				187

#define KEY_UP				0
#define KEY_RIGHT			1
#define KEY_DOWN			2
#define KEY_F9				3
#define KEY_F6				4
#define KEY_F3				5
#define KEY_ENTER			6
#define KEY_FN				7
#define KEY_LEFT			8
#define KEY_ALT				9
#define KEY_F7				10
#define KEY_F8				11
#define KEY_F5				12
#define KEY_F1				13
#define KEY_F2				14
#define KEY_F0				15
#define KEY_CLR				16
#define KEY_OBRACKET	17
#define KEY_RETURN		18
#define KEY_CBRACKET	19
#define KEY_F4				20
#define KEY_SHIFT			21
#define KEY_BACKSLASH	22
#define KEY_CONTROL		23
#define KEY_OVER			24		// ^
#define KEY_MINUS			25		// -
#define KEY_ARROBA		26		// @
#define KEY_P					27
#define KEY_SEMICOLON 28		// ;
#define KEY_COLON			29		// :
#define KEY_SLASH			30		// /
#define KEY_DOT				31		// .
#define KEY_0					32
#define KEY_9					33
#define KEY_O					34
#define KEY_I					35
#define KEY_L					36
#define KEY_K					37
#define KEY_M					38
#define KEY_COMMA			39
#define KEY_8					40
#define KEY_7					41
#define KEY_U					42
#define KEY_Y					43
#define KEY_H					44
#define KEY_J					45
#define KEY_N					46
#define KEY_SPACE 		47
#define KEY_6					48
#define KEY_5					49
#define KEY_R					50
#define KEY_T					51
#define KEY_G					52
#define KEY_F					53
#define KEY_B					54
#define KEY_V					55
#define KEY_4					56
#define KEY_3					57
#define KEY_E					58
#define KEY_W					59
#define KEY_S					60
#define KEY_D					61
#define KEY_C					62
#define KEY_X					63
#define KEY_1					64
#define KEY_2					65
#define KEY_ESC				66
#define KEY_ESCAPE		66
#define KEY_Q					67
#define KEY_TAB				68
#define KEY_A					69
#define KEY_CAPS			70
#define KEY_Z					71
#define JOY_UP				72
#define JOY_DOWN			73
#define JOY_LEFT			74
#define JOY_RIGHT			75
#define JOY_FIRE_1		76
#define JOY_FIRE_2		77
#define KEY_DEL				78
#define KEY_DELETE		78

unsigned char Device_KeyPut (unsigned char asciiCode) __sdcccall(0);
unsigned int Device_KeyStatus (unsigned char scanCode) __sdcccall(0);
unsigned char Device_KeyTest (unsigned char scanCode) __sdcccall(0);
unsigned char Device_MouseKeyStatus (void) __sdcccall(0);
unsigned long Device_MousePosition (void) __sdcccall(0);
void Device_TimeSetEx (TIME *t) __sdcccall(0);
void Device_TimeSet (unsigned char second, unsigned char minute, unsigned char hour, unsigned char day, unsigned char month, unsigned int year, unsigned char timeZone) __sdcccall(0);
void Device_TimeGet (TIME *t) __sdcccall(0);

#endif
