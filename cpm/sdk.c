/*=============================================================================
 CP\/M SymbOS CP/M Compatibility Layer
 (C) Copyright 2024, Prevtenet & Prodatron

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the “Software”), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
=============================================================================*/

#define ATTRIB_READONLY 1
#define ATTRIB_HIDDEN 2
#define ATTRIB_SYSTEM 4
#define ATTRIB_VOLUME 8
#define ATTRIB_DIRECTORY 16
#define ATTRIB_ARCHIVE 32

#define MSC_KRL_TMADDR 7
#define MSR_KRL_TMADDT 135
#define MSC_SHL_PTHADD 69
#define MSR_SHL_PTHADD 197
#define MSC_SHL_CHRTST 70
#define MSR_SHL_CHRTST 198
#define MSC_SHL_CHRWTC 71
#define MSR_SHL_CHRWTC 199

void library(void) __naked {
__asm
SySystem_CallFunction::
        ld (_msg_buf+04),bc   ;copy registers into the message buffer
        ld (_msg_buf+06),de
        ld (_msg_buf+08),hl
        ld (_msg_buf+10),ix
        ld (_msg_buf+12),iy
        push af
        pop hl
        ld (_msg_buf+02),hl
        pop hl
        ld e,(hl)
        inc hl
        ld d,(hl)
        inc hl
        push hl
        ld (_msg_buf+00),de   ;module und funktion number
        ld a,e
        ld (SyCallN),a
        ld iy,#_msg_buf
        ld a,(_proc_id)
        .db 0xdd
        ld l,a
        ld a,#3
        .db 0xdd
        ld h,a
        rst #0x10                 ;send message
SyCall1:: rst #0x30
        ld iy,#_msg_buf
        ld a,(_proc_id)
        .db 0xdd
        ld l,a
        ld a,#3
        .db 0xdd
        ld h,a
        rst #0x18                 ;wait for answer
        .db 0xdd
        dec l
        jr nz,SyCall1
        ld a,(_msg_buf)
        sub #128
        ld e,a
        ld a,(SyCallN)
        cp e
        jr nz,SyCall1
        ld hl,(_msg_buf+02)   ;get registers out of the message buffer
        push hl
        pop af
        ld bc,(_msg_buf+04)
        ld de,(_msg_buf+06)
        ld hl,(_msg_buf+08)
        ld ix,(_msg_buf+10)
        ld iy,(_msg_buf+12)
        ret
SyCallN:: .db 0

SyFile_FILOPN::
        call SySystem_CallFunction
        .db 26
        .db 18
        ret

SyFile_FILCLO::
        call SySystem_CallFunction
        .db 26
        .db 19
        ret

SyFile_FILINP::
        call SySystem_CallFunction
        .db 26
        .db 20
        ret
__endasm;
}

// SymShell Directory_Input call (not provided by SDK)
unsigned char Directory_Input(unsigned char path_bank, char* path_addr, unsigned char attributes, unsigned char dest_bank, char* dest_addr, unsigned short dest_length, unsigned short skip_entries, unsigned short* count_var) {
    reg_a = dest_bank;
    reg_de = (unsigned short)dest_addr;
    reg_bc = dest_length;
    reg_hl = (unsigned short)path_addr;
    reg_iy = skip_entries;
    reg_ix = ((unsigned short)path_bank << 8) | attributes;
    reg_gen = (unsigned short)count_var; // because SySystem_CallFunction will clobber the local
    __asm
        ld a, (_reg_a)
        ld de, (_reg_de)
        ld bc, (_reg_bc)
        ld hl, (_reg_hl)
        ld iy, (_reg_iy)
        ld ix, (_reg_ix)
        call SySystem_CallFunction
        .db 26
        .db 38
        ld (_reg_hl), hl
    __endasm;
    *(unsigned short*)reg_gen = reg_hl;
    return msg_buf[2] & 0x01; // carry bit = error
}

