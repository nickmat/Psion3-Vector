/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: GENERAL DIALOG CLASS MEMBERS     *  Date Started: 17 Sep 1996  *
 *    File: VECDLG.C        Type: C SOURCE   *  Date Revised:  8 Jan 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1996-1998, Nick Matthews
*/

#include <hwim.h>
#include <limits.h>
#include <p_math.h>
#include <ncedit.g>
#include <edwin.g>
#include <chlist.g>
#include <files.g>
#include <vector.g>
#include "vecbld.h"
#include <vector.rsg>
#include "vector.h"


static INT GridShowList[] = { 1, 2, 4, 5, 8, 10 };

VOID SetTitleScaleUnits( UINT format )
{
    TEXT buf[80];
    TEXT* unit;

    unit = Drg->vec.Pref.unitname;
    hAtos( buf, format, ( unit[0] ? " " : "" ), unit );
    hDlgSetText( 0, buf );
}

VOID DlgSetScaleAunit( UBYTE index, AUNIT au )
{
    TEXT buf[DIM_TEXT_MAX_Z];

    LaunitToScaleText( buf, au );
    hDlgSetEdwin( index, buf );
}

AUNIT DlgSenseScaleAunit( UBYTE index )
{
    return ScaleTextToAunit( hDlgSenseEdwin( index ) );
}

VOID DlgSetScaleLaunit( UBYTE index, LAUNIT* lau )
{
    TEXT buf[DIM_TEXT_MAX_Z];

    LaunitToScaleText( buf, *lau );
    hDlgSetEdwin( index, buf );
}

VOID DlgSenseScaleLaunit( UBYTE index, LAUNIT* lau )
{
    ScaleTextToLaunit( lau, hDlgSenseEdwin( index ) );
}

VOID SetTitlePaperUnits( UINT format )
{
    TEXT buf[80];
    TEXT unit[10];

    hLoadChlistResBuf( SET_UNITS_LIST, Drg->vec.Units, unit );
    hAtos( buf, format, unit );
    hDlgSetText( 0, buf );
}

VOID DlgSetPaperAunit( UBYTE index, AUNIT au )
{
    TEXT buf[DIM_TEXT_MAX_Z];

    LaunitToPaperText( buf, au );
    hDlgSetEdwin( index, buf );
}

AUNIT DlgSensePaperAunit( UBYTE index )
{
    return PaperTextToAunit( hDlgSenseEdwin( index ) );
}

VOID DlgSetPaperRAunit( UBYTE index, AUNIT au, AUNIT orig )
{
    LAUNIT lau;
    TEXT buf[DIM_TEXT_MAX_Z];

    lau = (LONG) au - orig;
    LaunitToPaperText( buf, lau );
    hDlgSetEdwin( index, buf );
}

AUNIT DlgSensePaperRAunit( UBYTE index, AUNIT orig )
{
    LAUNIT lau;

    PaperTextToLaunit( &lau, hDlgSenseEdwin( index ) );
    return (AUNIT) ( lau + orig );
}

VOID DlgSetScaleRpt( UBYTE index, A_PT* pPt, A_PT* pOrig )
{
    TEXT buf[RPT_TEXT_MAX_Z];

    RptToScaleText( buf, pPt, pOrig );
    hDlgSetText( index, buf );
}

#pragma save, METHOD_CALL

/*---------------------------[ Open File Dialog ]-------------------------*/

METHOD VOID opendlg_dl_dyn_init( PR_OPENDLG* self )
{
    OPENDLG_DATA* pData;

    pData = self->dlgbox.rbuf;
    if( pData->fname[0] != '\0' )
    {
        hDlgSet( OPENDLG_NAME, pData->fname );
    }
}

METHOD INT opendlg_dl_key( PR_OPENDLG* self, INT index, INT keycode, INT actbut )
{
    OPENDLG_DATA* pData;

    pData = self->dlgbox.rbuf;
    hDlgSense( OPENDLG_NAME, pData->fname );
    pData->format = hDlgSenseChlist( OPENDLG_TYPE );
    return WN_KEY_CHANGED;
}

/*---------------------------[ File New Dialog ]--------------------------*/

