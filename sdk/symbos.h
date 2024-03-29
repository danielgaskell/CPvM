// SYMBOS API for SDCC
// Coded by: Alberto De Hoyo Nebot (Nerlaska)

#ifndef __INC_SYMBOS__
#define __INC_SYMBOS__

#ifndef SYMBOS_STACKBUFFER_SIZE
#define SYMBOS_STACKBUFFER_SIZE  128
#endif

#ifndef SYMBOS_MSGBUFFER_SIZE
#define SYMBOS_MSGBUFFER_SIZE  14
#endif

//P R O C E S S - I D S
#define PRC_ID_KERNEL    1   //kernel process
#define PRC_ID_DESKTOP   2   //desktop manager process
#define PRC_ID_SYSTEM    3   //system manager process

//M E S S A G E S
//General
#define MSC_GEN_QUIT     0   //application is beeing asked, to quit itself
#define MSC_GEN_FOCUS    255 //application is beeing asked, to focus its window

//Kernel Commands
#define MSC_KRL_MTADDP   1   //add process (P1/2=stack, P3=priority (7 high - 1 low), P4=ram bank (0-8))
#define MSC_KRL_MTDELP   2   //delete process (P1=ID)
#define MSC_KRL_MTADDT   3   //add timer (P1/2=stack, P4=ram bank (0-8))
#define MSC_KRL_MTDELT   4   //delete timer (P1=ID)
#define MSC_KRL_MTSLPP   5   //set process to sleep mode
#define MSC_KRL_MTWAKP   6   //wake up process
#define MSC_KRL_TMADDT   7   //add counter service (P1/2=address, P3=ram bank, P4=process, P5=frequency)
#define MSC_KRL_TMDELT   8   //delete counter service (P1/2=address, P3=ram bank)
#define MSC_KRL_TMDELP   9   //delete all counter services of one process (P1=process ID)

//Kernel Responses
#define MSR_KRL_MTADDP   129 //process has been added (P1=0/1->ok/failed, P2=ID)
#define MSR_KRL_MTDELP   130 //process has been deleted
#define MSR_KRL_MTADDT   131 //timer process has been deleted (P1=0/1->ok/failed, P2=ID)
#define MSR_KRL_MTDELT   132 //timer has been removed
#define MSR_KRL_MTSLPP   133 //process is sleeping now
#define MSR_KRL_MTWAKP   134 //process has been waked up
#define MSR_KRL_TMADDT   135 //counter service has been added (P1=0/1->ok/failed)
#define MSR_KRL_TMDELT   136 //counter service has been deleted
#define MSR_KRL_TMDELP   137 //all counter services of a process have been deleted

//System Commands
#define MSC_SYS_PRGRUN   16  //load application or document (P1/2=address filename, P3=ram bank filename)
#define MSC_SYS_PRGEND   17  //quit application (P1=ID)
#define MSC_SYS_SYSWNX   18  //open dialogue to change current window (next) (-)
#define MSC_SYS_SYSWPR   19  //open dialogue to change current window (previouse) (vorheriges) (-)
#define MSC_SYS_PRGSTA   20  //open dialogue to load application or document (-)
#define MSC_SYS_SYSSEC   21  //open system secuity dialogue (-)
#define MSC_SYS_SYSQIT   22  //open shut shown dialogue (-)
#define MSC_SYS_SYSOFF   23  //shut down (-)
#define MSC_SYS_PRGSET   24  //start control panel (P1=submodul -> 0=main window, 1=display settings, 2=date/time)
#define MSC_SYS_PRGTSK   25  //start taskmanager (-)
#define MSC_SYS_SYSFIL   26  //call filemanager function (P1=number, P2-13=AF,BC,DE,HL,IX,IY)
#define MSC_SYS_SYSHLP   27  //start help (-)
#define MSC_SYS_SYSCFG   28  //call config function (P1=number, 0=load, 1=save, 2=reload background)
#define MSC_SYS_SYSWRN   29  //open warning window (P1/2=adresse, P3=ram bank, P4=number of buttons)
#define MSC_SYS_PRGSRV   30  //shared service function (P4=type [0=search, 1=start, 2=release], P1/2=addresse 12char ID, P3=ram bank 12char ID or P3=program ID, if type=2)
#define MSC_SYS_SELOPN   31  //open fileselect dialogue (P6=filename ram bank, P8/9=filename address, P7=forbidden attributes, P10=max entries, P12=max buffer size)

