;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;* Project: GRAPHIC LIBRARY FOR THE PSION 3a *  Written by: Nick Matthews  *
;*  Module: BITMAP CIRCLE DRAWING PRIMATIVES *  Date Started:  3 Jan 1997  *
;*    File: CIRCLE.ASM      Type: ASSEMBLER  *  Date Revised: 31 Jul 1997  *
;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
; Copyright (c) 1997 Nick Matthews
;
; These routines are design to write to a bitmap which is then copied to the
; screen. They make the following assumptions:-
;    A handle of a valid bitmap is supplied, created as follows:-
;
;        W_OPEN_BIT_SEG bitseg;
;        bitseg.size.x = 512; /* Do not change without checking Plot */
;        bitseg.size.y = 160;
;        BitmapID = gCreateBit( WS_BIT_SEG_ACCESS, &bitseg );
;        BlackBM = p_sgopen( bitseg.seg_name );
;
;    Mode can be DRAW_MODE_SET, DRAW_MODE_CLR, DRAW_MODE_INV or DRAW_MODE_TST
;
; Written for Microsoft MASM V6.11
; Assemble using ML /c /Cp
;

        .MODEL small
        INCLUDE  kcmacros.inc
        INCLUDE  vector.inc

; Constants
VSMALLCIRCLE equ 10  ; Very small circles may as well be clip plotted

