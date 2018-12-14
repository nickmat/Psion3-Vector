/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: ABOUT AND REGISTER DIALOGS       *  Date Started: 16 Jul 1997  *
 *    File: VECABOUT.C      Type: C SOURCE   *  Date Revised: 16 Jul 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <hwim.h>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"
#include "regread.h"

TEXT* RegFileName = "LOC::M:\\OPD\\VECTOR.REG";

/***************************************************************************
 **  GetRegName  Read the name from the registration file, if it exists.
 **  ~~~~~~~~~~
 */

static INT GetRegName( TEXT* name )
{
    TEXT code[RC_CODE_MAX_Z];
    TEXT prog[RC_PROG_MAX_Z];
    VOID* pcb;
    INT len;

    if( p_open( &pcb, RegFileName, P_FTEXT | P_FOPEN | P_FSHARE ) )
    {
        return FALSE;
    }
    len = p_read( pcb, code, RC_CODE_MAX );
    p_close( pcb );
    if( len <= 0 )
    {
        return FALSE;
    }
    code[len] = '\0';
    if( rcReadCode( name, prog, code ) == 0 )
    {
        return FALSE;
    }
    if( p_scmp( prog, "vp" ) != 0 )
    {
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************
 **  CheckReg  Check for registration file and if found, decode. Set Drg
 **  ~~~~~~~~  RegFlag and RegName. Return TRUE if registered, else FALSE.
 */

INT CheckReg( VOID )
{
    TEXT name[RC_NAME_MAX_Z];
    INT ret;

    ret = GetRegName( name );
    Drg->drg.RegFlag = ret;
    if( ret == FALSE )
    {
        hLoadResBuf( SR_UNREGISTERED, Drg->drg.RegName );
    }
    else
    {
        p_scpy( Drg->drg.RegName, name );
    }
    return ret;
}

/*-----------------------------[ About Dialog ]---------------------------*/

#pragma METHOD_CALL

/***************************************************************************
 **  dl_dyn_init  Initiate 1st About dialog, set up help key
 **  ~~~~~~~~~~~
 */

METHOD VOID aboutdlg_dl_dyn_init( PR_ABOUTDLG* self )
{
    TEXT buf[40];

    self->dlgbox.helprid = HELP_REGISTER;
    if( Drg->drg.RegFlag )
    {
        hAtos( buf, SR_REGISTERED_TO_FMT, Drg->drg.RegName );
        hDlgSetText( 4, buf );
    }
}

/***************************************************************************
 **  dl_key  Handle key entry for About dialog.
 **  ~~~~~~
 */

METHOD INT aboutdlg_dl_key( PR_ABOUTDLG* self, INT index, INT keycode,
    INT actbut )
{
    /* We only get here if Enter, Esc or Space (to Register) is pressed */
    if( keycode == W_KEY_ESCAPE && Drg->drg.RegFlag )
    {
        return WN_KEY_CHANGED;
    }
    if( keycode == ' ' || keycode == W_KEY_ESCAPE )
    {
        if( ! LaunchDialog( NULL, REGISTER_DIALOG, C_REGDLG ) )
        {
            return WN_KEY_NO_CHANGE;
        }
    }
    if( ! Drg->drg.RegFlag )
    {
        LaunchDialog( NULL, ABOUT2_DIALOG, C_ABOUT2DLG );
    }
    return WN_KEY_CHANGED;
}

/*-----------------------------[ About2 Dialog ]--------------------------*/

/***************************************************************************
 **  dl_dyn_init  Initiate 2nd About dialog, set up help key
 **  ~~~~~~~~~~~
 */

METHOD VOID about2dlg_dl_dyn_init( PR_ABOUT2DLG* self )
{
    self->dlgbox.helprid = HELP_REGISTER;
}

/***************************************************************************
 **  dl_key  Handle key entry for 2nd About dialog. Stops enter being used
 **  ~~~~~~  to exit.
 */

METHOD INT about2dlg_dl_key( PR_ABOUT2DLG* self, INT index, INT keycode,
    INT actbut )
{
    /* We only get here if Enter is pressed */
    return WN_KEY_NO_CHANGE;
}

/*----------------------------[ Register Dialog ]-------------------------*/

/***************************************************************************
 **  dl_key  Handle program registration dialog.
 **  ~~~~~~
 */

METHOD INT regdlg_dl_key( PR_REGDLG* self, INT index, INT keycode,
    INT actbut )
{
    TEXT code[RC_CODE_MAX_Z];
    TEXT name[RC_NAME_MAX_Z];
    TEXT prog[RC_PROG_MAX_Z];
    VOID* pcb;

    /* We only get here if Enter is pressed */
    p_scpy( code, hDlgSenseEdwin( 1 ) );
    if( rcReadCode( name, prog, code ) == 0 ||
        prog[0] != 'v' || prog[1] != 'p' )
    {
        hInfoPrint( SR_INVALID_REG_CODE );
        return WN_KEY_NO_CHANGE;
    }

    hEnsurePath( RegFileName );
    if( p_open( &pcb, RegFileName, P_FTEXT | P_FREPLACE | P_FUPDATE ) )
    {
        return WN_KEY_CHANGED;
    }
    p_write( pcb, code, p_slen( code ) );
    p_close( pcb );

    CheckReg();
    UpdateInfo( IW_ALL );

    return WN_KEY_CHANGED;
}

/* End of VecAbout.c */
