/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: PROPERTY BUILD & DIALOG CLASSES  *  Date Started: 16 Apr 1997  *
 *    File: VECPROP.C       Type: C SOURCE   *  Date Revised:  1 May 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <hwim.h>
#include <chlist.g>
#include <vector.g>
#include <vecbld.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"

static INT GetSRid( INT type )
{
    switch( type )
    {
    case -1:
        return SR_V_SELECTION;
    case V_LINE:
        return SR_V_LINE;
    case V_POLYLINE:
        return SR_V_POLYLINE;
    case V_POLYGON:
        return SR_V_POLYGON;
    case V_CIRCLE:
        return SR_V_CIRCLE;
    case V_3PT_ARC:
        return SR_V_3PT_ARC;
    case V_BOX:
        return SR_V_BOX;
    case V_ARC:
        return SR_V_ARC;
    case V_LINK:
        return SR_V_LINK;
    case V_TEXT:
        return SR_V_TEXT;
    case V_GROUP:
        return SR_V_GROUP;
    case V_SYMBOL:
        return SR_V_SYMBOL;
    case V_CHARACTER:
        return SR_V_CHARACTER;
    }
    return SR_V_UNKNOWN;
}

static PR_VASTR* MakeItemList( PROP_ITEM* list, INT count )
{
    PR_VASTR* valist;
    TEXT buf[UNAME_MAX_Z];
    INT i;

    valist = f_new( CAT_VECBLD_OLIB, C_VASTR );
    p_send3( valist, O_VA_INIT, 64 );

    for( i = 0 ; i < count ; i++ )
    {
        hLoadResBuf( GetSRid( list[i].type ), buf );
        p_send4( valist, O_VA_INSERT, i, buf );
    }
    return valist;
}

static VOID SetTitle( INT type )
{
    TEXT buf[50];
    TEXT tname[UNAME_MAX_Z];
    INT rid;

    hLoadResBuf( GetSRid( type ), tname );
    if( type == -1 || type == V_GROUP )
    {
        rid = SR_PROP_TITLE_FMT;
    }
    else
    {
        rid = SR_PROP_TITLE_UNIT_FMT;
    }
    hAtos( buf, rid, tname, Drg->vec.Pref.unitname );
    hDlgSetText( 0, buf );
}

#pragma METHOD_CALL

/***************************************************************************
 **  aggrpdlg  Aggregate property dialog class members
 **  ~~~~~~~~
 */

METHOD VOID aggrpdlg_dl_dyn_init( PR_AGGRPDLG* self )
{
    APROP_DATA* data;
    TEXT buf[40];
    SE_CHLIST selist;

    data = self->dlgbox.rbuf;

    SetTitle( data->type );

    self->aggrpdlg.vaLayer = (PR_VASTR*) p_send2( Drg, O_DG_MAKE_LAYLIST );
    if( data->layer < 0 )
    {
        hLoadResBuf( SR_MULTIPLE, buf );
        p_send4( self->aggrpdlg.vaLayer, O_VA_INSERT, 0, buf );
        selist.nsel = 0;
    }
    else
    {
        selist.nsel = data->layer;
    }
    selist.data = (PR_VAROOT*) self->aggrpdlg.vaLayer;
    selist.set_flags = SE_CHLIST_NSEL | SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, AGGRP_LAYER, &selist );

    self->aggrpdlg.vaItems = MakeItemList( data->itemlist, data->itemsize );
    selist.data = (PR_VAROOT*) self->aggrpdlg.vaItems;
    selist.nsel = data->curitem;
    selist.set_flags = SE_CHLIST_NSEL | SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, self->aggrpdlg.ItemCtrl, &selist );
}

METHOD INT aggrpdlg_dl_key( PR_AGGRPDLG* self, INT index, INT keycode,
    INT actbut )
{
    APROP_DATA* data;

    /* check for layer change */
    data = self->dlgbox.rbuf;
    data->nlayer = hDlgSenseChlist( AGGRP_LAYER );
    if( data->layer < 0 )
    {
        --data->nlayer;
    }

    if( keycode == W_KEY_RETURN || keycode == 'A' )
    {
        Drg->drg.PropChanged = TRUE;
    }

    return WN_KEY_CHANGED;
}

