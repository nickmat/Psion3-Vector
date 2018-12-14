/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: BUILD ELEMENT CLASS MEMBERS      *  Date Started: 24 Sep 1996  *
 *    File: VECBUILD.C      Type: C SOURCE   *  Date Revised:  1 Jun 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1996, 1997, Nick Matthews
*/

#include <hwim.h>
#include <p_math.h>
#include <vector.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"

/***************************************************************************
 **  General helper functions
 **  ~~~~~~~~~~~~~~~~~~~~~~~~
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

INT ComparePt( A_PT* pt1, A_PT* pt2 )
{
    if( pt1->x == pt2->x && pt1->y == pt2->y )
    {
        return 0;
    }
    return 1;
}

/***************************************************************************
 **  AddToStore  Save the element buffer to store. If sucessful, updates
 **  ~~~~~~~~~~  drawing and undo and returns 0. If out of memory, calls
 **  p_leave( E_GEN_NOMEMORY ).
 */

#pragma save, ENTER_CALL

INT AddToStore( VOID* pBuf )
{
    UINT hand;

    hand = p_send3( Data, O_DD_ADD, pBuf );
    ((UBYTE*)pBuf)[LAYER_BYTE] &= ~LAYER_FLAGS;
    p_send3( Drg, O_DG_DRAW_EL, pBuf );
    p_send3( Undo, O_UD_SAVE_DELETE, hand );
    return 0;
}

#pragma restore

/***************************************************************************
 **  InsertInStore  Insert the element buffer in store. If sucessful,
 **  ~~~~~~~~~~~~~  updates undo and returns 0. If out of memory, calls
 **  p_leave( E_GEN_NOMEMORY ).
 */

#pragma save, ENTER_CALL

INT InsertInStore( UINT pos, VOID* pBuf )
{
    p_send4( Data, O_DD_INSERT, pos, pBuf );
    p_send3( Undo, O_UD_SAVE_DELETE, pos );
    return 0;
}

#pragma restore

/***************************************************************************
 **  BldLaunchDialog  Launch a build module dialog box and return the result
 **  ~~~~~~~~~~~~~~~
 */

INT BldLaunchDialog( VOID* buf, INT id, INT class )
{
    DL_DATA dial_data;

    dial_data.id = id;
    dial_data.rbuf = buf;
    dial_data.pdlg = NULL;
    return hLaunchDial( CAT_VECBLD_VECBLD, class, &dial_data );
}

/***************************************************************************
 **  bld_back3  Step back one step if element is a 3 step build
 **  ~~~~~~~~~
 */

static VOID bld_back3( PR_BUILD* self )
{
    if( self->build.state > BLD_2ND )
    {
        BegDraw();
        self->build.state = BLD_2ND;
        Drg->drg.StepID = self->build.step2;
        EndDraw();
        return;
    }
    p_send2( self, O_BLD_CANCEL );
}

/*---------------------------[ build - base class ]-----------------------*/

#pragma METHOD_CALL

/***************************************************************************
 **  destroy  If the build object wasn't a temporary one, reset to viewing
 **  ~~~~~~~  mode.
 */

METHOD VOID build_destroy( PR_BUILD* self )
{
    if( self->build.temp == FALSE )
    {
        hDestroy( Band );
        Build = NULL;
        Drg->drg.CmdID = SR_VIEWING;
        Drg->drg.StepID = SR_BLANK;
        p_send3( w_ws->wserv.cli, O_DW_SET_TEXT, FALSE );
    }
    p_supersend2( self, O_DESTROY );
}

/***************************************************************************
 **  bld_cancel  Put back to 1st state, or if already there, destroy self.
 **  ~~~~~~~~~~
 */

METHOD VOID build_bld_cancel( PR_BUILD* self )
{
    hDestroy( Band );
    if( self->build.state == BLD_1ST )
    {
        p_send2( self, O_DESTROY );
        return;
    }
    p_send3( w_ws->wserv.cli, O_DW_SET_TEXT, FALSE );
    self->build.state = BLD_1ST;
    Drg->drg.StepID = self->build.step1;
}

/***************************************************************************
 **  bld_step  Carry out the build steps for creating a simple element.
 **  ~~~~~~~~  Based on V_LINE but works for others.
 */

