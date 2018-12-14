;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;* Project: GRAPHIC LIBRARY FOR THE PSION 3a *  Written by: Nick Matthews  *
;*  Module: CLEAR BITMAP ROUTINES            *  Date Started:  5 Aug 1997  *
;*    File: CLEAR.ASM       Type: ASSEMBLER  *  Date Revised: 11 Aug 1997  *
;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
; Copyright (c) 1997 Nick Matthews
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

; Function arguments
BitMap  equ    4
pRect   equ    6


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  ClearBitmap
;;
;;  Purpose:
;;      Clear the bitmap (All bytes set to zero).
;;
;;  Input:
;;      ES = Start of bitmap, write protection off
;;
;;  Output:
;;      Bitmap cleared.
;;
;;  Uses:
;;      AX, CX, DI and D flag
;;      Restores D Flag
;;

ClearBitmap PROC  NEAR


        ; Clear the bitmap
        pushf                     ; Save the flags (for direction flag)
        CLR  ax
        CLR  di
        mov  cx, BITMAPSIZE/2     ; Number of words in bitmap
        cld
        rep stosw                 ; copies ax to es:di cx times
        popf                      ; restores direction flag
        ret

ClearBitmap ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  void kcClearBitmap( HANDLE bitmap );
;;
;;  Purpose:
;;      Clear the bitmap (All bytes set to zero).
;;
;;  Input:
;;      Bitmap handle on the stack.
;;
;;  Output:
;;      Bitmap cleared.
;;
;;  Uses:
;;      Everything except SI, restores BP, SP, DI & ES and D Flag
;;      Switches off write protection and (if DatATFlag was originally zero)
;;      restores it on exit.
;;

_kcClearBitmap PROC  NEAR

        push bp
        mov  bp, sp
        push di

IF DEBUG EQ 1
        ; Do a SegSize here to check the segment handle is valid.
        mov  ah, NmSegSize        ; Function number
        mov  bx, [bp+BitMap]      ; Bitmap segment handle
        int  SegManager
        cmp  ax, BITMAPSIZE/16    ; Expected bitmap size, in para
        jne  PanicBitmapSizeWrong ; Bad news
ENDIF
        int  GenDataSegment       ; Get the o/s data space in ES
        mov  bx, [bp+BitMap]      ; Get the segment handle
        mov  es, es:[bx]          ; Get the base segment

        push ss:[DatATFlag]
        mov  byte ptr ss:[DatATFlag], 0
        out  15h, al              ; Disable write protection

        call ClearBitmap

        ; Restore Write Protection
        pop  ss:[DatATFlag]
        cmp  byte ptr ss:[DatATFlag], 0
        jz   @F                   ; Skip Enable write protection
        out  14h, al              ; Enable write protection
        ; Restore es
@@:     cli                       ; Disable interrupts
        push ds                   ; Save copy of DS
        pop  es                   ; Recover ES
        sti                       ; Re-enable interrupts
        ; Restore registers and stack
        pop  di
        mov  sp, bp
        pop  bp
        ret

_kcClearBitmap ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  ClearRect
;;
;;  Purpose:
;;      Clear a normalised rectangle of screen
;;
;;  Input:
;;      [BP+ClrRect] = ptr to rect structure
;;      ES = Start of bitmap, write protection off
;;
;;  Output:
;;      Bitmap has rectangle cleared.
;;
;;  Uses:
;;      AX, BX, CX, DX, DI & SI

ClearRect PROC  NEAR

        mov  si, [bp+pRect]

        mov  dx, [si+RectL] ; dx = x beg
        mov  cx, dx
        and  cl, 007h
        mov  al, 0FFh
        shl  al, cl         ; al = mask for beg col

        mov  di, [si+RectR] ; di = x end
        dec  di
        mov  cx, di
        and  cl, 007h
        mov  bl, 0FEh
        shl  bl, cl
        not  bl
        mov  ah, bl         ; ah = mask for end col

        mov  cx, [si+RectT] ; cx = y 1st row
        dec  cx
        mov  bh, cl
        CLR  bl             ; bx = y * 256
        shr  bx, 1          ; bx = y * 128
        shr  bx, 1          ; bx = y * 64 = start of row

        sub  cx, [si+RectB] ; cx = count of rows
        inc  cx
        mov  si, cx         ; si = count of rows

        shr  dx, 1
        shr  dx, 1
        shr  dx, 1
        add  dx, bx         ; dx = offset to beg of 1st row

        shr  di, 1
        shr  di, 1
        shr  di, 1
        add  di, bx         ; di = offset to end of 1st row

        ; al = beg mask, ah = end mask,
        ; dx = offset 1st row, di = end offset, si = number of rows
        ; Uses:  bx = offset into bitmap, cx = row count,
CRLOOP: cmp  dx, di
        jne  CRL10
        and  al, ah         ; Next word == end word so combine flags
