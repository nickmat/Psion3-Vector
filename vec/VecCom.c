/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: VECCM COMMAND CLASS MEMBERS      *  Date Started:  1 Sep 1996  *
 *    File: VECCOM.C        Type: C SOURCE   *  Date Revised: 30 May 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1996, 1997, Nick Matthews
*/

#include <hwim.h>
#include <edwin.g>
#include <sa_.rsg>
#include <vector.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"
#include "vecxch.h"


/***************************************************************************
 **  LaunchDialog  Launch a main module dialog box and return the result
 **  ~~~~~~~~~~~~
 */

INT LaunchDialog( VOID* buf, INT id, INT class )
{
    DL_DATA dial_data;

    dial_data.id = id;
    dial_data.rbuf = buf;
    dial_data.pdlg = NULL;
    return hLaunchDial( CAT_VECTOR_VECTOR, class, &dial_data );
}

/***************************************************************************
 **  CreateBuild  Create and initialise a new build class, destroying any
 **  ~~~~~~~~~~~  previous one.
 */

static VOID CreateBuild( INT buildclass )
{
    hDestroy( Build );
    Build = p_newlibh( BuildDyl, buildclass );
    if( Build == NULL )
    {
        hInfoPrint( SRM_NO_MEMORY );
        return;
    }
    p_send2( Build, O_BLD_INIT );
    UpdateInfo( IW_MODE );
}

/***************************************************************************
 **  GetPath  Put the current path (without filename) into buffer 'path'.
 **  ~~~~~~~  Return pointer to the permanent copy of the current filename.
 */

static TEXT* GetPath( TEXT* path )
{
    TEXT* pfn;
    INT len;
    P_FPARSE crk;

    pfn = (TEXT*) p_send2( Drg->drg.DrgFile, O_DF_GET_FNAME );
    p_scpy( path, pfn );
    f_fparse( path, NULL, path, &crk );
    len = crk.system + crk.device + crk.path;
    path[len] = '\0';
    return pfn;
}


/***************************************************************************
 **  SaveChanges  Protected function to save current drawing to file
 **  ~~~~~~~~~~~
 */

static INT SaveChanges( VOID )
{
    INT err, ret, flag;

    flag = TRUE;
    if( Drg->drg.ChangedFlag )
    {
        hBusyPrint( 0, SR_SAVING_FILE, p_send2( Drg->drg.DrgFile, O_DF_GET_NAME ) );
        for(;;)
        {
            err = p_entersend2( Drg->drg.DrgFile, O_DF_SAVE_FILE );
            if( err == 0 )
            {
                break;
            }

            p_send2( Drg->drg.DrgFile, O_DF_KILL );
            wClientPosition( 0, 0 );
            ret = p_send4( Drg, O_DG_CANCEL_RETRY, ES_CANT_SAVE_FILE, err );
            if( ret == ALERT_CANCEL )
            {
                flag = FALSE;
                break;
            }
        }
        wCancelBusyMsg();
    }
    return flag;
}

/***************************************************************************
 **  ReadFile  Replace current file with the given filename
 **  ~~~~~~~~
 */

static INT ReadFile( TEXT* newfile )
{
    TEXT* pfn;

    p_send3( Drg, O_DG_RESET_DATA_,  DG_CLEAR_SETTINGS );
    pfn = (TEXT*) p_send3( Drg->drg.DrgFile, O_DF_SET_FNAME, newfile );
    p_send3( Drg, O_DG_SET_FNAME, pfn );
    if( p_send2( Drg, O_DG_READ_FILE ) == FALSE )
    {
        p_send3( Drg, O_DG_RESET_DATA_,  DG_CLEAR_SETTINGS );
        return 1;
    }
    return 0;
}

/***************************************************************************
 **  DylError
 **  ~~~~~~~~
 */

VOID DylError( INT err )
{
    switch( err )
    {
    case USER_ABORT:
        hInfoPrint( SR_USER_ABORT );
        break;
    case INPUT_FILE_ERR:
        hInfoPrint( SR_INPUT_FILE_ERR );
        break;
    case MATHS_ERR:
        hInfoPrint( SR_MATHS_ERR );
        break;
    default:
        hInfoPrintErr( err );
    }
}

/***************************************************************************
 **  CallDylMember
 **  ~~~~~~~~~~~~~
 */

#pragma save,ENTER_CALL

LOCAL_C INT CallDylMember( HANDLE dyl, UINT member, VOID* data )
{
    VOID* pClass;
    INT err;

    pClass = f_newlibh( dyl, C_XCHANGE );
    if( p_send2( pClass, O_XCH_VERSION ) == VERSION_XCH )
    {
        err = p_entersend3( pClass, member, data );
    }
    else
    {
        hInfoPrint( SR_VDL_VERSION_ERR );
        err = 0;
    }
    p_send2( pClass, O_DESTROY );
    return err;
}

#pragma restore

/***************************************************************************
 **  ExportFile
 **  ~~~~~~~~~~
 */

GLDEF_C INT ExportFile( TEXT* libname, VOID* data )
{
    TEXT buf[P_FNAMESIZE];
    HANDLE dyl;
    INT err;

    p_fparse( libname, DatCommandPtr, buf, NULL );
    err = p_loadlib( buf, &dyl, TRUE );
    if( err == 0 )
    {
        err = p_enter4( CallDylMember, dyl, O_XCH_EXPORT, data );
        p_unloadlib( dyl );
    }
    else
    {
        hErrorDialog( err, SR_FILE_NAME_FMT, buf );
        return 0;
    }
    if( err )
    {
        p_delete( ((EXPORT_DATA*) data)->fname );
        DylError( err );
    }
    return err;
}