//System Responses
#define MSR_SYS_PRGRUN   144 //application has been started (P1=result -> 0=ok, 1=file doesnt exist, 2=file is not executable, 3=error while loading  [P8=filemanager error code], 4=memory full, P8=app ID, P9=process ID)
#define MSR_SYS_SYSFIL   154 //filemanager function returned (P1=number, P2-13=AF,BC,DE,HL,IX,IY)
#define MSR_SYS_SYSWRN   157 //The system manager sends back this message to the application, when a infobox should be opened, or if the user clicked one of the buttons.
#define MSR_SYS_PRGSRV   158 //shared service function response (P1=state [5=not found, other codes see MSR_SYS_PRGRUN], P8=app ID, P9=process ID)
#define MSR_SYS_SELOPN   159 //message from fileselect dialogue (P1 -> 0=Ok, 1=cancel, 2=already in use, 3=no memory free, 4=no window free, -1=open ok, super window has been opened [P2=number])

//Desktop Commands
#define MSC_DSK_WINOPN   32  //open window (P1=ram bank, P2/3=address data record)
#define MSC_DSK_WINMEN   33  //redraw menu bar (P1=window ID) [only if focus]
#define MSC_DSK_WININH   34  //redraw window content (P1=window ID, P2=-1/-Num/Object, P3=Object) [only if focus]
#define MSC_DSK_WINTOL   35  //redraw window toolbar (P1=window ID) [only if focus]
#define MSC_DSK_WINTIT   36  //redraw window title (P1=window ID) [only if focus]
#define MSC_DSK_WINSTA   37  //redraw window status lien (P1=window ID) [only if focus]
#define MSC_DSK_WINMVX   38  //set content x offset (P1=window ID, P2/3=XPos) [only if focus]
#define MSC_DSK_WINMVY   39  //set content y offset (P1=window ID, P2/3=XPos) [only if focus]
#define MSC_DSK_WINTOP   40  //takes window to the front (P1=window ID) [always]
#define MSC_DSK_WINMAX   41  //maximize window (P1=window ID) [always]
#define MSC_DSK_WINMIN   42  //minimize window (P1=window ID) [always]
#define MSC_DSK_WINMID   43  //restore window size (P1=window ID) [always]
#define MSC_DSK_WINMOV   44  //moves window to a new position (P1=window ID, P2/3=XPos, P4/5=YPos) [always]
#define MSC_DSK_WINSIZ   45  //resize the window (P1=window ID, P2/3=XPos, P4/5=YPos) [always]
#define MSC_DSK_WINCLS   46  //closes and removes window (P1=window ID) [always]
#define MSC_DSK_WINDIN   47  //redraw window content, even if it hasnt focus (P1=window ID, P2=-1/Objekt) [always]
#define MSC_DSK_DSKSRV   48  //desktop service request (P1=type, P2-P5=parameters)
#define MSC_DSK_WINSLD   49  //redraw window scrollbars (P1=window ID) [only if focus]
#define MSC_DSK_WINPIN   50  //redraw window content part (P1=window ID, P2=-1/-Num/Object, P3=Object, P4/5=Xbeg, P6/7=Ybeg, P8/9=Xlen, P10/11=Xlen) [always]

//Desktop Responses
#define MSR_DSK_WOPNER   160 //open window failed// the maximum of 32 windows has been reached
#define MSR_DSK_WOPNOK   161 //open window successfull (P4=number)
#define MSR_DSK_WCLICK   162 //window has been clicked (P1=window number, P2=action, P3=subspezification, P4/5,P6/7,P8/9=parameters)
#define MSR_DSK_DSKSRV   163 //desktop service answer (P1=type, P2-P5=parameters)
#define MSR_DSK_WFOCUS   164 //window got/lost focus (P1=window number, P2=type [0=blur, 1=focus])
#define MSR_DSK_CFOCUS   165 //control focus changed (P1=window number, P2=control number, P3=reason [0=mouse click/wheel, 1=tab key])