CRL10:
        not  al
        mov  bx, dx
        mov  cx, si
CR_LP2: ; do column
        cmp  bx, BITMAPSIZE  ; Just in case (Write protection is off)
        jnb  PanicWriteOutsideBitmap
        and  es:[bx], al    ; Clear pixel
        sub  bx, ROWLENGTH
        loop CR_LP2

        cmp  dx, di         ; check to see if done
        je   CR_RET
        inc  dx             ; Set dx to next word
        mov  al, 0FFh       ; Reset mask
        jmp  CRLOOP

CR_RET: ret

ClearRect ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  void kcClearRect( HANDLE bitmap, P_RECT* rect );
;;
;;  Purpose:
;;      Clear the given rectangle in the bitmap (All bytes set to zero).
;;
;;  Input:
;;      Bitmap handle and pointer to a normalised P_RECT struct on stack
;;
;;  Output:
;;      Rectangle in the bitmap cleared.
;;
;;  Uses:
;;      Everything, restores BP, SP, SI, DI & ES and D Flag
;;      Switches off write protection and (if DatATFlag was originally zero)
;;      restores it on exit.
;;

BitMap  equ    4
pRect   equ    6

_kcClearRect PROC  NEAR

        push bp
        mov  bp, sp
        push di
        push si

;IF DEBUG EQ 1
        ; Do a SegSize here to check the segment handle is valid.
        mov  ah, NmSegSize        ; Function number
        mov  bx, [bp+BitMap]      ; Bitmap segment handle
        int  SegManager
        cmp  ax, BITMAPSIZE/16    ; Expected bitmap size, in para
        jne  PanicBitmapSizeWrong ; Bad news
;ENDIF
        int  GenDataSegment       ; Get the o/s data space in ES
        mov  bx, [bp+BitMap]      ; Get the segment handle
        mov  es, es:[bx]          ; Get the base segment

        push ss:[DatATFlag]
        mov  byte ptr ss:[DatATFlag], 0
        out  15h, al              ; Disable write protection

        call ClearRect

        ; Restore Write Protection
        pop  ss:[DatATFlag]
        cmp  byte ptr ss:[DatATFlag], 0
        jz   @F                   ; Skip Enable write protection
        out  14h, al              ; Enable write protection
        ; Restore es
@@:     cli                       ; Disable interrupts
        push ds                   ; Save copy of DS
        pop  es                   ; Recover ES
        sti                       ; Re-enable interrupts
        ; Restore registers and stack
        pop  si
        pop  di
        mov  sp, bp
        pop  bp
        ret                       ; Return from function

_kcClearRect ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  ScrollBitmap
;;
;;  Purpose:
;;      Scroll the complete bitmap in the directions given [bp+ScrollX] and
;;      [bp+ScrollY]. Clears the area exposed.
;;
;;  Input:
;;      [bp+ScrollX] and [bp+ScrollY].
;;      ES = Start of bitmap, write protection off
;;
;;  Output:
;;      Bitmap is scroll/cleared as required.
;;
;;  Uses:
;;      All registers. Subroutine ClearBitmap
;;

ScrollBitmap PROC  NEAR

        mov  ax, [bp+ScrollY]
        TST  ax
        jz   SCROLL_HOZ
        jns  SCROLL_DOWN

        ; Scroll up
        neg  ax                 ; ax = rows to scroll = sa
        cmp  ax, COLLENGTH
        jae  CLEAR_ALL

        mov  si, ax             ; si = sa
        mov  cl, 6
        shl  si, cl             ; si = sa * 64 = offset to 1st row to move
        CLR  di                 ; di = 0 = start of bitmap
        mov  cx, BITMAPSIZE
        sub  cx, si             ; cx = size if scroll area
        shr  cx, 1

SU_LP1: ; don't use rep instruction because of possible interupts
        mov  ax, es:[si]
        mov  es:[di], ax
        add  si, 2
        add  di, 2
        loop SU_LP1

        CLR  ax
SU_LP2: ; Clear remaining area
        mov  es:[di], ax
        add  di, 2
        cmp  di, BITMAPSIZE
        jl   SU_LP2
        jmp  SCROLL_HOZ

SCROLL_DOWN: ; ax = rows to scroll down
        cmp  ax, COLLENGTH
        jae  CLEAR_ALL

        mov  cl, 6
        shl  ax, cl

        mov  di, BITMAPSIZE
        mov  si, di
        sub  si, ax
        mov  cx, si
        shr  cx, 1

SD_LP1: sub  si, 2
        sub  di, 2
        mov  ax, es:[si]
        mov  es:[di], ax
        loop SD_LP1

        CLR  ax
SD_LP2: sub  di, 2
        mov  es:[di],ax
        cmp  di, 2
        jge  SD_LP2   ; jump if di > 2
        ;jmp SCROLL_HOZ