/***************************************************************************
 **  ImportFile  Replace current file with the given imported file.
 **  ~~~~~~~~~~  Returns 0=Success 1=VersionErr
 */

GLDEF_C INT ImportFile( TEXT* libname, TEXT* fname )
{
    TEXT buf[P_FNAMESIZE];
    TEXT* pfn;
    HANDLE dyl;
    INT err;
    ARECT rect;

    p_fparse( libname, DatCommandPtr, buf, NULL );
    err = p_loadlib( buf, &dyl, TRUE );
    if( err == 0 )
    {
        BegDraw();
        p_send3( Drg, O_DG_RESET_DATA_, DG_LEAVE_SETTINGS );
        /*p_send2( Drg, O_DG_SET_VIEW );*/
        err = p_enter4( CallDylMember, dyl, O_XCH_IMPORT, fname );
        p_unloadlib( dyl );
        if( err )
        {
            p_send3( Drg, O_DG_RESET_DATA_, DG_CLEAR_SETTINGS );
            DylError( err );
        }
        else
        {
            p_fparse( ".VEC", fname, buf, NULL );
            pfn = (TEXT*) p_send3( Drg->drg.DrgFile, O_DF_SET_FNAME, buf );
            p_send3( Drg, O_DG_SET_FNAME, pfn );
            Drg->drg.ChangedFlag = TRUE;
            p_send3( Drg, O_DG_GET_EXTENT, &rect );
            p_send3( Drg, O_DG_ZOOM_TO_BOX, &rect );
        }
        EndDraw();
        UpdateInfo( IW_ALL );
    }
    else
    {
        hErrorDialog( err, SR_FILE_NAME_FMT, buf );
        return 0;
    }
    return err;
}

/***************************************************************************
 **  DoExit  Exit the program
 **  ~~~~~~
 */

static VOID DoExit( PR_VECCM* self )
{
    KillUndo();
    if( ! Drg->drg.RegFlag )
    {
        p_send2( self, O_COM_ABOUT );
    }
    p_supersend2( self, O_COM_EXIT );
}

/*--------------------[ veccm Command Class members ]---------------------*/

#pragma METHOD_CALL

/***************************************************************************
 **  com_statwin  Cycle around status window On, Small and Off.
 **  ~~~~~~~~~~~  See SDK OOP Guide Page 5-11.
 */

METHOD VOID veccm_com_statwin( PR_VECCM* self )
{
    UINT swin;

    if( Drg->vec.View & VW_LSTATUS )
    {
        swin = VW_SSTATUS;
    }
    else if( Drg->vec.View & VW_SSTATUS )
    {
        swin = VW_OFF;
    }
    else
    {
        swin = VW_LSTATUS;
    }
    Drg->vec.View &= ~VW_STATUS;
    Drg->vec.View |= swin;
    p_send2( Drg, O_DG_SET_VIEW );
}

/***************************************************************************
 **  com_mode_change  Change view to the next in the Diamond List
 **  ~~~~~~~~~~~~~~~  See SDK HWIM Manual Page 4-4.
 */

METHOD VOID veccm_com_mode_change( PR_VECCM* self, INT shifted )
{
    INT i;
    UBYTE view;

    if( Drg->vec.DiamondList == 0 ) return; /* No list */
    view = Drg->vec.DiamondView;
    for( i = 0 ; i < VDL_MAX ; i++ )
    {
        if( shifted )
        {
            view >>= 1;
            if( view == 0 ) view = VDL_FOURTH;
        }
        else
        {
            view <<= 1;
            if( view > VDL_FOURTH ) view = 0x01;
        }
        if( view & Drg->vec.DiamondList )
        {
            break;
        }
    }
    p_send3( Drg, O_DG_CHANGE_VIEW, view );
}

/***************************************************************************
 **  com_file_change  Save current file and Create or Open (acording to
 **  ~~~~~~~~~~~~~~~  command) the file named pname.
 **  See SDK OOP Guide Page 11-2.
 */

METHOD INT veccm_com_file_change( PR_VECCM* self, INT command, TEXT* pname )
{
    TEXT* pfn;

    if( SaveChanges() == FALSE )
    {
        return 0;
    }
    switch( command )
    {
    case H_COMMAND_CREATE_FILE:
        p_send3( Drg, O_DG_RESET_DATA_, DG_CLEAR_SETTINGS );
        ReadTemplate( "default" );
        p_send2( Drg, O_DG_SET_VIEW );
        pfn = (TEXT*) p_send3( Drg->drg.DrgFile, O_DF_SET_FNAME, pname );
        p_send3( Drg, O_DG_SET_FNAME, pfn );
        break;
    case H_COMMAND_OPEN_FILE:
        ReadFile( pname );
        break;
    }
    return 0;
}

/***************************************************************************
 **  com_menu  Prepare Symbol Edit option
 **  ~~~~~~~~
 */

