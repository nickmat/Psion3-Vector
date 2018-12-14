/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: LAYER DIALOG CLASS MEMBERS       *  Date Started: 29 May 1997  *
 *    File: VECLDLG.C       Type: C SOURCE   *  Date Revised: 30 May 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <hwim.h>
#include <edwin.g>
#include <chlist.g>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"


static VOID MakeLayerDesc( PR_LAYERDLG* self, TEXT* buf, INT layer )
{
    INT id;
    UINT mask;
    TEXT* str;
    TEXT onoff[10];

    str = self->layerdlg.name[layer];
    mask = 1 << layer;
    id = ( mask & self->layerdlg.mask ) ? SRU_ON : SRU_OFF;
    hLoadResBuf( id, onoff );
    hAtos( buf, SR_LAYER_LIST_FMT, 'A' + layer, str, onoff );
}

static VOID UpdateLayerDesc( PR_LAYERDLG* self, INT layer )
{
    TEXT buf[40];

    MakeLayerDesc( self, buf, layer );
    p_send4( self->layerdlg.list, O_VA_REPLACE, layer, buf );
    p_send2( self, O_WN_DODRAW );
}

#pragma METHOD_CALL

METHOD VOID layerdlg_dl_dyn_init( PR_LAYERDLG* self )
{
    INT i;
    TEXT buf[40];

    self->layerdlg.current = Drg->vec.Layer;
    self->layerdlg.mask = Drg->vec.LayMask;

    self->layerdlg.list = f_new( CAT_VECTOR_OLIB, C_VASTR );
    p_send3( self->layerdlg.list, O_VA_INIT, UNAME_MAX_Z * 4 );
    for( i = 0 ; i < 16 ; i++ )
    {
        p_send4( Drg, O_DG_GET_LAYER_STR, self->layerdlg.name[i], i );
        MakeLayerDesc( self, buf, i );
        p_send4( self->layerdlg.list, O_VA_INSERT, i, buf );
    }
    /* List is set up by dl_set_size */
}

METHOD INT layerdlg_dl_set_size( PR_LAYERDLG* self )
{
    SE_CHLIST selist;

    p_supersend2( self, O_DL_SET_SIZE );

    /* No need to destroy the existing dummy one first! */
    selist.data = (PR_VAROOT*) self->layerdlg.list;
    selist.nsel = self->layerdlg.current;
    selist.set_flags = SE_CHLIST_NSEL | SE_CHLIST_DATA;
    p_send4( self, O_WN_SET, 1, &selist );

    return 0;
}

METHOD INT layerdlg_dl_key( PR_LAYERDLG* self, INT index, INT keycode,
    INT actbut )
{
    LAYNAMDLG_DATA data;
    INT layer;
    UWORD size, i;
    TEXT* newlist;

    /* We get here if Enter, Menu(toggle) or Space(rename) is pressed */
    layer = hDlgSenseChlist( 1 );
    switch( keycode )
    {
    case W_KEY_RETURN:
        break;
    case W_KEY_MENU:
        self->layerdlg.mask ^= ( 1 << layer );
        UpdateLayerDesc( self, layer );
        return WN_KEY_NO_CHANGE;
    case W_KEY_SPACE:
        data.layer = layer;
        data.name = self->layerdlg.name[layer];
        if( LaunchDialog( &data, LAYER_NAME_DIALOG, C_LAYNAMDLG ) )
        {
            self->layerdlg.changed = TRUE;
            UpdateLayerDesc( self, layer );
        }
        /* Fall through */
    default:
        return WN_KEY_NO_CHANGE;
    }

    /* Enter pressed */
    if( self->layerdlg.changed )
    {
        for( size = i = 0 ; i < 16 ; i++ )
        {
            size += p_slen( self->layerdlg.name[i] ) + 1;
        }
        newlist = p_realloc( Drg->drg.LayBuf, size );
        if( newlist )
        {
            Drg->drg.LayBufSize = size;
            Drg->drg.LayBuf = newlist;
            for( size = i = 0 ; i < 16 ; i++ )
            {
                Drg->drg.LayStr[i] = &newlist[size];
                p_scpy( &newlist[size], self->layerdlg.name[i] );
                size += p_slen( &newlist[size] ) + 1;
            }
        }
    }
    Drg->vec.Layer = layer;
    Drg->vec.LayMask = self->layerdlg.mask;
    Drg->vec.LayMask |= ( 1 << layer ); /* Selected layer must be ON */
    Drg->drg.ChangedFlag = TRUE;
    p_send2( Drg, O_DG_REDRAW );
    UpdateInfo( IW_LAYER );

    return WN_KEY_CHANGED;
}

METHOD VOID laynamdlg_dl_dyn_init( PR_LAYNAMDLG* self )
{
    LAYNAMDLG_DATA* data;
    TEXT buf[40];

    data = self->dlgbox.rbuf;

    hAtos( buf, SR_LAYER_NAME_TITLE_FMT, 'A' + data->layer );
    hDlgSetText( 0, buf );

    hDlgSetEdwin( 1, data->name );
}

METHOD INT laynamdlg_dl_key( PR_LAYNAMDLG* self, INT index, INT keycode,
    INT actbut )
{
    LAYNAMDLG_DATA* data;

    /* We only get here if Enter is pressed */
    data = self->dlgbox.rbuf;
    p_scpy( data->name, hDlgSenseEdwin( 1 ) );

    return WN_KEY_CHANGED;
}

/* End of VECLDLG.C file */
