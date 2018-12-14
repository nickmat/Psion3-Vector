/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: CALCULATIONS USING DOUBLES       *  Date Started: 11 Jan 1997  *
 *    File: CALCARC.C       Type: C SOURCE   *  Date Revised: 11 Jan 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

  Copyright (c) 1997 Nick Matthews
*/

#include <hwim.h>
#include <p_math.h>
#include <vector.g>
#include "vector.h"

#include "vector.rsg"

typedef struct
{
    LONG dxa, dya;
    LONG xap, yap;
    LONG dxb, dyb;
    LONG xbp, ybp;
} LXCALC;

typedef struct
{
    DOUBLE dxa, dya;
    DOUBLE xap, yap;
    DOUBLE dxb, dyb;
    DOUBLE xbp, ybp;
} DXCALC;

typedef struct
{
    DOUBLE x;
    DOUBLE y;
} DA_PT;

/***************************************************************************
 **  CalcQuadrant  Complete the beg and end angles of the quadrant arc from
 **  ~~~~~~~~~~~~  the arc centre point and the beg point.
 */

VOID CalcQuadrant( EL_ARC* arc, A_PT* pt )
{
    arc->radius = CalcAngle( &arc->beg, &arc->centre, pt );
    arc->end.sin = arc->beg.cos;
    arc->end.cos = -arc->beg.sin;
}

/***************************************************************************
 **  CalcInterceptCo  Calcutate the cordinate of the intercept, given a
 **  ~~~~~~~~~~~~~~~  DXCALC struct. Uses the following calc:-
 **
 **  xc = ( dxa*dxb(ybp-yap) + dxb*dya*xap - dxa*dyb*xbp )
 **                 / ( dxb*dya - dxa*dyb );
 **
 **  term1 = dxa * dxb * ( ybp - yap )
 **  term2 = dxb * dya * xap
 **  term3 = dxa * dyb * xbp
 **  divisor = dxb * dya - dxa * dyb
 **  xc = ( term1 + term2 - term3 ) / divisor
 */

static VOID CalcInterceptCo( DOUBLE* result, DXCALC* calc )
{
    DOUBLE t2, t3;

    /* Use result for term1 */
    p_fld( result, &calc->ybp );
    p_fsub( result, &calc->yap );
    p_fmul( result, &calc->dxa );
    p_fmul( result, &calc->dxb );
    /* Use t2 for term2 */
    p_fld( &t2, &calc->xap );
    p_fmul( &t2, &calc->dxb );
    p_fmul( &t2, &calc->dya );
    /* Use t3 for term3 */
    p_fld( &t3, &calc->xbp );
    p_fmul( &t3, &calc->dxa );
    p_fmul( &t3, &calc->dyb );
    /* Use result for the sum of the terms */
    p_fadd( result, &t2 );
    p_fsub( result, &t3 );
    /* Use t2 (& t3) for divisor */
    p_fld( &t2, &calc->dxb );
    p_fmul( &t2, &calc->dya );
    p_fld( &t3, &calc->dxa );
    p_fmul( &t3, &calc->dyb );
    p_fsub( &t2, &t3 );

    /* Final answer xc */
    p_fdiv( result, &t2 );
}

/***************************************************************************
 **  LtoDXCalc  Copy a LXCALC struct to a DXCALC struct.
 **  ~~~~~~~~~
 */

static VOID LtoDXCalc( DXCALC* result, LXCALC* calc )
{
    p_longtof( &result->dxa, &calc->dxa );
    p_longtof( &result->dya, &calc->dya );
    p_longtof( &result->dxb, &calc->dxb );
    p_longtof( &result->dyb, &calc->dyb );
    p_longtof( &result->xap, &calc->xap );
    p_longtof( &result->yap, &calc->yap );
    p_longtof( &result->xbp, &calc->xbp );
    p_longtof( &result->ybp, &calc->ybp );
}

/***************************************************************************
 **  DtoDYCalc  Copy one DXCALC to another, transposing x and y values.
 **  ~~~~~~~~~
 */

static VOID DtoDYCalc( DXCALC* y, DXCALC* x )
{
    p_fld( &y->dxa, &x->dya );
    p_fld( &y->dya, &x->dxa );
    p_fld( &y->xap, &x->yap );
    p_fld( &y->yap, &x->xap );
    p_fld( &y->dxb, &x->dyb );
    p_fld( &y->dyb, &x->dxb );
    p_fld( &y->xbp, &x->ybp );
    p_fld( &y->ybp, &x->xbp );
}