METHOD VOID veccm_com_menu( PR_VECCM* self, INT menu, PR_VAROOT* array )
{
    INT rid;
    UBYTE* op;

    if( w_ws->wserv.info->menubar_id != VECTOR_MENBAR ) return;

    if( menu == MENUBOP_EDIT ) /* Edit menu */
    {
        switch( Drg->drg.FileType )
        {
        case FTYPE_LIBRARY:
            rid = SR_EDIT_LIBRARY_MO;
            break;
        case FTYPE_FONT:
            rid = SR_EDIT_FONT_MO;
            break;
        default:
            p_send3( array, O_VA_DELETE, MENUOP_SYMBOL_EDIT );
            return;
        }
        op = (UBYTE*) p_send3( array, O_VA_PREC, MENUOP_SYMBOL_EDIT );
        hLoadResBuf( rid,  op + 2 );
    }
}

/***************************************************************************
 **  com_accl_check  Check Symbol Edit option
 **  ~~~~~~~~~~~~~~
 */

METHOD INT veccm_com_accl_check( PR_VECCM* self, INT comid )
{
    UINT ftype;

    if( comid == O_COM_EDIT_SYMBOL )
    {
        ftype = Drg->drg.FileType;
        if( ftype != FTYPE_LIBRARY && ftype != FTYPE_FONT )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/***************************************************************************
 **  com_exit  Save current file and, if no errors, Exit application.
 **  ~~~~~~~~  See SDK OOP Guide Page 11-3.
 */

METHOD VOID veccm_com_exit( PR_VECCM* self )
{
    if( SaveChanges() )
    {
        DoExit( self );
    }
}

/***************************************************************************
 **  com_exit_lose  Exit without saving changes
 **  ~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_exit_lose( PR_VECCM* self )
{
    if( Drg->drg.ChangedFlag == FALSE || hConfirm( SR_LOSE_ALL_CHANGES ) )
    {
        DoExit( self );
    }
}

/***************************************************************************
 **  com_about method
 **  ~~~~~~~~~
 */

METHOD VOID veccm_com_about( PR_VECCM* self )
{
    LaunchDialog( NULL, ABOUT_DIALOG, C_ABOUTDLG );
}

/***************************************************************************
 **  com_edit_symbol  Edit symbol or font method
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_edit_symbol( PR_VECCM* self )
{
    if( Drg->drg.FileType == FTYPE_LIBRARY )
    {
        p_send3( w_ws, O_WS_DO_SUBMENU, LIBRARY_SUB_ACCS );
    }
    else if( Drg->drg.FileType == FTYPE_FONT )
    {
        p_send3( w_ws, O_WS_DO_SUBMENU, FONT_SUB_ACCS );
    }
}

/***************************************************************************
 **  com_new_file  Save current file and, if no errors, open new file.
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_new_file( PR_VECCM* self )
{
    TEXT fn[P_FNAMESIZE];
    CNEWDLG_DATA data;
    TEXT* pfn;

    data.fname = fn;
    if( LaunchDialog( &data, CREATE_NEW_DLG, C_CNEWDLG ) && SaveChanges() )
    {
        p_send3( Drg, O_DG_RESET_DATA_, DG_CLEAR_SETTINGS );
        if( data.tplate[0] == '\0' )
        {
            p_scpy( data.tplate, "default" );
        }
        if( data.tplate[0] != '*' )
        {
            ReadTemplate( data.tplate );
        }
        p_send2( Drg, O_DG_SET_VIEW );

        p_send3( Drg->drg.DrgFile, O_DF_SET_FNAME, fn );
        pfn = GetPath( fn );
        p_send3( Drg, O_DG_SET_FNAME, pfn );
    }
}

/***************************************************************************
 **  com_open_file  Save current file and, if no errors, open existing file.
 **  ~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_open_file( PR_VECCM* self )
{
    OPENDLG_DATA data;
    TEXT oldfn[P_FNAMESIZE];
    INT ret;

    GetPath( data.fname );
    p_scpy( oldfn, (TEXT*) p_send2( Drg->drg.DrgFile, O_DF_GET_FNAME ) );
    if( LaunchDialog( &data, OPEN_DIALOG, C_OPENDLG ) )
    {
        Drg->drg.ChangedFlag = TRUE;
        if( SaveChanges() == FALSE )
        {
            return;
        }
        switch( data.format )
        {
        case OPENDLG_VEC:
            ret = ReadFile( data.fname );
            break;
        case OPENDLG_DXF:
            ret = ImportFile( "\\APP\\VECTOR\\VECDXF.VDL", data.fname );
            break;
        case OPENDLG_QCD:
            ret = ImportFile( "\\APP\\VECTOR\\VECQCD.VDL", data.fname );
            break;
        }
        if( ret )
        {
            ReadFile( oldfn );
        }
    }
}

/***************************************************************************
 **  com_save_as  Change filename and save data.
 **  ~~~~~~~~~~~
 */

METHOD VOID veccm_com_save_as( PR_VECCM* self )
{
    TEXT fn[P_FNAMESIZE];
    TEXT oldfn[P_FNAMESIZE];
    TEXT* pfn;
    SAVEDLG_DATA data;
    EXPORT_DATA_QCD qcd;

    pfn = (TEXT*) p_send2( Drg->drg.DrgFile, O_DF_GET_FNAME );
    p_scpy( oldfn, pfn );
    data.fname = fn;
    if( LaunchDialog( &data, SAVE_AS_DLG, C_SAVEDLG ) )
    {
        switch( data.export )
        {
        case SAVE_FTYPE_PIC:
        case SAVE_FTYPE_PCX:
            p_send5( Extra, O_EX_EXPORT, fn, data.export, data.width );
            return;
        case SAVE_FTYPE_DXF:
            qcd.fname = fn;
            /*qcd.res = data.qcdres;*/
            ExportFile( "\\App\\Vector\\Vecdxf.vdl", &qcd );
            return;
        case SAVE_FTYPE_QCD:
            qcd.fname = fn;
            qcd.res = data.qcdres;
            ExportFile( "\\APP\\VECTOR\\VECQCD.VDL", &qcd );
            return;
        }

        if( data.export )
        {
            p_send5( Extra, O_EX_EXPORT, fn, data.export, data.width );
            return;
        }
        p_send3( Drg->drg.DrgFile, O_DF_SET_FNAME, fn );
        Drg->drg.ChangedFlag = TRUE;
        if( SaveChanges() == FALSE )
        {
            data.usenew = FALSE;
        }
        if( data.usenew )
        {
            p_send3( Drg, O_DG_SET_FNAME, pfn );
        }
        else
        {
            p_send3( Drg->drg.DrgFile, O_DF_SET_FNAME, oldfn );
        }
    }
}

/***************************************************************************
 **  com_save  Saves current file if changed. Only displays a dialog if a
 **  ~~~~~~~~  problem.
 */

METHOD VOID veccm_com_save( PR_VECCM* self )
{
    Drg->drg.ChangedFlag = TRUE;
    if( SaveChanges() )
    {
        hInfoPrint( SR_FILE_SAVED );
    }
}

/***************************************************************************
 **  com_revert  Reopen existing file.
 **  ~~~~~~~~~~
 */

METHOD VOID veccm_com_revert( PR_VECCM* self )
{
    if( hConfirm( SR_LOSE_ALL_CHANGES ) )
    {
        p_send3( Drg, O_DG_RESET_DATA_, DG_CLEAR_SETTINGS );
        if( p_send2( Drg, O_DG_READ_FILE ) == FALSE )
        {
            p_send3( Drg, O_DG_RESET_DATA_, DG_CLEAR_SETTINGS );
        }
    }
}

/***************************************************************************
 **  com_undo  Undo the last command
 **  ~~~~~~~~
 */

METHOD VOID veccm_com_undo( PR_VECCM* self )
{
    p_send2( Undo, O_UD_UNDO );
}

/***************************************************************************
 **  com_redo  Redo the undo command
 **  ~~~~~~~~
 */

METHOD VOID veccm_com_redo( PR_VECCM* self )
{
    p_send2( Redo, O_UD_UNDO );
}

/***************************************************************************
 **  com_erase  Erase selected elements
 **  ~~~~~~~~~
 */

METHOD VOID veccm_com_erase( PR_VECCM* self )
{
    CreateBuild( C_EERASE );
}

/***************************************************************************
 **  com_group  Create Group from selected elements
 **  ~~~~~~~~~
 */

METHOD VOID veccm_com_group( PR_VECCM* self )
{
    CreateBuild( C_EGROUP );
}

/***************************************************************************
 **  com_ungroup  Ungroup selected elements
 **  ~~~~~~~~~
 */

METHOD VOID veccm_com_ungroup( PR_VECCM* self )
{
    CreateBuild( C_EUNGRP );
}

/***************************************************************************
 **  com_move  Move selected elements
 **  ~~~~~~~~
 */

METHOD VOID veccm_com_move( PR_VECCM* self )
{
    CreateBuild( C_EMOVE );
}

/***************************************************************************
 **  com_copy  Copy selected elements
 **  ~~~~~~~~
 */

METHOD VOID veccm_com_copy( PR_VECCM* self )
{
    CreateBuild( C_ECOPY );
}

/***************************************************************************
 **  com_scale  Scale selected elements
 **  ~~~~~~~~~
 */

METHOD VOID veccm_com_scale( PR_VECCM* self )
{
    CreateBuild( C_ESCALE );
}

/***************************************************************************
 **  com_stretch  Rotate selected elements
 **  ~~~~~~~~~~~
 */

METHOD VOID veccm_com_stretch( PR_VECCM* self )
{
    CreateBuild( C_ESTRETCH );
}

/***************************************************************************
 **  com_rotate  Rotate selected elements
 **  ~~~~~~~~~~
 */

METHOD VOID veccm_com_rotate( PR_VECCM* self )
{
    CreateBuild( C_EROTATE );
}

/***************************************************************************
 **  com_mirror  Mirror image selected elements
 **  ~~~~~~~~~~
 */

METHOD VOID veccm_com_mirror( PR_VECCM* self )
{
    CreateBuild( C_EMIRROR );
}

/***************************************************************************
 **  com_break  Break selected elements
 **  ~~~~~~~~~
 */

METHOD VOID veccm_com_break( PR_VECCM* self )
{
    CreateBuild( C_EBREAK );
}

/***************************************************************************
 **  com_property  Show Property for selected elements
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_property( PR_VECCM* self )
{
    CreateBuild( C_EPROP );
}

/***************************************************************************
 **  com_line  Draw a line
 **  ~~~~~~~~
 */

METHOD VOID veccm_com_line( PR_VECCM* self )
{
    CreateBuild( C_NLINE );
}

/***************************************************************************
 **  com_polyline  Draw a polyline
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_polyline( PR_VECCM* self )
{
    CreateBuild( C_NPLINE );
}

/***************************************************************************
 **  com_arc_centre  Draw a circular arc
 **  ~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_arc_centre( PR_VECCM* self )
{
    CreateBuild( C_NARC );
}

/***************************************************************************
 **  com_quadrant  Draw a quadrant (centre) arc
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_quadrant( PR_VECCM* self )
{
    CreateBuild( C_NQUAD );
}

/***************************************************************************
 **  com_arc_3pt  Draw a circular 3 point arc
 **  ~~~~~~~~~~~
 */

METHOD VOID veccm_com_arc_3pt( PR_VECCM* self )
{
    CreateBuild( C_N3PARC );
}

/***************************************************************************
 **  com_text  Draw a text line
 **  ~~~~~~~~
 */

METHOD VOID veccm_com_text( PR_VECCM* self )
{
    CreateBuild( C_NTEXT );
}

/***************************************************************************
 **  com_box  Draw a rectangular box
 **  ~~~~~~~
 */

METHOD VOID veccm_com_box( PR_VECCM* self )
{
    CreateBuild( C_NBOX );
}

/***************************************************************************
 **  com_polygon  Draw a polygon
 **  ~~~~~~~~~~~
 */

METHOD VOID veccm_com_polygon( PR_VECCM* self )
{
    CreateBuild( C_NPGON );
}

/***************************************************************************
 **  com_circle  Draw a circle
 **  ~~~~~~~~~~
 */

METHOD VOID veccm_com_circle( PR_VECCM* self )
{
    CreateBuild( C_NCIRCLE );
}

/***************************************************************************
 **  com_symbol  Select and draw a symbol
 **  ~~~~~~~~~~
 */

METHOD VOID veccm_com_symbol( PR_VECCM* self )
{
    CreateBuild( C_NSYM );
}

/***************************************************************************
 **  com_dim  Dimensions method
 **  ~~~~~~~
 */

METHOD VOID veccm_com_dim( PR_VECCM* self )
{
    p_send3( w_ws, O_WS_DO_SUBMENU, DIM_SUB_ACCS );
}

/***************************************************************************
 **  com_first  Display the first (in the diamond list) view.
 **  ~~~~~~~~~
 */

METHOD VOID veccm_com_first( PR_VECCM* self )
{
    p_send3( Drg, O_DG_CHANGE_VIEW, VDL_FIRST );
}

/***************************************************************************
 **  com_second  Display the second (in the diamond list) view.
 **  ~~~~~~~~~~
 */

METHOD VOID veccm_com_second( PR_VECCM* self )
{
    p_send3( Drg, O_DG_CHANGE_VIEW, VDL_SECOND );
}

/***************************************************************************
 **  com_third  Display the third (in the diamond list) view.
 **  ~~~~~~~~~
 */

METHOD VOID veccm_com_third( PR_VECCM* self )
{
    p_send3( Drg, O_DG_CHANGE_VIEW, VDL_THIRD );
}

/***************************************************************************
 **  com_fourth  Display the fourth (in the diamond list) view.
 **  ~~~~~~~~~~
 */

METHOD VOID veccm_com_fourth( PR_VECCM* self )
{
    p_send3( Drg, O_DG_CHANGE_VIEW, VDL_FOURTH );
}

/***************************************************************************
 **  com_set_list  Dialog to select items to be included int the Status
 **  ~~~~~~~~~~~~  Window diamond list.
 */

METHOD VOID veccm_com_set_list( PR_VECCM* self )
{
    if( LaunchDialog( NULL, DIAMONDLIST_DIALOG, C_DLISTDLG ) )
    {
        p_send2( Drg, O_DG_SET_DLIST );
    }
}

/***************************************************************************
 **  com_snap  Dialog to set snap & grid.
 **  ~~~~~~~~
 */

METHOD VOID veccm_com_snap( PR_VECCM* self )
{
     LaunchDialog( NULL, SNAP_GRID_DIALOG, C_SGRIDDLG );
}

/***************************************************************************
 **  com_layers  Dialog to set layers for display and current property.
 **  ~~~~~~~~~
 */

METHOD VOID veccm_com_layers( PR_VECCM* self )
{
    LaunchDialog( NULL, LAYER_DIALOG, C_LAYERDLG );
}

/***************************************************************************
 **  com_set_text  Dialog to set text style.
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_set_text( PR_VECCM* self )
{
    TEXTDLG_DATA tsd;

    tsd.select = Drg->vec.Text;
    for(;;)
    {
        LaunchDialog( &tsd, SET_TEXT_DIALOG, C_STEXTDLG );
        switch( tsd.key )
        {
        case '+': /* Add new style */
            LaunchDialog( &tsd, ADD_TSTYLE_DIALOG, C_ATEXTDLG );
            break;
        case ' ': /* Change current style */
            LaunchDialog( &tsd, ADD_TSTYLE_DIALOG, C_CTEXTDLG );
            break;
        case W_KEY_DELETE_LEFT: /* Delete Current style */
            if( DeleteTextStyle( tsd.select ) )
            {
                tsd.select = Drg->vec.Text;
            }
            break;
        case W_KEY_RETURN: /* Select style */
            Drg->vec.Text = tsd.select;
            /* Fall through */
        default:
            UpdateInfo( IW_TEXT );
            return;
        }
    }
}

