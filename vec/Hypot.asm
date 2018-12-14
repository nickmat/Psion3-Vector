;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;* Project: GRAPHIC LIBRARY FOR THE PSION 3a *  Written by: Nick Matthews  *
;*  Module: CALC SQUARE ROOT AND HYPOTENUSE  *  Date Started: 19 Dec 1996  *
;*    File: HYPOT.ASM       Type: ASSEMBLER  *  Date Revised: 19 Dec 1996  *
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
;;  SqRoot
;;
;;  Purpose:
;;      Find the square root of a DWORD
;;
;;  Input:
;;      DX,BX = number as DWORD
;;
;;  Output:
;;      AX = integer part (floor) of the square root
;;
;;  Uses:
;;      AX, BX, CX, DX, SI & DI


SqRoot  PROC  NEAR

        mov  cx, 16          ; Loop counter (16 = Bits per long / 2)
        CLR  ax              ; a accumulator
        CLR  si              ; r remainder
        CLR  di              ; e trial product

SQ1:    shl  bx, 1           ; shift 2 bits from number to remainder
        rcl  dx, 1
        rcl  si, 1
        shl  bx, 1
        rcl  dx, 1
        rcl  si, 1           ; r = (r << 2) + TOP2BITS(x); x <<= 2;

        shl  ax, 1           ; a <<= 1;
        mov  di, ax
        stc
        rcl  di, 1           ; e = (a << 1) + 1;
        cmp  si, di
        jb   SQ2             ; if( r >= e )
                             ; {
        sub  si, di          ;     r -= e;
        inc  ax              ;     a++;
                             ; }
SQ2:    loop SQ1

        ret                  ; Return from function

SqRoot  ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  unsigned int kcSquareRoot( unsigned long number );
;;
;;  Purpose:
;;      Find the square root of a unsigned long number.
;;
;;  Input:
;;      The unsigned long number
;;
;;  Output:
;;      Unsigned integer part of the square root of number in AX
;;
;;  Uses:
;;


; Function parameter offsets from BP
NumLo   equ    4             ; Lest significant word of number
NumHi   equ    6             ; Most significant word of number

_kcSquareRoot PROC  NEAR

        push bp
        mov  bp, sp
        push di
        push si

        mov  bx, [bp+NumLo]
        mov  dx, [bp+NumHi]  ; bx,dx=number
        call SqRoot          ; ax=SquareRoot(number)

        ; Restore registers and stack
        pop si
        pop di
        mov sp, bp
        pop bp
        ret                  ; Return from function

_kcSquareRoot ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  unsigned kcHypotenuse( int side1, int side2 );
;;
;;  Purpose:
;;      Find the square root of the sum of two squares.
;;
;;  Input:
;;      Two signed integers,
;;
;;  Output:
;;      Unsigned integer = Square root of ( side1 * side1 + side2 * side2 )
;;
;;  Uses:
;;


; Function parameter offsets from BP
Side1   equ    4             ; side1
Side2   equ    6             ; side2

_kcHypotenuse PROC  NEAR

        push bp
        mov  bp, sp
        push di
        push si

        mov  ax, [bp+Side1]  ; ax=side1
        ABS  ax              ; We don't need to deal with signed numbers
        mul  ax              ; ax,dx=side1*side1
        mov  bx, ax
        mov  cx, dx          ; bx,cx=side1*side1
        mov  ax, [bp+Side2]  ; ax=side2
        ABS  ax
        mul  ax              ; ax,dx=side2*side2
        add  bx, ax
        adc  dx, cx          ; bx,dx=side1*side1+side2*side2
        call SqRoot          ; ax=SquareRoot(side1*side1+side2*side2)

        ; Restore registers and stack
        pop si
        pop di
        mov sp, bp
        pop bp
        ret                  ; Return from function

_kcHypotenuse ENDP

        END

; End of HYPOT.ASM file