/***************************************************************************
 **  ScaleDPt  Scale a double point, as below
 **  ~~~~~~~~
 **  result.x = dpt.x * scale + opt.x;
 **  result.y = dpt.y * scale + opt.y;
 */

static VOID ScaleDPt( DA_PT* result, DA_PT* dpt, DA_PT* opt, DOUBLE* scale )
{
    p_fld( &result->x, &dpt->x );
    p_fmul( &result->x, scale );
    p_fadd( &result->x, &opt->x );

    p_fld( &result->y, &dpt->y );
    p_fmul( &result->y, scale );
    p_fadd( &result->y, &opt->y );
}

/***************************************************************************
 **  DPtToLPt  Copy a double point to a long point. Return 1 if succesful
 **  ~~~~~~~~  or a neg error if not.
 */

static INT DPtToLPt( LA_PT* result, DA_PT* dpt )
{
    LONG xl, yl;
    INT ret;

    ret = p_intl( &xl, &dpt->x );
    if( ret < 0 )
    {
        return ret;
    }
    ret = p_intl( &yl, &dpt->y );
    if( ret < 0 )
    {
        return ret;
    }
    result->x = xl;
    result->y = yl;
    return 1;
}

/***************************************************************************
 **  LPtToAPt  Copy a long point to a aunit point. Return 1 if succesful
 **  ~~~~~~~~  or a 0 if not.
 */

static INT LPtToAPt( A_PT* result, LA_PT* lpt )
{
    if( lpt->x < 0 || lpt->x > 0xffff )
    {
        return 0;
    }
    if( lpt->y < 0 || lpt->y > 0xffff )
    {
        return 0;
    }
    result->x = lpt->x;
    result->y = lpt->y;
    return 1;
}

/***************************************************************************
 **  DPtToAPt  Copy a double point to a aunit point. Return 1 if succesful
 **  ~~~~~~~~  or less than 1 if not.
 */

static INT DPtToAPt( A_PT* result, DA_PT* dpt )
{
    LA_PT lpt;
    INT ret;

    ret = DPtToLPt( &lpt, dpt );
    if( ret < 0 )
    {
        return ret;
    }
    return LPtToAPt( result, &lpt );
}

/***************************************************************************
 **  CalcDXPt  From a LXCALC struct, calculate the the intersect as a double
 **  ~~~~~~~~  point.
 */

static VOID CalcDXPt( DA_PT* dpt, LXCALC* calc )
{
    DXCALC xc, yc;

    LtoDXCalc( &xc, calc );
    CalcInterceptCo( &dpt->x, &xc );
    DtoDYCalc( &yc, &xc );
    CalcInterceptCo( &dpt->y, &yc );
}

/***************************************************************************
 **  GetIntersect  Calculate intersect of two lines. Return 1 if succesful
 **  ~~~~~~~~~~~~  or les than 1 if not.
 */


INT GetIntersect( A_PT* result, A_PT* aline, A_PT* bline )
{
    LXCALC c;
    DA_PT dpt;

    c.dxa = (LONG) aline[1].x - aline[0].x;
    c.dya = (LONG) aline[1].y - aline[0].y;
    c.xap = aline[0].x;
    c.yap = aline[0].y;
    c.dxb = (LONG) bline[1].x - bline[0].x;
    c.dyb = (LONG) bline[1].y - bline[0].y;
    c.xbp = bline[0].x;
    c.ybp = bline[0].y;
    CalcDXPt( &dpt, &c );
    return DPtToAPt( result, &dpt );
}

/***************************************************************************
 **  CalcArcCentreDPt  Calculate the centre of a 3 pt arc as a double pt
 **  ~~~~~~~~~~~~~~~~
 */