METHOD VOID cnewdlg_dl_dyn_init( PR_CNEWDLG* self )
{
    PR_LKLIST* scan;
    TEXT buf[20];
    SE_CHLIST selist;
    INT def;

    scan = (PR_LKLIST*) f_new( CAT_VECTOR_VECTOR, C_LKLIST );
    self->cnewdlg.tplist =
        (PR_VASTR*) p_send4( scan, O_LKL_CREATE_LIST, "vtp", TRUE );
    hDestroy( scan );

    hLoadResBuf( SR__DEFAULT_, buf );
    p_send4( self->cnewdlg.tplist, O_VA_INSERT, 0, buf );
    def = p_send3( self->cnewdlg.tplist, O_VA_SEARCH, "Default" );

    if( def < 0 )
    {
        def = 0;
    }
    selist.nsel = def;
    selist.data = (PR_VAROOT*) self->cnewdlg.tplist;
    selist.set_flags = SE_CHLIST_DATA | SE_CHLIST_NSEL;
    p_send4( self, O_WN_SET, CNEWDLG_TEMPLATE, &selist );
}

METHOD INT cnewdlg_dl_key( PR_CNEWDLG* self, INT index, INT keycode, INT actbut )
{
    CNEWDLG_DATA* data;
    INT tpnum;

    data = self->dlgbox.rbuf;

    tpnum = hDlgSenseChlist( CNEWDLG_TEMPLATE );
    if( tpnum )
    {
        p_send4( self->cnewdlg.tplist, O_VA_COPY, tpnum, data->tplate );
    }
    else
    {
        data->tplate[0] = '\0';
    }

    hDlgSense( CNEWDLG_NAME, data->fname );
    return WN_KEY_CHANGED;
}

METHOD VOID cnewdlg_dl_changed( PR_CNEWDLG* self, INT index )
{
    PR_FNEDIT* fned;
    INT ftype;

    if( index == CNEWDLG_TYPE )
    {
        ftype = hDlgSenseChlist( CNEWDLG_TYPE );
        fned = (PR_FNEDIT*) p_send3( self, O_DL_INDEX_TO_HANDLE, CNEWDLG_NAME );
        SetExtension( fned->fnedit.extbuf, ftype, FTYPE_IMPORT );
        fned->fnedit.defext = fned->fnedit.extbuf;
    }
}

/*-------------------------[ List Symbols Dialog ]------------------------*/

METHOD VOID listsymdlg_dl_dyn_init( PR_LISTSYMDLG* self )
{
    PR_VASTR* name;
    PR_VASTR* ref;
    EL_SYMBOLHDR sym;
    TEXT buf[10];
    INT i;
    SE_CHLIST selist;

    name = f_new( CAT_VECTOR_OLIB, C_VASTR );
    p_send3( name, O_VA_INIT, LNAME_MAX_Z * 4 );
    ref = f_new( CAT_VECTOR_OLIB, C_VASTR );
    p_send3( ref, O_VA_INIT, LNAME_MAX_Z * 4 );
    for( i = 0 ; i < Drg->drg.SymbolCount ; i++ )
    {
        p_send4( Data, O_DD_READ, Drg->drg.SymbolList[i], &sym );
        /* Ensure name is zero terminated */
        sym.bound.pos.x = 0; /* Pretend you didn't see this! */
        p_send4( name, O_VA_INSERT, i, sym.name );
        p_atos( buf, "%d", sym.ref );
        p_send4( ref, O_VA_INSERT, i, buf );
    }

    selist.data = (PR_VAROOT*) name;
    selist.set_flags = SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, 1, &selist );

    selist.data = (PR_VAROOT*) ref;
    selist.set_flags = SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, 2, &selist );
}

METHOD VOID listsymdlg_dl_changed( PR_LISTSYMDLG* self, INT index )
{
    INT i;

    switch( index )
    {
    case 1:
        i = hDlgSenseChlist( 1 );
        hDlgSetChlist( 2, i );
        break;
    case 2:
        i = hDlgSenseChlist( 2 );
        hDlgSetChlist( 1, i );
    }
}

/*------------------[ Create Font Space Character Dialog ]----------------*/

