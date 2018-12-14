/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: EXPORT DRAWING TO BITMAP FILE    *  Date Started: 13 Aug 1997  *
 *    File: VECEXP.C        Type: C SOURCE   *  Date Revised: 26 Aug 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <plib.h>
#include <p_crc.h>
#include <hwim.h>
#include <vector.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"
#include "vecpcx.h"

#define ROW_BYTES   64
#define ROW_BUF_MAX 128
#define PIC_CRC_POS 8

#define FBUFLEN 128

typedef struct
{
    VOID* pcb;
    UINT  index;
    UBYTE buf[FBUFLEN];
} FBUFFER;

static VOID FlushBuffer( FBUFFER* fbuf )
{
    f_write( fbuf->pcb, fbuf->buf, fbuf->index );
    fbuf->index = 0;
}

static VOID WriteByte( FBUFFER* fbuf, INT byte )
{
    fbuf->buf[fbuf->index++] = byte;
    if( fbuf->index == FBUFLEN )
    {
        FlushBuffer( fbuf );
    }
}

static VOID DrawBitmap( UINT row1, UINT rows, UINT cols )
{
    SRECT sRect;
    SWORD upp;
    AUNIT pos;

    sRect.pos.x = 0;
    sRect.pos.y = 0;
    sRect.lim.x = ( cols > 512 ) ? 512 : cols;
    sRect.lim.y = rows;

    upp = Drg->vec.upp;
    pos = Drg->vec.Page.pos.y;
    Drg->vec.Scrn.pos.y = ( pos / upp + row1 ) * upp;
    pos = Drg->vec.Page.pos.x;
    Drg->vec.Scrn.pos.x = ( pos / upp ) * upp;

    p_send2( Drg, O_DG_CLEAR_BITMAP );
    p_send3( Drg, O_DG_DRAW_ALL, &sRect );
    if( cols > 512 )
    {
        p_send2( Drg, O_DG_SWAP_BITMAP );
        sRect.lim.x = cols - 512;
        pos += upp * 512;
        Drg->vec.Scrn.pos.x = ( pos / upp ) * upp;
        p_send3( Drg, O_DG_DRAW_ALL, &sRect );
        p_send2( Drg, O_DG_SWAP_BITMAP );
    }
}

static INT ReverseByte( INT byte )
{
    INT i, rev;

    rev = 0;
    for( i = 0 ; i < 8 ; i++ )
    {
        rev |= ( ( byte >> i ) & 1 ) << ( 7 - i );
    }
    return rev ^ 0x00ff;
}

static VOID WritePcxLine( FBUFFER* fbuf, UBYTE* buf, UINT len )
{
    UBYTE* p;
    UINT end;
    UINT count;
    INT byte;

    p = buf;
    byte = *p++;
    count = 1;
    do
    {
        end = ( p == ( buf + len ) );
        if( byte == *p && count < 0x3f && ! end )
        {
            count++;
            p++;
        }
        else
        {
            byte = ReverseByte( byte );
            if( count > 1 || ( byte & 0xc0 ) == 0xc0 )
            {
                WriteByte( fbuf, count + 0xc0 );
            }
            WriteByte( fbuf, byte );
            byte = *p++;
            count = 1;
        }
    } while( ! end );
}


#pragma save, ENTER_CALL

static INT WritePicFile( VOID* pcb, UINT cols, UINT rows )
{
    PIC_HEAD* pHead;
    UBYTE buf[ROW_BUF_MAX];
    LONG pos;
    HANDLE handle1, handle2;
    UINT row1, bm_rows;
    UINT col_bytes;
    UWORD crc;
    INT i;

    col_bytes = ( ( cols + 15 ) / 16 ) * 2;

    pHead = (PIC_HEAD*) p_send2( Drg, O_DG_GET_PIC_HEAD );
    pHead->wspic.size.x = cols;
    pHead->wspic.size.y = rows;
    pHead->wspic.byte_size = col_bytes * rows;
    f_write( pcb, pHead, sizeof(PIC_HEAD) );

    crc = 0;
    row1 = 0;
    handle1 = p_send3( Drg, O_DG_GET_BMHAND, BITMAP_BLACK );
    handle2 = p_send3( Drg, O_DG_GET_BMHAND, BITMAP_GREY );
    for(;;)
    {
        pos = 0L;
        if( row1 + 160 > rows )
        {
            bm_rows = rows - row1;
        }
        else
        {
            bm_rows = 160;
        }
        DrawBitmap( row1, bm_rows, cols );
        for( i = 0 ; i < bm_rows ; i++ )
        {
            p_sgcopyfr( handle1, pos, buf, ROW_BYTES );
            if( cols > 512 )
            {
                p_sgcopyfr( handle2, pos, &buf[ROW_BYTES], ROW_BYTES );
            }
            f_write( pcb, buf, col_bytes );
            p_crc( &crc, buf, col_bytes );
            pos += ROW_BYTES;
        }
        row1 += 160;
        if( row1 >= rows ) break;
    }
    pos = PIC_CRC_POS;
    f_seek( pcb, P_FABS, &pos );
    f_write( pcb, &crc, sizeof(UWORD) );
    return p_iow( pcb, P_FFLUSH );  /* Don't give p_close a chance to fail */
}

