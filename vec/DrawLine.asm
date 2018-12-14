;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;* Project: GRAPHIC LIBRARY FOR THE PSION 3a *  Written by: Nick Matthews  *
;*  Module: BITMAP DRAWING PRIMATIVES        *  Date Started: 24 Oct 1996  *
;*    File: GRAPHA.ASM      Type: ASSEMBLER  *  Date Revised:  8 Nov 1996  *
;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
; Copyright (c) 1996 Nick Matthews
;
; These routines are design to write to a bitmap which is then copied to the
; screen. They make the following assumptions:-
;    _hBitmap  contains the handle of a valid bitmap, created as follows:-
;
;        W_OPEN_BIT_SEG bitseg;
;        bitseg.size.x = 512; /* Do not change without checking Plot */
;        bitseg.size.y = 160;
;        BitmapID = gCreateBit( WS_BIT_SEG_ACCESS, &bitseg );
;        BlackBM = p_sgopen( bitseg.seg_name );
;
;    _Mode  contains DRAW_MODE_SET, DRAW_MODE_CLR or DRAW_MODE_INV
;
; Written for Microsoft MASM V6.11
; Assemble using ML /c /Cp
;

        .MODEL small
        INCLUDE  kcmacros.inc
        INCLUDE  vector.inc
        .CODE

PanicWriteOutsideBitmap PROC NEAR
        mov al, PanicNumWriteOutsideBitmap
        int ProcPanic
PanicWriteOutsideBitmap ENDP


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Subroutine: Plot
;;
;;  Purpose:
;;      Set, clea, invert or test (depending on Mode value) a bitmap pixel.
;;
;;  Input:
;;      AX = y coordinate
;;      BX = x coordinate
;;      ES = Bitmap segment
;;      [bp+Mode]   = Drawing mode
;;      BITMAPSIZE  = Size of bitmap segment
;;
;;  Output:
;;      Bitmap has x,y bit set, cleared, inverted or tested.
;;      BX = Byte offset into bitmap
;;      AL = Mask used (Inverted for DRAW_MODE_CLR)
;;      CX = Drawing mode
;;
;;  Uses:
;;      Changes AX, BX & CX
;;
;;  Errors:
;;      Panics if attempts to write outside bitmap segment

Plot    PROC NEAR

        ; Because the bitmap is 512 pixels wide (64 bytes) we can use
        ; shifts instead of a multiply
        mov  ah, al          ; get y in ah (y is 0 to 159)
        CLR  al              ; ax = y * 256
        shr  ax, 1           ; ax = y * 128
        shr  ax, 1           ; ax = y * 64
        mov  cx, bx          ; save x in cx
        shr  bx, 1           ; bx = x / 2
        shr  bx, 1           ; bx = x / 4
        shr  bx, 1           ; bx = x / 8
        add  bx, ax          ; bx now has index of byte to change

        and  cl, 007h        ; cl = x % 8
        mov  al, 1
        shl  al, cl          ; al now contains bit mask

        cmp  bx, BITMAPSIZE  ; Just in case (Write protection is off)
        jnb  PanicWriteOutsideBitmap

        mov  cx, [bp+Mode]   ; Get drawing mode
        cmp  cx, DRAW_MODE_SET
        jnz  P1
        or   es:[bx], al     ; Set pixel on
        jmp  P_RET

P1:     cmp  cx, DRAW_MODE_CLR
        jnz  P2
        not  al              ; Invert bit mask
        and  es:[bx], al     ; Clear pixel
        jmp  P_RET

P2:     cmp  cx, DRAW_MODE_INV
        jnz  P3
        xor  es:[bx], al     ; Invert pixel
        jmp  P_RET

        ; Don't update screen, just mark as hit and set EndX to x
        ; to stop the caller from continuing.
P3:     cmp  cx, DRAW_MODE_TST
        mov  ax, 1
        mov  [bp+TestF], ax  ; Set Marker
        mov  si, [bp+EndX]   ; Make sure caller returns

P_RET:  ret

Plot    ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Subroutine: PlotTest
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