static INT CalcArcCentreDPt( DA_PT* result, EL_3PT* arc )
{
    LXCALC cxc;
    A_PT pt[4];
    INT ll, sl; /* Longest and shortest arc lines */
    AUNIT hyp[3];
    DOUBLE two;
    WORD i;

    pt[0] = arc->beg;
    pt[1] = arc->mid;
    pt[2] = arc->end;
    pt[3] = arc->beg;

    hyp[0] = kcHypotenuse( arc->beg.x - arc->mid.x, arc->beg.y - arc->mid.y );
    hyp[1] = kcHypotenuse( arc->mid.x - arc->end.x, arc->mid.y - arc->end.y );
    hyp[2] = kcHypotenuse( arc->end.x - arc->beg.x, arc->end.y - arc->beg.y );

    ll = 0;
    if( hyp[1] > hyp[0] ) ll = 1;
    if( hyp[2] > hyp[ll] ) ll = 2;

    sl = 2;
    if( hyp[1] < hyp[2] ) sl = 1;
    if( hyp[0] < hyp[sl] ) sl = 0;
    if( hyp[sl] == 0 ) return 0;

    cxc.dxa = (LONG) pt[ll+1].y - pt[ll].y;
    cxc.dya = (LONG) pt[ll].x - pt[ll+1].x;
    cxc.xap = (LONG) pt[ll].x + pt[ll+1].x;
    cxc.yap = (LONG) pt[ll].y + pt[ll+1].y;

    cxc.dxb = (LONG) pt[sl+1].y - pt[sl].y;
    cxc.dyb = (LONG) pt[sl].x - pt[sl+1].x;
    cxc.xbp = (LONG) pt[sl].x + pt[sl+1].x;
    cxc.ybp = (LONG) pt[sl].y + pt[sl+1].y;

    CalcDXPt( result, &cxc );
    i = 2;
    p_itof( &two, &i );
    p_fdiv( &result->x, &two );
    p_fdiv( &result->y, &two );
    return 1;
}

/***************************************************************************
 **  CalcArcCentreAPt  Calculate the centre of a 3 pt arc as a aunit pt
 **  ~~~~~~~~~~~~~~~~  Return 1 if successful, less than 1 if not.
 */

INT CalcArcCentreAPt( A_PT* result, EL_3PT* arc )
{
    DA_PT dpt;

    if( CalcArcCentreDPt( &dpt, arc ) == 0 ) return 0;
    return DPtToAPt( result, &dpt );
}

/***************************************************************************
 **  CalcArcCentreLPt  Calculate the centre of a 3 pt arc as a long pt
 **  ~~~~~~~~~~~~~~~~  Return 1 if successful, less than 1 if not.
 */

INT CalcArcCentreLPt( LA_PT* result, EL_3PT* arc )
{
    DA_PT dpt;

    if( CalcArcCentreDPt( &dpt, arc ) == 0 ) return 0;
    return DPtToLPt( result, &dpt );
}

/***************************************************************************
 **  CalcPerpendicularPt  Calculate the perpendicular point on a given line
 **  ~~~~~~~~~~~~~~~~~~~  from a given point. If OnLine is TRUE, then only
 **  return success if the point is found on the line.
 */

INT CalcPerpendicularPt( A_PT* result, A_PT* line, A_PT* pt, INT OnLine )
{
    LXCALC cxc;
    DA_PT dpt;
    INT ret;
    AUNIT x, xmax, xmin;

    cxc.dxa = (LONG) line[1].x - line[0].x;
    cxc.dya = (LONG) line[1].y - line[0].y;
    cxc.xap = line[0].x;
    cxc.yap = line[0].y;

    cxc.dxb = (LONG) line[0].y - line[1].y;
    cxc.dyb = (LONG) line[1].x - line[0].x;
    cxc.xbp = pt->x;
    cxc.ybp = pt->y;

    CalcDXPt( &dpt, &cxc );
    ret = DPtToAPt( result, &dpt );
    if( ret <= 0 || ! OnLine )
    {
        return ret;
    }
    x = result->x;
    if( line[0].x > line[1].x )
    {
        xmax = line[0].x;
        xmin = line[1].x;
    }
    else
    {
        xmax = line[1].x;
        xmin = line[0].x;
    }
    if( x > xmax || x < xmin )
    {
        return FALSE;
    }
    x = result->y;
    if( line[0].y > line[1].y )
    {
        xmax = line[0].y;
        xmin = line[1].y;
    }
    else
    {
        xmax = line[1].y;
        xmin = line[0].y;
    }
    if( x > xmax || x < xmin )
    {
        return FALSE;
    }
    return TRUE;
}

