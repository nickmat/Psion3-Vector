/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR PSION EPOC16          *  Written by: Nick Matthews  *
 *  Module: QuickCAD DYL IMPORT FUNCTION     *  Date Started:  3 Apr 1998  *
 *    File: VecQcdIm.C      Type: C SOURCE   *  Date Revised: 16 May 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1998 Nick Matthews
*/

#include <hwim.h>
#include <vecqcd.g>
#include <vector.g>
#include <vector.rsg>
#include "vec.h"

#define CODE(A,B)  ( ( ((UINT)(B)) << 8 ) | ((UINT)(A)) )
#define MAKECODE(B) ( *(UINT*) (B) )

static VOID UpdateEntry( PR_XCHANGE* self, TEXT* fbuf )
{
    INT num;
    LONG lnum;
    TEXT* pText;

    pText = &fbuf[4];
    p_stol( &pText, &lnum );
    if( fbuf[3] == '-' )
    {
        lnum = -lnum;
    }
    num = (INT) lnum;
    switch( MAKECODE( fbuf ) )
    {
    case CODE( 'N', 'M' ):
        switch( fbuf[3] )
        {
        case 'L':
            num = QCD_LINE;
            break;
        case 'B':
            num = QCD_BOX;
            break;
        case 'A':
            num = QCD_ARC;
            break;
        case 'C':
            num = QCD_CIRCLE;
            break;
        case 'T':
            num = QCD_TEXT;
            break;
        case 'N':
            num = QCD_NUMBER;
            break;
        case 'F':
            num = QCD_FILL;
            break;
        }
        self->xchange.entry.type = num;
        break;
    case CODE( 'X', '1' ):
        self->xchange.entry.x1 = num;
        break;
    case CODE( 'X', '2' ):
        self->xchange.entry.x2 = num;
        break;
    case CODE( 'X', '3' ):
        self->xchange.entry.x3 = num;
        break;
    case CODE( 'Y', '1' ):
        self->xchange.entry.y1 = num;
        break;
    case CODE( 'Y', '2' ):
        self->xchange.entry.y2 = num;
        break;
    case CODE( 'Y', '3' ):
        self->xchange.entry.y3 = num;
        break;
    case CODE( 'R', 'A' ):
        self->xchange.entry.radius = num;
        break;
    case CODE( 'T', 'H' ):
        self->xchange.entry.width = num;
        break;
    case CODE( 'F', 'C' ):
        self->xchange.entry.color = lnum;
        break;
    case CODE( 'F', 'S' ):
        self->xchange.entry.fontsize = num;
        break;
    case CODE( 'F', 'B' ):
        self->xchange.entry.bold = num;
        break;
    case CODE( 'F', 'I' ):
        self->xchange.entry.italic = num;
        break;
    case CODE( 'F', 'U' ):
        self->xchange.entry.underline = num;
        break;
    case CODE( 'F', 'N' ):
        fbuf[3+30] = '\0';
        p_scpy( self->xchange.entry.fontname, &fbuf[3] );
        break;
    case CODE( 'T', 'X' ):
        fbuf[3+200] = '\0';
        p_scpy( self->xchange.entry.text, &fbuf[3] );
        break;
    case CODE( 'N', 'O' ):
        fbuf[3+6] = '\0';
        p_scpy( self->xchange.entry.number, &fbuf[3] );
        break;
    }
}

static VOID SetPoint( A_PT* pPt, INT x, INT y, UINT res )
{
    LONG mul;
    AUNIT wd;

    wd = Drg->vec.Page.lim.x - Drg->vec.Page.pos.x;
    mul = (LONG) x * wd;
    pPt->x = p_send4( Drg, O_DG_LONG_DIV_UINT, &mul, res ) + Drg->vec.Page.pos.x;
    mul = (LONG) y * wd;
    pPt->y = p_send4( Drg, O_DG_LONG_DIV_UINT, &mul, res ) + Drg->vec.Page.pos.y;
}

