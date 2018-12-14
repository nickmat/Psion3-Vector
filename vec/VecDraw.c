/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: BASIC DRAWING FUNCTIONS          *  Date Started:  6 Mar 1997  *
 *    File: VECDRAW.C       Type: C SOURCE   *  Date Revised: 30 Nov 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997 - 1998, Nick Matthews
*/

#include <hwim.h>
#include <p_math.h>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"


static AUNIT CalcRot( AUNIT centre, LONG val1, LONG val2 )
{
    LONG sval;

    sval = centre + ( val1 + val2 ) / Drg->drg.Divisor;
    if( sval < 0 || sval >= AA )
    {
        p_leave( OUTSIDE_AA );
    }
    return (AUNIT) sval;
}

static AUNIT CalcScale( AUNIT ax, AUNIT cx )
{
    ULONG val, scale, base;
    LONG sval;

    scale = Drg->drg.Scale;
    base = Drg->drg.Base;
    if( ax > cx )
    {
        val = ( scale * ( ax - cx ) ) / base;
        sval = val;
    }
    else
    {
        val = ( scale * ( cx - ax ) ) / base;
        sval = - (LONG) val;
    }
    sval += cx;
    if( sval < 0 || sval >= AA )
    {
        p_leave( OUTSIDE_AA );
    }
    return (AUNIT) sval;
}

static SWORD CalcAddAngle( LONG mcos, LONG msin )
{
    return ( mcos * Drg->drg.CosQ + msin * Drg->drg.SinQ ) / Drg->drg.Divisor;
}

VOID ProcessPt( A_PT* result, A_PT* source )
{
    LONG dx, dy;

    switch( Drg->drg.ProcessOp )
    {
    case PO_MOVE:
        kcAddPt( result, source, Drg->drg.OffsetX, Drg->drg.OffsetY );
        return;
    case PO_STRETCH:
        if( PointInRect( source, &Drg->drg.StretchRect ) )
        {
            kcAddPt( result, source, Drg->drg.OffsetX, Drg->drg.OffsetY );
            return;
        }
        break;
    case PO_ROTATE:
        if( Drg->drg.Divisor != 0 )
        {
            dx = (LONG) source->x - Drg->drg.CentreX;
            dy = (LONG) source->y - Drg->drg.CentreY;
            result->x = CalcRot( Drg->drg.CentreX,
                dx * Drg->drg.CosQ, -dy * Drg->drg.SinQ );
            result->y = CalcRot( Drg->drg.CentreY,
                dx * Drg->drg.SinQ, dy * Drg->drg.CosQ );
            return;
        }
        break;
    case PO_MIRROR:
        if( Drg->drg.Divisor != 0 )
        {
            dx = (LONG) source->x - Drg->drg.CentreX;
            dy = (LONG) source->y - Drg->drg.CentreY;
            result->x = CalcRot( Drg->drg.CentreX,
                dx * Drg->drg.CosQ, dy * Drg->drg.SinQ );
            result->y = CalcRot( Drg->drg.CentreY,
                dx * Drg->drg.SinQ, -dy * Drg->drg.CosQ );
            return;
        }
        break;
    case PO_SCALE:
        if( Drg->drg.Base != Drg->drg.Scale )
        {
            result->x = CalcScale( source->x, Drg->drg.CentreX );
            result->y = CalcScale( source->y, Drg->drg.CentreY );
            return;
        }
        break;
    }
    /* Default is no operation */
    *result = *source;
}

VOID ProcessDim( AUNIT* result, AUNIT* source )
{
    /* Dimensions (ie circle radius) are only affected by scale ops */
    if( Drg->drg.ProcessOp == PO_SCALE )
    {
        if( Drg->drg.Base != Drg->drg.Scale )
        {
            *result = CalcScale( *source, 0 );
            return;
        }
    }
    /* Default is no operation */
    *result = *source;
}

