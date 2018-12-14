/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: JUMP TO DIALOGS CLASS MEMBERS    *  Date Started: 24 Mar 1997  *
 *    File: VECJUMP.C       Type: C SOURCE   *  Date Revised: 24 Mar 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <p_math.h>
#include <limits.h>
#include <hwim.h>
#include <edwin.g>
#include <chlist.g>
#include <vector.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"

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

static VOID ScaleToInt3( LONG* m1, LONG* m2, LONG* m3 )
{
    while( *m1 < INT_MIN || *m1 > INT_MAX
        || *m2 < INT_MIN || *m2 > INT_MAX
        || *m3 < INT_MIN || *m3 > INT_MAX )
    {
        *m1 = (*m1) >> 1;
        *m2 = (*m2) >> 1;
        *m3 = (*m3) >> 1;
    }
}

/***************************************************************************
 **  NearestCur  If the test point is closer to the cursur than best, update
 **  ~~~~~~~~~~  best and return TRUE, else return FALSE.
 */

static INT NearestCur( A_PT* test, AUNIT* best )
{
    INT dx, dy;
    AUNIT dist;

    dx = test->x - Drg->vec.Cur.x;
    dy = test->y - Drg->vec.Cur.y;
    /*dist = kcHypotenuse( dx, dy );*/
    dist = p_send4( Drg, O_DG_HYPOT, dx, dy );
    if( dist < *best )
    {
        *best = dist;
        return TRUE;
    }
    return FALSE;
}

/***************************************************************************
 **  PointInBound  Return TRUE if the point is within the boundary set by
 **  ~~~~~~~~~~~~  by two points, return FALSE if not.
 */

