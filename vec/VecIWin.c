/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: INFORMATION WINDOW CLASS MEMBERS *  Date Started: 10 Oct 1996  *
 *    File: VECIWIN.C       Type: C SOURCE   *  Date Revised: 14 Feb 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1996, 1997, Nick Matthews
*/

#include <hwim.h>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"

/* Text size position */
#define SMALL 8
#define LARGE 12


#ifdef PSION_3A
#define REG_LINE 131

static P_RECT ItemRect[] =
{
    { { 2,   1 }, { VIW_WIDTH - 2,  16 } }, /* 0 IW_COMMAND */
    { { 2,  16 }, { VIW_WIDTH - 2,  31 } }, /* 1 IW_STEP */
    { { 6,  31 }, { VIW_WIDTH - 6,  43 } }, /* 2 IW_LOC_A */
    { { 6,  43 }, { VIW_WIDTH - 6,  55 } }, /* 3 IW_LOC_L */
    { { 6,  55 }, { VIW_WIDTH - 6,  67 } }, /* 4 IW_LOC_R */
    { { 6,  67 }, { VIW_WIDTH - 6,  79 } }, /* 5 IW_LOC_P */
    { { 6,  80 }, { VIW_WIDTH / 2,  89 } }, /* 6 IW_SNAP */
    { { VIW_WIDTH / 2, 80 }, { VIW_WIDTH - 6, 89 } }, /* 7 IW_GRID */
    { { 6,  89 }, { VIW_WIDTH - 6, 104 } }, /* 8 IW_LAYER_CUR */
    { { 6, 104 }, { VIW_WIDTH - 6, 114 } }, /* 9 IW_LAYER_ON */
    { { 6, 114 }, { VIW_WIDTH - 6, 129 } }, /*10 IW_TEXT */
    { { 6, 134 }, { VIW_WIDTH - 6, 143 } }, /*11 IW_REG_TO */
    { { 6, 143 }, { VIW_WIDTH - 6, 158 } }, /*12 IW_REG_NAME */
    /* Compound rects to simplify updating */
    { { 2,   1 }, { VIW_WIDTH - 2,  31 } }, /*13 IW_MODE (COMMAND & STEP */
    { { 6,  31 }, { VIW_WIDTH - 6,  79 } }, /*14 IW_LOC (LOC_?) */
    { { 6,  91 }, { VIW_WIDTH - 6, 116 } }, /*15 IW_LAYER ( _CUR & _ON ) */
    { { 2,   1 }, { VIW_WIDTH - 2, 158 } }  /*16 IW_ALL */
};
#else  /* PSION_SI */

#define LAY_LINE (VIW_WIDTH-21)
#define CMD_LINE  23
#define REG_LINE 134

static P_RECT ItemRect[] =
{
    { { 2,   1 }, { VIW_WIDTH -  2,  12 } }, /* 0 IW_COMMAND */
    { { 2,  12 }, { VIW_WIDTH -  2,  23 } }, /* 1 IW_STEP */
    { { 4,  25 }, { VIW_WIDTH - 24,  35 } }, /* 2 IW_LOC_A */
    { { 4,  47 }, { VIW_WIDTH - 24,  57 } }, /* 3 IW_LOC_L */
    { { 4,  69 }, { VIW_WIDTH - 24,  79 } }, /* 4 IW_LOC_R */
    { { 4,  91 }, { VIW_WIDTH - 24, 101 } }, /* 5 IW_LOC_P */
    { { 4, 113 }, { VIW_WIDTH - 24, 123 } }, /* 6 IW_SNAP */
    { { 4, 123 }, { VIW_WIDTH - 24, 134 } }, /* 7 IW_GRID */
    { { VIW_WIDTH - 20, 25 }, { VIW_WIDTH - 10, 134 } }, /* 8 IW_LAYER_CUR Not used */
    { { VIW_WIDTH - 10, 25 }, { VIW_WIDTH -  5, 134 } }, /* 9 IW_LAYER_ON */
    { { 4, 136 }, { VIW_WIDTH - 2, 148 } }, /*10 IW_TEXT */
    { { 2, 134 }, { VIW_WIDTH -  2, 143 } }, /*11 IW_REG_TO Not used */
    { { 2, 148 }, { VIW_WIDTH -  2, 158 } }, /*12 IW_REG_NAME */
    /* Compound rects to simplify updating */
    { { 2,   1 }, { VIW_WIDTH -  2,  25 } }, /*13 IW_MODE (COMMAND & STEP */
    { { 6,  25 }, { VIW_WIDTH - 24, 111 } }, /*14 IW_LOC (LOC_?) */
    { { VIW_WIDTH - 22, 25 }, { VIW_WIDTH -  5, 135 } }, /*15 IW_LAYER ( _CUR & _ON ) */
    { { 2,   1 }, { VIW_WIDTH -  2, 158 } }  /*16 IW_ALL */
};
#endif /* PSION_?? */