PlotTest PROC NEAR

        ; Don't update screen, just mark as hit and set EndX to x
        ; to stop the caller from continuing.
        mov  ax, 1
        mov  [bp+TestF], ax  ; Set Marker
        mov  si, [bp+EndX]   ; Make sure caller returns

P_RET:  ret

PlotTest ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  DrawDiagLine
;;
;;  Purpose:
;;      Draw straight diagonal line on _BlackBM
;;
;;  Input:
;;      [bp+CoX1] = x1, [bp+CoY1] = y1, [bp+CoX2] = x2, [bp+CoY2] = y2
;;      [bp+ClipL], [bp+ClipT], [bp+ClipR], [bp+ClipB] = clip box
;;      ES = Start of bitmap, write protection off
;;
;;  Output:
;;      Bitmap has line x1,y1 to x2,y2 set, cleared or inverted.
;;
;;  Uses:
;;      AX, BX, CX, DX, DI & SI
;;      Subroutine Plot
;;
;;  Notes:
;;      Use px,py in place of dx,dy to avoid confusion with register names.

DrawDiagLine PROC  NEAR
        mov  si, [bp+Line]
        mov  ax, [si+LineX1] ; Get px = abs( x1 - x2 ) in ax
        sub  ax, [si+LineX2]
        jns  @F
        neg  ax
@@:     mov  bx, [si+LineY1] ; Get py = abs( y1 - y2 ) in bx
        sub  bx, [si+LineY2]
        jns  @F
        neg  bx
@@:
        ; Load Co?? variables, depending on steep slope
        cmp  ax, bx
        jng  DD2             ; Jump if steep slope

        ; Load up for shallow slope (slope <= 1)
        CLR  cx
        mov  [bp+Steep], cx  ; Clear Steep flag
        mov  cx, [si+LineX1] ; Load x1, y1
        mov  [bp+CoX1], cx
        mov  cx, [si+LineY1]
        mov  [bp+CoY1], cx
        mov  cx, [si+LineX2] ; Load x2, y2
        mov  [bp+CoX2], cx
        mov  cx, [si+LineY2]
        mov  [bp+CoY2], cx
        jmp  DD3

DD2:    ; Load up for steep slope (slope > 1)
        mov  cx, 1
        mov  [bp+Steep], cx  ; Set Steep flag
        xchg ax, bx          ; swap px,py
        mov  cx, [si+LineX1] ; Load in reverse x1, y1
        mov  [bp+CoY1], cx
        mov  cx, [si+LineY1]
        mov  [bp+CoX1], cx
        mov  cx, [si+LineX2] ; Load in reverse x2, y2
        mov  [bp+CoY2], cx
        mov  cx, [si+LineY2]
        mov  [bp+CoX2], cx
        mov  cx, [bp+ClipL]  ; Swap ClipL, ClipB
        xchg cx, [bp+ClipB]
        mov  [bp+ClipL], cx
        mov  cx, [bp+ClipR]  ; Swap ClipR, ClipT
        xchg cx, [bp+ClipT]
        mov  [bp+ClipR], cx

DD3:    mov  cx, 1
        mov  [bp+YSign], cx  ; Set y positive direction
        mov  cx, [bp+CoX1]
        cmp  cx, [bp+CoX2]
        jl   DD4             ; Jump if x1 < x2

        ; Load up for x1 high
        mov  si, [bp+CoX2]   ; Set si to x2
        mov  di, [bp+CoY2]   ; Set di to y2
        mov  cx, [bp+CoX1]
        mov  [bp+EndX], cx   ; Set EndX to x1
        mov  cx, [bp+CoY1]
        mov  [bp+EndY], cx   ; Set EndY to y1
        cmp  di, cx          ; Compare y2, y1
        jl   DD5
        mov  cx, -1
        mov  [bp+YSign], cx  ; Set y negative slope
        jmp  DD5

DD4:    ; Load up for x2 high
        mov  si, [bp+CoX1]   ; Set si to x1
        mov  di, [bp+CoY1]   ; Set di to y1
        mov  cx, [bp+CoX2]
        mov  [bp+EndX], cx   ; Set EndX to x2
        mov  cx, [bp+CoY2]
        mov  [bp+EndY], cx   ; Set EndY to y2
        cmp  di, cx          ; Compare y1, y2
        jl   DD5
        mov  cx, -1
        mov  [bp+YSign], cx  ; Set y negative slope

