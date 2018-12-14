/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR PSION EPOC16          *  Written by: Nick Matthews  *
 *  Module: DXF DYL EXPORT FUNCTION          *  Date Started: 26 Aug 1998  *
 *    File: VecDxfEx.C      Type: C SOURCE   *  Date Revised: 26 Aug 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1998 Nick Matthews
*/

#include <hwim.h>
#include <p_math.h>
#include <vecdxf.g>
#include <vector.g>
#include <vector.rsg>
#include "vec.h"

#define LayerBit(L) ( 1 << ( (L) & 0x0f ) )
#define DATASIZE(H) ( ( ((UBYTE*)(H))[SIZE_BYTE] - sizeof(ELHDR) ) / sizeof(A_PT) )
#define LAYERS_MAX 16
#define WriteAun(A,B,C) WriteDim( (A), (B), (LAUNIT) (C) )
static VOID WriteElem( PR_XCHANGE* self, ELEM* pEl );


/***************************************************************************
 **  LaunchDialog  Launch a dialog box and return the result
 **  ~~~~~~~~~~~~
 */

INT LaunchDialog( VOID* buf, INT id, INT class )
{
    DL_DATA dial_data;

    dial_data.id = id;
    dial_data.rbuf = buf;
    dial_data.pdlg = NULL;
    return hLaunchDial( CAT_VECDXF_VECDXF, class, &dial_data );
}

/***************************************************************************
 **  MakeLinkName  Create a unique name for symbol link. Text buf must be at
 **  ~~~~~~~~~~~~  least LINK_NAME_MAX_Z in size.
 */

static VOID MakeLinkName( TEXT* buf, ELEM* pEl )
{
    INT lib;

    p_bfil( buf, LINK_NAME_MAX_Z, '\0' );
    lib = pEl->link.lib;
    if( lib >= Drg->drg.LibCount )
    {
        p_leave( CORRUPT_FILE );
    }
    hAtos( buf, SR_DXF_LNK_NAME_FMT, Drg->drg.LibList[lib].fname, pEl->link.ref );
}

/***************************************************************************
 **  WriteDxfGroup  Write the DXF group name to file, checking to see if the
 **  ~~~~~~~~~~~~~  user has pressed Esc.
 */

static VOID WriteDxfGroup( PR_XCHANGE* self, UINT grp )
{
    TEXT buf[10];

    p_send2( self->xchange.idle, O_AO_QUEUE );
    p_send2( w_am, O_AM_START );
    if( self->xchange.cancel ) p_leave( USER_ABORT );

    hAtos( buf, SR_DXF_GROUP_FMT, grp );
    p_write( self->xchange.pcb, buf, p_slen( buf ) );
}

/***************************************************************************
 **  WriteDim  Write a dimension group to file, converting to the correct
 **  ~~~~~~~~  units.
 */

static VOID WriteDim( PR_XCHANGE* self, UINT grp, LAUNIT lau )
{
    TEXT buf[40];
    INT ret;

    WriteDxfGroup( self, grp );
    ret = p_send4( Drg, O_DG_LAU_TO_S_TEXT, buf, &lau );
    if( ret != 0 )
    {
        hAtos( buf, SR_DXF_INT_FMT, 0 );
    }
    p_write( self->xchange.pcb, buf, p_slen( buf ) );
}

/***************************************************************************
 **  WriteRes  Write a text group to file, using a string from the
 **  ~~~~~~~~  resource file.
 */

static VOID WriteRes( PR_XCHANGE* self, UINT grp, UINT id )
{
    TEXT buf[20];

    WriteDxfGroup( self, grp );

    hLoadResBuf( id, buf );
    p_write( self->xchange.pcb, buf, p_slen( buf ) );
}

/***************************************************************************
 **  WriteTxt  Write a text group to file, using the given string.
 **  ~~~~~~~~
 */

static VOID WriteTxt( PR_XCHANGE* self, UINT grp, TEXT* txt )
{
    WriteDxfGroup( self, grp );

    p_write( self->xchange.pcb, txt, p_slen( txt ) );
}

