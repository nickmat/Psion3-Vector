/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: [RUBBER] BAND CLASS MEMBERS      *  Date Started: 24 Sep 1996  *
 *    File: VECBAND.C       Type: C SOURCE   *  Date Revised: 13 May 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1996-98, Nick Matthews
*/

#include <hwim.h>
#include <vector.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"

/***************************************************************************
 **  DrawEditBand  Draw the marked elements grey. If overlaid == 0 then the
 **  ~~~~~~~~~~~~  elements overlay the original, so clear the black layer
 **  (so we can see the grey). Draw the original black again if we have just
 **  moved off original.
 */

LOCAL_C VOID DrawEditBand( PR_BAND* self, int overlaid )
{
    UWORD i, cnt;
    INT process;

    Drg->drg.DAttr.bitmap = hGreyBM;
    Drg->drg.DAttr.mode = DRAW_MODE_SET;

    Rewind();
    cnt = p_send2( Data, O_DD_COUNT );
    for( i = 0 ; i < cnt ; i++ )
    {
        p_send3( Data, O_DD_NEXT, DBuf );
        if( ( DBuf[LAYER_BYTE] & BAND_LAYER ) &&
            ( Drg->vec.LayMask & LayerBit( DBuf[LAYER_BYTE] ) ) )
        {
            Draw( DBuf );
            if( Drg->drg.BandLayer == BL_BLACK )
            {
                if( overlaid == 0 && Drg->drg.BandLayer == BL_BLACK )
                {
                    Drg->drg.DAttr.bitmap = hBlackBM;
                    Drg->drg.DAttr.mode = DRAW_MODE_CLR;
                    Draw( DBuf );
                    Drg->drg.DAttr.bitmap = hGreyBM;
                    Drg->drg.DAttr.mode = DRAW_MODE_SET;
                }
                else if( self->band.OrigHidden  )
                {
                    Drg->drg.DAttr.bitmap = hBlackBM;
                    process = Drg->drg.ProcessOp;
                    Drg->drg.ProcessOp = PO_NONE;
                    Draw( DBuf );
                    Drg->drg.ProcessOp = process;
                    Drg->drg.DAttr.bitmap = hGreyBM;
                }
            }
        }
    }
    if( overlaid == 0 && Drg->drg.BandLayer == BL_BLACK )
    {
        self->band.OrigHidden = TRUE;
    }
    else if( self->band.OrigHidden  )
    {
        self->band.OrigHidden = FALSE;
    }
    Drg->drg.ProcessOp = PO_NONE;
}

/*---------------------------[ band - Base class ]------------------------*/

#pragma METHOD_CALL

/***************************************************************************
 **  destroy  Remove any displayed band and destroy
 **  ~~~~~~~
 */

METHOD VOID band_destroy( PR_BAND* self )
{
    if( self->band.On == TRUE )
    {
        BegDraw();
        Band = NULL;
        EndDraw();
    }
    else
    {
        Band = NULL;
    }
    p_supersend2( self, O_DESTROY );
}

/***************************************************************************
 **  bnd_init  Initiate a band. Sets up address to use for element, the
 **  ~~~~~~~~  moving point and a ref to use if the moving point is invalid.
 */

METHOD VOID band_bnd_init( PR_BAND* self, ELEM* pel, A_PT* move, A_PT* ref )
{
    self->band.pel = pel;
    self->band.Move = move;
    self->band.Ref = ref;
}

/***************************************************************************
 **  bnd_set_next  Update the move point address.
 **  ~~~~~~~~~~~~
 */

METHOD VOID band_bnd_set_next( PR_BAND* self, A_PT* mpt )
{
    self->band.Ref = self->band.Move;
    self->band.Move = mpt;
}

/***************************************************************************
 **  bnd_draw  Default is to quick draw the element, this is overridden for
 **  ~~~~~~~~  anything else.
 */

METHOD INT band_bnd_draw( PR_BAND* self )
{
    UINT quick;

    quick = Drg->drg.Quick;
    Drg->drg.Quick = FALSE;
    p_send3( Drg, O_DG_DRAW_EL, self->band.pel );
    Drg->drg.Quick = quick;
    return 0;
}

/***************************************************************************
 **  bnd_on  Draw the band. If the drawing process call p_leave, try again
 **  ~~~~~~  using the ref point (normally redraws the original)
 */

METHOD VOID band_bnd_on( PR_BAND* self, AUNIT ax, AUNIT ay )
{
    INT err;

    if( self->band.On == FALSE )
    {
        self->band.Move->x = ax;
        self->band.Move->y = ay;
        Drg->drg.pClip = &Drg->drg.Clip;
        err = p_entersend2( self, O_BND_DRAW );
        if( err < 0 )
        {
            if( err == OUTSIDE_AA )
            {
                hInfoPrint( SRM_OUTSIDE_AA );
                if( self->band.Ref == NULL )
                {
                    /* Don't know how to make safe */
                    p_send2( Build, O_BLD_CANCEL );
                    return;
                }
                /* Retreat to a safe point */
                *self->band.Move = *self->band.Ref;
                kcClearBitmap( hGreyBM );
                p_send2( self, O_BND_DRAW );
            }
            else
            {
                p_leave( err );
            }
        }
        self->band.On = TRUE;
    }
}