DD5:    ; save BegX, BegY
        mov  [bp+BegX], si   ; Save beg points
        mov  [bp+BegY], di

        ; Check if all to the right (if x1 >= ClipR)
        cmp  si, [bp+ClipR]
        jge  DD_RET          ; Nothing to do
        ; Check if all to the left (if x2 < ClipL)
        mov  cx, [bp+EndX]
        cmp  cx, [bp+ClipL]
        jl   DD_RET          ; Nothing to do

        ; calculate p start and incremental steps
        mov  cx, bx
        shl  cx, 1           ; cx = 2*py
        mov  [bp+OnY0], cx
        sub  cx, ax          ; cx = 2*py - px
        mov  dx, cx          ; dx = p
        sub  cx, ax          ; cx = 2*py - 2*px
        mov  [bp+OnY1], cx

        ; Check for starting before left edge of clip box
        cmp  si, [bp+ClipL]
        jge  DD6             ; Adjustment not required

        ; // Adjust for start, clip to Left edge
        ; temp = INT( ( 2*py * ( ClipL - BegX ) ) / px );
        ; y = BegY - (temp+1)/2;
        ; x = ClipL;
        ; si,di = x,y = BegX,BegY
        ; ax,bx = px,px
        ; dx = p
        ; cx = free
        mov  cx, ax           ; Save px in cx
        push dx               ; Save p
        mov  ax, [bp+ClipL]   ; ax = ClipL
        mov  si, ax           ; si = ClipL = x
        sub  ax, [bp+BegX]    ; ax = ClipL-BegX
        shl  ax, 1            ; ax = 2*(ClipL-BegX)
        imul bx               ; ax,dx = 2*py*(ClipL-BegX)
        idiv cx               ; ax = INT( 2*py*(ClipL-BegX)/px ) = temp
        inc  ax               ; ax = temp+1
        sar  ax, 1            ; ax = (temp+1)/2
        mov  dx, [bp+YSign]
        cmp  dx, 1
        je   @F
        neg  ax
@@:     add  di, ax           ; di = BegY+(temp+1)/2 = y
        pop  dx               ; restore dx to p
        mov  ax, cx           ; restore ax to px

DD6:    ; si,di = x,y    First pixel to be plotted
        ; ax,bx = px,py ; dx = p
        ; cx = free
        ; Check for starting before top/bottom of clip box
        ; Do positive slope first
        mov  cx, [bp+YSign]
        cmp  cx, 1
        jne  DD7

        ; Range check - Positive slope
        mov  cx, [bp+ClipT]
        cmp  cx, di           ; cmp ClipT, y
        jle  DD_RET           ; All above top

        mov  cx, [bp+ClipB]
        cmp  cx, [bp+EndY]    ; cmp ClipB, EndY
        jg   DD_RET           ; All below bottom

        ; Adjust EndY to clip end

        cmp  cx, di           ; cmp ClipB, y
        jle  DD9              ; No more clipping necessary

        mov  di, cx           ; Clip y to ClipB
        jmp  DD8              ; Start clipping

DD7:    ; Range check - Negative slope
        mov  cx, [bp+ClipB]
        cmp  cx, di           ; cmp ClipB, y
        jg   DD_RET           ; All below bottom

        mov  cx, [bp+ClipT]
        cmp  cx, [bp+EndY]    ; cmp ClipT, EndY
        jl   DD_RET           ; All above top

        cmp  cx, di           ; cmp ClipT, y
        jg  DD9              ; No more clipping necessary

        neg  bx               ; Use negative value for py
        mov  di, cx
        dec  di               ; Clip y to ClipT-1