SCROLL_HOZ:
        mov  dx, [bp+ScrollX]
        TST  dx
        jz   SB_RET
        jns  SCROLL_RIGHT

        ; Scroll left
        neg  dx
        cmp  dx, XMAX
        jae  CLEAR_ALL

        CLR  bx        ; bx = 0 (Row offset)

SLLOOP:
        CLR  di        ; di = 0
        mov  si, dx
        mov  cl, 3
        shr  si, cl    ; si = sl / 8
        mov  cx, dx
        and  cx, 007h  ; cl = sl % 8

        mov  ch, es:[bx+si]

SL_LP1:
        mov  al, ch
        inc  si
        cmp  si, ROWLENGTH
        jae  SL10
        mov  ah, es:[bx+si]
        mov  ch, ah
        shr  ax, cl
        mov  es:[bx+di], al
        inc  di
        jmp  SL_LP1

SL10:   shr  al, cl
        mov  es:[bx+di], al
        CLR  al

SL_LP2:
        inc  di
        cmp  di, ROWLENGTH
        jae  SL20
        mov  es:[bx+di], al
        jmp  SL_LP2

SL20:   add  bx, ROWLENGTH
        cmp  bx, BITMAPSIZE
        jl   SLLOOP
        jmp  SB_RET

SCROLL_RIGHT:
        cmp  dx, XMAX
        jae  CLEAR_ALL

        CLR  bx        ; bx = 0 (Row offset)

SRLOOP:
        mov  si, ROWLENGTH  ; si = 64 (bytes per row)
        mov  di, dx
        mov  cl, 3
        shr  di, cl         ; di = sr / 8
        sub  si, di         ; si = rowlen - sr / 8
        mov  di, ROWLENGTH  ; di = 64 (bytes per row)
        mov  cx, dx
        and  cx, 007h       ; cl = sr % 8

        dec  si
        mov  ch, es:[bx+si]

SR_LP1:
        mov  ah, ch
        dec  si
        TST  si
        js   SR10
        mov  al, es:[bx+si]
        mov  ch, al
        shl  ax, cl
        dec  di
        mov  es:[bx+di], ah
        jmp  SR_LP1

SR10:   shl  ah, cl
        dec  di
        mov  es:[bx+di], ah
        CLR  ah

SR_LP2:
        dec  di
        TST  di
        js   SR20
        mov  es:[bx+di], ah
        jmp  SR_LP2

SR20:   add  bx, ROWLENGTH
        cmp  bx, BITMAPSIZE
        jl   SRLOOP
        jmp  SB_RET

CLEAR_ALL:
        call ClearBitmap

SB_RET: ret

ScrollBitmap ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  void kcScrollBitmap( HANDLE bitmap, INT sx, INT sy );
;;
;;  Purpose:
;;      Scroll the complete bitmap in the directions given by sx and sy.
;;      Clears the area exposed.
;;
;;  Input:
;;      Bitmap handle and horizontal and vertical distances to scroll
;;
;;  Output:
;;      Bitmap is scroll/cleared as required.
;;
;;  Uses:
;;      Everything, restores BP, SP, SI, DI & ES and D Flag
;;      Switches off write protection and (if DatATFlag was originally zero)
;;      restores it on exit.
;;

BitMap  equ    4
ScrollX equ    6
ScrollY equ    8

_kcScrollBitmap PROC  NEAR

        push bp
        mov  bp, sp
        push di
        push si

;IF DEBUG EQ 1
        ; Do a SegSize here to check the segment handle is valid.
        mov  ah, NmSegSize        ; Function number
        mov  bx, [bp+BitMap]      ; Bitmap segment handle
        int  SegManager
        cmp  ax, BITMAPSIZE/16    ; Expected bitmap size, in para
        jne  PanicBitmapSizeWrong ; Bad news
;ENDIF
        int  GenDataSegment       ; Get the o/s data space in ES
        mov  bx, [bp+BitMap]      ; Get the segment handle
        mov  es, es:[bx]          ; Get the base segment

        push ss:[DatATFlag]
        mov  byte ptr ss:[DatATFlag], 0
        out  15h, al              ; Disable write protection

        call ScrollBitmap

        ; Restore Write Protection
        pop  ss:[DatATFlag]
        cmp  byte ptr ss:[DatATFlag], 0
        jz   @F                   ; Skip Enable write protection
        out  14h, al              ; Enable write protection
        ; Restore es
@@:     cli                       ; Disable interrupts
        push ds                   ; Save copy of DS
        pop  es                   ; Recover ES
        sti                       ; Re-enable interrupts
        ; Restore registers and stack
        pop  si
        pop  di
        mov  sp, bp
        pop  bp
        ret                       ; Return from function

_kcScrollBitmap ENDP

        END

; End of GRAPHA.ASM file


