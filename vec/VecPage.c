/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: PAGE DIALOG CLASS MEMBERS        *  Date Started:  8 Jan 1998  *
 *    File: VECPAGE.C       Type: C SOURCE   *  Date Revised:  8 Jan 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1998 Nick Matthews
*/

#include <hwim.h>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"

AUNIT CheckPageValue( UBYTE index )
{
    AUNIT val;

    /*val = DlgSenseAunit( index );*/
    val = DlgSensePaperAunit( index );
    if( val < 1000 )
    {
        val = 1000;
        hInfoPrint( SR_PAGE_SIZE_TOO_LOW );
        hBeep();
    }
    if( val > 32767 )
    {
        val = 32500;
        hInfoPrint( SR_PAGE_SIZE_TOO_HIGH );
        hBeep();
    }
    return val;
}

static VOID SetMargins( PR_SPAGEDLG* self )
{
    TEXT buf[40];
    TEXT tbuf[DIM_TEXT_MAX_Z];
    TEXT lbuf[DIM_TEXT_MAX_Z];
    TEXT rbuf[DIM_TEXT_MAX_Z];
    TEXT bbuf[DIM_TEXT_MAX_Z];

    LaunitToPaperText( tbuf, self->spagedlg.mg[MARGIN_TOP] );
    LaunitToPaperText( lbuf, self->spagedlg.mg[MARGIN_LEFT] );
    LaunitToPaperText( rbuf, self->spagedlg.mg[MARGIN_RIGHT] );
    LaunitToPaperText( bbuf, self->spagedlg.mg[MARGIN_BOTTOM] );
    hAtos( buf, SR_PAGE_MARGINS_FMT, tbuf, lbuf, rbuf, bbuf );
    hDlgSetText( SPAGEDLG_MARGIN, buf );
}

#pragma METHOD_CALL

/***************************************************************************
 **  spagedlg  Page dialog class members
 **  ~~~~~~~~
 */

METHOD VOID spagedlg_dl_changed( PR_SPAGEDLG* self, INT index )
{
    UWORD size;
    TEXT hbuf[DIM_TEXT_MAX_Z];
    TEXT wbuf[DIM_TEXT_MAX_Z];
    TEXT* ht;
    TEXT* wd;

    size = hDlgSenseChlist( SPAGEDLG_SIZE );
    switch( size )
    {
    case 0:  /* Custom */
        LaunitToPaperText( wbuf, self->spagedlg.wd );
        LaunitToPaperText( hbuf, self->spagedlg.ht );
        break;
    case 1:  /* A4 */
        LaunitToPaperText( wbuf, 21000 );
        LaunitToPaperText( hbuf, 29700 );
        break;
    case 2:  /* Letter */
        LaunitToPaperText( wbuf, 21590 );
        LaunitToPaperText( hbuf, 27940 );
        break;
    }
    wd = wbuf;
    ht = hbuf;
    if( size && hDlgSenseChlist( SPAGEDLG_ORIENT ) )
    {
        /* Not custom and is Landscape */
        wd = hbuf;
        ht = wbuf;
    }
    hDlgSetEdwin( SPAGEDLG_WIDTH, wd );
    hDlgSetEdwin( SPAGEDLG_HEIGHT, ht );

    if( size )
    {
        /* Not Custom */
        p_send4( self, O_DL_ITEM_LOCK, SPAGEDLG_WIDTH, TRUE );
        p_send4( self, O_DL_ITEM_LOCK, SPAGEDLG_HEIGHT, TRUE );
        p_send4( self, O_DL_ITEM_LOCK, SPAGEDLG_ORIENT, FALSE );
    }
    else
    {
        /* Custom */
        p_send4( self, O_DL_ITEM_LOCK, SPAGEDLG_WIDTH, FALSE );
        p_send4( self, O_DL_ITEM_LOCK, SPAGEDLG_HEIGHT, FALSE );
        p_send4( self, O_DL_ITEM_LOCK, SPAGEDLG_ORIENT, TRUE );
        if( self->spagedlg.wd > self->spagedlg.ht )
        {
            hDlgSetChlist( SPAGEDLG_ORIENT, 1 );
        }
        else
        {
            hDlgSetChlist( SPAGEDLG_ORIENT, 0 );
        }
    }
}