DD8:    ; Clip x value - this works for both pos & neg slopes
        ; di = y (already clipped), si = x (to be changed)
        ; ax,bx = px,py  Not required again (bx = -py for neg slope)
        ; dx = p, cx = ClipB (ClipT for neg slopes)

        ; temp = ( px * ( 2*ClipB - 2*BegY - 1 ) ) / 2*py
        ; x = BegX + temp + 1;
        mov  si, dx           ; Save p in si
        xchg ax, cx           ; ax = ClipB, cx = px
        shl  ax, 1            ; ax = 2*ClipB
        mov  dx, [bp+BegY]    ; dx = BegY
        shl  dx, 1            ; dx = 2*BegY
        sub  ax, dx           ; ax = 2*ClipB-2*BegY
        dec  ax               ; ax = 2*ClipB-2*BegY-1
        imul cx               ; dx,ax = px*(2*ClipB-2*BegY-1)
        shl  bx, 1            ; bx = 2*py
        idiv bx               ; ax = px*(2*ClipB-2*BegY-1)/2*py = temp
        ;sar  ax, 1            ; ax = temp/2
        add  ax, [bp+BegX]    ; ax = BegX+temp
        inc  ax               ; ax = BegX + temp + 1 = x
        ; Check if still in clip box
        cmp  ax, [bp+ClipR]
        jge  DD_RET           ; Rest of line outside clip box

        mov  dx, si           ; Restore p to dx
        mov  si, ax           ; si = x


DD9:    ; Adjust p if x,y not at start of line
        ; si,di = x,y & dx = p
        ; ax, bx, cx = free
        cmp  si, [bp+BegX]
        je   DD10             ; Start has not been clipped

        ; temp = ABS( y - BegY );
        ; p = p + OnY0 * ( x - BegX - temp ) + OnY1 * temp
        ; Note, intermeadiate values may be longs
        push dx               ; save p
        mov  cx, di           ; cx = y
        DIF  cx, [bp+BegY]    ; cx = temp = abs(y1-BegY)
        mov  ax, [bp+OnY1]    ; ax = OnY1
        imul cx               ; dx,ax = OnY1*temp
        mov  bx, ax           ; dx,bx = OnY1*temp  (dx can be ignored)
        neg  cx               ; cx = -temp
        add  cx, si           ; cx = x-temp
        sub  cx, [bp+BegX]    ; cx = x-BegX-temp
        mov  ax, [bp+OnY0]    ; ax = OnY0
        imul cx               ; dx,ax = OnY0*(x-BegX-temp)
        add  ax, bx           ; ax = OnY0*(x-BegX-temp)+OnY1*temp
        pop  dx               ; dx = p
        add  dx, ax           ; dx = p + OnY0*(x-BegX-temp) + OnY1*temp

DD10:   ; Set EndX to clip at ClipR-1 if necessary
        mov  ax, [bp+ClipR]
        dec  ax
        cmp  ax, [bp+EndX]
        jg   DD11
        mov  [bp+EndX], ax

DD11:   ; Set EndY to clip end of line
        mov  ax, [bp+YSign]
        cmp  ax, 1
        jne  DD12

        ; Positive slope,  Clip EndY at ClipT-1
        mov  ax, [bp+ClipT]
        dec  ax
        jmp  DD13

DD12:   ; Negative slope, clip EndY at ClipB
        mov  ax, [bp+ClipB]
DD13:   mov  [bp+EndY], ax

DDLOOP: ; Main loop.  dx = p, si = x, di = y
        ; Plot the current x, y Pixel
        mov  cx, [bp+Steep]
        jcxz DDL6            ; Jump if Steep = 0

        mov  bx, di          ; Steep, so reverse x, y
        mov  ax, si
        call WORD PTR [bp+PlotP]
        jmp  DDL7

DDL6:   mov  bx, si          ; Load x, y
        mov  ax, di
        call WORD PTR [bp+PlotP]

DDL7:   cmp  si, [bp+EndX]   ; Have we done enough yet
        jnb  DD_RET          ; Jump out of loop (All done)

        inc  si
        cmp  dx, 0
        jge  DDL8

        add  dx, [bp+OnY0]
        jmp  DDLOOP

DDL8:   cmp  di, [bp+EndY]
        je   DD_RET          ; Reached Horizontal Edge
        add  di, [bp+YSign]
        add  dx, [bp+OnY1]
        jmp  DDLOOP

DD_RET: ret