/***************************************************************************
 **  WriteInt  Write a integer group to file.
 **  ~~~~~~~~
 */

static VOID WriteInt( PR_XCHANGE* self, UINT grp, INT i )
{
    TEXT buf[20];

    WriteDxfGroup( self, grp );

    hAtos( buf, SR_DXF_INT_FMT, i );
    p_write( self->xchange.pcb, buf, p_slen( buf ) );
}

/***************************************************************************
 **  WriteLng  Write a long integer group to file.
 **  ~~~~~~~~
 */

static VOID WriteLng( PR_XCHANGE* self, UINT grp, LONG lng )
{
    TEXT buf[20];

    WriteDxfGroup( self, grp );

    hAtos( buf, SR_DXF_LONG_FMT, lng );
    p_write( self->xchange.pcb, buf, p_slen( buf ) );
}

/***************************************************************************
 **  WriteDbl  Write a floating point group to file, using fixed format to
 **  ~~~~~~~~  2 decimal places.
 */

static VOID WriteDbl( PR_XCHANGE* self, UINT grp, DOUBLE* pDbl )
{
    TEXT buf[40];
    P_DTOB dtob;
    INT ret;

    WriteDxfGroup( self, grp );

    dtob.type = P_DTOB_FIXED;
    dtob.width = 20;
    dtob.ndec = 2;
    dtob.point = '.';
    dtob.triad = ' ';
    dtob.trilen = 0;
    ret = p_dtob( buf, pDbl, &dtob );
    if( ret > 3 )
    {
        if( buf[ret-1] == '0' )
        {
            --ret;
            if( buf[ret-1] == '0' )
            {
                ret -= 2;
            }
        }
        buf[ret] = '\0';
        p_write( self->xchange.pcb, buf, p_slen( buf ) );
    }
}

/***************************************************************************
 **  WriteApt  Write an absolute point to file as 2 groups.
 **  ~~~~~~~~
 */

static VOID WriteApt( PR_XCHANGE* self, UINT grp1, UINT grp2, A_PT* pPt )
{
    LAUNIT lu;

    lu = (LAUNIT) pPt->x - Drg->vec.Orig.x;
    WriteDim( self, grp1, lu );

    lu = (LAUNIT) Drg->vec.Orig.y - pPt->y;
    WriteDim( self, grp2, lu );
}

/***************************************************************************
 **  WriteLpt  Write a long absolute point to file as 2 groups.
 **  ~~~~~~~~
 */

static VOID WriteLpt( PR_XCHANGE* self, UINT grp1, UINT grp2, LA_PT* pPt )
{
    LAUNIT lu;

    lu = pPt->x - Drg->vec.Orig.x;
    WriteDim( self, grp1, lu );

    lu = Drg->vec.Orig.y - pPt->y;
    WriteDim( self, grp2, lu );
}

/***************************************************************************
 **  WriteAngle  Write an angle group to file
 **  ~~~~~~~~~~
 */

static VOID WriteAngle( PR_XCHANGE* self, UINT grp, ANGLE* pAng )
{
    DOUBLE dbl;
    ANGLE ang;

    ang.sin = -pAng->sin;
    ang.cos = pAng->cos;
    p_send4( Drg, O_DG_GET_ANGLE_DBL, &dbl, &ang );
    WriteDbl( self, grp, &dbl );
}

/***************************************************************************
 **  WriteScale  Write a scale group to file, from a multiplier and divisor.
 **  ~~~~~~~~~~
 */

static VOID WriteScale( PR_XCHANGE* self, UINT grp, UWORD mul, UWORD div )
{
    DOUBLE dbl, tmp;
    LONG val;
    TEXT buf[40];
    P_DTOB dtob;
    INT ret;

    val = mul;
    p_longtof( &dbl, &val );
    val = div;
    p_longtof( &dbl, &val );
    p_fdiv( &dbl, &tmp );

    WriteDxfGroup( self, grp );

    dtob.type = P_DTOB_GENERAL;
    dtob.width = 20;
    dtob.point = '.';

    ret = p_dtob( buf, &dbl, &dtob );
    if( ret > 0 )
    {
        buf[ret] = '\0';
    }
    else
    {
        hAtos( buf, SR_DXF_INT_FMT, 0 );
    }
    p_write( self->xchange.pcb, buf, p_slen( buf ) );
}