METHOD VOID cspacedlg_dl_dyn_init( PR_CSPACEDLG* self )
{
    EL_CHAR* ch;
    AUNIT pitch;

    SetTitleScaleUnits( SR_CREATE_SPACE_FMT );

    if( Drg->drg.CharList && Drg->drg.CharList[32] != 0xffff )
    {
        p_send4( Data, O_DD_READ, Drg->drg.CharList[32], DBuf );
        ch = (EL_CHAR*) DBuf;
        pitch = ch->pitch;
    }
    else
    {
        pitch = 1000;
    }
    DlgSetScaleAunit( 1, pitch );
}

METHOD INT cspacedlg_dl_key( PR_CSPACEDLG* self, INT index, INT keycode, INT actbut )
{
    EL_CHAR ch;
    UINT hand;

    /* We only get here if Enter is pressed */
    ch.grp.size = sizeof(EL_CHAR);
    ch.grp.type = V_CHARACTER;
    ch.grp.count = 0;
    ch.grp.flags = 0;
    ch.ref = hDlgSenseNcedit( 2 );
    ch.spare = 0;
    ch.pitch = DlgSenseScaleAunit( 1 );
    ch.bound.pos.x = 0x8000;
    ch.bound.pos.y = 0x8000;
    ch.bound.lim.x = 0x8000 + ch.pitch;
    ch.bound.lim.y = 0x8000;
    ch.hot = ch.bound.pos;

    BegUndo( SR_CREATE_SPACE_UNDONE );
    p_send4( Undo, O_UD_SAVE, U_FONT_CHAR, 0 );
    if( Drg->drg.CharList )
    {
        hand = Drg->drg.CharList[ch.ref];
        if( hand != 0xffff )
        {
            if( hConfirm( SR_REPLACE_EXIST_CHAR, ch.ref ) )
            {
                p_send4( Data, O_DD_READ, hand, DBuf );
                p_send3( Data, O_DD_DELETE, hand );
                p_send4( Undo, O_UD_SAVE_INSERT, hand, DBuf );
            }
            else
            {
                EndUndo();
                return WN_KEY_NO_CHANGE;
            }
        }
    }
    hand = p_send3( Data, O_DD_ADD, &ch );
    p_send3( Undo, O_UD_SAVE_DELETE, hand );
    p_send2( Drg, O_DG_MAKE_CHAR_TAB );
    EndUndo();

    return WN_KEY_CHANGED;
}

/*-------------------------[ Snap Grid Dialog ]-----------------------*/

METHOD VOID sgriddlg_dl_dyn_init( PR_SGRIDDLG* self )
{
    INT flags;

    SetTitleScaleUnits( SR_GRID_SET_FMT );

    DlgSetScaleAunit( 1, Drg->vec.Pref.grid.space );

    flags = Drg->vec.Pref.grid.flags;
    hDlgSetChlist( 2, flags & GRID_SNAP );
    hDlgSetChlist( 3, ( flags & GRID_DISP ) >> 1 );
    hDlgSetChlist( 4, Drg->vec.Pref.grid.showlist & GRID_MINOR );
    hDlgSetChlist( 5, ( Drg->vec.Pref.grid.showlist & GRID_MAJOR ) >> 4 );
}

METHOD INT sgriddlg_dl_key( PR_SGRIDDLG* self, INT index, INT keycode, INT actbut )
{
    UINT result;
    GRIDSET grid;

    /* We only get here if Enter is pressed */

    grid.space = DlgSenseScaleAunit( 1 );

    result = hDlgSenseChlist( 2 );
    grid.flags = result;

    result = hDlgSenseChlist( 3 );
    grid.flags |= result << 1;

    grid.minor = hDlgSenseChlist( 4 );
    grid.major = hDlgSenseChlist( 5 );

    result = hDlgSenseChlist( 4 );
    grid.minor = GridShowList[ result ];
    grid.showlist = result;
    result = hDlgSenseChlist( 5 );
    grid.major = GridShowList[ result ];
    grid.showlist |= result << 4;

    p_send3( Drg, O_DG_SET_SNAP_GRID, &grid );

    return WN_KEY_CHANGED;
}

/*-------------------------[ View Settings Dialog ]-----------------------*/

