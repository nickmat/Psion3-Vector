/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: TEXT DRAWING FUNCTIONS           *  Date Started: 11 Mar 1997  *
 *    File: VECTEXT.C       Type: C SOURCE   *  Date Revised: 24 Feb 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, 1998, Nick Matthews
*/

#include <hwim.h>
#include <vector.g>
#include <vecsym.g>
#include <vector.rsg>
#include "vector.h"

static VOID OpenFont( FONT* font );

/***************************************************************************
 **  AdjustLkDim  Adjust the text element dimension using the given text
 **  ~~~~~~~~~~~  settings
 */

VOID AdjustLkDim( AUNIT* dim, LKSET* set )
{
    UWORD div;

    div = set->sdiv & ~FLIP_FLAG;
    *dim = ( (LONG) *dim * set->smul ) / div;
}

/***************************************************************************
 **  AdjustLkRDim  Adjust the text element dimension using the given text
 **  ~~~~~~~~~~~~  settings
 */

VOID AdjustLkRDim( RUNIT* dim, LKSET* set )
{
    UWORD div;

    div = set->sdiv & ~FLIP_FLAG;
    *dim = ( (LONG) *dim * set->smul ) / div;
}

/***************************************************************************
 **  AdjustLkAng  Adjust the text element angle using the given text
 **  ~~~~~~~~~~~  settings
 */

VOID AdjustLkAng( ANGLE* ang, LKSET* set )
{
    ANGLE temp;

    if( set->sdiv & FLIP_FLAG )
    {
        temp.sin = -( (LONG) ang->sin * set->a.cos - (LONG) ang->cos * set->a.sin ) / TRIG_DIV;
        temp.cos = ( (LONG) ang->cos * set->a.cos + (LONG) ang->sin * set->a.sin ) / TRIG_DIV;
    }
    else
    {
        temp.sin = ( (LONG) ang->sin * set->a.cos + (LONG) ang->cos * set->a.sin ) / TRIG_DIV;
        temp.cos = ( (LONG) ang->cos * set->a.cos - (LONG) ang->sin * set->a.sin ) / TRIG_DIV;
    }
    *ang = temp;
}

/***************************************************************************
 **  AdjustLkPt  Adjust the text char element point using the given text
 **  ~~~~~~~~~~  settings
 */

VOID AdjustLkPt( A_PT* pt, A_PT* pos, LKSET* set, A_PT* hot, LONG* offset )
{
    LONG dx, dy;
    UWORD div;

    dx = (LONG) pt->x - hot->x;
    if( offset )
    {
        dx += *offset;
    }
    dy = (LONG) pt->y - hot->y;
    /* Scale */
    div = set->sdiv & ~FLIP_FLAG;
    dx = ( dx * set->smul ) / div;
    dy = ( dy * set->smul ) / div;
    /* Flip */
    if( set->sdiv & FLIP_FLAG )
    {
        dy = -dy;
    }
    /* Rotate */
    pt->x = pos->x + ( dx * set->a.cos - dy * set->a.sin ) / TRIG_DIV;
    pt->y = pos->y + ( dx * set->a.sin + dy * set->a.cos ) / TRIG_DIV;
}

/***************************************************************************
 **  AdjustFileEl  Adjust the text char element using the given text
 **  ~~~~~~~~~~~~  settings
 */

VOID AdjustFileEl(
    UBYTE* el, A_PT* pos, LKSET* set, A_PT* hot, LONG* offset )
{
    ELHDR* hdr;
    A_PT*  pt;
    int size, i;
    EL_CIRCLE* cir;
    EL_ARC* arc;
    ANGLE ang;

    hdr = (ELHDR*) el;
    pt = (A_PT*) &el[sizeof(ELHDR)];
    switch( hdr->type )
    {
    case V_LINE:
    case V_POLYLINE:
    case V_POLYGON:
    case V_3PT_ARC:
        size = ( hdr->size - sizeof(ELHDR) ) / sizeof(A_PT);
        break;
    case V_ARC:
        arc = (EL_ARC*) el;
        if( set->sdiv & FLIP_FLAG )
        {
            /* Swop beg and end */
            ang = arc->beg; arc->beg = arc->end; arc->end = ang;
        }
        AdjustLkAng( &arc->beg, set );
        AdjustLkAng( &arc->end, set );
        /* Fall through */
    case V_CIRCLE:
        cir = (EL_CIRCLE*) el;
        AdjustLkDim( &cir->radius, set );
        size = 1;
        break;
    default:
        /* Ignore all others */
        return;
    }
    /*hdr->attr = 0;*/ /* We need to get the original <<=========<<<< */
    for( i = 0 ; i < size ; i++ )
    {
        AdjustLkPt( &pt[i], pos, set, hot, offset );
    }
}

