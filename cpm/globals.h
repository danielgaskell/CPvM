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

// NOTE: make sure this stays set to a blank 16-byte part of the TPA, including buffers
#define DPBADDR 0xFEF0
#define TPA_END 0xFD00

typedef struct {
    unsigned char drive;
    unsigned char filename[8];
    unsigned char ext[3];
    unsigned char extent;
    unsigned char handle; // occupies S1
    unsigned char module;
    unsigned char record_count;
    unsigned char dirinfo[16];
    unsigned char current_record;
    unsigned short random_record;
    unsigned char random_record_overflow;
} FCB;

typedef struct {
    unsigned char user;
    unsigned char filename[8];
    unsigned char ext[3];
    unsigned char extent;
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned char record_count;
    unsigned char dirinfo[16];
} DIR;

// general buffers
char buffer[257];
char buffer2[256];
char buffer3[256];
char buffer4[256];
char dir_buffer[64*22];
char drive_paths[4][64];

// general globals (e.g., register transfer)
unsigned char reg_a;
unsigned short reg_hl;
unsigned short reg_bc;
unsigned short reg_de;
unsigned short reg_iy;
unsigned short reg_ix;
unsigned short reg_gen;
unsigned char gen_a;
char* gen_ptr;

// terminal/system
unsigned char termpid;
unsigned char termwidth;
unsigned char termheight;
unsigned char termver;
unsigned char proc_id;
unsigned char bnk_vm;
unsigned char bnk_tpa;
unsigned char bnk_tpa2vm; // tpa to cpmvm (low=tpa, high=cpmvm)
unsigned char bnk_vm2tpa; // cpmvm to tpa (low=cpmvm, high=tpa)
unsigned char tpa_error;
unsigned char keyconv[4];
unsigned char inverse;
unsigned char adm_attribs[2];
unsigned char adm_foreground;
unsigned char cursor_on;
unsigned char termresp;
char* out_ptr;
char* out_buffer;

// BDOS status
unsigned short drive_ro;
unsigned char default_drive;
unsigned short login_vector;
unsigned char user_number;
unsigned short dma;

// BDOS filesystem
FCB fcb_buffer;
DIR dir_entry;
char* dir_buffer_ptr;
char dir_buffer_on;
unsigned short dir_buffer_count;
unsigned short dir_buffer_count_open;
unsigned char handles_used[8];
unsigned short handles_pos[8];
unsigned short handles_len[8];
unsigned short handles_fcb[8]; // used to help close read-only files automatically

// CCP status
unsigned char launch_ccp;
char* ccp_parameters;

// escape codes
char* control_codes;
unsigned char escape_chars_expected;
unsigned char escape_char_bracket;
unsigned char escape_char_position;
unsigned char escape_char_y;
unsigned char escape_attrib_mode;
char ansi_parms[8];
unsigned char ansi_parm1;
unsigned char ansi_parm2;
void escapes(void) __naked {
__asm
    .dw esc_null 	; ESC (, handled separately
    .dw esc_null 	; ESC ), handled separately
    .dw esc_apos 	; ESC *
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_1
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_A
    .dw esc_B
    .dw esc_C
    .dw esc_D
    .dw esc_E
    .dw esc_null
    .dw esc_null
    .dw esc_H
    .dw esc_I
    .dw esc_J
    .dw esc_K
    .dw esc_L
    .dw esc_M
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_Q
    .dw esc_R
    .dw esc_null
    .dw esc_T
    .dw esc_null
    .dw esc_null
    .dw esc_W
    .dw esc_null
    .dw esc_null	; ESC Y, handled separately
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null	; ESC b, foreground color, handled separately
    .dw esc_null	; ESC c, background color, handled separately
    .dw esc_d
    .dw esc_e
    .dw esc_f
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_j
    .dw esc_k
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_o
    .dw esc_null	; ESC p, handled separately
    .dw esc_null	; ESC q, handled separately
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null
    .dw esc_null	; ESC v [wrap on]
    .dw esc_null	; ESC w [wrap off]
    .dw esc_y

    esc_null::	.db 0
    esc_apos::	.db 0x0C
                .db 0
    esc_1::		.db 0x11
                .db 0x12
                .db 0
    esc_A::		.db 0x0B
                .db 0
    esc_B::		.db 0x0A
                .db 0
    esc_C::		.db 0x09
                .db 0
    esc_D::		.db 0x08
                .db 0
    esc_E::		.db 0x0C
                .db 0
    esc_H::		.db 0x1E
                .db 0
    esc_I::		.db 0x0B
                .db 0
    esc_J::		.db 0x12
                .db 0x0A
                .db 0x14
                .db 0x0B
                .db 0
    esc_K::		.db 0x12
                .db 0
    esc_L::		.db 0x0A
                .db 0x0D
                .db 0
    esc_M::		.db 0x0D
                .db 0x12
                .db 0x1D
                .db 0x01
                .db 0
    esc_Q::		.db 0x20
                .db 0
    esc_R::		.db 0x11
                .db 0x12
                .db 0
    esc_T::		.db 0x12
                .db 0
    esc_W::		.db 0x08
                .db 0x20
                .db 0x08
                .db 0
    esc_d::		.db 0x17
                .db 0x0B
                .db 0x13
                .db 0x0A
                .db 0
    esc_e::		.db 0x03
                .db 0
    esc_f::		.db 0x02
                .db 0
    esc_j::		.db 0x04
                .db 0
    esc_k::		.db 0x05
                .db 0
    esc_o::		.db 0x11
                .db 0
    esc_y::		.db 0x12
                .db 0x0A
                .db 0x14
                .db 0x0B
                .db 0
__endasm;
}

void generic_dpb(void) __naked {
__asm
    .dw 64      ; records_per_track
    .db 5       ; block_shift
    .db 0x1F    ; block_mask
    .db 1       ; extent_mask
    .dw 2047    ; highest_block
    .dw 1023    ; highest_entry
    .db 255     ; alloc_bitmap_1
    .db 0       ; alloc_bitmap_2
    .dw 0       ; checksum_vector
    .dw 2       ; reserved_offset
__endasm;
}

// TRANSFER AREA
#include "..\\sdk\\symbos_transfer_area_start.h"
char msg_buf[14];
#include "..\\sdk\\symbos_transfer_area_end.h"
