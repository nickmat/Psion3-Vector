/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: SELECT AND BUILD CLASS MEMBERS   *  Date Started: 18 Jan 1997  *
 *    File: VECSEL.C        Type: C SOURCE   *  Date Revised:  2 Jun 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <hwim.h>
#include <p_math.h>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"
#include "vecbld.h"

/***************************************************************************
 **  RemoveSelection  Remove flags from elements and redraw.
 **  ~~~~~~~~~~~~~~~
 */

LOCAL_C VOID RemoveSelection( PR_SELECT* self )
{
    UINT hand;

    if( self->select.BandLayerOn == TRUE )
    {
        Rewind();
        BegDraw();
        p_send2( w_ws->wserv.cli, O_DW_CLEAR_GREY );
        for(;;)
        {
            hand = p_send3( Data, O_DD_NEXT, DBuf );
            if( hand == EL_EOD ) break;
            if( DBuf[LAYER_BYTE] & LAYER_FLAGS )
            {
                if( DBuf[LAYER_BYTE] & BAND_LAYER )
                {
                    DBuf[LAYER_BYTE] &= ~LAYER_FLAGS;
                    DBuf[LAYER_BYTE] |= PREV_LAYER;
                }
                else
                {
                    DBuf[LAYER_BYTE] &= ~LAYER_FLAGS;
                }
                p_send3( Drg, O_DG_DRAW_EL, DBuf );
                p_send4( Data, O_DD_REPLACE, hand, DBuf );
            }
        }
        EndDraw();
        self->select.BandLayerOn = FALSE;
    }
}

/***************************************************************************
 **  CreateSelection  Run thru store marking the elements selected by Drg
 **  ~~~~~~~~~~~~~~~  member. Return TRUE if selection found, else FALSE.
 */

static INT CreateSelection( PR_SELECT* self, UINT member, ARECT* rect )
{
    INT mark;
    UINT ehandle;
    INT as;

    hBusyPrint( 2, SR_SELECTION );
    BegDraw();
    Drg->drg.BandLayer = BL_GREY;
    self->select.BandLayerOn = FALSE;
    mark = FALSE;
    self->select.Count = 0;
    Rewind();
    for(;;)
    {
        ehandle = p_send4( Drg, member, rect, DBuf );
        if( ehandle == EL_EOD ) break;
        if( p_send3( self, O_SEL_FILTER, DBuf ) )
        {
            mark = TRUE;
            self->select.Count++;
            as = TRUE;
        }
        else
        {
            as = FALSE;
        }
        p_send5( Drg, O_DG_MARK_EL, ehandle, DBuf, as );
        self->select.BandLayerOn = TRUE;
    }
    EndDraw();
    wCancelBusyMsg();
    return mark;
}

/***************************************************************************
 **  SelectStep  Carry out Build steps 1st and 2nd to select the required
 **  ~~~~~~~~~~  elements.
 */

