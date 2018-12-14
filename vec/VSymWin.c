/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: SYMBOL SELECT WINDOW CLASS       *  Date Started: 20 Jan 1997  *
 *    File: VSYMWIN.C       Type: C SOURCE   *  Date Revised: 25 Feb 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, 1998, Nick Matthews
*/

#include <hwim.h>
#include <p_gen.h>
#include <chlist.g>
#include <p_math.h>
#include <p_config.h>
#include <ncedit.g>
#include <vector.g>
#include <vecsym.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"
#include "vecdebug.h"

#define PU_MAX_TEXT 80

#define SYM_CTRL_NULL               (-1)
#ifdef PSION_SI
    #define SYM_CTRL_SYMBOL_NAME      0
    #define SYM_CTRL_DRAWING          1
    #define SYM_CTRL_LIBRARY          2
    #define SYM_CTRL_SCALE            3
    #define SYM_CTRL_ROTATE           4
    #define SYM_CTRL_MIRROR           5
    #define SYM_CTRL_CREATE           6
    #define SYM_CTRL_VIEW_SCALE       7
    /* SYM_CTRL_KEYS_TXT              8 */
    #define SYM_CTRL_SELECT_TXT       9
    #define SYM_CTRL_PREV_BTN         SYM_CTRL_NULL
    #define SYM_CTRL_NEXT_BTN         SYM_CTRL_NULL
    #define SYM_CTRL_LIST_SYM_TXT     10
    #define SYM_CTRL_LIST_SYM_BTN     SYM_CTRL_NULL
    #define SYM_CTRL_CHANGE_LIB_TXT   11
    #define SYM_CTRL_CHANGE_LIB_BTN   SYM_CTRL_NULL
    #define SYM_CTRL_CHANGE_SET_TXT   12
    #define SYM_CTRL_CHANGE_SET_BTN   SYM_CTRL_NULL
    /* SYM_CTRL_HELP_TXT              13 */

    #define SYM_VIEW_WIN_X   105
    #define SYM_VIEW_WIN_Y    25
    #define SYM_VIEW_WIN_HT  110
    #define SYM_VIEW_WIN_WD  110

    #define SYM_FONT         (WS_FONT_BASE + 8)
    #define SYM_TEXT_BASE    10
#else  /* PSION_3A */
    #define SYM_CTRL_SYMBOL_NAME      0
    #define SYM_CTRL_DRAWING          1
    #define SYM_CTRL_LIBRARY          2
    #define SYM_CTRL_SCALE            3
    #define SYM_CTRL_ROTATE           4
    #define SYM_CTRL_MIRROR           5
    #define SYM_CTRL_CREATE           6
    #define SYM_CTRL_VIEW_SCALE       7
    #define SYM_CTRL_SELECT_TXT       8
    #define SYM_CTRL_PREV_BTN         9
    #define SYM_CTRL_NEXT_BTN        10
    #define SYM_CTRL_LIST_SYM_TXT    11
    #define SYM_CTRL_LIST_SYM_BTN    12
    #define SYM_CTRL_CHANGE_LIB_TXT  13
    #define SYM_CTRL_CHANGE_LIB_BTN  14
    #define SYM_CTRL_CHANGE_SET_TXT  15
    #define SYM_CTRL_CHANGE_SET_BTN  16

    #define SYM_VIEW_WIN_X   150
    #define SYM_VIEW_WIN_Y    28
    #define SYM_VIEW_WIN_HT  110
    #define SYM_VIEW_WIN_WD  110

    #define SYM_FONT         (WS_FONT_BASE + 9)
    #define SYM_TEXT_BASE    12
#endif /* PSION_?? */


VOID Debug( INT n1, INT n2, INT n3 )
{
    p_send5( Drg, O_DG_DEBUG, n1, n2, n3 );
}

/***************************************************************************
 **  LoadResBuf  Load the resource rid into the gien buffer buf. If the size
 **  ~~~~~~~~~~  of the resource exceeds maxsize then panic as we have
 **  probably corrupted the heap or stack. If an error occurs it calls
 **  p_leave, normally with E_GEN_NOMEMORY or E_FILE_NEXIST.
 */