METHOD VOID viewdlg_dl_dyn_init( PR_VIEWDLG* self )
{
    hDlgSetChlist( 1, ( Drg->vec.View & VW_STATUS ) >> 4 );
    hDlgSetChlist( 2, ( Drg->vec.View & VW_INFO ) != 0 );
    hDlgSetChlist( 3, Drg->vec.View & VW_SCROLL );
    /*hDlgSetChlist( 4, ( Drg->vec.View & VW_RULER ) >> 2 );*/
    /*hDlgSetChlist( 5, ( Drg->vec.View & VW_CMD ) != 0 );*/
}

METHOD INT viewdlg_dl_key( PR_VIEWDLG* self, INT index, INT keycode, INT actbut )
{
    INT result;

    /* We only get here if Enter is pressed */
    Drg->vec.View = 0;

    result = hDlgSenseChlist( 1 );
    Drg->vec.View |= result << 4;

    result = hDlgSenseChlist( 2 );
    Drg->vec.View |= result ? VW_INFO : 0;

    result = hDlgSenseChlist( 3 );
    Drg->vec.View |= result;

    /*result = hDlgSenseChlist( 4 );*/
    /*Drg->vec.View |= result << 2;*/

    /*result = hDlgSenseChlist( 5 );*/
    /*Drg->vec.View |= result ? VW_CMD : 0;*/

    p_send2( Drg, O_DG_SET_VIEW );

    return WN_KEY_CHANGED;
}

/*---------------------[ Diamond List Settings Dialog ]-------------------*/

METHOD VOID dlistdlg_dl_dyn_init( PR_DLISTDLG* self )
{
    hDlgSetChlist( 1, Drg->vec.DiamondList & VDL_FIRST );
    hDlgSetChlist( 2, Drg->vec.DiamondList & VDL_SECOND );
    hDlgSetChlist( 3, Drg->vec.DiamondList & VDL_THIRD );
    hDlgSetChlist( 4, Drg->vec.DiamondList & VDL_FOURTH );
}

METHOD INT dlistdlg_dl_key( PR_DLISTDLG* self, INT index, INT keycode, INT actbut )
{
    UINT result;

    /* We only get here if Enter is pressed */
    Drg->vec.DiamondList = 0;

    result = hDlgSenseChlist( 1 );
    Drg->vec.DiamondList |= result;

    result = hDlgSenseChlist( 2 );
    Drg->vec.DiamondList |= result << 1;

    result = hDlgSenseChlist( 3 );
    Drg->vec.DiamondList |= result << 2;

    result = hDlgSenseChlist( 4 );
    Drg->vec.DiamondList |= result << 3;

    return WN_KEY_CHANGED;
}

/*------------------------[ Cursor Setting Dialog ]-----------------------*/

METHOD VOID setcurdlg_dl_dyn_init( PR_SETCURDLG* self )
{
    hDlgSetChlist( SETCURDLG_SIZE, Drg->drg.Cfg.cursor.size );
    hDlgSetChlist( SETCURDLG_COLOR, Drg->drg.Cfg.cursor.color );
    hDlgSetChlist( SETCURDLG_WIDTH, Drg->drg.Cfg.cursor.width );
    hDlgSetChlist( SETCURDLG_SELBOX, Drg->drg.Cfg.cursor.selbox );
}

METHOD INT setcurdlg_dl_key( PR_SETCURDLG* self, INT index, INT keycode,
    INT actbut )
{
    CURTYPE cursor;

    /* We only get here if Enter is pressed */
    cursor.size = hDlgSenseChlist( SETCURDLG_SIZE );
    cursor.color = hDlgSenseChlist( SETCURDLG_COLOR );
    cursor.width = hDlgSenseChlist( SETCURDLG_WIDTH );
    cursor.selbox = hDlgSenseChlist( SETCURDLG_SELBOX );

    p_send3( w_ws->wserv.cli, O_DW_SET_CURSOR, &cursor );

    if( hDlgSenseChlist( SETCURDLG_SAVE ) )
    {
        p_send3( Drg, O_DG_UPDATE_CFG, CFG_CURSOR );
    }
    return WN_KEY_CHANGED;
}

