/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: MISC. DRAWING FUNCTIONS          *  Date Started: 10 Feb 1996  *
 *    File: VECMISC.C       Type: C SOURCE   *  Date Revised: 10 Feb 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <p_gen.h>
#include <p_math.h>
#include <limits.h>
#include <p_config.h>
#include <hwim.h>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"

/***************************************************************************
 **  NormaliseRect  Check and correct if rect.pos is greater than rect.lim
 **  ~~~~~~~~~~~~~
 */

VOID NormaliseRect( ARECT* rect )
{
    AUNIT temp;

    if( rect->pos.x > rect->lim.x )
    {
        temp = rect->pos.x;
        rect->pos.x = rect->lim.x;
        rect->lim.x = temp;
    }
    if( rect->pos.y > rect->lim.y )
    {
        temp = rect->pos.y;
        rect->pos.y = rect->lim.y;
        rect->lim.y = temp;
    }
}

/***************************************************************************
 **  PointInRect  Return TRUE if the point in inside the normalised
 **  ~~~~~~~~~~~  rectangle rect, return FALSE if not.
 */

INT PointInRect( const A_PT* pt, const ARECT* rect )
{
    if( pt->x >= rect->pos.x && pt->x <= rect->lim.x &&
        pt->y >= rect->pos.y && pt->y <= rect->lim.y )
    {
        return TRUE;
    }
    return FALSE;
}

/***************************************************************************
 **  RectOverlap  Return TRUE if the two rects overlap, return FALSE if not.
 **  ~~~~~~~~~~~
 */

static INT RangeOverlap( AUNIT au1, AUNIT au2, AUNIT pos, AUNIT lim )
{
    AUNIT temp;

    if( au1 > au2 )
    {
        temp = au1;
        au1 = au2;
        au2 = temp;
    }
    if( au1 >= lim )
    {
        return FALSE;
    }
    if( au2 <= pos )
    {
        return FALSE;
    }
    return TRUE;
}

INT RectOverlap( const ARECT* pRectRaw, const ARECT* pRectNorm )
{
    if( RangeOverlap( pRectRaw->pos.x, pRectRaw->lim.x,
        pRectNorm->pos.x, pRectNorm->lim.x ) == FALSE )
    {
        return FALSE;
    }
    return RangeOverlap( pRectRaw->pos.y, pRectRaw->lim.y,
        pRectNorm->pos.y, pRectNorm->lim.y );
}

VOID AddPointToBound( ARECT* rect, A_PT* pt )
{
    if( pt->x < rect->pos.x )
    {
        rect->pos.x = pt->x;
    }
    else if( pt->x >= rect->lim.x )
    {
        rect->lim.x = pt->x + 1;
    }
    if( pt->y < rect->pos.y )
    {
        rect->pos.y = pt->y;
    }
    else if( pt->y >= rect->lim.y )
    {
        rect->lim.y = pt->y + 1;
    }
}

INT BitToIndex( UINT flag )
{
    INT i = 0;

    for(;;)
    {
        flag >>= 1;
        if( flag == 0 ) break;
        i++;
    }
    return i;
}


INT ComparePt( A_PT* pt1, A_PT* pt2 )
{
    if( pt1->x == pt2->x && pt1->y == pt2->y )
    {
        return 0;
    }
    return 1;
}

/***************************************************************************
 **  AddAngle  Add ang2 to ang1
 **  ~~~~~~~~
 */

VOID AddAngle( ANGLE* pAng1, ANGLE* pAng2 )
{
    SWORD sin, cos;

    sin = ( (LONG) pAng1->sin * pAng2->cos + (LONG) pAng1->cos * pAng2->sin ) / TRIG_DIV;
    cos = ( (LONG) pAng1->cos * pAng2->cos - (LONG) pAng1->sin * pAng2->sin ) / TRIG_DIV;
    pAng1->sin = sin;
    pAng1->cos = cos;
}

/***************************************************************************
 **  RotatePt  Rotate a point about a centre
 **  ~~~~~~~~
 */

INT RotatePt( A_PT* pPt, A_PT* pPtCtr, ANGLE* pAng )
{
    LONG dx, dy, x, y;

    dx = (LONG) pPt->x - pPtCtr->x;
    dy = (LONG) pPt->y - pPtCtr->y;
    x = pPtCtr->x + ( dx * pAng->cos + dy * pAng->sin ) / TRIG_DIV;
    y = pPtCtr->y + ( dy * pAng->cos - dx * pAng->sin ) / TRIG_DIV;

    if( x >= AA || x < 0 ) return 1;
    if( y >= AA || y < 0 ) return 1;
    pPt->x = x;
    pPt->y = y;
    return 0;
}

