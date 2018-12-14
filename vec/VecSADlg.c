/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: SAVE AS DIALOG CLASS MEMBERS     *  Date Started: 17 Sep 1996  *
 *    File: VECSADLG.C      Type: C SOURCE   *  Date Revised: 17 Sep 1996  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1996, 1997, Nick Matthews
*/

#include <hwim.h>
#include <p_math.h>
#include <ncedit.g>
#include <edwin.g>
#include <chlist.g>
#include <files.g>
#include <vector.g>
#include "vecbld.h"
#include <vector.rsg>
#include "vector.h"


VOID SetExtension( TEXT* buf, INT ftype, INT export )
{
    TEXT* ext;

    if( export )
    {
        ftype += 32;
    }
    switch( ftype )
    {
    case FTYPE_TEMPLATE:
    case SAVE_FTYPE_VTP + 32:
        ext = ".vtp";
        break;
    case FTYPE_LIBRARY:
    case SAVE_FTYPE_VSL + 32:
        ext = ".vsl";
        break;
    case FTYPE_FONT:
    case SAVE_FTYPE_VFT + 32:
        ext = ".vft";
        break;
    case SAVE_FTYPE_PIC + 32:
        ext = ".pic";
        break;
    case SAVE_FTYPE_PCX + 32:
        ext = ".pcx";
        break;
    case SAVE_FTYPE_DXF + 32:
        ext = ".dxf";
        break;
    case SAVE_FTYPE_QCD + 32:
        ext = ".qcd";
        break;
    case SAVE_FTYPE_VEC + 32:
    default:
        ext = ".vec";
        break;
    }
    p_scpy( buf, ext );
}

static INT GetHeight( INT width )
{
    AUNIT tmp;
    INT upp;

    tmp = Drg->vec.Page.lim.x - Drg->vec.Page.pos.x - 1;
    upp = tmp / width + 1;
    tmp = Drg->vec.Page.lim.y - Drg->vec.Page.pos.y - 1;
    return tmp / upp + 1;
}

static VOID UpdateExport( PR_SAVEDLG* self, INT ftype )
{
    SAVEDLG_DATA* data;
    INT height;
    TEXT buf[40];

    data = self->dlgbox.rbuf;

    switch( ftype )
    {
    case SAVE_FTYPE_PIC:
    case SAVE_FTYPE_PCX:
        height = GetHeight( data->width );
        hAtos( buf, SR_BM_EXPORT_ITEM_FMT, data->width, height );
        break;
    /*case SAVE_FTYPE_DXF:*/
    case SAVE_FTYPE_QCD:
        hAtos( buf, SR_QCD_EXPORT_ITEM_FMT, data->qcdres );
        break;
    default:
        return;
    }
    hDlgSetText( SAVEDLG_EXPORT, buf );
}

#pragma save, METHOD_CALL

/*----------------------------[ Save as Dialog ]--------------------------*/

METHOD VOID savedlg_dl_dyn_init( PR_SAVEDLG* self )
{
    SAVEDLG_DATA* data;

    data = self->dlgbox.rbuf;
    data->width = 512;
    data->qcdres = 800;

    hDlgSetChlist( SAVEDLG_TYPE, Drg->drg.FileType );
}

METHOD INT savedlg_dl_key( PR_SAVEDLG* self, INT index, INT keycode, INT actbut )
{
    SAVEDLG_DATA* data;
    TEXT* fn;
    INT ftype;

    data = self->dlgbox.rbuf;
    fn = data->fname;

    hDlgSense( SAVEDLG_NAME, fn );
    data->usenew = hDlgSenseChlist( SAVEDLG_USENEW );
    ftype = hDlgSenseChlist( SAVEDLG_TYPE );

    if( ftype <= SAVE_FTYPE_INTERNAL )
    {
        data->export = 0;
        if( data->usenew )
        {
            p_send3( Drg->drg.DrgFile, O_DF_SET_FNAME, fn );
            p_send3( w_am, O_AM_NEW_FILENAME, fn );
        }
    }
    else
    {
        data->export = ftype;
    }

    return WN_KEY_CHANGED;
}

