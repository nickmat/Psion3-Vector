/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: DRAWING WINDOW CLASS MEMBERS     *  Date Started: 23 Sep 1996  *
 *    File: VECDWIN.C       Type: C SOURCE   *  Date Revised: 10 Oct 1996  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1996 - 1998, Nick Matthews
*/

#include <hwim.h>
#include <vector.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"

/*#define DBX5_Debug(A,B,C)    Debug( (A), (B), (C) )*/
#define DBX5_Debug(A,B,C)

static VOID Debug( INT n1, INT n2, TEXT* n3 )
{
    p_send5( Drg, O_DG_DEBUG, n1, n2, n3 );
}




static AUNIT FindFirstGrid( AUNIT au, AUNIT orig, AUNIT spc )
{
    AUNIT da, nda;

    orig %= spc;
    da = au - orig;
    nda = ( da / spc ) * spc;
    if( da != nda )
    {
        nda += spc;
    }
    return orig + nda;
}

VOID DrawGrid( GRIDSET* pGrid, SRECT* pRect )
{
    DATTR attr;
    GRID_DATA data;
    AUNIT au, col;
    AUNIT spc, majspc;
    INT oldgrid;
    UINT upp;

    if( pGrid->space == 0 ) return;

    if( pGrid->flags & GRID_BLACK )
    {
        attr.bitmap = hBlackBM;
        attr.mode = DRAW_MODE_INV;
    }
    else
    {
        attr.bitmap = hGreyBM;
        attr.mode = DRAW_MODE_SET;
    }
    attr.width = 0;
    attr.dash = NULL;

    upp = Drg->vec.upp;

    spc = pGrid->space * pGrid->minor;
    oldgrid = Drg->drg.GridAuto;
    Drg->drg.GridAuto = 1;
    while( spc / upp < 3 )
    {
        spc <<= 1;
        Drg->drg.GridAuto <<= 1;
    }
    if( Drg->drg.GridAuto != oldgrid )
    {
        UpdateInfo( IW_GRID );
    }
    majspc = spc * pGrid->major;
    data.upp = upp;
    data.pps = spc / upp;
    data.gps = spc % upp;
    data.maj = pGrid->major;
    data.majpps = majspc / upp;
    data.majgps = majspc % upp;

    au = FindFirstGrid( Drg->vec.Scrn.pos.x, Drg->vec.Orig.x, spc );
    data.first.x = AtoSdim( au ) - AtoSdim( Drg->vec.Scrn.pos.x );
    data.gap.x = au % upp;

    col = FindFirstGrid( Drg->vec.Scrn.pos.x, Drg->vec.Orig.x, majspc );
    data.col1st = ( col - au ) / spc;

    au = FindFirstGrid( Drg->vec.Scrn.pos.y, Drg->vec.Orig.y, spc );
    data.first.y = AtoSdim( au ) - AtoSdim( Drg->vec.Scrn.pos.y );
    data.gap.y = au % upp;

    au = FindFirstGrid( Drg->vec.Scrn.pos.y, Drg->vec.Orig.y, majspc );
    data.maj1st = AtoSdim( au ) - AtoSdim( Drg->vec.Scrn.pos.y );
    data.majgapy = au % upp;

    if( pRect == NULL )
    {
        pRect = &Drg->drg.Clip;
    }
    kcDrawGrid( &data, pRect, &attr );
}

static void ClearGrid( PR_VECDW *self )
{
    kcClearBitmap( hGreyBM );
    self->vecdw.GridOn = FALSE;
    if( Drg->drg.BandLayer == BL_GREY )
    {
        p_send2( Drg, O_DG_REDRAW );
    }
}

