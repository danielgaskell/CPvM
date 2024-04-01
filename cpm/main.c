/*=============================================================================
 CP\/M SymbOS CP/M Compatibility Layer
 (C) Copyright 2024, Prevtenet & Prodatron

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the �Software�), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
=============================================================================*/

//#define TRACE

#include "..\sdk\symbos.h"
#include "..\sdk\symbos_system.h"
#include "..\sdk\symbos_kernel.h"
#include "..\sdk\symbos_memory.h"
#include "..\sdk\symbos_shell.h"
#include "..\sdk\symbos_file.h"
#include <stdlib.h>
#include <string.h>
#include "resource.h"
#include "globals.h"
#include "tpa.c"
#include "sdk.c"
#include "util.c"
#include "ccp.c"
#include "bdos.c"

void main(void) {
    // set up single-char control-code mappings (mostly ADM-3A)
    control_codes = "        \x08\x19\x0A\x0B\x09\x0D   \x06 \x15   \x14\x12 \x0C     ";
    keyconv[0] = 5;  //    up arrow = Wordstar-style Ctrl+E
    keyconv[1] = 24; //  down arrow = Wordstar-style Ctrl+X
    keyconv[2] = 19; //  left arrow = Wordstar-style Ctrl+S
    keyconv[3] = 4;  // right arrow = Wordstar-style Ctrl+D

    // set up internal values
    proc_id = symbos_info.processID;
    bnk_vm = GET_APPLICATION_BANK();
    dir_buffer_on = 0;
    dir_buffer_count = 0;
    escape_chars_expected = 0;
    out_buffer = buffer3;
    memset(handles_used, 0, 8);
    launch_ccp = 0;
    cursor_on = 0;
    inverse = 0;

    // parse command-line, loading console in the process (FIXME check for >=2.3!)
    termpid = 0; // ensure console load will fail unless explicitly found in command line
    termresp = 0;
    parse_command_tail(Shell_StartParam(CODEBASE), 0);

    // terminal version warning
    if (termver < 23)
        strout("SymShell 2.3+ required for full compatibility.\r\n");

    // set up default paths
    Shell_PathAdd(termpid, bnk_vm, 0, "", &drive_paths[0][0]);
    strcpy(&drive_paths[1][0], "A:\\");
    strcpy(&drive_paths[2][0], "B:\\");
    strcpy(&drive_paths[3][0], "C:\\");

    // set up CP/M values and load TPA
    drive_ro = 0;
    default_drive = 0;
    login_vector = 1; // just A drive logged in
    user_number = 0;
    tpa_error = tpa_init(); // init TPA, including most of base page
    if (tpa_error) {
        strout("Out of memory\r\n");
        symbos_exit();
    }

    // load DPB into TPA
    Banking_Copy(bnk_vm, (unsigned short)&generic_dpb, bnk_tpa, DPBADDR, 16);

    // load app
    if (launch_ccp) {
        strout("\r\nCP\\/M - SymbOS CP/M Virtual Machine\r\n");
        strout("(c) 2024 Prevtenet & Prodatron\r\n");
        strout("===================================\r\n\n");
        strout("CP/M 2.2 (C) 1979, Digital Research\r\n");
        strout("CPvM 0.6, 63K TPA\r\n");
        ccp();
    } else {
        Banking_Copy(bnk_vm, (unsigned short)buffer2, bnk_tpa, 0x5C, 164); // load command-line arguments (generated by symbos_coninit) into base page
        strcpy(buffer2, dir_buffer);
        reg_a = app_load(0x100);
        if (reg_a) {
            strout("Error loading .COM file\r\n");
            symbos_exit();
        }
        app_run();
    }
}

// INSANITY ALERT: For reasons I absolutely cannot fathom, on this project only,
// the SymbosMake routine for creating the relocation table goes haywire unless
// the code segment is padded to the correct size. Exactly what size this is
// I have been unable to ascertain, but in the meantime, adjust the number below
// whenever it starts generating a too-large file that doesn't run.
void padding(void) __naked {
    __asm
    .bndry 4
    __endasm;
}