; Local variables used by circle routines
; (as well as DrawVar's in vector.inc)
Oc_xmr  equ  -2  - DrawVar ; Octant boxes boundaries
Oc_xmt  equ  -4  - DrawVar
Oc_xc   equ  -6  - DrawVar
Oc_xpt  equ  -8  - DrawVar
Oc_xpr  equ  -10 - DrawVar
Oc_ymr  equ  -12 - DrawVar
Oc_ymt  equ  -14 - DrawVar
Oc_yc   equ  -16 - DrawVar
Oc_ypt  equ  -18 - DrawVar
Oc_ypr  equ  -20 - DrawVar

PlotF1  equ  -22 - DrawVar ; Plot functions for each octant
PlotF2  equ  -24 - DrawVar
PlotF3  equ  -26 - DrawVar
PlotF4  equ  -28 - DrawVar
PlotF5  equ  -30 - DrawVar
PlotF6  equ  -32 - DrawVar
PlotF7  equ  -34 - DrawVar
PlotF8  equ  -36 - DrawVar

RDSR2   equ  -38 - DrawVar ; Radius/SqRoot(2)
; Arc start & stop values
BegOc   equ  -40 - DrawVar ; Arc beginning octant
EndOc   equ  -42 - DrawVar ; Arc ending octant
BegXP   equ  -44 - DrawVar ; Arc beg Plot xo value
EndXP   equ  -46 - DrawVar ; Arc end Plot xo value

CirVar  equ   46 + DrawVar

        .CODE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  RectCompClip
;;
;;  Purpose:
;;      Compare a given rect with the Clip Box and determine if inside,
;;      surrounding, overlapping or outside.
;;      NOTE: If Bottom & Left edges coincides with ClipB & ClipL these
;;      will be considered inside. If Top and Right edges coincide with
;;      ClipT and ClipR they are considered outside.
;;
;;  Input:
;;      BX = Top, DX = Left, SI = Bottom, DI = Right
;;      Clip box [bp+ClipL], [bp+ClipR], [bp+ClipT] & [bp+ClipB]
;;
;;  Output:
;;      AX = -1  Rect is Inside clip box
;;      AX =  0  Rect is Outside clip box
;;      AX =  1  Rect is Surrounding clip box
;;      AX =  2  Rect is Overlapping clip box
;;      Flags are set to match AX return value
;;
;;  Uses:
;;      AX for return value, all others unchanged

RectCompClip PROC  NEAR

        ; Test for inside, all 4 tests must pass
        cmp  si, [bp+ClipB]       ; B >= ClipB
        jl   RC1
        cmp  bx, [bp+ClipT]       ; T < ClipT
        jge  RC1
        cmp  dx, [bp+ClipL]       ; L >= ClipL
        jl   RC1
        cmp  di, [bp+ClipR]       ; R < ClipR
        jge   RC1
        ; Must be inside (or the same size)
        mov  ax, -1
        jmp  RC_RET

RC1:    ; Test if surrounds, all 4 tests must pass
        cmp  si, [bp+ClipB]       ; B < ClipB
        jge  RC2
        cmp  bx, [bp+ClipT]       ; T >= ClipT
        jl   RC2
        cmp  dx, [bp+ClipL]       ; L < ClipL
        jge  RC2
        cmp  di, [bp+ClipR]       ; R >= ClipR
        jl   RC2
        ; Must surround clip box (at least one edge outside)
        mov  ax, 1
        jmp  RC_RET

RC2:    ; Test edges for overlapping, each edge must pass
        cmp  bx, [bp+ClipB]       ; 1. T >= ClipB
        jl   RC_OUT
        cmp  si, [bp+ClipT]       ; 2. B < ClipT
        jge  RC_OUT
        cmp  di, [bp+ClipL]       ; 3. R >= ClipL
        jl   RC_OUT
        cmp  dx, [bp+ClipR]       ; 4. L < ClipR
        jge  RC_OUT
RC_OVR: ; Must be overlapping
        mov  ax, 2
        jmp  RC_RET

RC_OUT: ; Must be outside
        CLR  ax

RC_RET: TST  ax                   ; Set flags
        ret

RectCompClip ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  ClipCPlot
;;
;;  Purpose:
;;      Add an offset and plot a point only if inside the clip box.
;;
;;  Input:
;;      BX = x, AX = y
;;      [bp+CoXC], [bp+CoYC], Constant offset
;;      Clip box [bp+ClipL], [bp+ClipR], [bp+ClipT] & [bp+ClipB]
;;      ES = Start of bitmap, write protection off
;;
;;  Output:
;;      Bitmap has circle set, cleared, inverted or tested
;;
;;  Uses:
;;      AX, BX, CX, DX & DI
;;      Subroutine Plot (See DrawLine.asm)

ClipCPlot PROC  NEAR

        ; Test y (Testing y first is more efficient)
        add  ax, [bp+CoYC]
        cmp  ax, [bp+ClipB]
        jl   CP_RET
        cmp  ax, [bp+ClipT]
        jge  CP_RET

        ; Test x
        add  bx, [bp+CoXC]
        cmp  bx, [bp+ClipL]
        jl   CP_RET
        cmp  bx, [bp+ClipR]
        jge  CP_RET

        call WORD PTR [bp+PlotP]  ; Passed all the tests
CP_RET: ret

ClipCPlot ENDP

;CPlot PROC  NEAR
;
;        add  ax, [bp+CoYC]
;        add  bx, [bp+CoXC]
;        call WORD PTR [bp+PlotP]
;        ret
;
;CPlot ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  ArcCPlot
;;
;;  Purpose:
;;      Determine if octant is the beggining or end of the arc and plot
;;      accordingly.
;;
;;  Input:
;;      BX = x, AX = y, DX = Octant number, SI = x offset from centre
;;      [bp+CoXC], [bp+CoYC], Constant offset
;;      Clip box [bp+ClipL], [bp+ClipR], [bp+ClipT] & [bp+ClipB]
;;      ES = Start of bitmap, write protection off
;;
;;  Output:
;;      Bitmap has circle set, cleared, inverted or tested
;;
;;  Uses:
;;      AX, BX, CX, DX & SI
;;      Subroutine ClipCPlot

ArcCPlot PROC  NEAR

        ; Check if before Beg
        cmp  dl, [bp+BegOc]
        jne  AP20              ; Arc doesn't beg here
        mov  dh, dl
        and  dh, 1
        jnz  AP15
        ; Check if full arc
        cmp  dl, [bp+EndOc]
        jne  AP10
        mov  cx, [bp+BegXP]
        cmp  cx, [bp+EndXP]
        jle  AP10
        ; Full arc
        cmp  si, [bp+BegXP]
        jge AP30
        cmp  si, [bp+EndXP]
        jle  AP30
        jmp  AP_RET
AP10:   ; even:  quit if x < BegXP
        cmp  si, [bp+BegXP]
        jl   AP_RET
        jmp  AP20
AP15:   ; Check if full arc
        cmp  dl, [bp+EndOc]
        jne  AP18
        mov  cx, [bp+BegXP]
        cmp  cx, [bp+EndXP]
        jge  AP18
        ; Full arc
        cmp  si, [bp+BegXP]
        jle  AP30
        cmp  si, [bp+EndXP]
        jge  AP30
        jmp  AP_RET
AP18:   ; odd:  quit if x > BegXP
        cmp  si, [bp+BegXP]
        jg   AP_RET
AP20:   ; Check if passed End
        cmp  dl, [bp+EndOc]
        jne  AP30
        mov  dh, dl
        and  dh, 1
        jnz  AP25
        ; quit if x > EndXP
        cmp  si, [bp+EndXP]
        jg   AP_RET
        jmp  AP30
AP25:   ; quit if x < EndXP
        cmp  si, [bp+EndXP]
        jl   AP_RET
AP30:   call ClipCPlot
AP_RET: ret

ArcCPlot ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  CirclePoints
;;
;;  Purpose:
;;      Plot a point in each octant of the circle
;;
;;  Input:
;;      SI = xo, DI = yo, Offsets from centre point
;;      [bp+CoXC], [bp+CoYC], Circle centre point
;;      Clip box [bp+ClipL], [bp+ClipR], [bp+ClipT] & [bp+ClipB]
;;      ES = Start of bitmap, write protection off
;;
;;  Output:
;;      Bitmap has circle set, cleared, inverted or tested
;;
;;  Uses:
;;      AX, BX, CX
;;      Subroutine PlotCircle

CirclePoints PROC  NEAR
        push dx
        CLR  dx
        ; Octant 0    y,  x
        mov  bx, di          ; bx = yo
        mov  ax, si          ; ax = xo
        call WORD PTR [bp+PlotF1]
        ; Octant 1    x,  y
        inc  dx
        mov  bx, si          ; bx = xo
        mov  ax, di          ; ax = yo
        call WORD PTR [bp+PlotF2]
        ; Octant 2   -x,  y
        inc  dx
        mov  bx, si
        neg  bx
        mov  ax, di
        call WORD PTR [bp+PlotF3]
        ; Octant 3   -y,  x
        inc  dx
        mov  bx, di
        neg  bx
        mov  ax, si
        call WORD PTR [bp+PlotF4]
        ; Octant 4   -y, -x
        inc  dx
        mov  bx, di
        neg  bx
        mov  ax, si
        neg  ax
        call WORD PTR [bp+PlotF5]
        ; Octant 5   -x, -y
        inc  dx
        mov  bx, si
        neg  bx
        mov  ax, di
        neg  ax
        call WORD PTR [bp+PlotF6]
        ; Octant 6    x, -y
        inc  dx
        mov  bx, si
        mov  ax, di
        neg  ax
        call WORD PTR [bp+PlotF7]
        ; Octant 7    y, -x
        inc  dx
        mov  bx, di
        mov  ax, si
        neg  ax
        call WORD PTR [bp+PlotF8]
        pop  dx
        ret

CirclePoints ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  DrawSmallCircle
;;
;;  Purpose:
;;      Draw a circle with an WORD radius
;;
;;  Input:
;;      Centre point [bp+CoXC], [bp+CoYC]. Radius [bp+Rad]
;;      Clip box [bp+ClipL], [bp+ClipR], [bp+ClipT] & [bp+ClipB]
;;      ES = Start of bitmap, write protection off
;;
;;  Output:
;;      Bitmap has circle set, cleared, inverted or tested
;;
;;  Uses:
;;      AX, BX, CX, DX & DI
;;      Subroutine PlotCircle

DrawSmallCircle PROC  NEAR

        ; Clip circle, starting with complete circle
        ; Start with: ax=xc, cx=yc, di=Rad
        ; Load bx=Top, dx=Left, si=Bottom, di=Right
        mov  si, cx          ; si = yc
        sub  si, di          ; si = yc-rad = ymr = Bottom
        mov  dx, ax          ; dx = xc
        sub  dx, di          ; dx = xc-rad = xmr = Left
        mov  bx, cx          ; bx = yc
        add  bx, di          ; bx = yc+rad = ypr = Top
        add  di, ax          ; di = xc+rad = xpr = Right
        call RectCompClip    ; Sets flags
        jz   SC_RET          ; Circle is outside Clip, we can quit now
        js   SC25 ; 20            ; Circle is all inside so use Plot

        ; Circle Surrounds or overlaps clip box
        ; Save the outside dim
        mov  [bp+Oc_ypr], bx ; Top
        mov  [bp+Oc_xmr], dx ; Left
        mov  [bp+Oc_ymr], si ; Bottom
        mov  [bp+Oc_xpr], di ; Right
        mov  di, [bp+Rad]
        cmp  di, VSMALLCIRCLE
        jb   SC25            ; Very small circles can use ClipPlot

        ; Calc t = Rad/SqRt(2)
        push ax              ; Save return from RectCompClip
        mov  ax, ISQR2
        mul  di              ; dx = rad/SqRoot(2) = t
        TST  ax
        jns  @F
        inc  dx              ; Round up
@@:     mov  [bp+RDSR2], dx

        ; Circle limits surround clip box, so check inner rect
        mov  di, dx          ; di = t
        mov  ax, [bp+CoXC]  ; ax = xc
        mov  cx, [bp+CoYC]  ; cx = yc
        ; Start with: ax=xc, cx=yc, di=Rad/SqRt(2)
        ; Load bx=Top, dx=Left, si=Bottom, di=Right
        mov  si, cx          ; si = yc
        sub  si, di          ; si = yc-t = ymt = Bottom
        mov  dx, ax          ; dx = xc
        sub  dx, di          ; dx = xc-t = xmt = Left
        mov  bx, cx          ; bx = yc
        add  bx, di          ; bx = yc+t = ypt = Top
        add  di, ax          ; di = xc+t = xpt = Right
        pop  ax
        cmp  ax, 1           ; cx contains return from RectCompClip
        jne  SC10            ; Circle definately overlaps
        call RectCompClip    ; Sets flags
        cmp  ax, 1
        jz   SC_RET          ; Clip box is completely inside circle

SC10:   ; Circle is overlapped so determine plot function for each octant
        ; Save the inside dim
        mov  [bp+Oc_ypt], bx ; Top
        mov  [bp+Oc_xmt], dx ; Left
        mov  [bp+Oc_ymt], si ; Bottom
        mov  [bp+Oc_xpt], di ; Right
        ; Check each Octant
        ; Octant 0
        mov  dx, [bp+Oc_xpt]
        mov  bx, [bp+Oc_ypt]
        mov  di, [bp+Oc_xpr]
        mov  si, [bp+CoYC]
        mov  cx, 0
        call LoadOctantPlot
        ; Octant 1
        mov  dx, [bp+CoXC]
        mov  bx, [bp+Oc_ypr]
        mov  di, [bp+Oc_xpt]
        mov  si, [bp+Oc_ypt]
        mov  cx, -2
        call LoadOctantPlot
        ; Octant 2
        mov  dx, [bp+Oc_xmt]
        mov  bx, [bp+Oc_ypr]
        mov  di, [bp+CoXC]
        mov  si, [bp+Oc_ypt]
        mov  cx, -4
        call LoadOctantPlot
        ; Octant 3
        mov  dx, [bp+Oc_xmr]
        mov  bx, [bp+Oc_ypt]
        mov  di, [bp+Oc_xmt]
        mov  si, [bp+CoYC]
        mov  cx, -6
        call LoadOctantPlot
        ; Octant 4
        mov  dx, [bp+Oc_xmr]
        mov  bx, [bp+CoYC]
        mov  di, [bp+Oc_xmt]
        mov  si, [bp+Oc_ymt]
        mov  cx, -8
        call LoadOctantPlot
        ; Octant 5
        mov  dx, [bp+Oc_xmt]
        mov  bx, [bp+Oc_ymt]
        mov  di, [bp+CoXC]
        mov  si, [bp+Oc_ymr]
        mov  cx, -10
        call LoadOctantPlot
        ; Octant 6
        mov  dx, [bp+CoXC]
        mov  bx, [bp+Oc_ymt]
        mov  di, [bp+Oc_xpt]
        mov  si, [bp+Oc_ymr]
        mov  cx, -12
        call LoadOctantPlot
        ; Octant 7
        mov  dx, [bp+Oc_xpt]
        mov  bx, [bp+CoYC]
        mov  di, [bp+Oc_xpr]
        mov  si, [bp+Oc_ymt]
        mov  cx, -14
        call LoadOctantPlot
        jmp  SC40

;SC20:   ; Load all Octants with CPlot
;        mov  ax, CPlot
;        jmp  SC28

SC25:   ; Load all Octants with ClipPlot
        mov  ax, ClipCPlot

SC28:   CLR  di
        mov  cx, 8
SC30:   mov  [bp+PlotF1+di], ax
        sub  di, 2
        loop SC30

SC40:   ; Check for Arc
        CLR  ax
        cmp  ax, [bp+ArcF]
        jz   SC45
        call AdjustForArc

SC45:   ; Now plot the circle
        ; Make no assumptions about current regs
        CLR  si              ; si = x
        mov  di, [bp+Rad]    ; di=Rad = y
        mov  dx, di          ; dx=Rad
        neg  dx              ; dx=-Rad
        inc  dx              ; dx=1-Rad = d

SCLOOP: ; Main loop
        call CirclePoints    ; si,di = x,y dx=d All others free
        cmp  si, di
        jge  SC_RET

        TST  dx
        jns  SCL1
        ; d is neg
        mov  cx, si          ; cx = x
        shl  cx, 1           ; cx = 2*x
        add  cx, 3           ; cx = 2*x+3
        add  dx, cx          ; dx = d = d+2*x+3
        jmp  SCL2

SCL1:   ; d is pos
        mov  cx, si          ; cx = x
        sub  cx, di          ; cx = x-y
        shl  cx, 1           ; cx = 2*(x-y)
        add  cx, 5           ; cx = 2*(x-y)+5
        add  dx, cx          ; dx = d = d+2*(x-y)+5
        dec  di              ; di = y--

SCL2:   inc  si              ; si = x++
        jmp  SCLOOP

SC_RET: ret

DrawSmallCircle ENDP

LoadOctantPlot PROC NEAR

        ; Rect in bx, dx, si & di. Index in cx
        call RectCompClip
        mov  di, cx
        jnz  @F
        mov  [bp+PlotF1+di], DummyPlot  ; Return zero - Rect outside
        ret
@@:     ;js   @F
        mov  [bp+PlotF1+di], ClipCPlot  ; Return Pos - Rect overlaps
        ret
;@@:     mov  [bp+PlotF1+di], CPlot      ; Return Neg - Rect inside
;        ret

LoadOctantPlot ENDP

DummyPlot PROC NEAR
        ret
DummyPlot ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  INT CDECL kcDrawCircle( INT* circle, P_RECT* clip, DATTR* attr );
;;
;;  Purpose:
;;      Draw a circle clipped to given rect with the given attributes
;;
;;  Input:
;;      Pointer to centre coordinates and radius in an array of INTs
;;        in order:- xc, yc, rad
;;      Pointer to a normalised P_RECT struct to clip to
;;      Pointer to a DATTR struct. Uses members:-
;;        Bitmap - The bitmap handle to draw to
;;        Mode   - The Drawing Mode (DRAW_MODE_SET, _CLR, _INV)
;;        Other members ignored at present.
;;
;;  Output:
;;      Bitmap has line x1,y1 to x2,y2 set, cleared, inverted or tested
;;
;;  Uses:
;;      Everything, restores BP, SP, DI, SI & ES
;;      Calls
;;      Switches off write protection and (if DatATFlag was originally zero)
;;      restores it on exit.
;;

; Function parameter offsets from BP
Circle  equ    4
Clip    equ    6
Attr    equ    8
_kcDrawCircle PROC  NEAR

        push bp
        mov  bp, sp
        sub  sp, CirVar        ; Space for Circle local variables
        push di
        push si

IF DEBUG EQ 1
        ; Do a SegSize here to check the segment handle is valid.
        mov  si, [bp+Attr]     ; Get the attribute pointer
        mov  bx, [si+BitmapAO] ; bx = Bitmap handle
        mov  ah, NmSegSize     ; Function number
        int  SegManager
        cmp  ax, BITMAPSIZE/16 ; Expected bitmap size, in para
        jne  PanicBitmapSizeWrong ; Bad news
ENDIF
        int GenDataSegment     ; Get the o/s data space in ES
        mov  si, [bp+Attr]     ; Get the attribute pointer
        mov  bx, [si+BitmapAO] ; bx = Bitmap handle
        mov  es, es:[bx]       ; es = Bitmap segment

        push ss:[DatATFlag]
        mov  byte ptr ss:[DatATFlag], 0
        out  15h, al           ; Disable write protection

        ; Load local variables from Attr struct
        mov  ax, [si+ModeAO]   ; ax = Drawing mode
        mov  [bp+Mode], ax
        mov  [bp+PlotP], Plot  ; Plot to standardised & checked bitmap

        ; Setup clip box details. ClipT & ClipB are as for normal cartesian
        ; coordiates. This is the opposite to the Psion screen, but as long
        ; as we are consistant, it should all work!
        mov  si, [bp+Clip]     ; si = Clip box struct
        TST  si
        jz   DC1               ; NULL Clip box pointer

        ; Load the P_RECT struct as follows
        ;  ClipL = rect.tl.x
        ;  ClipB = rect.tl.y
        ;  ClipR = rect.br.x
        ;  ClipT = rect.br.y
        mov  ax, [si+RectTLX]  ; ax = ClipL
        mov  [bp+ClipL], ax
        mov  ax, [si+RectTLY]  ; ax = ClipB
        mov  [bp+ClipB], ax
        mov  ax, [si+RectBRX]  ; ax = ClipR
        mov  [bp+ClipR], ax
        mov  ax, [si+RectBRY]  ; ax = ClipT
        mov  [bp+ClipT], ax
        jmp  DC2               ; Continue

DC1:    ; NULL clip box pointer, so load default
        CLR  ax
        mov  [bp+ClipL], ax
        mov  [bp+ClipB], ax
        mov  ax, XMAX
        mov  [bp+ClipR], ax
        mov  ax, YMAX
        mov  [bp+ClipT], ax

DC2:    ; Load up local variables from data pointer
        CLR  ax
        mov  [bp+ArcF], ax      ; Clear ArcF flag
        mov  si, [bp+Circle]
        mov  ax, [si+CentX]  ; ax = xc
        mov  [bp+CoXC], ax
        mov  cx, [si+CentY]  ; cx = yc
        mov  [bp+CoYC], cx
        mov  di, [si+CRad]   ; di = Rad
        mov  [bp+Rad], di
        ; Now draw it
        call DrawSmallCircle      ; Draw a small circle line

DC_RET: ; Restore Write Protection
        pop  ss:[DatATFlag]
        cmp  byte ptr ss:[DatATFlag], 0
        jz   @F             ; Skip Enable write protection
        out  14h, al        ; Enable write protection
        ; Restore es
@@:     cli                 ; Disable interrupts
        push ds             ; Save copy of DS
        pop  es             ; Recover ES
        sti                 ; Re-enable interrupts
        ; Restore registers and stack
        pop si
        pop di
        mov sp, bp
        pop bp
        ret                 ; Return from function

_kcDrawCircle ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Subroutine: PlotCircleTest
;;
;;  Purpose:
;;      Set TestF to show that the bitmap would have been written to.
;;      Adjusts [bp+EndX] to force caller to return.
;;
;;  Input:
;;      AX = y coordinate, not used
;;      BX = x coordinate, not used
;;
;;  Output:
;;      [bp+TestF] is set to 1
;;
;;  Uses:
;;      Changes AX & SI
;;

PlotCircleTest PROC NEAR

        ; Don't update screen, just mark as hit and set x = y
        ; to stop the caller from continuing.
        mov  ax, 1
        mov  [bp+TestF], ax  ; Set Marker
        mov  si, di          ; Make sure caller returns
        ret

PlotCircleTest ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  INT CDECL kcIsCircleInBox( INT* circle, P_RECT* rect, DATTR* attr );
;;
;;  Purpose:
;;      Test if part of a circle is in a rectangle
;;
;;  Input:
;;      Pointer to centre coordinates and radius in an array of INTs
;;        in order:- xc, yc, radius
;;      Pointer to a normalised P_RECT struct
;;      Pointer to a DATTR struct. Entrys ignored at present
;;
;;  Output:
;;      TRUE if a pixel would have been plotted, otherwise FALSE
;;
;;  Uses:
;;      Everything, restores BP, SP, DI, SI
;;      Calls DrawCircle (with [Mode] = DRAW_MODE_TST)
;;

_kcIsCircleInRect PROC  NEAR

        push bp
        mov  bp, sp
        sub  sp, CirVar        ; Space for Circle local variables
        push di
        push si

        ; Setup clip box details. ClipT & ClipB are as for normal cartesian
        ; coordiates. This is the opposite to the Psion screen, but as long
        ; as we are consistant, it should all work! Load the P_RECT struct
        ; as follows:-
        ;  ClipL = rect.tl.x
        ;  ClipB = rect.tl.y
        ;  ClipR = rect.br.x
        ;  ClipT = rect.br.y
        mov  si, [bp+Clip]     ; si = Clip box struct
        mov  ax, [si+RectTLX]  ; ax = ClipL
        mov  [bp+ClipL], ax
        mov  ax, [si+RectTLY]  ; ax = ClipB
        mov  [bp+ClipB], ax
        mov  ax, [si+RectBRX]  ; ax = ClipR
        mov  [bp+ClipR], ax
        mov  ax, [si+RectBRY]  ; ax = ClipT
        mov  [bp+ClipT], ax

        ; Set default values
        CLR  ax                ; Set TestF to FALSE
        mov  [bp+TestF], ax
        mov  [bp+ArcF], ax      ; Clear ArcF flag

        mov  [bp+PlotP], PlotCircleTest ; Test only, don't plot
        ; Load up local variables from data pointer
        mov  si, [bp+Circle]
        mov  ax, [si+CentX]  ; ax = xc
        mov  [bp+CoXC], ax
        mov  cx, [si+CentY]  ; cx = yc
        mov  [bp+CoYC], cx
        mov  di, [si+CRad]   ; di = Rad
        mov  [bp+Rad], di
        call DrawSmallCircle   ; pretend draw a circle
        ; Leave return flag in ax
        mov  ax, [bp+TestF]

IB_RET: ; Restore registers and stack
        pop  si
        pop  di
        mov  sp, bp
        pop  bp
        ret                    ; Return from function

_kcIsCircleInRect ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Subroutine: AdjustForArc
;;
;;  Purpose:
;;      Set up Start and Stop octants and remove unplotted octants
;;
;;  Input:
;;      [bp+PlotF?] subroutines set to ClipCPlot or DummyPlot
;;
;;  Output:
;;      [bp+PlotF?] is set to ClipCPlot, DummyPlot or ArcCPlot
;;
;;  Uses:
;;      AX, BX, CX, DI. Updates [bp+PlotF?] table

AdjustForArc PROC NEAR

        ; Set arc beg plot function
        mov  di, [bp+BegOc]
        mov  bx, di
        mov  cx, di             ; sets cl=BegOc, ch=0 (mark)
        shl  di, 1
        neg  di

        mov  ax, [bp+PlotF1+di]
        cmp  ax, DummyPlot
        je   @F                 ; Don't need to bother with beg plot
        mov  [bp+PlotF1+di], ArcCPlot

        ; Check if beg and end in same octant
@@:     cmp  bl, [bp+EndOc]
        jne  AALOOP
        mov  ax, [bp+BegXP]

        ; Check if it is nearly a circle
        mov  bh, bl
        and  bh, 01h            ; Just look at the last bit
        jz   @F

        ; if beg < end then near full circle
        cmp  ax, [bp+EndXP]
        jl   AA_RET
        jmp  AALOOP

@@:     ; if beg > end then near full circle
        cmp  ax, [bp+EndXP]
        jg   AA_RET

AALOOP: cmp  cl, [bp+EndOc]
        jne  AAL1
        ; Set arc end plot function
        mov  ax, [bp+PlotF1+di]
        cmp  ax, DummyPlot
        je   @F
        mov  [bp+PlotF1+di], ArcCPlot
@@:     mov  ch, 1

AAL1:   sub  di, 2
        inc  cl
        cmp  cl, 8
        jl   @F
        CLR  di
        CLR  cl
@@:     cmp  cl, bl   ; Are we back to the beginning
        je   AA_RET
        TST  ch
        jz   AALOOP
        mov  [bp+PlotF1+di], DummyPlot
        jmp  AAL1  ; Don't need to check for end again

AA_RET: ret

AdjustForArc ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  INT CDECL kcDraw2AngArc( D_2AARC* arc, P_RECT* clip, DATTR* attr );
;;
;;  Purpose:
;;      Draw an arc clipped to given rect with the given attributes
;;
;;  Input:
;;      Pointer to long centre coordinates, long radius and angles
;;        in order:- xc, yc, rad
;;      Pointer to a normalised P_RECT struct to clip to
;;      Pointer to a DATTR struct. Uses members:-
;;        Bitmap - The bitmap handle to draw to
;;        Mode   - The Drawing Mode (DRAW_MODE_SET, _CLR, _INV)
;;        Other members ignored at present.
;;
;;  Output:
;;      Bitmap has arc set, cleared, inverted or tested
;;
;;  Uses:
;;      Everything, restores BP, SP, DI, SI & ES
;;      Calls
;;      Switches off write protection and (if DatATFlag was originally zero)
;;      restores it on exit.
;;

; Function parameter offsets from BP
Arc     equ    4
Clip    equ    6
Attr    equ    8

_kcDraw2AngArc PROC  NEAR

        push bp
        mov  bp, sp
        sub  sp, CirVar        ; Space for Circle local variables
        push di
        push si

IF DEBUG EQ 1
        ; Do a SegSize here to check the segment handle is valid.
        mov  si, [bp+Attr]     ; Get the attribute pointer
        mov  bx, [si+BitmapAO] ; bx = Bitmap handle
        mov  ah, NmSegSize     ; Function number
        int  SegManager
        cmp  ax, BITMAPSIZE/16 ; Expected bitmap size, in para
        jne  PanicBitmapSizeWrong ; Bad news
ENDIF
        int GenDataSegment     ; Get the o/s data space in ES
        mov  si, [bp+Attr]     ; Get the attribute pointer
        mov  bx, [si+BitmapAO] ; bx = Bitmap handle
        mov  es, es:[bx]       ; es = Bitmap segment

        push ss:[DatATFlag]
        mov  byte ptr ss:[DatATFlag], 0
        out  15h, al           ; Disable write protection

        ; Load local variables from Attr struct
        mov  ax, [si+ModeAO]   ; ax = Drawing mode
        mov  [bp+Mode], ax
        mov  [bp+PlotP], Plot  ; Standard Plot

        ; Setup clip box details. ClipT & ClipB are as for normal cartesian
        ; coordiates. This is the opposite to the Psion screen, but as long
        ; as we are consistant, it should all work!
        mov  si, [bp+Clip]     ; si = Clip box struct
        TST  si
        jz   DC1               ; NULL Clip box pointer

        ; Load the P_RECT struct as follows
        ;  ClipL = rect.tl.x
        ;  ClipB = rect.tl.y
        ;  ClipR = rect.br.x
        ;  ClipT = rect.br.y
        mov  ax, [si+RectTLX]  ; ax = ClipL
        mov  [bp+ClipL], ax
        mov  ax, [si+RectTLY]  ; ax = ClipB
        mov  [bp+ClipB], ax
        mov  ax, [si+RectBRX]  ; ax = ClipR
        mov  [bp+ClipR], ax
        mov  ax, [si+RectBRY]  ; ax = ClipT
        mov  [bp+ClipT], ax
        jmp  DC2               ; Continue

DC1:    ; NULL clip box pointer, so load default
        CLR  ax
        mov  [bp+ClipL], ax
        mov  [bp+ClipB], ax
        mov  ax, XMAX
        mov  [bp+ClipR], ax
        mov  ax, YMAX
        mov  [bp+ClipT], ax

DC2:    ; Load up local variables from data pointer
        mov  ax, 1
        mov  [bp+ArcF], ax   ; Set ArcF flag
        mov  si, [bp+Arc]
        mov  ax, [si+ABegYL]
        mov  [bp+BegXP], ax
        mov  ax, [si+AEndYL]
        mov  [bp+EndXP], ax
        CLR  ah
        mov  al, [si+ABegO]
        mov  [bp+BegOc], ax
        mov  al, [si+AEndO]
        mov  [bp+EndOc], ax
        mov  ax, [si+CentXL] ; ax = xc
        mov  [bp+CoXC], ax
        mov  cx, [si+CentYL] ; cx = yc
        mov  [bp+CoYC], cx
        mov  di, [si+CRadL]  ; di = Rad
        mov  [bp+Rad], di
        ; Now draw it
        call DrawSmallCircle      ; Draw a small circle line

DC_RET: ; Restore Write Protection
        pop  ss:[DatATFlag]
        cmp  byte ptr ss:[DatATFlag], 0
        jz   @F             ; Skip Enable write protection
        out  14h, al        ; Enable write protection
        ; Restore es
@@:     cli                 ; Disable interrupts
        push ds             ; Save copy of DS
        pop  es             ; Recover ES
        sti                 ; Re-enable interrupts
        ; Restore registers and stack
        pop si
        pop di
        mov sp, bp
        pop bp
        ret                 ; Return from function

_kcDraw2AngArc ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  INT CDECL kcIs2ArgArcInBox( VOID* arc, P_RECT* rect, DATTR* attr );
;;
;;  Purpose:
;;      Test if part of an arc is in a rectangle
;;
;;  Input:
;;      Pointer to long centre coordinates, long radius and angles
;;        in order:- xc, yc, rad
;;      Pointer to a normalised P_RECT struct
;;      Pointer to a DATTR struct. Entrys ignored at present
;;
;;  Output:
;;      TRUE if a pixel would have been plotted, otherwise FALSE
;;
;;  Uses:
;;      Everything, restores BP, SP, DI, SI
;;      Calls DrawCircle (with [Mode] = DRAW_MODE_TST)
;;

_kcIs2AngArcInRect PROC  NEAR

        push bp
        mov  bp, sp
        sub  sp, CirVar        ; Space for Circle local variables
        push di
        push si

        ; Setup clip box details. ClipT & ClipB are as for normal cartesian
        ; coordiates. This is the opposite to the Psion screen, but as long
        ; as we are consistant, it should all work! Load the P_RECT struct
        ; as follows:-
        ;  ClipL = rect.tl.x
        ;  ClipB = rect.tl.y
        ;  ClipR = rect.br.x
        ;  ClipT = rect.br.y
        mov  si, [bp+Clip]     ; si = Clip box struct
        mov  ax, [si+RectTLX]  ; ax = ClipL
        mov  [bp+ClipL], ax
        mov  ax, [si+RectTLY]  ; ax = ClipB
        mov  [bp+ClipB], ax
        mov  ax, [si+RectBRX]  ; ax = ClipR
        mov  [bp+ClipR], ax
        mov  ax, [si+RectBRY]  ; ax = ClipT
        mov  [bp+ClipT], ax

        ; Set default values
        CLR  ax                ; Set TestF to FALSE
        mov  [bp+TestF], ax
        mov  [bp+ArcF], ax      ; Clear ArcF flag

        mov  [bp+PlotP], PlotCircleTest ; Test only, don't plot
        ; Load up local variables from data pointer
        mov  ax, 1
        mov  [bp+ArcF], ax   ; Set ArcF flag
        mov  si, [bp+Arc]
        mov  ax, [si+ABegYL]
        mov  [bp+BegXP], ax
        mov  ax, [si+AEndYL]
        mov  [bp+EndXP], ax
        CLR  ah
        mov  al, [si+ABegO]
        mov  [bp+BegOc], ax
        mov  al, [si+AEndO]
        mov  [bp+EndOc], ax
        mov  ax, [si+CentXL] ; ax = xc
        mov  [bp+CoXC], ax
        mov  cx, [si+CentYL] ; cx = yc
        mov  [bp+CoYC], cx
        mov  di, [si+CRadL]  ; di = Rad
        mov  [bp+Rad], di
        ; Now draw it
        call DrawSmallCircle   ; pretend draw a circle
        ; Leave return flag in ax
        mov  ax, [bp+TestF]

IB_RET: ; Restore registers and stack
        pop  si
        pop  di
        mov  sp, bp
        pop  bp
        ret                    ; Return from function

_kcIs2AngArcInRect ENDP

        END

; End of CIRCLE.ASM file