METHOD VOID build_bld_step( PR_BUILD* self, AUNIT ax, AUNIT ay )
{
    switch( self->build.state )
    {
    case BLD_1ST:
        self->build.el.line.hdr.size = sizeof(EL_LINE);
        self->build.el.line.hdr.type = self->build.eltype;
        self->build.el.line.hdr.attr = 0;
        self->build.el.line.hdr.layer = BAND_LAYER | NEW_LAYER;
        self->build.el.line.beg.x = ax;
        self->build.el.line.beg.y = ay;
        hDestroy( Band );
        p_send3( Drg, O_DG_CREATE_BAND, C_BAND );
        p_send5( Band, O_BND_INIT, &self->build.el,
            &self->build.el.line.end, &self->build.el.line.beg );
        Drg->drg.StepID = self->build.step2;
        self->build.state = BLD_2ND;
        break;
    case BLD_2ND:
        p_send2( self, O_BLD_SAVE );
        break;
    }
}

/***************************************************************************
 **  bld_save  Save a single element
 **  ~~~~~~~~
 */

METHOD VOID build_bld_save( PR_BUILD* self )
{
    ELEM* pel = &self->build.el;
    UINT hand;

    pel->hdr.attr = Drg->vec.Attr;
    pel->hdr.layer = Drg->vec.Layer | NEW_LAYER;
    BegDraw();
    hDestroy( Band );
    p_send3( Undo, O_UD_SET_MSG, self->build.UndoMsg );
    if( p_enter2( AddToStore, pel ) )
    {
        hInfoPrint( SRM_NO_MEMORY );
    }
    else
    {
        Rewind();
        for(;;)
        {
            hand = p_send3( Data, O_DD_NEXT, DBuf );
            if( hand == EL_EOD ) break;
            if( DBuf[LAYER_BYTE] & LAYER_FLAGS )
            {
                if( DBuf[LAYER_BYTE] & NEW_LAYER )
                {
                    DBuf[LAYER_BYTE] &= ~LAYER_FLAGS;
                    DBuf[LAYER_BYTE] |= PREV_LAYER;
                }
                else
                {
                    DBuf[LAYER_BYTE] &= ~LAYER_FLAGS;
                }
                p_send4( Data, O_DD_REPLACE, hand, DBuf );
            }
        }
    }
    EndDraw();
    build_bld_cancel( self );
}

/*--------------------------[ nline - Single Line ]-----------------------*/

/***************************************************************************
 **  bld_init  Initiate a build line element. Based on build.
 **  ~~~~~~~~
 */

METHOD VOID nline_bld_init( PR_NLINE* self )
{
    self->build.state = BLD_1ST;
    self->build.cmd = Drg->drg.CmdID = SR_LINE;
    self->build.step1 = Drg->drg.StepID = SR_1ST_PT;
    self->build.step2 = SR_2ND_PT;
    self->build.eltype = V_LINE;
    self->build.UndoMsg = SR_LINE_UNDONE;
}

/*---------------------------[ npline - Polyline ]------------------------*/

/***************************************************************************
 **  bld_init  Initiate a build polyline element. Based on nline.
 **  ~~~~~~~~
 */

METHOD VOID npline_bld_init( PR_NPLINE* self )
{
    p_supersend2( self, O_BLD_INIT );
    self->build.cmd = Drg->drg.CmdID = SR_POLYLINE;
    self->npline.polytype = V_POLYLINE;
    self->build.UndoMsg = SR_POLYLINE_UNDONE;
}

/***************************************************************************
 **  bld_step  Carry out the build steps for creating a multiple point
 **  ~~~~~~~~  element. Based on V_POLYLINE but works for others.
 */