/***************************************************************************
 **  GetLinkRect  Convert a pointer to link element to the bounding rect
 **  ~~~~~~~~~~~  in the array of 4 points.
 */

VOID GetLinkRect( A_PT* aPt, EL_LINK* pLink )
{
    LKSET set;

    aPt[3].x = aPt[0].x = pLink->pos.x + pLink->box.pos.x;
    aPt[1].y = aPt[0].y = pLink->pos.y + pLink->box.pos.y;
    aPt[1].x = aPt[2].x = pLink->pos.x + pLink->box.lim.x;
    aPt[3].y = aPt[2].y = pLink->pos.y + pLink->box.lim.y;
    set = pLink->set;
    if( pLink->hdr.type == V_TEXT )
    {
        set.smul *= 16;
    }
    AdjustLkPt( &aPt[0], &pLink->pos, &set, &pLink->pos, NULL );
    AdjustLkPt( &aPt[1], &pLink->pos, &set, &pLink->pos, NULL );
    AdjustLkPt( &aPt[2], &pLink->pos, &set, &pLink->pos, NULL );
    AdjustLkPt( &aPt[3], &pLink->pos, &set, &pLink->pos, NULL );
}

VOID GetLinkBound( ARECT* pRect, EL_LINK* pLink )
{
    A_PT pgon[4];

    GetLinkRect( pgon, pLink );
    pRect->pos = pRect->lim = pgon[0];
    AddPointToBound( pRect, &pgon[1] );
    AddPointToBound( pRect, &pgon[2] );
    AddPointToBound( pRect, &pgon[3] );
}

INT CheckFontOpen( FONT* pFont )
{
    if( pFont->pfcb == NULL )
    {
        OpenFont( pFont );
        if( pFont->pfcb == NULL )
        {
            return -1;
        }
    }
    return 0;
}

VOID DrawText( EL_TEXT* pText )
{
    FONT* pFont;
    int i, j, count;
    LONG pos;
    UWORD word;
    EL_CHAR chel;
    LONG offset;
    INT process;
    EL_TEXTHDR txt;
    ARECT rect;

    txt = *(EL_TEXTHDR*) pText;
    if( Drg->drg.ProcessOp != PO_NONE )
    {
        ProcessPt( &txt.pos, &pText->pos );
        ProcessLink( &txt.set, &pText->set );
    }

    pFont = &Drg->drg.FontList[ txt.font ];
    if( CheckFontOpen( pFont ) < 0 )
    {
        DrawTextBox( pText );
        return;
    }
    if( Drg->drg.Primative == PRIM_ISINRECT )
    {
        GetLinkBound( &rect, (EL_LINK*) pText );
        if( ! RectOverlap( Drg->drg.paInRect, &rect ) )
        {
            return;
        }
    }
    process = Drg->drg.ProcessOp;
    Drg->drg.ProcessOp = PO_NONE;
    count = txt.hdr.size - SIZEOF_TEXTHDR;
    offset = 0;
    for( i = 0 ; i < count ; i++ )
    {
        pos = pFont->chlist + pText->text[i] * sizeof(UWORD);
        f_seek( pFont->pfcb, P_FABS, &pos );
        f_read( pFont->pfcb, &word, sizeof(UWORD) );
        if( word == 0xffff ) continue;
        pos = pFont->data + word;
        f_seek( pFont->pfcb, P_FABS, &pos );
        f_read( pFont->pfcb, &chel, sizeof(EL_CHAR) );
        for( j = 0 ; j < chel.grp.count ; j++ )
        {
            f_read( pFont->pfcb, FBuf, 1 );
            f_read( pFont->pfcb, &FBuf[1], FBuf[0]-1 );
            AdjustFileEl( FBuf, &txt.pos, &txt.set, &chel.hot, &offset );
            Draw( FBuf );
        }
        offset += chel.pitch;
    }
    Drg->drg.ProcessOp = process;
}

/***************************************************************************
 **  SetTextLen  Reads the font file to calculate the unadjusted text
 **  ~~~~~~~~~~  length. Returns 0 if sucessful, -1 if unable to read font
 **  file or the (positive) number of characters able to fit if the string
 **  is too long.
 */

RUNIT RMax( RUNIT ru1, RUNIT ru2 )
{
    if( ru1 > ru2 )
    {
        return ru1;
    }
    return ru2;
}

RUNIT RMin( RUNIT ru1, RUNIT ru2 )
{
    if( ru1 > ru2 )
    {
        return ru2;
    }
    return ru1;
}

