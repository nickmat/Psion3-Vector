/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR PSION EPOC16          *  Written by: Nick Matthews  *
 *  Module: DXF DYL IMPORT FUNCTION          *  Date Started: 26 Aug 1998  *
 *    File: VecDxfIm.C      Type: C SOURCE   *  Date Revised: 26 Aug 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1998 Nick Matthews
*/

#include <p_math.h>
#include <hwim.h>
#include <vecdxf.g>
#include <vector.g>
#include <vector.rsg>
#include "vec.h"

/*#define DBX4_Debug(A,B,C)    Debug( (A), (B), (C) )*/
/*#define DBX4_Trace(A,B,C)    Trace( self, (A), (B), (C) )*/
/*#define DBX4_SetTrace(F)     ( self->xchange.trace = (F) )*/
#define DBX4_Debug(A,B,C)
#define DBX4_Trace(A,B,C)
#define DBX4_SetTrace(F)

VOID Debug( INT n1, INT n2, TEXT* n3 )
{
    p_send5( Drg, O_DG_DEBUG, n1, n2, n3 );
}

VOID Trace( PR_XCHANGE* self, INT n1, INT n2, TEXT* n3 )
{
    if( self->xchange.trace )
    {
        p_send5( Drg, O_DG_DEBUG, n1, n2, n3 );
    }
}

#define ERR_DXF_FILE_FORMAT  INPUT_FILE_ERR

extern VOID ReadEntities( PR_XCHANGE* self );

/***************************************************************************
 **  stoui  Convert zero terminated string to a UINT.
 **  ~~~~~
 */

UINT stoui( TEXT* str )
{
    INT ret;
    WORD fmt[2];
    UINT result;

    fmt[0] = '%' + ( 'u' << 8 ); fmt[1] = '\0';
    ret = p_stoa( &str, (TEXT*) fmt, &result );
    if( ret )
    {
        p_leave( ERR_DXF_FILE_FORMAT );
    }
    return result;
}

/***************************************************************************
 **  stoi  Convert zero terminated string to a INT.
 **  ~~~~
 */

INT stoi( TEXT* str )
{
    INT ret;
    WORD fmt[2];
    INT result;

    fmt[0] = '%' + ( 'd' << 8 ); fmt[1] = '\0';
    ret = p_stoa( &str, (TEXT*) fmt, &result );
    if( ret )
    {
        p_leave( ERR_DXF_FILE_FORMAT );
    }
    return result;
}

/***************************************************************************
 **  ftoau  Convert DOUBLE to AUINT, rounding to nearest value.
 **  ~~~~~  If success returns 0 else returns 1 & places 0 in au.
 */

INT ftoau( AUNIT* au, DOUBLE* du )
{
    DOUBLE tmp1, tmp2;
    WORD wd;
    LONG lg;
    INT ret;

    wd = 1;
    p_itof( &tmp1, &wd );
    wd = 2;
    p_itof( &tmp2, &wd );
    p_fdiv( &tmp1, &tmp2 );
    p_fld( &tmp2, du );
    p_fadd( &tmp2, &tmp1 );
    ret = p_intl( &lg, &tmp2 );
    if( ret || lg < 0 || lg >= AA )
    {
        return 1;
    }
    *au = (AUNIT) lg;
    return 0;
}

static VOID InitConsts( PR_XCHANGE* self )
{
    DOUBLE dbl;
    WORD wd;

    /* deg_rad = asin(1) / 90 */
    wd = 1;
    p_itof( &dbl, &wd );
    p_asin( &self->xchange.deg_rad, &dbl );
    wd = 90;
    p_itof( &dbl, &wd );
    p_fdiv( &self->xchange.deg_rad, &dbl );
}

static VOID ProcessPt( PR_XCHANGE* self, D_PT* result, D_PT* target )
{
    DOUBLE dx, dy, tmp;

    /* dx = tx - hx */
    p_fld( &dx, &target->x );
    p_fsub( &dx, &self->xchange.hotspot.x );
    p_fmul( &dx, &self->xchange.xscale );

    /* dy = ty - hy */
    p_fld( &dy, &target->y );
    p_fsub( &dy, &self->xchange.hotspot.y );
    p_fmul( &dy, &self->xchange.yscale );

    /* rx = hx + dx * cos - dy * sin */
    p_fld( &tmp, &dy );
    p_fmul( &tmp, &self->xchange.sin );
    p_fld( &result->x, &dx );
    p_fmul( &result->x, &self->xchange.cos );
    p_fsub( &result->x, &tmp );
    p_fadd( &result->x, &self->xchange.hotspot.x );

    /* ry = hy + dy * cos + dx * sin */
    p_fld( &tmp, &dx );
    p_fmul( &tmp, &self->xchange.sin );
    p_fld( &result->y, &dy );
    p_fmul( &result->y, &self->xchange.cos );
    p_fadd( &result->y, &tmp );
    p_fadd( &result->y, &self->xchange.hotspot.y );

    /* position on drawing */
    p_fadd( &result->x, &self->xchange.blkpos.x );
    p_fadd( &result->y, &self->xchange.blkpos.y );
}

/***************************************************************************
 **  SetPt  Returns zero if success or pos value if out of range.
 **  ~~~~~
 */