INT IsLayerOn( INT layer )
{
    /* NEW_LAYER overides all other considerations */
    if( layer & NEW_LAYER ) return BL_GREY;

    /* drg.Layer must be switched on */
    if( Drg->vec.LayMask & LayerBit( layer ) )
    {
        if( layer & BAND_LAYER )
        {
            return Drg->drg.BandLayer;
        }
        return BL_BLACK;
    }
    return BL_OFF;
}

/***************************************************************************
 **  GetRelPolarPt  Fill in the relative point from the given asin, acos
 **  ~~~~~~~~~~~~~  and radius (hypotenuse) values.
 */

VOID GetRelPolarPt( R_PT* rpt, SWORD asin, SWORD acos, AUNIT radius )
{
    rpt->x = ( (LONG) acos * radius ) / TRIG_DIV;
    rpt->y = ( (LONG) asin * radius ) / TRIG_DIV;
}


/***************************************************************************
 **  GetPolarPt  Fill in the point from the given centre, angle and radius
 **  ~~~~~~~~~~  values.
 */

VOID GetPolarPt( A_PT* pPt, A_PT* pCentrePt, ANGLE* pAngle, AUNIT radius )
{
    pPt->x = pCentrePt->x + ( (LONG) pAngle->cos * radius ) / TRIG_DIV;
    pPt->y = pCentrePt->y + ( (LONG) pAngle->sin * radius ) / TRIG_DIV;
}


/***************************************************************************
 **  DWin utility functions
 **  ~~~~~~~~~~~~~~~~~~~~~~
 */

VOID BegDrawClear( VOID )
{
    p_send2( w_ws->wserv.cli, O_WN_BEG_DRAW_CLEAR );
}

VOID BegDraw( VOID )
{
    p_send2( w_ws->wserv.cli, O_WN_BEG_DRAW );
}

VOID EndDraw( VOID )
{
    p_send2( w_ws->wserv.cli, O_WN_END_DRAW );
}

VOID Rewind( VOID )
{
    p_send2( Data, O_DD_REWIND );
}

VOID BegUndo( INT msg )
{
    p_send3( Undo, O_UD_BLOCK_BEG, msg );
}

VOID EndUndo( VOID )
{
    p_send2( Undo, O_UD_BLOCK_END );
}

VOID AddUndo( VOID* pBuf )
{
    UINT hand;

    hand = p_send3( Data, O_DD_ADD, pBuf );
    p_send3( Undo, O_UD_SAVE_DELETE, hand );
}

VOID KillUndo( VOID )
{
    p_send2( Undo, O_UD_KILL );
    p_send2( Redo, O_UD_KILL );
}

VOID GetDWinSize( P_EXTENT* pext )
{
    p_send3( w_ws->wserv.cli, O_DW_GET_SIZE, pext );
}

/***************************************************************************
 **  GetHeaderSize  Checks for a valid file. Returns 0 if file not a valid
 **  ~~~~~~~~~~~~~  drawing file or the overall header (Hdr+ExHdr) size.
 **  Expects file to be positioned either by a p_seek or by just having
 **  opened the file.
 */

UINT GetHeaderSize( VOID* pfcb )
{
    DRGHDR fhdr, cmp;

    p_send4( w_am, O_AM_LOAD_RES_BUF, SR_FILE_SIG, cmp.sig );
    f_read( pfcb, &fhdr, sizeof(DRGHDR) );
    if( p_bcmp( fhdr.sig, 16, cmp.sig, 16 ) != 0 )
    {
        return 0;
    }
    /* Version numbers not checked */
    return fhdr.hdrsize;
}

/***************************************************************************
 **  SeekTable  Step through the files table looking for give type. Returns
 **  ~~~~~~~~~  the file position of the start of the table if found or 0 if
 **  not. A copy of the Table header is copied into thdr.
 */