static VOID LoadResBuf( INT rid, VOID* buf, INT maxsize )
{
    INT size;

    size = p_send4( w_am, O_AM_LOAD_RES_BUF, rid, buf );
    if( size > maxsize )
    {
        p_panic( PN_PU_TEXT_TOO_BIG );
    }
}

/***************************************************************************
 **  DrawPuControl  Draw the given pop-up control in the given state. Calls
 **  ~~~~~~~~~~~~~  p_leave if there is a problem with memory or resource
 **  file, or returns 0 if ok. Panics if
 */

#pragma save, ENTER_CALL

static INT DrawPuControl( PR_SYMW* self, PU_CTRL* ctrl, UINT state )
{
    TEXT buf[PU_MAX_TEXT];
    INT border, shadow, flags;

    if( ctrl->type == PU_CTRL_TEXT )
    {
        p_send4( self, ctrl->str_id, buf, ctrl );
    }
    else if( ctrl->str_id )
    {
        LoadResBuf( ctrl->str_id, buf, PU_MAX_TEXT );
    }
    else
    {
        buf[0] = '\0';
    }
    switch( ctrl->type )
    {
    case PU_CTRL_TEXT:
    case PU_CTRL_STATIC:
        gPrintBoxText( &ctrl->rect, SYM_TEXT_BASE, ctrl->style, 0, buf, p_slen(buf) );
        break;
    case PU_CTRL_BUTTON:
        wDrawButton2( W_BUTTON_TYPE_2, &ctrl->rect, buf, state );
        break;
    case PU_CTRL_FRAME:
        switch( ctrl->style )
        {
        case PU_FRAME_LIGHT:
            border = W_BORDER_TYPE_0;
            flags = W_BORD_SHADOW_S;
            break;
        case PU_FRAME_MEDIUM:
            border = W_BORDER_TYPE_1;
            flags = W_BORD_SHADOW_S;
            break;
        case PU_FRAME_HEAVY:
            border = W_BORDER_TYPE_1;
            flags = W_BORD_SHADOW_D;
            break;
        default:
            p_panic( PN_PU_UNKNOWN_FRAME_CTRL );
        }
        shadow = ( state == PU_STATE_ACTIVE ) ? W_BORD_SHADOW_ON : 0;
        gBorder2Rect( border, &ctrl->rect, flags | shadow );
        break;
    default:
        p_panic( PN_PU_UNKNOWN_CTRL );
    }
    return 0;
}

#pragma restore

/***************************************************************************
 **  InvalidateControl  Cause the given control to be redrawn
 **  ~~~~~~~~~~~~~~~~~
 */

VOID InvalidateControl( PR_SYMW* self, INT ctrl )
{
    P_RECT* rect;

    if( ctrl == SYM_CTRL_NULL ) return;
    rect = &self->symw.pu->list[ctrl].rect;
    wInvalidateRect( self->win.id, rect );
}

/***************************************************************************
 **  PushButton  Redraw the given button control
 **  ~~~~~~~~~~
 */

VOID PushButton( PR_SYMW* self, INT ctrl )
{
    P_RECT* rect;

    if( ctrl == SYM_CTRL_NULL ) return;
    rect = &self->symw.pu->list[ctrl].rect;
    wInvalidateRect( self->win.id, rect );
}


/***************************************************************************
 **  LaunchDialog  Launch a local catagory dialog with given data, resource
 **  ~~~~~~~~~~~~  id and class.
 */

INT LaunchDialog( VOID* buf, INT id, INT class )
{
    DL_DATA dial_data;

    dial_data.id = id;
    dial_data.rbuf = buf;
    dial_data.pdlg = NULL;
    return hLaunchDial( CAT_VECSYM_VECSYM, class, &dial_data );
}

/***************************************************************************
 **  SymsetToLkset  Set the given link settings from the given Symbol
 **  ~~~~~~~~~~~~~  settings.
 */

VOID SymsetToLkset( LKSET* lk, SYMSET* ss )
{
    lk->a = ss->rotate;
    lk->smul = ss->scale.q;
    lk->sdiv = ss->scale.d;
    if( ss->mirror == FLIP_HORIZONTAL )
    {
        lk->sdiv |= FLIP_FLAG;
    }
    if( ss->mirror == FLIP_VERTICAL  )
    {
        lk->sdiv |= FLIP_FLAG;
        lk->a.sin = -ss->rotate.sin;
        lk->a.cos = -ss->rotate.cos;
    }
}