void ProcessAng( ANGLE* result, ANGLE* source )
{
    SWORD tmp;

    /* Default is no operation, Only affected by PO_ROTATE & PO_MIRROR */
    switch( Drg->drg.ProcessOp )
    {
    case PO_ROTATE:
        if( Drg->drg.Divisor != 0 )
        {
            tmp = CalcAddAngle( source->cos, -source->sin );
            result->sin = CalcAddAngle( source->sin, source->cos );
            result->cos = tmp;
            return;
        }
        break;
    case PO_MIRROR:
        if( Drg->drg.Divisor != 0 )
        {
            tmp = CalcAddAngle( source->cos, source->sin );
            result->sin = -CalcAddAngle( source->sin, -source->cos );
            result->cos = tmp;
            return;
        }
        break;
    }
    *result = *source;
}

VOID ProcessLink( LKSET* result, LKSET* source )
{
    UWORD vflip, hflip;
    LONG mul, div;
    SWORD tmp;

    /* Default is no operation, Not affected by PO_MOVE & PO_STRETCH */
    *result = *source;
    switch( Drg->drg.ProcessOp )
    {
    case PO_ROTATE:
        if( Drg->drg.Divisor != 0 )
        {
            tmp = CalcAddAngle( source->a.cos, -source->a.sin );
            result->a.sin = CalcAddAngle( source->a.sin, source->a.cos );
            result->a.cos = tmp;
            return;
        }
        return;
    case PO_MIRROR:
        if( Drg->drg.Divisor != 0 )
        {
            result->sdiv ^= FLIP_FLAG;
            tmp = CalcAddAngle( source->a.cos, source->a.sin );
            result->a.sin = -CalcAddAngle( source->a.sin, -source->a.cos );
            result->a.cos = tmp;
            return;
        }
        return;
    case PO_SCALE:
        if( Drg->drg.Base != Drg->drg.Scale )
        {
            hflip = source->smul & FLIP_FLAG;
            mul = (LONG) ( source->smul & ~FLIP_FLAG ) * Drg->drg.Scale;
            vflip = source->sdiv & FLIP_FLAG;
            div = (LONG) ( source->sdiv & ~FLIP_FLAG ) * Drg->drg.Base;
            while( mul >= FLIP_FLAG || div >= FLIP_FLAG )
            {
                mul >>= 1;
                if( mul == 0 ) mul = 1;
                div >>= 1;
                if( div == 0 ) div = 1;
            }
            result->smul = ( (UWORD) mul ) | hflip;
            result->sdiv = ( (UWORD) div ) | vflip;
            return;
        }
        return;
    }
}


void DrawLine( A_PT* data )
{
    A_PT pt[2];
    INT su[4];

    /*if( Drg->drg.Plot )*/
    /*{*/
        /*p_send3( Drg->drg.Plot, O_PLOT_LINE, data );*/
        /*return;*/
    /*}*/
    if( Drg->drg.ProcessOp != PO_NONE )
    {
        ProcessPt( &pt[0], &data[0] );
        ProcessPt( &pt[1], &data[1] );
        data = pt;
    }
    su[0] = AtoSdim( data[0].x ) - Drg->drg.sScrnPosX;
    su[1] = AtoSdim( data[0].y ) - Drg->drg.sScrnPosY;
    su[2] = AtoSdim( data[1].x ) - Drg->drg.sScrnPosX;
    su[3] = AtoSdim( data[1].y ) - Drg->drg.sScrnPosY;
    switch( Drg->drg.Primative )
    {
    case PRIM_DRAW:
        kcDrawLine( su, Drg->drg.pClip, &Drg->drg.DAttr );
        return;
    case PRIM_ISINRECT:
        Drg->drg.IsInRect |= kcIsLineInRect( su, Drg->drg.psInRect, NULL );
        return;
    case PRIM_PLOT:
        p_send3( Drg->drg.Plot, O_PLOT_LINE, data );
    }
}

void DrawPolyline( A_PT* data, INT size )
{
    INT i;

    for( i = 0 ; i < size - 1 ; i++ )
    {
        DrawLine( &data[i] );
    }
}