DrawDiagLine ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  DrawVertLine
;;
;;  Purpose:
;;      Draw straight vertical line on _BlackBM (x1 == x2)
;;
;;  Input:
;;      [BP+4] = x1, [BP+6] = y1, [BP+8] = x2, [BP+10] = y2
;;      ES = Start of bitmap, write protection off
;;
;;  Output:
;;      Bitmap has line x1,y1 to x2,y2 set, cleared or inverted.
;;
;;  Uses:
;;      AX, BX, CX, DX & DI
;;      Subroutine Plot

DrawVertLine PROC  NEAR

        mov  si, [bp+Line]
        mov  bx, [si+LineX1]
        cmp  bx, [bp+ClipL]
        jl   DV_RET          ; Line to left of clip box
        cmp  bx, [bp+ClipR]
        jge  DV_RET          ; Line to right of clip box

        mov  ax, [si+LineY1]
        mov  dx, [si+LineY2]
        cmp  ax, dx
        jle  @F
        xchg ax, dx

@@:     ; bx = x, ax = y1 (bottom), dx = y2 (top)
        mov  cx, [bp+ClipT]
        cmp  ax, cx
        jge  DV_RET          ; Line all above clip box
        cmp  dx, cx          ; cmp y2, ClipT
        jl   @F
        mov  dx, cx          ; Trim off top of line
        dec  dx
@@:
        mov  cx, [bp+ClipB]
        cmp  dx, cx
        jl   DV_RET          ; Line all below clip box
        cmp  ax, cx          ; cmp y1, ClipB
        jge  @F
        mov  ax, cx          ; Trim off bottom of line

@@:     ; bx = x, ax = y (bottom), dx = EndY (top)
        mov  di, ax          ; Save beg y in di
        call WORD PTR [bp+PlotP] ; Plot the first pixel & set bx, cx & al

        ; We now have bx=bitmap offset, cx=mask, dx=EndY, di=y
DVLOOP:
        cmp dx, di
        jz  DV_RET           ; Finished, jump out of loop
        inc di
        add bx, ROWLENGTH    ; Add another row to offset

        cmp bx, BITMAPSIZE   ; Just in case (Write protection is off)
        jnb PanicWriteOutsideBitmap

        cmp cx, DRAW_MODE_SET
        jnz DV2
        or  es:[bx], al      ; Set pixel on
        jmp DVLOOP

DV2:    cmp cx, DRAW_MODE_CLR
        jnz DV3
        and es:[bx], al      ; Clear pixel
        jmp DVLOOP

DV3:    xor es:[bx], al      ; Invert pixel
        jmp  DVLOOP

DV_RET: ret

DrawVertLine ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  DrawHoriLine
;;
;;  Purpose:
;;      Draw straight horizontal line on _BlackBM (y1 == y2)
;;
;;  Input:
;;      [BP+CoX1] = x1, [BP+CoY1] = y1, [BP+CoX2] = x2
;;      ES = Start of bitmap, write protection off
;;
;;  Output:
;;      Bitmap has line x1,y1 to x2,y2 set, cleared or inverted.
;;
;;  Uses:
;;      AX, BX, CX, DX & DI

DrawHoriLine PROC  NEAR

        mov  si, [bp+Line]
        mov  ax, [si+LineY1]
        cmp  ax, [bp+ClipB]
        jl   DH_RET          ; All below clip box
        cmp  ax, [bp+ClipT]
        jge  DH_RET          ; All above clip box
        mov  bx, [si+LineX1]
        mov  dx, [si+LineX2]
        cmp  bx, dx          ; cmp x1, x2
        jl   @F
        xchg bx, dx
@@:
        ; ax = y,  bx = x, dx = x2
        mov  cx, [bp+ClipL]
        cmp  dx, cx          ; cmp x2, ClipL
        jl   DH_RET          ; All to left of clip box
        cmp  bx, cx          ; cmp x, ClipL
        jge  @F
        mov  bx, cx          ; Trim x to clip box
@@:
        mov  cx, [bp+ClipR]
        cmp  bx, cx          ; cmp x, ClipR
        jge  DH_RET          ; All to right of clip box
        cmp  dx, cx          ; cmp x2, ClipR
        jle  @F
        mov  dx, cx          ; Trim x2 to clip box
        dec  dx

