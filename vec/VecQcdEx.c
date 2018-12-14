/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR PSION EPOC16          *  Written by: Nick Matthews  *
 *  Module: QuickCAD DYL EXPORT FUNCTION     *  Date Started: 19 May 1998  *
 *    File: VecQcdEx.C      Type: C SOURCE   *  Date Revised: 19 May 1998  *
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

#define LayerBit(L) ( 1 << ( (L) & 0x0f ) )
#define DATASIZE(H) ( ( ((UBYTE*)(H))[SIZE_BYTE] - sizeof(ELHDR) ) / sizeof(A_PT) )

static VOID WriteElem( PR_XCHANGE* self, VOID* pcb, ELEM* pEl );

static UINT GetDim( RUNIT ru, UINT res )
{
    LONG mul;
    AUNIT wd;

    wd = Drg->vec.Page.lim.x - Drg->vec.Page.pos.x;
    mul = (LONG) ru * res;
    return p_send4( Drg, O_DG_LONG_DIV_UINT, &mul, wd );
}

static UINT GetLDim( LONG lu, UINT res )
{
    AUNIT wd;

    wd = Drg->vec.Page.lim.x - Drg->vec.Page.pos.x;
    lu *= res;
    return p_send4( Drg, O_DG_LONG_DIV_UINT, &lu, wd );
}

static VOID SetFlag( TEXT* buf, UINT flag )
{
    if( flag )
    {
        buf[0] = ' ';
        buf[1] = '0';
    }
    else
    {
        buf[0] = '-';
        buf[1] = '1';
    }
    buf[2] = '\0';
}