void DrawPolygon( A_PT* data, INT size )
{
    A_PT line[2];

    line[0] = data[0];
    line[1] = data[size-1];
    DrawLine( line );
    DrawPolyline( data, size );
}

VOID DrawBox( A_PT* data )
{
    A_PT pt[2];

    pt[0] = data[0];
    pt[1].x = data[1].x;
    pt[1].y = data[0].y;
    DrawLine( pt );
    pt[0] = data[1];
    DrawLine( pt );
    pt[0] = data[0];
    pt[1].x = data[0].x;
    pt[1].y = data[1].y;
    DrawLine( pt );
    pt[0] = data[1];
    DrawLine( pt );
}

void DrawCircle( A_PT* data )
{
    A_PT pt[2];
    INT su[3];

    /*if( Drg->drg.Plot )*/
    /*{*/
        /*p_send3( Drg->drg.Plot, O_PLOT_CIRCLE, data );*/
        /*return;*/
    /*}*/
    if( Drg->drg.ProcessOp != PO_NONE )
    {
        ProcessPt( &pt[0], &data[0] );
        ProcessDim( &pt[1].x, &data[1].x );
        data = pt;
    }
    su[0] = AtoSdim( data[0].x ) - Drg->drg.sScrnPosX;
    su[1] = AtoSdim( data[0].y ) - Drg->drg.sScrnPosY;
    su[2] = AtoSdim( data[1].x );   /* radius */
    switch( Drg->drg.Primative )
    {
    case PRIM_DRAW:
        kcDrawCircle( su, Drg->drg.pClip, &Drg->drg.DAttr );
        return;
    case PRIM_ISINRECT:
        Drg->drg.IsInRect |= kcIsCircleInRect( su, Drg->drg.psInRect, NULL );
        return;
    case PRIM_PLOT:
        p_send3( Drg->drg.Plot, O_PLOT_CIRCLE, data );
    }
}

/* This must match Arc data in Vector.inc */
typedef struct
{
    LONG xc;
    LONG yc;
    LONG rad;
    LONG begdy;
    LONG enddy;
    UBYTE bego;
    UBYTE endo;
} D_2AARC;


static VOID CalcDy( LONG* dy, UBYTE* oct, ANGLE* ang, AUNIT rad )
{
    LONG da;
    INT sin, cos;
    UBYTE o;

    cos = ABS( ang->cos );
    sin = ABS( ang->sin );
    if( cos > sin )
    {
        da = sin;
        o = 0;
    }
    else
    {
        da = cos;
        o = 1;
    }
    *dy = ( da * rad ) / TRIG_DIV;
    if( ang->cos < 0 )
    {
        o ^= 0x03;
    }
    if( ang->sin < 0 )
    {
        o ^= 0x07;
    }
    *oct = o;
}

void DrawArc( EL_ARC* arc )
{
    D_2AARC darc;
    EL_ARC data;
    AUNIT rad;

    /*if( Drg->drg.Plot )*/
    /*{*/
        /*p_send3( Drg->drg.Plot, O_PLOT_ARC, arc );*/
        /*return;*/
    /*}*/
    if( Drg->drg.ProcessOp != PO_NONE )
    {
        ProcessPt( &data.centre, &arc->centre );
        ProcessDim( &data.radius, &arc->radius );
        if( Drg->drg.ProcessOp == PO_MIRROR && Drg->drg.Divisor != 0 )
        {
            /* Swop beg and end */
            ProcessAng( &data.beg, &arc->end );
            ProcessAng( &data.end, &arc->beg );
        }
        else
        {
            ProcessAng( &data.beg, &arc->beg );
            ProcessAng( &data.end, &arc->end );
        }
        arc = &data;
    }
    darc.xc = AtoSdim( arc->centre.x ) - Drg->drg.sScrnPosX;
    darc.yc = AtoSdim( arc->centre.y ) - Drg->drg.sScrnPosY;
    darc.rad = rad = AtoSdim( arc->radius );
    CalcDy( &darc.begdy, &darc.bego, &arc->beg, rad );
    CalcDy( &darc.enddy, &darc.endo, &arc->end, rad );
    switch( Drg->drg.Primative )
    {
    case PRIM_DRAW:
        kcDraw2AngArc( &darc, Drg->drg.pClip, &Drg->drg.DAttr );
        return;
    case PRIM_ISINRECT:
        Drg->drg.IsInRect |= kcIs2AngArcInRect( &darc, Drg->drg.psInRect, NULL );
        return;
    case PRIM_PLOT:
        p_send3( Drg->drg.Plot, O_PLOT_ARC, arc );
    }
}