/***************************************************************************
 **  WriteTxtHt  Write a scale group to file, from a LKSET.
 **  ~~~~~~~~~~
 */

static VOID WriteTxtHt( PR_XCHANGE* self, UINT grp, LKSET* pSet )
{
    LONG mul, div, height;

    mul = 2540L * pSet->smul;
    div = pSet->sdiv & ~FLIP_FLAG;
    p_send5( Drg, O_DG_L_DIV_L_EQ_L, &height, &mul, &div );
    WriteDim( self, grp, height );
}

/***************************************************************************
 **  WriteGrpNam  Write a block name for an element group.
 **  ~~~~~~~~~~~
 */

static VOID WriteGrpNam( PR_XCHANGE* self, UINT grp, INT ch, UINT num )
{
    TEXT buf[20];

    WriteDxfGroup( self, grp );

    hAtos( buf, SR_DXF_GRP_NAME_FMT, ch, num );
    p_write( self->xchange.pcb, buf, p_slen( buf ) );
}

/***************************************************************************
 **  WriteUName  Write a user name as a text group.
 **  ~~~~~~~~~~
 */

static VOID WriteUName( PR_XCHANGE* self, UINT grp, TEXT* uname )
{
    TEXT buf[UNAME_MAX_Z];
    INT i;

    WriteDxfGroup( self, grp );

    for( i = 0 ; i < UNAME_MAX ; i++ )
    {
        if( uname[i] == ' ' )
        {
            buf[i] = '_';
        }
        else
        {
            buf[i] = uname[i];
        }
    }
    buf[UNAME_MAX] = '\0';
    p_write( self->xchange.pcb, buf, p_slen( buf ) );
}

/***************************************************************************
 **  WriteLayer  Write a layer name from the given layer index.
 **  ~~~~~~~~~~
 */

static VOID WriteLayer( PR_XCHANGE* self, UINT grp, UINT layer )
{
    TEXT buf[UNAME_MAX_Z];

    p_send4( Drg, O_DG_GET_LAYER_STR, buf, layer & ~LAYER_FLAGS );
    WriteUName( self, grp, buf );
}

/***************************************************************************
 **  WriteLine  Write a line element.
 **  ~~~~~~~~~
 */

static VOID WriteLine( PR_XCHANGE* self, ELEM* pEl )
{
    WriteRes( self, 0, SR_DXF_LINE );
    WriteLayer( self, 8, pEl->hdr.layer );
    WriteApt( self, 10, 20, &pEl->line.beg );
    WriteApt( self, 11, 21, &pEl->line.end );
}


/***************************************************************************
 **  WritePolyline  Write a polygon (if polygon=TRUE) or polyline element.
 **  ~~~~~~~~~~~~~
 */

static VOID WritePolyline( PR_XCHANGE* self, ELEM* pEl, INT polygon )
{
    INT num;
    INT i;
    A_PT* pt;

    WriteRes( self, 0, SR_DXF_POLYLINE );
    WriteLayer( self, 8, pEl->hdr.layer );
    WriteInt( self, 66, 1 );
    if( polygon )
    {
        WriteInt( self, 70, 1 );
    }

    num = DATASIZE( pEl );
    pt = &pEl->pline.beg;
    for( i = 0 ; i < num ; i++ )
    {
        WriteRes( self, 0, SR_DXF_VERTEX );
        WriteLayer( self, 8, pEl->hdr.layer );
        WriteApt( self, 10, 20, &pt[i] );
    }
    WriteRes( self, 0, SR_DXF_SEQEND );
    WriteLayer( self, 8, pEl->hdr.layer );
}

/***************************************************************************
 **  WriteCircle  Write a circle element.
 **  ~~~~~~~~~~~
 */