METHOD VOID aggrpdlg_dl_changed( PR_AGGRPDLG* self, INT index )
{
    APROP_DATA* data;

    data = self->dlgbox.rbuf;
    data->curitem = hDlgSenseChlist( self->aggrpdlg.ItemCtrl );
}

/***************************************************************************
 **  grppdlg  Group and Selection property dialog class members. Subclasses
 **  ~~~~~~~  aggrpdlg. SELECTION and V_GROUP
 */

METHOD VOID grppdlg_dl_dyn_init( PR_GRPPDLG* self )
{
    self->aggrpdlg.ItemCtrl = GRPP_ITEMS;
    p_supersend2( self, O_DL_DYN_INIT );
}

/***************************************************************************
 **  charpdlg  Character property dialog class members. Subclasses aggrpdlg.
 **  ~~~~~~~~  V_CHARACTER
 */

METHOD VOID charpdlg_dl_dyn_init( PR_CHARPDLG* self )
{
    APROP_DATA* data;
    EL_CHAR* ch;

    self->aggrpdlg.ItemCtrl = CHARP_ITEMS;
    p_supersend2( self, O_DL_DYN_INIT );
    data = self->dlgbox.rbuf;
    ch = (EL_CHAR*) data->elem;

    p_send4( Drg, O_DG_DLG_SET_S_AU, CHARP_PITCH, ch->pitch );

    hDlgSetNcedit( CHARP_ASCII, ch->ref );
}

METHOD INT charpdlg_dl_key( PR_CHARPDLG* self, INT index, INT keycode,
    INT actbut )
{
    APROP_DATA* data;
    EL_CHAR* ch;

    /* We only get here if Enter is pressed */
    p_supersend5( self, O_DL_KEY, index, keycode, actbut );
    data = self->dlgbox.rbuf;
    ch = (EL_CHAR*) data->elem;

    ch->pitch = p_send3( Drg, O_DG_DLG_GET_S_AU, CHARP_PITCH );

    ch->ref = hDlgSenseNcedit( CHARP_ASCII );

    p_send2( Drg, O_DG_MAKE_CHAR_TAB );
    return WN_KEY_CHANGED;
}

METHOD INT charpdlg_dl_set_size( PR_CHARPDLG* self )
{
    APROP_DATA* data;
    EL_CHAR* ch;

    p_supersend2( self, O_DL_SET_SIZE );
    data = self->dlgbox.rbuf;
    ch = (EL_CHAR*) data->elem;

    p_send5( Drg, O_DG_DLG_SET_S_RPT, CHARP_HOTSPOT, &ch->hot, &Drg->vec.Orig );

    return 0;
}

METHOD VOID charpdlg_dl_launch_sub( PR_CHARPDLG* self, INT index )
{
    APROP_DATA* data;
    EL_CHAR* ch;

    data = self->dlgbox.rbuf;
    ch = (EL_CHAR*) data->elem;

    BldLaunchDialog( &ch->hot, POINT_PROP_DIALOG, C_PTDLG );
    p_send5( Drg, O_DG_DLG_SET_S_RPT, CHARP_HOTSPOT, &ch->hot, &Drg->vec.Orig );
}

/***************************************************************************
 **  sympdlg  Symbol property dialog class members. Subclasses aggrpdlg.
 **  ~~~~~~~  V_SYMBOL
 */

METHOD VOID sympdlg_dl_dyn_init( PR_SYMPDLG* self )
{
    APROP_DATA* data;
    EL_SYMBOL* sym;
    TEXT buf[SNAME_MAX_Z];

    self->aggrpdlg.ItemCtrl = SYMP_ITEMS;
    p_supersend2( self, O_DL_DYN_INIT );
    data = self->dlgbox.rbuf;
    sym = (EL_SYMBOL*) data->elem;

    hDlgSetNcedit( SYMP_REF, sym->ref );
    p_bcpy( buf, sym->name, SNAME_MAX );
    buf[SNAME_MAX] = '\0';
    hDlgSetText( SYMP_NAME, buf );
}