static VOID WriteQcd( PR_XCHANGE* self, VOID* pcb, QCD_ENTRY* pQcd )
{
    QCD_ENTRY* pEnt;
    TEXT buf[20];
    TEXT* dbuf;

    pEnt = &self->xchange.entry;
    dbuf = (TEXT*) DBuf;

    hLoadResBuf( SR_QCD_NAME + pQcd->type, buf );
    hAtos( dbuf, SR_QCD_NAME_FMT, buf );
    p_write( pcb, dbuf, p_slen( dbuf ) );

    if( pEnt->x1 != pQcd->x1 )
    {
        hAtos( dbuf, SR_QCD_ENT_FMT, 'X', '1', pQcd->x1  );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pEnt->x2 != pQcd->x2 )
    {
        hAtos( dbuf, SR_QCD_ENT_FMT, 'X', '2', pQcd->x2  );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pEnt->x3 != pQcd->x3 )
    {
        hAtos( dbuf, SR_QCD_ENT_FMT, 'X', '3', pQcd->x3  );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pEnt->y1 != pQcd->y1 )
    {
        hAtos( dbuf, SR_QCD_ENT_FMT, 'Y', '1', pQcd->y1  );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pEnt->y2 != pQcd->y2 )
    {
        hAtos( dbuf, SR_QCD_ENT_FMT, 'Y', '2', pQcd->y2  );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pEnt->y3 != pQcd->y3 )
    {
        hAtos( dbuf, SR_QCD_ENT_FMT, 'Y', '3', pQcd->y3  );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pEnt->radius != pQcd->radius )
    {
        hAtos( dbuf, SR_QCD_PLONG_FMT, 'R', 'A', pQcd->radius  );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pEnt->width != pQcd->width )
    {
        hAtos( dbuf, SR_QCD_ENT_FMT, 'T', 'H', pQcd->width  );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pEnt->color != pQcd->color )
    {
        if( pQcd->color >= 0 )
        {
            hAtos( dbuf, SR_QCD_PLONG_FMT, 'F', 'C', pQcd->color  );
        }
        else
        {
            hAtos( dbuf, SR_QCD_NLONG_FMT, 'F', 'C', -pQcd->color  );
        }
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pEnt->fontsize != pQcd->fontsize )
    {
        hAtos( dbuf, SR_QCD_ENT_FMT, 'F', 'S', pQcd->fontsize  );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pEnt->bold != pQcd->bold )
    {
        SetFlag( buf, pQcd->bold );
        hAtos( dbuf, SR_QCD_STR_FMT, 'F', 'B', buf  );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pEnt->italic != pQcd->italic )
    {
        SetFlag( buf, pQcd->italic );
        hAtos( dbuf, SR_QCD_STR_FMT, 'F', 'I', buf  );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pEnt->underline != pQcd->underline )
    {
        SetFlag( buf, pQcd->underline );
        hAtos( dbuf, SR_QCD_STR_FMT, 'F', 'U', buf  );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( p_scmp( pEnt->fontname, pQcd->fontname ) != 0 )
    {
        hAtos( dbuf, SR_QCD_STR_FMT, 'F', 'N', pQcd->fontname );
        p_write( pcb, dbuf, p_slen( dbuf ) );
    }
    if( pQcd->textflag )
    {
        hAtos( dbuf, SR_QCD_STR_FMT, 'T', 'X', pQcd->text );
        p_write( pcb, dbuf, p_slen( dbuf ) );
        pQcd->textflag = FALSE;
    }
    *pEnt = *pQcd;
}

static VOID WriteLine( PR_XCHANGE* self, VOID* pcb, A_PT* pt )
{
    QCD_ENTRY qcd;

    qcd = self->xchange.entry;
    qcd.type = QCD_LINE;
    qcd.x1 = GetDim( (LONG) pt[0].x - Drg->vec.Page.pos.x, self->xchange.res );
    qcd.x2 = GetDim( (LONG) pt[1].x - Drg->vec.Page.pos.x, self->xchange.res );
    qcd.y1 = GetDim( (LONG) pt[0].y - Drg->vec.Page.pos.y, self->xchange.res );
    qcd.y2 = GetDim( (LONG) pt[1].y - Drg->vec.Page.pos.y, self->xchange.res );
    qcd.width = 2;
    WriteQcd( self, pcb, &qcd );
}


static VOID WritePolyline( PR_XCHANGE* self, VOID* pcb, ELEM* pEl )
{
    INT num;
    INT i;
    A_PT* pt;

    num = DATASIZE( pEl );
    pt = &pEl->pline.beg;
    for( i = 0 ; i < num - 1 ; i++ )
    {
        WriteLine( self, pcb, &pt[i] );
    }
}

static VOID WritePolygon( PR_XCHANGE* self, VOID* pcb, ELEM* pEl )
{
    INT size;
    A_PT line[2];

    size = DATASIZE( pEl );
    line[0] = pEl->pline.beg;
    line[1] = pEl->pline.pt[size-2];
    WriteLine( self, pcb, line );
    WritePolyline( self, pcb, pEl );
}

static VOID SetArc( QCD_ENTRY* qcd, EL_3PT* arc, UINT res )
{
    qcd->type = QCD_ARC;
    qcd->x1 = GetDim( arc->beg.x - Drg->vec.Page.pos.x, res );
    qcd->x2 = GetDim( arc->mid.x - Drg->vec.Page.pos.x, res );
    qcd->x3 = GetDim( arc->end.x - Drg->vec.Page.pos.x, res );
    qcd->y1 = GetDim( arc->beg.y - Drg->vec.Page.pos.y, res );
    qcd->y2 = GetDim( arc->mid.y - Drg->vec.Page.pos.y, res );
    qcd->y3 = GetDim( arc->end.y - Drg->vec.Page.pos.y, res );
    qcd->width = 2;
}

static VOID Write3PtArc( PR_XCHANGE* self, VOID* pcb, ELEM* pEl )
{
    QCD_ENTRY qcd;
    EL_LARC larc;

    qcd = self->xchange.entry;
    SetArc( &qcd, &pEl->el3pt, self->xchange.res );
    p_send4( Drg, O_DG_3PT_TO_LARC, &larc, pEl );
    qcd.radius = GetLDim( larc.radius, self->xchange.res );
    WriteQcd( self, pcb, &qcd );
}

static VOID WriteArc( PR_XCHANGE* self, VOID* pcb, ELEM* pEl )
{
    QCD_ENTRY qcd;
    EL_LARC larc;
    EL_3PT arc;

    larc.hdr = pEl->hdr;
    larc.centre.x = pEl->arc.centre.x;
    larc.centre.y = pEl->arc.centre.y;
    larc.radius = pEl->arc.radius;
    larc.beg = pEl->arc.beg;
    larc.end = pEl->arc.end;
    p_send4( Drg, O_DG_LARC_TO_3PT, &arc, &larc );

    qcd = self->xchange.entry;
    SetArc( &qcd, &arc, self->xchange.res );
    qcd.radius = GetDim( pEl->arc.radius, self->xchange.res );
    WriteQcd( self, pcb, &qcd );
}

static VOID WriteCircle( PR_XCHANGE* self, VOID* pcb, ELEM* pEl )
{
    QCD_ENTRY qcd;
    EL_3PT arc;
    A_PT cpt;
    AUNIT rad;

    arc.hdr = pEl->hdr;
    cpt = pEl->circle.centre;
    rad = pEl->circle.radius;
    arc.beg.x = cpt.x - rad;
    arc.beg.y = cpt.y;
    arc.mid.x = cpt.x;
    arc.mid.y = cpt.y - rad;
    arc.end.x = cpt.x + rad;
    arc.end.y = cpt.y;

    qcd = self->xchange.entry;
    SetArc( &qcd, &arc, self->xchange.res );
    qcd.type = QCD_CIRCLE;
    qcd.radius = GetDim( rad, self->xchange.res );
    WriteQcd( self, pcb, &qcd );
}

static VOID WriteText( PR_XCHANGE* self, VOID* pcb, ELEM* pEl )
{
    QCD_ENTRY qcd;
    LONG mul;
    UINT div;

    qcd = self->xchange.entry;
    qcd.type = QCD_TEXT;

    mul = (LONG) pEl->text.set.smul * 2500 * 4;
    div = ( pEl->text.set.sdiv & ~FLIP_FLAG ) * 3;
    pEl->text.pos.y -= p_send4( Drg, O_DG_LONG_DIV_UINT, &mul, div );

    qcd.x1 = GetDim( pEl->text.pos.x - Drg->vec.Page.pos.x, self->xchange.res );
    qcd.y1 = GetDim( pEl->text.pos.y - Drg->vec.Page.pos.y, self->xchange.res );

    mul = (LONG) pEl->text.set.smul * 72;
    div = pEl->text.set.sdiv & ~FLIP_FLAG;
    qcd.fontsize = p_send4( Drg, O_DG_LONG_DIV_UINT, &mul, div );

    pEl->buf[pEl->buf[0]] = '\0';
    p_scpy( qcd.text, (TEXT*) pEl->text.text );

    qcd.textflag = TRUE;
    WriteQcd( self, pcb, &qcd );
}

static VOID WriteLink( PR_XCHANGE* self, VOID* pcb, ELEM* pEl )
{
    LIB* pLib;
    int i;
    LONG pos;
    UWORD word;
    EL_LINK link;
    EL_SYMBOLHDR sym;

    if( pEl->link.sym == 0xff )
    {
        /*DrawLinkBox( &pEl->link );*/
        return;
    }
    link = pEl->link;
    pLib = &Drg->drg.LibList[ link.lib ];

    if( pLib->pfcb == NULL )
    {
        p_send3( Drg, O_DG_OPEN_LIB, pLib );
        if( pLib->pfcb == NULL )
        {
            /*DrawLinkBox( &link );*/
            return;
        }
    }

    pos = pLib->symlist + link.sym * sizeof(UWORD);
    f_seek( pLib->pfcb, P_FABS, &pos );
    f_read( pLib->pfcb, &word, sizeof(UWORD) );
    pos = pLib->data + word;
    f_seek( pLib->pfcb, P_FABS, &pos );
    f_read( pLib->pfcb, &sym, sizeof(EL_SYMBOLHDR) );
    for( i = sym.grp.count ; i > 0 ; --i )
    {
        f_read( pLib->pfcb, pEl, 1 );
        f_read( pLib->pfcb, &pEl->buf[1], pEl->buf[0] - 1 );
        /*if( pEl->hdr.type == V_GROUP )*/
        /*{*/
            /*i += pEl->grp.count;*/
            /*continue;*/
        /*}*/
        p_send5( Drg, O_DG_ADJUST_LKEL, pEl, &link, &sym.hot );
        WriteElem( self, pcb, pEl );
    }
}

static VOID WriteElem( PR_XCHANGE* self, VOID* pcb, ELEM* pEl )
{
    switch( pEl->hdr.type )
    {
    case V_LINE:
        WriteLine( self, pcb, &pEl->line.beg );
        break;
    case V_POLYLINE:
        WritePolyline( self, pcb, pEl );
        break;
    case V_POLYGON:
        WritePolygon( self, pcb, pEl );
        break;
    case V_3PT_ARC:
        Write3PtArc( self, pcb, pEl );
        break;
    case V_ARC:
        WriteArc( self, pcb, pEl );
        break;
    case V_CIRCLE:
        WriteCircle( self, pcb, pEl );
        break;
    case V_TEXT:
        WriteText( self, pcb, pEl );
        break;
    case V_LINK:
        WriteLink( self, pcb, pEl );
        break;
    }
}

/***************************************************************************
 **  xch_export  Export the given file. Calls p_leave if error occurs.
 **  ~~~~~~~~~~
 */

#pragma METHOD_CALL

METHOD VOID xchange_xch_export( PR_XCHANGE* self, EXPORT_DATA_QCD* data )
{
    VOID* pcb;
    INT ret;
    TEXT* fbuf;
    UINT hand;

    wInfoMsg( data->fname );

    self->xchange.res = data->res;
    ret = p_open( &pcb, data->fname, P_FTEXT | P_FUPDATE | P_FREPLACE );
    if( ret ) p_leave( ret );
    fbuf = (TEXT*) p_send2( Drg, O_DG_GET_FBUF );
    hLoadResBuf( SR_QCD_FILEID1, fbuf );
    p_write( pcb, fbuf, p_slen( fbuf ) );
    hAtos( fbuf, SR_QCD_FILEID2, data->res );
    p_write( pcb, fbuf, p_slen( fbuf ) );

    p_send2( Data, O_DD_REWIND );
    for(;;)
    {
        hand = p_send3( Data, O_DD_NEXT, fbuf );
        if( hand == EL_EOD ) break;
        /* Yield to allow dialogs to update */
        /*p_send2( idle, O_AO_QUEUE );*/
        /*p_send2( w_am, O_AM_START );*/
        /*if( self->plot.cancel ) break;*/
        /*if( fbuf[TYPE_BYTE] & V_AGGR_BIT ) continue;*/
        if( Drg->vec.LayMask & LayerBit( fbuf[LAYER_BYTE] ) )
        {
            WriteElem( self, pcb, (ELEM*) fbuf );
        }
    }
    p_close( pcb );
}


/* End of VecQcdIm.c file */