METHOD VOID spagedlg_dl_dyn_init( PR_SPAGEDLG* self )
{
    SetTitlePaperUnits( SR_SET_PAGE_FMT );

    self->spagedlg.wd = Drg->vec.Page.lim.x - Drg->vec.Page.pos.x;
    self->spagedlg.ht = Drg->vec.Page.lim.y - Drg->vec.Page.pos.y;
    self->spagedlg.mg[MARGIN_LEFT]   = Drg->vec.Marg.pos.x - Drg->vec.Page.pos.x;
    self->spagedlg.mg[MARGIN_TOP]    = Drg->vec.Marg.pos.y - Drg->vec.Page.pos.y;
    self->spagedlg.mg[MARGIN_RIGHT]  = Drg->vec.Page.lim.x - Drg->vec.Marg.lim.x;
    self->spagedlg.mg[MARGIN_BOTTOM] = Drg->vec.Page.lim.y - Drg->vec.Marg.lim.y;

    hDlgSetChlist( SPAGEDLG_SIZE, Drg->vec.Pref.page.size );
    hDlgSetChlist( SPAGEDLG_ORIENT, Drg->vec.Pref.page.orient );
    SetMargins( self );
    hDlgSetChlist( SPAGEDLG_DISPLAY, Drg->vec.PgDisp );

    spagedlg_dl_changed( self, 1 );
}

METHOD INT spagedlg_dl_key( PR_SPAGEDLG* self, INT index, INT keycode, INT actbut )
{
    AUNIT wd, ht;

    /* We only get here if Enter is pressed */
    Drg->vec.Pref.page.size = hDlgSenseChlist( SPAGEDLG_SIZE );
    Drg->vec.Pref.page.orient = hDlgSenseChlist( SPAGEDLG_ORIENT );

    wd = CheckPageValue( SPAGEDLG_WIDTH );
    ht = CheckPageValue( SPAGEDLG_HEIGHT );
    if( self->spagedlg.mg[MARGIN_LEFT] + self->spagedlg.mg[MARGIN_RIGHT] >= wd ||
        self->spagedlg.mg[MARGIN_TOP] + self->spagedlg.mg[MARGIN_BOTTOM] >= ht )
    {
        hInfoPrint( SR_MARGINS_TOO_BIG );
        return WN_KEY_CHANGED;
    }

    BegDraw();
    p_send5( Drg, O_DG_SET_PAGE, wd, ht, self->spagedlg.mg );
    Drg->vec.PgDisp = hDlgSenseChlist( SPAGEDLG_DISPLAY );
    EndDraw();

    return WN_KEY_CHANGED;
}

METHOD VOID spagedlg_dl_focus( PR_SPAGEDLG* self, WORD index, WORD flag )
{
    if( flag != FALSE ) return;

    switch( index )
    {
    case SPAGEDLG_WIDTH:
        self->spagedlg.wd = CheckPageValue( index );
        break;
    case SPAGEDLG_HEIGHT:
        self->spagedlg.ht = CheckPageValue( index );
        break;
    }
    spagedlg_dl_changed( self, 1 );
}

METHOD VOID spagedlg_dl_launch_sub( PR_SPAGEDLG* self, INT index )
{
    if( LaunchDialog( self->spagedlg.mg, SET_MARGINS_DIALOG, C_SMARGDLG ) )
    {
        SetMargins( self );
    }
}

/***************************************************************************
 **  smargdlg  Page margins dialog class members
 **  ~~~~~~~~
 */

METHOD VOID smargdlg_dl_dyn_init( PR_SMARGDLG* self )
{
    AUNIT* marg;

    marg = self->dlgbox.rbuf;

    SetTitlePaperUnits( SR_MARGIN_TITLE_FMT );
    DlgSetPaperAunit( SMARGDLG_TOP, marg[MARGIN_TOP] );
    DlgSetPaperAunit( SMARGDLG_LEFT, marg[MARGIN_LEFT] );
    DlgSetPaperAunit( SMARGDLG_RIGHT, marg[MARGIN_RIGHT] );
    DlgSetPaperAunit( SMARGDLG_BOTTOM, marg[MARGIN_BOTTOM] );
}

METHOD INT smargdlg_dl_key( PR_SMARGDLG* self, INT index, INT keycode, INT actbut )
{
    AUNIT* marg;

    marg = self->dlgbox.rbuf;

    marg[MARGIN_TOP] = DlgSensePaperAunit( SMARGDLG_TOP );
    marg[MARGIN_LEFT] = DlgSensePaperAunit( SMARGDLG_LEFT );
    marg[MARGIN_RIGHT] = DlgSensePaperAunit( SMARGDLG_RIGHT );
    marg[MARGIN_BOTTOM] = DlgSensePaperAunit( SMARGDLG_BOTTOM );

    return WN_KEY_CHANGED;
}

/* End of VecPage.c file */