/***************************************************************************
 **  com_view method
 **  ~~~~~~~~
 */

METHOD VOID veccm_com_view( PR_VECCM* self )
{
    LaunchDialog( NULL, VIEW_DIALOG, C_VIEWDLG );
}

/***************************************************************************
 **  com_set_pref method
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_set_pref( PR_VECCM* self )
{
    p_send3( w_ws, O_WS_DO_SUBMENU, PREF_SUB_ACCS );
}

/***************************************************************************
 **  com_set_print  Save current file and, if no errors, open existing file.
 **  ~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_set_print( PR_VECCM* self )
{
    PRN prn;

    prn = Drg->drg.Cfg.prn;
    LaunchDialog( &prn, SET_PRINTER_DIALOG, C_SPRNDLG );
}

/***************************************************************************
 **  com_print  Print the drawing.
 **  ~~~~~~~~~
 */

METHOD VOID veccm_com_print( PR_VECCM* self )
{
    DoPrint();
}

/***************************************************************************
 **  com_jump method
 **  ~~~~~~~~
 */

METHOD VOID veccm_com_jump( PR_VECCM* self )
{
    p_send3( w_ws, O_WS_DO_SUBMENU, JUMP_SUB_ACCS );
}

/***************************************************************************
 **  com_zoom_to  Submenu
 **  ~~~~~~~~~~~
 */