VOID UpdateInfo( INT index )
{
    TEXT cbuf[20], sbuf[20];

    if( Drg->drg.IWin )
    {
        p_send3( Drg->drg.IWin, O_IW_UPDATE, index );
    }
    else if( index == IW_MODE )
    {
        hLoadResBuf( Drg->drg.CmdID, cbuf );
        hLoadResBuf( Drg->drg.StepID, sbuf );
        hInfoPrint( SR_MODE_INFO_FMT, cbuf, sbuf );
    }
}

static void GetLayerOnStr(  TEXT* buf, UWORD mask, INT ch )
{
    int i, bit;

    for( i = 0, bit = 1 ; i < 16 ; i++, bit <<= 1 )
    {
        buf[i] = ( bit & mask ) ? ch + i : ' ';
    }
    buf[16] = '\0';
}

static void DispText( int item, int align, TEXT* buf, int size )
{
    gPrintBoxText( &ItemRect[item], size, align, 0, buf, p_slen(buf) );
}

static void DispLocText( P_RECT* pRect, int align, TEXT* buf )
{
    gPrintBoxText( pRect, 9, align, 0, buf, p_slen(buf) );
}


static VOID DispCartPt( INT item, INT rid, A_PT* pptDisp, A_PT* pptOrig )
{
    TEXT buf[40];
    TEXT xbuf[DIM_FIXED_MAX_Z];
    TEXT ybuf[DIM_FIXED_MAX_Z];
    LAUNIT dx, dy;
    P_RECT rect;

    rect = ItemRect[item];
    hLoadResBuf( rid, buf );
    DispLocText( &rect, G_TEXT_ALIGN_LEFT, buf );

    dx = (LONG) pptDisp->x - pptOrig->x;
    LaunitToScaleFixed( xbuf, dx );
    dy = (LONG) pptDisp->y - pptOrig->y;
    dy = Drg->vec.yDir ? -dy : dy;
    LaunitToScaleFixed( ybuf, dy );

#ifdef PSION_3A
    rect.tl.x = 16;
    rect.br.x = 62;
    p_atos( buf, "%s,", xbuf );
#else /* PSION_SI */
    rect.tl.x = 16;
    rect.br.x = VIW_WIDTH - 24;
    p_atos( buf, "%s", xbuf );
#endif /* PSION_?? */
    DispLocText( &rect, G_TEXT_ALIGN_RIGHT, buf );

#ifdef PSION_3A
    rect.tl.x = 62;
    rect.br.x = 104;
#else /* PSION_SI */
    rect.tl.y += 10;
    rect.br.y += 10;
#endif /* PSION_SI */
    DispLocText( &rect, G_TEXT_ALIGN_RIGHT, ybuf );
}

static VOID DispPolarPt( INT item, INT rid, A_PT* pptDisp, A_PT* pptOrig )
{
    TEXT buf[40];
    TEXT rbuf[DIM_FIXED_MAX_Z];
    TEXT abuf[10];
    INT dx, dy;
    AUNIT au;
    SWORD sin, cos;
    P_RECT rect;

    rect = ItemRect[item];
    hLoadResBuf( rid, buf );
    DispLocText( &rect, G_TEXT_ALIGN_LEFT, buf );

    dx = (INT) ( (LONG) pptDisp->x - pptOrig->x );
    dy = (INT) ( (LONG) pptDisp->y - pptOrig->y );
    au = kcHypotenuse( dx, dy );
    LaunitToScaleFixed( rbuf, au );

    if( au == 0 )
    {
        abuf[0] = '0';
        abuf[1] = '\0';
    }
    else
    {
        sin = ( (LONG) dy * TRIG_DIV ) / au;
        cos = ( (LONG) dx * TRIG_DIV ) / au;
        GetAngleStr( abuf, sin, cos );
    }
    rect = ItemRect[item];

#ifdef PSION_3A
    rect.tl.x = 16;
    rect.br.x = 62;
    p_atos( buf, "%s,", rbuf );
#else /* PSION_SI */
    rect.tl.x = 16;
    rect.br.x = VIW_WIDTH - 24;
    p_atos( buf, "%s", rbuf );
#endif /* PSION_?? */
    DispLocText( &rect, G_TEXT_ALIGN_RIGHT, buf );

#ifdef PSION_3A
    rect.tl.x = 62;
    rect.br.x = 104;
#else /* PSION_SI */
    rect.tl.y += 10;
    rect.br.y += 10;
#endif /* PSION_SI */
    p_atos( buf, "%s\370", abuf );
    DispLocText( &rect, G_TEXT_ALIGN_RIGHT, buf );
}