/***************************************************************************
 **  UpdateLibrary  Set the current library and update settings.
 **  ~~~~~~~~~~~~~
 */

INT UpdateLibrary( PR_SYMW* self, INT iLib )
{
    LIB* pLib;

    Drg->vec.CurLib = iLib;
    pLib = &Drg->drg.LibList[ iLib ];
    self->symw.pSymset = &pLib->set;
    SymsetToLkset( &self->symw.lkset, self->symw.pSymset );
    if( pLib->pfcb == NULL )
    {
        p_send3( Drg, O_DG_OPEN_LIB, pLib );
    }
    if( pLib->pfcb == NULL )
    {
        return FALSE;
    }
    InvalidateControl( self, SYM_CTRL_LIBRARY );
    return TRUE;
}

/***************************************************************************
 **  SelectLibDialog  Set up and run the "Select Library" dialog
 **  ~~~~~~~~~~~~~~~
 */

INT SelectLibDialog( PR_SYMW* self )
{
    PR_LKLIST* scan;
    PR_VASTR* pList;
    SLIBDLG_DATA data;
    WORD ext[2];
    TEXT* libname;
    INT i;

    scan = (PR_LKLIST*) p_send3( Drg, O_DG_NEW_CLASS, C_LKLIST );
    ext[0] = 'v' + ( 's' << 8 ); ext[1] = 'l';
    pList = (PR_VASTR*) p_send4( scan, O_LKL_CREATE_LIST, ext, FALSE );
    hDestroy( scan );

    if( pList == NULL )
    {
        hInfoPrint( SR_NO_LIBRARIES );
        return FALSE;
    }
    data.vastr = pList;
    data.current = 0;
    if( Drg->vec.CurLib < Drg->drg.LibCount )
    {
        libname = Drg->drg.LibList[ Drg->vec.CurLib ].fname;
        i = p_send3( pList, O_VA_SEARCH, libname );
        if( i > 0 ) data.current = i;
    }
    if( ! LaunchDialog( &data, SELECT_LIB_DIALOG, C_SLIBDLG ) )
    {
        return FALSE;
    }
    if( data.nsel >= 0 )
    {
        UpdateLibrary( self, data.nsel );
        return TRUE;
    }
    return FALSE;
}

/***************************************************************************
 **  SelectSymbolDialog  Set up and run the "Select Symbol" dialog
 **  ~~~~~~~~~~~~~~~~~~
 */

INT SelectSymbolDialog( PR_SYMW* self )
{
    PR_VASTR* pList;
    SLIBDLG_DATA data;

    pList = (PR_VASTR*) p_send3( Drg, O_DG_MAKE_SYMLIST, Drg->vec.CurLib );
    if( pList == NULL )
    {
        return FALSE;
    }
    data.vastr = pList;
    data.current = Drg->vec.CurSym;
    if( ! LaunchDialog( &data, SELECT_SYM_DIALOG, C_SSYMDLG ) )
    {
        return FALSE;
    }
    Drg->vec.CurSym = data.nsel;
    return TRUE;
}

/***************************************************************************
 **  ReadElement  Read a single element from current position in given file
 **  ~~~~~~~~~~~
 */

VOID ReadElement( VOID* pfcb, UBYTE* pbuf )
{
    f_read( pfcb, pbuf, 1 );
    f_read( pfcb, &pbuf[1], pbuf[0]-1 );
}

/***************************************************************************
 **  DrawSymbol  Draw the current symbol in the "Symbol Window"
 **  ~~~~~~~~~~
 */