void DrawCursor( PR_VECDW *self, int OnFlag )
{
    DATTR attr;
    INT x, y;
    INT su[8];
    INT size;
    /*S_PT spt;*/

    if( self->vecdw.CurOn != OnFlag )
    {
        if( Drg->drg.Cfg.cursor.color == CURCOLOR_BLACK )
        {
            attr.bitmap = hBlackBM;
        }
        else
        {
            attr.bitmap = hGreyBM;
        }
        attr.mode = DRAW_MODE_INV;
        attr.width = 0;
        attr.dash = NULL;
        Drg->drg.sScrnPosX = AtoSdim( Drg->vec.Scrn.pos.x );
        Drg->drg.sScrnPosY = AtoSdim( Drg->vec.Scrn.pos.y );
        x = AtoSdim( Drg->vec.Cur.x ) - Drg->drg.sScrnPosX;
        y = AtoSdim( Drg->vec.Cur.y ) - Drg->drg.sScrnPosY;
        switch( Drg->drg.Cfg.cursor.size )
        {
            case CURSIZE_SMALL:  size = 5;   break;
            case CURSIZE_MEDIUM: size = 8;   break;
            case CURSIZE_LARGE:  size = 12;  break;
            case CURSIZE_FULL:
            default:             size = 480; break;
        }
        su[0] = x - size;
        su[1] = y;
        su[2] = x + size;
        su[3] = y;
        kcDrawLine( su, NULL, &attr );
        if( Drg->drg.Cfg.cursor.width )
        {
            --su[1]; --su[3];
            kcDrawLine( su, NULL, &attr );
            su[1] += 2; su[3] += 2;
            kcDrawLine( su, NULL, &attr );
        }
        su[0] = x;
        su[1] = y - size;
        su[2] = x;
        su[3] = y + size;
        kcDrawLine( su, NULL, &attr );
        if( Drg->drg.Cfg.cursor.width )
        {
            --su[0]; --su[2];
            kcDrawLine( su, NULL, &attr );
            su[0] += 2; su[2] += 2;
            kcDrawLine( su, NULL, &attr );
        }
        if( Drg->drg.Cfg.cursor.selbox )
        {
            size = ( Drg->vec.SelectBox + 1 ) /2;
            su[0] = x - size; su[1] = y + size;
            su[2] = x + size; su[3] = y + size;
            kcDrawLine( su, NULL, &attr );
            su[0] = x + size; su[1] = y + size - 1;
            su[2] = x + size; su[3] = y - size + 1;
            kcDrawLine( su, NULL, &attr );
            su[0] = x + size; su[1] = y - size;
            su[2] = x - size; su[3] = y - size;
            kcDrawLine( su, NULL, &attr );
            su[0] = x - size; su[1] = y - size + 1;
            su[2] = x - size; su[3] = y + size - 1;
            kcDrawLine( su, NULL, &attr );
        }

        /* Draw Page */
        Drg->drg.DAttr.bitmap = hGreyBM;
        Drg->drg.DAttr.mode = DRAW_MODE_SET;
        if( Drg->vec.PgDisp & PGDISP_PAGE )
        {
            DrawBox( (A_PT*) &Drg->vec.Page );
        }
        if( Drg->vec.PgDisp & PGDISP_MARG )
        {
            DrawBox( (A_PT*) &Drg->vec.Marg );
        }

        self->vecdw.CurOn = OnFlag;
        if( Drg->vec.Pref.grid.flags & GRID_BLACK )
        {
            DrawGrid( /*self,*/ &Drg->vec.Pref.grid, NULL );
        }
    }
}


void ExtentToRect( P_RECT* rect, const P_EXTENT* extent )
{
    rect->tl = extent->tl;
    rect->br.x = extent->tl.x + extent->width;
    rect->br.y = extent->tl.y + extent->height;
}

void UpdateWindows( void )
{
    if( Drg->drg.IWin )
    {
        p_send3( Drg->drg.IWin, O_IW_UPDATE, IW_LOC );
        /*p_send3( Drg->drg.IWin, O_IW_UPDATE, IW_GRID );*/
    }
    if( Drg->drg.VSWin )
    {
        p_send2( Drg->drg.VSWin, O_WN_DODRAW );
    }
    if( Drg->drg.HSWin )
    {
        p_send2( Drg->drg.HSWin, O_WN_DODRAW );
    }
    if( Drg->drg.HSWin == NULL && Drg->drg.VSWin == NULL )
    {
        /* Slow down to allow the ballistic cursor to work */
        p_sleep( 1L );
    }
}

#pragma METHOD_CALL

/*-----------------------[ vecdw - Drawing Window ]-----------------------*/

METHOD VOID vecdw_dw_clear_bitmaps( PR_VECDW *self )
{
    kcClearBitmap( hBlackBM );
    kcClearBitmap( hGreyBM );
    self->vecdw.CurOn = FALSE;
    self->vecdw.GridOn = FALSE;
}