METHOD VOID npline_bld_step( PR_NPLINE* self, AUNIT ax, AUNIT ay )
{
    A_PT* pt;

    switch( self->build.state )
    {
    case BLD_1ST:
        self->npline.count = 0;
        p_supersend4( self, O_BLD_STEP, ax, ay );
        return;
    case BLD_2ND:
        self->build.el.pline.hdr.size = sizeof(ELHDR) + sizeof(A_PT) * 3;
        self->build.el.pline.hdr.type = self->npline.polytype;
        self->npline.count = 1;
        pt = &self->build.el.pline.pt[0];
        p_send3( Band, O_BND_SET_NEXT, &pt[1] );
        Drg->drg.StepID = SR_NEXT_PT;
        self->build.state = BLD_NEXT;
        break;
    case BLD_NEXT:
        if( ( ax == self->build.el.pline.pt[self->npline.count-1].x
            && ay == self->build.el.pline.pt[self->npline.count-1].y )
            || self->npline.count == POINTS_MAX )
        {
            /* Entered twice on same point or buffer full so end op */
            self->build.el.pline.hdr.size -= sizeof(A_PT);
            if( self->npline.count == 1 )
            {
                self->build.el.pline.hdr.type = V_LINE;
            }
            p_send2( self, O_BLD_SAVE );
            return;
        }
        pt = &self->build.el.pline.pt[self->npline.count];
        p_send3( Band, O_BND_SET_NEXT, &pt[1] );
        self->build.el.pline.hdr.size += sizeof(A_PT);
        self->npline.count++;
        break;
    }
    pt->x = ax;
    pt->y = ay;
}

/***************************************************************************
 **  bld_back  Step back one step if element is a multiple step build
 **  ~~~~~~~~
 */

METHOD VOID npline_bld_back( PR_NPLINE* self )
{
    A_PT* pt;

    if( self->build.state == BLD_NEXT )
    {
        BegDraw();
        --self->npline.count;
        if( self->npline.count == 0 )
        {
            Drg->drg.StepID = self->build.step2;
            self->build.state = BLD_2ND;
        }
        pt = &self->build.el.pline.pt[self->npline.count];
        pt[0] = pt[1];
        self->build.el.pline.hdr.size -= sizeof(A_PT);
        p_send3( Band, O_BND_SET_NEXT, &pt[0] );
        EndDraw();
        return;
    }
    p_supersend2( self, O_BLD_BACK );
}

/*----------------------------[ npgon - Polygon ]-------------------------*/

/***************************************************************************
 **  bld_init  Initiate a build polygon element. Based on npline.
 **  ~~~~~~~~
 */

METHOD VOID npgon_bld_init( PR_NPGON* self )
{
    p_supersend2( self, O_BLD_INIT );
    self->build.cmd = Drg->drg.CmdID = SR_POLYGON;
    self->npline.polytype = V_POLYGON;
    self->build.UndoMsg = SR_POLYGON_UNDONE;
}

/*----------------------------[ nbox - Rectangle ]------------------------*/

/***************************************************************************
 **  bld_init  Initiate a build box polygon element. Based on build.
 **  ~~~~~~~~
 */

METHOD VOID nbox_bld_init( PR_NBOX* self )
{
    self->build.state = BLD_1ST;
    self->build.cmd = Drg->drg.CmdID = SR_BOX;
    self->build.step1 = Drg->drg.StepID = SR_1ST_CORNER;
    self->build.step2 = SR_2ND_CORNER;
    self->build.eltype = V_BOX;
    self->build.UndoMsg = SR_BOX_UNDONE;
}

/***************************************************************************
 **  bld_save  Convert box to polygon element and save.
 **  ~~~~~~~~
 */

METHOD VOID nbox_bld_save( PR_BUILD* self )
{
    self->build.el.pline.hdr.size = sizeof(ELHDR) + sizeof(A_PT) * 4;
    self->build.el.pline.hdr.type = V_POLYGON;
    self->build.el.pline.pt[1] = self->build.el.pline.pt[0];
    self->build.el.pline.pt[0].x = self->build.el.pline.beg.x;
    self->build.el.pline.pt[2].x = self->build.el.pline.pt[1].x;
    self->build.el.pline.pt[2].y = self->build.el.pline.beg.y;
    p_supersend2( self, O_BLD_SAVE );
}

/*-------------------------[ ncircle - Draw Circle ]----------------------*/

/***************************************************************************
 **  bld_init  Initiate a build circle element. Based on build.
 **  ~~~~~~~~
 */

METHOD VOID ncircle_bld_init( PR_NCIRCLE* self )
{
    self->build.state = BLD_1ST;
    self->build.cmd = Drg->drg.CmdID = SR_CIRCLE;
    self->build.step1 = Drg->drg.StepID = SR_CENTRE_PT;
    self->build.UndoMsg = SR_CIRCLE_UNDONE;
}

/***************************************************************************
 **  bld_step  Carry out the build steps for creating circle element.
 **  ~~~~~~~~
 */