INT PointInBound( const A_PT* pt, const A_PT* bound )
{
    if( bound[0].x < bound[1].x )
    {
        if( pt->x < bound[0].x || pt->x > bound[1].x )
        {
            return FALSE;
        }
    }
    else
    {
        if( pt->x < bound[1].x || pt->x > bound[0].x )
        {
            return FALSE;
        }
    }
    if( bound[0].y < bound[1].y )
    {
        if( pt->y < bound[0].y || pt->y > bound[1].y )
        {
            return FALSE;
        }
    }
    else
    {
        if( pt->y < bound[1].y || pt->y > bound[0].y )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/***************************************************************************
 **  CheckLineEnd  Check the element for line end points that are in the
 **  ~~~~~~~~~~~~  rect and closer to the cursor than current best.
 **  Update best and copy point to pt if one is found.
 */

static VOID CheckLineEnd( ARECT* rect, A_PT* pt, AUNIT* pbest, ELEM* el )
{
    A_PT* data;
    INT i, size;
    A_PT aPt[2];

    switch( el->hdr.type )
    {
    case V_3PT_ARC:
        el->size = sizeof(EL_LINE);
        el->el3pt.mid = el->el3pt.end;
        /* Fall through */
    case V_LINE:
    case V_POLYLINE:
    case V_POLYGON:
        data = &el->line.beg;
        size = ( el->size - sizeof(ELHDR) ) / sizeof(A_PT);
        break;
    case V_ARC:
        /*GetPolarPt( &aPt[0], &el->arc.centre, &el->arc.beg, el->arc.radius );*/
        /*GetPolarPt( &aPt[1], &el->arc.centre, &el->arc.end, el->arc.radius );*/
        p_send5( Drg, O_DG_GET_POLAR_PT, &aPt[0], &el->arc.centre, &el->arc.beg );
        p_send5( Drg, O_DG_GET_POLAR_PT, &aPt[1], &el->arc.centre, &el->arc.end );
        data = aPt;
        size = 2;
        break;
    default:
        return;
    }
    for( i = 0 ; i < size ; i++ )
    {
        if( PointInRect( &data[i], rect ) && NearestCur( &data[i], pbest ) )
        {
            *pt = data[i];
        }
    }
}

/***************************************************************************
 **  CheckLineMid  Check the element for middle line points that are in the
 **  ~~~~~~~~~~~~  rect and closer to the cursor than current best.
 **  Update best and copy point to pt if one is found.
 */

static VOID CheckLineMid( ARECT* rect, A_PT* pt, AUNIT* pbest, ELEM* el )
{
    A_PT* data;
    INT i, size;
    A_PT mpt;

    switch( el->hdr.type )
    {
    case V_LINE:
    case V_POLYLINE:
    case V_POLYGON:
        data = &el->line.beg;
        size = ( el->size - sizeof(ELHDR) ) / sizeof(A_PT);
        if( el->hdr.type == V_POLYGON )
        {
            data[size] = data[0];
        }
        else
        {
            --size;
        }
        break;
    default:
        return;
    }
    for( i = 0 ; i < size ; i++ )
    {
        mpt.x = ( (LONG) data[i].x + data[i+1].x ) >> 1;
        mpt.y = ( (LONG) data[i].y + data[i+1].y ) >> 1;
        if( PointInRect( &mpt, rect ) && NearestCur( &mpt, pbest ) )
        {
            *pt = mpt;
        }
    }
}

/***************************************************************************
 **  PointOnLine  Determine if the point on the line is between, or on, the
 **  ~~~~~~~~~~~   beg and end points, return TRUE if it is.
 */

static INT UnitInRange( AUNIT au, AUNIT au1, AUNIT au2 )
{
    AUNIT tmp;

    if( au1 > au2 )
    {
        tmp = au1;
        au1 = au2;
        au2 = tmp;
    }
    if( au < au1 ) return FALSE;
    if( au > au2 ) return FALSE;
    return TRUE;
}

static INT PointOnLine( A_PT* pPt, A_PT* pPtLine )
{
    if( UnitInRange( pPt->x, pPtLine[0].x, pPtLine[1].x ) == FALSE )
    {
        return FALSE;
    }
    return UnitInRange( pPt->y, pPtLine[0].y, pPtLine[1].y );
}

/***************************************************************************
 **  FindIntersect  Find the best intersect with the line at the 2 point
 **  ~~~~~~~~~~~~~  array pPtLine. Fill pPtResult and return TRUE if found,
 **  or return FALSE if none found.
 */

INT FindIntersect( A_PT* pPtResult, A_PT* pPtLine, ARECT* pRect )
{
    UINT hand;
    INT ret;
    ELEM* pElem;
    A_PT* pPtData;
    INT i, size;
    A_PT pt;
    AUNIT auBest;

    ret = FALSE;
    pElem = (ELEM*) p_send2( Drg, O_DG_GET_FBUF );
    auBest = 0x7fff;
    hand = p_send2( Data, O_DD_POS );
    p_send2( Data, O_DD_REWIND );
    while( p_send3( Data, O_DD_NEXT, pElem ) != EL_EOD )
    {
        if( p_send2( Data, O_DD_POS ) == hand ) continue;
        switch( pElem->hdr.type )
        {
        case V_LINE:
        case V_POLYLINE:
        case V_POLYGON:
            pPtData = &pElem->line.beg;
            size = ( pElem->size - sizeof(ELHDR) ) / sizeof(A_PT);
            if( pElem->hdr.type == V_POLYGON )
            {
                pPtData[size] = pPtData[0];
            }
            else
            {
                --size;
            }
            break;
        default:
            continue;
        }
        for( i = 0 ; i < size ; i++ )
        {
            /*if( RectOverlap( (ARECT*) &pPtData[i], pRect ) == FALSE ) continue;*/
            if( p_send4( Drg, O_DG_RECT_OVERLAP, &pPtData[i], pRect ) == FALSE ) continue;
            /*if( GetIntersect( &pt, pPtLine, &pPtData[i] ) < 1 ) continue;*/
            if( p_send5( Drg, O_DG_CALC_INTERSECT, &pt, pPtLine, &pPtData[i] ) < 1 ) continue;
            if( PointInRect( &pt, pRect ) == FALSE ) continue;
            if( PointOnLine( &pt, &pPtData[i] ) == FALSE ) continue;
            if( PointOnLine( &pt, pPtLine ) == FALSE ) continue;
            if( NearestCur( &pt, &auBest ) == FALSE ) continue;
            *pPtResult = pt;
            ret = TRUE;
        }
    }
    p_send3( Data, O_DD_SET_POS, hand );
    return ret;
}

/***************************************************************************
 **  CheckIntersect  Check the element for an itersect point with another
 **  ~~~~~~~~~~~~~~  element, within the jump rect and closer to the cursor
 **  than current best. Update best and copy point to pt if one is found.
 */

static VOID CheckIntersect( ARECT* rect, A_PT* pt, AUNIT* pbest, ELEM* el )
{
    A_PT* data;
    INT i, size;
    A_PT mpt;

    switch( el->hdr.type )
    {
    case V_LINE:
    case V_POLYLINE:
    case V_POLYGON:
        data = &el->line.beg;
        size = ( el->size - sizeof(ELHDR) ) / sizeof(A_PT);
        if( el->hdr.type == V_POLYGON )
        {
            data[size] = data[0];
        }
        else
        {
            --size;
        }
        break;
    default:
        return;
    }
    for( i = 0 ; i < size ; i++ )
    {
        /*if( RectOverlap( (ARECT*) &data[i], rect ) == FALSE ) continue;*/
        if( p_send4( Drg, O_DG_RECT_OVERLAP, &data[i], rect ) == FALSE ) continue;
        if( FindIntersect( &mpt, &data[i], rect ) == FALSE ) continue;
        if( PointInRect( &mpt, rect ) && NearestCur( &mpt, pbest ) )
        {
            *pt = mpt;
        }
    }
}

/***************************************************************************
 **  CheckCentre  Check the element for centre points that are in the
 **  ~~~~~~~~~~~  rect and closer to the cursor than current best.
 **  Update best and copy point to pt if one is found.
 */

static VOID CheckCentre( ARECT* rect, A_PT* pt, AUNIT* pbest, ELEM* el )
{
    A_PT cpt;

    switch( el->hdr.type )
    {
    case V_CIRCLE:
    case V_ARC:
        cpt = el->circle.centre;
        break;
    case V_3PT_ARC:
        /*if( CalcArcCentreAPt( &cpt, &el->el3pt ) == FALSE )*/
        if( p_send4( Drg, O_DG_CALC_ARC_CNTR, &cpt, &el->el3pt ) == FALSE )
        {
            return;
        }
        break;
    default:
        return;
    }
    if( PointInRect( &cpt, rect ) && NearestCur( &cpt, pbest ) )
    {
        *pt = cpt;
    }
}

/***************************************************************************
 **  CheckQuadrant  Check the element for quadrant points that are in the
 **  ~~~~~~~~~~~~~  rect and closer to the cursor than current best.
 **  Update best and copy point to pt if one is found.
 */

static VOID CheckQuadrant( ARECT* rect, A_PT* pt, AUNIT* pbest, ELEM* el )
{
    A_PT cpt;
    AUNIT rad;
    A_PT qpt[4];
    int i;

    switch( el->hdr.type )
    {
    case V_CIRCLE:
    case V_ARC:
        cpt = el->circle.centre;
        rad = el->circle.radius;
        break;
    case V_3PT_ARC:
        /*if( CalcArcCentreAPt( &cpt, &el->el3pt ) == FALSE )*/
        if( p_send4( Drg, O_DG_CALC_ARC_CNTR, &cpt, &el->el3pt ) == FALSE )
        {
            return;
        }
        /*rad = kcHypotenuse( el->el3pt.beg.x - cpt.x, el->el3pt.beg.y - cpt.y );*/
        rad = p_send4( Drg, O_DG_HYPOT, el->el3pt.beg.x - cpt.x, el->el3pt.beg.y - cpt.y );
        break;
    default:
        return;
    }
    qpt[0].x = cpt.x + rad;
    qpt[0].y = cpt.y;
    qpt[1].x = cpt.x;
    qpt[1].y = cpt.y + rad;
    qpt[2].x = cpt.x - rad;
    qpt[2].y = cpt.y;
    qpt[3].x = cpt.x;
    qpt[3].y = cpt.y - rad;
    for( i = 0 ; i < 4 ; i++ )
    {
        if( PointInRect( &qpt[i], rect ) && NearestCur( &qpt[i], pbest ) )
        {
            *pt = qpt[i];
        }
    }
}

/***************************************************************************
 **  CheckPerp  Check the element for points that are perpendicular from the
 **  ~~~~~~~~~  last Enter point and that are in the rect and closer to the
 **  cursor than current best. Update best and copy point to pt if one is
 **  found.
 */

static VOID CheckPerp( ARECT* rect, A_PT* pt, AUNIT* pbest, ELEM* el )
{
    A_PT* data;
    INT i, size;
    A_PT ppt;
    A_PT  cpt;
    AUNIT rad;
    INT type;
    LONG dx, dy;
    AUNIT h;

    type = el->hdr.type;
    switch( type )
    {
    case V_LINE:
    case V_POLYLINE:
    case V_POLYGON:
        data = &el->line.beg;
        size = ( el->size - sizeof(ELHDR) ) / sizeof(A_PT);
        if( el->hdr.type == V_POLYGON )
        {
            data[size] = data[0];
        }
        else
        {
            --size;
        }
        type = V_LINE;
        break;
    case V_CIRCLE:
    case V_ARC:
        cpt = el->circle.centre;
        rad = el->circle.radius;
        break;
    case V_3PT_ARC:
        /*if( CalcArcCentreAPt( &cpt, &el->el3pt ) == FALSE )*/
        if( p_send4( Drg, O_DG_CALC_ARC_CNTR, &cpt, &el->el3pt ) == FALSE )
        {
            return;
        }
        /*rad = kcHypotenuse( el->el3pt.beg.x - cpt.x, el->el3pt.beg.y - cpt.y );*/
        rad = p_send4( Drg, O_DG_HYPOT, el->el3pt.beg.x - cpt.x, el->el3pt.beg.y - cpt.y );
        break;
    default:
        return;
    }
    if( type == V_LINE )
    {
        for( i = 0 ; i < size ; i++ )
        {
            /*CalcPerpendicularPt( &ppt, &data[i], &Drg->drg.LastPt, TRUE );*/
            if( p_send5( Drg, O_DG_CALC_PERP, &ppt, &data[i], &Drg->drg.LastPt ) )
            {
                if( PointInBound( &ppt, &data[i] ) == FALSE )
                {
                    continue;
                }
                if( PointInRect( &ppt, rect ) && NearestCur( &ppt, pbest ) )
                {
                    *pt = ppt;
                }
            }
        }
    }
    else
    {
        dx = (LONG) Drg->drg.LastPt.x - cpt.x;
        dy = (LONG) Drg->drg.LastPt.y - cpt.y;
        /*h = kcHypotenuse( (AUNIT) dx, (AUNIT) dy );*/
        h = p_send4( Drg, O_DG_HYPOT, (AUNIT) dx, (AUNIT) dy );
        if( h == 0 )
        {
            return;
        }
        /*ppt.x = cpt.x + ( dx * rad ) / h;*/
        /*ppt.y = cpt.y + ( dy * rad ) / h;*/
        dx = cpt.x + ( dx * rad );
        ppt.x = p_send4( Drg, O_DG_LONG_DIV_UINT, &dx, h );
        dy = cpt.y + ( dy * rad );
        ppt.y = p_send4( Drg, O_DG_LONG_DIV_UINT, &dy, h );
        if( PointInRect( &ppt, rect ) && NearestCur( &ppt, pbest ) )
        {
            *pt = ppt;
        }
    }
}

/***************************************************************************
 **  GetTangent  Get the tangent point if it exists and return TRUE, else
 **  ~~~~~~~~~~  return FALSE.
 */

static INT GetTangent( A_PT* tpt, A_PT* pt, A_PT* cpt, AUNIT radius )
{
    LONG dx, dy;
    LONG m1, m2, div;
    LONG tmp1, tmp2;

    dx = (LONG) cpt->x - pt->x;
    dy = (LONG) cpt->y - pt->y;
    div = dx * dx + dy * dy;
    m1 = div - (LONG) radius * radius;
    if( m1 < 0 )
    {
        return FALSE;
    }
    /*m2 = (LONG) kcSquareRoot( m1 ) * radius;*/
    m2 = (LONG) p_send3( Drg, O_DG_SQ_ROOT, &m1 ) * radius;
    ScaleToInt3( &m1, &m2, &div );
    if( div == 0 )
    {
        return FALSE;
    }
    /*tpt[0].x = pt->x + ( m1 * dx - m2 * dy ) / div;*/
    /*tpt[0].y = pt->y + ( m1 * dy + m2 * dx ) / div;*/
    /*tpt[1].x = pt->x + ( m1 * dx + m2 * dy ) / div;*/
    /*tpt[1].y = pt->y + ( m1 * dy - m2 * dx ) / div;*/

    /* LONG div bug work around */
    tmp1 = ( m1 * dx - m2 * dy );
    p_send5( Drg, O_DG_L_DIV_L_EQ_L, &tmp2, &tmp1, &div );
    tpt[0].x = pt->x + tmp2;

    tmp1 = ( m1 * dy + m2 * dx );
    p_send5( Drg, O_DG_L_DIV_L_EQ_L, &tmp2, &tmp1, &div );
    tpt[0].y = pt->y + tmp2;

    tmp1 = ( m1 * dx + m2 * dy );
    p_send5( Drg, O_DG_L_DIV_L_EQ_L, &tmp2, &tmp1, &div );
    tpt[1].x = pt->x + tmp2;

    tmp1 = ( m1 * dy - m2 * dx );
    p_send5( Drg, O_DG_L_DIV_L_EQ_L, &tmp2, &tmp1, &div );
    tpt[1].y = pt->y + tmp2;

    return TRUE;
}

/***************************************************************************
 **  CheckTangent  Check the element for points that are tangental from the
 **  ~~~~~~~~~~~~  last Enter point and that are in the rect and closer to
 **  the cursor than current best. Update best and copy point to pt if one
 **  is found.
 */

static VOID CheckTangent( ARECT* rect, A_PT* pt, AUNIT* pbest, ELEM* el )
{
    A_PT cpt;
    AUNIT rad;
    A_PT tpt[2];
    int i;

    switch( el->hdr.type )
    {
    case V_CIRCLE:
    case V_ARC:
        cpt = el->circle.centre;
        rad = el->circle.radius;
        break;
    case V_3PT_ARC:
        /*if( CalcArcCentreAPt( &cpt, &el->el3pt ) == FALSE )*/
        if( p_send4( Drg, O_DG_CALC_ARC_CNTR, &cpt, &el->el3pt ) == FALSE )
        {
            return;
        }
        /*rad = kcHypotenuse( el->el3pt.beg.x - cpt.x, el->el3pt.beg.y - cpt.y );*/
        rad = p_send4( Drg, O_DG_HYPOT, el->el3pt.beg.x - cpt.x, el->el3pt.beg.y - cpt.y );
        break;
    default:
        return;
    }
    if( GetTangent( tpt, &Drg->drg.LastPt, &cpt, rad ) )
    {
        for( i = 0 ; i < 2 ; i++ )
        {
            if( PointInRect( &tpt[i], rect ) && NearestCur( &tpt[i], pbest ) )
            {
                *pt = tpt[i];
            }
        }
    }
}

/***************************************************************************
 **  GetJumpNearRect  Get the rectangle to look for nearest point and ensure
 **  ~~~~~~~~~~~~~~~  that it is on the page.
 */

static VOID GetJumpNearRect( ARECT* arect, SRECT* srect )
{
    AUNIT size;

    size = ( Drg->vec.Nearest + 1 ) / 2;

    arect->pos.x = MAX( Drg->vec.Cur.x - size, Drg->vec.Page.pos.x );
    arect->pos.y = MAX( Drg->vec.Cur.y - size, Drg->vec.Page.pos.y );
    arect->lim.x = MIN( Drg->vec.Cur.x + size, Drg->vec.Page.lim.x );
    arect->lim.y = MIN( Drg->vec.Cur.y + size, Drg->vec.Page.lim.y );
}

/***************************************************************************
 **  JumpToNearest  Jump to the nearest point within the jump rect
 **  ~~~~~~~~~~~~~
 */

VOID JumpToNearest( INT nearest )
{
    ARECT arect;
    SRECT srect;
    A_PT pt;
    UINT hand;
    AUNIT best = 0x7fff;

    hBusyPrint( 2, SR_SEARCHING );
    GetJumpNearRect( &arect, &srect );
    p_send2( Data, O_DD_REWIND );
    for(;;)
    {
        hand = p_send3( Data, O_DD_NEXT, DBuf );
        if( hand == EL_EOD ) break;
        if( DBuf[TYPE_BYTE] & V_AGGR_BIT ) continue;
        /*if( IsLayerOn( DBuf[LAYER_BYTE] ) != BL_BLACK ) continue;*/
        if( p_send3( Drg, O_DG_IS_LAYER_ON, DBuf[LAYER_BYTE] ) != BL_BLACK ) continue;
        switch( nearest )
        {
        case NEAR_LINE_END:
            CheckLineEnd( &arect, &pt, &best, (ELEM*) DBuf );
            break;
        case NEAR_LINE_MID:
            CheckLineMid( &arect, &pt, &best, (ELEM*) DBuf );
            break;
        case NEAR_INTERSECT:
            CheckIntersect( &arect, &pt, &best, (ELEM*) DBuf );
            break;
        case NEAR_CENTRE:
            CheckCentre( &arect, &pt, &best, (ELEM*) DBuf );
            break;
        case NEAR_QUADRANT:
            CheckQuadrant( &arect, &pt, &best, (ELEM*) DBuf );
            break;
        case NEAR_PERP:
            CheckPerp( &arect, &pt, &best, (ELEM*) DBuf );
            break;
        case NEAR_TANGENT:
            CheckTangent( &arect, &pt, &best, (ELEM*) DBuf );
            break;
        }
    }
    wCancelBusyMsg();
    if( best == 0x7fff )
    {
        hInfoPrint( SR_NO_JUMP_FOUND );
    }
    else
    {
        p_send4( Drg, O_DG_MOVE_CURSOR, pt.x, pt.y );
        Drg->vec.OffGrid = TRUE;
    }
}

/*--------------------------[ jmpdlg - Jump Dialog ]----------------------*/

#pragma METHOD_CALL

/***************************************************************************
 **  dl_dyn_init  Initiate Symbol Settings Dialog
 **  ~~~~~~~~~~~
 */

METHOD VOID jmpdlg_dl_dyn_init( PR_JMPDLG* self )
{
    JMPDLG_DATA* data;
    TEXT bufx[DIM_TEXT_MAX_Z];
    TEXT bufy[DIM_TEXT_MAX_Z];
    TEXT buf[40];
    A_PT* pt;
    LONG lg;

    data = self->dlgbox.rbuf;

    p_send3( Drg, O_DG_DLG_TITLE_S, SR_JUMP_TITLE_FMT );

    if( data->flags & JMP_RELATIVE )
    {
        pt = data->pPt;
        lg = (LONG) pt->x - Drg->vec.Orig.x;
        p_send4( Drg, O_DG_LAU_TO_S_TEXT, bufx, &lg );
        lg = (LONG) pt->y - Drg->vec.Orig.y;
        if( Drg->vec.yDir )
        {
            lg = -lg;
        }
        p_send4( Drg, O_DG_LAU_TO_S_TEXT, bufy, &lg );
        hAtos( buf, SR_JUMP_REL_FMT, bufx, bufy );
        hDlgSetText( 3, buf );
    }
}

/***************************************************************************
 **  dl_key  Complete Symbol Settings Dialog
 **  ~~~~~~
 */

METHOD INT jmpdlg_dl_key( PR_JMPDLG* self, INT index, INT keycode, INT actbut )
{
    JMPDLG_DATA* data;
    R_PT rpt;
    A_PT pt;
    AUNIT radius;
    DOUBLE angle;
    ANGLE a;
    LAUNIT lau;

    /* We only get here if Enter is pressed */
    data = self->dlgbox.rbuf;

    if( data->flags & JMP_POLAR )
    {
        radius = p_send3( Drg, O_DG_S_TEXT_TO_AU, hDlgSenseEdwin( 1 ) );
        hDlgSenseFledit( 2, &angle );
        p_send4( Drg, O_DG_SET_ANGLE, &a, &angle );
        p_send5( Drg, O_DG_REL_POLAR_PT, &rpt, &a, radius );
        pt.x = data->pPt->x + rpt.x;
        pt.y = data->pPt->y + rpt.y;
    }
    else /* x,y */
    {
        p_send4( Drg, O_DG_DLG_GET_S_LAU, 1, &lau );
        pt.x = lau + data->pPt->x;

        p_send4( Drg, O_DG_DLG_GET_S_LAU, 2, &lau );
        lau = Drg->vec.yDir ? -lau : lau;
        pt.y = lau + data->pPt->y;
    }

    if( ! PointInRect( &pt, &Drg->vec.Page ) )
    {
        hInfoPrint( SR_POINT_OFF_PAGE );
        return WN_KEY_NO_CHANGE;
    }

    p_send4( Drg, O_DG_MOVE_CURSOR, pt.x, pt.y );
    Drg->vec.OffGrid = TRUE;

    return WN_KEY_CHANGED;
}

/***************************************************************************
 **  ex_jump_nearest
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID extra_ex_jump_near( PR_EXTRA* self, INT nearest )
{
    JumpToNearest( nearest );
}

/***************************************************************************
 **  ex_jump_dialog
 **  ~~~~~~~~~~~~~~
 */

METHOD VOID extra_ex_jump_dialog( PR_EXTRA* self, INT dialog, A_PT* pPt, UINT flags )
{
    JMPDLG_DATA data;

    data.flags = flags;
    data.pPt = pPt;

    BldLaunchDialog( &data, dialog, C_JMPDLG );
}

/* End of VECTDLG.C file */