INT SetTextRect( EL_TEXT* pText )
{
    FONT* pFont;
    int i, count;
    LONG pos;
    UWORD word;
    EL_CHAR chel;
    LAUNIT width;
    RUNIT test;
    RRECT rect;

    pFont = &Drg->drg.FontList[ pText->font ];
    if( CheckFontOpen( pFont ) < 0 )
    {
        return -1;
    }
    count = pText->hdr.size - SIZEOF_TEXTHDR;
    width = 0;
    p_bfil( &rect, sizeof(RRECT), 0 );
    for( i = 0 ; i < count ; i++ )
    {
        pos = pFont->chlist + pText->text[i] * sizeof(UWORD);
        f_seek( pFont->pfcb, P_FABS, &pos );
        f_read( pFont->pfcb, &word, sizeof(UWORD) );
        if( word == 0xffff ) continue;
        pos = pFont->data + word;
        f_seek( pFont->pfcb, P_FABS, &pos );
        f_read( pFont->pfcb, &chel, sizeof(EL_CHAR) );
        test = ( (RUNIT) ( chel.bound.pos.y - chel.hot.y + 8 ) ) / 16;
        rect.pos.y = RMin( rect.pos.y, test );
        test = ( (RUNIT) ( chel.bound.lim.y - chel.hot.y + 8 ) ) / 16;
        rect.lim.y = RMax( rect.lim.y, test );
        test = ( width + chel.bound.pos.x - chel.hot.x + 8 ) / 16;
        rect.pos.x = RMin( rect.pos.x, test );
        test = ( width + chel.bound.lim.x - chel.hot.x + 8 ) / 16;
        rect.lim.x = RMax( rect.lim.x, test );
        width += chel.pitch;
    }
    pText->box = rect;
    return 0;
}

/***************************************************************************
 **  GetTextRect  Convert a pointer to text element to the bounding rect
 **  ~~~~~~~~~~~  in the array of 4 points.
 */

VOID GetTextRect( A_PT* aPt, EL_TEXT* pText )
{
    GetLinkRect( aPt, (EL_LINK*) pText );
}

VOID DrawTextBox( EL_TEXT* pText )
{
    A_PT rect[4];

    GetLinkRect( rect, (EL_LINK*) pText );
    DrawPolygon( rect, 4 );
}

VOID GetLinkRrect( RRECT* pRrect, EL_LINK* pLink )
{
    LKSET set;

    *pRrect = pLink->box;
    set = pLink->set;
    if( pLink->hdr.type == V_TEXT )
    {
        set.smul *= 16;
    }
    AdjustLkRDim( &pRrect->pos.x, &set );
    AdjustLkRDim( &pRrect->pos.y, &set );
    AdjustLkRDim( &pRrect->lim.x, &set );
    AdjustLkRDim( &pRrect->lim.y, &set );
}

INT DeleteTextStyle( UINT select )
{
    TEXT* uname;
    DL_DATA data;

    if( Drg->drg.TextSize == 1 )
    {
        data.id = DEL_ALL_TSTYLE_DIALOG;
        data.rbuf = NULL;
        data.pdlg = NULL;
        hLaunchDial( CAT_VECTOR_HWIM, C_DLGBOX, &data );
        return FALSE;
    }
    uname = (TEXT*) Drg->drg.TextList[ select ].name;
    if( hConfirm( SR_TSTYLE_DEL_FMT, uname ) )
    {
        DeleteFromList( Drg->drg.TextList, &Drg->drg.TextSize,
            sizeof(TSTYLE), select );
        if( select == Drg->vec.Text )
        {
            Drg->vec.Text = 0;  /* Make default current */
        }
        else if( Drg->vec.Text > select )
        {
            --Drg->vec.Text;
        }
        return TRUE;
    }
    return FALSE;
}


/***************************************************************************
 **  OpenFont  Open the font file for the supplied font struct. The
 **  ~~~~~~~~  struct has only the font name field, this function will
 **  complete the remaining fields.
 */

static VOID OpenFont( FONT* pFont )
{
    PR_FTOPEN* ftopen;

    if( p_scmp( pFont->fname, "*Default" ) == 0 )
    {
        OpenAddFile4( pFont );
        return;
    }
    ftopen = f_new( CAT_VECTOR_VECTOR, C_FTOPEN );
    p_send3( ftopen, O_FTO_OPEN, pFont );
    hDestroy( ftopen );
}

/*--------------------------[ Draw (Symbol) Link ]------------------------*/