METHOD VOID ncircle_bld_step( PR_NCIRCLE* self, AUNIT ax, AUNIT ay )
{
    EL_CIRCLE* pCir;

    pCir = (EL_CIRCLE*) &self->build.el;
    switch( self->build.state )
    {
    case BLD_1ST:
        pCir->hdr.size = sizeof(EL_CIRCLE);
        pCir->hdr.type = V_CIRCLE;
        pCir->hdr.attr = 0;
        pCir->hdr.layer = BAND_LAYER | NEW_LAYER;
        pCir->centre.x = ax;
        pCir->centre.y = ay;
        hDestroy( Band );
        p_send3( Drg, O_DG_CREATE_BAND, C_BCIRCLE );
        p_send5( Band, O_BND_INIT, pCir, &Band->band.cpt, &pCir->centre );
        Drg->drg.StepID = SR_CIRCLE_PT;
        self->build.state = BLD_2ND;
        break;
    case BLD_2ND:
        p_send2( self, O_BLD_SAVE );
        break;
    }

#if 0
    CIRCLE_BAND* el;

    el = (CIRCLE_BAND*) &self->build.el;
    switch( self->build.state )
    {
    case BLD_1ST:
        el->circle.hdr.size = sizeof(EL_CIRCLE);
        el->circle.hdr.type = V_CIRCLE;
        el->circle.hdr.attr = 0;
        el->circle.hdr.layer = BAND_LAYER | NEW_LAYER;
        el->circle.centre.x = ax;
        el->circle.centre.y = ay;
        hDestroy( Band );
        p_send3( Drg, O_DG_CREATE_BAND, C_BCIRCLE );
        p_send5( Band, O_BND_INIT, el, &el->cpt, &el->circle.centre );
        Drg->drg.StepID = SR_CIRCLE_PT;
        self->build.state = BLD_2ND;
        break;
    case BLD_2ND:
        p_send2( self, O_BLD_SAVE );
        break;
    }
#endif
}

/*----------------------[ narc - Draw Arc (centre) ]---------------------*/

/***************************************************************************
 **  bld_init  Initiate a build arc (centred) element. Based on build.
 **  ~~~~~~~~
 */

METHOD VOID narc_bld_init( PR_NARC* self )
{
    self->build.state = BLD_1ST;
    self->build.cmd = Drg->drg.CmdID = SR_ARC_CENTRE;
    self->build.step1 = Drg->drg.StepID = SR_CENTRE_PT;
    self->build.step2 = SR_ARC_START;
    self->build.eltype = V_LINE; /* used for initial start line */
    self->build.UndoMsg = SR_ARC_CNTR_UNDONE;
}

/***************************************************************************
 **  bld_step  Carry out the build steps for creating a centred arc element.
 **  ~~~~~~~~
 */

METHOD VOID narc_bld_step( PR_NARC* self, AUNIT ax, AUNIT ay )
{
    EL_ARC* pArc;
    A_PT pt;

    pArc = (EL_ARC*) &self->build.el;
    switch( self->build.state )
    {
    case BLD_1ST:
        p_supersend4( self, O_BLD_STEP, ax, ay );
        break;
    case BLD_2ND:
        pt.x = ax;
        pt.y = ay;
        pArc->hdr.size = sizeof(EL_ARC);
        pArc->hdr.type = V_ARC;
        pArc->radius = p_send4( Drg, O_DG_HYPOT, ax - pArc->centre.x, ay - pArc->centre.y );
        p_send5( Drg, O_DG_CALC_ANGLE, &pArc->beg, &pArc->centre, &pt );
        BegDraw();
        hDestroy( Band );
        p_send3( Drg, O_DG_CREATE_BAND, C_BARC );
        p_send5( Band, O_BND_INIT, pArc, &Band->band, &pArc->centre );
        Band->band.pt = pt;
        EndDraw();
        Drg->drg.StepID = SR_ARC_ANGLE;
        self->build.state = BLD_3RD;
        break;
    case BLD_3RD:
        p_send2( self, O_BLD_SAVE );
        break;
    }

#if 0
    ARC_BAND* el;

    el = (ARC_BAND*) &self->build.el;
    switch( self->build.state )
    {
    case BLD_1ST:
        p_supersend4( self, O_BLD_STEP, ax, ay );
        break;
    case BLD_2ND:
        el->arc.hdr.size = sizeof(EL_ARC);
        el->arc.hdr.type = V_ARC;
        el->pt.x = ax;
        el->pt.y = ay;
        el->arc.radius = p_send4( Drg, O_DG_HYPOT,
            ax - el->arc.centre.x, ay - el->arc.centre.y );
        p_send5( Drg, O_DG_CALC_ANGLE,
            &el->arc.beg, &el->arc.centre, &el->pt );
        BegDraw();
        hDestroy( Band );
        p_send3( Drg, O_DG_CREATE_BAND, C_BARC );
        p_send5( Band, O_BND_INIT, el, &el->pt, &el->arc.centre );
        EndDraw();
        Drg->drg.StepID = SR_ARC_ANGLE;
        self->build.state = BLD_3RD;
        break;
    case BLD_3RD:
        p_send2( self, O_BLD_SAVE );
        break;
    }
#endif
}