static VOID WriteCircle( PR_XCHANGE* self, ELEM* pEl )
{
    WriteRes( self, 0, SR_DXF_CIRCLE );
    WriteLayer( self, 8, pEl->hdr.layer );
    WriteApt( self, 10, 20, &pEl->circle.centre );
    WriteDim( self, 40, (LAUNIT) pEl->circle.radius );
}

/***************************************************************************
 **  Write3PtArc  Write a 3 pt arc element, converting it to a centre arc
 **  ~~~~~~~~~~~  first.
 */

static VOID Write3PtArc( PR_XCHANGE* self, ELEM* pEl )
{
    EL_LARC larc;

    p_send4( Drg, O_DG_3PT_TO_LARC, &larc, pEl );
    WriteRes( self, 0, SR_DXF_ARC );
    WriteLayer( self, 8, pEl->hdr.layer );
    WriteLpt( self, 10, 20, &larc.centre );
    WriteDim( self, 40, larc.radius );
    WriteAngle( self, 50, &larc.end );
    WriteAngle( self, 51, &larc.beg );
}

/***************************************************************************
 **  WriteArc  Write an arc element.
 **  ~~~~~~~~
 */

static VOID WriteArc( PR_XCHANGE* self, ELEM* pEl )
{
    WriteRes( self, 0, SR_DXF_ARC );
    WriteLayer( self, 8, pEl->hdr.layer );
    WriteApt( self, 10, 20, &pEl->arc.centre );
    WriteDim( self, 40, (LAUNIT) pEl->arc.radius );
    WriteAngle( self, 50, &pEl->arc.end );
    WriteAngle( self, 51, &pEl->arc.beg );
}

/***************************************************************************
 **  WriteText  Write a text element.
 **  ~~~~~~~~~
 */

static VOID WriteText( PR_XCHANGE* self, ELEM* pEl )
{
    WriteRes( self, 0, SR_DXF_TEXT );
    WriteLayer( self, 8, pEl->hdr.layer );
    WriteApt( self, 10, 20, &pEl->text.pos );
    WriteTxtHt( self, 40, &pEl->text.set );
    pEl->buf[pEl->buf[0]] = '\0'; /* Terminate string */
    WriteTxt( self, 1, (TEXT*) pEl->text.text );
}

/***************************************************************************
 **  WriteInsert  Write an insert (aggregate) element.
 **  ~~~~~~~~~~~
 */

static VOID WriteInsert( PR_XCHANGE* self, ELEM* pEl, INT ch )
{
    WriteRes( self, 0, SR_DXF_INSERT );
    WriteGrpNam( self, 2, ch, pEl->sym.ref );
    WriteLayer( self, 8, 0 );
    WriteInt( self, 10, 0 );
    WriteInt( self, 20, 0 );
}

/***************************************************************************
 **  WriteElem  Write the give elememnt to file
 **  ~~~~~~~~~
 */

static VOID WriteElem( PR_XCHANGE* self, ELEM* pEl )
{
    switch( pEl->hdr.type )
    {
    case V_LINE:
        WriteLine( self, pEl );
        break;
    case V_POLYLINE:
        WritePolyline( self, pEl, FALSE );
        break;
    case V_POLYGON:
        WritePolyline( self, pEl, TRUE );
        break;
    case V_3PT_ARC:
        Write3PtArc( self, pEl );
        break;
    case V_ARC:
        WriteArc( self, pEl );
        break;
    case V_CIRCLE:
        WriteCircle( self, pEl );
        break;
    case V_TEXT:
        WriteText( self, pEl );
        break;
    case V_LINK:
        WriteInsert( self, pEl, 'L' );
        break;
    case V_GROUP:
        WriteInsert( self, pEl, 'G' );
        break;
    case V_SYMBOL:
        WriteInsert( self, pEl, 'S' );
        break;
    case V_CHARACTER:
        WriteInsert( self, pEl, 'C' );
        break;
    default:
        return;
    }
    self->xchange.total++;
    p_send4( DatDialogPtr, O_DXFDL_UPDATE, SR_DXF_EXP_UPDATE_FMT, self->xchange.total );
}