INT SetPt( PR_XCHANGE* self, A_PT* aPt, D_PT* dPtTarg )
{
    D_PT dPt;
    DOUBLE dbl;
    INT ret;

    if( self->xchange.process )
    {
        ProcessPt( self, &dPt, dPtTarg );
    }
    else
    {
        p_fld( &dPt.x, &dPtTarg->x );
        p_fld( &dPt.y, &dPtTarg->y );
    }

    /* pt.x = ( offset.x + dxf.x ) / scale */
    p_fld( &dbl, &self->xchange.offset.x );
    p_fadd( &dbl, &dPt.x );
    p_fdiv( &dbl, &self->xchange.scale );
    ret = ftoau( &aPt->x, &dbl );

    /* pt.y = ( offset.y - dxf.y ) / scale */
    p_fld( &dbl, &self->xchange.offset.y );
    p_fsub( &dbl, &dPt.y );
    p_fdiv( &dbl, &self->xchange.scale );
    ret += ftoau( &aPt->y, &dbl );

    return ret;
}

/***************************************************************************
 **  SetDim  Scale and convert the DOUBLE to a AUNIT. Return zero if success
 **  ~~~~~~  or pos value if out of range.
 */

INT SetDim( PR_XCHANGE* self, AUNIT* au, DOUBLE* dbl )
{
    DOUBLE tmp;

    p_fld( &tmp, dbl );
    if( self->xchange.process )
    {
        p_fmul( &tmp, &self->xchange.xscale );
    }
    p_fdiv( &tmp, &self->xchange.scale );
    return ftoau( au, &tmp );
}

/***************************************************************************
 **  SetAng  Convert a DOUBLE to an ANGLE. Return zero if success or pos
 **  ~~~~~~  value if out of range.
 */

INT SetAng( PR_XCHANGE* self, ANGLE* pAng, DOUBLE* dbl )
{
    DOUBLE tmp;
    WORD wd;

    p_fadd( dbl, &self->xchange.angbase );
    if( self->xchange.angdir == 0 )
    {
        wd = 360;
        p_itof( &tmp, &wd );
        p_fneg( dbl );
        p_fadd( dbl, &tmp );
    }
    p_send4( Drg, O_DG_SET_ANGLE, pAng, dbl );
    return 0;
}

/***************************************************************************
 **  ReadEntry  Read a DXF group, placing the text into self->xchange.fbuf
 **  ~~~~~~~~~  and group into self->xchange.grp
 */

VOID ReadEntry( PR_XCHANGE* self )
{
    INT ret;
    TEXT* fbuf;

    fbuf = self->xchange.fbuf;

    do
    {
        ret = p_read( self->xchange.pcb, fbuf, EL_BUFSIZE - 1 );
        if( ret >= 0 && ret < EL_BUFSIZE )
        {
            fbuf[ret] = '\0';
        }
        else
        {
            p_leave( ret );
        }
        self->xchange.grp = stoui( fbuf );

        ret = p_read( self->xchange.pcb, fbuf, EL_BUFSIZE - 1 );
        if( ret >= 0 && ret < EL_BUFSIZE )
        {
            fbuf[ret] = '\0';
        }
        else
        {
            p_leave( ret );
        }
    } while( self->xchange.grp == 999 );

    p_send2( self->xchange.idle, O_AO_QUEUE );
    p_send2( w_am, O_AM_START );
    if( self->xchange.cancel ) p_leave( USER_ABORT );

    DBX4_Trace( 901, self->xchange.grp, self->xchange.fbuf );
}

/***************************************************************************
 **  IsGroupText  Return TRUE if the text in fbuf is equel to the resource
 **  ~~~~~~~~~~~  string given by the id.
 */

INT IsGroupText( PR_XCHANGE* self, UINT id )
{
    TEXT buf[20];

    hLoadResBuf( id, buf );
    if( p_scmpi( self->xchange.fbuf, buf ) == 0 )
    {
        return TRUE;
    }
    return FALSE;
}


VOID StoreDbl( DOUBLE* dbl, TEXT* text )
{
    p_stod( &text, dbl, '.' );
}

/***************************************************************************
 **  ReadEntryDetails  Fill out the inp struct with entries from the file
 **  ~~~~~~~~~~~~~~~~  until a 0 or 9 group entry is found.
 */

VOID ReadEntryDetails( PR_XCHANGE* self )
{
    TEXT* fbuf;

    fbuf = self->xchange.fbuf;
    p_bfil( &self->xchange.inp, sizeof(DXF_INPUT), '\0' );
    for(;;)
    {
        ReadEntry( self );
        switch( self->xchange.grp )
        {
        case 0:
        case 9:
            return;
        case 1:
            p_scpy( self->xchange.inp.txt1, fbuf );
            break;
        case 2:
            fbuf[MAX_NAME] = '\0';  /* Truncate name if necessary */
            p_scpy( self->xchange.inp.txt2, fbuf );
            break;
        case 7:
            fbuf[MAX_NAME] = '\0';
            p_scpy( self->xchange.inp.txt7, fbuf );
            break;
        case 8:
            fbuf[UNAME_MAX] = '\0';
            p_scpy( self->xchange.inp.txt8, fbuf );
            break;
        case 10:
            StoreDbl( &self->xchange.inp.p10_20.x, fbuf );
            break;
        case 11:
            StoreDbl( &self->xchange.inp.p11_21.x, fbuf );
            break;
        case 14:
            StoreDbl( &self->xchange.inp.p14_24.x, fbuf );
            break;
        case 15:
            StoreDbl( &self->xchange.inp.p15_25.x, fbuf );
            break;
        case 20:
            StoreDbl( &self->xchange.inp.p10_20.y, fbuf );
            break;
        case 21:
            StoreDbl( &self->xchange.inp.p11_21.y, fbuf );
            break;
        case 24:
            StoreDbl( &self->xchange.inp.p14_24.y, fbuf );
            break;
        case 25:
            StoreDbl( &self->xchange.inp.p15_25.y, fbuf );
            break;
        case 40:
            StoreDbl( &self->xchange.inp.dim40, fbuf );
            break;
        case 41:
            StoreDbl( &self->xchange.inp.dim41, fbuf );
            break;
        case 42:
            StoreDbl( &self->xchange.inp.dim42, fbuf );
            break;
        case 50:
            StoreDbl( &self->xchange.inp.ang50, fbuf );
            break;
        case 51:
            StoreDbl( &self->xchange.inp.ang51, fbuf );
            break;
        case 62:
            self->xchange.inp.pen62 = stoi( fbuf );
            break;
        case 66:
            self->xchange.inp.f66 = stoui( fbuf );
            break;
        case 70:
            self->xchange.inp.f70 = stoui( fbuf );
            break;
        case 71:
            self->xchange.inp.f71 = stoui( fbuf );
            break;
        case 72:
            self->xchange.inp.f72 = stoui( fbuf );
            break;
        case 73:
            self->xchange.inp.f73 = stoui( fbuf );
            break;
        case 75:
            self->xchange.inp.f75 = stoui( fbuf );
            break;
        case 76:
            self->xchange.inp.f76 = stoui( fbuf );
            break;
        }
    }
}