METHOD VOID veccm_com_zoom_to( PR_VECCM* self )
{
    p_send3( w_ws, O_WS_DO_SUBMENU, ZOOM_SUB_ACCS );
}

/***************************************************************************
 **  com_zoom_in  Zoom in, enlarge drawing by the ZoomStep percentage.
 **  ~~~~~~~~~~~
 */

METHOD VOID veccm_com_zoom_in( PR_VECCM* self )
{
    SWORD upp;

    upp = ( (long) Drg->vec.upp * 100 ) / Drg->vec.ZoomPercent;
    p_send3( Drg, O_DG_SET_ZOOM, upp );
}

/***************************************************************************
 **  com_zoom_out  Zoom out, reduce drawing by the ZoomStep percentage.
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_zoom_out( PR_VECCM* self )
{
    SWORD upp;

    upp = ( (long) Drg->vec.upp * Drg->vec.ZoomPercent ) / 100 + 1;
    p_send3( Drg, O_DG_SET_ZOOM, upp );
}

/***************************************************************************
 **  com_zoom_box  Set Zoom to box drawn with a temporary build
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_zoom_box( PR_VECCM* self )
{
    PR_BUILD* tbld;

    if( Build && Build->build.cmd == SR_ZOOM_BOX ) return;

    tbld = f_newlibh( BuildDyl, C_ZBOX );
    p_send2( tbld, O_BLD_INIT );
    UpdateInfo( IW_MODE );
}

/***************************************************************************
 **  com_zoom_extent  Set Zoom to fit current drawing
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_zoom_extent( PR_VECCM* self )
{
    ARECT rect;

    p_send3( Drg, O_DG_GET_EXTENT, &rect );
    p_send3( Drg, O_DG_ZOOM_TO_BOX, &rect );
}

/***************************************************************************
 **  com_zoom_actual  Set Zoom to acual size (1 pixel = 0.26mm for Psion 3a)
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_zoom_actual( PR_VECCM* self )
{
    p_send3( Drg, O_DG_SET_ZOOM, 26 );
}

/***************************************************************************
 **  com_zoom_width  Set Zoom to show full width of page
 **  ~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_zoom_width( PR_VECCM* self )
{
    SWORD upp;
    P_EXTENT extent;

    GetDWinSize( &extent );
    upp = ( Drg->vec.Page.lim.x - Drg->vec.Page.pos.x ) / extent.width + 1;
    p_send3( Drg, O_DG_SET_ZOOM, upp );
}

/***************************************************************************
 **  com_zoom_height  Set Zoom to show full height of page
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_zoom_height( PR_VECCM* self )
{
    SWORD upp;
    P_EXTENT extent;

    GetDWinSize( &extent );
    upp = ( Drg->vec.Page.lim.y - Drg->vec.Page.pos.y ) / extent.height + 1;
    p_send3( Drg, O_DG_SET_ZOOM, upp );
}

/***************************************************************************
 **  com_zoom_factor  Set Zoom to set multiplication factor
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_zoom_factor( PR_VECCM* self )
{
    LaunchDialog( NULL, SET_ZOOM_MAG_DIALOG, C_ZOOMMAGDLG );
}

/***************************************************************************
 **  com_dim_horiz Draw a horizontal dimension
 **  ~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_dim_horiz( PR_VECCM* self )
{
    CreateBuild( C_NDIMH );
}

/***************************************************************************
 **  com_jmp_abs  Jump to absotute x,y location
 **  ~~~~~~~~~~~
 */

