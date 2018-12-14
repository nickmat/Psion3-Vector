;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;* Project: GRAPHIC LIBRARY FOR THE PSION 3a *  Written by: Nick Matthews  *
;*  Module: PROCESS MOVING POINT FUNCTIONS   *  Date Started:  2 Jan 1997  *
;*    File: PROCESS.ASM     Type: ASSEMBLER  *  Date Revised:  2 Jan 1997  *
;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
; Copyright (c) 1997 Nick Matthews
;
; Written for Microsoft MASM V6.11
; Assemble using ML /c /Cp
;

        .MODEL small
        INCLUDE kcmacros.inc
        INCLUDE vector.inc

OutsideAA   equ  -300

; Offsets for data values
ProcessOp   equ  0
OffsetX     equ  2
OffsetY     equ  4
CentreX     equ  6
CentreY     equ  8
CosM        equ 10
SinM        equ 12
Quot        equ 14
ScaleM      equ 16
ScaleQ      equ 18
StrRectL    equ 20
StrRectT    equ 22
StrRectR    equ 24
StrRectB    equ 26

; ProcessOp values
PONone      equ  0
POMove      equ  1
PORotate    equ  2
POScale     equ  3
POMirror    equ  4
POStretch   equ  5

        .CODE

_p_leave    PROTO NEAR


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  AddOffset
;;
;;  Purpose:
;;      Add a signed offset to an unsigned absolute unit.
;;      If the addition causes an overflow, call p_leave( OUTSIDE_AA );
;;
;;  Input:
;;      AX Unsigned absolute unit, DX signed offset
;;
;;  Output:
;;      AX Sum of absoute unit and offset
;;      If an overflow occurs p_leave is called and the function will not
;;      return.
;;
;;  Uses:
;;      Changes AX & DX
;;

AddOffset PROC NEAR
        TST  dx
        jns  AO_POS

        ; Negative offset
        neg  dx
        sub  ax, dx          ; Subtraction of two unsigned
        jnc  AO_RET
        jmp  AO_OUT

        ; Positive offset
AO_POS: add  ax, dx          ; Addition of two unsigned
        jnc  AO_RET

AO_OUT: ; Overflow occured
        mov  ax, OutsideAA
        call _p_leave        ; We don't return from this call

AO_RET: ret

AddOffset ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  void kcAddPt( A_PT* dest, A_PT* source, INT ax, INT ay );
;;
;;  Purpose:
;;      Add ax,ay to the source point and return in dest point.
;;      If the addition causes an overflow, call p_leave( OUTSIDE_AA );
;
;;  Input:
;;      Pointer to source point, offsets ax and ay
;;
;;  Output:
;;      Sum of source and offsets at address pointed to by dest pointer
;;      If an overflow occurs p_leave is called and the function will not
;;      return.
;;
;;  Uses:
;;      AX, CX, DX, SI & DI


; Function parameter offsets from BP
PDest   equ    4
PSrce   equ    6
AddX    equ    8
AddY    equ   10

_kcAddPt PROC  NEAR

        push bp
        mov  bp, sp
        push di
        push si

        ; Load registers
        mov  di, [bp+PDest]
        mov  si, [bp+PSrce]

        ; add ax
        mov  ax, [si+PtX]
        mov  dx, [bp+AddX]
        call AddOffset
        mov  [di+PtX], ax

        ; add ay
        mov  ax, [si+PtY]
        mov  dx, [bp+AddY]
        call AddOffset
        mov  [di+PtY], ax

        ; Restore registers and stack
        pop  si
        pop  di
        mov  sp, bp
        pop  bp
        ret                  ; Return from function

_kcAddPt ENDP

        END

; End of PROCESS.ASM file