METHOD INT sympdlg_dl_key( PR_SYMPDLG* self, INT index, INT keycode,
    INT actbut )
{
    APROP_DATA* data;
    EL_SYMBOL* sym;
#if 0   /* RETAIN FOR FUTURE VERSION */
    UINT rec;
    A_PT* pt;
    TEXT buf[25];
#endif

    data = self->dlgbox.rbuf;
    sym = (EL_SYMBOL*) data->elem;

#if 0   /* RETAIN FOR FUTURE VERSION */
    if( keycode == 'P' )
    {
        if( self->sympdlg.NoAttach )
        {
            hInfoPrint( SR_NO_ATTACHED_PTS );
            return WN_KEY_NO_CHANGE;
        }
        rec = hDlgSenseChlist( SYMP_ATTACH );
        pt = &sym->attach[rec];
        BldLaunchDialog( pt, POINT_PROP_DIALOG, C_PTDLG );
        p_send5( Drg, O_DG_RPT_TO_S_TEXT, buf, pt, &Drg->vec.Orig );
        p_send4( self->sympdlg.vapts, O_VA_REPLACE, rec, buf );
        p_send2( self, O_WN_DODRAW );
        return WN_KEY_NO_CHANGE;
    }
#endif

    p_supersend5( self, O_DL_KEY, index, keycode, actbut );

    sym->ref = hDlgSenseNcedit( SYMP_REF );
    /*p_scpy( (TEXT*) sym->name, hDlgSenseEdwin( SYMP_NAME ) );*/

    return WN_KEY_CHANGED;
}

METHOD INT sympdlg_dl_set_size( PR_SYMPDLG* self )
{
    APROP_DATA* data;
    EL_SYMBOL* sym;
#if 0   /* RETAIN FOR FUTURE VERSION */
    TEXT buf[RPT_TEXT_MAX_Z];
    INT i, size;
    A_PT* pts;
    SE_CHLIST selist;
#endif

    p_supersend2( self, O_DL_SET_SIZE );
    data = self->dlgbox.rbuf;
    sym = (EL_SYMBOL*) data->elem;

    p_send5( Drg, O_DG_DLG_SET_S_RPT, SYMP_HOTSPOT, &sym->hot, &Drg->vec.Orig );

#if 0   /* RETAIN FOR FUTURE VERSION */
    /* Setup attach point list */
    self->sympdlg.vapts = f_new( CAT_VECBLD_OLIB, C_VASTR );
    p_send3( self->sympdlg.vapts, O_VA_INIT, 64 );
    size = ( sym->grp.size - SIZEOF_SYMBOLHDR ) / sizeof(A_PT);
    pts = sym->attach;
    if( size )
    {
        for( i = 0 ; i < size ; i++ )
        {
            p_send5( Drg, O_DG_RPT_TO_S_TEXT, buf, &pts[i], &Drg->vec.Orig );
            p_send4( self->sympdlg.vapts, O_VA_INSERT, i, buf );
        }
    }
    else
    {
        hLoadResBuf( SR_NONE, buf );
        p_send4( self->sympdlg.vapts, O_VA_INSERT, 0, buf );
        self->sympdlg.NoAttach = TRUE;
    }
    selist.data = (PR_VAROOT*) self->sympdlg.vapts;
    selist.set_flags = SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, SYMP_ATTACH, &selist );
#endif

    return 0;
}

METHOD VOID sympdlg_dl_launch_sub( PR_SYMPDLG* self, INT index )
{
    APROP_DATA* data;
    EL_SYMBOL* sym;
    TEXT buf[40];

    data = self->dlgbox.rbuf;
    sym = (EL_SYMBOL*) data->elem;

    switch( index )
    {
    case SYMP_HOTSPOT:
        BldLaunchDialog( &sym->hot, POINT_PROP_DIALOG, C_PTDLG );
        p_send5( Drg, O_DG_DLG_SET_S_RPT, SYMP_HOTSPOT, &sym->hot, &Drg->vec.Orig );
        break;
    case SYMP_NAME:
        p_bcpy( buf, sym->name, SNAME_MAX );
        buf[SNAME_MAX] = '\0';
        BldLaunchDialog( buf, SYM_NAME_DIALOG, C_SYMNAMDLG );
        p_bcpy( sym->name, buf, SNAME_MAX );
        hDlgSetText( index, buf );
        break;
    }
}

