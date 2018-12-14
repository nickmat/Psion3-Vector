/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: MAIN AND WINDOW SERVER MEMBERS   *  Date Started: 27 Aug 1996  *
 *    File: VECMAIN.C       Type: C SOURCE   *  Date Revised:  9 Jan 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1996-1998, Nick Matthews
*/

#include <hwim.h>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"
#include "vecbld.h"

UBYTE*    FBuf;
PR_UNDO*  Redo;

HANDLE hBlackBM;
HANDLE hGreyBM;
INT    idBlackBM;
INT    idGreyBM;
HANDLE BuildDyl;
#ifdef PSION_3A
    TEXT*  CfgFileName = "LOC::M:\\OPD\\VECTOR.CFG";
#else  /* PSION_SI */
    TEXT*  CfgFileName = "LOC::M:\\OPD\\VECTORS.CFG";
#endif /* PSION_?? */
TEXT*  ErrorStrings[MAX_ERROR_STRINGS];

/***************************************************************************
 **  main  See SDK Vol 3 "Object Oriented Programming Guide" V2.10 Page 1-15
 **  ~~~~
 */

GLDEF_C VOID main( VOID )
{
    IN_HWIMMAN app;
    IN_WSERV ws;
    VOID* handle;
    VOID* dcb;
    INT err;

    p_linklib( 0 );

    err = p_openlib( &dcb, DatCommandPtr );
    if( err )
    {
        return;
    }
    err = p_loadfilelib( dcb, VECBLD_DYL, &BuildDyl, TRUE );
    if( err )
    {
        p_close( dcb );
        return;
    }

    app.flags = FLG_APPMAN_FULLSCREEN | FLG_APPMAN_RSCFILE |
        FLG_APPMAN_CLEAN | FLG_APPMAN_SRSCFILE;
    app.wserv_cat = p_getlibh( CAT_VECTOR_VECTOR );
    app.wserv_class = C_VECWS;

    ws.com_cat = p_getlibh( CAT_VECTOR_VECTOR );
    ws.com_class = C_VECCM;

    handle = p_new( CAT_VECTOR_HWIM, C_HWIMMAN );
    p_send4( handle, O_AM_INIT, &app, &ws );
}

/***************************************************************************
 **  ReadErrorStrings  Read in the error strings now, so that they can be
 **  ~~~~~~~~~~~~~~~~  accessed without causing further errors.
 */

VOID ReadErrorStrings( VOID )
{
    INT i;

    for( i = 0 ; i < MAX_ERROR_STRINGS ; i++ )
    {
        hLoadResource( ERROR_STRINGS + i, &ErrorStrings[i] );
    }
}

/*------------------------[ vecws - Window Server ]-----------------------*/

/***************************************************************************
 **  ws_dyn_init  Initialise the window server and start of app.
 **  ~~~~~~~~~~~
 */

#pragma METHOD_CALL

METHOD VOID vecws_ws_dyn_init( PR_VECWS* self )
{
    self->wserv.cli = f_new( CAT_VECTOR_VECTOR, C_VECDW );
    p_send2( self->wserv.cli, O_WN_INIT );
    self->wserv.help_index_id = VECTOR_HELP;

    ReadErrorStrings();

    Data = f_new( CAT_VECTOR_VECTOR, C_DDATA );
    p_send3( Data, O_DD_INIT, 256 );

    DeleteTempFiles();

    Undo = f_new( CAT_VECTOR_VECTOR, C_UNDO );
    Redo = f_new( CAT_VECTOR_VECTOR, C_UNDO );
    p_send4( Undo, O_UD_INIT, Redo, SR_UNDO );
    p_send4( Redo, O_UD_INIT, Undo, SR_REDO );

    DBuf = f_alloc( EL_BUFSIZE );
    FBuf = f_alloc( EL_BUFSIZE );

    Extra = f_newlibh( BuildDyl, C_EXTRA );

    Drg = f_new( CAT_VECTOR_VECTOR, C_DRG );
    p_send2( Drg, O_DG_INIT );

    if( CheckReg() )
    {
        wInfoMsgCorner( "Vector " VERSION " \270 1997,98 Nick Matthews",
            W_CORNER_TOP_LEFT );
    }
    else
    {
        p_send2( self->wserv.com, O_COM_ABOUT );
    }
    p_send4( self->wserv.com, O_COM_FILE_CHANGE, w_am->hwimman.command,
        DatUsedPathNamePtr );
}

/* End of VECMAIN.C file */
