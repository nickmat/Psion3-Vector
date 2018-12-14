/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR PSION EPOC16          *  Written by: Nick Matthews  *
 *  Module: DXF DYL GENERAL FUNCTIONS        *  Date Started: 13 Oct 1998  *
 *    File: VecDxfOb.C      Type: C SOURCE   *  Date Revised: 13 Oct 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1998 Nick Matthews
*/

#include <hwim.h>
#include <vecdxf.g>
#include <vector.g>
#include <vector.rsg>
#include "vec.h"

#pragma METHOD_CALL

/***************************************************************************
 **  destroy  Clear up all class items
 **  ~~~~~~~  (a cleanup class could do this better!)
 */

METHOD VOID xchange_destroy( PR_XCHANGE* self )
{
    if( self->xchange.pcb )
    {
        p_close( self->xchange.pcb );
    }
    if( self->xchange.laytab )
    {
        p_free( self->xchange.laytab );
    }
    p_supersend2( self, O_DESTROY );
}

/***************************************************************************
 **  xch_version  Return the compiled version number. NOTE dyl and app must
 **  ~~~~~~~~~~~  be compiled with the same headers.
 */

METHOD UINT xchange_xch_version( PR_XCHANGE* self )
{
    return VERSION_XCH;
}

/*------------------------------------------------------------------------*/
/***************************************************************************
 **  dxfescdlg  Cancel export/import dialog
 **  ~~~~~~~~~
 */

METHOD VOID dxfescdlg_dl_set_size( PR_DXFESCDLG* self )
{
    p_supersend2( self, O_DL_SET_SIZE );
    p_send4( self, O_DXFDL_UPDATE, SR_BLANK, 0 );
}

METHOD INT dxfescdlg_dl_key( PR_DXFESCDLG* self, INT index, INT keycode, INT actbut )
{
    INT* esc;

    esc = (INT*) self->dlgbox.rbuf;

    *esc = TRUE;
    return WN_KEY_NO_CHANGE;
}

METHOD VOID dxfescdlg_dxfdl_update( PR_DXFESCDLG* self, UINT id, INT num )
{
    TEXT buf[40];

    hAtos( buf, id, num );
    hDlgSetText( DXFESCDLG_PROGRESS, buf );
}

/* End of VecDxfOb.c file */
