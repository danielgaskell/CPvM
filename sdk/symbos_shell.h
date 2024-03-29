#ifndef __SYMBOS_SHELL__
#define __SYMBOS_SHELL__

#define SHELL_OUT_STANDARD  0
#define SHELL_OUT_SCREEN    1

int Shell_CharInput (unsigned char shellPID, unsigned char channel) __sdcccall(0);
void Shell_CharOutput (unsigned char shellPID, unsigned char channel, unsigned char byte) __sdcccall(0);
int Shell_StringInput (unsigned char shellPID, unsigned char channel, unsigned char bank, char *str) __sdcccall(0);
void Shell_StringOutput (unsigned char shellPID, unsigned char channel, unsigned char bank, unsigned char *str, int len) __sdcccall(0);
void Shell_Exit (unsigned char shellPID, int mode) __sdcccall(0);
char *Shell_StartParam (unsigned short startCodeAddr) __sdcccall(0);
char *Shell_NextParam (char *param, char *str) __sdcccall(0);

#endif