METHOD VOID veccm_com_jmp_abs( PR_VECCM* self )
{
    p_send5( Extra, O_EX_JUMP_DIALOG, POINT_PROP_DIALOG, &Drg->vec.Orig, JMP_ABSOLUTE );
}

/***************************************************************************
 **  com_jmp_cur_cart  Jump to x,y location relative to cursor
 **  ~~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_jmp_cur_cart( PR_VECCM* self )
{
    p_send5( Extra, O_EX_JUMP_DIALOG, JUMP_XY_REL_DIALOG, &Drg->vec.Cur, JMP_RELATIVE );
}

/***************************************************************************
 **  com_jmp_cur_polar  Jump to polar point relative to cursor
 **  ~~~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_jmp_cur_polar( PR_VECCM* self )
{
    p_send5( Extra, O_EX_JUMP_DIALOG, JUMP_POLAR_DIALOG, &Drg->vec.Cur,
        JMP_RELATIVE | JMP_POLAR );
}

/***************************************************************************
 **  com_jmp_last_cart  Jump to x,y location relative to last Enter
 **  ~~~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_jmp_last_cart( PR_VECCM* self )
{
    p_send5( Extra, O_EX_JUMP_DIALOG, JUMP_XY_REL_DIALOG, &Drg->drg.LastPt, JMP_RELATIVE );
}

/***************************************************************************
 **  com_jmp_last_polar  Jump to polar point relative to last Enter
 **  ~~~~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_jmp_last_polar( PR_VECCM* self )
{
    p_send5( Extra, O_EX_JUMP_DIALOG, JUMP_POLAR_DIALOG, &Drg->drg.LastPt,
        JMP_RELATIVE | JMP_POLAR );
}

/***************************************************************************
 **  com_jmp_end  Jump to nearest End of line
 **  ~~~~~~~~~~~
 */

