;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;* Project: GRAPHIC LIBRARY FOR THE PSION 3a *  Written by: Nick Matthews  *
;*  Module: STANDARD HEADER FILE FOR MS MASM *  Date Started: 21 Dec 1996  *
;*    File: VECTOR.INC      Type: MASM INC   *  Date Revised: 21 Dec 1996  *
;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
; Copyright (c) 1996 Nick Matthews
;
; Written for Microsoft MASM V6.11
;

; Magic Statics
DatATFlag             = 20h

; System Services
SegManager            = 080h
  NmSegSize           = 005h
GenDataSegment        = 08Fh
ProcPanic             = 090h

; Constants
ROWLENGTH             = 512/8
COLLENGTH             = 160
BITMAPSIZE            = 512/8 * 160
XMAX                  = 480
YMAX                  = COLLENGTH
DRAW_MODE_SET         = 0
DRAW_MODE_CLR         = 1
DRAW_MODE_INV         = 2
DRAW_MODE_TST         = 3

; Local variables used by draw and plot routines
; Local variables offsets from BP
Mode    equ   -2   ; 02 Drawing Mode             E P
ClipL   equ   -4   ; 04 Left clip boundary       E C
ClipT   equ   -6   ; 06 Top clip boundary        E C
ClipR   equ   -8   ; 08 Right clip boundary      E C
ClipB   equ  -10   ; 0A Bottom clip boundary     E C
EndX    equ  -12   ; 0C Value of last x pixel (to end loop) PT
CoX1    equ  -14   ; 0E x1
CoXC    equ  CoX1  ; 0E xc Center x
CoY1    equ  -16   ; 10 y1
CoYC    equ  CoY1  ; 10 yc Centre y
CoX2    equ  -18   ; 12 x2
Rad     equ  CoX2  ; 12 Circle radius
CoY2    equ  -20   ; 14 y2
Steep   equ  -22   ; 16 Set if steep slope (reverse x,y)
ArcF    equ  Steep ; 16 Draw Arc if set
YSign   equ  -24   ; 18 Set to +1 or -1 depending on pos or neg slope
OnY0    equ  -26   ; 1A Value to add to p when increase in y is zero
OnY1    equ  -28   ; 1C Value to add to p when increase in y is +/- 1
BegX    equ  -30   ; 1E Starting x coord (maybe outside clip box)
BegY    equ  -32   ; 20 Starting y coord
EndY    equ  -34   ; 22 Value of last y row
TestF   equ  -36   ; 24 Flag set if DRAW_MODE_TST mode writes to bitmap
PlotP   equ  -38   ; 26 Ploting Proceedure to use  E

DrawVar equ   38   ; Total common local variable space required



; Panic numbers
PanicNumWriteOutsideBitmap = 230
PanicNumBitmapSizeWrong    = 229

; Structure Offsets
; P_RECT structure (Don't forget we are using normal cartisian coordinates)
RectTLX     equ  0      ; Left-most
RectTLY     equ  2      ; Bottom-most
RectBRX     equ  4      ; Right-most
RectBRY     equ  6      ; Top-most
; Alternative P_RECT structure using normal cartisian coordinates
RectL       equ  0      ; Left-most
RectB       equ  2      ; Bottom-most
RectR       equ  4      ; Right-most
RectT       equ  6      ; Top-most

; Point structure (either A_PT (AUNIT) or P_POINT (INT) )
PtX         equ  0
PtY         equ  2


; LINE or array of 2 P_POINT's or array of 4 INT's
LineX1      equ  0
LineY1      equ  2
LineX2      equ  4
LineY2      equ  6

; Circle data array
CentX       equ  0
CentY       equ  2
CRad        equ  4

; Arc data struct (a D_2AARC struct, see CalcArc.c)
CentXL      equ  0
CentXH      equ  2
CentYL      equ  4
CentYH      equ  6
CRadL       equ  8
CRadH       equ 10
ABegYL      equ 12
ABegYH      equ 14
AEndYL      equ 16
AEndYH      equ 18
ABegO       equ 20
AEndO       equ 21

; DATTR struct
BitmapAO    equ  0
ModeAO      equ  2
;WidthAO     equ  4
;DashAO      equ  6
;ColorAO     equ  8

; Prototypes
kcDrawLine    PROTO NEAR ;C  x1:WORD, y1:WORD, x2:WORD, y2:WORD
kcClearBitmap PROTO NEAR ;C bitmap:WORD

;        .DATA

;        EXTERN  _hBitmap:SWORD
;        EXTERN  _Mode:SWORD

        .CODE

Plot     PROTO NEAR
TestPlot PROTO NEAR
PanicBitmapSizeWrong PROTO NEAR
PanicWriteOutsideBitmap PROTO NEAR

@CurSeg ENDS        ; End current

; End of VECTOR.ASM file