/***************************************************************************
 **  wn_init  Initiate the main drawing window and bitmaps. Remember that
 **  ~~~~~~~  Drg, Data and DBuf do not exist yet.
 */

METHOD VOID vecdw_wn_init( PR_VECDW *self )
{
    W_WINDATA wd;
    W_OPEN_BIT_SEG bitseg;

    wd.extent.tl.x=0;
    wd.extent.tl.y=0;
    wd.extent.width=wserv_channel->conn.info.pixels.x;
    wd.extent.height=wserv_channel->conn.info.pixels.y;
    wd.background = W_WIN_BACK_NONE | W_WIN_BACK_GREY_NONE;
    p_send5( self, O_WN_CONNECT, NULL,
        W_WIN_EXTENT | W_WIN_BACKGROUND, &wd );

    bitseg.size.x = 512;  /* Use 512 to enable efficient Plot routine */
    bitseg.size.y = 160;

    idBlackBM = gCreateBit( WS_BIT_SEG_ACCESS, &bitseg );
    if( idBlackBM < 0 ) p_panic( PN_CANNOT_CREATE_BITMAP );
    hBlackBM = p_sgopen( bitseg.seg_name );
    if( hBlackBM < 0 ) p_panic( PN_CANNOT_OPEN_BITMAP );

    idGreyBM = gCreateBit( WS_BIT_SEG_ACCESS, &bitseg );
    if( idGreyBM < 0 ) p_panic( PN_CANNOT_CREATE_BITMAP );
    hGreyBM = p_sgopen( bitseg.seg_name );
    if( hGreyBM < 0 ) p_panic( PN_CANNOT_OPEN_BITMAP );

    vecdw_dw_clear_bitmaps( self );

    p_send3(self,O_WN_VISIBLE,WV_INITVIS);
    /*p_send3(self,O_WN_EMPHASISE,TRUE);*/
}

METHOD VOID vecdw_wn_draw( PR_VECDW *self )
{
    P_POINT zpt;
    P_RECT rect;
    G_GC gc;

    if( self->vecdw.Disable )
    {
        return;
    }
    zpt.x = zpt.y = 0;
    rect.tl.x = rect.tl.y = 0;
    rect.br.x = 512;
    rect.br.y = 160;
    gCopyBit( &zpt, idBlackBM, &rect, G_TRMODE_REPL );
    gc.flags = G_GC_FLAG_GREY_PLANE;
    gSetGC( 0, G_GC_MASK_GREY, &gc );
    gCopyBit( &zpt, idGreyBM, &rect, G_TRMODE_REPL );
}

METHOD VOID vecdw_dw_clear_grey( PR_VECDW* self )
{
    kcClearBitmap( hGreyBM );
    self->vecdw.GridOn = FALSE;
}

METHOD VOID vecdw_wn_beg_draw( PR_VECDW* self )
{
    if( self->vecdw.DrawCount == 0 )
    {
        DrawCursor( self, FALSE );
        if( Band )
        {
            p_send2( Band, O_BND_OFF );
            self->vecdw.GridOn = FALSE;
        }
    }
    self->vecdw.DrawCount++;
}

METHOD VOID vecdw_wn_beg_draw_clear( PR_VECDW* self )
{
    if( Band )
    {
        Band->band.On = FALSE;
    }
    self->vecdw.DrawCount++;
    vecdw_dw_clear_bitmaps( self );
}

METHOD VOID vecdw_wn_end_draw( PR_VECDW* self )
{
    --self->vecdw.DrawCount;
    if( self->vecdw.DrawCount == 0 )
    {
        if( Drg->drg.GridDisp == FALSE && self->vecdw.GridOn )
        {
            ClearGrid( self );
        }
        if( Band )
        {
            p_send4( Band, O_BND_ON, Drg->vec.Cur.x, Drg->vec.Cur.y );
        }
        if( Drg->drg.GridDisp && self->vecdw.GridOn == FALSE )
        {
            DrawGrid( /*self,*/ &Drg->vec.Pref.grid, NULL );
            if( Drg->vec.Pref.grid.flags & GRID_GREY )
            {
                self->vecdw.GridOn = TRUE;
            }
        }
        DrawCursor( self, TRUE );
        p_send2( self, O_WN_DODRAW );
        UpdateWindows();
    }
}