/***************************************************************************
 **  bld_back  Step back one step for centred arc.
 **  ~~~~~~~~
 */

METHOD VOID narc_bld_back( PR_NARC* self )
{
    EL_LINE* pLine;

    if( self->build.state < BLD_3RD )
    {
        p_supersend2( self, O_BLD_BACK );
        return;
    }
    BegDraw();
    pLine = (EL_LINE*) &self->build.el;
    pLine->hdr.size = sizeof(EL_LINE);
    pLine->hdr.type = V_LINE;
    hDestroy( Band );
    p_send3( Drg, O_DG_CREATE_BAND, C_BAND );
    p_send5( Band, O_BND_INIT, pLine, &pLine->end, &pLine->beg );
    bld_back3( (PR_BUILD*) self );
    EndDraw();

#if 0
    ARC_BAND* el;

    if( self->build.state < BLD_3RD )
    {
        p_supersend2( self, O_BLD_BACK );
        return;
    }
    BegDraw();
    el = (ARC_BAND*) &self->build.el;
    el->arc.hdr.size = sizeof(EL_LINE);
    el->arc.hdr.type = V_LINE;
    hDestroy( Band );
    p_send3( Drg, O_DG_CREATE_BAND, C_BAND );
    p_send5( Band, O_BND_INIT, &self->build.el,
        &self->build.el.line.end, &self->build.el.line.beg );
    bld_back3( (PR_BUILD*) self );
    EndDraw();
#endif
}

/*-----------------------[ n3parc - Draw 3 Point Arc ]--------------------*/

/***************************************************************************
 **  bld_init  Initiate a build 3 point arc element. Based on nline.
 **  ~~~~~~~~
 */

METHOD VOID n3parc_bld_init( PR_N3PARC* self )
{
    p_supersend2( self, O_BLD_INIT );
    self->build.cmd = Drg->drg.CmdID = SR_ARC_3PT;
    self->build.UndoMsg = SR_ARC_3PT_UNDONE;
}

/***************************************************************************
 **  bld_step  Carry out the build steps for creating a 3 point arc element.
 **  ~~~~~~~~
 */

METHOD VOID n3parc_bld_step( PR_N3PARC* self, AUNIT ax, AUNIT ay )
{
    switch( self->build.state )
    {
    case BLD_1ST:
        p_supersend4( self, O_BLD_STEP, ax, ay );
        break;
    case BLD_2ND:
        self->build.el.el3pt.hdr.size = sizeof(EL_3PT);
        self->build.el.el3pt.hdr.type = V_3PT_ARC;
        self->build.el.el3pt.end = self->build.el.el3pt.mid;
        Drg->drg.StepID = SR_MID_PT;
        self->build.state = BLD_3RD;
        break;
    case BLD_3RD:
        p_send2( self, O_BLD_SAVE );
        break;
    }
}

/***************************************************************************
 **  bld_back  Step back one step for 3 point arc.
 **  ~~~~~~~~
 */

METHOD VOID n3parc_bld_back( PR_N3PARC* self )
{
    if( self->build.state < BLD_3RD )
    {
        p_supersend2( self, O_BLD_BACK );
        return;
    }
    BegDraw();
    self->build.el.el3pt.hdr.size = sizeof(EL_LINE);
    self->build.el.el3pt.hdr.type = V_LINE;
    self->build.el.el3pt.mid = self->build.el.el3pt.end;
    bld_back3( (PR_BUILD*) self );
    EndDraw();
}

/*----------------------[ nquad - Draw Quarter Circle ]-------------------*/