/***************************************************************************
 **  bnd_off  Remove the band from the screen.
 **  ~~~~~~~
 */

METHOD VOID band_bnd_off( PR_BAND* self )
{
    if( self->band.On == TRUE )
    {
        kcClearBitmap( hGreyBM );
        self->band.On = FALSE;
    }
}

/*-------------------------------[ bcircle ]-----------------------------*/

/***************************************************************************
 **  bnd_draw  Calc circle radius and draw it
 **  ~~~~~~~~
 */

METHOD INT bcircle_bnd_draw( PR_BCIRCLE* self )
{
    EL_CIRCLE* pCir;

    pCir = (EL_CIRCLE*) self->band.pel;

    pCir->radius = kcHypotenuse( self->band.cpt.x - pCir->centre.x,
        self->band.cpt.y - pCir->centre.y );
    p_send3( Drg, O_DG_DRAW_EL, pCir );
    return 0;
#if 0
    CIRCLE_BAND* el;

    el = (CIRCLE_BAND*) self->band.pel;
    el->circle.radius = kcHypotenuse( el->cpt.x - el->circle.centre.x,
        el->cpt.y - el->circle.centre.y );
    p_send3( Drg, O_DG_DRAW_EL, self->band.pel );
    return 0;
#endif
}

/*---------------------------------[ barc ]-------------------------------*/

/***************************************************************************
 **  bnd_draw  Calc arc end radius and draw it
 **  ~~~~~~~~
 */

METHOD INT barc_bnd_draw( PR_BARC* self )
{
    EL_ARC* pArc;

    pArc = (EL_ARC*) self->band.pel;
    CalcAngle( &pArc->end, &pArc->centre, &self->band.pt );
    p_send3( Drg, O_DG_DRAW_EL, pArc );
    self->band.cpt = pArc->centre;
    DrawLine( &self->band.pt );
    return 0;

#if 0
    ARC_BAND* el;

    el = (ARC_BAND*) self->band.pel;
    CalcAngle( &el->arc.end, &el->arc.centre, &el->pt );
    p_send3( Drg, O_DG_DRAW_EL, self->band.pel );
    el->cpt = el->arc.centre;
    DrawLine( &el->pt );
    return 0;
#endif
}

/*--------------------------------[ bquad ]------------------------------*/

/***************************************************************************
 **  bnd_draw  Calc quadrant element and draw it
 **  ~~~~~~~~
 */

METHOD INT bquad_bnd_draw( PR_BQUAD* self )
{
    EL_ARC* pArc;

    pArc = (EL_ARC*) self->band.pel;
    CalcQuadrant( pArc, &self->band.pt );
    p_send3( Drg, O_DG_DRAW_EL, pArc );
    return 0;

#if 0
    ARC_BAND* el;

    el = (ARC_BAND*) self->band.pel;
    CalcQuadrant( &el->arc, &el->pt );
    p_send3( Drg, O_DG_DRAW_EL, self->band.pel );
    return 0;
#endif
}

/*---------------------------------[ bdimh ]------------------------------*/

/***************************************************************************
 **  bnd_draw  Update dimension struct and draw it
 **  ~~~~~~~~
 */

METHOD INT bdimh_bnd_draw( PR_BDIMH* self )
{
    EL_DIM* pDim;

    pDim = (EL_DIM*) self->band.pel;

    pDim->pos = *self->band.Move;
    UpdateDimHorizText( pDim );
    p_send3( Drg, O_DG_DRAW_EL, pDim );
    return 0;
}

/*---------------------------------[ bmove ]------------------------------*/

/***************************************************************************
 **  bnd_draw  Set up Move settings and draw edit band
 **  ~~~~~~~~
 */

METHOD INT bmove_bnd_draw( PR_BMOVE* self )
{
    int overlaid;
    A_PT* move;
    A_PT* ref;

    move = self->band.Move;
    ref = self->band.Ref;

    Drg->drg.OffsetX = move->x - ref->x;
    Drg->drg.OffsetY = move->y - ref->y;
    overlaid = Drg->drg.OffsetX | Drg->drg.OffsetY;
    Drg->drg.ProcessOp = PO_MOVE;

    DrawEditBand( (PR_BAND*) self, overlaid );
    return 0;
}

/*--------------------------------[ bscale ]------------------------------*/

/***************************************************************************
 **  bnd_draw  Set up Scale settings and draw edit band
 **  ~~~~~~~~
 */