/***************************************************************************
 **  WriteLayerTable
 **  ~~~~~~~~~~~~~~~
 */

static VOID WriteLayerTable( PR_XCHANGE* self )
{
    INT pen, i;

    WriteRes( self, 0, SR_DXF_TABLE );
    WriteRes( self, 2, SR_DXF_LAYER );
    WriteInt( self, 70, LAYERS_MAX );

    for( i = 0 ; i < LAYERS_MAX ; i++ )
    {
        WriteRes( self, 0, SR_DXF_LAYER );
        WriteLayer( self, 2, i );
        WriteInt( self, 70, 0 );
        pen = ( Drg->vec.LayMask & LayerBit( i ) ) ? 7 : -7;
        WriteInt( self, 62, pen );
        WriteRes( self, 6, SR_DXF_CONTINUOUS );
    }

    WriteRes( self, 0, SR_DXF_ENDTAB );
}

/***************************************************************************
 **  WriteStyleTable
 **  ~~~~~~~~~~~~~~~
 */

static VOID WriteStyleTable( PR_XCHANGE* self )
{
    INT i;
    TSTYLE* pTStyle;

    WriteRes( self, 0, SR_DXF_TABLE );
    WriteRes( self, 2, SR_DXF_STYLE );
    WriteInt( self, 70, Drg->drg.TextSize );

    for( i = 0 ; i < Drg->drg.TextSize ; i++ )
    {
        pTStyle = &Drg->drg.TextList[i];

        WriteRes( self, 0, SR_DXF_STYLE );
        WriteUName( self, 2, (TEXT*) pTStyle->name );
        WriteInt( self, 70, 0 );
        WriteTxtHt( self, 40, &pTStyle->set );   /* Height */
        WriteInt( self, 41, 1 );                 /* Width */
        WriteInt( self, 50, 0 );                 /* Oblique angle */
        WriteInt( self, 71, 0 );                 /* Flag (fliped) */
        WriteTxtHt( self, 42, &pTStyle->set );   /* Last Height */
        WriteTxt( self, 3, Drg->drg.FontList[pTStyle->font].fname ); /* Font file name */
        WriteRes( self, 4, SR_BLANK );           /* Big-font file name */
    }

    WriteRes( self, 0, SR_DXF_ENDTAB );
}

/***************************************************************************
 **  ScanEntities  Scans store, writing to file. Aggregate elements are
 **  ~~~~~~~~~~~~  entered as INSERT objects and their elements ignored.
 */

static VOID ScanEntities( PR_XCHANGE* self, ELEM* pEl, UINT maxel )
{
    UINT hand;
    INT c, m;

    for( m = maxel ; m > 0 ; --m )
    {
        hand = p_send3( Data, O_DD_NEXT, pEl );
        if( hand == EL_EOD ) break;

        if( pEl->hdr.type == V_GROUP || pEl->hdr.type == V_LINK )
        {
            pEl->sym.ref = hand;
        }
        WriteElem( self, pEl );

        /* Skip Aggregate elements */
        if( pEl->hdr.type & V_AGGR_BIT )
        {
            for( c = pEl->grp.count ; c > 0 ; --c )
            {
                hand = p_send3( Data, O_DD_NEXT, pEl );
                if( hand == EL_EOD ) break;
                if( pEl->hdr.type & V_AGGR_BIT )
                {
                    c += pEl->grp.count;
                }
            }
        }
    }
}

/***************************************************************************
 **  ScanBlocks  Reentrant code. Scans store looking for aggregate elements.
 **  ~~~~~~~~~~  Only returns when there are no aggregates or they have all
 **  been written to file.
 */