INT GetLCircleSeg( ANGLE* ang1, ANGLE* ang2, A_PT* line, LA_PT* cpt, LAUNIT* rad )
{
    LXCALC ic;
    DXCALC dc, dcy;
    DOUBLE t, r, p, c, sqr, sqp, px, py;
    DA_PT ppt;
    WORD i;
    LONG l;
    LONG sinA, cosA, sinB, cosB;
    AUNIT h;

    ic.dxa = (LONG) line[1].x - line[0].x;
    ic.dya = (LONG) line[1].y - line[0].y;
    ic.xap = line[0].x;
    ic.yap = line[0].y;

    ic.dxb = -ic.dya;
    ic.dyb = ic.dxa;
    ic.xbp = cpt->x;
    ic.ybp = cpt->y;

    /* Calc xp,yp = perpendicular to line thru circle centre */
    LtoDXCalc( &dc, &ic );
    CalcInterceptCo( &ppt.x, &dc );
    DtoDYCalc( &dcy, &dc );
    CalcInterceptCo( &ppt.y, &dcy );

    /* px = xp - xc */
    p_fld( &px, &ppt.x );
    p_fsub( &px, &dc.xbp );

    /* px = yp - yc */
    p_fld( &py, &ppt.y );
    p_fsub( &py, &dc.ybp );

    /* Calc sqp = sq( px ) + sq( py ) */
    p_fld( &sqp, &px );
    p_fmul( &sqp, &sqp );
    p_fld( &t, &py );
    p_fmul( &t, &t );
    p_fadd( &sqp, &t );

    /* Get sqr ( sq(rad ) ) */
    p_longtof( &r, rad );
    p_fld( &sqr, &r );
    p_fmul( &sqr, &sqr );
    if( p_fcmp( &sqr, &sqp ) < 0 ) /* r > p */
    {
        /* line doesn't cut circle */
        return FALSE;
    }
    p_sqrt( &p, &sqp );
    if( p_intl( &l, &p ) != 0 ) return FALSE;
    if( l == 0 )
    {
        /* Line goes thru centre */
        h = kcHypotenuse( (SWORD) ic.dxa, (SWORD) ic.dya );
        if( h == 0 ) return FALSE;
        ang1->sin = ( ic.dya * TRIG_DIV ) / h;
        ang1->cos = ( ic.dxa * TRIG_DIV ) / h;
        ang2->sin = - ang1->sin;
        ang2->cos = - ang1->cos;
        return TRUE;
    }
    if( l == *rad ) return FALSE;

    /* Calc c (half cord length) = sqrt( sq(rad) - sqp ) */
    p_fld( &c, &sqr );
    p_fsub( &c, &sqp );
    p_sqrt( &c, &c );

    /* calc angles */
    i = TRIG_DIV;
    p_itof( &t, &i );

    p_fmul( &py, &t );
    p_fdiv( &py, &p );
    if( p_intl( &sinA, &py ) != 0 ) return FALSE;

    p_fmul( &px, &t );
    p_fdiv( &px, &p );
    if( p_intl( &cosA, &px ) != 0 ) return FALSE;

    p_fmul( &c, &t );
    p_fdiv( &c, &r );
    if( p_intl( &sinB, &c ) != 0 ) return FALSE;

    p_fmul( &p, &t );
    p_fdiv( &p, &r );
    if( p_intl( &cosB, &p ) != 0 ) return FALSE;

    ang1->sin = ( sinA * cosB + cosA * sinB ) / TRIG_DIV;
    ang1->cos = ( cosA * cosB - sinA * sinB ) / TRIG_DIV;

    ang2->sin = ( sinA * cosB - cosA * sinB ) / TRIG_DIV;
    ang2->cos = ( cosA * cosB + sinA * sinB ) / TRIG_DIV;

    return TRUE;
}

INT LHypot( LAUNIT* result, LAUNIT* dx, LAUNIT* dy )
{
    DOUBLE d1, d2;

    p_longtof( &d1, dx );
    p_longtof( &d2, dy );

    p_fmul( &d1, &d1 );
    p_fmul( &d2, &d2 );
    p_fadd( &d1, &d2 );
    p_sqrt( &d1, &d1 );

    return p_intl( result, &d1 );
}