VOID SelectStep( PR_SELECT* self, AUNIT ax, AUNIT ay )
{
    ARECT* rect;
    UINT member;
    AUNIT unit;
    EL_LINE* box;

    box = &self->build.el.line;
    rect = (ARECT*) &box->beg;
    switch( self->build.state )
    {
    case BLD_1ST:   /* Select or Start Select Window */
        hDestroy( Band );
        if( self->select.PointsOnly )
            member = O_DG_NEXT_PT_INRECT;
        else
            member = O_DG_NEXT_INRECT;
        unit = Drg->vec.upp * ( ( Drg->vec.SelectBox + 1 ) / 2 );
        rect->pos.x = ax - unit;
        rect->pos.y = ay - unit;
        rect->lim.x = ax + unit + Drg->vec.upp;
        rect->lim.y = ay + unit + Drg->vec.upp;
        if( CreateSelection( self, member, rect ) )
        {
            /* Found something under cursor */
            Drg->drg.StepID = self->select.step3;
            self->build.state = BLD_3RD;
            break;
        }
        self->build.eltype = V_BOX;
        self->build.step2 = SR_SEL_CORNER;
        p_supersend4( self, O_BLD_STEP, ax, ay );
        break;
    case BLD_2ND:    /* Remove Select Window and element, get From Point */
        hDestroy( Band );
        if( ax > box->beg.x )
        {
            member = O_DG_NEXT_ENCLOSED;
        }
        else
        {
            member = O_DG_NEXT_INRECT;
        }
        if( self->select.PointsOnly )
        {
            member = O_DG_NEXT_PT_INRECT;
        }
        p_send3( Drg, O_DG_NORMALISE_RECT, rect );
        if( CreateSelection( self, member, rect ) == FALSE )
        {
            Drg->drg.StepID = SR_SELECT;
            self->build.state = BLD_1ST;
            break;
        }
        Drg->drg.StepID = self->select.step3;
        self->build.state = BLD_3RD;
        break;
    default:
        p_panic( PN_UNKNOWN_BUILD_STEP );
    }
    if( self->select.PointsOnly && self->build.state == BLD_3RD )
    {
        self->select.Rect = *rect;
    }
}

/*--------------------------[ select - base class ]-----------------------*/

#pragma METHOD_CALL

/***************************************************************************
 **  grp_agr_only  Filter to ignore V_CHARACTER and V_SYMBOL elements
 **  ~~~~~~~~~~~~
 */