/*-------------------------[ Selection box Dialog ]-----------------------*/

METHOD VOID selboxdlg_dl_dyn_init( PR_SELBOXDLG* self )
{
    hDlgSetNcedit( 1, Drg->vec.SelectBox );
}

METHOD INT selboxdlg_dl_key( PR_SELBOXDLG* self, INT index, INT keycode, INT actbut )
{
    /* We only get here if Enter is pressed */
    BegDraw();
    Drg->vec.SelectBox = hDlgSenseNcedit( 1 );
    EndDraw();

    return WN_KEY_CHANGED;
}

/*--------------------[ Movement Key Setting Dialog ]-------------------*/

METHOD VOID setmovedlg_dl_dyn_init( PR_SETMOVEDLG* self )
{
    hDlgSetChlist( SETMOVEDLG_CUR, Drg->drg.Cfg.move.keys );
    hDlgSetChlist( SETMOVEDLG_CUR_SHIFT, Drg->drg.Cfg.move.shift );
    hDlgSetChlist( SETMOVEDLG_CUR_CTRL, Drg->drg.Cfg.move.ctrl );
    hDlgSetChlist( SETMOVEDLG_CUR_S_C, Drg->drg.Cfg.move.sh_ctrl );
    hDlgSetNcedit( SETMOVEDLG_SLOW, Drg->drg.Cfg.move.slow );
    hDlgSetNcedit( SETMOVEDLG_FAST, Drg->drg.Cfg.move.fast );
}

METHOD INT setmovedlg_dl_key( PR_SETMOVEDLG* self, INT index, INT keycode, INT actbut )
{
    /* We only get here if Enter is pressed */
    Drg->drg.Cfg.move.keys = hDlgSenseChlist( SETMOVEDLG_CUR );
    Drg->drg.Cfg.move.shift = hDlgSenseChlist( SETMOVEDLG_CUR_SHIFT );
    Drg->drg.Cfg.move.ctrl = hDlgSenseChlist( SETMOVEDLG_CUR_CTRL );
    Drg->drg.Cfg.move.sh_ctrl = hDlgSenseChlist( SETMOVEDLG_CUR_S_C );
    Drg->drg.Cfg.move.slow = hDlgSenseNcedit( SETMOVEDLG_SLOW );
    Drg->drg.Cfg.move.fast = hDlgSenseNcedit( SETMOVEDLG_FAST );

    if( hDlgSenseChlist( SETMOVEDLG_SAVE ) )
    {
        p_send3( Drg, O_DG_UPDATE_CFG, CFG_MOVE );
    }
    return WN_KEY_CHANGED;
}

/*---------------------[ Keyboard Map Setting Dialog ]--------------------*/

METHOD VOID skeydlg_dl_dyn_init( PR_SKEYDLG* self )
{
    INT i;

    for( i = 0 ; i < KEY_MAP_SIZE ; i++ )
    {
        self->skeydlg.keys[i] = Drg->drg.Cfg.keys[i] - KEY_MAP_FIRST;
    }
    hDlgSetChlist( SKEYDLG_COM, self->skeydlg.keys[0] );
}

METHOD INT skeydlg_dl_key( PR_SKEYDLG* self, INT index, INT keycode, INT actbut )
{
    INT i;

    /* We only get here if Enter is pressed */
    for( i = 0 ; i < KEY_MAP_SIZE ; i++ )
    {
        Drg->drg.Cfg.keys[i] = self->skeydlg.keys[i] + KEY_MAP_FIRST;
    }
    if( hDlgSenseChlist( SKEYDLG_SAVE ) )
    {
        p_send3( Drg, O_DG_UPDATE_CFG, CFG_KEYS );
    }

    return WN_KEY_CHANGED;
}

METHOD VOID skeydlg_dl_changed( PR_SKEYDLG* self, INT index )
{
    INT i;

    i = hDlgSenseChlist( SKEYDLG_KEY );
    switch( index )
    {
    case SKEYDLG_KEY:
        hDlgSetChlist( SKEYDLG_COM, self->skeydlg.keys[i] );
        break;
    case SKEYDLG_COM:
        self->skeydlg.keys[i] = hDlgSenseChlist( SKEYDLG_COM );
    }
}