/***************************************************************************
 **  bld_init  Initiate a build arc quadrant element. Based on build.
 **  ~~~~~~~~
 */

METHOD VOID nquad_bld_init( PR_NQUAD* self )
{
    self->build.state = BLD_1ST;
    self->build.cmd = Drg->drg.CmdID = SR_QUADRANT;
    self->build.step1 = Drg->drg.StepID = SR_CENTRE_PT;
    self->build.UndoMsg = SR_QUAD_UNDONE;
}

/***************************************************************************
 **  bld_step  Carry out the build steps for creating an arc quadrant
 **  ~~~~~~~~  element.
 */

METHOD VOID nquad_bld_step( PR_NQUAD* self, AUNIT ax, AUNIT ay )
{
    EL_ARC* pArc;

    pArc = (EL_ARC*) &self->build.el;
    switch( self->build.state )
    {
    case BLD_1ST:
        pArc->hdr.size = sizeof(EL_ARC);
        pArc->hdr.type = V_ARC;
        pArc->hdr.attr = 0;
        pArc->hdr.layer = BAND_LAYER | NEW_LAYER;
        pArc->centre.x = ax;
        pArc->centre.y = ay;
        hDestroy( Band );
        p_send3( Drg, O_DG_CREATE_BAND, C_BQUAD );
        p_send5( Band, O_BND_INIT, pArc, &Band->band.pt, &pArc->centre );
        Drg->drg.StepID = SR_QUAD_START;
        self->build.state = BLD_2ND;
        break;
    case BLD_2ND:
        p_send2( self, O_BLD_SAVE );
        break;
    }

#if 0
    ARC_BAND* el;

    el = (ARC_BAND*) &self->build.el;
    switch( self->build.state )
    {
    case BLD_1ST:
        el->arc.hdr.size = sizeof(EL_ARC);
        el->arc.hdr.type = V_ARC;
        el->arc.hdr.attr = 0;
        el->arc.hdr.layer = BAND_LAYER | NEW_LAYER;
        el->arc.centre.x = ax;
        el->arc.centre.y = ay;
        hDestroy( Band );
        p_send3( Drg, O_DG_CREATE_BAND, C_BQUAD );
        p_send5( Band, O_BND_INIT, el, &el->pt, &el->arc.centre );
        Drg->drg.StepID = SR_QUAD_START;
        self->build.state = BLD_2ND;
        break;
    case BLD_2ND:
        p_send2( self, O_BLD_SAVE );
        break;
    }
#endif
}

/*--------------------------[ ntext - Line of text ]----------------------*/

/***************************************************************************
 **  bld_init  Initiate a build text element. Based on build.
 **  ~~~~~~~~
 */

METHOD VOID ntext_bld_init( PR_NTEXT* self )
{
    self->build.state = BLD_1ST;
    self->build.cmd = Drg->drg.CmdID = SR_TEXT;
    self->build.step1 = Drg->drg.StepID = SR_POS_TEXT;
    self->build.UndoMsg = SR_TEXT_UNDONE;
}

/***************************************************************************
 **  bld_step  Carry out the build steps for creating a text element.
 **  ~~~~~~~~
 */

METHOD VOID ntext_bld_step( PR_NTEXT* self, AUNIT ax, AUNIT ay )
{
    TSTYLE* txt;

    txt = &Drg->drg.TextList[ Drg->vec.Text ];
    switch( self->build.state )
    {
    case BLD_1ST:
        self->build.el.text.hdr.size = SIZEOF_TEXTHDR;
        self->build.el.text.hdr.type = V_TEXT;
        self->build.el.text.hdr.attr = 0;
        self->build.el.text.hdr.layer = BAND_LAYER | NEW_LAYER;
        self->build.el.text.font = txt->font;
        self->build.el.text.pos.x = ax;
        self->build.el.text.pos.y = ay;
        self->build.el.text.set = txt->set;
        hDestroy( Band );
        p_send3( Drg, O_DG_CREATE_BAND, C_BAND );
        p_send5( Band, O_BND_INIT, &self->build.el,
            &self->build.el.text.pos, &self->build.el.text.pos );
        p_send3( w_ws->wserv.cli, O_DW_SET_TEXT, TRUE );
        Drg->drg.StepID = SR_ENTER_TEXT;
        self->build.state = BLD_2ND;
        break;
    case BLD_2ND:
        if( self->build.el.text.hdr.size - SIZEOF_TEXTHDR )
        {
            p_send2( self, O_BLD_SAVE );
        }
        else
        {
            p_send2( self, O_BLD_CANCEL );
        }
        break;
    }
}