METHOD VOID symnamdlg_dl_dyn_init( PR_SYMNAMDLG* self )
{
    hDlgSetEdwin( 1, (TEXT*) self->dlgbox.rbuf );
}

METHOD INT symnamdlg_dl_key( PR_SYMNAMDLG* self, INT index, INT keycode, INT actbut )
{
    p_scpy( (TEXT*) self->dlgbox.rbuf, hDlgSenseEdwin( 1 ) );
    return WN_KEY_CHANGED;
}

/***************************************************************************
 **  propdlg  Fixed size property dialog class members, Used directly for
 **  ~~~~~~~  Line and 3pt Arc, subclassed for others.
 */

METHOD VOID propdlg_dl_dyn_init( PR_PROPDLG* self )
{
    EL_LINE* line;
    SE_CHLIST selist;
    PR_VAROOT* vaLayer;

    line = self->dlgbox.rbuf;

    SetTitle( line->hdr.type );
    vaLayer = (PR_VAROOT*) p_send2( Drg, O_DG_MAKE_LAYLIST );
    selist.data = vaLayer;
    selist.nsel = line->hdr.layer & ~LAYER_FLAGS;
    selist.set_flags = SE_CHLIST_NSEL | SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, PROP_LAYER, &selist );
}

METHOD INT propdlg_dl_key( PR_PROPDLG* self, INT index, INT keycode,
    INT actbut )
{
    EL_LINE* line;
    UINT layerflags;

    /* We only get here if Enter is pressed */
    line = self->dlgbox.rbuf;

    layerflags = line->hdr.layer & LAYER_FLAGS;
    line->hdr.layer = hDlgSenseChlist( PROP_LAYER ) | layerflags;
    /* Points are automatically updated */

    Drg->drg.PropChanged = TRUE;

    return WN_KEY_CHANGED;
}

METHOD INT propdlg_dl_set_size( PR_PROPDLG* self )
{
    EL_LINE* line;
    A_PT* pts;

    p_supersend2( self, O_DL_SET_SIZE );
    line = self->dlgbox.rbuf;
    pts = &line->beg;

    switch( line->hdr.type )
    {
    case V_3PT_ARC:
        p_send5( Drg, O_DG_DLG_SET_S_RPT, PROP_BEG+2, &pts[2], &Drg->vec.Orig );
        /* Fall through */
    case V_LINE:
        p_send5( Drg, O_DG_DLG_SET_S_RPT, PROP_BEG+1, &pts[1], &Drg->vec.Orig );
        /* Fall through */
    default:
        p_send5( Drg, O_DG_DLG_SET_S_RPT, PROP_BEG, pts, &Drg->vec.Orig );
    }
    return 0;
}

METHOD VOID propdlg_dl_launch_sub( PR_PROPDLG* self, INT index )
{
    EL_LINE* line;
    A_PT* pt;

    line = self->dlgbox.rbuf;
    pt = &line->beg + ( index - PROP_BEG );
    BldLaunchDialog( pt, POINT_PROP_DIALOG, C_PTDLG );
    p_send5( Drg, O_DG_DLG_SET_S_RPT, index, pt, &Drg->vec.Orig );
}

/***************************************************************************
 **  cirpdlg  Circle property dialog class members. Subclasses propdlg
 **  ~~~~~~~
 */

METHOD VOID cirpdlg_dl_dyn_init( PR_CIRPDLG* self )
{
    ELEM* el;

    p_supersend2( self, O_DL_DYN_INIT );

    el = self->dlgbox.rbuf;

    p_send4( Drg, O_DG_DLG_SET_S_AU, CIRP_RADIUS, el->circle.radius );
}

