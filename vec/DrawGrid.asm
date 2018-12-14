;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;* Project: GRAPHIC LIBRARY FOR THE PSION 3a *  Written by: Nick Matthews  *
;*  Module: DRAW GRID DOTS TO BITMAP         *  Date Started: 14 Feb 1997  *
;*    File: DRAWGRID.ASM    Type: ASSEMBLER  *  Date Revised: 14 Feb 1997  *
;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
; Copyright (c) 1997 Nick Matthews
;
; Written for Microsoft MASM V6.11
; Assemble using ML /c /Cp
;

        .MODEL small
        INCLUDE  kcmacros.inc
        INCLUDE  vector.inc

; GRID_DATA Struct
D_1stX  equ  0
D_1stY  equ  2
D_GapX  equ  4
D_GapY  equ  6
D_Upp   equ  8
D_Pps   equ 10
D_Gps   equ 12
D_Maj   equ 14
D_Col1  equ 16
D_Maj1  equ 18
D_MGapY equ 20
D_MPps  equ 22
D_MGps  equ 24

; Local variables, as for DrawLine with the following reused
Maj1stY equ CoX1
MajGapY equ CoY1
MajPps  equ CoX2
MajGps  equ CoY2
Min1stY equ Steep
MinGapY equ YSign
MinPps  equ OnY0
MinGps  equ OnY1
Major   equ BegX
Upp     equ BegY
GapX    equ  -2  - DrawVar
Col     equ  -4  - DrawVar
Pps     equ  -6  - DrawVar
Gps     equ  -8  - DrawVar

GridVar equ   8 + DrawVar

        .CODE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  DrawGrid
;;
;;  Purpose:
;;      Draw grid on bitmap
;;
;;  Input:
;;      [BP+Data] = GRID_DATA structure
;;      ES = Start of bitmap, write protection off
;;
;;  Output:
;;      Bitmap has Grid dots set, cleared or inverted.
;;
;;  Uses:
;;      AX, BX, CX, DX, SI & DI

DrawGrid PROC  NEAR

        ; Load up variables from Data
        mov  si, [bp+Data]
        mov  bx, [si+D_1stX]  ; Put into SI later
        mov  ax, [si+D_1stY]
        mov  [bp+Min1stY], ax
        mov  cx, [si+D_GapX]  ; Leave GapX in CX
        mov  ax, [si+D_GapY]
        mov  [bp+MinGapY], ax
        mov  ax, [si+D_Upp]
        mov  [bp+Upp], ax
        mov  ax, [si+D_Pps]
        mov  [bp+MinPps], ax
        mov  ax, [si+D_Gps]
        mov  [bp+MinGps], ax
        mov  ax, [si+D_Maj]
        dec  ax               ; Decrease ready for count
        mov  [bp+Major], ax
        mov  ax, [si+D_Col1]
        mov  [bp+Col], ax
        mov  ax, [si+D_Maj1]
        mov  [bp+Maj1stY], ax
        mov  ax, [si+D_MGapY]
        mov  [bp+MajGapY], ax
        mov  ax, [si+D_MPps]
        mov  [bp+MajPps], ax
        mov  ax, [si+D_MGps]
        mov  [bp+MajGps], ax
        mov  si, bx           ; D_1stX

DG_XLP: ; cx=GapX, dx=GapY, si=x, di=y
        cmp  si, [bp+ClipR]
        jae  DG_RET           ; All done
        mov  [bp+GapX], cx    ; Free up cx

        ; check if a major col
        mov  cx, [bp+Col]
        jcxz DG_X1

        ; Load up a Major spaced column
        mov  di, [bp+Maj1stY]
        mov  dx, [bp+MajGapY]
        mov  cx, [bp+MajPps]
        mov  [bp+Pps], cx
        mov  cx, [bp+MajGps]
        mov  [bp+Gps], cx
        dec  WORD PTR [bp+Col]
        jmp  DG_YLP

DG_X1:  ; Load up a minor spaced column
        mov  di, [bp+Min1stY]
        mov  dx, [bp+MinGapY]
        mov  cx, [bp+MinPps]
        mov  [bp+Pps], cx
        mov  cx, [bp+MinGps]
        mov  [bp+Gps], cx
        mov  cx, [bp+Major]
        mov  [bp+Col], cx

DG_YLP: ; dx=GapY, si=x, di=y
        cmp  di, [bp+ClipT]
        jae  DG_X2           ; End inner loop
        mov  bx, si          ; bx=x
        mov  ax, di          ; ax=y
        call WORD PTR [bp+PlotP]
        add  di, [bp+Pps]    ; y += Pps
        add  dx, [bp+Gps]    ; GapY += Gps
        cmp  dx, [bp+Upp]
        jl   DG_YLP
        inc  di
        sub  dx, [bp+Upp]
        jmp  DG_YLP
DG_X2:  ; Update x and GapX
        mov  cx, [bp+GapX]
        add  si, [bp+MinPps]
        add  cx, [bp+MinGps]
        cmp  cx, [bp+Upp]
        jl   DG_XLP
        inc  si
        sub  cx, [bp+Upp]
        jmp  DG_XLP

DG_RET: ret

DrawGrid ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  INT CDECL kcDrawGrid( INT* data, SRECT* clip, DATTR* attr );
;;
;;  Purpose:
;;      Draw a grid of dots (pixels) on the given bitmap
;;
;;  Input:
;;      Pointer to an array of ints:-
;;        First absolute x coord
;;        First absolute y coord
;;        Space between dots in absolute units
;;        Absolute units per pixel
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
Data    equ    4
Clip    equ    6
Attr    equ    8

_kcDrawGrid PROC  NEAR

        push bp
        mov  bp, sp
        sub  sp, GridVar
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
        jz   DG1               ; NULL Clip box pointer

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
        jmp  DG2               ; Continue

DG1:    ; NULL clip box pointer, so load default
        CLR  ax
        mov  [bp+ClipL], ax
        mov  [bp+ClipB], ax
        mov  ax, XMAX
        mov  [bp+ClipR], ax
        mov  ax, YMAX
        mov  [bp+ClipT], ax
DG2:
        call DrawGrid       ; Go do it

DL_RET: ; Restore Write Protection
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

_kcDrawGrid ENDP

        END

; End of GRAPHA.ASM file