METHOD INT bscale_bnd_draw( PR_BSCALE* self )
{
    int overlaid;
    int dx, dy;
    A_PT* move;
    A_PT* ref;

    move = self->band.Move;
    ref = self->band.Ref;

    if( self->bscale.doscale == FALSE )
    {
        self->bscale.line[0] = *ref;
        Drg->drg.CentreX = ref->x;
        Drg->drg.CentreY = ref->y;
        dx = move->x - ref->x;
        dy = move->y - ref->y;
        Drg->drg.Base = kcHypotenuse( dx, dy );

        overlaid = 0;
        Drg->drg.ProcessOp = PO_NONE;
    }
    else
    {
        dx = Drg->drg.CentreX - move->x;
        dy = Drg->drg.CentreY - move->y;
        Drg->drg.Scale = kcHypotenuse( dx, dy );

        overlaid = Drg->drg.Base - Drg->drg.Scale;
        Drg->drg.ProcessOp = PO_SCALE;
    }
    DrawEditBand( (PR_BAND*) self, overlaid );
    self->bscale.line[1] = *move;
    DrawLine( self->bscale.line );
    return 0;
}

/***************************************************************************
 **  bnd_set_scale  Set flag to show that the base has been set.
 **  ~~~~~~~~~~~~~
 */

METHOD VOID bscale_bnd_set_scale( PR_BSCALE* self, INT flag )
{
    self->bscale.doscale = flag;
}

/*-------------------------------[ bstretch ]-----------------------------*/

/***************************************************************************
 **  bnd_draw  Set up Stretch settings and draw edit band
 **  ~~~~~~~~
 */

METHOD INT bstretch_bnd_draw( PR_BSTRETCH* self )
{
    int overlaid;
    A_PT* move;
    A_PT* ref;

    move = self->band.Move;
    ref = self->band.Ref;

    /* NOTE: Stretch build writes to Drg->drg.aStretchRect */
    Drg->drg.OffsetX = move->x - ref->x;
    Drg->drg.OffsetY = move->y - ref->y;
    overlaid = Drg->drg.OffsetX | Drg->drg.OffsetY;

    Drg->drg.ProcessOp = PO_STRETCH;

    DrawEditBand( (PR_BAND*) self, overlaid );
    return 0;
}

/*--------------------------------[ brotate ]-----------------------------*/

/***************************************************************************
 **  bnd_draw  Set up Rotate settings and draw edit band
 **  ~~~~~~~~
 */

METHOD INT brotate_bnd_draw( PR_BROTATE* self )
{
    int overlaid;
    INT px, py;
    A_PT* move;
    A_PT* ref;

    move = self->band.Move;
    ref = self->band.Ref;

    Drg->drg.CentreX = ref->x;
    Drg->drg.CentreY = ref->y;
    Drg->drg.CosQ = px = move->x - ref->x;
    Drg->drg.SinQ = py = move->y - ref->y;
    Drg->drg.Divisor = kcHypotenuse( px, py );

    if( py == 0 && px >= 0 )
    {
        overlaid = 0;
        Drg->drg.Divisor = 0;
    }
    else
    {
        overlaid = 1;
    }

    Drg->drg.ProcessOp = PO_ROTATE;
    DrawEditBand( (PR_BAND*) self, overlaid );

    /* Draw the angle line */
    DrawLine( ref );
    return 0;
}

/*--------------------------------[ bmirror ]-----------------------------*/

/***************************************************************************
 **  bnd_draw  Set up Mirror settings and draw edit band
 **  ~~~~~~~~
 */

METHOD INT bmirror_bnd_draw( PR_BMIRROR* self )
{
    int overlaid;
    INT dx, dy;
    A_PT* move;
    A_PT* ref;

    move = self->band.Move;
    ref = self->band.Ref;

    Drg->drg.CentreX = ref->x;
    Drg->drg.CentreY = ref->y;
    dx = move->x - ref->x;
    dy = move->y - ref->y;
    if( dx < 0 )
    {
        dx = -dx;
        dy = -dy;
    }

    if( dx == 0 && dy == 0 )
    {
        overlaid = 0;
        Drg->drg.Divisor = 0;
    }
    else
    {
        overlaid = 1;
        /* CosQ = sq(dx)-sq(dy); SinQ = 2*dx*dy; Divisor = sq(dx)+sq(dy); */
        kcTrig2A( dx, dy, &Drg->drg.CosQ, &Drg->drg.SinQ, &Drg->drg.Divisor );
    }

    Drg->drg.ProcessOp = PO_MIRROR;
    DrawEditBand( (PR_BAND*) self, overlaid );

    /* Draw the angle line */
    DrawLine( ref );
    return 0;
}

/*--------------------------------[ bbreak ]------------------------------*/

/***************************************************************************
 **  bnd_draw  Just draw edit band and break line
 **  ~~~~~~~~
 */

METHOD INT bbreak_bnd_draw( PR_BBREAK* self )
{
    Drg->drg.ProcessOp = PO_NONE;
    DrawEditBand( (PR_BAND*) self, 0 );

    /* Draw the break line */
    DrawLine( self->band.Ref );
    return 0;
}

/* End of VECBAND.C file */