METHOD INT cirpdlg_dl_key( PR_CIRPDLG* self, INT index, INT keycode,
    INT actbut )
{
    ELEM* el;

    /* We only get here if Enter is pressed */
    p_supersend5( self, O_DL_KEY, index, keycode, actbut );

    el = self->dlgbox.rbuf;
    el->circle.radius = p_send3( Drg, O_DG_DLG_GET_S_AU, CIRP_RADIUS );

    return WN_KEY_CHANGED;
}

/***************************************************************************
 **  arcpdlg  Centred Arc property dialog class members. Subclasses propdlg
 **  ~~~~~~~
 */

METHOD VOID arcpdlg_dl_dyn_init( PR_ARCPDLG* self )
{
    ELEM* el;
    DOUBLE angle;

    p_supersend2( self, O_DL_DYN_INIT );

    el = self->dlgbox.rbuf;

    p_send4( Drg, O_DG_DLG_SET_S_AU, ARCP_RADIUS, el->arc.radius );

    p_send4( Drg, O_DG_GET_ANGLE_DBL, &angle, &el->arc.beg );
    hDlgSetFledit( ARCP_BEG, &angle );

    p_send4( Drg, O_DG_GET_ANGLE_DBL, &angle, &el->arc.end );
    hDlgSetFledit( ARCP_END, &angle );
}

METHOD INT arcpdlg_dl_key( PR_ARCPDLG* self, INT index, INT keycode,
    INT actbut )
{
    ELEM* el;
    DOUBLE angle;

    /* We only get here if Enter is pressed */
    p_supersend5( self, O_DL_KEY, index, keycode, actbut );

    el = self->dlgbox.rbuf;
    el->arc.radius = p_send3( Drg, O_DG_DLG_GET_S_AU, ARCP_RADIUS );

    hDlgSenseFledit( ARCP_BEG, &angle );
    p_send4( Drg, O_DG_SET_ANGLE, &el->arc.beg, &angle );

    hDlgSenseFledit( ARCP_END, &angle );
    p_send4( Drg, O_DG_SET_ANGLE, &el->arc.end, &angle );

    return WN_KEY_CHANGED;
}

/***************************************************************************
 **  textpdlg  Text property dialog class members. Subclasses propdlg
 **  ~~~~~~~~
 */

METHOD VOID textpdlg_dl_dyn_init( PR_TEXTPDLG* self )
{
    ELEM* el;
    SE_CHLIST selist;
    TEXT buf[10];
    DOUBLE angle;
    INT flag;

    p_supersend2( self, O_DL_DYN_INIT );
    el = self->dlgbox.rbuf;

    el->buf[el->size] = '\0';
    hDlgSetEdwin( TEXTP_TEXT, (TEXT*) el->text.text );

    selist.data = (PR_VAROOT*) p_send2( Drg, O_DG_MAKE_FONTLIST );
    selist.nsel = el->text.font;
    selist.set_flags = SE_CHLIST_NSEL | SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, TEXTP_FONT, &selist );

    p_send4( Drg, O_DG_GET_TEXTHT_STR, buf, &el->text.set );
    hDlgSetEdwin( TEXTP_SIZE, buf );

    p_send4( Drg, O_DG_GET_ANGLE_DBL, &angle, &el->text.set.a );
    hDlgSetFledit( TEXTP_ANGLE, &angle );

    flag = ( el->text.set.sdiv & FLIP_FLAG ) ? 1 : 0;
    hDlgSetChlist( TEXTP_FLIP, flag );
}

METHOD INT textpdlg_dl_key( PR_TEXTPDLG* self, INT index, INT keycode, INT actbut )
{
    ELEM* el;
    TEXT* str;
    DOUBLE angle;
    UWORD flag;

    /* We only get here if Enter is pressed */
    p_supersend5( self, O_DL_KEY, index, keycode, actbut );
    el = self->dlgbox.rbuf;

    p_scpy( (TEXT*) el->text.text, hDlgSenseEdwin( TEXTP_TEXT ) );
    el->size = p_slen( (TEXT*) el->text.text ) + SIZEOF_TEXTHDR;

    el->text.font = hDlgSenseChlist( TEXTP_FONT );

    str = hDlgSenseEdwin( TEXTP_SIZE );
    p_send4( Drg, O_DG_SET_TEXTHT, str, &el->text.set );

    hDlgSenseFledit( TEXTP_ANGLE, &angle );
    p_send4( Drg, O_DG_SET_ANGLE, &el->text.set.a, &angle );

    el->text.set.sdiv &= ~FLIP_FLAG;
    flag = hDlgSenseChlist( TEXTP_FLIP ) ? FLIP_FLAG : 0;
    el->text.set.sdiv |= flag;

    p_send3( Drg, O_DG_SET_TEXT_RECT, el );

    return WN_KEY_CHANGED;
}