VOID DrawSymbol( PR_SYMW* self )
{
    LONG pos;
    TE_SYM loc;
    LIB* lib;
    EL_SYMBOL* sym;
    EL_LINK* lk;
    AUNIT SymDia, ScrnDia, ht, wd;
    A_PT cpt;

    lib = &Drg->drg.LibList[ Drg->vec.CurLib ];

    sym = (EL_SYMBOL*) self->symw.Symbol;
    pos = lib->symlist + Drg->vec.CurSym * sizeof(TE_SYM);
    f_seek( lib->pfcb, P_FABS, &pos );
    f_read( lib->pfcb, &loc, sizeof(TE_SYM) );
    pos = lib->data + loc;
    f_seek( lib->pfcb, P_FABS, &pos );
    ReadElement( lib->pfcb, self->symw.Symbol );

    lk = self->symw.pLink;
    lk->box.pos.x = sym->bound.pos.x - sym->hot.x;
    lk->box.pos.y = sym->bound.pos.y - sym->hot.y;
    lk->box.lim.x = sym->bound.lim.x - sym->hot.x;
    lk->box.lim.y = sym->bound.lim.y - sym->hot.y;
    lk->ref = sym->ref;
    lk->set = self->symw.lkset;
    lk->lib = Drg->vec.CurLib;
    lk->sym = Drg->vec.CurSym;

    /* Set up drawing limits */
    Drg->vec.Scrn = self->symw.OrigScrn;
    Drg->vec.upp = self->symw.OrigUpp;

    ht = sym->bound.lim.y - sym->bound.pos.y;
    wd = sym->bound.lim.x - sym->bound.pos.x;
    SymDia = p_send4( Drg, O_DG_HYPOT, ht, wd );
    p_send4( Drg, O_DG_ADJUST_LKDIM, &SymDia, &lk->set );
    self->symw.ViewScale = 1;
    /* Make it fit */
    for(;;)
    {
        ScrnDia = p_send3( Drg, O_DG_SU2AU, SYM_VIEW_WIN_WD );
        if( SymDia < ScrnDia )
        {
            break;
        }
        Drg->vec.upp *= 2;
        self->symw.ViewScale *= 2;
    }
    cpt.x = ( sym->bound.lim.x - sym->bound.pos.x ) / 2 + sym->bound.pos.x;
    cpt.y = ( sym->bound.lim.y - sym->bound.pos.y ) / 2 + sym->bound.pos.y;
    lk->pos.x = lk->pos.y = 0;
    p_send5( Drg, O_DG_ADJUST_LKPT, &cpt, lk, &sym->hot );
    lk->pos.x = 16384 + ScrnDia / 2 - cpt.x;
    lk->pos.y = 16384 + ScrnDia / 2 - cpt.y;
    Drg->vec.Scrn.pos.x = 16384;
    Drg->vec.Scrn.pos.y = 16384;
    Drg->vec.Scrn.lim.x = 16384 + ScrnDia;
    Drg->vec.Scrn.lim.y = 16384 + ScrnDia;

    p_send2( Drg, O_DG_CLEAR_BITMAP );
    p_send3( Drg, O_DG_DRAW_EL, lk );
    InvalidateControl( self, SYM_CTRL_SYMBOL_NAME );
    InvalidateControl( self, SYM_CTRL_DRAWING );
    InvalidateControl( self, SYM_CTRL_VIEW_SCALE );
}

/***************************************************************************
 **  UpdateSettings  Cause the Link settings in "Select Symbol Window" to be
 **  ~~~~~~~~~~~~~~  updated.
 */

VOID UpdateSettings( PR_SYMW* self )
{
    InvalidateControl( self, SYM_CTRL_SCALE );
    InvalidateControl( self, SYM_CTRL_ROTATE );
    InvalidateControl( self, SYM_CTRL_MIRROR );
    InvalidateControl( self, SYM_CTRL_CREATE );
}

/***************************************************************************
 **  destroy  Destroy the "Select Symbol Window" symw class
 **  ~~~~~~~
 */

#pragma METHOD_CALL

METHOD VOID symw_destroy( PR_SYMW* self )
{
    w_ws->wserv.help_index_id = VECTOR_HELP;
    p_free( self->symw.pu );
    p_supersend2( self, O_DESTROY );
}

/***************************************************************************
 **  wn_init  Initiate the "Select Symbol Window". May call p_leave on error
 **  ~~~~~~~  Returns 0 if completes ok, 1 if user aborts or
 */