UINT SeekTable( VOID* pfcb, UWORD offset, UWORD type, TBHDR* thdr )
{
    LONG pos;
    UWORD i, count;
    UWORD size;

    pos = sizeof(DRGHDR) + sizeof(UWORD) + offset;
    f_seek( pfcb, P_FABS, &pos );
    f_read( pfcb, &count, sizeof(UWORD) );
    pos = SignatureSize + sizeof(UWORD) + offset;
    f_seek( pfcb, P_FABS, &pos );
    f_read( pfcb, &size, sizeof(UWORD) );
    pos = size + offset;
    f_seek( pfcb, P_FABS, &pos );
    for( i = 0 ; i < count ; i++ )
    {
        f_read( pfcb, thdr, sizeof(TBHDR) );
        pos += sizeof(TBHDR);
        if( thdr->type == type )
        {
            return (UINT) pos;
        }
        pos += thdr->size;
        f_seek( pfcb, P_FABS, &pos );
    }
    return 0;
}

static WORD TextUnitHt[] = { 254, 2540, 100, 7200 };
                          /*  cm   mm   inch  pts  */

VOID GetTextHtStr( TEXT* buf, UWORD smul, UWORD sdiv, INT units )
{
    E_CONFIG cfg;
    LONG val;
    INT i, r;

    p_getctd( &cfg );
    sdiv &= ~FLIP_FLAG;
    val = ( (LONG) smul * TextUnitHt[ units ] ) / sdiv;
    i = (INT) ( val / 100 );
    r = (INT) ( val % 100 );
    if( r )
    {
         p_atos( buf, "%d%c%02d", i, cfg.decimalSeparator, r );
    }
    else
    {
         p_atos( buf, "%d", i );
    }
}

INT SetTextHt( UWORD* smul, UWORD* sdiv, TEXT* pstr, INT units )
{
    E_CONFIG cfg;
    LONG val;
    WORD real;
    INT ret;

    p_getctd( &cfg );
    ret = p_stoi( &pstr, &real );
    if( ret == E_GEN_FAIL )
    {
        real = 0;
    }
    else if( ret < 0 )
    {
        return FALSE;
    }
    val = (LONG) real * 100;
    if( *pstr++ == cfg.decimalSeparator )
    {
        if( *pstr && p_isdigit( *pstr ) )
        {
            val += ( *pstr - '0' ) * 10;
            pstr++;
        }
        if( *pstr && p_isdigit( *pstr ) )
        {
            val += *pstr - '0';
        }
    }
    *sdiv = TextUnitHt[ units ];
    while( val > 0x8fff )
    {
        *sdiv >>= 1;
        val >>= 1;
    }
    *smul = (UINT) val;
    return TRUE;
}

static VOID GetRadPerDeg( DOUBLE* d )
{
    LONG l;
    DOUBLE t;

    l = PI_Q;
    p_longtof( d, &l );
    l = PI_D;
    p_longtof( &t, &l );
    p_fdiv( d, &t );
    l = 180;
    p_longtof( &t, &l );
    p_fdiv( d, &t );
}


static INT dtoi( DOUBLE* d )
{
    DOUBLE two, half;
    INT sign;
    WORD i = 1;

    p_itof( &two, &i );     /* two = 1 */
    p_fld( &half, &two );   /* half = 1 */
    p_fadd( &two, &half );  /* two = 2 */
    p_fdiv( &half, &two );  /* half = 0.5 */
    p_fsub( &two, &two );   /* two = 0 */
    sign = p_fcmp( &two, d );
    p_fld( &two, d );       /* two = d */
    if( sign < 0 )
    {
        p_fsub( &two, &half ); /* two = d - 0.5 */
    }
    else
    {
        p_fadd( &two, &half ); /* two = d + 0.5 */
    }
    p_inti( &i, &two );     /* i = ROUND( d ) */
    return i;
}

VOID SetAngle( SWORD* asin, SWORD* acos, DOUBLE* angle )
{
    DOUBLE d1, d2, t;
    WORD i;

    GetRadPerDeg( &d1 );
    p_fmul( &d1, angle ); /* d1 = angle in radians */
    i = TRIG_DIV;
    p_itof( &t, &i );
    p_sin( &d2, &d1 );
    p_fmul( &d2, &t );
    *asin = dtoi( &d2 );
    p_cos( &d2, &d1 );
    p_fmul( &d2, &t );
    *acos = dtoi( &d2 );
}