#pragma METHOD_CALL

METHOD VOID veciw_wn_init( PR_VECIW *self, WORD left )
{
    W_WINDATA wd;

    wd.extent.tl.x = left;
    wd.extent.tl.y = 0;
    wd.extent.width = VIW_WIDTH;
    wd.extent.height = wserv_channel->conn.info.pixels.y;
    wd.background = W_WIN_BACK_CLR | W_WIN_BACK_GREY_CLR;
    p_send5( self, O_WN_CONNECT, NULL,
        W_WIN_EXTENT | W_WIN_BACKGROUND, &wd );
    p_send2( self, O_WN_DODRAW );
}

METHOD VOID veciw_iw_set_pos( PR_VECIW* self, WORD pos )
{
    W_WINDATA wd;

    wInquireWindow( self->win.id, &wd );
    wd.extent.tl.x = pos;
    wSetWindow( self->win.id, W_WIN_EXTENT, &wd );
}

METHOD VOID veciw_iw_update( PR_VECIW* self, INT item )
{
    P_RECT* rect;

    rect = &ItemRect[item];
    wInvalidateRect( self->win.id, rect );
}

#ifdef PSION_3A

METHOD VOID veciw_wn_draw( PR_VECIW *self )
{
    G_GC gc;
    TEXT buf[40];
    TEXT dim[DIM_TEXT_MAX_Z+2];
    TEXT* str;
    AUNIT val;
    INT linex, liney;

    p_supersend2( self, O_WN_DRAW );
    gc.font = WS_FONT_BASE + 8;
    gSetGC( 0, G_GC_MASK_FONT, &gc );

    DispCartPt( IW_LOC_A, SR_A_LOC_FMT, &Drg->vec.Cur, &Drg->vec.Orig );
    DispCartPt( IW_LOC_L, SR_L_LOC_FMT, &Drg->drg.LastPt, &Drg->vec.Orig );
    DispCartPt( IW_LOC_R, SR_R_LOC_FMT, &Drg->vec.Cur, &Drg->drg.LastPt );
    DispPolarPt( IW_LOC_P, SR_P_LOC_FMT, &Drg->vec.Cur, &Drg->drg.LastPt );

    if( Drg->vec.Pref.grid.flags & GRID_SNAP )
    {
        LaunitToScaleText( dim, Drg->vec.Pref.grid.space );
    }
    else
    {
        hLoadResBuf( SR_OFF, dim );
    }
    hAtos( buf, SR_SNAP_FMT, dim );
    DispText( IW_SNAP, G_TEXT_ALIGN_LEFT, buf, SMALL );

    if( Drg->vec.Pref.grid.flags & GRID_DISP )
    {
        val = Drg->vec.Pref.grid.space * Drg->vec.Pref.grid.minor * Drg->drg.GridAuto;
        LaunitToScaleText( dim, val );
    }
    else
    {
        hLoadResBuf( SR_OFF, dim );
    }
    hAtos( buf, SR_GRID_FMT, dim );
    DispText( IW_GRID, G_TEXT_ALIGN_LEFT, buf, SMALL );

    gDrawLine( 0, REG_LINE, VIW_WIDTH, REG_LINE );
    hLoadResBuf( SR_REGISTERED_TO, buf );
    DispText( IW_REG_TO, G_TEXT_ALIGN_CENTRE, buf, SMALL );

    gc.font = WS_FONT_BASE + 9; /* Swiss 11 pt */
    gSetGC( 0, G_GC_MASK_FONT, &gc );

    p_send4( w_am, O_AM_LOAD_RES_BUF, Drg->drg.CmdID, buf );
    DispText( IW_COMMAND, G_TEXT_ALIGN_CENTRE, buf, LARGE );

    if( Drg->drg.CmdID == SR_VIEWING )
    {
        hLoadChlistResBuf( SET_UNITS_LIST, Drg->vec.Units, dim );
        hAtos( buf, SR_DISP_SCALE_FMT, Drg->vec.Pref.scale.mul, dim,
            Drg->vec.Pref.scale.div, Drg->vec.Pref.unitname );
    }
    else
    {
        p_send4( w_am, O_AM_LOAD_RES_BUF, Drg->drg.StepID, buf );
    }
    DispText( IW_STEP, G_TEXT_ALIGN_CENTRE, buf, LARGE );

    p_send4( Drg, O_DG_GET_LAYER_STR, buf, Drg->vec.Layer );
    DispText( IW_LAYER_CUR, G_TEXT_ALIGN_LEFT, buf, LARGE );

    str = (TEXT*) Drg->drg.TextList[Drg->vec.Text].name;
    DispText( IW_TEXT, G_TEXT_ALIGN_LEFT, str, LARGE );

    DispText( IW_REG_NAME, G_TEXT_ALIGN_CENTRE, Drg->drg.RegName, LARGE );

    gc.font = WS_FONT_BASE + 12; /* Mono 6x6 */
    gSetGC( 0, G_GC_MASK_FONT, &gc );

    GetLayerOnStr( buf, Drg->vec.LayMask, 'A' );
    gPrintBoxText( &ItemRect[IW_LAYER_ON], 6,
        G_TEXT_ALIGN_LEFT, 0, buf, p_slen(buf) );
    linex = ItemRect[IW_LAYER_ON].tl.x + Drg->vec.Layer * 6;
    liney = ItemRect[IW_LAYER_ON].tl.y + 7;
    gDrawLine( linex, liney, linex+5, liney );
}