METHOD VOID vecdw_dw_set_size( PR_VECDW* self, P_EXTENT* pextent )
{
    W_WINDATA wd;

    wd.extent = *pextent;
    wSetWindow( self->win.id, W_WIN_EXTENT, &wd );
}

METHOD VOID vecdw_dw_get_size( PR_VECDW* self, P_EXTENT* pextent )
{
    W_WINDATA wd;

    wInquireWindow( self->win.id, &wd );
    *pextent = wd.extent;
}

METHOD VOID vecdw_dw_get_srect( PR_VECDW* self, SRECT* psr )
{
    W_WINDATA wd;

    wInquireWindow( self->win.id, &wd );
    psr->pos.x = psr->pos.y = 0;
    psr->lim.x = wd.extent.width;
    psr->lim.y = wd.extent.height;
}

METHOD VOID vecdw_dw_set_cursor( PR_VECDW* self, CURTYPE* cur )
{
    DrawCursor( self, FALSE );
    Drg->drg.Cfg.cursor = *cur;
    DrawCursor( self, TRUE );
}

METHOD VOID vecdw_dw_set_text( PR_VECDW* self, INT flag )
{
    self->vecdw.TextOn = flag;
}

METHOD VOID vecdw_dw_set_disable( PR_VECDW* self, INT flag )
{
    self->vecdw.Disable = flag;
}