METHOD INT symw_wn_init( PR_SYMW* self, NSYM_DATA* data )
{
    W_WINDATA wd;
    EL_LINK* lk;
    INT NotFound;

    hLoadResource( SELECT_SYMBOL_DLG, &self->symw.pu );
    wd.extent = self->symw.pu->frame;
    wd.background = W_WIN_BACK_CLR | W_WIN_BACK_GREY_CLR;
    p_send5( self, O_WN_CONNECT, NULL, W_WIN_EXTENT | W_WIN_BACKGROUND, &wd );

    self->symw.BlackBM = data->blackbm;
    self->symw.GreyBM = data->greybm;
    self->symw.pLink = lk = data->link;
    self->symw.pRet = data->ret;
    *self->symw.pRet = 1;           /* Standard Esc exit */
    self->symw.BtnDown = -1;

    NotFound = FALSE;
    for(;;)
    {
        if( Drg->drg.LibCount == 0 || NotFound )
        {
            if( ! SelectLibDialog( self ) )
            {
                return 1;
            }
        }
        if( UpdateLibrary( self, Drg->vec.CurLib ) )
        {
            break;
        }
        else
        {
            hInfoPrint( SR_OPEN_LIB_FAIL_FMT, Drg->drg.LibList[Drg->vec.CurLib].fname );
            NotFound = TRUE;
        }
    }

    p_send3( w_ws->wserv.cli, O_DW_SET_DISABLE, TRUE );
    /* Save copies of properties that we are going to change */
    self->symw.OrigScrn = Drg->vec.Scrn;
    self->symw.OrigUpp = Drg->vec.upp;
    self->symw.OrigLayMask = Drg->vec.LayMask;
    Drg->vec.LayMask = 0xffff;

    lk->hdr.size = sizeof(EL_LINK);
    lk->hdr.type = V_LINK;
    lk->hdr.attr = 0;
    lk->hdr.layer = 0;
    lk->set = self->symw.lkset;
    lk->lib = Drg->vec.CurLib;
    DrawSymbol( self );

    self->win.flags = WIN_3dBORDER | IN_BWIN_SHADOW_2 | IN_BWIN_CUSHION |
        PR_WIN_NO_DDP;
    /*p_send3( self, O_WN_EMPHASISE,TRUE );*/
    w_ws->wserv.help_index_id = HELP_SYMBOLS;
    hInitVis( self );
    return 0;
}

/***************************************************************************
 **  wn_draw  Draw the complete "Symbol Select Window"
 **  ~~~~~~~
 */

METHOD VOID  symw_wn_draw( PR_SYMW *self )
{
    P_POINT zpt;
    P_RECT rect;
    G_GC gc;
    INT i, state, ret;

    p_supersend2( self, O_WN_DRAW );
    gc.font = SYM_FONT;
    gSetGC( 0, G_GC_MASK_FONT, &gc );

    for( i = 0 ; i < self->symw.pu->count ; i++ )
    {
        state = i == self->symw.BtnDown ? PU_STATE_DOWN : PU_STATE_ACTIVE;
        ret = p_enter4( DrawPuControl, self, &self->symw.pu->list[i], state );
        if( ret )
        {
            hInfoPrint( SR_ERR_D_IN_CTRL_D_FMT, ret, i );
            break;
        }
    }

    zpt.x = SYM_VIEW_WIN_X;
    zpt.y = SYM_VIEW_WIN_Y;
    rect.tl.x = 0;
    rect.tl.y = 0;
    rect.br.x = SYM_VIEW_WIN_WD;
    rect.br.y = SYM_VIEW_WIN_HT;
    gCopyBit( &zpt, self->symw.BlackBM, &rect, G_TRMODE_REPL );
    gc.flags = G_GC_FLAG_GREY_PLANE;
    gSetGC( 0, G_GC_MASK_GREY, &gc );
    gCopyBit( &zpt, self->symw.GreyBM, &rect, G_TRMODE_REPL );
}

/***************************************************************************
 **  wn_key  Handle all "Select Symbol Window" key strokes.
 **  ~~~~~~
 */

