/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: SCROLL WINDOW CLASS MEMBERS      *  Date Started: 19 Oct 1996  *
 *    File: VECSWIN.C       Type: C SOURCE   *  Date Revised: 19 Oct 1996  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <hwim.h>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"


#pragma METHOD_CALL

METHOD VOID vecvsw_wn_init( PR_VECVSW *self, WORD left, WORD len )
{
    W_WINDATA wd;

    wd.extent.tl.x = left;
    wd.extent.tl.y = 0;
    wd.extent.width = VSW_WIDTH;
    wd.extent.height = len;
    wd.background = W_WIN_BACK_CLR | W_WIN_BACK_GREY_CLR;
    p_send5( self, O_WN_CONNECT, NULL,
        W_WIN_EXTENT | W_WIN_BACKGROUND, &wd );
    /*p_send3(self,O_WN_VISIBLE,WV_INITVIS);*/
    self->vecsw.Len = len - 2;
    p_send2( self, O_WN_DODRAW );
}

METHOD VOID vecvsw_sw_set_pos( PR_VECVSW* self, WORD left, WORD len )
{
    W_WINDATA wd;

    wInquireWindow( self->win.id, &wd );
    wd.extent.tl.x = left;
    wd.extent.height = len;
    wSetWindow( self->win.id, W_WIN_EXTENT, &wd );
    self->vecsw.Len = len - 2;
}

METHOD VOID vecvsw_wn_draw( PR_VECVSW *self )
{
    G_GC gc;
    P_RECT rect;
    INT pos, y1, y2;
    AUNIT sdim, pdim;

    pos = Drg->vec.Scrn.pos.y - Drg->vec.Page.pos.y;
    sdim = Drg->vec.Scrn.lim.y - Drg->vec.Scrn.pos.y;
    pdim = Drg->vec.Page.lim.y - Drg->vec.Page.pos.y;
    if(  pos <= 0 )
    {
        y1 = 1;
    }
    else
    {
        y1 = (int) ( ( (long) pos * self->vecsw.Len ) / pdim ) + 1;
    }
    y2 = (int) ( ( (long) sdim * self->vecsw.Len ) / pdim ) + y1;

    rect.tl.x = 1;
    rect.tl.y = y1;
    rect.br.x = VSW_WIDTH - 1;
    rect.br.y = y2;

    p_supersend2( self, O_WN_DRAW );

    gc.flags = G_GC_FLAG_GREY_PLANE;
    gSetGC( WS_TEMPORARY_GC, G_GC_MASK_GREY, &gc );
    gClrRect( &rect, G_TRMODE_SET );
}

METHOD VOID vechsw_wn_init( PR_VECHSW *self, WORD top, WORD len )
{
    W_WINDATA wd;

    wd.extent.tl.x = 0;
    wd.extent.tl.y = top;
    wd.extent.width = len;
    wd.extent.height = VSW_WIDTH;
    wd.background = W_WIN_BACK_CLR | W_WIN_BACK_GREY_CLR;
    p_send5( self, O_WN_CONNECT, NULL,
        W_WIN_EXTENT | W_WIN_BACKGROUND, &wd );
    /*p_send3(self,O_WN_VISIBLE,WV_INITVIS);*/
    self->vecsw.Len = len - 2;
    p_send2( self, O_WN_DODRAW );
}

METHOD VOID vechsw_sw_set_pos( PR_VECHSW* self, WORD top, WORD len )
{
    W_WINDATA wd;

    wInquireWindow( self->win.id, &wd );
    wd.extent.tl.y = top;
    wd.extent.width = len;
    wSetWindow( self->win.id, W_WIN_EXTENT, &wd );
    self->vecsw.Len = len - 2;
}

METHOD VOID vechsw_wn_draw( PR_VECHSW *self )
{
    G_GC gc;
    P_RECT rect;
    INT pos, x1, x2;
    AUNIT sdim, pdim;

    pos = Drg->vec.Scrn.pos.x - Drg->vec.Page.pos.x;
    sdim = Drg->vec.Scrn.lim.x - Drg->vec.Scrn.pos.x;
    pdim = Drg->vec.Page.lim.x - Drg->vec.Page.pos.x;
    if( pos <= 0 )
    {
        x1 = 1;
    }
    else
    {
        x1 = (int) ( ( (long) pos * self->vecsw.Len ) / pdim ) + 1;
    }
    x2 = (int) ( ( (long) sdim * self->vecsw.Len ) /pdim ) + x1;
    rect.tl.x = x1;
    rect.tl.y = 1;
    rect.br.x = x2;
    rect.br.y = VSW_WIDTH - 1;

    p_supersend2( self, O_WN_DRAW );

    gc.flags = G_GC_FLAG_GREY_PLANE;
    gSetGC( WS_TEMPORARY_GC, G_GC_MASK_GREY, &gc );
    gClrRect( &rect, G_TRMODE_SET );
}

/* End of VECSWIN.C file */