static VOID ScanBlocks( PR_XCHANGE* self, ELEM* pEl, UINT maxel )
{
    INT m;
    UINT hand;

    for( m = maxel ; m > 0 ; --m )
    {
        hand = p_send3( Data, O_DD_NEXT, pEl );
        if( hand == EL_EOD ) break;
        if( pEl->hdr.type & V_AGGR_BIT )
        {
            ScanBlocks( self, pEl, pEl->grp.count );
            p_send4( Data, O_DD_READ, hand, pEl );

            WriteRes( self, 0, SR_DXF_BLOCK );
            WriteLayer( self, 8, 0 );
            switch( pEl->grp.type )
            {
            case V_GROUP:
                WriteGrpNam( self, 2, 'G', hand );
                break;
            case V_SYMBOL:
                WriteGrpNam( self, 2, 'S', pEl->sym.ref );
                break;
            case V_CHARACTER:
                WriteGrpNam( self, 2, 'C', pEl->sym.ref );
                break;
            }
            WriteInt( self, 70, 0 );
            WriteInt( self, 10, 0 );
            WriteInt( self, 20, 0 );

            ScanEntities( self, pEl, pEl->grp.count );

            WriteRes( self, 0, SR_DXF_ENDBLK );
            WriteLayer( self, 8, 0 );
        }
    }
}

/***************************************************************************
 **  WriteLinkBlock
 **  ~~~~~~~~~~~~~~
 */

static VOID WriteLinkBlock( PR_XCHANGE* self, ELEM* pEl, UINT hand )
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

    WriteRes( self, 0, SR_DXF_BLOCK );
    WriteLayer( self, 8, 0 );
    WriteGrpNam( self, 2, 'L', hand );
    WriteInt( self, 70, 0 );
    WriteInt( self, 10, 0 );
    WriteInt( self, 20, 0 );

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
        if( pEl->hdr.type == V_GROUP )
        {
            i += pEl->grp.count;
            continue;
        }
        p_send5( Drg, O_DG_ADJUST_LKEL, pEl, &link, &sym.hot );
        pEl->hdr.layer = link.hdr.layer;
        WriteElem( self, pEl );
    }

    WriteRes( self, 0, SR_DXF_ENDBLK );
    WriteLayer( self, 8, 0 );
}

/***************************************************************************
 **  ScanLinks  Scans store looking for Symbol Links, then writes symbol as
 **  ~~~~~~~~~  a block.
 */

static VOID ScanLinks( PR_XCHANGE* self, ELEM* pEl )
{
    UINT hand;

    p_send2( Data, O_DD_REWIND );
    for(;;)
    {
        hand = p_send3( Data, O_DD_NEXT, pEl );
        if( hand == EL_EOD ) break;
        if( pEl->hdr.type == V_LINK )
        {
            WriteLinkBlock( self, pEl, hand );
        }
    }
}

/***************************************************************************
 **  WriteTopRight  Write Top Right of page as groups 10 & 20.
 **  ~~~~~~~~~~~~~
 */

static VOID WriteTopRight( PR_XCHANGE* self )
{
    A_PT pt;

    pt.x = Drg->vec.Page.lim.x;
    pt.y = Drg->vec.Page.pos.y;
    WriteApt( self, 10, 20, &pt );
}

/***************************************************************************
 **  WriteBottomLeft  Write Bottom Left of page as groups 10 & 20.
 **  ~~~~~~~~~~~~~~~
 */

static VOID WriteBottomLeft( PR_XCHANGE* self )
{
    A_PT pt;

    pt.x = Drg->vec.Page.pos.x;
    pt.y = Drg->vec.Page.lim.y;
    WriteApt( self, 10, 20, &pt );
}

/***************************************************************************
 **  WriteHeader
 **  ~~~~~~~~~~~
 */