/***************************************************************************
 **  ReadDPt  Read in a point, without any scaling.
 **  ~~~~~~~
 */

VOID ReadDPt( PR_XCHANGE* self, D_PT* dpt )
{
    ReadEntryDetails( self );
    p_fld( &dpt->x, &self->xchange.inp.p10_20.x );
    p_fld( &dpt->y, &self->xchange.inp.p10_20.y );
}

/***************************************************************************
 **  GetLayer  Return the layer number, given the layer string
 **  ~~~~~~~~
 */

UINT GetLayer( PR_XCHANGE* self, TEXT* str )
{
    INT i;

    if( self->xchange.laytab == NULL )
    {
        return 0;
    }

    for( i = 0 ; i < self->xchange.laymax ; i++ )
    {
        if( p_scmpi( self->xchange.laytab[i].name, str ) == 0 )
        {
            return i & 0x000f;
        }
    }
    return 0;
}

/***************************************************************************
 **  AddData  Add element to store
 **  ~~~~~~~
 */

UINT AddData( PR_XCHANGE* self, VOID* pEl )
{
    UINT hand;
    INT total;

    hand = p_send3( Data, O_DD_ADD, pEl );
    self->xchange.count++;
    self->xchange.total++;
    total = self->xchange.total;
    /*if( ( total & 0x0007 ) == total )*/
    {
        p_send4( DatDialogPtr, O_DXFDL_UPDATE, SR_DXF_IMP_UPDATE_FMT, total );
    }
    return hand;
}

/***************************************************************************
 **  DelData  Remove an element from store
 **  ~~~~~~~
 */

VOID DelData( PR_XCHANGE* self, UINT hand )
{
    p_send3( Data, O_DD_ADD, hand );
    --self->xchange.count;
    --self->xchange.total;
}

/***************************************************************************
 **  ReadLine  Read in and create a LINE element
 **  ~~~~~~~~
 */

VOID ReadLine( PR_XCHANGE* self )
{
    EL_LINE* pLine;

    ReadEntryDetails( self );
    pLine = (EL_LINE*) DBuf;

    pLine->hdr.size = sizeof(EL_LINE);
    pLine->hdr.type = V_LINE;
    pLine->hdr.attr = 0;
    pLine->hdr.layer = GetLayer( self, self->xchange.inp.txt8 );
    SetPt( self, &pLine->beg, &self->xchange.inp.p10_20 );
    SetPt( self, &pLine->end, &self->xchange.inp.p11_21 );
    AddData( self, pLine );
}

/***************************************************************************
 **  ReadPolyline  Read in and create a POLYLINE or POLYGON element
 **  ~~~~~~~~~~~~
 */

VOID ReadPolyline( PR_XCHANGE* self )
{
    EL_PLINE* pPline;
    INT count;
    A_PT* pPt;

    ReadEntryDetails( self );
    pPline = (EL_PLINE*) DBuf;
    pPt = &pPline->beg;

    if( self->xchange.inp.f66 != 1 )
    {
        return; /* No vertices entities follow */
    }
    count = 0;
    if( self->xchange.inp.f70 & 0x0001 )
    {
        pPline->hdr.type = V_POLYGON;
    }
    else
    {
        pPline->hdr.type = V_POLYLINE;
    }
    pPline->hdr.attr = 0;
    pPline->hdr.layer = GetLayer( self, self->xchange.inp.txt8 );

    for(;;)
    {
        if( IsGroupText( self, SR_DXF_VERTEX ) )
        {
            if( count < POINTS_MAX )
            {
                ReadEntryDetails( self );
                SetPt( self, &pPt[count], &self->xchange.inp.p10_20 );
                count++;
            }
        }
        else if( IsGroupText( self, SR_DXF_SEQEND ) )
        {
            ReadEntryDetails( self );
            break;
        }
        else
        {
            p_leave( ERR_DXF_FILE_FORMAT );
        }
    }

    if( count < 2 )
    {
        return;
    }
    if( count == 2 )
    {
        pPline->hdr.type = V_LINE;
    }
    pPline->hdr.size = sizeof(ELHDR) + sizeof(A_PT) * count;
    AddData( self, pPline );
}

/***************************************************************************
 **  ReadCircle  Read in and create a CIRCLE element
 **  ~~~~~~~~~~
 */

