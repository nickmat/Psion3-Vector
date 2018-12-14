/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: TEXT STYLE DIALOGS CLASS MEMBERS *  Date Started: 21 Mar 1997  *
 *    File: VECTDLG.C       Type: C SOURCE   *  Date Revised: 22 Mar 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <hwim.h>
#include <edwin.g>
#include <chlist.g>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"

static VOID FillTextDlg( PR_ATEXTDLG* self )
{
    TEXTDLG_DATA* tsd;
    TSTYLE* style;
    PR_LKLIST* locs;
    TEXT* fontname;
    SE_CHLIST selist;
    TEXT buf[10];
    DOUBLE angle;

    tsd = self->dlgbox.rbuf;
    style = &tsd->ts;

    hDlgSetEdwin( ATEXTDLG_NAME, (TEXT*) style->name );

    locs = f_new( CAT_VECTOR_VECTOR, C_LKLIST );
    self->atextdlg.list =
        (PR_VASTR*) p_send4( locs, O_LKL_CREATE_LIST, "vft", TRUE );
    hDestroy( locs );
    p_send4( self->atextdlg.list, O_VA_INSERT, 0, "*Default" );

    selist.data = (PR_VAROOT*) self->atextdlg.list;
    fontname = Drg->drg.FontList[ style->font ].fname;
    selist.nsel = p_send3( self->atextdlg.list, O_VA_SEARCH, fontname );
    if( selist.nsel < 0 ) selist.nsel = 0;
    selist.set_flags = SE_CHLIST_NSEL | SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, ATEXTDLG_FONT, &selist );

    hDlgSetChlist( ATEXTDLG_UNITS, style->units );
    GetTextHtStr( buf, style->set.smul, style->set.sdiv, style->units );
    hDlgSetEdwin( ATEXTDLG_SIZE, buf );

    GetAngleDbl( &angle, style->set.a.sin, style->set.a.cos );
    hDlgSetFledit( ATEXTDLG_ANGLE, &angle );
}

static VOID ReadTextDlg( PR_ATEXTDLG* self )
{
    TEXTDLG_DATA* tsd;
    TSTYLE* style;
    INT i;
    TEXT* str;
    DOUBLE angle;
    TEXT* fontname;

    tsd = self->dlgbox.rbuf;
    style = &tsd->ts;

    p_scpy( (TEXT*) style->name, hDlgSenseEdwin( ATEXTDLG_NAME ) );

    i = hDlgSenseChlist( ATEXTDLG_FONT );
    fontname = (TEXT*) p_send3( self->atextdlg.list, O_VA_PREC, i );
    style->font = p_send3( Drg, O_DG_GET_FONT, fontname );
    if( style->font < 0 ) style->font = 0;

    style->units = hDlgSenseChlist( ATEXTDLG_UNITS );
    str = hDlgSenseEdwin( ATEXTDLG_SIZE );
    SetTextHt( &style->set.smul, &style->set.sdiv, str, style->units );

    hDlgSenseFledit( ATEXTDLG_ANGLE, &angle );
    SetAngle( &style->set.a.sin, &style->set.a.cos, &angle );
}

#pragma METHOD_CALL

/*----------------------------[ Set Text Dialog ]-------------------------*/

METHOD VOID stextdlg_dl_dyn_init( PR_STEXTDLG* self )
{
    TEXTDLG_DATA* tsd;
    int i;
    TSTYLE* ts;
    FONT* ft;
    TEXT buf[60], sbuf[10], ubuf[10], abuf[10];
    SE_CHLIST selist;

    tsd = self->dlgbox.rbuf;

    self->stextdlg.list = f_new( CAT_VECTOR_OLIB, C_VASTR );
    p_send3( self->stextdlg.list, O_VA_INIT, UNAME_MAX * 2 );
    for( i = 0 ; i < Drg->drg.TextSize ; i++ )
    {
        ts = &Drg->drg.TextList[i];
        ft = &Drg->drg.FontList[ ts->font ];
        GetTextHtStr( sbuf, ts->set.smul, ts->set.sdiv, ts->units );
        hLoadChlistResBuf( FONT_UNITS_LIST, ts->units, ubuf );
        GetAngleStr( abuf, ts->set.a.sin, ts->set.a.cos );
        hAtos( buf, SR_TSTYLE_FMT, ts->name, ft->fname, sbuf, ubuf, abuf );
        p_send5( self->stextdlg.list, O_VA_INSERTM, i, buf, 1 );
    }
    selist.data = (PR_VAROOT*) self->stextdlg.list;
    selist.nsel = tsd->select;
    selist.set_flags = SE_CHLIST_NSEL | SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, 1, &selist );
}