/*------------------------[ Set Origin box Dialog ]-----------------------*/

METHOD VOID sorigdlg_dl_dyn_init( PR_SORIGDLG* self )
{
    SetTitlePaperUnits( SR_SET_ORIGIN_TITLE_FMT );
    DlgSetPaperRAunit( SORIGDLG_X, Drg->vec.Orig.x, Drg->vec.Page.pos.x );
    DlgSetPaperRAunit( SORIGDLG_Y, Drg->vec.Orig.y, Drg->vec.Page.pos.y );
    hDlgSetChlist( SORIGDLG_Y_DIR, Drg->vec.yDir );
}

METHOD INT sorigdlg_dl_key( PR_SORIGDLG* self, INT index, INT keycode, INT actbut )
{
    /* We only get here if Enter is pressed */

    BegDraw();

    Drg->vec.Orig.x = DlgSensePaperRAunit( SORIGDLG_X, Drg->vec.Page.pos.x );
    Drg->vec.Orig.y = DlgSensePaperRAunit( SORIGDLG_Y, Drg->vec.Page.pos.y );
    Drg->vec.yDir = hDlgSenseChlist( SORIGDLG_Y_DIR );

    p_send2( Drg, O_DG_SNAP_TO_GRID );
    EndDraw();
    UpdateInfo( IW_ALL );

    return WN_KEY_CHANGED;
}

/*----------------------[ Set Command Actions Dialog ]--------------------*/

METHOD VOID movcpydlg_dl_dyn_init( PR_MOVCPYDLG* self )
{
    hDlgSetChlist( MOVCPYDLG_SCALE, Drg->vec.MoveCopy & COM_ACT_SCALE );
    hDlgSetChlist( MOVCPYDLG_STRETCH, Drg->vec.MoveCopy & COM_ACT_STRETCH );
    hDlgSetChlist( MOVCPYDLG_ROTATE, Drg->vec.MoveCopy & COM_ACT_ROTATE );
    hDlgSetChlist( MOVCPYDLG_MIRROR, Drg->vec.MoveCopy & COM_ACT_MIRROR );
}

METHOD INT movcpydlg_dl_key( PR_MOVCPYDLG* self, INT index, INT keycode, INT actbut )
{
    UINT result;

    /* We only get here if Enter is pressed */
    Drg->vec.MoveCopy = COM_ACT_COPY;

    result = hDlgSenseChlist( MOVCPYDLG_SCALE );
    Drg->vec.MoveCopy |= result;

    result = hDlgSenseChlist( MOVCPYDLG_STRETCH );
    Drg->vec.MoveCopy |= result << 1;

    result = hDlgSenseChlist( MOVCPYDLG_ROTATE );
    Drg->vec.MoveCopy |= result << 2;

    result = hDlgSenseChlist( MOVCPYDLG_MIRROR );
    Drg->vec.MoveCopy |= result << 3;

    if( Build && Drg->drg.BandLayer != BL_GREY )
    {
        if( Build->build.CopyFlag & Drg->vec.MoveCopy )
        {
            Drg->drg.BandLayer = BL_BLACK;
        }
        else
        {
            Drg->drg.BandLayer = BL_OFF;
        }
        p_send2( Drg, O_DG_REDRAW );
    }

    return WN_KEY_CHANGED;
}

/*--------------------------[ Jump to box Dialog ]------------------------*/

METHOD VOID jmpboxdlg_dl_dyn_init( PR_JMPBOXDLG* self )
{
    SetTitlePaperUnits( SR_JUMP_BOX_FMT );
    DlgSetPaperAunit( 1, Drg->vec.Nearest );
}

METHOD INT jmpboxdlg_dl_key( PR_JMPBOXDLG* self, INT index, INT keycode, INT actbut )
{
    /* We only get here if Enter is pressed */
    Drg->vec.Nearest = DlgSensePaperAunit( 1 );

    return WN_KEY_CHANGED;
}

/*-----------------------[ Set Undo Active Dialog ]-----------------------*/

METHOD VOID undodlg_dl_dyn_init( PR_UNDODLG* self )
{
    hDlgSetChlist( 1, Drg->vec.UndoOff );
}