METHOD VOID veccm_com_jmp_end( PR_VECCM* self )
{
    p_send3( Extra, O_EX_JUMP_NEAR, NEAR_LINE_END );
}

/***************************************************************************
 **  com_jmp_mid  Jump to nearest middle of line
 **  ~~~~~~~~~~~
 */

METHOD VOID veccm_com_jmp_mid( PR_VECCM* self )
{
    p_send3( Extra, O_EX_JUMP_NEAR, NEAR_LINE_MID );
}

/***************************************************************************
 **  com_jmp_intersect  Jump to nearest intersect of two lines
 **  ~~~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_jmp_intersect( PR_VECCM* self )
{
    p_send3( Extra, O_EX_JUMP_NEAR, NEAR_INTERSECT );
}

/***************************************************************************
 **  com_jmp_centre  Jump to nearest centre of circle or arc
 **  ~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_jmp_centre( PR_VECCM* self )
{
    p_send3( Extra, O_EX_JUMP_NEAR, NEAR_CENTRE );
}

/***************************************************************************
 **  com_jmp_quad  Jump to nearest quadrant of circle or arc
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_jmp_quad( PR_VECCM* self )
{
    p_send3( Extra, O_EX_JUMP_NEAR, NEAR_QUADRANT );
}

/***************************************************************************
 **  com_jmp_perp  Jump to nearest perpendicular point from last enter point
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_jmp_perp( PR_VECCM* self )
{
    p_send3( Extra, O_EX_JUMP_NEAR, NEAR_PERP );
}

/***************************************************************************
 **  com_jmp_tangent  Jump to nearest tangent point from last enter point
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_jmp_tangent( PR_VECCM* self )
{
    p_send3( Extra, O_EX_JUMP_NEAR, NEAR_TANGENT );
}

/***************************************************************************
 **  com_create_sym  Create Symbol from selected elements
 **  ~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_create_sym( PR_VECCM* self )
{
    if( Drg->drg.FileType == FTYPE_LIBRARY )
    {
        CreateBuild( C_ECSYM );
    }
    else
    {
        hInfoPrint( SR_NOT_LIB_FILE );
    }
}

/***************************************************************************
 **  com_ungroup_sym  Ungroup selected symbol elements
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_ungroup_sym( PR_VECCM* self )
{
    if( Drg->drg.FileType == FTYPE_LIBRARY )
    {
        CreateBuild( C_EUNSYM );
    }
    else
    {
        hInfoPrint( SR_NOT_LIB_FILE );
    }
}

/***************************************************************************
 **  com_list_sym  Dialog to list symbols in current file (doesn't actually
 **  ~~~~~~~~~~~~  do anything).
 */

METHOD VOID veccm_com_list_sym( PR_VECCM* self )
{
    if( Drg->drg.FileType == FTYPE_LIBRARY )
    {
        if( Drg->drg.SymbolCount )
        {
            LaunchDialog( NULL, LIST_SYMBOL_DIALOG, C_LISTSYMDLG );
        }
        else
        {
            hInfoPrint( SR_NO_SYMBOLS );
        }
    }
    else
    {
        hInfoPrint( SR_NOT_LIB_FILE );
    }
}

/***************************************************************************
 **  com_create_char  Create Font character from selected elements
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_create_char( PR_VECCM* self )
{
    if( Drg->drg.FileType == FTYPE_FONT )
    {
        CreateBuild( C_ECCHAR );
    }
    else
    {
        hInfoPrint( SR_NOT_FONT_FILE );
    }
}

/***************************************************************************
 **  com_create_space  Dialog to create a font character space
 **  ~~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_create_space( PR_VECCM* self )
{
    if( Drg->drg.FileType == FTYPE_FONT )
    {
        LaunchDialog( NULL, CREATE_FONT_SPACE_DIALOG, C_CSPACEDLG );
    }
    else
    {
        hInfoPrint( SR_NOT_FONT_FILE );
    }
}

/***************************************************************************
 **  com_ungroup_char  Ungroup selected character elements
 **  ~~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_ungroup_char( PR_VECCM* self )
{
    if( Drg->drg.FileType == FTYPE_FONT )
    {
        CreateBuild( C_EUNCHAR );
    }
    else
    {
        hInfoPrint( SR_NOT_FONT_FILE );
    }
}

/***************************************************************************
 **  com_set_cursor  Dialog to set cursor appearance.
 **  ~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_set_cursor( PR_VECCM* self )
{
    LaunchDialog( NULL, SET_CURSOR_DIALOG, C_SETCURDLG );
}

/***************************************************************************
 **  com_select_box  Dialog to set cursor selection box.
 **  ~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_select_box( PR_VECCM* self )
{
    LaunchDialog( NULL, SELECT_BOX_DIALOG, C_SELBOXDLG );
}

/***************************************************************************
 **  com_movement  Dialog to set movement key functions.
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_movement( PR_VECCM* self )
{
    LaunchDialog( NULL, SET_MOVEMENT_DIALOG, C_SETMOVEDLG );
}

/***************************************************************************
 **  com_keyboard  Dialog to set keyboard functions.
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_keyboard( PR_VECCM* self )
{
    LaunchDialog( NULL, SET_KEY_DIALOG, C_SKEYDLG );
}

/***************************************************************************
 **  com_units  Dialog to set units for display and input.
 **  ~~~~~~~~~
 */