VOID GetAngleDbl( DOUBLE* d, SWORD asin, SWORD acos )
{
    DOUBLE t;
    WORD i;
    WORD abssin = ABS( asin );
    WORD abscos = ABS( acos );

    i = TRIG_DIV;
    p_itof( &t, &i );
    if( abssin < abscos )
    {
        /* Use asin */
        p_itof( d, &abssin );
        p_fdiv( d, &t );
        p_asin( d, d );
    }
    else
    {
        /* Use acos */
        p_itof( d, &abscos );
        p_fdiv( d, &t );
        p_acos( d, d );
    }
    /* d contains angle in radians */
    GetRadPerDeg( &t );
    p_fdiv( d, &t );
    /* Add quadrant value */
    if( acos < 0 )
    {
        i = 180;
        p_itof( &t, &i );
        p_fneg( d );
        p_fadd( d, &t );
    }
    if( asin < 0 )
    {
        i = 360;
        p_itof( &t, &i );
        p_fneg( d );
        p_fadd( d, &t );
    }
}


VOID GetAngleStr( TEXT* buf, SWORD asin, SWORD acos )
{
    DOUBLE angle;
    P_DTOB format;
    E_CONFIG cfg;
    INT len;

    p_getctd( &cfg );
    GetAngleDbl( &angle, asin, acos );
    format.type = P_DTOB_FIXED;
    format.width = 7;
    format.ndec = 2;
    format.point = cfg.decimalSeparator;
    format.trilen = 0;
    len = p_dtob( buf, &angle, &format );
    if( len )
    {
        buf[len] = '\0';
        --len;
        while( buf[len] == '0' )
        {
            buf[len] = '\0';
            --len;
        }
        if( buf[len] == cfg.decimalSeparator )
        {
            buf[len] = '\0';
        }
    }
    else
    {
        /* Somethings gone wrong! */
        buf[0] = '\0';
    }
}

VOID GetScaleDbl( DOUBLE* scale, LKSET* set )
{
    DOUBLE dbl;
    WORD word;

    p_itof( scale, (WORD*) &set->smul );
    word = set->sdiv & ~FLIP_FLAG;
    p_itof( &dbl, &word );
    p_fdiv( scale, &dbl );
}

VOID SetScale( LKSET* set, DOUBLE* scale )
{
    DOUBLE d1, d2;
    WORD word;
    SWORD flip;

    flip = set->sdiv & FLIP_FLAG;
    p_fld( &d1, scale );
    word = 1;
    p_itof( &d2, &word );
    if( p_fcmp( scale, &d2 ) > 0 )
    {
        /* Scale greater than 1 */
        set->smul = 16384;
        p_itof( &d2, (WORD*) &set->smul );
        p_fdiv( &d2, &d1 );
        p_inti( (WORD*) &set->sdiv, &d2 );
    }
    else
    {
        /* Scale less than 1 */
        set->sdiv = 16384;
        p_itof( &d2, (WORD*) &set->sdiv );
        p_fmul( &d1, &d2 );
        p_inti( (WORD*) &set->smul, &d1 );
    }
    set->sdiv |= flip;
}

VOID DeleteFromList( VOID* list, INT* size, INT unit, INT item )
{
    UBYTE* target;
    UBYTE* source;
    INT last;
    INT len;

    target = ( (UBYTE*) list ) + item * unit;
    source = target + unit;
    last = *size - 1;
    len = ( last - item ) * unit;
    if( len < 0 )
    {
        p_panic( PN_DELETE_ITEM_FROM_LIST );
    }
    if( len > 0 )
    {
        p_bcpy( target, source, len );
    }
    f_realloc( list, last * unit );
    --*size;
}

SWORD TestUpp( SWORD upp )
{
    if( upp > 256 )
    {
        hInfoPrint( SR_MIN_ZOOM );
        upp = 256;
    }
    else if( upp < 4 )
    {
        hInfoPrint( SR_MAX_ZOOM );
        upp = 4;
    }
    return upp;
}

INT IsKeyDown( VOID )
{
    INT i;
    UWORD scan[10];

    p_getscancodes( scan );
    for( i = 0 ; i < 10 ; i++ )
    {
        if( scan[i] )
        {
            return TRUE;
        }
    }
    return FALSE;
}

AUNIT GetMean( AUNIT au1, AUNIT au2 )
{
    LAUNIT lau;

    lau = ( (LAUNIT) au1 + au2 ) >> 1;
    return lau;
}

/* End of VecMisc.c File */
