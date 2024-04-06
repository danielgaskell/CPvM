;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
;@                                                                            @
;@                      C P \/ M   (CP/M virtual machine)                     @
;@                              TPA WRAPPER CODE                              @
;@                                                                            @
;@                     (c) 2024 by Prevtenet & Prodatron                      @
;@                                                                            @
;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

tpa_end     equ #fc00       ;stack = end of TPA+1 = start of wrapper code (must be >#c000)
wrap_len    equ #0300
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
        ds 32
tpairq  db 0
        ld (tpairq1+1),sp   ; temp stack for IRQ calls
        ld sp,tpairq
        push af
        push bc
        push de
        push hl
        push ix
        push iy
        db #3e:ret
        ld (tpairq),a
        ld l,255
        call tpabds4
        di
        xor a
        ld (tpairq),a
        ld hl,CONST+1
        inc (hl)        ; increment "IRQs since last refresh"
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
tpabds  ld (tpabds1+1),sp   ;normal entry point - sets up/destroys temp stack
        ld sp,tpastk
        call tpabds2
tpabds1 ld sp,0
        ret

;handle buffered commands
tpabds2 ld a,c
        cp C_WRITE
        jr z,C_WRITE_buf
        cp C_RAWIO
        jr z,C_RAWIO_tpa
        cp C_STAT
        jr z,CONST
tpabds3 ld a,(cwblen)
        or a
        call nz,cwbsnd

;direct-call entry point (assumes temp stack has already been set up)
        ld l,c          ;C,DE -> L,DE=bdos call registers
tpabds4 ld a,(vm_bnk)
        ld b,a
        ld ix,(vm_adr)
        ld iy,(vm_stk)
        di              ;save TPA-side shadow registers
        exx
        ex af,af'
        push af
        push bc
        push de
        push hl
        ex af,af'
        exx
        call jmp_bnkcll
        ld a,l          ;copy result in HL to BA
        ld b,h
        di              ;restore TPA-side shadow registers
        exx
        ex af,af'
        pop hl
        pop de
        pop bc
        pop af
        ex af,af'
        exx
        ei
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
        call tpabds4
        pop hl
        pop de
        pop bc
        ret

C_RAWIO_tpa
        ld a,e
        inc a
        jr nz,C_WRITE_buf ;E != 0xFF, treat as C_WRITE
CONST   ld a,0
        cp #4
        ld a,#0
        jr nc,tparawc     ;>=4 IRQs since last call = normal BDOS call
        ld b,#0           ;...else respond with "no char waiting"
        ld hl,#0
        jp tpabds1
tparawc ld (CONST+1),a    ;reset IRQ count
        jr tpabds3
;Technically, C_STAT should repeat the last status rather than "no char", but
;since apps almost always use this for single-step decision-making it would be
;surprising if this causes problems.
        
;temp stack for BDOS calls (separate from IRQ since that may interrupt BDOS)
        ds 32
tpastk


;==============================================================================
;### BIOS CALLS ###############################################################
;==============================================================================

;must start on a page boundary
align 256,0

;vector table
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
tpabio_04   ld e,c:ld c,C_WRITE:  jp tpabds         ;CONOUT
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
wrplen  equ $-tpa_end   ;#300 max!
nolist

ds wrap_len-wrplen
nolist
