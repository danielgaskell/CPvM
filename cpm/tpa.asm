;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
;@                                                                            @
;@                      C P \/ M   (CP/M virtual machine)                     @
;@                              TPA WRAPPER CODE                              @
;@                                                                            @
;@                     (c) 2024 by Prevtenet & Prodatron                      @
;@                                                                            @
;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

tpa_end     equ #fd00       ;stack = end of TPA+1 = start of wrapper code (must be >#c000)
wrap_len    equ #0200
jmp_bnkcll  equ #ff03
C_READ      equ 01
C_WRITE     equ 02
A_READ      equ 03
A_WRITE     equ 04
L_WRITE     equ 05
C_RAWIO     equ 06
C_STAT      equ 11
F_DMAOFF    equ 26
B_READ      equ 252

org tpa_end
WRITE "tpa.bin"

;==============================================================================
;### JUMP INS #################################################################
;==============================================================================

;BDOS and VM related calls
jr tpabds       ;BDOS call
jr tparet       ;application returns to CCP
jr tpairq       ;IRQ occurred
jr tpaapp       ;start application
ds 16-8

vm_bnk  db 0
vm_adr  dw 0
vm_stk  dw 0


;==============================================================================
;### BDOS AND VM RELATED ######################################################
;==============================================================================

;### TPAAPP -> starts loaded application
tpaapp  ld sp,tpa_end
        ld hl,tparet
        push hl
        jp #100

;### TPARET -> returns from a CP/M program to the CCP
tparet  ld c,254
        jr tpabds

;### TPAIRQ -> IRQ occured, do a hard interrupt to keep the multitasking running
        ds 32       ;temp stack, IRQ uses only 2 bytes of app stack
tpairq  db 0
        ld (tpairq1+1),sp
        ld sp,tpairq
        push af
        push bc
        push de
        push hl
        push ix
        push iy
        db #3e:ret
        ld (tpairq),a
        ld c,255
        call tpabds0
        di
        xor a
        ld (tpairq),a
        pop iy
        pop ix
        pop hl
        pop de
        pop bc
        pop af
tpairq1 ld sp,0
        ei
        ret

;### TPABDS -> BDOS function call
;### Input      C=function, DE=parameter
;### Output     AB,HL=return parameters
tpabds  ld a,c
        cp C_WRITE
        jr z,C_WRITE_buf
        cp C_RAWIO
        jr nz,tpabds2
        ld a,e
        inc a
        jr nz,C_WRITE_buf
tpabds2 ld a,(cwblen)
        or a
        call nz,cwbsnd

tpabds0 ld l,c          ;C,DE -> L,DE=bdos call registers
tpabds1 ld a,(vm_bnk)
        ld b,a
        ld ix,(vm_adr)
        ld iy,(vm_stk)
        call jmp_bnkcll
        ld a,l          ;copy result in HL to BA
        ld b,h
        ret

C_WRITE_buf
        ld hl,cwblen
        bit 7,(hl)
        call nz,cwbsnd
        ld c,(hl)
        inc (hl)
        ld b,0
        ld hl,cwbbuf
        add hl,bc
        ld (hl),e
        ret
cwbsnd  push bc         ;print and empty buffer
        push de
        push hl
        ld hl,cwblen
        ld c,(hl)
        ld b,0
        ld (hl),b
        ld hl,cwbbuf
        add hl,bc
        ld (hl),b
        ld l,253
        ld de,cwbbuf
        call tpabds1
        pop hl
        pop de
        pop bc
        ret

;==============================================================================
;### BIOS CALLS ###############################################################
;==============================================================================

;must start on a page boundary
align 256,0

;vector table
;modify bios_siz when changing the number of calls
jp tpabio_00    ;BOOT
jp tpabio_01    ;WBOOT
jp tpabio_02    ;CONST
jp tpabio_03    ;CONIN
jp tpabio_04    ;CONOUT
jp tpabio_05    ;LIST
jp tpabio_06    ;PUNCH
jp tpabio_07    ;READER
jp tpabio_08    ;HOME
jp tpabio_09    ;SELDSK
jp tpabio_10    ;SETTRK
jp tpabio_11    ;SETSEC
jp tpabio_12    ;SETDMA
jp tpabio_13    ;READ
jp tpabio_14    ;WRITE
jp tpabio_15    ;LISTST
jp tpabio_16    ;SECTRAN

;calls
tpabio_00
tpabio_01   ld c,0:               jp tpabds         ;BOOT/WBOOT
tpabio_02   ld c,C_STAT:          jp tpabds         ;CONST
tpabio_03   ld c,B_READ:          jp tpabds         ;CONIN
tpabio_04   ld e,c:               jp C_WRITE_buf    ;CONOUT                 (direct c_write_buf)
tpabio_05   ld e,c:ld c,L_WRITE:  jp tpabds         ;LIST
tpabio_06   ld e,c:ld c,A_WRITE:  jp tpabds         ;PUNCH
tpabio_07          ld c,A_READ:   jp tpabds         ;READER
tpabio_08   ret                                     ;HOME       *ignored*
tpabio_09   ret                                     ;SELDSK     *ignored*
tpabio_10   ret                                     ;SETTRK     *ignored*
tpabio_11   ret                                     ;SETSEC     *ignored*
tpabio_12   ld d,b:ld e,c:ld c,F_DMAOFF:jp tpabds   ;SETDMA
tpabio_13   ld a,1:ret                              ;READ       *illegal*   (returns A=1 -> error)
tpabio_14   ld a,1:ret                              ;WRITE      *illegal*   (returns A=1 -> error)
tpabio_15   ld a,#ff:ret                            ;LISTST                 (not yet implemented)
tpabio_16   ld l,c:ld h,b:ret                       ;SECTRAN    *ignored*   (returns HL=BC)

;C_WRITE buffer (here to ensure BIOS can start on a page boundary)
cwblen  db 0
cwbbuf  ds 128+1

list
wrplen  equ $-tpa_end   ;#200 max!
nolist

ds wrap_len-wrplen
nolist