VOID GetAngleL( ANGLE* angle, LA_PT* cpt, A_PT* pt )
{
    DOUBLE cos, sin, div, k;
    WORD i;
    LONG l;

    l = pt->x;
    p_longtof( &cos, &l );
    p_longtof( &k, &cpt->x );
    p_fsub( &cos, &k );
    l = pt->y;
    p_longtof( &sin, &l );
    p_longtof( &k, &cpt->y );
    p_fsub( &sin, &k );

    p_fld( &div, &cos );
    p_fmul( &div, &div );
    p_fld( &k, &sin );
    p_fmul( &k, &k );
    p_fadd( &div, &k );
    p_sqrt( &div, &div );
    i = 0;
    p_itof( &k, &i );
    if( p_fcmp( &div, &k ) == 0 )
    {
        angle->sin = 0;
        angle->cos = TRIG_DIV;
        return;
    }
    i = TRIG_DIV;
    p_itof( &k, &i );
    p_fmul( &cos, &k );
    p_fdiv( &cos, &div );
    p_inti( &angle->cos, &cos );
    p_fmul( &sin, &k );
    p_fdiv( &sin, &div );
    p_inti( &angle->sin, &sin );
}

AUNIT CalcAngle( ANGLE* angle, A_PT* cpt, A_PT* pt )
{
    LONG cos, sin;
    AUNIT div;

    cos = (LONG) pt->x - cpt->x;
    sin = (LONG) pt->y - cpt->y;
    div = kcSquareRoot( cos * cos + sin * sin );
    if( div == 0 )
    {
        angle->sin = 0;
        angle->cos = TRIG_DIV;
    }
    else
    {
        angle->cos = ( cos * TRIG_DIV ) / div;
        angle->sin = ( sin * TRIG_DIV ) / div;
    }
    return div;
}

/* Return   1 if aa > ab,
**          0 if aa == ab,
**         -1 if aa < ab
*/

INT AngleCmp( ANGLE* aa, ANGLE* ab )
{
    UINT qa, qb;

    qa = 0;
    if( aa->cos < 0 ) qa = 1;
    if( aa->sin < 0 ) qa ^= 0x03;

    qb = 0;
    if( ab->cos < 0 ) qb = 1;
    if( ab->sin < 0 ) qb ^= 0x03;

    if( qa > qb ) return 1;
    if( qa < qb ) return -1;

    /* qa == qb */
    switch( qa )
    {
    case 0:
    case 3:
        if( aa->sin > ab->sin ) return 1;
        if( aa->sin < ab->sin ) return -1;
        break;
    case 1:
    case 2:
        if( aa->sin < ab->sin ) return 1;
        if( aa->sin > ab->sin ) return -1;
        break;
    }

    switch( qa )
    {
    case 0:
    case 1:
        if( aa->cos < ab->cos ) return 1;
        if( aa->cos > ab->cos ) return -1;
        break;
    case 2:
    case 3:
        if( aa->cos > ab->cos ) return 1;
        if( aa->cos < ab->cos ) return -1;
        break;
    }
    return 0;
}

INT IsAngleInArc( ANGLE* ang, ANGLE* beg, ANGLE* end )
{
    INT res;

    res = AngleCmp( end, beg );
    if( res > 0 )
    {
        if( AngleCmp( ang, beg ) < 0 || AngleCmp( ang, end ) > 0 )
        {
            return FALSE;
        }
    }
    if( res < 0 )
    {
        if( AngleCmp( ang, beg ) < 0 && AngleCmp( ang, end ) > 0 )
        {
            return FALSE;
        }
    }
    return TRUE;
}

INT Convert3PtArcToLArc( EL_LARC* arc, EL_3PT* arc3pt )
{
    LAUNIT dx, dy;
    ANGLE ang;

    if( CalcArcCentreLPt( &arc->centre, arc3pt ) != 1 )
    {
        return FALSE;
    }
    dx = arc->centre.x - (LONG) arc3pt->beg.x;
    dy = arc->centre.y - (LONG) arc3pt->beg.y;
    if( LHypot( &arc->radius, &dx, &dy ) < 0 )
    {
        return FALSE;
    }

    GetAngleL( &arc->beg, &arc->centre, &arc3pt->beg );
    GetAngleL( &ang, &arc->centre, &arc3pt->mid );
    GetAngleL( &arc->end, &arc->centre, &arc3pt->end );

    if( IsAngleInArc( &ang, &arc->beg, &arc->end ) == FALSE )
    {
        /* Swap the angles round */
        ang = arc->beg; arc->beg = arc->end; arc->end = ang;
    }
    return TRUE;
}

