;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;* Project: GRAPHIC LIBRARY FOR THE PSION 3a *  Written by: Nick Matthews  *
;*  Module: STANDARD MACROS FOR MS MASM      *  Date Started: 20 Dec 1996  *
;*    File: KCMACROS.INC    Type: MASM INC   *  Date Revised:  5 Jan 1997  *
;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
; Copyright (c) 1996, 1997 Nick Matthews
;
; Written for Microsoft MASM V6.11
;

; Useful macros

; CLR - Clear a register
CLR MACRO    reg
      xor  reg, reg
    ENDM

; TST - Set flags to match reg
TST MACRO    reg
      or   reg, reg
    ENDM

; ABS - Absolute value of reg
ABS MACRO    reg
      or   reg, reg
      jns  @F
      neg  reg
@@:
    ENDM

; DIF - Absolute difference between dreg and sreg
DIF MACRO    dreg, sreg
      sub  dreg, sreg
      jns  @F
      neg  dreg
@@:
    ENDM

; SET - Set a register and flags to a value
SET MACRO    reg, val
      mov  reg, val
      or   reg, reg
    ENDM

; NEGD - Negate a DWord (32 bit)
NEGD MACRO   reg1, reg2
      neg  reg1
      neg  reg2
      sbb  reg1, 0
     ENDM

; Mathematical constants

; ISQR2 - Inverse of SquareRoot of 2  (10000h/SqRoot(2))
; ie. x = y/SqRoot(2) and y is in cx
;       mov  ax, ISQR2
;       mul  cx
; answer is now in dx (ax can be ignored)
ISQR2   equ  46341

; End of KCMACROS.ASM file