METHOD INT symw_wn_key( PR_SYMW* self, INT keycode, INT modifiers )
{
    LIB* lib;
    INT change = FALSE;
    SWORD tmp;

    lib = &Drg->drg.LibList[ Drg->vec.CurLib ];

    switch( keycode )
    {
    case W_KEY_LEFT:
        --Drg->vec.CurSym;
        if( Drg->vec.CurSym < 0 )
            Drg->vec.CurSym = lib->symcount - 1;
        change = TRUE;
        PushButton( self, SYM_CTRL_PREV_BTN );
        break;
    case W_KEY_RIGHT:
        Drg->vec.CurSym++;
        if( Drg->vec.CurSym == lib->symcount )
            Drg->vec.CurSym = 0;
        change = TRUE;
        PushButton( self, SYM_CTRL_NEXT_BTN );
        break;
    case W_KEY_UP:
        tmp = self->symw.pSymset->rotate.cos;
        self->symw.pSymset->rotate.cos = self->symw.pSymset->rotate.sin;
        self->symw.pSymset->rotate.sin = -tmp;
        InvalidateControl( self, SYM_CTRL_ROTATE );
        SymsetToLkset( &self->symw.lkset, self->symw.pSymset );
        change = TRUE;
        break;
    case W_KEY_DOWN:
        tmp = self->symw.pSymset->rotate.cos;
        self->symw.pSymset->rotate.cos = -self->symw.pSymset->rotate.sin;
        self->symw.pSymset->rotate.sin = tmp;
        InvalidateControl( self, SYM_CTRL_ROTATE );
        SymsetToLkset( &self->symw.lkset, self->symw.pSymset );
        change = TRUE;
        break;
    case 'V': case 'v':
        self->symw.pSymset->mirror &= ~FLIP_HORIZONTAL;
        self->symw.pSymset->mirror ^= FLIP_VERTICAL;
        InvalidateControl( self, SYM_CTRL_MIRROR );
        SymsetToLkset( &self->symw.lkset, self->symw.pSymset );
        change = TRUE;
        break;
    case 'H': case 'h':
        self->symw.pSymset->mirror &= ~FLIP_VERTICAL;
        self->symw.pSymset->mirror ^= FLIP_HORIZONTAL;
        InvalidateControl( self, SYM_CTRL_MIRROR );
        SymsetToLkset( &self->symw.lkset, self->symw.pSymset );
        change = TRUE;
        break;
    case 'F': case 'f':
        self->symw.pSymset->rotate.cos = -self->symw.pSymset->rotate.cos;
        self->symw.pSymset->rotate.sin = -self->symw.pSymset->rotate.sin;
        InvalidateControl( self, SYM_CTRL_ROTATE );
        SymsetToLkset( &self->symw.lkset, self->symw.pSymset );
        change = TRUE;
        break;
    case W_KEY_DIAMOND:
        if( SelectLibDialog( self ) )
        {
            change = TRUE;
        }
        break;
    case W_KEY_MENU:
        if( SelectSymbolDialog( self ) )
        {
            change = TRUE;
        }
        break;
    case W_KEY_TAB:
        if( LaunchDialog( self->symw.pSymset, SYM_SETTINGS_DLG, C_SYMSETDLG ) )
        {
            UpdateSettings( self );
            SymsetToLkset( &self->symw.lkset, self->symw.pSymset );
            change = TRUE;
        }
        break;
    case W_KEY_RETURN:
        *self->symw.pRet = 0;
        /* Fall through */
    case W_KEY_ESCAPE:
        /* *self->symw.pRet preset to 1 */
        /* Restore copies of properties that may have changed */
        Drg->vec.Scrn = self->symw.OrigScrn;
        Drg->vec.upp = self->symw.OrigUpp;
        Drg->vec.LayMask = self->symw.OrigLayMask;
        p_send3( w_ws->wserv.cli, O_DW_SET_DISABLE, FALSE );
        p_send2( w_am, O_AM_STOP );
        break;
    case W_KEY_DELETE_LEFT:
        break;
    }
    if( change )
    {
        DrawSymbol( self );
    }
    return WN_KEY_NO_CHANGE;
}

/***************************************************************************
 **  sym_library  Copy the current library and prompt string to given
 **  ~~~~~~~~~~~  buffer.
 */

METHOD VOID symw_syw_library( PR_SYMW* self, TEXT* buf, PU_CTRL* ctrl )
{
    TEXT fstr[PU_MAX_TEXT];

    LoadResBuf( SEL_SYM_LIBRARY, fstr, PU_MAX_TEXT );
    p_atos( buf, fstr, Drg->drg.LibList[ Drg->vec.CurLib ].fname );
}

/***************************************************************************
 **  sym_name  Copy the current symbol name string to given buffer.
 **  ~~~~~~~~
 */