//Shell Commands
#define MSC_SHL_CHRINP   64  //char is requested (P1=channel [0=standard, 1=keyboard])
#define MSC_SHL_STRINP   65  //line is requested (P1=channel [0=standard, 1=keyboard], P2=ram bank, P3/4=address)
#define MSC_SHL_CHROUT   66  //char should be writtten (P1=channel [0=standard, 1=screen], P2=char)
#define MSC_SHL_STROUT   67  //line should be writtten (P1=channel [0=standard, 1=screen], P2=ram bank, P3/4=address, P5=length)
#define MSC_SHL_EXIT     68  //application released focus or quit itself (P1 -> 0=quit, 1=blur)

//Shell Responses
#define MSR_SHL_CHRINP   192 //char has been received (P1=EOF-flag [0=no EOF], P2=char, P3=error status)
#define MSR_SHL_STRINP   193 //line has been received (P1=EOF-flag [0=no EOF], P3=error status)
#define MSR_SHL_CHROUT   194 //char has been written (P3=error status)
#define MSR_SHL_STROUT   195 //line has been written (P3=error status)

//Screensaver Messages
#define MSC_SAV_INIT     1   //initialises the screen saver (P1=bank of config data, P2/3=address of config data [64bytes])
#define MSC_SAV_START    2   //start screen saver
#define MSC_SAV_CONFIG   3   //open screen savers own config window (at the end the screen saver has to send the result back to the sender)
#define MSR_SAV_CONFIG   4   //returns user adjusted screen saver config data (P1=bank of config data, P2/3=address of config data [64bytes])

//D E S K T O P - A C T I O N S
#define DSK_ACT_CLOSE    5   //close button has been clicked or ALT+F4 has been pressed
#define DSK_ACT_MENU     6   //menu entry has been clicked (P8/9=menu entry value)
#define DSK_ACT_CONTENT  14  //a control of the content has been clicked (P3=sub spec [see dsk_sub...], P4=key or P4/5=Xpos within the window, P6/7=Ypos, P8/9=control value)
#define DSK_ACT_TOOLBAR  15  //a control of the toolbar has been clicked (see DSK_ACT_CONTENT)
#define DSK_ACT_KEY      16  //key has been pressed without touching/modifying a control (P4=Ascii Code)

#define DSK_SUB_MLCLICK  0   //left mouse button has been clicked
#define DSK_SUB_MRCLICK  1   //right mouse button has been clicked
#define DSK_SUB_MDCLICK  2   //double click with the left mouse button
#define DSK_SUB_MMCLICK  3   //middle mouse button has been clicked
#define DSK_SUB_KEY      7   //keyboard has been clicked and did modify/click a control (P4=Ascii Code)
#define DSK_SUB_MWHEEL   8   //mouse wheel has been moved (P4=Offset)

//D E S K T O P - S E R V I C E S
#define DSK_SRV_MODGET   1   //get screen mode (output P2=mode)
#define DSK_SRV_MODSET   2   //set screen mode (input P2=mode)
#define DSK_SRV_COLGET   3   //get colour      (input P2=number, output P2=number, P3/4=RGB value)
#define DSK_SRV_COLSET   4   //set colour      (input P2=number, P3/4=RGB value)
#define DSK_SRV_DSKSTP   5   //freeze desktop  (input P2=type [0=Pen0, 1=Raster, 2=background, 255=no screen modification, switch off mouse])
#define DSK_SRV_DSKCNT   6   //continue desktop
#define DSK_SRV_DSKPNT   7   //clear desktop   (Eingabe P2=Typ [0=Pen0, 1=Raster, 2=background])
#define DSK_SRV_DSKBGR   8   //initialize and redraw desktop background
#define DSK_SRV_DSKPLT   9   //redraw the complete desktop