METHOD INT vecdw_wn_key( PR_VECDW* self, INT key, INT modifiers )
{
    A_PT apt;
    int movecur = FALSE;
    int movewin = FALSE;
    int chngmode = FALSE;
    int action;
    static AUNIT step;
    static INT speed;
    INT minstep;

    DBX5_Debug( 101, 0, NULL );
    if( p_isprint( key ) )
    {
        DBX5_Debug( 102, 0, NULL );
        if( self->vecdw.TextOn && Build )
        {
            p_send4( Build, O_BLD_KEY, key, modifiers );
            return WN_KEY_NO_CHANGE;
        }
        else if( p_isalnum( key ) )
        {
            if( p_isalpha( key ) )
            {
                action = p_tofold( key ) - 'A';
            }
            else /* digit */
            {
                action = key - '0' + 26;
            }
            p_send2( w_ws->wserv.com, Drg->drg.Cfg.keys[ action ] );
            return WN_KEY_NO_CHANGE;
        }
    }

    apt = Drg->vec.Cur;

    if( key == W_KEY_LEFT || key == W_KEY_RIGHT
        || key == W_KEY_UP || key == W_KEY_DOWN )
    {
        DBX5_Debug( 105, 0, NULL );
        if( Drg->vec.OffGrid )
        {
            if( Drg->vec.Pref.grid.flags & GRID_SNAP )
            {
                AdjustCur( &apt, &Drg->vec.Cur );
            }
            Drg->vec.OffGrid = FALSE;
        }
        modifiers &= W_SHIFT_MODIFIER | W_CTRL_MODIFIER;
        switch( modifiers )
        {
        case W_SHIFT_MODIFIER:
            action = Drg->drg.Cfg.move.shift;
            break;
        case W_CTRL_MODIFIER:
            action = Drg->drg.Cfg.move.ctrl;
            break;
        case W_SHIFT_MODIFIER | W_CTRL_MODIFIER:
            action = Drg->drg.Cfg.move.sh_ctrl;
            break;
        default:
            action = Drg->drg.Cfg.move.keys;
            break;
        }
        switch( action )
        {
        case MOVE_ACT_BALLISTIC:
            movecur = TRUE;
            if( w_ws->wserv.ws.u.key.count > 1 )
            {
                if( IsKeyDown() == FALSE )
                {
                    DBX5_Debug( 103, 0, NULL );
                    return WN_KEY_NO_CHANGE;
                }
                if( step < 2000 && step / Drg->vec.upp < 40 )
                {
                    switch( speed )
                    {
                    case 0:
                    case 1:
                        speed = 5;  break;
                    case 5:
                        speed = 10; break;
                    case 10:
                        speed = 40; break;
                    default:
                        speed = 1;
                    }
                }
            }
            else
            {
                speed = 1;
            }
            break;
        case MOVE_ACT_SLOW_WIN:
            movewin = TRUE;
            apt = Drg->vec.Scrn.pos;
            speed = Drg->drg.Cfg.move.slow;
            break;
        case MOVE_ACT_SLOW_CUR:
            movecur = TRUE;
            speed = Drg->drg.Cfg.move.slow;
            break;
        case MOVE_ACT_FAST_WIN:
            movewin = TRUE;
            apt = Drg->vec.Scrn.pos;
            speed = Drg->drg.Cfg.move.fast;
            break;
        case MOVE_ACT_FAST_CUR:
            movecur = TRUE;
            speed = Drg->drg.Cfg.move.fast;
            break;
        }
        DBX5_Debug( 106, 0, NULL );
        if( Drg->vec.Pref.grid.flags & GRID_SNAP )
        {
            minstep = Drg->vec.Pref.grid.space;
        }
        else
        {
            minstep = Drg->vec.upp;
        }
        step = minstep * speed;
    }

    switch( key )
    {
    case W_KEY_LEFT:
        apt.x -= step;
        break;
    case W_KEY_RIGHT:
        apt.x += step;
        break;
    case W_KEY_UP:
        apt.y -= step;
        break;
    case W_KEY_DOWN:
        apt.y += step;
        break;
    case W_KEY_PAGE_UP:
    case W_KEY_PAGE_DOWN:
        if( modifiers == ( W_SHIFT_MODIFIER | W_PSION_MODIFIER ) )
        {
            step = ( Drg->vec.Scrn.lim.x - Drg->vec.Scrn.pos.x ) / 2;
            if( key == W_KEY_PAGE_UP ) step = -step;
            Drg->vec.Cur.x += step;
            apt = Drg->vec.Scrn.pos;
            apt.x += step;
        }
        else
        {
            step = ( Drg->vec.Scrn.lim.y - Drg->vec.Scrn.pos.y ) / 2;
            if( key == W_KEY_PAGE_UP ) step = -step;
            Drg->vec.Cur.y += step;
            apt = Drg->vec.Scrn.pos;
            apt.y += step;
        }
        movewin = TRUE;
        break;
    case W_KEY_HOME:
        movecur = TRUE;
        if( modifiers == ( W_SHIFT_MODIFIER | W_PSION_MODIFIER ) )
        {
            apt = Drg->vec.Page.pos;
        }
        else
        {
            apt = Drg->vec.Scrn.pos;
        }
        AdjustCur( &apt, &apt );
        break;
    case W_KEY_END:
        movecur = TRUE;
        if( modifiers == ( W_SHIFT_MODIFIER | W_PSION_MODIFIER ) )
        {
            apt = Drg->vec.Page.lim;
        }
        else
        {
            apt = Drg->vec.Scrn.lim;
        }
        --apt.x;
        --apt.y;
        AdjustCur( &apt, &apt );
        break;
    case W_KEY_TAB:
        if( Build && p_send2( Build, O_BLD_RESELECT ) )
        {
            chngmode = TRUE;
        }
        break;
    case '+':
    case '-':
        break;
    case W_KEY_RETURN:
        Drg->drg.LastPt = Drg->vec.Cur;
        UpdateInfo( IW_LOC );
        if( Build )
        {
            p_send4( Build, O_BLD_STEP, Drg->vec.Cur.x, Drg->vec.Cur.y );
            chngmode = TRUE;
        }
        break;
    case W_KEY_ESCAPE:
        if( Build )
        {
            p_send2( Build, O_BLD_CANCEL );
            chngmode = TRUE;
        }
        break;
    case W_KEY_DELETE_LEFT:
        if( Build )
        {
            p_send2( Build, O_BLD_BACK );
            chngmode = TRUE;
        }
        break;
    }
    DBX5_Debug( 107, 0, NULL );
    if( movecur )
    {
        DBX5_Debug( 108, 0, NULL );
        p_send4( Drg, O_DG_MOVE_CURSOR, apt.x, apt.y );
    }
    if( movewin )
    {
        DBX5_Debug( 109, 0, NULL );
        p_send4( Drg, O_DG_MOVE_SCREEN, apt.x, apt.y );
    }
    if( chngmode )
    {
        DBX5_Debug( 110, 0, NULL );
        UpdateInfo( IW_MODE );
    }
    DBX5_Debug( 104, 0, NULL );
    return WN_KEY_NO_CHANGE;
}

/* End of VECDWIN.C file */