METHOD VOID symw_syw_name( PR_SYMW* self, TEXT* buf, PU_CTRL* ctrl )
{
    EL_SYMBOL* sym = (EL_SYMBOL*) self->symw.Symbol;

    p_bfil( buf, SNAME_MAX_Z, 0 );
    p_bcpy( buf, sym->name, SNAME_MAX );
}

/***************************************************************************
 **  sym_scale  Copy the current scale and prompt string to given buffer.
 **  ~~~~~~~~~
 */

METHOD VOID symw_syw_scale( PR_SYMW* self, TEXT* buf, PU_CTRL* ctrl )
{
    TEXT fstr[PU_MAX_TEXT];
    SYMSET* pset;

    pset = self->symw.pSymset;
    LoadResBuf( SEL_SYM_SCALE, fstr, PU_MAX_TEXT );
    p_atos( buf, fstr, pset->scale.q, pset->scale.d );
}

/***************************************************************************
 **  sym_rotate  Copy the current rotate angle and prompt string to given
 **  ~~~~~~~~~~  buffer.
 */

METHOD VOID symw_syw_rotate( PR_SYMW* self, TEXT* buf, PU_CTRL* ctrl )
{
    TEXT nbuf[10];
    TEXT fstr[PU_MAX_TEXT];
    E_CONFIG cfg;
    P_DTOB dfmt;
    DOUBLE angle;

    p_getctd( &cfg );
    dfmt.type = P_DTOB_GENERAL;
    dfmt.width = 6;
    dfmt.ndec = 2; /* Ignored? */
    dfmt.point = cfg.decimalSeparator;
    dfmt.trilen = 0;
    p_bfil( nbuf, sizeof(nbuf), 0 );
    p_send4( Drg, O_DG_GET_ANGLE_DBL, &angle, &self->symw.pSymset->rotate );
    p_dtob( nbuf, &angle, &dfmt );

    LoadResBuf( SEL_SYM_ROTATE, fstr, PU_MAX_TEXT );
    p_atos( buf, fstr, nbuf );
}

/***************************************************************************
 **  sym_mirror  Copy the current flip and prompt string to given buffer.
 **  ~~~~~~~~~~
 */

METHOD VOID symw_syw_mirror( PR_SYMW* self, TEXT* buf, PU_CTRL* ctrl )
{
    TEXT fstr[32];
    TEXT mstr[32];
    INT rid;

    LoadResBuf( SEL_SYM_MIRROR, fstr, 32 );
    switch( self->symw.pSymset->mirror )
    {
    case 0:
        rid = SEL_SYM_NONE;
        break;
    case 1:
        rid = SEL_SYM_HORIZONTAL;
        break;
    case 2:
        rid = SEL_SYM_VERTICAL;
        break;
    }
    LoadResBuf( rid, mstr, 32 );
    p_atos( buf, fstr, mstr );
}

/***************************************************************************
 **  sym_create  Copy the current create type and prompt string to given
 **  ~~~~~~~~~~  buffer.
 */

METHOD VOID symw_syw_create( PR_SYMW* self, TEXT* buf, PU_CTRL* ctrl )
{
    TEXT fstr[32];
    TEXT mstr[32];
    INT rid;

    LoadResBuf( SEL_SYM_CREATE, fstr, 32 );
    rid = self->symw.pSymset->create ? SEL_SYM_COPY : SEL_SYM_LINK;
    LoadResBuf( rid, mstr, 32 );
    p_atos( buf, fstr, mstr );
}

/***************************************************************************
 **  sym_view_scale  Copy the current viewing scale and prompt string to
 **  ~~~~~~~~~~~~~~  given buffer.
 */

METHOD VOID symw_syw_view_scale( PR_SYMW* self, TEXT* buf, PU_CTRL* ctrl )
{
    TEXT fstr[PU_MAX_TEXT];

    LoadResBuf( SEL_SYM_VIEW_SCALE, fstr, PU_MAX_TEXT );
    p_atos( buf, fstr, self->symw.ViewScale );
}

/*---------------------------[ Select Lib Dialog ]------------------------*/

/***************************************************************************
 **  dl_dyn_init  Initiate Select Library Dialog
 **  ~~~~~~~~~~~
 */