VOID ReadCircle( PR_XCHANGE* self )
{
    EL_CIRCLE* pCircle;

    ReadEntryDetails( self );
    pCircle = (EL_CIRCLE*) DBuf;

    pCircle->hdr.size = sizeof(EL_CIRCLE);
    pCircle->hdr.type = V_CIRCLE;
    pCircle->hdr.attr = 0;
    pCircle->hdr.layer = GetLayer( self, self->xchange.inp.txt8 );
    SetPt( self, &pCircle->centre, &self->xchange.inp.p10_20 );
    SetDim( self, &pCircle->radius, &self->xchange.inp.dim40 );
    AddData( self, pCircle );
}

/***************************************************************************
 **  ReadArc  Read in and create an ARC (centre) element
 **  ~~~~~~~
 */

VOID ReadArc( PR_XCHANGE* self )
{
    EL_ARC* pArc;

    ReadEntryDetails( self );
    pArc = (EL_ARC*) DBuf;

    pArc->hdr.size = sizeof(EL_ARC);
    pArc->hdr.type = V_ARC;
    pArc->hdr.attr = 0;
    pArc->hdr.layer = GetLayer( self, self->xchange.inp.txt8 );
    SetPt( self, &pArc->centre, &self->xchange.inp.p10_20 );
    SetDim( self, &pArc->radius, &self->xchange.inp.dim40 );
    if( self->xchange.angdir == 0 )
    {
        SetAng( self, &pArc->end, &self->xchange.inp.ang50 );
        SetAng( self, &pArc->beg, &self->xchange.inp.ang51 );
    }
    else
    {
        SetAng( self, &pArc->beg, &self->xchange.inp.ang50 );
        SetAng( self, &pArc->end, &self->xchange.inp.ang51 );
    }
    AddData( self, pArc );
}

/***************************************************************************
 **  ReadText  Read in and create a TEXT element
 **  ~~~~~~~~
 */

VOID ReadText( PR_XCHANGE* self )
{
    EL_TEXT* pText;

    ReadEntryDetails( self );
    pText = (EL_TEXT*) DBuf;

    self->xchange.inp.txt1[BYTES_MAX] = '\0'; /* Truncate if necessary */
    pText->hdr.size = SIZEOF_TEXTHDR + p_slen( self->xchange.inp.txt1 );
    pText->hdr.type = V_TEXT;
    pText->hdr.attr = 0;
    pText->hdr.layer = GetLayer( self, self->xchange.inp.txt8 );
    SetPt( self, &pText->pos, &self->xchange.inp.p10_20 );

    pText->set.sdiv = 2540;
    SetDim( self, &pText->set.smul, &self->xchange.inp.dim40 );
    if( pText->set.sdiv == 0 )
    {
        return; /* This wont work */
    }
    SetAng( self, &pText->set.a, &self->xchange.inp.ang50 );
    pText->slant = 0;
    pText->font = 0;
    p_scpy( (TEXT*) pText->text, self->xchange.inp.txt1 );

    if( p_send3( Drg, O_DG_SET_TEXT_RECT, pText ) == 0 )
    {
        AddData( self, pText );
    }
}

/***************************************************************************
 **  RestoreFile  Position the file at the point given by a previous sense
 **  ~~~~~~~~~~~  call.
 */

VOID RestoreFile( PR_XCHANGE* self, LONG* pos )
{
    f_seek( self->xchange.pcb, P_FRSET, pos );
    p_read( self->xchange.pcb, self->xchange.fbuf, EL_BUFSIZE - 1 );
    ReadEntryDetails( self );
}

/***************************************************************************
 **  SetBlock  Position the file at the beginning of the block found from
 **  ~~~~~~~~  the given record number.
 */

VOID SetBlock( PR_XCHANGE* self, UINT rec )
{
    DXF_BLK blk;

    p_send4( self->xchange.blktab, O_VA_COPY, rec, &blk );
    RestoreFile( self, &blk.pos );
}

/***************************************************************************
 **  ReadInsert  Read in and create INSERT elements from block
 **  ~~~~~~~~~~
 */

VOID ReadInsert( PR_XCHANGE* self )
{
    LONG cpos;
    /*D_PT oopt;*/
    UINT rec;
    ELGRP grp;
    UINT hand;
    INT elcnt;
    DOUBLE zero, one;
    WORD wd;

    f_seek( self->xchange.pcb, P_FRSENSE, &cpos );
    ReadEntryDetails( self );

    p_fld( &self->xchange.blkpos.x, &self->xchange.inp.p10_20.x );
    p_fld( &self->xchange.blkpos.y, &self->xchange.inp.p10_20.y );

    p_fmul( &self->xchange.inp.ang50, &self->xchange.deg_rad );
    p_sin( &self->xchange.sin, &self->xchange.inp.ang50 );
    p_cos( &self->xchange.cos, &self->xchange.inp.ang50 );

    wd = 0;
    p_itof( &zero, &wd );
    wd = 1;
    p_itof( &one, &wd );
    if( p_fcmp( &zero, &self->xchange.inp.dim41 ) == 0 )
    {
        p_fld( &self->xchange.xscale, &one );
    }
    else
    {
        p_fld( &self->xchange.xscale, &self->xchange.inp.dim41 );
    }
    if( p_fcmp( &zero, &self->xchange.inp.dim42 ) == 0 )
    {
        p_fld( &self->xchange.yscale, &one );
    }
    else
    {
        p_fld( &self->xchange.yscale, &self->xchange.inp.dim42 );
    }

    self->xchange.process = TRUE;

    if( p_send4( self->xchange.blktab, O_VA_FINDISQ, self->xchange.inp.txt2, &rec ) == 0 )
    {
        SetBlock( self, rec );
        ReadEntryDetails( self );

        p_fld( &self->xchange.hotspot.x, &self->xchange.inp.p10_20.x );
        p_fld( &self->xchange.hotspot.y, &self->xchange.inp.p10_20.y );

        grp.size = sizeof(ELGRP);
        grp.type = V_GROUP;
        grp.count = 0;
        grp.flags = 0;
        hand = AddData( self, &grp );
        elcnt = self->xchange.count;

        ReadEntities( self );

        grp.count = self->xchange.count - elcnt;
        if( grp.count > 0 )
        {
            p_send4( Data, O_DD_REPLACE, hand, &grp );
            self->xchange.count -= grp.count;
        }
        else
        {
            DelData( self, hand );
        }

        RestoreFile( self, &cpos );
    }
    self->xchange.process = FALSE;
}