void Draw3PtArc( EL_3PT* elem )
{
    EL_3PT aarc;
    EL_3PT* arc;
    EL_LARC larc;
    D_2AARC darc;
    AUNIT rad;

    /*if( Drg->drg.Plot )*/
    /*{*/
        /*p_send3( Drg->drg.Plot, O_PLOT_3PT_ARC, elem );*/
        /*return;*/
    /*}*/
    if( Drg->drg.ProcessOp != PO_NONE )
    {
        ProcessPt( &aarc.beg, &elem->beg );
        ProcessPt( &aarc.mid, &elem->mid );
        ProcessPt( &aarc.end, &elem->end );
        arc = &aarc;
    }
    else
    {
        arc = elem;
    }
    if( Convert3PtArcToLArc( &larc, arc ) == FALSE )
    {
        DrawPolyline( &elem->beg, 3 );
        return;
    }
    darc.xc = ( larc.centre.x / Drg->vec.upp ) - Drg->drg.sScrnPosX;
    darc.yc = ( larc.centre.y / Drg->vec.upp ) - Drg->drg.sScrnPosY;
    darc.rad = larc.radius / Drg->vec.upp;
    if( darc.rad > 0xffff || darc.rad < 1 )
    {
        DrawPolyline( &elem->beg, 3 );
        return;
    }
    rad = darc.rad;
    CalcDy( &darc.begdy, &darc.bego, &larc.beg, rad );
    CalcDy( &darc.enddy, &darc.endo, &larc.end, rad );
    switch( Drg->drg.Primative )
    {
    case PRIM_DRAW:
        kcDraw2AngArc( &darc, Drg->drg.pClip, &Drg->drg.DAttr );
        return;
    case PRIM_ISINRECT:
        Drg->drg.IsInRect |= kcIs2AngArcInRect( &darc, Drg->drg.psInRect, NULL );
        return;
    case PRIM_PLOT:
        p_send3( Drg->drg.Plot, O_PLOT_3PT_ARC, elem );
    }
}

void Draw( UBYTE* element )
{
    /* If not Quick, set up Drg->drg.DAttr here */
    switch( element[TYPE_BYTE] )
    {
    case V_LINE:
        DrawLine( (A_PT*) &element[DATA_BYTE] );
        break;
    case V_POLYLINE:
        DrawPolyline( (A_PT*) &element[DATA_BYTE], DATASIZE( element ) );
        break;
    case V_POLYGON:
        DrawPolygon( (A_PT*) &element[DATA_BYTE], DATASIZE( element ) );
        break;
    case V_BOX:
        DrawBox( (A_PT*) &element[DATA_BYTE] );
        break;
    case V_CIRCLE:
        DrawCircle( (A_PT*) &element[DATA_BYTE] );
        break;
    case V_ARC:
        DrawArc( (EL_ARC*) element );
        break;
    case V_3PT_ARC:
        Draw3PtArc( (EL_3PT*) element );
        break;
    case V_TEXT:
        if( Drg->drg.Quick )
        {
            DrawTextBox( (EL_TEXT*) element );
            break;
        }
        DrawText( (EL_TEXT*) element );
        break;
    case V_LINK:
        if( Drg->drg.Quick )
        {
            DrawLinkBox( (EL_LINK*) element );
            break;
        }
        DrawLink( (EL_LINK*) element );
        break;
    case V_DIM_HORIZ:
        DrawDimHoriz( (EL_DIM*) element );
        break;
    }
}

#if 0
/*---------------------------[ IsElementInRect ]--------------------------*/