METHOD VOID slibdlg_dl_dyn_init( PR_SLIBDLG* self )
{
    SLIBDLG_DATA* data;
    SE_CHLIST selist;

    data = self->dlgbox.rbuf;

    selist.data = (PR_VAROOT*) data->vastr;
    selist.nsel = data->current;
    selist.set_flags = SE_CHLIST_NSEL | SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, 1, &selist );
}

/***************************************************************************
 **  dl_key  Complete Select Library Dialog
 **  ~~~~~~
 */

METHOD INT slibdlg_dl_key( PR_SLIBDLG* self, INT index, INT keycode,
    INT actbut )
{
    SLIBDLG_DATA* data;
    TEXT* libname;
    LIB* pLib;
    INT i;

    /* We only get here if Enter is pressed */
    data = self->dlgbox.rbuf;

    i = hDlgSenseChlist( 1 );
    libname = (TEXT*) p_send3( data->vastr, O_VA_PREC, i );
    i = p_send3( Drg, O_DG_GET_LIB, libname );
    data->nsel = i;
    if( i < 0 ) return WN_KEY_CHANGED;

    pLib = &Drg->drg.LibList[i];
    if( pLib->pfcb == NULL )
    {
        p_send3( Drg, O_DG_OPEN_LIB, pLib );
        if( pLib->pfcb == NULL )
        {
        }
    }

    return WN_KEY_CHANGED;
}

/*---------------------------[ Select Sym Dialog ]------------------------*/

/***************************************************************************
 **  dl_dyn_init  Initiate Select Symbol Dialog
 **  ~~~~~~~~~~~
 */

METHOD VOID ssymdlg_dl_dyn_init( PR_SSYMDLG* self )
{
    SLIBDLG_DATA* data;
    SE_CHLIST selist;

    data = self->dlgbox.rbuf;

    selist.data = (PR_VAROOT*) data->vastr;
    selist.nsel = data->current;
    selist.set_flags = SE_CHLIST_NSEL | SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, 1, &selist );
}

/***************************************************************************
 **  dl_key  Complete Select Symbol Dialog
 **  ~~~~~~
 */

METHOD INT ssymdlg_dl_key( PR_SSYMDLG* self, INT index, INT keycode,
    INT actbut )
{
    SLIBDLG_DATA* data;

    /* We only get here if Enter is pressed */
    data = self->dlgbox.rbuf;

    data->nsel = hDlgSenseChlist( 1 );
    return WN_KEY_CHANGED;
}

/*------------------------[ Symbol Settings Dialog ]----------------------*/

/***************************************************************************
 **  dl_item_new  Create a Set Scale dialog item class
 **  ~~~~~~~~~~~
 */

METHOD VOID* symsetdlg_dl_item_new( PR_SYMSETDLG* self, AD_DLGBOX* par )
{
    return (VOID*) p_send3( Drg, O_DG_NEW_CLASS, par->class );
}

/***************************************************************************
 **  dl_dyn_init  Initiate Symbol Settings Dialog
 **  ~~~~~~~~~~~
 */

METHOD VOID symsetdlg_dl_dyn_init( PR_SYMSETDLG* self )
{
    SYMSET* pset;
    DOUBLE angle;

    pset = (SYMSET*) self->dlgbox.rbuf;

    hDlgSet( 1, &pset->scale );
    p_send4( Drg, O_DG_GET_ANGLE_DBL, &angle, &pset->rotate );
    hDlgSetFledit( 2, &angle );
    hDlgSetChlist( 3, pset->mirror );
    hDlgSetChlist( 4, pset->create );
}

/***************************************************************************
 **  dl_key  Complete Symbol Settings Dialog
 **  ~~~~~~
 */

METHOD INT symsetdlg_dl_key( PR_SYMSETDLG* self, INT index, INT keycode,
    INT actbut )
{
    SYMSET* pset;
    DOUBLE angle;

    /* We only get here if Enter is pressed */
    pset = (SYMSET*) self->dlgbox.rbuf;

    hDlgSense( 1, &pset->scale );
    hDlgSenseFledit( 2, &angle );
    p_send4( Drg, O_DG_SET_ANGLE, &pset->rotate, &angle );
    pset->mirror = hDlgSenseChlist( 3 );
    pset->create = hDlgSenseChlist( 4 );

    return WN_KEY_CHANGED;
}

/* End of VSYMWIN.C file */