/***************************************************************************
 **  ScanSection  Read and ignore unknown section
 **  ~~~~~~~~~~~
 */

VOID ScanSection( PR_XCHANGE* self )
{
    p_send4( DatDialogPtr, O_DXFDL_UPDATE, SR_DXF_IMP_UPDATE_SKP, self->xchange.fbuf );
    for(;;)
    {
        ReadEntry( self );
        if( self->xchange.grp == 0 )
        {
            if( IsGroupText( self, SR_DXF_ENDSEC ) )
            {
                return;
            }
        }
    }
}


/***************************************************************************
 **  ReadEntities  Read entities section
 **  ~~~~~~~~~~~~
 */

VOID ReadEntities( PR_XCHANGE* self )
{
    for(;;)
    {
        DBX4_Trace( 301, self->xchange.grp, self->xchange.fbuf );
        if( self->xchange.grp == 0 )
        {
            if( IsGroupText( self, SR_DXF_LINE ) )
            {
                ReadLine( self );
            }
            else if( IsGroupText( self, SR_DXF_POLYLINE ) )
            {
                ReadPolyline( self );
            }
            else if( IsGroupText( self, SR_DXF_CIRCLE ) )
            {
                ReadCircle( self );
            }
            else if( IsGroupText( self, SR_DXF_ARC ) )
            {
                ReadArc( self );
            }
            else if( IsGroupText( self, SR_DXF_TEXT ) )
            {
                ReadText( self );
            }
            else if( IsGroupText( self, SR_DXF_INSERT ) )
            {
                ReadInsert( self );
            }
            else if( IsGroupText( self, SR_DXF_ENDBLK ) )
            {
                return;
            }
            else if( IsGroupText( self, SR_DXF_ENDSEC ) )
            {
                return;
            }
            else
            {
                ReadEntryDetails( self );
            }
        }
        else
        {
            ReadEntry( self );
        }
    }
}

/***************************************************************************
 **  ScanBlock  Read and ignore block
 **  ~~~~~~~~~
 */

VOID ScanBlock( PR_XCHANGE* self )
{
    for(;;)
    {
        if( self->xchange.grp == 0 )
        {
            if( IsGroupText( self, SR_DXF_ENDBLK ) )
            {
                return;
            }
        }
        ReadEntry( self );
    }
}

/***************************************************************************
 **  ReadBlocks  Read blocks section
 **  ~~~~~~~~~~
 */

VOID ReadBlocks( PR_XCHANGE* self )
{
    DXF_BLK blk;
    UWORD wd;

    p_send4( DatDialogPtr, O_DXFDL_UPDATE, SR_DXF_IMP_UPDATE_BLK, 0 );
    for(;;)
    {
        f_seek( self->xchange.pcb, P_FRSENSE, &blk.pos );
        ReadEntry( self );
        if( self->xchange.grp == 0 )
        {
            if( IsGroupText( self, SR_DXF_BLOCK ) )
            {
                ReadEntryDetails( self );
                p_scpy( blk.name, self->xchange.inp.txt2 );
                p_send4( self->xchange.blktab, O_VA_INSERTISQ, &blk, &wd );
                ScanBlock( self );
            }
            else if( IsGroupText( self, SR_DXF_ENDSEC ) )
            {
                return;
            }
        }
    }
}

/***************************************************************************
 **  ScanTable  Read and ignore table
 **  ~~~~~~~~~
 */

VOID ScanTable( PR_XCHANGE* self )
{
    for(;;)
    {
        ReadEntry( self );
        if( self->xchange.grp == 0 )
        {
            if( IsGroupText( self, SR_DXF_ENDTAB ) )
            {
                return;
            }
        }
    }
}


/***************************************************************************
 **  UpdateLayerTab  Create a new layer table for drawing
 **  ~~~~~~~~~~~~~~
 */

VOID UpdateLayerTab( PR_XCHANGE* self )
{
    INT size;
    INT i;
    INT max;
    TEXT* newlist;
    UWORD mask;

    max = self->xchange.laymax;
    if( max > 16 )
    {
        max = 16;
    }
    for( size = i = 0 ; i < 16 ; i++ )
    {
        mask = 1 << i;
        if( i < max )
        {
            if( self->xchange.laytab[i].off )
            {
                Drg->vec.LayMask &= ~mask;
            }
            else
            {
                Drg->vec.LayMask |= mask;
            }
            size += p_slen( self->xchange.laytab[i].name ) + 1;
        }
        else
        {
            Drg->vec.LayMask |= mask;
            size++;
        }
    }
    newlist = p_realloc( Drg->drg.LayBuf, size );
    if( newlist )
    {
        Drg->drg.LayBufSize = size;
        Drg->drg.LayBuf = newlist;
        for( size = i = 0 ; i < 16 ; i++ )
        {
            Drg->drg.LayStr[i] = &newlist[size];
            if( i < max )
            {
                p_scpy( &newlist[size], self->xchange.laytab[i].name );
                size += p_slen( &newlist[size] ) + 1;
            }
            else
            {
                newlist[size] = '\0';
                size++;
            }
        }
    }
    Drg->vec.Layer = GetLayer( self, self->xchange.clayer );
}

