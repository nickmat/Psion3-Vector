/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: BUILD BREAK CLASS MEMBERS        *  Date Started:  3 Apr 1997  *
 *    File: VECBREAK.C      Type: C SOURCE   *  Date Revised: 24 Jul 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <hwim.h>
#include <p_math.h>
#include <vector.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"

VOID AddUndoUnmarked( ELEM* pBuf )
{
    UINT hand;

    pBuf->hdr.layer &= ~LAYER_FLAGS;
    pBuf->hdr.layer |= NEW_LAYER;
    hand = p_send3( Data, O_DD_ADD, pBuf );
    p_send3( Undo, O_UD_SAVE_DELETE, hand );
}

/***************************************************************************
 **  CheckEdge  Return TRUE if au is between, or equal to, au1 and au2.
 **  ~~~~~~~~~
 */

static INT CheckEdge( AUNIT au, AUNIT au1, AUNIT au2 )
{
    if( au1 < au2 )
    {
        if( au < au1 || au > au2 )
        {
            return FALSE;
        }
    }
    else
    {
        if( au < au2 || au > au1 )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/***************************************************************************
 **  CheckXPt  Checks the intersect point between a line and the break line.
 **  ~~~~~~~~  Returns one of the XPT_? defines.
 */

#define XPT_OFF_LINE  (-1)
#define XPT_ON_END_0    0
#define XPT_ON_END_1    1
#define XPT_ON_LINE     2

static INT CheckXPt( A_PT* ipt, A_PT* line, A_PT* bline )
{
    if( ! p_send5( Drg, O_DG_CALC_INTERSECT, ipt, line, bline ) )
    {
        return XPT_OFF_LINE;
    }
    if( CheckEdge( ipt->x, line[0].x, line[1].x ) == FALSE )
    {
        return XPT_OFF_LINE;
    }
    if( CheckEdge( ipt->y, line[0].y, line[1].y ) == FALSE )
    {
        return XPT_OFF_LINE;
    }
    if( ComparePt( ipt, &line[0] ) == 0 )
    {
        return XPT_ON_END_0;
    }
    if( ComparePt( ipt, &line[1] ) == 0 )
    {
        return XPT_ON_END_1;
    }
    return XPT_ON_LINE;
}

/***************************************************************************
 **  AngleInArc  Return true if pAng is in pLarc.
 **  ~~~~~~~~~~
 */

static INT AngleInArc( ANGLE* pAng, EL_LARC* pLarc )
{
    return p_send4( Drg, O_DG_ANGLE_IN_ARC, pAng, &pLarc->beg );
}

/***************************************************************************
 **  SaveArc  Save pLarc as a V_ARC or V_3PT_ARC.
 **  ~~~~~~~
 */

static VOID SaveArc( EL_LARC* pLarc, INT type )
{
    EL_ARC arc;
    EL_3PT arc3pt;

    if( type == V_ARC )
    {
        arc.hdr = pLarc->hdr;
        arc.centre.x = pLarc->centre.x;
        arc.centre.y = pLarc->centre.y;
        arc.radius = pLarc->radius;
        arc.beg = pLarc->beg;
        arc.end = pLarc->end;
        AddUndoUnmarked( (ELEM*) &arc );
        return;
    }
    /* Convert larc to 3pt_arc */
    if( ! p_send4( Drg, O_DG_LARC_TO_3PT, &arc3pt, pLarc ) )
    {
        /* roll back */
        return;
    }
    arc3pt.hdr = pLarc->hdr;
    AddUndoUnmarked( (ELEM*) &arc3pt );
}

/***************************************************************************
 **  BreakCircle  Break pElem into arcs. pElem may be a circle or arc
 **  ~~~~~~~~~~~
 */

static INT BreakCircle( ELEM* pElem, EL_LINE* pLine )
{
    INT type;
    EL_LARC larc;
    ANGLE aAng[2];
    ELEM* pElemBuf;
    EL_LARC arc1, arc2;

    type = pElem->hdr.type;
    if( type == V_3PT_ARC )
    {
        if( ! p_send4( Drg, O_DG_3PT_TO_LARC, &larc, pElem ) )
        {
            return FALSE;
        }
    }
    else
    {
        larc.centre.x = pElem->circle.centre.x;
        larc.centre.y = pElem->circle.centre.y;
        larc.radius = pElem->circle.radius;
        if( type == V_ARC )
        {
            larc.beg = pElem->arc.beg;
            larc.end = pElem->arc.end;
        }
    }
    larc.hdr = pElem->hdr;

    if( p_send5( Drg, O_DG_BREAK_CIRCLE, aAng, &larc, pLine ) > 0 )
    {
        if( type == V_CIRCLE )
        {
            pElemBuf = (ELEM*) DBuf;
            pElemBuf->circle = pElem->circle;
            pElemBuf->size = sizeof(EL_ARC);
            pElemBuf->hdr.type = V_ARC;
            pElemBuf->arc.beg = aAng[0];
            pElemBuf->arc.end = aAng[1];
            AddUndoUnmarked( pElemBuf );
            pElemBuf->arc.beg = aAng[1];
            pElemBuf->arc.end = aAng[0];
            AddUndoUnmarked( pElemBuf );
            return TRUE;
        }
        else  /* V_ARC and V_3PT_ARC */
        {
            if( AngleInArc( &aAng[0], &larc ) )
            {
                arc1 = larc;
                arc1.end = aAng[0];
                if( AngleInArc( &aAng[1], &arc1 ) )
                {
                    arc2 = arc1;
                    arc2.end = aAng[1];
                    SaveArc( &arc2, type );
                    arc1.beg = aAng[1];
                }
                SaveArc( &arc1, type );

                arc1 = larc;
                arc1.beg = aAng[0];
                if( AngleInArc( &aAng[1], &arc1 ) )
                {
                    arc2 = arc1;
                    arc2.end = aAng[1];
                    SaveArc( &arc2, type );
                    arc1.beg = aAng[1];
                }
                SaveArc( &arc1, type );
                return TRUE;
            }
            else if( AngleInArc( &aAng[1], &larc ) )
            {
                arc1 = larc;
                arc1.end = aAng[1];
                SaveArc( &arc1, type );

                arc1 = larc;
                arc1.beg = aAng[1];
                SaveArc( &arc1, type );
                return TRUE;
            }
        }
    }
    return FALSE;
}

/***************************************************************************
 **  SavePoly  Save pElem as a V_LINE or V_POLYLINE.
 **  ~~~~~~~~
 */

static VOID SavePoly( ELEM* pElem )
{
    if( pElem->size == sizeof(EL_LINE) )
    {
        pElem->hdr.type = V_LINE;
    }
    else
    {
        pElem->hdr.type = V_POLYLINE;
    }
    AddUndoUnmarked( pElem );
}

/***************************************************************************
 **  BreakPoly  Break pElem into polylines. pElem may be a polyline or
 **  ~~~~~~~~~  polygon.
 */

static INT BreakPoly( ELEM* buf, A_PT* data, INT size, EL_LINE* line )
{
    ELEM* nbuf;
    A_PT* ndata;
    INT i, j, k;
    A_PT ipt;
    INT xpt;
    INT cut = FALSE;

    nbuf = (ELEM*) p_send2( Drg, O_DG_GET_FBUF );
    ndata = &nbuf->line.beg;
    for( i = 0 ; i < size ; i++ )
    {
        xpt = CheckXPt( &ipt, &data[i], &line->beg );
        if( xpt == XPT_ON_LINE || xpt == XPT_ON_END_1 )
        {
            if( buf->hdr.type == V_POLYGON )
            {
                if( xpt == XPT_ON_LINE )
                {
                    ndata[0] = ipt;
                    j = 1;
                }
                else /* xpt == XPT_ON_END_1 */
                {
                    j = 0;
                }
                k = i + 1;
                while( j <= size )
                {
                    ndata[j++] = data[k++];
                    if( k == size )
                    {
                        k = 0;
                    }
                }
                if( xpt == XPT_ON_LINE )
                {
                    ndata[j] = ipt;
                    size++;
                }
                nbuf->hdr = buf->hdr;
                nbuf->size = sizeof(ELHDR) + sizeof(A_PT) * ( size + 1 );
                nbuf->hdr.type = V_POLYLINE;
                *buf = *nbuf;
                i = 0;
                continue;
            }
            *nbuf = *buf;
            if( xpt == XPT_ON_LINE )
            {
                ndata[i+1] = ipt;
            }
            else /* xpt == XPT_ON_END_1 */
            {
                if( i == size - 1 )
                {
                    break; /* Ignore a break at the end of polyline */
                }
            }
            nbuf->size = sizeof(ELHDR) + sizeof(A_PT) * ( i + 2 );
            SavePoly( nbuf );

            if( xpt == XPT_ON_LINE )
            {
                ndata[0] = ipt;
                j = 1;
            }
            else /* xpt == XPT_ON_END_1 */
            {
                j = 0;
            }
            k = i + 1;
            while( k <= size )
            {
                ndata[j++] = data[k++];
            }
            nbuf->size = sizeof(ELHDR) + sizeof(A_PT) * j;

            *buf = *nbuf;
            i = 0;
            size = j - 1;
            cut = TRUE;
        }
    }
    if( cut )
    {
        SavePoly( buf );
        return TRUE;
    }
    return FALSE;
}

/***************************************************************************
 **  BreakElement  If element is broken, adds new elements to store and
 **  ~~~~~~~~~~~~  returns TRUE (1), if element doesn't break, returns FALSE
 **  (0). If an error occurs returns a negative error code.
 */

#pragma save, ENTER_CALL

static INT BreakElement( ELEM* pElem, EL_LINE* line )
{
    A_PT* data;
    A_PT ipt;
    INT size;
    ELEM* pElemBuf1;

    pElemBuf1 = (ELEM*) DBuf;
    switch( pElem->hdr.type )
    {
    case V_LINE:
        if( CheckXPt( &ipt, &pElem->line.beg, &line->beg ) == XPT_ON_LINE )
        {
            pElemBuf1->line = pElem->line;
            pElemBuf1->line.end = ipt;
            AddUndoUnmarked( pElemBuf1 );

            pElemBuf1->line = pElem->line;
            pElemBuf1->line.beg = ipt;
            AddUndoUnmarked( pElemBuf1 );
            return TRUE;
        }
        break;
    case V_POLYLINE:
    case V_POLYGON:
        *pElemBuf1 = *pElem;
        data = &pElemBuf1->line.beg;
        size = ( pElem->size - sizeof(ELHDR) ) / sizeof(A_PT);
        if( pElem->hdr.type == V_POLYGON )
        {
            data[size] = data[0];
        }
        else
        {
            --size;
        }
        return BreakPoly( pElemBuf1, data, size, line );
    case V_CIRCLE:
    case V_ARC:
    case V_3PT_ARC:
        return BreakCircle( pElem, line );
    }
    return FALSE;
}

#pragma restore

/***************************************************************************
 **  DoBreak  Carry out break for all marked elements
 **  ~~~~~~~
 */

static VOID DoBreak( PR_EBREAK* self, EL_LINE* pLine )
{
    ELEM* pElem;
    INT count;
    INT msg;
    UINT hand;
    INT ret;

    pElem =  &self->build.el;
    count = 0;
    msg = SR_NOTHING_TO_BREAK;
    Rewind();
    BegUndo( SR_BREAK_UNDONE );
    for(;;)
    {
        hand = p_send3( Drg, O_DG_NEXT_MARKED, pElem );
        if( hand == EL_EOD ) break;
        p_send2( Undo, O_UD_MARK );
        ret = p_enter3( BreakElement, pElem, pLine );
        if( ret == 1 )
        {
            p_send4( Undo, O_UD_SAVE_INSERT, hand, pElem );
            p_send3( Data, O_DD_DELETE, hand );
            count++;
            msg = SR_BROKEN_COUNT_FMT;
        }
        else if( ret < 0 )
        {
            p_send3( Undo, O_UD_ROLL_BACK, UNDO_MARK );
            msg = SR_FAILED_BREAK_FMT;
            break;
        }
    }
    EndUndo();
    hInfoPrint( msg, count );
}

/***************************************************************************
 **  bld_init  Initiate a Select and Break build. Based on select.
 **  ~~~~~~~~
 */

#pragma METHOD_CALL

METHOD VOID ebreak_bld_init( PR_EBREAK* self )
{
    self->build.cmd = SR_BREAK;
    p_supersend2( self, O_BLD_INIT );
    self->select.step3 = SR_1ST_PT;
}

/***************************************************************************
 **  bld_step  Step method for Break build
 **  ~~~~~~~~
 */

METHOD VOID ebreak_bld_step( PR_EBREAK* self, AUNIT ax, AUNIT ay )
{
    EL_LINE line;

    switch( self->build.state )
    {
    case BLD_1ST:
    case BLD_2ND:
        SelectStep( (PR_SELECT*) self, ax, ay );
        break;
    case BLD_3RD:
        self->build.el.line.hdr.size = sizeof(EL_LINE);
        self->build.el.line.hdr.type = V_LINE;
        self->build.el.line.hdr.attr = 0;
        self->build.el.line.hdr.layer = BAND_LAYER | NEW_LAYER;
        self->build.el.line.beg.x = ax;
        self->build.el.line.beg.y = ay;
        hDestroy( Band );
        p_send3( Drg, O_DG_CREATE_BAND, C_BBREAK );
        p_send5( Band, O_BND_INIT, &self->build.el,
            &self->build.el.line.end, &self->build.el.line.beg );
        Drg->drg.StepID = SR_2ND_PT;
        self->build.state = BLD_4TH;
        break;
    case BLD_4TH:
        line = self->build.el.line;
        line.end.x = ax;
        line.end.y = ay;
        if( ComparePt( &line.beg, &line.end ) == 0 )
        {
            hInfoPrint( SR_MUST_DRAW_LINE );
            break;
        }
        hDestroy( Band );
        DoBreak( self, &line );
        p_send2( self, O_BLD_CANCEL );
        break;
    }
}

/***************************************************************************
 **  sel_filter  Filter to ignore elements that cannot be broken
 **  ~~~~~~~~~~
 */

METHOD INT ebreak_sel_filter( PR_EBREAK* self, ELEM* pEl )
{
    switch( pEl->hdr.type )
    {
    case V_CHARACTER:
    case V_SYMBOL:
    case V_GROUP:
    case V_TEXT:
    case V_LINK:
        return FALSE;
    }
    return TRUE;
}

/* End of VecBreak.c file */
