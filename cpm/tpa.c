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

#include "tpa.h"

void asm_constants(void) __naked {
__asm
    tpa_end     .equ #0xfd00         ;stack = end of TPA+1 = start of wrapper code (must be >#c000)
    wrap_len    .equ #0x0200

    jmp_memget  .equ #0x8118 ;MEMGET
    jmp_memfre  .equ #0x811B ;MEMFRE
    jmp_bnkcop  .equ #0x8130 ;BNKCOP
    jmp_bnkcll  .equ #0xff03 ;BNKCLL
    jmp_bnkret  .equ #0xff00 ;BNKRET

    jmp_tpabds  .equ tpa_end+00    ;bdos call (=bdos begin)
    jmp_tparet  .equ tpa_end+02    ;return from app
    jmp_tpairq  .equ tpa_end+04    ;irq
    jmp_tpaapp  .equ tpa_end+06    ;start app
    jmp_tpabot  .equ tpa_end+256+3 ;warm boot/bios call start
__endasm;
}

unsigned char low_beg(void) __naked {
__asm
    jp jmp_tpabot    ;low_boot
    .db 0            ;low_io
    .db 0            ;low_dsk
    jp jmp_tpabds    ;low_bdos
    .ds 6*8          ;low_rst
    jp jmp_tpairq
    .ds 5
    .ds 16           ;low_work
    .ds 12           ;low_free
    .ds 36           ;low_fcb
    .db 0            ;low_cmdlen
    .ds 127          ;low_cmdtxt
__endasm;
}

unsigned char tpa_init(void) __naked {
__asm
;### MEMINI -> reserves and inits TPA memory
;### Output     A=1 memory full
    xor a
    ld e,a
    ld bc,#0xffff-0x400-0x100+1
    rst #0x20                      ;reserve TPA bank (full 64K bank)
    .dw jmp_memget
    jp c, tpa_init_fail
    ld hl,#_BDOS
    ld (_wrap_beg+17),hl
    ld hl,#_symbos_stackBuffer+512
    ld (_wrap_beg+19),hl
    ld hl,#_bnk_tpa
    ld (hl),a
    ld a,(_bnk_vm)
    ld (_wrap_beg+16),a
    add a
    add a
    add a
    add a
    add (hl)
    ld (_bnk_tpa2vm),a
    rlca
    rlca
    rlca
    rlca
    ld (_bnk_vm2tpa),a
    ld hl,#_wrap_beg                ;copy wrapper code into top segment
    ld de,#tpa_end
    ld bc,#wrap_len
    push af
    rst #0x20
    .dw jmp_bnkcop
    pop af
    ld hl,#_low_beg                 ;copy vectors/buffers/bytes into low storage
    ld de,#0x000
    ld bc,#0x100
    rst #0x20
    .dw jmp_bnkcop
    ld a,#0
    or a
    ret
tpa_init_fail::
    ld a,#1
    ret
__endasm;
}

unsigned char tpa_free(void) __naked {
__asm
;### MEMFRE -> restores and frees TPA memory
    ld a,(_bnk_vm2tpa)
    ld hl,#0x0000
    ld e,l
    ld d,h
    ld bc,#0x400
    rst #0x20                       ;restore kernel code at #0000-#03ff
    .dw jmp_bnkcop
    ld a,(_bnk_tpa)
    ld hl,#0x400
    ld bc,#0xffff-0x400-0x100
    rst #0x20                       ;free TPA memory
    .dw jmp_memfre
    ret
__endasm;
}

// Assumes filename (as full path) will be stored in buffer2.
#pragma save
#pragma disable_warning 85
unsigned char app_load(unsigned short addr) __naked {
__asm
        ld (apploda),hl ; passed as HL
        ld ix,(_bnk_vm-1)
        ld hl,#_buffer2
        call SyFile_FILOPN
        jr c,applod2
        push af
        ld de,(_bnk_tpa)
        ld hl,(apploda)
        ld bc,#tpa_end-0x100-0x100
        call SyFile_FILINP
        pop bc
        jr c,applod1
        ld a,b
        call SyFile_FILCLO
        ;...maybe check header (if symshell app)
        or a
        ld a,#0
        ret
applod1:: push af                 ;error -> close file
        ld a,b
        call SyFile_FILCLO
        pop af
applod2:: ;...                    ;A=file error
        ld a,#1
        ret
apploda:: .dw 0
__endasm;
}
#pragma restore

void app_run(void) __naked {
__asm
    ld hl,#0x0080
    ld (_dma),hl
    ld a,(_bnk_tpa)
    ld b,a
    ld ix,#jmp_tpaapp
    ld iy,#tpa_end
    di              ;save VM-side shadow registers
__endasm;
__asm__ ("ex af,af'"); // because the __asm parser doesn't like apostrophes (derp)
__asm
    ld (vmshda+1),a
__endasm;
__asm__ ("ex af,af'");
__asm
    exx
    ld (vmshdb+1),bc
    ld (vmshdd+1),de
    ld (vmshdh+1),hl
    exx
    jp jmp_bnkcll
__endasm;
}