VOID DrawLinkBox( EL_LINK* pLink )
{
    A_PT pt[2];

    pt[0].x = pLink->pos.x + pLink->box.pos.x;
    pt[0].y = pLink->pos.y + pLink->box.pos.y;
    AdjustLkPt( &pt[0], &pLink->pos, &pLink->set, &pLink->pos, NULL );
    pt[1].x = pLink->pos.x + pLink->box.lim.x;
    pt[1].y = pLink->pos.y + pLink->box.lim.y;
    AdjustLkPt( &pt[1], &pLink->pos, &pLink->set, &pLink->pos, NULL );
    DrawBox( pt );
}

/***************************************************************************
 **  DrawLink
 **  ~~~~~~~~
 */

VOID DrawLink( EL_LINK* pLink )
{
    LIB* lib;
    int i;
    LONG pos;
    UWORD word;
    INT process;
    EL_LINK lk;
    EL_SYMBOLHDR sym;
    ARECT rect;

    if( pLink->sym == 0xff )
    {
        DrawLinkBox( pLink );
        return;
    }
    lk = *pLink;
    if( Drg->drg.ProcessOp != PO_NONE )
    {
        ProcessPt( &lk.pos, &pLink->pos );
        ProcessLink( &lk.set, &pLink->set );
    }
    lib = &Drg->drg.LibList[ lk.lib ];

    if( lib->pfcb == NULL )
    {
        p_send3( Drg, O_DG_OPEN_LIB, lib );
        if( lib->pfcb == NULL )
        {
            DrawLinkBox( pLink );
            return;
        }
    }
    if( Drg->drg.Primative == PRIM_ISINRECT )
    {
        GetLinkBound( &rect, pLink );
        if( ! RectOverlap( Drg->drg.paInRect, &rect ) )
        {
            return;
        }
    }

    process = Drg->drg.ProcessOp;
    Drg->drg.ProcessOp = PO_NONE;

    pos = lib->symlist + lk.sym * sizeof(UWORD);
    f_seek( lib->pfcb, P_FABS, &pos );
    f_read( lib->pfcb, &word, sizeof(UWORD) );
    pos = lib->data + word;
    f_seek( lib->pfcb, P_FABS, &pos );
    f_read( lib->pfcb, &sym, sizeof(EL_SYMBOLHDR) );
    for( i = sym.grp.count ; i > 0 ; --i )
    {
        f_read( lib->pfcb, FBuf, 1 );
        f_read( lib->pfcb, &FBuf[1], FBuf[0]-1 );
        if( FBuf[TYPE_BYTE] == V_GROUP )
        {
            i += FBuf[COUNT_BYTE];
            continue;
        }
        AdjustFileEl( FBuf, &lk.pos, &lk.set, &sym.hot, NULL );
        Draw( FBuf );
    }

    Drg->drg.ProcessOp = process;
}

INT IsLinkBoxInRect( SRECT* rect, EL_LINK* pLink )
{
    EL_4PT pgon;

    GetLinkRect( pgon.pt, pLink );
    pgon.hdr.size = sizeof(EL_4PT);
    pgon.hdr.type = V_POLYGON;
    pgon.hdr.attr = 0;
    return IsElementInRect( rect, (UBYTE*) &pgon );
}


/***************************************************************************
 **  IsLinkInRect
 **  ~~~~~~~~~~~~
 */

INT IsLinkInRect( SRECT* rect, EL_LINK* pLink )
{
    LIB* lib;
    int i;
    LONG pos;
    UWORD word;
    EL_LINK lk;
    EL_SYMBOLHDR sym;

    lk = *pLink;
    lib = &Drg->drg.LibList[ lk.lib ];
    if( pLink->sym == 0xff || lib->pfcb == NULL )
    {
        return IsLinkBoxInRect( rect, pLink );
    }

    pos = lib->symlist + lk.sym * sizeof(UWORD);
    f_seek( lib->pfcb, P_FABS, &pos );
    f_read( lib->pfcb, &word, sizeof(UWORD) );
    pos = lib->data + word;
    f_seek( lib->pfcb, P_FABS, &pos );
    f_read( lib->pfcb, &sym, sizeof(EL_SYMBOLHDR) );
    for( i = sym.grp.count ; i > 0 ; --i )
    {
        f_read( lib->pfcb, FBuf, 1 );
        f_read( lib->pfcb, &FBuf[1], FBuf[0]-1 );
        if( FBuf[TYPE_BYTE] == V_GROUP )
        {
            i += FBuf[COUNT_BYTE];
            continue;
        }
        AdjustFileEl( FBuf, &lk.pos, &lk.set, &sym.hot, NULL );
        if( IsElementInRect( rect, FBuf ) == TRUE )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* End of VecText.c file */