@@:     ; ax = y, bx = x, dx = EndX
        ; Because the bitmap is 512 pixels wide (64 bytes) we can use
        ; shifts instead of a multiply
        mov  ah, al         ; Get y in ah (y is 0 to 159)
        CLR  al             ; ax = y * 256
        shr  ax, 1          ; ax = y * 128
        shr  ax, 1          ; ax = y * 64, start of row

        mov  cx, bx         ; Save beg x in cx
        shr  bx, 1          ; bx = beg x / 2
        shr  bx, 1          ; bx = beg x / 4
        shr  bx, 1          ; bx = beg x / 8
        add  bx, ax         ; bx now has index of beg word to change

        mov  di, dx         ; Save end x in di
        shr  di, 1          ; di = end x / 2
        shr  di, 1          ; di = end x / 4
        shr  di, 1          ; di = end x / 8
        add  di, ax         ; di now has index of end word to change

        cmp  di, BITMAPSIZE  ; Just in case (Write protection is off)
        jnb  PanicWriteOutsideBitmap

        and  cl, 007h       ; cl = BegX % 8
        mov  al, 0FFh
        shl  al, cl         ; ax now contains beg mask

        mov  cx, dx         ; Copy EndX to cx
        and  cl, 007h       ; cl = EndX % 8
        mov  dl, 0FEh
        shl  dl, cl
        not  dl             ; dx now contains end mask

        mov  cx, [bp+Mode]  ; Get drawing mode

DHLOOP: cmp  bx, di
        jne  DH2
        ; Next word == end word so combine flags
        and  al, dl

DH2:    cmp  cx, DRAW_MODE_SET
        jnz  DH3
        or   es:[bx], al    ; Set pixel on
        jmp  DH5

DH3:    cmp  cx, DRAW_MODE_CLR
        jnz  DH4
        not  al
        and  es:[bx], al    ; Clear pixel
        jmp  DH5

DH4:    xor  es:[bx], al    ; Invert pixel

DH5:    cmp  bx, di         ; check to see if done
        je   DH_RET
        inc  bx             ; Set bx to next word
        mov  al, 0FFh       ; Reset mask
        jmp  DHLOOP

DH_RET: ret

DrawHoriLine ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  void kcDrawLine( int x1, int y1, int x2, int y2 )
;;  INT CDECL kcDrawLine( INT* line, P_RECT* clip, DATTR* attr );
;;
;;  Purpose:
;;      Draw a straight line on the _BlackBM bitmap
;;
;;  Input:
;;      Pointer to start and finish coordinates in an array of INTs
;;        in order:- x1, y1, x2, y2
;;      Pointer to a normalised P_RECT struct to clip to
;;      Pointer to a DATTR struct. Uses members:-
;;        Bitmap - The bitmap handle to draw to
;;        Mode   - The Drawing Mode (DRAW_MODE_SET, _CLR, _INV)
;;        Other members ignored at present.
;;
;;  Output:
;;      Bitmap has line x1,y1 to x2,y2 set, cleared or inverted.
;;
;;  Uses:
;;      Everything, restores BP, SP, DI, SI & ES
;;      Calls DrawVertLine, DrawHoriLine & DrawDiagLine
;;      Switches off write protection and (if DatATFlag was originally zero)
;;      restores it on exit.
;;

; Function parameter offsets from BP
Line    equ    4
Clip    equ    6
Attr    equ    8

_kcDrawLine PROC  NEAR

        push bp
        mov  bp, sp
        sub  sp, DrawVar
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
        jz   DL1               ; NULL Clip box pointer

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
        jmp  DL2               ; Continue

DL1:    ; NULL clip box pointer, so load default
        CLR  ax
        mov  [bp+ClipL], ax
        mov  [bp+ClipB], ax
        mov  ax, XMAX
        mov  [bp+ClipR], ax
        mov  ax, YMAX
        mov  [bp+ClipT], ax

DL2:    ; Sort out special cases, Horizontal & Vertical. (45deg to come)
        mov  si, [bp+Line]
        mov  ax, [si+LineY1]   ; Compare y1 & y2
        cmp  ax, [si+LineY2]
        jnz  @F
        call DrawHoriLine      ; Draw a horizontal line
        jmp  DL_RET