METHOD VOID savedlg_dl_changed( PR_SAVEDLG* self, INT index )
{
    PR_FNEDIT* fned;
    INT ftype;

    if( index != SAVEDLG_TYPE ) return;

    ftype = hDlgSenseChlist( SAVEDLG_TYPE );
    fned = (PR_FNEDIT*) p_send3( self, O_DL_INDEX_TO_HANDLE, SAVEDLG_NAME );
    SetExtension( fned->fnedit.extbuf, ftype, FTYPE_EXPORT );
    fned->fnedit.defext = fned->fnedit.extbuf;

    if( ftype <= SAVE_FTYPE_INTERNAL )
    {
        p_send4( self, O_DL_ITEM_DIM, SAVEDLG_USENEW, FALSE );
        p_send4( self, O_DL_ITEM_DIM, SAVEDLG_EXPORT, TRUE );
    }
    else if( ftype == SAVE_FTYPE_DXF )
    {
        p_send4( self, O_DL_ITEM_DIM, SAVEDLG_USENEW, TRUE );
        p_send4( self, O_DL_ITEM_DIM, SAVEDLG_EXPORT, TRUE );
    }
    else /* File export */
    {
        UpdateExport( self, ftype );
        p_send4( self, O_DL_ITEM_DIM, SAVEDLG_USENEW, TRUE );
        p_send4( self, O_DL_ITEM_DIM, SAVEDLG_EXPORT, FALSE );
    }
}

METHOD VOID savedlg_dl_launch_sub( PR_SAVEDLG* self, INT index )
{
    INT ftype;
    UINT dlg_resource;
    UINT dlg_class;

    ftype = hDlgSenseChlist( SAVEDLG_TYPE );
    switch( ftype )
    {
    case SAVE_FTYPE_PIC:
    case SAVE_FTYPE_PCX:
        dlg_resource = SAVE_BITMAP_DIALOG;
        dlg_class = C_BMEXPDLG;
        break;
    /*case SAVE_FTYPE_DXF:*/
        /*dlg_resource = SAVE_QCD_DIALOG;*/
        /*dlg_class = C_QCDEXPDLG;*/
        /*break;*/
    case SAVE_FTYPE_QCD:
        dlg_resource = SAVE_QCD_DIALOG;
        dlg_class = C_QCDEXPDLG;
        break;
    default:
        return;
    }
    if( LaunchDialog( self->dlgbox.rbuf, dlg_resource, dlg_class ) )
    {
        UpdateExport( self, ftype );
    }
}

/*------------------------[ Save as bitmap Dialog ]-----------------------*/

METHOD VOID bmexpdlg_dl_dyn_init( PR_BMEXPDLG* self )
{
    SAVEDLG_DATA* data;

    data = self->dlgbox.rbuf;
    hDlgSetNcedit( 1, data->width );
}

METHOD INT bmexpdlg_dl_key( PR_BMEXPDLG* self, INT index, INT keycode, INT actbut )
{
    SAVEDLG_DATA* data;

    data = self->dlgbox.rbuf;
    data->width = hDlgSenseNcedit( 1 );
    return WN_KEY_CHANGED;
}

/*--------------------------[ Save as qcd Dialog ]------------------------*/

METHOD VOID qcdexpdlg_dl_dyn_init( PR_QCDEXPDLG* self )
{
    SAVEDLG_DATA* data;

    data = self->dlgbox.rbuf;
    hDlgSetNcedit( 1, data->qcdres );
}

METHOD INT qcdexpdlg_dl_key( PR_QCDEXPDLG* self, INT index, INT keycode, INT actbut )
{
    SAVEDLG_DATA* data;

    data = self->dlgbox.rbuf;
    data->qcdres = hDlgSenseNcedit( 1 );
    return WN_KEY_CHANGED;
}

/* End of VecSADlg.c file */
