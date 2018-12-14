;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;* Project: GRAPHIC LIBRARY FOR THE PSION 3a *  Written by: Nick Matthews  *
;*  Module: VARIOUS TRIGONOMETRY FUNCTIONS   *  Date Started: 29 Dec 1996  *
;*    File: TRIG.ASM        Type: ASSEMBLER  *  Date Revised: 29 Dec 1996  *
;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
; Copyright (c) 1996 Nick Matthews
;
; Written for Microsoft MASM V6.11
; Assemble using ML /c /Cp
;

        .MODEL small
        INCLUDE kcmacros.inc
        .CODE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  void kcTrig2A(
;;      int px, int py, int* cos2q, int* sin2q, unsigned* divisor );
;;
;;  Purpose:
;;      Calculate the sin and cos for twice the angle give by the slope
;;      px,py. Values are approx 16 bit integer multipliers and a 16 bit
;;      unsigned divisor.
;;
;;  Input:
;;      Two signed integers px, py. Pointers to address for the results.
;;
;;  Output:
;;      *cos2q = px * px - py * py;
;;      *sin2q = 2 * px * py;
;;      *divisor = px * px + py * py;
;;      Values all divided by a power of 2 as neccessay to fit 16 bits.
;;
;;  Uses:
;;      AX, BX, CX, DX and DI.
;;


; Function parameter offsets from BP
PX      equ    4
PY      equ    6
PCosQ   equ    8
PSinQ   equ   10
PDivr   equ   12
; Local variables
CosQL   equ   -2
CosQH   equ   -4
SinQL   equ   -6
SinQH   equ   -8
DivrL   equ  -10
DivrH   equ  -12

T2AVar  equ   12     ; Space required for local variables

_kcTrig2A PROC  NEAR

        push bp
        mov  bp, sp
        sub  sp, T2AVar
        push di

        ; Do SinQ first
        mov  bx, [bp+PX]     ; bx = px
        mov  ax, [bp+PY]     ; ax = py
        imul bx              ; ax,dx = px*py
        shl  ax, 1
        rcl  dx, 1           ; dx,ax = 2*px*py
        mov  [bp+SinQH], dx
        mov  [bp+SinQL], ax  ; SinQ = 2*px*py

        ; Do CosQ next
        mov  ax, [bp+PY]     ; ax = py
        imul ax              ; dx,ax = py*py
        mov  di, dx
        mov  cx, ax          ; di,cx = py*py
        mov  ax, bx          ; ax = px
        imul bx              ; dx,ax = px*px
        sub  ax, cx
        sbb  dx, di          ; dx,ax = px*px-py*py
        mov  [bp+CosQH], dx
        mov  [bp+CosQL], ax  ; CosQ = px*px-py*py

        ; Now Divr. si,cx = py*py
        mov  ax, bx          ; ax = px
        mul  bx              ; dx,ax = px*px
        add  ax, cx
        adc  dx, di          ; dx,ax = px*px+py*py
        mov  [bp+DivrH], dx
        mov  [bp+DivrL], ax  ; CosQ = px*px+py*py

T_Test: ; Factorize until CosQ, SinQ are 16 bit ints
        ; and Quot is a 16 bit unsigned
        mov  cx, [bp+SinQH]
        mov  ax, [bp+SinQL]
        TST  ax
        jns  @F            ; Jump if positive
        inc  cx            ; Turn -1 into 0
@@:     TST  cx
        jnz  T_Div2

        mov  cx, [bp+CosQH]
        mov  ax, [bp+CosQL]
        TST  ax
        jns  @F            ; Jump if positive
        inc  cx            ; Turn -1 into 0
@@:     TST  cx
        jnz  T_Div2

        mov  cx, [bp+DivrH]
        TST  cx
        jnz  T_Div2

        ; Write the answers to the supplied addresses
        mov  di, [bp+PSinQ]
        mov  ax, [bp+SinQL]
        mov  [di], ax

        mov  di, [bp+PCosQ]
        mov  ax, [bp+CosQL]
        mov  [di], ax

        mov  di, [bp+PDivr]
        mov  ax, [bp+DivrL]
        mov  [di], ax

        ; Restore registers and stack
        pop di
        mov sp, bp
        pop bp
        ret                  ; Return from function

T_Div2: ; Divide all values by 2, then go back retest
        sar  WORD PTR [bp+SinQH], 1
        rcr  WORD PTR [bp+SinQL], 1
        sar  WORD PTR [bp+CosQH], 1
        rcr  WORD PTR [bp+CosQL], 1
        shr  WORD PTR [bp+DivrH], 1
        rcr  WORD PTR [bp+DivrL], 1
        jmp  T_Test          ; Try again

_kcTrig2A ENDP

        END

; End of HYPOT.ASM file