static INT WritePcxFile( VOID* pcb, UINT cols, UINT rows )
{
    FBUFFER fbuf;
    PCX_HEAD head;
    UBYTE buf[ROW_BUF_MAX];
    LONG pos;
    HANDLE handle1, handle2;
    UINT row1, bm_rows;
    UINT col_bytes;
    INT i;

    fbuf.pcb = pcb;
    fbuf.index = 0;

    col_bytes = ( ( cols + 15 ) / 16 ) * 2;

    p_bfil( &head, sizeof(PCX_HEAD), 0 );
    head.Identifier = 0x0a;
    head.Version = 3;
    head.Encoding = 1;
    head.BitsPerPixel = 1;
    head.PicDimWindow.lim.x = cols - 1;
    head.PicDimWindow.lim.y = rows - 1;
    head.HorzRes = 640;
    head.VertRes = 480;
    head.NumBitPlanes = 1;
    head.BytesPerLine = col_bytes;
    f_write( pcb, &head, sizeof(PCX_HEAD) );

    row1 = 0;
    handle1 = p_send3( Drg, O_DG_GET_BMHAND, BITMAP_BLACK );
    handle2 = p_send3( Drg, O_DG_GET_BMHAND, BITMAP_GREY );
    for(;;)
    {
        pos = 0L;
        if( row1 + 160 > rows )
        {
            bm_rows = rows - row1;
        }
        else
        {
            bm_rows = 160;
        }
        DrawBitmap( row1, bm_rows, cols );
        for( i = 0 ; i < bm_rows ; i++ )
        {
            p_sgcopyfr( handle1, pos, buf, ROW_BYTES );
            if( cols > 512 )
            {
                p_sgcopyfr( handle2, pos, &buf[ROW_BYTES], ROW_BYTES );
            }
            WritePcxLine( &fbuf, buf, col_bytes );
            pos += ROW_BYTES;
        }
        row1 += 160;
        if( row1 >= rows ) break;
    }
    FlushBuffer( &fbuf );
    return p_iow( pcb, P_FFLUSH );  /* Don't give p_close a chance to fail */
}

#pragma restore

/***************************************************************************
 **  ex_export  Export to given file, format and size
 **  ~~~~~~~~~
 */

#pragma METHOD_CALL

METHOD VOID extra_ex_export( PR_EXTRA* self, TEXT* fname, INT type, INT width )
{
    A_PT scrnpos;
    SWORD upp;
    UINT bandlayer;
    INT process;
    INT quick;
    AUNIT tmp;
    UINT rows;
    VOID* pcb;
    INT ret;

    hInfoPrint( SR_EXPORTED_AS_FMT, fname, width );

    /* Save Drg settings */
    bandlayer = Drg->drg.BandLayer;
    process = Drg->drg.ProcessOp;
    quick = Drg->drg.Quick;
    upp = Drg->vec.upp;
    scrnpos = Drg->vec.Scrn.pos;

    BegDraw(); /* Clear the cursor & grid etc. */

    /* calc & change Drg settings */
    Drg->drg.BandLayer = BL_BLACK;
    Drg->drg.ProcessOp = PO_NONE;
    Drg->drg.Quick = FALSE;

    tmp = Drg->vec.Page.lim.x - Drg->vec.Page.pos.x - 1;
    Drg->vec.upp = tmp / width + 1;
    tmp = Drg->vec.Page.lim.y - Drg->vec.Page.pos.y - 1;
    rows = tmp / Drg->vec.upp + 1;

    ret = p_open( &pcb, fname, P_FSTREAM | P_FREPLACE | P_FUPDATE | P_FRANDOM );
    if( ret == 0 )
    {
        switch( type )
        {
        case SAVE_FTYPE_PIC:
            ret = p_enter4( WritePicFile, pcb, width, rows );
            break;
        case SAVE_FTYPE_PCX:
            ret = p_enter4( WritePcxFile, pcb, width, rows );
            break;
        default:
            ret = 0;
        }
        p_close( pcb );
        if( ret != 0 )
        {
            /* Something went wrong */
            p_delete( fname );
        }
    }
    else
    {
        /* Error opening file */
    }

    /* Restore Drg settings */
    Drg->drg.BandLayer = bandlayer;
    Drg->drg.ProcessOp = process;
    Drg->drg.Quick = quick;
    Drg->vec.Scrn.pos = scrnpos;
    Drg->vec.upp = upp;

    p_send2( Drg, O_DG_REDRAW );
    EndDraw();
}

/* End of VecExp.c file */