/***************************************************************************
 **  ReadLayerTable  Read Layer table
 **  ~~~~~~~~~~~~~~
 */

VOID ReadLayerTable( PR_XCHANGE* self )
{
    DXF_LAYER* pLayer;
    INT maxent;
    INT ent;

    ent = 0;
    ReadEntryDetails( self );
    maxent = self->xchange.inp.f70;
    pLayer = (DXF_LAYER*) f_alloc( sizeof(DXF_LAYER) * maxent );
    self->xchange.laytab = pLayer;

    for(;;)
    {
        if( self->xchange.grp == 0 )
        {
            if( IsGroupText( self, SR_DXF_LAYER ) )
            {
                ReadEntryDetails( self );
                if( ent < maxent )
                {
                    self->xchange.inp.txt2[UNAME_MAX] = '\0';
                    p_scpy( pLayer[ent].name, self->xchange.inp.txt2 );
                    pLayer[ent].off = ( self->xchange.inp.pen62 < 0 );
                    ent++;
                }
            }
            else if( IsGroupText( self, SR_DXF_ENDTAB ) )
            {
                self->xchange.laymax = ent;
                UpdateLayerTab( self );
                return;
            }
            else
            {
                p_leave( ERR_DXF_FILE_FORMAT );
            }
        }
        else
        {
            p_leave( ERR_DXF_FILE_FORMAT );
        }
    }
}

/***************************************************************************
 **  UpdateVport  Use the active view port settings to set Snap and Grid
 **  ~~~~~~~~~~~
 */

VOID UpdateVport( PR_XCHANGE* self )
{
    UBYTE flags;
    UBYTE list;
    INT snap;
    INT grid;
    INT step;

    flags = 0;
    if( self->xchange.snapon )
    {
        flags = GRID_SNAP;
    }
    if( self->xchange.gridon )
    {
        flags |= GRID_BLACK;
    }
    Drg->vec.Pref.grid.flags = flags;

    snap = self->xchange.snapspc;
    grid = self->xchange.gridspc;
    Drg->vec.Pref.grid.space = snap;

    if( snap <= 0 )
    {
        return;
    }
    grid = snap + snap / 2;
    step = grid / snap;
    if( step > 9 )
    {
        step = 10;
        list = 5;
    }
    else if( step > 7 )
    {
        step = 8;
        list = 4;
    }
    else if( step > 4 )
    {
        step = 5;
        list = 3;
    }
    else if( step > 3 )
    {
        step = 4;
        list = 2;
    }
    else if( step > 2 )
    {
        step = 2;
        list = 1;
    }
    else
    {
        step = 1;
        list = 0;
    }
    Drg->vec.Pref.grid.minor = step;
    Drg->vec.Pref.grid.showlist &= ~GRID_MINOR;
    Drg->vec.Pref.grid.showlist |= list;
}

/***************************************************************************
 **  ReadVportTable  Read VPORT table
 **  ~~~~~~~~~~~~~~
 */

VOID ReadVportTable( PR_XCHANGE* self )
{
    TEXT buf[20];
    INT  flag;

    flag = FALSE;
    ReadEntryDetails( self );
    for(;;)
    {
        if( self->xchange.grp == 0 )
        {
            if( IsGroupText( self, SR_DXF_VPORT ) )
            {
                ReadEntryDetails( self );
                hLoadResBuf( SR_DXF_V_ACTIVE, buf );
                if( p_scmpi( self->xchange.inp.txt2, buf ) == 0 &&
                    self->xchange.vportok == 0 )
                {
                    self->xchange.snapon = self->xchange.inp.f75;
                    self->xchange.gridon = self->xchange.inp.f76;
                    SetDim( self, (AUNIT*) &self->xchange.snapspc, &self->xchange.inp.p14_24.x );
                    SetDim( self, (AUNIT*) &self->xchange.gridspc, &self->xchange.inp.p15_25.x );
                    self->xchange.vportok = TRUE;
                    flag = TRUE;
                }
            }
            else if( IsGroupText( self, SR_DXF_ENDTAB ) )
            {
                if( flag )
                {
                    UpdateVport( self );
                }
                return;
            }
            else
            {
                p_leave( ERR_DXF_FILE_FORMAT );
            }
        }
        else
        {
            p_leave( ERR_DXF_FILE_FORMAT );
        }
    }
}

/***************************************************************************
 **  ReadTable  Read table
 **  ~~~~~~~~~
 */

VOID ReadTable( PR_XCHANGE* self )
{
    p_send4( DatDialogPtr, O_DXFDL_UPDATE, SR_DXF_IMP_UPDATE_TAB, 0 );
    ReadEntry( self );
    if( self->xchange.grp == 2 )
    {
        if( IsGroupText( self, SR_DXF_LAYER ) )
        {
            ReadLayerTable( self );
        }
        else if( IsGroupText( self, SR_DXF_VPORT ) )
        {
            ReadVportTable( self );
        }
        else
        {
            ScanTable( self );
        }
    }
    else
    {
        p_leave( ERR_DXF_FILE_FORMAT );
    }
}

/***************************************************************************
 **  ReadTables  Read tables section
 **  ~~~~~~~~~~
 */