/***************************************************************************
 **  linkpdlg  Link property dialog class members. Subclasses propdlg
 **  ~~~~~~~~
 */

METHOD VOID linkpdlg_dl_dyn_init( PR_LINKPDLG* self )
{
    ELEM* el;
    SE_CHLIST selist;
    DOUBLE dbl;
    INT flag;

    p_supersend2( self, O_DL_DYN_INIT );
    el = self->dlgbox.rbuf;

    selist.data = (PR_VAROOT*) p_send2( Drg, O_DG_MAKE_LIBLIST );
    selist.nsel = el->link.lib;
    selist.set_flags = SE_CHLIST_NSEL | SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, LINKP_LIB, &selist );

    selist.data = (PR_VAROOT*) p_send3( Drg, O_DG_MAKE_SYMLIST, el->link.lib );
    selist.nsel = el->link.sym;
    selist.set_flags = SE_CHLIST_NSEL | SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, LINKP_SYM, &selist );

    p_send4( Drg, O_DG_GET_SCALE_DBL, &dbl, &el->link.set );
    hDlgSetFledit( LINKP_SCALE, &dbl );

    p_send4( Drg, O_DG_GET_ANGLE_DBL, &dbl, &el->link.set.a );
    hDlgSetFledit( LINKP_ANGLE, &dbl );

    flag = ( el->link.set.sdiv & FLIP_FLAG ) ? 1 : 0;
    hDlgSetChlist( LINKP_FLIP, flag );
}

METHOD INT linkpdlg_dl_key( PR_LINKPDLG* self, INT index, INT keycode,
    INT actbut )
{
    ELEM* el;
    DOUBLE dbl;
    UWORD flag;

    /* We only get here if Enter is pressed */
    p_supersend5( self, O_DL_KEY, index, keycode, actbut );
    el = self->dlgbox.rbuf;

    /*el->link.lib = hDlgSenseChlist( LINKP_LIB );*/

    el->link.sym = hDlgSenseChlist( LINKP_SYM );
    el->link.ref = p_send4( Drg, O_DG_GET_SYM_REF, el->link.lib, el->link.sym );

    hDlgSenseFledit( LINKP_SCALE, &dbl );
    p_send4( Drg, O_DG_SET_SCALE, &el->link.set, &dbl );

    hDlgSenseFledit( LINKP_ANGLE, &dbl );
    p_send4( Drg, O_DG_SET_ANGLE, &el->link.set.a, &dbl );

    el->link.set.sdiv &= ~FLIP_FLAG;
    flag = hDlgSenseChlist( LINKP_FLIP ) ? FLIP_FLAG : 0;
    el->link.set.sdiv |= flag;

    return WN_KEY_CHANGED;
}

/***************************************************************************
 **  plinpdlg  Polyline and Polygon property dialog class members
 **  ~~~~~~~~
 */

METHOD VOID plinpdlg_dl_dyn_init( PR_PLINPDLG* self )
{
    EL_PLINE* line;
    SE_CHLIST selist;
    PR_VAROOT* vaLayer;

    line = (EL_PLINE*) self->dlgbox.rbuf;

    SetTitle( line->hdr.type );
    vaLayer = (PR_VAROOT*) p_send2( Drg, O_DG_MAKE_LAYLIST );
    selist.data = vaLayer;
    selist.nsel = line->hdr.layer & ~LAYER_FLAGS;
    selist.set_flags = SE_CHLIST_NSEL | SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, PROP_LAYER, &selist );
}