/***************************************************************************
 **  bld_key  Printable key input for build text element.
 **  ~~~~~~~
 */

METHOD VOID ntext_bld_key( PR_NTEXT* self, INT key, INT modifiers )
{
    INT len;
    INT ret;

    len = self->build.el.text.hdr.size - SIZEOF_TEXTHDR;
    BegDraw();
    self->build.el.text.text[len] = key;
    self->build.el.text.hdr.size++;
    ret = p_send3( Drg, O_DG_SET_TEXT_RECT, &self->build.el.text );
    if( len > BYTES_MAX || ret > 0 )
    {
        hInfoPrint( SR_TEXT_TOO_LONG );
        --self->build.el.text.hdr.size;
    }
    EndDraw();
}

/***************************************************************************
 **  bld_back  Step back one step (Del key) for text.
 **  ~~~~~~~~
 */

METHOD VOID ntext_bld_back( PR_NTEXT* self )
{
    INT len;

    len = self->build.el.text.hdr.size - SIZEOF_TEXTHDR;
    if( self->build.state != BLD_2ND || len == 0 )
    {
        p_supersend2( self, O_BLD_BACK );
        return;
    }
    BegDraw();
    --self->build.el.text.hdr.size;
    p_send3( Drg, O_DG_SET_TEXT_RECT, &self->build.el.text );
    EndDraw();
}

/*------------------[ ndimh - Draw Horizontal Dimension ]-----------------*/

/***************************************************************************
 **  bld_init  Initiate a build horizontal dimension element.
 **  ~~~~~~~~  Based on build.
 */

METHOD VOID ndimh_bld_init( PR_NDIMH* self )
{
    self->build.state = BLD_1ST;
    self->build.cmd = Drg->drg.CmdID = SR_DIM_HORIZ;
    self->build.step1 = Drg->drg.StepID = SR_1ST_PT;
    self->build.step2 = SR_2ND_PT;
    self->build.eltype = V_DIM_HORIZ;
    self->build.UndoMsg = SR_DIM_HORIZ_UNDONE;
}

/***************************************************************************
 **  bld_step  Carry out the build steps for creating a horizontal dimension
 **  ~~~~~~~~  element.
 */

METHOD VOID ndimh_bld_step( PR_NDIMH* self, AUNIT ax, AUNIT ay )
{
    EL_DIM* pDim;
    TSTYLE* txt;

    txt = &Drg->drg.TextList[ Drg->vec.Text ];
    pDim = (EL_DIM*) &self->build.el;

    switch( self->build.state )
    {
    case BLD_1ST:
        pDim->hdr.size = SIZEOF_DIMHDR;
        pDim->hdr.type = V_DIM_HORIZ;
        pDim->hdr.attr = 0;
        pDim->hdr.layer = BAND_LAYER | NEW_LAYER;
        pDim->pt1.x = ax;
        pDim->pt1.y = ay;
        pDim->pt2 = pDim->pt1;
        pDim->pos = pDim->pt1;
        pDim->text.hdr.size = SIZEOF_TEXTHDR;
        pDim->text.hdr.type = V_TEXT;
        pDim->text.hdr.attr = 0;
        pDim->text.hdr.layer = 0;
        pDim->text.font = txt->font;
        pDim->text.pos = pDim->pt1;
        pDim->text.set = txt->set;
        hDestroy( Band );
        p_send3( Drg, O_DG_CREATE_BAND, C_BDIMH );
        p_send5( Band, O_BND_INIT, pDim, &pDim->pt2, NULL );
        Band->band.step = BDIMH_STEP_2;
        Drg->drg.StepID = SR_2ND_PT;
        self->build.state = BLD_2ND;
        break;
    case BLD_2ND:
        Band->band.step = BDIMH_STEP_3;
        Band->band.pt = pDim->pt2;
        p_send3( Band, O_BND_SET_NEXT, &Band->band.pt );
        Drg->drg.StepID = SR_DIM_POS_TEXT;
        self->build.state = BLD_3RD;
        break;
    case BLD_3RD:
        p_send2( self, O_BLD_SAVE );
        break;
    }
}