@@:     mov  ax, [si+LineX1]   ; Compare x1 & x2
        cmp  ax, [si+LineX2]
        jnz  @F
        call DrawVertLine      ; Draw a vertical line
        jmp  DL_RET

@@:     call DrawDiagLine      ; General case, draw a diagonal line

DL_RET: ; Restore Write Protection
DL4:    pop  ss:[DatATFlag]
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

_kcDrawLine ENDP

;IF DEBUG EQ 1

PanicBitmapSizeWrong PROC NEAR
        mov al, PanicNumBitmapSizeWrong
        int ProcPanic
PanicBitmapSizeWrong ENDP

;ENDIF

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  INT CDECL kcIsLineInBox( INT* line, P_RECT* rect, DATTR* attr );
;;
;;  Purpose:
;;      Test if a straight line is in a rectangle
;;
;;  Input:
;;      Pointer to start and finish coordinates in an array of INTs
;;        in order:- x1, y1, x2, y2
;;      Pointer to a normalised P_RECT struct
;;      Pointer to a DATTR struct. Entrys ignored at present
;;
;;  Output:
;;      TRUE if a pixel would have been plotted, otherwise FALSE
;;
;;  Uses:
;;      Everything, restores BP, SP, DI, SI
;;      Calls DrawDiagLine (with [Mode] = DRAW_MODE_TST)
;;

_kcIsLineInRect PROC  NEAR

        push bp
        mov  bp, sp
        sub  sp, DrawVar
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

        ; Is line horizontal
        mov  si, [bp+Line]
        mov  ax, [si+LineY1]   ; Compare y1 & y2
        cmp  ax, [si+LineY2]
        jnz  IB10              ; No, carry on looking

        ; Test the horizontal line
        mov  ax, [si+LineY1]
        cmp  ax, [bp+ClipB]
        jl   IB_F              ; All below clip box
        cmp  ax, [bp+ClipT]
        jge  IB_F              ; All above clip box
        mov  bx, [si+LineX1]
        mov  dx, [si+LineX2]
        cmp  bx, dx            ; cmp x1, x2
        jl   @F
        xchg bx, dx
@@:
        ; ax = y,  bx = x, dx = x2
        mov  cx, [bp+ClipL]
        cmp  dx, cx            ; cmp x2, ClipL
        jl   IB_F              ; All to left of clip box
        mov  cx, [bp+ClipR]
        cmp  bx, cx            ; cmp x, ClipR
        jge  IB_F              ; All to right of clip box
        jmp  IB_T              ; Must be inside


IB10:   ; Is line vertical?
        mov  ax, [si+LineX1]   ; Compare x1 & x2
        cmp  ax, [si+LineX2]
        jnz  IB30

        mov  bx, [si+LineX1]
        cmp  bx, [bp+ClipL]
        jl   IB_F              ; Line to left of clip box
        cmp  bx, [bp+ClipR]
        jge  IB_F              ; Line to right of clip box

        mov  ax, [si+LineY1]
        mov  dx, [si+LineY2]
        cmp  ax, dx
        jle  @F
        xchg ax, dx
@@:
        ; bx = x, ax = y1 (bottom), dx = y2 (top)
        mov  cx, [bp+ClipT]
        cmp  ax, cx
        jge  IB_F              ; Line all above clip box
        mov  cx, [bp+ClipB]
        cmp  dx, cx
        jl   IB_F              ; Line all below clip box
                               ; Must be inside
IB_T:   mov  ax, 1             ; Return TRUE
        jmp  IB_RET

IB_F:   CLR  ax                ; Return FALSE
        jmp  IB_RET

IB30:   mov  [bp+PlotP], PlotTest ; Test only, don't plot
        call DrawDiagLine      ; General case, pretend draw a diagonal line
        ; Leave return flag in ax
        mov  ax, [bp+TestF]

IB_RET: ; Restore registers and stack
        pop  si
        pop  di
        mov  sp, bp
        pop  bp
        ret                    ; Return from function

_kcIsLineInRect ENDP

        END

; End of GRAPHA.ASM file