//J U M P S
#define jmp_memsum   #0x8100 //MEMSUM
#define jmp_sysinf   #0x8103 //SYSINF
#define jmp_clcnum   #0x8106 //CLCNUM
#define jmp_mtgcnt   #0x8109 //MTGCNT
#define jmp_timget   #0x810C //TIMGET
#define jmp_timset   #0x810F //TIMSET
#define jmp_memget   #0x8118 //MEMGET
#define jmp_memfre   #0x811B //MEMFRE
#define jmp_memsiz   #0x811E //MEMSIZ
#define jmp_meminf   #0x8121 //MEMINF
#define jmp_bnkrwd   #0x8124 //BNKRWD
#define jmp_bnkwwd   #0x8127 //BNKWWD
#define jmp_bnkrbt   #0x812A //BNKRBT
#define jmp_bnkwbt   #0x812D //BNKWBT
#define jmp_bnkcop   #0x8130 //BNKCOP
#define jmp_bnkget   #0x8133 //BNKGET
#define jmp_filf2t   #0x8136 //FILF2T
#define jmp_filt2f   #0x8139 //FILT2F
#define jmp_mosget   #0x813C //MOSGET
#define jmp_moskey   #0x813F //MOSKEY
#define jmp_bnk16c   #0x8142 //BNK16C
#define jmp_keytst   #0x8145 //KEYTST
#define jmp_keysta   #0x8148 //KEYSTA
#define jmp_keyput   #0x814B //KEYPUT
#define jmp_bufput   #0x814E //BUFPUT
#define jmp_bufget   #0x8151 //BUFGET
#define jmp_bufsta   #0x8154 //BUFSTA

//Filemanager Functions (call via MSC_SYS_SYSFIL)
#define FNC_FIL_STOINI   0
#define FNC_FIL_STONEW   1
#define FNC_FIL_STORLD   2
#define FNC_FIL_STODEL   3
#define FNC_FIL_STOINP   4
#define FNC_FIL_STOOUT   5
#define FNC_FIL_STOACT   6
#define FNC_FIL_STOINF   7

#define FNC_FIL_DEVDIR   13
#define FNC_FIL_DEVINI   14
#define FNC_FIL_DEVSET   15

#define FNC_FIL_FILINI   16
#define FNC_FIL_FILNEW   17
#define FNC_FIL_FILOPN   18
#define FNC_FIL_FILCLO   19
#define FNC_FIL_FILINP   20
#define FNC_FIL_FILOUT   21
#define FNC_FIL_FILPOI   22
#define FNC_FIL_FILF2T   23
#define FNC_FIL_FILT2F   24
#define FNC_FIL_FILLIN   25

#define FNC_FIL_DIRDEV   32
#define FNC_FIL_DIRPTH   33
#define FNC_FIL_DIRPRS   34
#define FNC_FIL_DIRPRR   35
#define FNC_FIL_DIRREN   36
#define FNC_FIL_DIRNEW   37
#define FNC_FIL_DIRINP   38
#define FNC_FIL_DIRDEL   39
#define FNC_FIL_DIRRMD   40
#define FNC_FIL_DIRMOV   41
#define FNC_FIL_DIRINF   42

typedef struct
{
	unsigned short codeLength;
	unsigned short dataLength;
	unsigned short transferLength;
	unsigned short startCodeAddr;
	unsigned short numRTEntries;
	unsigned short stackSize;
	unsigned short crunchDataSize;
	unsigned char crunchType;
	char appName[33];
	char exeName[8];
	unsigned short xCodeLength;
	unsigned short xDataLength;
	unsigned short xTransferLength;
	char reserved[28];
	unsigned char smallIcon[19];
	unsigned char bigIcon[146];
} APP_HEADER;

#define GET_DATA_AREA_ADDR() (*((unsigned short *)CODEBASE+6))
#define GET_TRANSFER_AREA_ADDR() (*((unsigned short *)CODEBASE+8))
#define GET_TIMER_0() (*((unsigned char *)CODEBASE+10))
#define GET_TIMER_1() (*((unsigned char *)CODEBASE+11))
#define GET_TIMER_2() (*((unsigned char *)CODEBASE+12))
#define GET_TIMER_3() (*((unsigned char *)CODEBASE+13))
#define GET_APPLICATION_BANK() (*((unsigned char *)CODEBASE+14))
#define GET_APPLICATION_ID() (*((unsigned char *)CODEBASE+88))
#define GET_MAIN_PROC_ID() (*((unsigned char *)CODEBASE+89))

typedef struct
{
	unsigned int IY;
	unsigned int IX;
	unsigned int HL;
	unsigned int DE;
	unsigned int BC;
	unsigned int AF;
	unsigned int startAddr;
	unsigned char processID;
	unsigned char msgBuffer[SYMBOS_MSGBUFFER_SIZE];
} APP_INFO;

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef NULL
#define NULL 0
#endif

extern APP_INFO symbos_info;

#endif //__INC_SYMBOS__