METHOD INT plinpdlg_dl_key( PR_PLINPDLG* self, INT index, INT keycode,
    INT actbut )
{
    EL_PLINE* line;
    UINT layerflags;
    UINT rec;
    A_PT* pt;
    TEXT buf[RPT_TEXT_MAX_Z];

    line = (EL_PLINE*) self->dlgbox.rbuf;

    if( keycode == W_KEY_MENU || keycode == 'P' )
    {
        rec = hDlgSenseChlist( PLINP_PTS );
        pt = &line->beg + rec;
        BldLaunchDialog( pt, POINT_PROP_DIALOG, C_PTDLG );
        p_send5( Drg, O_DG_RPT_TO_S_TEXT, buf, pt, &Drg->vec.Orig );
        p_send4( self->plinpdlg.vapts, O_VA_REPLACE, rec, buf );
        p_send2( self, O_WN_DODRAW );
        return WN_KEY_NO_CHANGE;
    }

    layerflags = line->hdr.layer & LAYER_FLAGS;
    line->hdr.layer = hDlgSenseChlist( PROP_LAYER ) | layerflags;
    /* Points are automatically updated */

    if( keycode == W_KEY_RETURN )
    {
        Drg->drg.PropChanged = TRUE;
    }

    return WN_KEY_CHANGED;
}

METHOD INT plinpdlg_dl_set_size( PR_PLINPDLG* self )
{
    EL_PLINE* line;
    INT i, size;
    A_PT* pts;
    TEXT buf[RPT_TEXT_MAX_Z];
    SE_CHLIST selist;

    p_supersend2( self, O_DL_SET_SIZE );
    line = (EL_PLINE*) self->dlgbox.rbuf;

    self->plinpdlg.vapts = f_new( CAT_VECBLD_OLIB, C_VASTR );
    p_send3( self->plinpdlg.vapts, O_VA_INIT, 64 );
    size = ( line->hdr.size - sizeof(ELHDR) ) / sizeof(A_PT);
    pts = &line->beg;
    for( i = 0 ; i < size ; i++ )
    {
        p_send5( Drg, O_DG_RPT_TO_S_TEXT, buf, &pts[i], &Drg->vec.Orig );
        p_send4( self->plinpdlg.vapts, O_VA_INSERT, i, buf );
    }
    selist.data = (PR_VAROOT*) self->plinpdlg.vapts;
    selist.set_flags = SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, PLINP_PTS, &selist );
    return 0;
}

/***************************************************************************
 **  ptdlg  Edit A_PT value dialog class members
 **  ~~~~~
 */

METHOD VOID ptdlg_dl_dyn_init( PR_PTDLG* self )
{
    A_PT* pt;
    LAUNIT lau;

    pt = (A_PT*) self->dlgbox.rbuf;
    p_send3( Drg, O_DG_DLG_TITLE_S, SR_EDIT_PT_TITLE_FMT );

    lau = (LAUNIT) pt->x - Drg->vec.Orig.x;
    p_send4( Drg, O_DG_DLG_SET_S_LAU, 1, &lau );

    lau = (LAUNIT) pt->y - Drg->vec.Orig.y;
    lau = Drg->vec.yDir ? -lau : lau;
    p_send4( Drg, O_DG_DLG_SET_S_LAU, 2, &lau );

    /*p_send5( Drg, O_DG_DLG_SET_S_RAU, 1, pt->x, Drg->vec.Orig.x );*/
    /*p_send5( Drg, O_DG_DLG_SET_S_RAU, 2, pt->y, Drg->vec.Orig.y );*/
}

METHOD INT ptdlg_dl_key( PR_PTDLG* self, INT index, INT keycode,
    INT actbut )
{
    A_PT* pt;
    LAUNIT lau;

    /* We only get here if Enter is pressed */
    pt = (A_PT*) self->dlgbox.rbuf;

    p_send4( Drg, O_DG_DLG_GET_S_LAU, 1, &lau );
    pt->x = lau + Drg->vec.Orig.x;

    p_send4( Drg, O_DG_DLG_GET_S_LAU, 2, &lau );
    lau = Drg->vec.yDir ? -lau : lau;
    pt->y = lau + Drg->vec.Orig.y;

    return WN_KEY_CHANGED;
}

/* End of VecProp.c file */