#else  /* PSION_SI */

#define LAYER_CHAR_HT 6

METHOD VOID veciw_wn_draw( PR_VECIW *self )
{
    G_GC gc;
    TEXT buf[40];
    TEXT dim[DIM_TEXT_MAX_Z+2];
    TEXT* str;
    AUNIT val;
    INT linex, liney;
    INT i;
    UINT bit;

    p_supersend2( self, O_WN_DRAW );
    gc.font = WS_FONT_BASE + 8;
    gSetGC( 0, G_GC_MASK_FONT, &gc );

    DispCartPt( IW_LOC_A, SR_A_LOC_FMT, &Drg->vec.Cur, &Drg->vec.Orig );
    DispCartPt( IW_LOC_L, SR_L_LOC_FMT, &Drg->drg.LastPt, &Drg->vec.Orig );
    DispCartPt( IW_LOC_R, SR_R_LOC_FMT, &Drg->vec.Cur, &Drg->drg.LastPt );
    DispPolarPt( IW_LOC_P, SR_P_LOC_FMT, &Drg->vec.Cur, &Drg->drg.LastPt );

    if( Drg->vec.Pref.grid.flags & GRID_SNAP )
    {
        LaunitToScaleText( dim, Drg->vec.Pref.grid.space );
    }
    else
    {
        hLoadResBuf( SR_OFF, dim );
    }
    hAtos( buf, SR_SNAP_FMT, dim );
    DispText( IW_SNAP, G_TEXT_ALIGN_LEFT, buf, SMALL );

    if( Drg->vec.Pref.grid.flags & GRID_DISP )
    {
        val = Drg->vec.Pref.grid.space * Drg->vec.Pref.grid.minor * Drg->drg.GridAuto;
        LaunitToScaleText( dim, val );
    }
    else
    {
        hLoadResBuf( SR_OFF, dim );
    }
    hAtos( buf, SR_GRID_FMT, dim );
    DispText( IW_GRID, G_TEXT_ALIGN_LEFT, buf, SMALL );

    gDrawLine( 0, CMD_LINE, VIW_WIDTH, CMD_LINE );
    gDrawLine( 0, REG_LINE, VIW_WIDTH, REG_LINE );
    gDrawLine( LAY_LINE, CMD_LINE, LAY_LINE, REG_LINE );

    gc.font = WS_FONT_BASE + 8; /* Swiss ?? pt */
    gSetGC( 0, G_GC_MASK_FONT, &gc );

    p_send4( w_am, O_AM_LOAD_RES_BUF, Drg->drg.CmdID, buf );
    DispText( IW_COMMAND, G_TEXT_ALIGN_CENTRE, buf, SMALL );

    p_send4( w_am, O_AM_LOAD_RES_BUF, Drg->drg.StepID, buf );
    DispText( IW_STEP, G_TEXT_ALIGN_CENTRE, buf, SMALL );

    str = (TEXT*) Drg->drg.TextList[Drg->vec.Text].name;
    DispText( IW_TEXT, G_TEXT_ALIGN_LEFT, str, SMALL );

    DispText( IW_REG_NAME, G_TEXT_ALIGN_CENTRE, Drg->drg.RegName, SMALL );

    gc.font = WS_FONT_BASE + 12; /* Mono 6x6 */
    gSetGC( 0, G_GC_MASK_FONT, &gc );

    linex = ItemRect[IW_LAYER_ON].tl.x;
    liney = ItemRect[IW_LAYER_ON].tl.y + 10;
    for( i = 0, bit = 1 ; i < 16 ; i++, bit <<= 1 )
    {
        buf[0] = ( bit & Drg->vec.LayMask ) ? 'A' + i : ' ';
        gPrintText( linex, liney + (LAYER_CHAR_HT*i), buf, 1 );
    }
    buf[0] = '*';
    gPrintText( linex - 7, liney + (LAYER_CHAR_HT*Drg->vec.Layer), buf, 1 );
}

#endif /* PSION_?? */

/* End of VECIWIN.C file */