/***************************************************************************
 **  bld_back  Step back one step for horizontal dimension.
 **  ~~~~~~~~
 */

METHOD VOID ndimh_bld_back( PR_NDIMH* self )
{
    EL_DIM* pDim;

    if( self->build.state < BLD_3RD )
    {
        p_supersend2( self, O_BLD_BACK );
        return;
    }

    pDim = (EL_DIM*) &self->build.el;
    BegDraw();
    pDim->pt2 = Band->band.pt;
    Band->band.step = BDIMH_STEP_2;
    p_send3( Band, O_BND_SET_NEXT, &pDim->pt2 );
    self->build.state = BLD_2ND;
    Drg->drg.StepID = SR_2ND_PT;
    EndDraw();
}

/*-----------------------[ tbuild - Temporary build ]--------------------*/

/***************************************************************************
 **  destroy  Destroy a temporary build and restore the previous build
 **  ~~~~~~~  object.
 */

METHOD VOID tbuild_destroy( PR_TBUILD* self )
{
    PR_VECDW* dw;

    hDestroy( Band );
    if( self->tbuild.oldband )
    {
        BegDraw();
        Band = self->tbuild.oldband;
        EndDraw();
    }
    Band = self->tbuild.oldband;
    Build = self->tbuild.oldbuild;
    Drg->drg.CmdID = self->tbuild.oldcmd;
    Drg->drg.StepID = self->tbuild.oldstep;
    dw = (PR_VECDW*) w_ws->wserv.cli;
    dw->vecdw.TextOn = self->tbuild.oldtext;
    if( Drg->drg.IWin )
    {
        p_send3( Drg->drg.IWin, O_IW_UPDATE, IW_MODE );
    }
    self->build.temp = TRUE;
    p_supersend2( self, O_DESTROY );
}

/***************************************************************************
 **  bld_init  Initiate a temporary build object that suspends the previous
 **  ~~~~~~~~  build rather than destroying it. Based on build.
 */

METHOD VOID tbuild_bld_init( PR_TBUILD* self )
{
    PR_VECDW* dw;

    self->tbuild.oldbuild = (PR_BUILD*) Build;
    self->tbuild.oldcmd = Drg->drg.CmdID;
    self->tbuild.oldstep = Drg->drg.StepID;
    dw = (PR_VECDW*) w_ws->wserv.cli;
    self->tbuild.oldtext = dw->vecdw.TextOn;
    self->tbuild.oldband = Band;
    if( Band )
    {
        BegDraw();
        Band = NULL;
        EndDraw();
    }
    Build = (PR_BUILD*) self;
}

/*----------------------[ zbox - Display a Zoom box ]---------------------*/

/***************************************************************************
 **  bld_init  Initiate a build zoom box. Based on tbuild.
 **  ~~~~~~~~
 */

METHOD VOID zbox_bld_init( PR_ZBOX* self )
{
    p_supersend2( self, O_BLD_INIT );
    self->build.state = BLD_1ST;
    self->build.cmd = Drg->drg.CmdID = SR_ZOOM_BOX;
    self->build.step1 = Drg->drg.StepID = SR_1ST_CORNER;
    self->build.step2 = SR_2ND_CORNER;
    self->build.eltype = V_BOX;
}

/***************************************************************************
 **  bld_step  Carry out the build steps for creating a zoom box.
 **  ~~~~~~~~
 */

METHOD VOID zbox_bld_step( PR_ZBOX* self, AUNIT ax, AUNIT ay )
{
    ARECT rect;

    switch( self->build.state )
    {
    case BLD_1ST:
        p_supersend4( self, O_BLD_STEP, ax, ay );
        break;
    case BLD_2ND:
        rect.pos.x = ax;
        rect.pos.y = ay;
        rect.lim = self->build.el.line.beg;
        BegDraw();
        hDestroy( Band );
        Drg->vec.Cur.x = p_send4( Drg, O_DG_AVERAGE, rect.pos.x, rect.lim.x );
        Drg->vec.Cur.y = p_send4( Drg, O_DG_AVERAGE, rect.pos.y, rect.lim.y );
        p_send2( Drg, O_DG_SNAP_TO_GRID );
        p_send3( Drg, O_DG_ZOOM_TO_BOX, &rect );
        EndDraw();
        hDestroy( self );
        break;
    }
}

/* End of VECBUILD.C File */