METHOD INT stextdlg_dl_key( PR_STEXTDLG* self, INT index, INT keycode,
    INT actbut )
{
    TEXTDLG_DATA* tsd;

    /*
    ** We get here if Select (Enter), Change (Space) or Delete (Del)
    ** is pressed
    */

    tsd = self->dlgbox.rbuf;
    tsd->select = hDlgSenseChlist( 1 );

    return WN_KEY_CHANGED;
}

/*----------------------------[ Add Text Dialog ]-------------------------*/

METHOD VOID atextdlg_dl_dyn_init( PR_ATEXTDLG* self )
{
    TEXTDLG_DATA* tsd;

    tsd = self->dlgbox.rbuf;
    tsd->ts = Drg->drg.TextList[ tsd->select ];
    hAtos( (TEXT*) tsd->ts.name, SR_TSTYLE_NAME_FMT, Drg->vec.NewText + 2 );

    FillTextDlg( self );
}

METHOD INT atextdlg_dl_key( PR_ATEXTDLG* self, INT index, INT keycode,
    INT actbut )
{
    TEXTDLG_DATA* tsd;
    TSTYLE* tsl;
    INT i;

    /* We only get here if Enter is pressed */

    ReadTextDlg( self );

    tsd = self->dlgbox.rbuf;

    i = Drg->drg.TextSize;
    tsl = p_realloc( Drg->drg.TextList, sizeof(TSTYLE) * (i+1) );
    if( tsl == NULL ) return WN_KEY_CHANGED;

    tsl[i] = tsd->ts;
    tsd->select = i;

    Drg->drg.TextList = tsl;
    Drg->drg.TextSize++;
    Drg->vec.NewText++;

    return WN_KEY_CHANGED;
}

METHOD VOID atextdlg_dl_changed( PR_ATEXTDLG* self, INT index )
{
    TEXTDLG_DATA* tsd;
    TSTYLE* ts;
    TEXT buf[UNAME_MAX+1];
    TEXT* str;

    tsd = self->dlgbox.rbuf;
    ts = &tsd->ts;

    switch( index )
    {
    case ATEXTDLG_UNITS:
        ts->units = hDlgSenseChlist( ATEXTDLG_UNITS );
        GetTextHtStr( buf, ts->set.smul, ts->set.sdiv, ts->units );
        hDlgSetEdwin( ATEXTDLG_SIZE, buf );
        break;
    case ATEXTDLG_SIZE:
        str = hDlgSenseEdwin( ATEXTDLG_SIZE );
        SetTextHt( &ts->set.smul, &ts->set.sdiv, str, ts->units );
        break;
    }
}

/*--------------------------[ Change Text Dialog ]------------------------*/

METHOD VOID ctextdlg_dl_dyn_init( PR_CTEXTDLG* self )
{
    TEXTDLG_DATA* tsd;
    TEXT title[30];

    tsd = self->dlgbox.rbuf;
    tsd->ts = Drg->drg.TextList[ tsd->select ];

    hLoadResBuf( SR_CREATE_TSTYLE, title );
    hDlgSetText( 0, title );
    FillTextDlg( (PR_ATEXTDLG*) self );
}

METHOD INT ctextdlg_dl_key( PR_CTEXTDLG* self, INT index, INT keycode,
    INT actbut )
{
    TEXTDLG_DATA* tsd;

    /* We only get here if Enter is pressed */

    ReadTextDlg( (PR_ATEXTDLG*) self );
    tsd = self->dlgbox.rbuf;
    Drg->drg.TextList[ tsd->select ] = tsd->ts;

    return WN_KEY_CHANGED;
}

/* End of VECTDLG.C file */