VOID ReadTables( PR_XCHANGE* self )
{
    for(;;)
    {
        ReadEntry( self );
        if( self->xchange.grp == 0 )
        {
            if( IsGroupText( self, SR_DXF_TABLE ) )
            {
                ReadTable( self );
            }
            else if( IsGroupText( self, SR_DXF_ENDSEC ) )
            {
                return;
            }
            else
            {
                p_leave( ERR_DXF_FILE_FORMAT );
            }
        }
        else
        {
            p_leave( ERR_DXF_FILE_FORMAT );
        }
    }
}

/***************************************************************************
 **  SetScale  Use the limmax and limmin settings to workout a suitable
 **  ~~~~~~~~  scale.
 */

VOID SetScale( PR_XCHANGE* self )
{
    WORD   div, tmp;
    DOUBLE dpageht, dpagewd, daa, dtwo, dtmp;
    LONG   lg;
    INT    ret;
    AUNIT  wd, ht;
    AUNIT  marg[4];
    D_PT   dPt;

    lg = AA;
    p_longtof( &daa, &lg );
    tmp = 2;
    p_itof( &dtwo, &tmp );

    p_fld( &self->xchange.scale, &Drg->drg.DScale );
    div = 1;

    /* pageht = ( pgmax.y - pgmin.y ) / scale */
    p_fld( &dpageht, &self->xchange.pgmax.y );
    p_fsub( &dpageht, &self->xchange.pgmin.y );
    p_fdiv( &dpageht, &self->xchange.scale );

    /* Set div to fit page in AA */
    p_fld( &dtmp, &daa );
    p_fdiv( &dtmp, &dtwo );                     /* dtmp = (AA/2) */
    while( p_fcmp( &dpageht, &dtmp ) > 0 )      /* while( pageht > (AA/2) ) */
    {                                           /* {                        */
        p_fmul( &self->xchange.scale, &dtwo );  /*     scale *= 2;          */
        p_fdiv( &dpageht, &dtwo );              /*     pageht /= 2;         */
        div *= 2;                               /*     div *= 2;            */
    }                                           /* }                        */

    /* pagewd = ( pgmax.x - pgmin.x ) / scale */
    p_fld( &dpagewd, &self->xchange.pgmax.x );
    p_fsub( &dpagewd, &self->xchange.pgmin.x );
    p_fdiv( &dpagewd, &self->xchange.scale );
    while( p_fcmp( &dpagewd, &dtmp ) > 0 )      /* while( pagewd > (AA/2) ) */
    {                                           /* {                        */
        p_fmul( &self->xchange.scale, &dtwo );  /*     scale *= 2;          */
        p_fdiv( &dpageht, &dtwo );              /*     pageht /= 2;         */
        p_fdiv( &dpagewd, &dtwo );              /*     pagewd /= 2;         */
        div *= 2;                               /*     div *= 2;            */
    }                                           /* }                        */

    marg[MARGIN_LEFT] = Drg->vec.Marg.pos.x - Drg->vec.Page.pos.x;
    marg[MARGIN_RIGHT] = Drg->vec.Page.lim.x - Drg->vec.Marg.lim.x;
    marg[MARGIN_TOP] = Drg->vec.Marg.pos.y - Drg->vec.Page.pos.y;
    marg[MARGIN_BOTTOM] = Drg->vec.Page.lim.y - Drg->vec.Marg.lim.y;
    ret = ftoau( &wd, &dpagewd );
    ret += ftoau( &ht, &dpageht );

    if( ret == 0 )
    {
        p_send5( Drg, O_DG_UPDATE_PAGE, wd, ht, marg );
        Drg->vec.Pref.page.size = SPAGEDLG_CUSTOM;
        if( wd > ht )
        {
            Drg->vec.Pref.page.orient = SPAGEDLG_LANDSCAPE;
        }
        else
        {
            Drg->vec.Pref.page.orient = SPAGEDLG_PORTRAIT;
        }
    }

    /* offset.x = Page.pos.x * scale - pgmin.x */
    lg = Drg->vec.Page.pos.x;
    p_longtof( &self->xchange.offset.x, &lg );
    p_fmul( &self->xchange.offset.x, &self->xchange.scale );
    p_fsub( &self->xchange.offset.x, &self->xchange.pgmin.x );

    /* offset.y = Page.pos.y * scale + pgmax.y */
    lg = Drg->vec.Page.pos.y;
    p_longtof( &self->xchange.offset.y, &lg );
    p_fmul( &self->xchange.offset.y, &self->xchange.scale );
    p_fadd( &self->xchange.offset.y, &self->xchange.pgmax.y );

    tmp = 0;
    p_itof( &dPt.x, &tmp );
    p_itof( &dPt.y, &tmp );
    SetPt( self, &Drg->vec.Orig, &dPt );
    Drg->vec.Pref.scale.div *= div;
    Drg->vec.yDir = Y_AXIS_UP;
    p_send2( Drg, O_DG_UPDATE_SCALE );
    p_send2( Drg, O_DG_SNAP_CURSOR );
}

/***************************************************************************
 **  ReadHeader  Read header section
 **  ~~~~~~~~~~
 */

#define SET_GRID  0x01
#define SET_SNAP  0x02