static VOID WriteHeader( PR_XCHANGE* self )
{
    UINT flag;

    p_send4( DatDialogPtr, O_DXFDL_UPDATE, SR_DXF_EXP_UPDATE_HDR, 0 );

    /* Version header */
    WriteRes( self, 9, SR_DXF_H_ACADVER );
    WriteRes( self, 1, SR_DXF_ACADVER );

    /* Drawing limits */
    WriteRes( self, 9, SR_DXF_H_EXTMIN );
    WriteBottomLeft( self );
    WriteRes( self, 9, SR_DXF_H_EXTMAX );
    WriteTopRight( self );

    WriteRes( self, 9, SR_DXF_H_LIMMIN );
    WriteBottomLeft( self );
    WriteRes( self, 9, SR_DXF_H_LIMMAX );
    WriteTopRight( self );

    /* Snap */
    WriteRes( self, 9, SR_DXF_H_SNAPMODE );
    flag = ( Drg->vec.Pref.grid.flags & GRID_SNAP ) ? 1 : 0;
    WriteInt( self, 70, flag );
    WriteRes( self, 9, SR_DXF_H_SNAPUNIT );
    WriteDim( self, 10, Drg->vec.Pref.grid.space );
    WriteDim( self, 20, Drg->vec.Pref.grid.space );
    WriteRes( self, 9, SR_DXF_H_SNAPBASE );
    WriteInt( self, 10, 0 );
    WriteInt( self, 20, 0 );
    WriteRes( self, 9, SR_DXF_H_SNAPANG );
    WriteInt( self, 50, 0 );

    /* Grid */
    WriteRes( self, 9, SR_DXF_H_GRIDMODE );
    flag = ( Drg->vec.Pref.grid.flags & GRID_DISP ) ? 1 : 0;
    WriteInt( self, 70, flag );
    WriteRes( self, 9, SR_DXF_H_GRIDUNIT );
    WriteDim( self, 10, Drg->vec.Pref.grid.space );
    WriteDim( self, 20, Drg->vec.Pref.grid.space );

    /* Layer */
    WriteRes( self, 9, SR_DXF_H_CLAYER );
    WriteLayer( self, 8, Drg->vec.Layer );
}

/***************************************************************************
 **  xch_export  Export the given file. Calls p_leave if error occurs.
 **  ~~~~~~~~~~
 */

#pragma METHOD_CALL

METHOD INT xchange_xch_export( PR_XCHANGE* self, EXPORT_DATA_DXF* data )
{
    INT ret;
    TEXT* fbuf;
    UINT maxel;
    DL_DATA escdata;

    if( self->xchange.idle == NULL )
    {
        self->xchange.idle = f_newsend( CAT_VECDXF_OLIB, C_AIDLE, O_AO_INIT );
    }

    self->xchange.fname = data->fname;

    if( self->xchange.esc == NULL )
    {
        escdata.id = ESC_EXPORT_DIALOG;
        escdata.rbuf = &self->xchange.cancel;
        escdata.pdlg = &self->xchange.esc;
        hLaunchDial( CAT_VECDXF_VECDXF, C_DXFESCDLG, &escdata );
    }

    ret = p_open( &self->xchange.pcb, data->fname, P_FTEXT | P_FUPDATE | P_FREPLACE );
    if( ret ) p_leave( ret );

    fbuf = (TEXT*) p_send2( Drg, O_DG_GET_FBUF );
    maxel = p_send2( Data, O_DD_COUNT );

    WriteRes( self, 0, SR_DXF_SECTION );
    WriteRes( self, 2, SR_DXF_HEADER );
    WriteHeader( self );
    WriteRes( self, 0, SR_DXF_ENDSEC );

    WriteRes( self, 0, SR_DXF_SECTION );
    WriteRes( self, 2, SR_DXF_TABLES );
    WriteLayerTable( self );
    WriteStyleTable( self );
    WriteRes( self, 0, SR_DXF_ENDSEC );

    WriteRes( self, 0, SR_DXF_SECTION );
    WriteRes( self, 2, SR_DXF_BLOCKS );
    ScanLinks( self, (ELEM*) fbuf );
    p_send2( Data, O_DD_REWIND );
    ScanBlocks( self, (ELEM*) fbuf, maxel );
    WriteRes( self, 0, SR_DXF_ENDSEC );

    WriteRes( self, 0, SR_DXF_SECTION );
    WriteRes( self, 2, SR_DXF_ENTITIES );
    p_send2( Data, O_DD_REWIND );
    ScanEntities( self, (ELEM*) fbuf, maxel );
    WriteRes( self, 0, SR_DXF_ENDSEC );

    WriteRes( self, 0, SR_DXF_EOF );
    return 0;
}

/* End of VecDxfIm.c file */
