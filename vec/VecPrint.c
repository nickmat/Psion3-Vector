/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: PRINTER FUNCTIONS & DIALOGS      *  Date Started: 20 Dec 1997  *
 *    File: VECPRINT.C      Type: C SOURCE   *  Date Revised: 13 Mar 1999  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997 - 1999 Nick Matthews
*/

#include <hwim.h>
#include <chlist.g>
#include <sa_.rsg>
#include <serdlgs.g>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"

static LONG BaudTable[] =
{
    0, 50, 75, 110, 134, 150, 300, 600, 1200, 1800, 2000, 2400, 3600, 4800, 7200,
    9600, 19200, 38400L, 57600L, 115200L
};

/* Order must be same as PRINT_MODEL_LIST */
static INT PlotClass[] =
{
    C_PCL5PLT, C_PLOT/*, C_CUSPCL5PLT, C_CUSHPGLPLT*/
};

/***************************************************************************
 **  DoPrint  Print the current drawing
 **  ~~~~~~~
 */

VOID DoPrint( VOID )
{
    PR_PLOT* pPlot;
    INT class;

    class = PlotClass[ Drg->drg.Cfg.prn.name ];
    pPlot = p_new( CAT_VECTOR_VECTOR, class );
    if( pPlot )
    {
        Drg->drg.Plot = pPlot;
        if( p_send2( pPlot, O_PLOT_INIT ) )
        {
            if( p_send2( pPlot, O_PLOT_OPEN ) == 0 )
            {
                if( hConfirm( SR_PRINT_DRAWING ) )
                {
                    Drg->drg.Primative = PRIM_PLOT;
                    p_send2( pPlot, O_PLOT_DRAW );
                    Drg->drg.Primative = PRIM_DRAW;
                }
                p_send2( pPlot, O_PLOT_CLOSE );
            }
        }
        hDestroy( pPlot );
        Drg->drg.Plot = NULL;
    }
}

/*------------------------------------------------------------------------*/

INT SysLaunchDialog( VOID* buf, INT id, INT class )
{
    DL_DATA dial_data;

    dial_data.id = id;
    dial_data.rbuf = buf;
    dial_data.pdlg = NULL;
    return hLaunchDial( CAT_VECTOR_HWIM, class, &dial_data );
}


static VOID UpdateSerial( PR_SPRNDLG* self )
{
    PRN* pPrn;
    TEXT buf[40];
    INT id;
    TEXT onoff[10];

    pPrn = self->dlgbox.rbuf;

    hAtos( buf, SR_SERIAL_CHAR_FMT, BaudTable[ pPrn->serport.tbaud ] );
    hDlgSetText( SPRNDLG_SER_CHAR, buf );

    id = ( pPrn->serport.hand & P_OBEY_XOFF ) ? SRU_ON : SRU_OFF;
    hLoadResBuf( id, onoff );
    hAtos( buf, SR_SERIAL_HAND_FMT, onoff );
    hDlgSetText( SPRNDLG_SER_HAND, buf );
}

static INT IrpExist( VOID )
{
    VOID* pcb;

    if( p_open( &pcb, "IRP:A", -1 ) != 0 )
    {
        return FALSE;
    }
    p_close( pcb );
    return TRUE;
}

#pragma METHOD_CALL

/***************************************************************************
 **  sprndlg  Set printer dialog class members
 **  ~~~~~~~
 */

METHOD VOID sprndlg_dl_changed( PR_SPRNDLG* self, INT index )
{
    /* Device has changed */
    hDlgItemDim( SPRNDLG_SER_CHAR, TRUE );
    hDlgItemDim( SPRNDLG_SER_HAND, TRUE );
    hDlgItemDim( SPRNDLG_FILE, TRUE );
    switch( hDlgSenseChlist( SPRNDLG_DEVICE ) )
    {
    case PRNDEV_SERIAL:
        hDlgItemDim( SPRNDLG_SER_CHAR, FALSE );
        hDlgItemDim( SPRNDLG_SER_HAND, FALSE );
        break;
    case PRNDEV_FILE:
        hDlgItemDim( SPRNDLG_FILE, FALSE );
        break;
    }
}

METHOD VOID sprndlg_dl_dyn_init( PR_SPRNDLG* self )
{
    PRN* pPrn;
    SE_CHLIST selist;

    pPrn = self->dlgbox.rbuf;

    if( ! IrpExist() )
    {
        /* No IR available */
        hDlgSense( SPRNDLG_DEVICE, &selist );
        p_send3( selist.data, O_VA_DELETE, PRNDEV_IRLINK );
        if( pPrn->device == PRNDEV_IRLINK )
        {
            pPrn->device = PRNDEV_PARA;
        }
    }
    hDlgSetChlist( SPRNDLG_MODEL, pPrn->name );
    hDlgSetChlist( SPRNDLG_DEVICE, pPrn->device );
    UpdateSerial( self );
    hDlgSet( SPRNDLG_FILE, pPrn->fname );

    /* Dim the unused items */
    sprndlg_dl_changed( self, SPRNDLG_DEVICE );
}

METHOD INT sprndlg_dl_key( PR_SPRNDLG* self, INT index, INT keycode, INT actbut )
{
    PRN* pPrn;

    pPrn = self->dlgbox.rbuf;

    pPrn->name = hDlgSenseChlist( SPRNDLG_MODEL );
    pPrn->device = hDlgSenseChlist( SPRNDLG_DEVICE );
    hDlgSense( SPRNDLG_FILE, pPrn->fname );

    Drg->drg.Cfg.prn = *pPrn;
    if( hDlgSenseChlist( SPRNDLG_SAVE ) )
    {
        p_send3( Drg, O_DG_UPDATE_CFG, CFG_PRN );
    }

    return WN_KEY_CHANGED;
}

METHOD VOID sprndlg_dl_launch_sub( PR_SPRNDLG* self, INT index )
{
    PRN* pPrn;

    pPrn = self->dlgbox.rbuf;

    switch( index )
    {
    case SPRNDLG_SER_CHAR:
        SysLaunchDialog( &pPrn->serport, -SYS_SET_PORT_DIALOG, C_SETPORTDLG );
        break;
    case SPRNDLG_SER_HAND:
        SysLaunchDialog( &pPrn->serport.hand, -SYS_SET_HSHK_DIALOG, C_SETHSHKDLG );
        break;
    }
    UpdateSerial( self );
}

/* End of VecPrint.c file */