VOID ReadHeader( PR_XCHANGE* self )
{
    D_PT grid, snap;
    UINT setgrid;

    setgrid = 0;
    p_send4( DatDialogPtr, O_DXFDL_UPDATE, SR_DXF_IMP_UPDATE_HDR, 0 );
    ReadEntry( self );
    for(;;)
    {
        if( self->xchange.grp == 0 )
        {
            if( IsGroupText( self, SR_DXF_ENDSEC ) )
            {
                SetScale( self );
                if( setgrid == ( SET_GRID & SET_SNAP ) )
                {
                    SetDim( self, (AUNIT*) &self->xchange.snapspc, &snap.x );
                    SetDim( self, (AUNIT*) &self->xchange.gridspc, &grid.x );
                    UpdateVport( self );
                }
                return;
            }
            else
            {
                p_leave( ERR_DXF_FILE_FORMAT );
            }
        }
        else if( self->xchange.grp == 9 )
        {
            if( IsGroupText( self, SR_DXF_H_LIMMIN ) )
            {
                ReadDPt( self, &self->xchange.pgmin );
            }
            else if( IsGroupText( self, SR_DXF_H_LIMMAX ) )
            {
                ReadDPt( self, &self->xchange.pgmax );
            }
            else if( IsGroupText( self, SR_DXF_H_ANGBASE ) )
            {
                ReadEntryDetails( self );
                p_fld( &self->xchange.angbase, &self->xchange.inp.ang50 );
            }
            else if( IsGroupText( self, SR_DXF_H_ANGDIR ) )
            {
                ReadEntryDetails( self );
                self->xchange.angdir = self->xchange.inp.f70;
            }
            else if( IsGroupText( self, SR_DXF_H_CLAYER ) )
            {
                ReadEntryDetails( self );
                p_scpy( self->xchange.clayer, self->xchange.inp.txt8 );
            }
            else if( IsGroupText( self, SR_DXF_H_GRIDMODE ) )
            {
                ReadEntryDetails( self );
                self->xchange.gridon = self->xchange.inp.f70;
            }
            else if( IsGroupText( self, SR_DXF_H_SNAPMODE ) )
            {
                ReadEntryDetails( self );
                self->xchange.snapon = self->xchange.inp.f70;
            }
            else if( IsGroupText( self, SR_DXF_H_GRIDUNIT ) )
            {
                ReadDPt( self, &grid );
                setgrid |= SET_GRID;
            }
            else if( IsGroupText( self, SR_DXF_H_SNAPUNIT ) )
            {
                ReadDPt( self, &snap );
                setgrid |= SET_SNAP;
            }
            else
            {
                ReadEntryDetails( self );
            }
        }
        else
        {
            p_leave( ERR_DXF_FILE_FORMAT );
        }
    }
}

/***************************************************************************
 **  ReadSection  Read file sections
 **  ~~~~~~~~~~~
 */

VOID ReadSection( PR_XCHANGE* self )
{
    ReadEntry( self );
    if( self->xchange.grp == 2 )
    {
        if( IsGroupText( self, SR_DXF_HEADER ) )
        {
            DBX4_Debug( 101, self->xchange.grp, self->xchange.fbuf );
            ReadHeader( self );
        }
        else if( IsGroupText( self, SR_DXF_TABLES ) )
        {
            DBX4_Debug( 102, self->xchange.grp, self->xchange.fbuf );
            ReadTables( self );
        }
        else if( IsGroupText( self, SR_DXF_BLOCKS ) )
        {
            DBX4_Debug( 103, self->xchange.grp, self->xchange.fbuf );
            ReadBlocks( self );
        }
        else if( IsGroupText( self, SR_DXF_ENTITIES ) )
        {
            DBX4_Debug( 104, self->xchange.grp, self->xchange.fbuf );
            p_send4( DatDialogPtr, O_DXFDL_UPDATE, SR_DXF_IMP_UPDATE_FMT, 0 );
            ReadEntry( self );
            ReadEntities( self );
        }
        else
        {
            ScanSection( self );
        }
    }
    else
    {
        p_leave( ERR_DXF_FILE_FORMAT );
    }
}

#pragma METHOD_CALL

/***************************************************************************
 **  xch_import  Import the given file. Calls p_leave if error occurs.
 **  ~~~~~~~~~~
 */

METHOD INT xchange_xch_import( PR_XCHANGE* self, TEXT* pFName )
{
    INT ret;
    DL_DATA escdata;

    if( self->xchange.idle == NULL )
    {
        self->xchange.idle = f_newsend( CAT_VECDXF_OLIB, C_AIDLE, O_AO_INIT );
    }

    if( self->xchange.esc == NULL )
    {
        escdata.id = ESC_IMPORT_DIALOG;
        escdata.rbuf = &self->xchange.cancel;
        escdata.pdlg = &self->xchange.esc;
        hLaunchDial( CAT_VECDXF_VECDXF, C_DXFESCDLG, &escdata );
    }

    if( self->xchange.blktab == NULL )
    {
        self->xchange.blktab = f_new( CAT_VECDXF_OLIB, C_VAFLAT );
        p_send4( self->xchange.blktab, O_VA_INIT, sizeof(DXF_BLK), 8 );
    }
    else
    {
        p_send2( self->xchange.blktab, O_VA_RESET );
    }

    self->xchange.fbuf = (TEXT*) p_send2( Drg, O_DG_GET_FBUF );
    ret = p_open( &self->xchange.pcb, pFName, P_FTEXT | P_FSHARE | P_FRANDOM );
    if( ret ) p_leave( ret );

    InitConsts( self );
    for(;;)
    {
        ReadEntry( self );
        if( self->xchange.grp == 0 )
        {
            if( IsGroupText( self, SR_DXF_SECTION ) )
            {
                ReadSection( self );
            }
            else if( IsGroupText( self, SR_DXF_EOF ) )
            {
                break;
            }
            else
            {
                p_leave( ERR_DXF_FILE_FORMAT );
            }
        }
        else
        {
            p_leave( ERR_DXF_FILE_FORMAT );
        }
    }
    p_send4( DatDialogPtr, O_DXFDL_UPDATE, SR_DXF_UPDATE_DONE, self->xchange.total );
    return 0;
}

/* End of VecDxfIm.c file */
