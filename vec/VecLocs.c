/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: LOCAL FILE SCAN CLASSES          *  Date Started: 16 May 1997  *
 *    File: VECLOCS.C       Type: C SOURCE   *  Date Revised: 16 May 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <hwim.h>
#include <vector.g>
#include "vector.h"

/***************************************************************************
 **  DeleteTempFiles  Delete all unused temporary files
 **  ~~~~~~~~~~~~~~~
 */

VOID DeleteTempFiles( VOID )
{
    PR_DELTMP* loc;

    loc = p_new( CAT_VECTOR_VECTOR, C_DELTMP );
    if( loc )
    {
        p_send4( loc, O_LS_SCAN, "vec\\temp\\*.unq", LOCS_FLG_LOC );
        hDestroy( loc );
    }
}

VOID ReadTemplate( TEXT* fname )
{
    PR_TPOPEN* scan;

    if( fname[0] != '\0' )
    {
        scan = f_new( CAT_VECTOR_VECTOR, C_TPOPEN );
        p_send3( scan, O_TPO_OPEN, fname );
        hDestroy( scan );
    }
}

#pragma METHOD_CALL

/***************************************************************************
 **  lklocs  Class to create a list of all available files with a given
 **  ~~~~~~  extension. If always is TRUE then a vastr object is returned
 **  even if it is empty.
 */

METHOD VOID* lklist_lkl_create_list( PR_LKLIST* self, TEXT* ext, INT always )
{
    TEXT buf[P_FNAMESIZE];

    self->lklist.list = f_new( CAT_VECTOR_OLIB, C_VASTR );
    p_send3( self->lklist.list, O_VA_INIT, UNAME_MAX * 2 );

    p_atos( buf, "app\\vector\\*.%s", ext );
    p_send4( self, O_LS_SCAN, buf, LOCS_FLG_LOC );
    if( self->lklist.count == 0 && ! always )
    {
        hDestroy( self->lklist.list );
        return NULL;
    }

    return self->lklist.list;
}

METHOD INT lklist_ls_filename( PR_LKLIST* self, UBYTE* pname )
{
    TEXT buf[14];
    INT i;

    if( p_scmpi( (TEXT*) pname, DatUsedPathNamePtr ) == 0 )
    {
        return FALSE;
    }
    p_scpy( buf, (TEXT*) self->locs.pname );
    i = p_sloc( buf, '.' );
    if( i < 1 ) return FALSE;
    buf[i] = '\0';
    p_scap( buf );
    if( p_send3( self->lklist.list, O_VA_SEARCH, buf ) < 0 )
    {
        p_send4( self->lklist.list, O_VA_INSERT, self->lklist.count, buf );
        self->lklist.count++;
    }
    return FALSE; /* Keep looking */
}

/***************************************************************************
 **  ftopen  Class to open a given font file name.
 **  ~~~~~~
 */

METHOD VOID ftopen_fto_open( PR_FTOPEN* self, FONT* font )
{
    TEXT buf[P_FNAMESIZE];

    self->ftopen.pFont = font;
    p_atos( buf, "app\\vector\\%s.vft", font->fname );
    p_send4( self, O_LS_SCAN, buf, LOCS_FLG_LOC );
}

METHOD INT ftopen_ls_filename( PR_FTOPEN* self, UBYTE* pname )
{
    FONT* font;
    INT ret;
    INT fhdr;
    TBHDR thdr;

    font = self->ftopen.pFont;
    ret = p_open( &font->pfcb, (TEXT*) pname,
        P_FSTREAM | P_FOPEN | P_FRANDOM | P_FSHARE );
    if( ret < 0 )
    {
        return FALSE; /* Keep looking */
    }
    fhdr = GetHeaderSize( font->pfcb );
    if( fhdr == 0 ) goto FontNotFound;
    f_read( font->pfcb, &font->data, sizeof(UWORD) );
    ret = SeekTable( font->pfcb, 0, T_CHAR_LIST, &thdr );
    if( ret < 0 ) goto FontNotFound;
    font->chlist = ret;
    /* Not yet used */
    /*font->fattr = NULL;*/
    return TRUE; /* All done */

FontNotFound:
    p_close( font->pfcb );
    font->pfcb = NULL;
    return FALSE; /* Keep looking */
}

/***************************************************************************
 **  lkopen  Class to open a given library file name.
 **  ~~~~~~
 */

METHOD VOID lkopen_lko_open( PR_LKOPEN* self, LIB* pLib )
{
    TEXT buf[P_FNAMESIZE];

    self->lkopen.pLib = pLib;
    p_atos( buf, "app\\vector\\%s.vsl", pLib->fname );
    p_send4( self, O_LS_SCAN, buf, LOCS_FLG_LOC );
}

METHOD INT lkopen_ls_filename( PR_LKOPEN* self, UBYTE* pname )
{
    LIB* pLib;
    INT ret;
    INT fhdr;
    TBHDR thdr;

    pLib = self->lkopen.pLib;
    ret = p_open( &pLib->pfcb, (TEXT*) pname, P_FSTREAM | P_FOPEN | P_FRANDOM | P_FSHARE );
    if( ret < 0 )
    {
        pLib->pfcb = NULL;
        return FALSE; /* Keep looking */
    }
    fhdr = GetHeaderSize( pLib->pfcb );
    if( fhdr == 0 ) goto LibNotFound;
    f_read( pLib->pfcb, &pLib->data, sizeof(UWORD) );
    ret = SeekTable( pLib->pfcb, 0, T_SYMBOLS, &thdr );
    if( ret < 0 ) goto LibNotFound;
    pLib->symlist = ret;
    pLib->symcount = thdr.size / sizeof(TE_SYM);
    Drg->vec.CurSym = 0;
    return TRUE; /* All done */

LibNotFound:
    p_close( pLib->pfcb );
    pLib->pfcb = NULL;
    return FALSE; /* Keep looking */
}

/***************************************************************************
 **  tpopen  Class to read in a template file.
 **  ~~~~~~
 */

METHOD INT tpopen_tpo_open( PR_TPOPEN* self, TEXT* lname )
{
    TEXT buf[P_FNAMESIZE];

    p_atos( buf, "app\\vector\\%s.vtp", lname );
    p_send4( self, O_LS_SCAN, buf, LOCS_FLG_LOC );
    return self->tpopen.success;
}

METHOD INT tpopen_ls_filename( PR_TPOPEN* self, UBYTE* pname )
{
    p_send3( Drg->drg.DrgFile, O_DF_SET_FNAME, pname );
    if( p_send2( Drg, O_DG_READ_FILE ) )
    {
        self->tpopen.success = TRUE;
        return TRUE;
    }
    return FALSE;
}

/***************************************************************************
 **  deltmp  Class to delete temporary files.
 **  ~~~~~~
 */

METHOD INT deltmp_ls_filename( PR_DELTMP* self, UBYTE* pname )
{
    p_delete( (TEXT*) pname );
    return FALSE;
}

/* End of VecText.c file */