METHOD VOID veccm_com_units( PR_VECCM* self )
{
    if( LaunchDialog( NULL, SET_UNITS_DIALOG, C_SUNITSDLG ) )
    {
        LaunchDialog( NULL, SHOW_PAGE_DIALOG, C_SHOWPGDLG );
    }
}

/***************************************************************************
 **  com_page  Dialog to set Page size.
 **  ~~~~~~~~
 */

METHOD VOID veccm_com_page( PR_VECCM* self )
{
    if( LaunchDialog( NULL, SET_PAGE_DIALOG, C_SPAGEDLG ) )
    {
        LaunchDialog( NULL, SHOW_PAGE_DIALOG, C_SHOWPGDLG );
    }
}

/***************************************************************************
 **  com_origin  Dialog to set Origin for measurements.
 **  ~~~~~~~~~~
 */

METHOD VOID veccm_com_origin( PR_VECCM* self )
{
    if( LaunchDialog( NULL, SET_ORIGIN_DIALOG, C_SORIGDLG ) )
    {
        LaunchDialog( NULL, SHOW_PAGE_DIALOG, C_SHOWPGDLG );
    }
}

/***************************************************************************
 **  com_move_copy  Dialog to set command actions
 **  ~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_move_copy( PR_VECCM* self )
{
    LaunchDialog( NULL, MOVE_COPY_DIALOG, C_MOVCPYDLG );
}

/***************************************************************************
 **  com_jump_box  Dialog to set jump to box.
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_jump_box( PR_VECCM* self )
{
    LaunchDialog( NULL, JUMP_BOX_DIALOG, C_JMPBOXDLG );
}

/***************************************************************************
 **  com_active_undo  Dialog to set undo command on/off
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_active_undo( PR_VECCM* self )
{
    LaunchDialog( NULL, SET_UNDO_DIALOG, C_UNDODLG );
}

/***************************************************************************
 **  com_set_zoom  Dialog to set zoom step percentage.
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_set_zoom( PR_VECCM* self )
{
    LaunchDialog( NULL, SET_ZOOM_DIALOG, C_SETZOOMDLG );
}

/***************************************************************************
 **  com_tog_snap  Toggle Snap
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_tog_snap( PR_VECCM* self )
{
    GRIDSET grid;

    grid = Drg->vec.Pref.grid;
    grid.flags ^= GRID_SNAP;
    p_send3( Drg, O_DG_SET_SNAP_GRID, &grid );
}

/***************************************************************************
 **  com_tog_b_grid  Toggle Grid between Black and Off
 **  ~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_tog_b_grid( PR_VECCM* self )
{
    GRIDSET grid;

    grid = Drg->vec.Pref.grid;
    grid.flags ^= GRID_BLACK;
    grid.flags &= ~GRID_GREY;
    p_send3( Drg, O_DG_SET_SNAP_GRID, &grid );
}

/***************************************************************************
 **  com_tog_g_grid  Toggle Grid between Grey and Off
 **  ~~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_tog_g_grid( PR_VECCM* self )
{
    GRIDSET grid;

    grid = Drg->vec.Pref.grid;
    grid.flags ^= GRID_GREY;
    grid.flags &= ~GRID_BLACK;
    p_send3( Drg, O_DG_SET_SNAP_GRID, &grid );
}

/***************************************************************************
 **  com_tog_info  Toggle Info window
 **  ~~~~~~~~~~~~
 */

METHOD VOID veccm_com_tog_info( PR_VECCM* self )
{
    Drg->vec.View ^= VW_INFO;
    p_send2( Drg, O_DG_SET_VIEW );
}

/***************************************************************************
 **  com_tog_quick  Toggle Quick draw flag
 **  ~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_tog_quick( PR_VECCM* self )
{
    Drg->drg.Quick ^= QUICK_TOGGLE;
    p_send2( Drg, O_DG_REDRAW );
}

/***************************************************************************
 **  com_not_alloc  Key code not allocated place holder
 **  ~~~~~~~~~~~~~
 */

METHOD VOID veccm_com_not_alloc( PR_VECCM* self )
{
    hInfoPrint( SR_KEY_NOT_ALLOCATED );
}


#ifdef PSION_SI

/***************************************************************************
 **  com_arc  Arc sub menu (Siena only)
 **  ~~~~~~~
 */

METHOD VOID veccm_com_arc( PR_VECCM* self )
{
    p_send3( w_ws, O_WS_DO_SUBMENU, ARC_SUB_ACCS );
}

#endif /* PSION_SI */

/***************************************************************************
 **  com_not_yet  Temporary place holder. For development only.
 **  ~~~~~~~~~~~
 */

METHOD VOID veccm_com_not_yet( PR_VECCM* self )
{
    wInfoMsg( "Not yet available" );
}

/* End of VECCOM.C file */