METHOD INT sel_grp_agr_only( PR_SELECT* self, ELEM* pEl )
{
    if( pEl->hdr.type == V_CHARACTER || pEl->hdr.type == V_SYMBOL )
    {
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************
 **  el_only  Filter for simple elements only (but allows groups)
 **  ~~~~~~~
 */

METHOD INT sel_el_only( PR_SELECT* self, ELEM* pEl )
{
    switch( pEl->hdr.type )
    {
    case V_CHARACTER:
    case V_SYMBOL:
    case V_LINK:
    case V_TEXT:
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************
 **  destroy  Remove selection and destroy
 **  ~~~~~~~
 */

METHOD VOID select_destroy( PR_SELECT* self )
{
    RemoveSelection( self );
    p_supersend2( self, O_DESTROY );
}

/***************************************************************************
 **  bld_cancel  Remove selection and cancel (back up to 1st state or
 **  ~~~~~~~~~~  destroy).
 */

METHOD VOID select_bld_cancel( PR_SELECT* self )
{
    RemoveSelection( self );
    p_supersend2( self, O_BLD_CANCEL );
}

/***************************************************************************
 **  bld_init  Default initiation as for MOVE. Subclasses must set build.cmd
 **  ~~~~~~~~  before supersending bld_init
 */

METHOD VOID select_bld_init( PR_SELECT* self )
{
    self->build.state = BLD_1ST;
    Drg->drg.CmdID = self->build.cmd;
    Drg->drg.StepID = self->build.step1 = SR_SELECT;
    self->select.step3 = SR_FROM_PT;
    self->select.step4 = SR_TO_PT;
    self->select.ProcessOp = PO_MOVE;
    self->select.BandClass = C_BMOVE;
}

/***************************************************************************
 **  bld_step  Create selection and handle a standard modify process.
 **  ~~~~~~~~
 */

METHOD VOID select_bld_step( PR_SELECT* self, AUNIT ax, AUNIT ay )
{
    A_PT* pt;
    UINT ehandle;
    UINT process;

    pt = self->select.pt;
    switch( self->build.state )
    {
    case BLD_1ST:   /* Select or Start Select Window */
    case BLD_2ND:    /* Remove Select Window and element, get From Point */
        SelectStep( self, ax, ay );
        break;
    case BLD_3RD:
        pt[0].x = ax;
        pt[0].y = ay;
        pt[1] = pt[0];
        if( self->build.CopyFlag & Drg->vec.MoveCopy )
        {
            Drg->drg.BandLayer = BL_BLACK;
        }
        else
        {
            Drg->drg.BandLayer = BL_OFF;
        }
        p_send3( Drg, O_DG_CREATE_BAND, self->select.BandClass );
        p_send5( Band, O_BND_INIT, &self->build.el, &self->select.pt[1], self->select.pt );
        Band->band.On = TRUE;
        Band->band.OrigHidden = TRUE;
        Drg->drg.StepID = self->select.step4;
        self->build.state = BLD_4TH;
        break;
    case BLD_4TH:  /* Carry out & clear up */
        hDestroy( Band );
        Rewind();
        BegUndo( self->build.UndoMsg );
        BegDraw();
        process = self->select.ProcessOp;
        for(;;)
        {
            ehandle = p_send3( Drg, O_DG_NEXT_MARKED, DBuf );
            if( ehandle == EL_EOD ) break;
            if( self->build.CopyFlag & Drg->vec.MoveCopy )
            {
                /* Do a copy */
                DBuf[3] &= ~LAYER_FLAGS;
                if( DBuf[1] & V_AGGR_BIT )
                {
                    /* Turn it into a group if it's not already */
                    DBuf[0] = sizeof(EL_GROUP);
                    DBuf[1] = V_GROUP;
                }
                else
                {
                    p_send4( Drg, O_DG_PROCESS_EL, DBuf, process );
                }
                if( p_enter2( AddToStore, DBuf ) )
                {
                    hInfoPrint( SRM_NO_MEMORY );
                    break;
                }
            }
            else
            {
                /* Do a move (Can not fail) */
                p_send4( Undo, O_UD_SAVE_REPLACE, ehandle, DBuf );
                p_send4( Drg, O_DG_PROCESS_EL, DBuf, process );
                p_send4( Data, O_DD_REPLACE, ehandle, DBuf );
            }
        }
        EndDraw();
        EndUndo();
        p_send2( self, O_BLD_CANCEL );
        break;
    }
}

/***************************************************************************
 **  bld_reselect  If build state is 1st or 2nd, create selection from
 **  ~~~~~~~~~~~~  elements marked as prev.
 */

METHOD INT select_bld_reselect( PR_SELECT* self )
{
    INT mark;
    UINT ehandle;
    INT as;

    if( self->build.state > BLD_2ND )
    {
        return FALSE;
    }
    if( self->select.PointsOnly )
    {
        return FALSE;
    }
    Rewind();
    BegDraw();
    Drg->drg.BandLayer = BL_GREY;
    self->select.BandLayerOn = FALSE;
    mark = FALSE;
    self->select.Count = 0;
    for(;;)
    {
        ehandle = p_send3( Data, O_DD_NEXT, DBuf );
        if( ehandle == EL_EOD ) break;
        if( ( DBuf[LAYER_BYTE] & PREV_LAYER ) == 0 ) continue;

        if( p_send3( self, O_SEL_FILTER, DBuf ) )
        {
            mark = TRUE;
            self->select.Count++;
            as = TRUE;
            self->select.BandLayerOn = TRUE;
        }
        else
        {
            as = FALSE;
        }
        p_send5( Drg, O_DG_MARK_EL, ehandle, DBuf, as );
    }
    if( mark )
    {
        hDestroy( Band );
        Drg->drg.StepID = self->select.step3;
        self->build.state = BLD_3RD;
    }
    EndDraw();
    return mark;
}

/***************************************************************************
 **  bld_back  If build state is 3rd or less, cancel selection, otherwise
 **  ~~~~~~~~  put back to 3rd state.
 */

METHOD VOID select_bld_back( PR_SELECT* self )
{
    if( self->build.state < BLD_4TH )
    {
        p_send2( self, O_BLD_CANCEL );
        return;
    }
    BegDraw();
    Drg->drg.BandLayer = BL_GREY;
    hDestroy( Band );
    p_send2( Drg, O_DG_REDRAW );
    EndDraw();
    Drg->drg.StepID = self->select.step3;
    self->build.state = BLD_3RD;
}

/*------------------------------[ emove - Move ]--------------------------*/

/***************************************************************************
 **  bld_init  Initiate a Select & Move build. Based on select.
 **  ~~~~~~~~
 */

METHOD VOID emove_bld_init( PR_EMOVE* self )
{
    self->build.cmd = SR_MOVE;
    p_supersend2( self, O_BLD_INIT );
    self->build.UndoMsg = SR_MOVE_UNDONE;
}

/*------------------------------[ ecopy - Copy ]--------------------------*/

/***************************************************************************
 **  bld_init  Initiate a Select & Copy build. Based on select.
 **  ~~~~~~~~
 */

METHOD VOID ecopy_bld_init( PR_ECOPY* self )
{
    self->build.cmd = SR_COPY;
    p_supersend2( self, O_BLD_INIT );
    self->build.CopyFlag = COM_ACT_COPY;
    self->build.UndoMsg = SR_COPY_UNDONE;
}

/*-----------------------------[ escale - Scale ]-------------------------*/

/***************************************************************************
 **  bld_init  Initiate a Select & Scale build. Based on select.
 **  ~~~~~~~~
 */

METHOD VOID escale_bld_init( PR_ESCALE* self )
{
    self->build.cmd = SR_SCALE;
    p_supersend2( self, O_BLD_INIT );
    self->select.step3 = SR_CENTRE_PT;
    self->select.step4 = SR_BASE_PT;
    self->build.CopyFlag = COM_ACT_SCALE;
    self->select.ProcessOp = PO_SCALE;
    self->select.BandClass = C_BSCALE;
    self->build.UndoMsg = SR_SCALE_UNDONE;
}

/***************************************************************************
 **  bld_step  5 step method for Scale build
 **  ~~~~~~~~
 */

METHOD VOID escale_bld_step( PR_ESCALE* self, AUNIT ax, AUNIT ay )
{
    A_PT pt;

    switch( self->build.state )
    {
    case BLD_1ST:
    case BLD_2ND:
    case BLD_3RD:
        p_supersend4( self, O_BLD_STEP, ax, ay );
        break;
    case BLD_4TH:
        pt.x = ax;
        pt.y = ay;
        if( ComparePt( &self->select.pt[0], &pt ) == 0 )
        {
            hInfoPrint( SR_BASE_NOT_ZERO );
            break;
        }
        BegDraw();
        self->select.pt[0] = pt;
        p_send3( Band, O_BND_SET_SCALE, TRUE );
        EndDraw();
        Drg->drg.StepID = SR_SCALE_PT;
        self->build.state = BLD_5TH;
        break;
    case BLD_5TH:
        self->build.state = BLD_4TH;
        p_supersend4( self, O_BLD_STEP, ax, ay );
    }
}

/***************************************************************************
 **  bld_back  Step back method for Scale 4th step
 **  ~~~~~~~~
 */

METHOD VOID escale_bld_back( PR_ESCALE* self )
{
    if( self->build.state < BLD_5TH )
    {
        p_supersend2( self, O_BLD_BACK );
        return;
    }
    BegDraw();
    self->select.pt[0].x = Drg->drg.CentreX;
    self->select.pt[0].y = Drg->drg.CentreY;
    p_send3( Band, O_BND_SET_SCALE, FALSE );
    Drg->drg.StepID = SR_BASE_PT;
    self->build.state = BLD_4TH;
    EndDraw();
}

/*---------------------------[ estretch - Stretch ]-----------------------*/

/***************************************************************************
 **  bld_init  Initiate a Select & Move build. Based on select.
 **  ~~~~~~~~
 */

METHOD VOID estretch_bld_init( PR_ESTRETCH* self )
{
    self->build.cmd = SR_STRETCH;
    p_supersend2( self, O_BLD_INIT );
    self->build.CopyFlag = COM_ACT_STRETCH;
    self->select.ProcessOp = PO_STRETCH;
    self->select.BandClass = C_BSTRETCH;
    self->select.PointsOnly = TRUE;
    self->build.UndoMsg = SR_STRETCH_UNDONE;
}

/***************************************************************************
 **  bld_step  Step method for Strech build to set up StretchRect
 **  ~~~~~~~~
 */

METHOD VOID estretch_bld_step( PR_ESTRETCH* self, AUNIT ax, AUNIT ay )
{
    p_supersend4( self, O_BLD_STEP, ax, ay );
    if( self->build.state == BLD_3RD )
    {
        Drg->drg.StretchRect = self->select.Rect;
        p_send3( Drg, O_DG_NORMALISE_RECT, &Drg->drg.StretchRect );
    }
}

/*----------------------------[ erotate - Rotate ]------------------------*/

/***************************************************************************
 **  bld_init  Initiate a Select & Rotate build. Based on select.
 **  ~~~~~~~~
 */

METHOD VOID erotate_bld_init( PR_EROTATE* self )
{
    self->build.cmd = SR_ROTATE;
    p_supersend2( self, O_BLD_INIT );
    self->select.step3 = SR_CENTRE_PT;
    self->select.step4 = SR_MAKE_ANGLE;
    self->build.CopyFlag = COM_ACT_ROTATE;
    self->select.ProcessOp = PO_ROTATE;
    self->select.BandClass = C_BROTATE;
    self->build.UndoMsg = SR_ROTATE_UNDONE;
}

/*----------------------------[ emirror - Mirror ]------------------------*/

/***************************************************************************
 **  bld_init  Initiate a Select & Mirror build. Based on select.
 **  ~~~~~~~~
 */

METHOD VOID emirror_bld_init( PR_EMIRROR* self )
{
    self->build.cmd = SR_MIRROR;
    p_supersend2( self, O_BLD_INIT );
    self->select.step3 = SR_1ST_PT;
    self->select.step4 = SR_2ND_PT;
    self->build.CopyFlag = COM_ACT_MIRROR;
    self->select.ProcessOp = PO_MIRROR;
    self->select.BandClass = C_BMIRROR;
    self->build.UndoMsg = SR_MIRROR_UNDONE;
}

/*-----------------------------[ eerase - Erase ]-------------------------*/

/***************************************************************************
 **  bld_init  Initiate a Select & Delete build. Based on select.
 **  ~~~~~~~~
 */

METHOD VOID eerase_bld_init( PR_EERASE* self )
{
    self->build.cmd = SR_ERASE;
    p_supersend2( self, O_BLD_INIT );
    self->select.step3 = SR_ERASE_SEL;
}

/***************************************************************************
 **  bld_step  Step method for Delete build.
 **  ~~~~~~~~
 */

METHOD VOID eerase_bld_step( PR_EERASE* self, AUNIT ax, AUNIT ay )
{
    UINT ehandle;

    switch( self->build.state )
    {
    case BLD_1ST:
    case BLD_2ND:
        SelectStep( (PR_SELECT*) self, ax, ay );
        break;
    case BLD_3RD:
        Rewind();
        BegDraw();
        BegUndo( SR_ERASE_UNDONE );
        for(;;)
        {
            ehandle = p_send3( Drg, O_DG_NEXT_MARKED, DBuf );
            if( ehandle == EL_EOD ) break;
            p_send4( Undo, O_UD_SAVE_INSERT, ehandle, DBuf );
            p_send3( Data, O_DD_DELETE, ehandle );
        }
        EndUndo();
        self->select.BandLayerOn = FALSE;
        p_send2( Drg, O_DG_REDRAW );
        EndDraw();
        Drg->drg.StepID = SR_SELECT;
        self->build.state = BLD_1ST;
        break;
    }
}

/* End of VECSEL.C File */