// SymShell Directory_DeleteFile call (not provided by SDK)
unsigned char Directory_DeleteFile(unsigned char path_bank, char* path_addr) {
    reg_hl = (unsigned short)path_addr;
    reg_ix = ((unsigned short)path_bank << 8);
    __asm
        ld hl, (_reg_hl)
        ld ix, (_reg_ix)
        call SySystem_CallFunction
        .db 26
        .db 39
    __endasm;
    return msg_buf[2] & 0x01; // carry bit = error
}

// SymShell Directory_Rename call (not provided by SDK)
unsigned char Directory_RenameFile(unsigned char path_bank, char* path_addr, char* new_name_addr) {
    reg_de = (unsigned short)new_name_addr;
    reg_hl = (unsigned short)path_addr;
    reg_ix = ((unsigned short)path_bank << 8);
    __asm
        ld de, (_reg_de)
        ld hl, (_reg_hl)
        ld ix, (_reg_ix)
        call SySystem_CallFunction
        .db 26
        .db 36
    __endasm;
    return msg_buf[2] & 0x01; // carry bit = error
}

// SymShell Shell_StringInput call (broken in SDK, so we redefine it here)
int Shell_StringInput(unsigned char shellPID, unsigned char channel, unsigned char bank, char *str) __sdcccall(0) {
    msg_buf[0] = 0;
    while (msg_buf[0] != MSR_SHL_STRINP) {
        msg_buf[0] = MSC_SHL_STRINP;
        msg_buf[1] = channel;
        msg_buf[2] = bank;
        *(unsigned short*)(msg_buf + 3) = (unsigned short)&str[0];
        Message_Send(symbos_info.processID, shellPID, msg_buf);
        while (!Message_Sleep_And_Receive(symbos_info.processID, shellPID, msg_buf));
    }
    return msg_buf[1];
}

// SymShell PathAdd call (not provided by SDK)
void Shell_PathAdd(unsigned char shellPID, unsigned char bank, char *basepath, char *component, char* str) {
    msg_buf[0] = 0;
    while (msg_buf[0] != MSR_SHL_PTHADD) {
        msg_buf[0] = MSC_SHL_PTHADD;
        *(unsigned short*)(msg_buf + 1) = (unsigned short)&basepath[0];
        *(unsigned short*)(msg_buf + 3) = (unsigned short)&component[0];
        *(unsigned short*)(msg_buf + 5) = (unsigned short)&str[0];
        msg_buf[7] = bank;
        Message_Send(symbos_info.processID, shellPID, msg_buf);
        while (!Message_Sleep_And_Receive(symbos_info.processID, shellPID, msg_buf));
    }
}

// SymShell CharTest call (not provided by SDK)
unsigned char Shell_CharTest(unsigned char shellPID, unsigned char channel, unsigned char lookahead) {
    msg_buf[0] = 0;
    while (msg_buf[0] != MSR_SHL_CHRTST) {
        msg_buf[0] = MSC_SHL_CHRTST;
        msg_buf[1] = channel;
        msg_buf[2] = lookahead;
        Message_Send(symbos_info.processID, shellPID, msg_buf);
        while (!Message_Sleep_And_Receive(symbos_info.processID, shellPID, msg_buf));
    }
    if (msg_buf[1] == 2)
        return msg_buf[2];
    else
        return 0;
}

// SymShell CharWatch call (not provided by SDK)
unsigned char Shell_CharWatch(unsigned char shellPID, unsigned char mode, unsigned char bank, unsigned short addr) {
    msg_buf[0] = 0;
    while (msg_buf[0] != MSR_SHL_CHRWTC) {
        msg_buf[0] = MSC_SHL_CHRWTC;
        msg_buf[1] = mode;
        msg_buf[2] = bank;
        *(unsigned short*)(msg_buf + 3) = addr;
        Message_Send(symbos_info.processID, shellPID, msg_buf);
        while (!Message_Sleep_And_Receive(symbos_info.processID, shellPID, msg_buf));
    }
    return msg_buf[3];
}