static INT CalcPtFromAngle( A_PT* result, A_PT* cpt, AUNIT rad, ANGLE* ang )
{
    LA_PT lpt;

    lpt.x = ( (LONG) ang->cos * rad ) / TRIG_DIV + cpt->x;
    lpt.y = ( (LONG) ang->sin * rad ) / TRIG_DIV + cpt->y;
    return LPtToAPt( result, &lpt );
}

INT CalcPtFromLAngle( A_PT* result, LA_PT* cpt, LAUNIT* rad, ANGLE* ang )
{
    DA_PT dpt;
    DOUBLE t, d;
    WORD i;

    i = TRIG_DIV;
    p_itof( &d, &i );
    p_longtof( &t, rad );
    p_fdiv( &t, &d );

    p_itof( &dpt.x, &ang->cos );
    p_fmul( &dpt.x, &t );
    p_longtof( &d, &cpt->x );
    p_fadd( &dpt.x, &d );

    p_itof( &dpt.y, &ang->sin );
    p_fmul( &dpt.y, &t );
    p_longtof( &d, &cpt->y );
    p_fadd( &dpt.y, &d );

    return DPtToAPt( result, &dpt );
}

INT ConvertLArcTo3PtArc( EL_3PT* arc3pt, EL_LARC* larc )
{
    ANGLE ang1, ang2;

    if( CalcPtFromLAngle( &arc3pt->beg, &larc->centre, &larc->radius, &larc->beg ) != 1 )
    {
        return FALSE;
    }
    if( CalcPtFromLAngle( &arc3pt->end, &larc->centre, &larc->radius, &larc->end ) != 1 )
    {
        return FALSE;
    }
    CalcAngle( &ang1, &arc3pt->end, &arc3pt->beg );
    ang2.sin = ang1.cos;
    ang2.cos = -ang1.sin;
    if( CalcPtFromLAngle( &arc3pt->mid, &larc->centre, &larc->radius, &ang2 ) != 1 )
    {
        return FALSE;
    }
    return TRUE;
}

INT GetArcRect( ARECT* pRect, ELEM* pEl )
{
    EL_LARC larc;
    ANGLE* pBeg;
    ANGLE* pEnd;
    ANGLE quad;
    LAUNIT lau;

    if( pEl->hdr.type == V_ARC )
    {
        CalcPtFromAngle( &pRect->pos,
            &pEl->arc.centre, pEl->arc.radius, &pEl->arc.beg );
        CalcPtFromAngle( &pRect->lim,
            &pEl->arc.centre, pEl->arc.radius, &pEl->arc.end );
        larc.centre.x = pEl->arc.centre.x;
        larc.centre.y = pEl->arc.centre.y;
        larc.radius = pEl->arc.radius;
        pBeg = &pEl->arc.beg;
        pEnd = &pEl->arc.end;
    }
    else /* V_3PT_ARC */
    {
        pRect->pos = pEl->el3pt.beg;
        pRect->lim = pEl->el3pt.end;
        if( Convert3PtArcToLArc( &larc, &pEl->el3pt ) == FALSE )
        {
            return FALSE;
        }
        pBeg = &larc.beg;
        pEnd = &larc.end;
    }
    NormaliseRect( pRect );
    quad.sin = 0;
    quad.cos = TRIG_DIV;
    if( IsAngleInArc( &quad, pBeg, pEnd ) )
    {
        lau = larc.centre.x + larc.radius;
        if( lau > 65535L ) lau = 65535L;
        pRect->lim.x = lau;
    }
    quad.sin = TRIG_DIV;
    quad.cos = 0;
    if( IsAngleInArc( &quad, pBeg, pEnd ) )
    {
        lau = larc.centre.y + larc.radius;
        if( lau > 65535L ) lau = 65535L;
        pRect->lim.y = lau;
    }
    quad.sin = 0;
    quad.cos = -TRIG_DIV;
    if( IsAngleInArc( &quad, pBeg, pEnd ) )
    {
        lau = larc.centre.x - larc.radius;
        if( lau < 0 ) lau = 0;
        pRect->pos.x = lau;
    }
    quad.sin = -TRIG_DIV;
    quad.cos = 0;
    if( IsAngleInArc( &quad, pBeg, pEnd ) )
    {
        lau = larc.centre.y - larc.radius;
        if( lau < 0 ) lau = 0;
        pRect->pos.y = lau;
    }
    return TRUE;
}

/* End of CALCARC.C */