METHOD INT undodlg_dl_key( PR_UNDODLG* self, INT index, INT keycode, INT actbut )
{
    /* We only get here if Enter is pressed */
    Drg->vec.UndoOff = hDlgSenseChlist( 1 );

    if( Drg->vec.UndoOff )
    {
        p_send2( Undo, O_UD_KILL );
        p_send2( Redo, O_UD_KILL );
    }

    return WN_KEY_CHANGED;
}

/*-----------------------[ Zoom Step Setting Dialog ]---------------------*/

METHOD VOID setzoomdlg_dl_dyn_init( PR_SETZOOMDLG* self )
{
    hDlgSetNcedit( 1, Drg->vec.ZoomPercent - 100 );
}

METHOD INT setzoomdlg_dl_key( PR_SETZOOMDLG* self, INT index, INT keycode, INT actbut )
{
    /* We only get here if Enter is pressed */
    Drg->vec.ZoomPercent = hDlgSenseNcedit( 1 ) + 100;
    return WN_KEY_CHANGED;
}

/*--------------------------[ Scale Edit Lodger ]-------------------------*/

METHOD VOID scaledit_wn_init( PR_SCALEDIT* self, IN_LNCEDIT* pin_lncedit, PR_WIN* landlord )
{
    self->lodger.landlord = landlord;
    self->mfne.selected = FALSE;
    self->mfne.changed = FALSE;
    self->mfne.cField = 0;
    self->mfne.nField = 2;

    /* First field */
    self->mfne.f[0].flags = MFNE_LEFT_ALIGN;
    self->mfne.f[0].value = pin_lncedit->value;
    self->mfne.f[0].limits[MFNE_LOWER] = pin_lncedit->low;
    self->mfne.f[0].limits[MFNE_UPPER] = pin_lncedit->high;
    self->mfne.f[0].width = 3;
    self->mfne.f[0].separator = ':';
    /* Second nearly the same as the first */
    self->mfne.f[1] = self->mfne.f[0];
    self->mfne.f[1].flags = MFNE_LEFT_ALIGN | MFNE_SUPPRESS_SEPARATOR;

    p_supersend2( self, O_WN_SET );
}

METHOD VOID scaledit_wn_set( PR_SCALEDIT* self, SCALE* pscale )
{
    self->mfne.f[0].value = pscale->q;
    self->mfne.f[1].value = pscale->d;
    p_supersend2( self, O_WN_SET );
}

METHOD VOID scaledit_wn_sense( PR_SCALEDIT* self, SCALE* pscale )
{
    pscale->q = (UBYTE) self->mfne.f[0].value;
    pscale->d = (UBYTE) self->mfne.f[1].value;
}

/*----------------------[ Zoom Magnification Dialog ]---------------------*/

METHOD VOID* zoommagdlg_dl_item_new( PR_ZOOMMAGDLG* self, AD_DLGBOX* par )
{
    return f_new( CAT_VECTOR_VECTOR, par->class );
}

METHOD VOID zoommagdlg_dl_dyn_init( PR_ZOOMMAGDLG* self )
{
    SCALE mag;

    mag.q = 1;
    mag.d = 1;
    hDlgSet( 1, &mag );
}

METHOD INT zoommagdlg_dl_key( PR_ZOOMMAGDLG* self, INT index, INT keycode, INT actbut )
{
    SCALE mag;
    SWORD upp;

    /* We only get here if Enter is pressed */
    hDlgSense( 1, &mag );
    upp = ( 26 * mag.q ) / mag.d;

    p_send3( Drg, O_DG_SET_ZOOM, upp );

    return WN_KEY_CHANGED;
}

/*----------------------------[ Debug Dialog ]----------------------------*/

METHOD VOID dbdlg_dl_dyn_init( PR_DBDLG* self )
{
    DB_DATA* data;
    TEXT buf[50];

    data = self->dlgbox.rbuf;
    hAtos( buf, data->rid, data->code1, data->code2 );
    hDlgSetText( 1, buf );
    if( data->text )
    {
        hDlgSetText( 2, data->text );
    }
}

/* End of VECDLG.C file */