static VOID AddEntry( PR_XCHANGE* self, UINT res )
{
    ELEM* pEl;
    EL_LARC larc;
    LONG mul;

    pEl = (ELEM*) DBuf;
    pEl->hdr.attr = 0;
    pEl->hdr.layer = 0;
    switch( self->xchange.entry.type )
    {
    case QCD_LINE:
        pEl->hdr.size = sizeof(EL_LINE);
        pEl->hdr.type = V_LINE;
        SetPoint( &pEl->line.beg, self->xchange.entry.x1, self->xchange.entry.y1, res );
        SetPoint( &pEl->line.end, self->xchange.entry.x2, self->xchange.entry.y2, res );
        break;
    case QCD_BOX:
        pEl->hdr.size = sizeof(EL_4PT);
        pEl->hdr.type = V_POLYGON;
        SetPoint( &pEl->el4pt.pt[0], self->xchange.entry.x1, self->xchange.entry.y1, res );
        SetPoint( &pEl->el4pt.pt[1], self->xchange.entry.x2, self->xchange.entry.y1, res );
        SetPoint( &pEl->el4pt.pt[2], self->xchange.entry.x2, self->xchange.entry.y2, res );
        SetPoint( &pEl->el4pt.pt[3], self->xchange.entry.x1, self->xchange.entry.y2, res );
        break;
    case QCD_ARC:
    case QCD_CIRCLE:
        pEl->hdr.size = sizeof(EL_3PT);
        pEl->hdr.type = V_3PT_ARC;
        SetPoint( &pEl->el3pt.beg, self->xchange.entry.x1, self->xchange.entry.y1, res );
        SetPoint( &pEl->el3pt.mid, self->xchange.entry.x2, self->xchange.entry.y2, res );
        SetPoint( &pEl->el3pt.end, self->xchange.entry.x3, self->xchange.entry.y3, res );
        if( self->xchange.entry.type == QCD_CIRCLE )
        {
            p_send4( Drg, O_DG_3PT_TO_LARC, &larc, pEl );
            pEl->hdr.size = sizeof(EL_CIRCLE);
            pEl->hdr.type = V_CIRCLE;
            pEl->circle.centre.x = (AUNIT) larc.centre.x;
            pEl->circle.centre.y = (AUNIT) larc.centre.y;
            pEl->circle.radius = (AUNIT) larc.radius;
        }
        break;
    case QCD_TEXT:
    case QCD_NUMBER:
        if( self->xchange.entry.type == QCD_TEXT )
        {
            pEl->hdr.size = SIZEOF_TEXTHDR + p_slen( self->xchange.entry.text );
            p_scpy( (TEXT*) pEl->text.text, self->xchange.entry.text );
            pEl->text.set.smul = self->xchange.entry.fontsize;
            pEl->text.set.sdiv = 72;
        }
        else
        {
            pEl->hdr.size = SIZEOF_TEXTHDR + p_slen( self->xchange.entry.number );
            p_scpy( (TEXT*) pEl->text.text, self->xchange.entry.number );
            pEl->text.set.smul = self->xchange.entry.fontsize * 3;
            pEl->text.set.sdiv = 72 * 4;
        }
        pEl->hdr.type = V_TEXT;
        SetPoint( &pEl->text.pos, self->xchange.entry.x1, self->xchange.entry.y1, res );
        mul = (LONG) self->xchange.entry.fontsize * 2500;
        pEl->text.pos.y += p_send4( Drg, O_DG_LONG_DIV_UINT, &mul, 72 );
        pEl->text.set.a.sin = 0;
        pEl->text.set.a.cos = TRIG_DIV;
        pEl->text.slant = 0;
        pEl->text.font = 0;
        p_send3( Drg, O_DG_SET_TEXT_RECT, pEl );
        break;
    default:
        return;
    }
    p_send3( Data, O_DD_ADD, pEl );
}

#pragma METHOD_CALL

/***************************************************************************
 **  xch_version  Return the compiled version number. NOTE dyl and app must
 **  ~~~~~~~~~~~  be compiled with the same headers.
 */

METHOD UINT xchange_xch_version( PR_XCHANGE* self )
{
    return VERSION_XCH;
}

/***************************************************************************
 **  xch_import  Import the given file. Calls p_leave if error occurs.
 **  ~~~~~~~~~~
 */

METHOD INT xchange_xch_import( PR_XCHANGE* self, TEXT* pFName )
{
    VOID* pcb;
    INT ret;
    TEXT* fbuf;
    UINT res;

    fbuf = (TEXT*) p_send2( Drg, O_DG_GET_FBUF );
    ret = p_open( &pcb, pFName, P_FTEXT | P_FSHARE );
    if( ret ) p_leave( ret );
    ret = p_read( pcb, fbuf, EL_BUFSIZE - 1 );
    /* check first line */
    fbuf[ret] = '\0';
    ret = p_read( pcb, fbuf, EL_BUFSIZE - 1 );
    /* check second line */
    fbuf[ret] = '\0';
    res = 800;
    ret = p_read( pcb, fbuf, EL_BUFSIZE - 1 );
    while( ret != E_FILE_EOF )
    {
        if( ret < 0 )
        {
            p_leave( ret );
        }
        fbuf[ret] = '\0';
        UpdateEntry( self, fbuf );
        ret = p_read( pcb, fbuf, EL_BUFSIZE - 1 );
        if( MAKECODE( fbuf ) == CODE( 'N', 'M' ) || ret == E_FILE_EOF )
        {
            AddEntry( self, res );
        }
    }
    p_close( pcb );
    return 0;
}


/* End of VecQcdIm.c file */
