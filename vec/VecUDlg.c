/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: UNITS AND SCALE DIALOG CLASS     *  Date Started: 27 Oct 1998  *
 *    File: VECPAGE.C       Type: C SOURCE   *  Date Revised: 27 Oct 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1998 Nick Matthews
*/

#include <hwim.h>
#include <ncedit.g>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"


#pragma METHOD_CALL

/*--------------------------[ Set Units Dialog ]--------------------------*/

METHOD VOID* sunitsdlg_dl_item_new( PR_SUNITSDLG* self, AD_DLGBOX* par )
{
    return f_new( CAT_VECTOR_VECTOR, par->class );
}

METHOD VOID sunitsdlg_dl_dyn_init( PR_SUNITSDLG* self )
{
    hDlgSetChlist( SUNITSDLG_PAPER, Drg->vec.Units );
    hDlgSetEdwin( SUNITSDLG_DRAWING, Drg->vec.Pref.unitname );
    hDlgSet( SUNITSDLG_SCALE, &Drg->vec.Pref.scale );
    hDlgSetChlist( SUNITSDLG_DECIMAL, Drg->vec.DispDp );
}

METHOD INT sunitsdlg_dl_key( PR_SUNITSDLG* self, INT index, INT keycode, INT actbut )
{
    TEXT* sz;

    /* We get here if Enter is pressed */
    Drg->vec.Units = hDlgSenseChlist( SUNITSDLG_PAPER );
    sz = hDlgSenseEdwin( SUNITSDLG_DRAWING );
    p_scpy( Drg->vec.Pref.unitname, sz );
    hDlgSense( SUNITSDLG_SCALE, &Drg->vec.Pref.scale );
    Drg->vec.DispDp = hDlgSenseChlist( SUNITSDLG_DECIMAL );

    UpdateDispScale();
    UpdateInfo( IW_ALL );

    return WN_KEY_CHANGED;
}

/*--------------------------[ DScale Edit Lodger ]-------------------------*/

METHOD VOID dscaled_wn_init( PR_DSCALED* self, IN_LNCEDIT* pin_lncedit,
    PR_WIN* landlord )
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
    self->mfne.f[0].width = 4;
    self->mfne.f[0].separator = ':';
    /* Second nearly the same as the first */
    self->mfne.f[1] = self->mfne.f[0];
    self->mfne.f[1].flags = MFNE_LEFT_ALIGN | MFNE_SUPPRESS_SEPARATOR;

    p_supersend2( self, O_WN_SET );
}

METHOD VOID dscaled_wn_set( PR_DSCALED* self, DSCALE* pscale )
{
    self->mfne.f[0].value = pscale->mul;
    self->mfne.f[1].value = pscale->div;
    p_supersend2( self, O_WN_SET );
}

METHOD VOID dscaled_wn_sense( PR_DSCALED* self, DSCALE* pscale )
{
    pscale->mul = (UWORD) self->mfne.f[0].value;
    pscale->div = (UWORD) self->mfne.f[1].value;
}

/*---------------------------[ Show Page Dialog ]--------------------------*/

METHOD VOID showpgdlg_dl_dyn_init( PR_SHOWPGDLG* self )
{
    LAUNIT lau;
    TEXT buf[DIM_TEXT_MAX_Z];

    SetTitleScaleUnits( SR_SHOW_PAGE_FMT );

    lau = (LAUNIT) Drg->vec.Page.lim.x - Drg->vec.Page.pos.x;
    LaunitToScaleText( buf, lau );
    hDlgSetText( SHOWPGDLG_WIDTH, buf );

    lau = (LAUNIT) Drg->vec.Page.lim.y - Drg->vec.Page.pos.y;
    LaunitToScaleText( buf, lau );
    hDlgSetText( SHOWPGDLG_HEIGHT, buf );

    lau = (LAUNIT) Drg->vec.Marg.pos.y - Drg->vec.Orig.y;
    lau = Drg->vec.yDir ? -lau : lau;
    LaunitToScaleText( buf, lau );
    hDlgSetText( SHOWPGDLG_TOP, buf );

    lau = (LAUNIT) Drg->vec.Marg.pos.x - Drg->vec.Orig.x;
    LaunitToScaleText( buf, lau );
    hDlgSetText( SHOWPGDLG_LEFT, buf );

    lau = (LAUNIT) Drg->vec.Marg.lim.y - Drg->vec.Orig.y;
    lau = Drg->vec.yDir ? -lau : lau;
    LaunitToScaleText( buf, lau );
    hDlgSetText( SHOWPGDLG_BOTTOM, buf );

    lau = (LAUNIT) Drg->vec.Marg.lim.x - Drg->vec.Orig.x;
    LaunitToScaleText( buf, lau );
    hDlgSetText( SHOWPGDLG_RIGHT, buf );
}

/* End of VecPage.c file */
