/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Project: Vector - Vector graphics system *  Written by: Nick Matthews  *
 *   Module: PCX File format                 *  Date Started: 15 Aug 1997  *
 *     File: VecPcx.h      Type: C Header    *  Date Revised: 15 Aug 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews

 Details obtained from "Encylopedia of Graphics File Formats" 2nd Edition.
 See page 662.

*/

#ifndef VECPCX_H
#define VECPCX_H

typedef struct
{
    UBYTE Identifier;    /* PCX id number (always 0x0A) */
    UBYTE Version;       /* Version number              */
    UBYTE Encoding;      /* Encoding format             */
    UBYTE BitsPerPixel;  /* Bits per pixel              */
    ARECT PicDimWindow;  /* Picture dimension window    */
    UWORD HorzRes;       /* Horizontal resolution       */
    UWORD VertRes;       /* Vertical Resolution         */
    UBYTE Palette[48];   /* 16-Colour EGA palette       */
    UBYTE Reserved1;     /* Reserved (always 0)         */
    UBYTE NumBitPlanes;  /* Number of bit planes        */
    UWORD BytesPerLine;  /* Bytes per scan line         */
    UWORD PaletteType;   /* Palette type                */
    UWORD HorzScrnSize;  /* Horizontal screen size      */
    UWORD VertScrnSize;  /* Vertical screen size        */
    UBYTE Reserved2[54]; /* Reserved (always 0)         */
} PCX_HEAD;

#endif /* VECPCX_H */