static INT IsLineInRect( SRECT* rect, A_PT* data )
{
    INT su[4];

    su[0] = AtoSdim( data[0].x );
    su[1] = AtoSdim( data[0].y );
    su[2] = AtoSdim( data[1].x );
    su[3] = AtoSdim( data[1].y );
    return kcIsLineInRect( su, rect, NULL );
}

static INT IsPolylineInRect( SRECT* rect, A_PT* data, INT size )
{
    INT i;

    for( i = 0 ; i < size - 1 ; i++ )
    {
        if( IsLineInRect( rect, &data[i] ) == TRUE )
        {
            return TRUE;
        }
    }
    return FALSE;
}

static INT IsCircleInRect( SRECT* rect, A_PT* data )
{
    INT su[3];

    su[0] = AtoSdim( data[0].x );
    su[1] = AtoSdim( data[0].y );
    su[2] = AtoSdim( data[1].x );
    return kcIsCircleInRect( su, rect, NULL );
}

static INT IsArcInRect( SRECT* rect, EL_ARC* arc )
{
    D_2AARC darc;
    AUNIT rad;

    darc.xc = AtoSdim( arc->centre.x );
    darc.yc = AtoSdim( arc->centre.y );
    darc.rad = rad = AtoSdim( arc->radius );
    CalcDy( &darc.begdy, &darc.bego, &arc->beg, rad );
    CalcDy( &darc.enddy, &darc.endo, &arc->end, rad );
    return kcIs2AngArcInRect( &darc, rect, NULL );
}

static INT Is3PtArcInRect( SRECT* rect, EL_3PT* arc )
{
    EL_LARC larc;
    D_2AARC darc;
    AUNIT rad;

    if( Convert3PtArcToLArc( &larc, arc ) == FALSE )
    {
        return IsPolylineInRect( rect, &arc->beg, 3 );
    }
    darc.xc = larc.centre.x / Drg->vec.upp;
    darc.yc = larc.centre.y / Drg->vec.upp;
    darc.rad = larc.radius / Drg->vec.upp;
    if( darc.rad > 0xffff || darc.rad < 1 )
    {
        return IsPolylineInRect( rect, &arc->beg, 3 );
    }
    rad = darc.rad;
    CalcDy( &darc.begdy, &darc.bego, &larc.beg, rad );
    CalcDy( &darc.enddy, &darc.endo, &larc.end, rad );
    return kcIs2AngArcInRect( &darc, rect, NULL );
}

LOCAL_C UINT IsTextInRect( SRECT* rect, EL_TEXT* pText )
{
    EL_4PT pgon;

    pgon.hdr.size = sizeof(EL_4PT);
    pgon.hdr.type = V_POLYGON;
    pgon.hdr.attr = 0;
    pgon.hdr.layer = pText->hdr.layer;
    GetTextRect( pgon.pt, pText );
    return IsElementInRect( rect, (UBYTE*) &pgon );
}

INT IsElementInRect( SRECT* rect, UBYTE* el )
{
    int size;
    A_PT* data;
    A_PT pt[2];

    data = (A_PT*) &el[DATA_BYTE];
    size = DATASIZE( el );

    switch( el[TYPE_BYTE] )
    {
    case V_LINE:
        return IsLineInRect( rect, data );
    case V_POLYGON:
        pt[0] = data[0];
        pt[1] = data[size-1];
        if( IsLineInRect( rect, pt ) == TRUE )
        {
            return TRUE;
        }
        /* Fall through */
    case V_POLYLINE:
        return IsPolylineInRect( rect, data, size );
    case V_CIRCLE:
        return IsCircleInRect( rect, data );
    case V_ARC:
        return IsArcInRect( rect, (EL_ARC*) el );
    case V_3PT_ARC:
        return Is3PtArcInRect( rect, (EL_3PT*) el );
    case V_TEXT:
        return IsTextInRect( rect, (EL_TEXT*) el );
    case V_LINK:
        return IsLinkInRect( rect, (EL_LINK*) el );
    }
    return FALSE;
}
#endif

/* End of VecDraw.c file */
