/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3x          *  Written by: Nick Matthews  *
 *  Module: DIMENSION DRAWING FUNCTIONS      *  Date Started: 30 Nov 1998  *
 *    File: VECDIM.C       Type: C SOURCE    *  Date Revised:  1 Dec 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, 1998, Nick Matthews
*/

#include <hwim.h>
#include <vector.g>
#include "vector.h"


#define OVERSHOOT    250
#define ARROW_LEN    400
#define ARROW_WIDTH  200
#define STUB_ARROW   ( ARROW_LEN + OVERSHOOT )

static VOID GetDimText( EL_TEXT* pText, AUNIT dim )
{
    INT len;

    LaunitToScaleFixed( (TEXT*) pText->text, dim );
    len = p_slen( (TEXT*) pText->text );
    p_scpy( (TEXT*) ( pText->text + len ), Drg->vec.Pref.unitname );
    len = p_slen( (TEXT*) pText->text );
    pText->hdr.size = SIZEOF_TEXTHDR + len;
}

VOID UpdateDimHorizText( EL_DIM* pDim )
{
    AUNIT x, dx;
    /*INT len;*/
    RRECT box;

    if( pDim->pt1.x > pDim->pt2.x )
    {
        dx = pDim->pt1.x - pDim->pt2.x;
        x = pDim->pt2.x;
    }
    else
    {
        dx = pDim->pt2.x - pDim->pt1.x;
        x = pDim->pt1.x;
    }
    GetDimText( (EL_TEXT*) &pDim->text, dx );

    SetTextRect( (EL_TEXT*) &pDim->text );
    GetLinkRrect( &box, (EL_LINK*) &pDim->text );
    if( pDim->pos.x > x + dx )
    {
        if( pDim->pos.x < x + dx + STUB_ARROW )
        {
            pDim->pos.x = x + dx + STUB_ARROW;
        }
        pDim->text.pos.x = pDim->pos.x + OVERSHOOT;
        pDim->text.pos.y = pDim->pos.y;
    }
    else if( pDim->pos.x < x )
    {
        if( pDim->pos.x > x - STUB_ARROW )
        {
            pDim->pos.x = x - STUB_ARROW;
        }
        pDim->text.pos.x = pDim->pos.x - OVERSHOOT - ( box.lim.x - box.pos.x );
        pDim->text.pos.y = pDim->pos.y;
    }
    else
    {
        pDim->pos.x = x + dx / 2;
        pDim->text.pos.x = x + ( (LONG) dx - ( box.lim.x - box.pos.x ) ) / 2 + box.pos.x;
        pDim->text.pos.y = pDim->pos.y - OVERSHOOT;
    }
}

VOID DrawArrow( A_PT* pPt, ANGLE* pAng )
{
    A_PT line[2];

    line[0] = *pPt;
    line[1].x = pPt->x - ARROW_LEN;
    line[1].y = pPt->y - ( ARROW_WIDTH / 2);
    RotatePt( &line[1], &line[0], pAng );
    DrawLine( line );

    line[1].x = pPt->x - ARROW_LEN;
    line[1].y = pPt->y + ( ARROW_WIDTH / 2);
    RotatePt( &line[1], &line[0], pAng );
    DrawLine( line );
}

VOID DrawDimHoriz( EL_DIM* pDim )
{
    A_PT line[2];
    A_PT pt1, pt2, pos;
    ANGLE ang;
    INT process;

    /*UpdateDimHorizText( pDim );*/
    if( Drg->drg.ProcessOp != PO_NONE )
    {
        ProcessPt( &pDim->pt1, &pDim->pt1 );
        ProcessPt( &pDim->pt2, &pDim->pt2 );
        ProcessPt( &pDim->pos, &pDim->pos );
        UpdateDimHorizText( pDim );
    }
    process = Drg->drg.ProcessOp;
    Drg->drg.ProcessOp = PO_NONE;

    if( pDim->pt1.x < pDim->pt2.x )
    {
        pt1 = pDim->pt1;
        pt2 = pDim->pt2;
    }
    else
    {
        pt2 = pDim->pt1;
        pt1 = pDim->pt2;
    }
    pos = pDim->pos;


    line[0] = pt1;
    line[1].x = pt1.x;
    if( pos.y > pt1.y )
    {
        line[1].y = pos.y + OVERSHOOT;
    }
    else
    {
        line[1].y = pos.y - OVERSHOOT;
    }
    DrawLine( line );

    line[0] = pt2;
    line[1].x = pt2.x;
    if( pos.y > pt2.y )
    {
        line[1].y = pos.y + OVERSHOOT;
    }
    else
    {
        line[1].y = pos.y - OVERSHOOT;
    }
    DrawLine( line );

    line[0].y = pos.y;
    line[1].y = pos.y;
    if( pos.x < pt1.x )
    {
        line[0].x = pt1.x;
        line[1].x = ( pos.x > pt1.x - STUB_ARROW ) ? pt1.x - STUB_ARROW : pos.x;
        DrawLine( line );
        line[0].x = pt2.x;
        line[1].x = pt2.x + STUB_ARROW;
        ang.sin = 0;
        ang.cos = TRIG_DIV;
    }
    else if( pos.x > pt2.x )
    {
        line[0].x = pt2.x;
        line[1].x = ( pos.x < pt2.x + STUB_ARROW ) ? pt2.x + STUB_ARROW : pos.x;
        DrawLine( line );
        line[0].x = pt1.x;
        line[1].x = pt1.x - STUB_ARROW;
        ang.sin = 0;
        ang.cos = TRIG_DIV;
    }
    else
    {
        line[0].x = pt1.x;
        line[1].x = pt2.x;
        ang.sin = 0;
        ang.cos = -TRIG_DIV;
    }
    DrawLine( line );

    pt1.y = pos.y;
    pt2.y = pos.y;
    DrawArrow( &pt1, &ang );
    ang.sin = -ang.sin;
    ang.cos = -ang.cos;
    DrawArrow( &pt2, &ang );

    GetDimText( (EL_TEXT*) &pDim->text, pt2.x - pt1.x );
    Draw( (UBYTE*) &pDim->text );
    Drg->drg.ProcessOp = process;
}

/* End of VECDIM.C file */
