/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: DRAWING FILE CLASS MEMBERS       *  Date Started:  2 Sep 1996  *
 *    File: VECFILE.C       Type: C SOURCE   *  Date Revised: 19 Oct 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1996, 1997, Nick Matthews
*/

#include <hwim.h>
#include <p_math.h>
#include <sa_.rsg>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"

#define DrawingSig  "kcVectorDrawing"

/***************************************************************************
 **  GetHeader  Write the current header details to drgfile class buffer.
 **  ~~~~~~~~~
 */

static VOID GetHeader( PR_DRGFILE* self )
{
    p_scpy( self->drgfile.hdr.sig, DrawingSig );
    self->drgfile.hdr.version = FILE_VERSION;
    self->drgfile.hdr.hdrsize = sizeof(DRGHDR) + sizeof(PRS_VEC);
    self->drgfile.hdr.minver = VERSION_SET;
}

/***************************************************************************
 **  ReadDataDraw  Read, Draw and Store Data from current file position.
 **  ~~~~~~~~~~~~
 */

LOCAL_C VOID ReadDataDraw( PR_DRGFILE* self )
{
    ELEM* pEl;
    INT ret;

    pEl = (ELEM*) DBuf;
    for(;;)
    {
        ret = p_read( self->drgfile.pcb, DBuf, 1 );
        if( ret <= 0 ) break;
        if( DBuf[0] == '\0' ) break;  /* Ilegal element */
        f_read( self->drgfile.pcb, &DBuf[1], DBuf[0]-1 );

        switch( pEl->hdr.type )
        {
        case V_LINK:
            pEl->link.sym = p_send4( Drg, O_DG_GET_SYM_INDEX, pEl->link.lib, pEl->link.ref );
            break;
        case V_TEXT:
            if( self->drgfile.set < 101 )
            {
                /* Convert old text elements */
                p_bcpy( FBuf, DBuf, EL_BUFSIZE );
                p_bcpy( &DBuf[16], &FBuf[10], EL_BUFSIZE - 20 );
                DBuf[SIZE_BYTE] = FBuf[SIZE_BYTE] + 6;
            }
            if( self->drgfile.set < 103 )
            {
                SetTextRect( (EL_TEXT*) DBuf );
            }
            break;
        case V_LINE:
        case V_POLYLINE:
        case V_POLYGON:
        case V_CIRCLE:
        case V_3PT_ARC:
        case V_ARC:
        case V_DIM_HORIZ:
        case V_DIM_VERT:
        case V_DIM_ALIGN:
        case V_DIM_ANGLE:
        case V_DIM_RADIUS:
        case V_DIM_DIA:
        case V_GROUP:
        case V_SYMBOL:
        case V_CHARACTER:
            break;
        default:
            continue;   /* ignore what we don't know */
        }

        p_send3( Data, O_DD_ADD, DBuf );
    }
    p_send2( Drg, O_DG_REDRAW );
}

/***************************************************************************
 **  UpdateLayList  Update LayList from the new LayBuf
 **  ~~~~~~~~~~~~~
 */

LOCAL_C VOID UpdateLayList( UWORD size )
{
    TEXT* p;
    INT i;

    Drg->drg.LayBufSize = size;
    p = Drg->drg.LayBuf;
    i = 0;
    while( p < Drg->drg.LayBuf + size )
    {
        Drg->drg.LayStr[i] = p;
        i++;
        if( i == 16 ) break;
        while( p[0] ) p++;
        p++;
    }
    while( i < 16 )
    {
        Drg->drg.LayStr[i] = NULL;
        i++;
    }
}

/***************************************************************************
 **  SaveTable  Save the given table details to file
 **  ~~~~~~~~~
 */

static VOID SaveTable( VOID* pcb, INT type, VOID* data, INT size )
{
    TBHDR thdr;

    if( data == NULL ) return;
    if( size == 0 ) return;

    thdr.size = size;
    thdr.type = type;
    f_write( pcb, &thdr, sizeof(TBHDR) );
    f_write( pcb, data, size );
}

/***************************************************************************
 **  CloseDF  Close the current file.
 **  ~~~~~~~
 */

VOID CloseDF( PR_DRGFILE* self )
{
    p_close( self->drgfile.pcb );
    self->drgfile.pcb = NULL;
}

/***************************************************************************
 **  df_init  Initiate the one and only drgfile class
 **  ~~~~~~~
 */

#pragma METHOD_CALL

METHOD VOID drgfile_df_init( PR_DRGFILE* self )
{
    GetHeader( self );
}

/***************************************************************************
 **  df_set_fname  Set the file name buffer and determine the file type
 **  ~~~~~~~~~~~~
 */

METHOD TEXT* drgfile_df_set_fname( PR_DRGFILE* self, TEXT* fname )
{
    TEXT* fn;
    TEXT* ext;
    TEXT* name;
    P_FPARSE pcrk;

    fn = self->drgfile.fname;
    p_scpy( fn, fname );
    p_fparse( fn, NULL, fn, &pcrk );
    name = &fn[ pcrk.system + pcrk.device + pcrk.path ];
    ext = &name[ pcrk.name ];
    if( p_scmp( ext, ".vsl" ) == 0 )
    {
        Drg->drg.FileType = FTYPE_LIBRARY;
    }
    else if( p_scmp( ext, ".vft" ) == 0 )
    {
        Drg->drg.FileType = FTYPE_FONT;
    }
    else if( p_scmp( ext, ".vtp" ) == 0 )
    {
        Drg->drg.FileType = FTYPE_TEMPLATE;
    }
    else
    {
        Drg->drg.FileType = FTYPE_DRAWING;
    }

    self->drgfile.name = name;
    return self->drgfile.fname;
}

/***************************************************************************
 **  df_get_fname  Return a pointer to the full file name buffer
 **  ~~~~~~~~~~~~
 */

METHOD TEXT* drgfile_df_get_fname( PR_DRGFILE* self )
{
    return self->drgfile.fname;
}

/***************************************************************************
 **  df_get_name  Return a pointer to the short file name
 **  ~~~~~~~~~~~
 */

METHOD TEXT* drgfile_df_get_name( PR_DRGFILE* self )
{
    return self->drgfile.name;
}

/***************************************************************************
 **  df_kill  Close and delete the current file
 **  ~~~~~~~
 */

METHOD VOID drgfile_df_kill( PR_DRGFILE* self )
{
    CloseDF( self );
    p_delete( self->drgfile.fname );
}

/***************************************************************************
 **  df_save_file  Reopens the file for writing and writes the current
 **  ~~~~~~~~~~~~  header, tables and data.
 **  Currently, the old file is truncated. Returns 0 if ok, else an error.
 */

METHOD INT drgfile_df_save_file( PR_DRGFILE* self )
{
    UWORD cnt, i;
    TBHDR thdr;
    VOID* pcb;
    INT tcount;
    INT tsize;
    INT SymListSize;
    INT CharListSize;
    INT LayListSize;
    INT FontListSize;
    INT TextListSize;
    INT LibListSize;

    /* Update DataOffset and TableCount */
    tcount = 0;
    tsize = 0;

    SymListSize = sizeof(TBENT_SYM) * Drg->drg.SymbolCount;
    if( SymListSize )
    {
        tcount++;
        tsize += SymListSize;
    }

    CharListSize = Drg->drg.CharList ? sizeof(UINT) * 256 : 0;
    if( CharListSize )
    {
        tcount++;
        tsize += CharListSize;
    }

    LayListSize = Drg->drg.LayBufSize;
    if( LayListSize )
    {
        tcount++;
        tsize += LayListSize;
    }

    FontListSize = LNAME_MAX * Drg->drg.FontSize;
    tcount++;
    tsize += FontListSize;

    TextListSize = sizeof(TSTYLE) * Drg->drg.TextSize;
    tcount++;
    tsize += TextListSize;

    LibListSize = sizeof(TE_LIB) * Drg->drg.LibCount;
    if( LibListSize )
    {
        tcount++;
        tsize += LibListSize;
    }

    tsize += sizeof(TBHDR) * tcount;
    Drg->vec.DataOffset = sizeof(DRGHDR) + sizeof(PRS_VEC) + tsize;
    Drg->vec.TableCount = tcount;

    /* Open file */
    CloseDF( self );
    hEnsurePath( self->drgfile.fname );
    f_open( &self->drgfile.pcb, self->drgfile.fname,
        P_FSTREAM | P_FREPLACE | P_FUPDATE );
    pcb = self->drgfile.pcb;

    /* Save Header */
    GetHeader( self );
    f_write( pcb, &self->drgfile.hdr, sizeof(DRGHDR) );
    f_write( pcb, &Drg->vec, sizeof(PRS_VEC) );

    SaveTable( pcb, T_SYMBOLS, Drg->drg.SymbolList, SymListSize );
    SaveTable( pcb, T_CHAR_LIST, Drg->drg.CharList, CharListSize );
    SaveTable( pcb, T_LAYERS, Drg->drg.LayBuf, LayListSize );
    /* Save Font table, only the names */
    thdr.size = FontListSize;
    thdr.type = T_FONT;
    f_write( pcb, &thdr, sizeof(TBHDR) );
    for( i = 0 ; i < Drg->drg.FontSize ; i++ )
    {
        f_write( pcb, Drg->drg.FontList[i].fname, LNAME_MAX );
    }
    SaveTable( pcb, T_TEXT_STYLE, Drg->drg.TextList, TextListSize );
    if( LibListSize )
    {
        /* Save Lib table, only names and settings */
        thdr.size = LibListSize;
        thdr.type = T_LIBRARY;
        f_write( pcb, &thdr, sizeof(TBHDR) );
        for( i = 0 ; i < Drg->drg.LibCount ; i++ )
        {
            f_write( pcb, Drg->drg.LibList[i].fname, LNAME_MAX );
            f_write( pcb, &Drg->drg.LibList[i].set, sizeof(SYMSET) );
        }
    }

    /* Save Data */
    cnt = p_send2( Data, O_DD_COUNT );
    p_send2( Data, O_DD_REWIND );
    for( i = 0 ; i < cnt ; i++ )
    {
        p_send3( Data, O_DD_NEXT, DBuf );
        DBuf[LAYER_BYTE] &= ~LAYER_FLAGS;
        f_write( pcb, DBuf, DBuf[SIZE_BYTE] );
    }
    CloseDF( self );
    return 0;
}

/***************************************************************************
 **  df_check_header  Open and read the current file header. Returns TRUE if
 **  ~~~~~~~~~~~~~~~  valid vec file or FALSE if not. Leaves file positioned
 **  at end of header, ready to read rest of header.
 */

METHOD INT drgfile_df_check_header( PR_DRGFILE* self )
{
    DRGHDR hd;

    CloseDF( self );
    f_open( &self->drgfile.pcb, self->drgfile.fname,
        P_FSTREAM | P_FOPEN | P_FRANDOM );
    f_read( self->drgfile.pcb, &hd, sizeof(DRGHDR) );
    if( p_bcmp( hd.sig, 16, DrawingSig, 16 ) != 0 )
    {
        CloseDF( self );
        return FALSE;
    }
    if( hd.version != FILE_VERSION || hd.minver > VERSION_SET )
    {
        return FALSE;
    }
    self->drgfile.ver = hd.version;
    self->drgfile.set = hd.minver;
    return TRUE;
}

/***************************************************************************
 **  df_read_file  Read in and store the entire file details. Assumes the
 **  ~~~~~~~~~~~~  file is already open and checked.
 */

METHOD INT drgfile_df_read_file( PR_DRGFILE* self )
{
    DRGHDR hd;
    UWORD size;
    LONG pos = 0L;
    UWORD i, symcount;
    TBHDR thdr;
    VOID* pcb;
    INT j, fontsize;
    FONT* font;
    LIB* lib;

    pcb = self->drgfile.pcb;
    /* Read in header */
    f_seek( pcb, P_FABS, &pos );
    f_read( pcb, &hd, sizeof(DRGHDR) );

    /* Read in extended header */
    size = hd.hdrsize - sizeof(DRGHDR);
    size = MIN( sizeof(PRS_VEC), size );
    /* Reading in extented header change drawing */
    BegDraw();
    f_read( pcb, &Drg->vec, size );
    if( hd.minver < 101 )
    {
        for( i= 0 ; i < VDL_MAX ; i++ )
        {
            Drg->vec.DListMask[i] = 0xffff;
        }
    }
    if( hd.minver < 102 )
    {
        Drg->vec.PgDisp = 0x03;
        Drg->vec.Marg.pos.x = Drg->vec.Page.pos.x + 1000;
        Drg->vec.Marg.pos.y = Drg->vec.Page.pos.y + 1000;
        Drg->vec.Marg.lim.x = Drg->vec.Page.lim.x - 1000;
        Drg->vec.Marg.lim.y = Drg->vec.Page.lim.y - 1000;
    }
    if( hd.minver < 104 )
    {
        p_bfil( Drg->vec.Pref.unitname, UNIT_NAME_MAX_Z, 0 );
        hLoadChlistResBuf( SET_UNITS_LIST, Drg->vec.Units, Drg->vec.Pref.unitname );
        Drg->vec.Pref.scale.mul = 1;
        Drg->vec.Pref.scale.div = 1;
        Drg->vec.Pref.page.spare = 0;
        Drg->vec.Pref.page.papermul = 1;
        Drg->vec.Orig = Drg->vec.Page.pos;
        Drg->vec.yDir = 0;
        Drg->vec.spare1 = 0;
        Drg->vec.spare2 = 0;
    }
    p_send2( Drg, O_DG_SET_VIEW );
    EndDraw();
    p_send2( Drg, O_DG_SET_DLIST );

    /* Set file to first table */
    pos = hd.hdrsize;
    f_seek( pcb, P_FABS, &pos );

    symcount = 0;

    for( i = 0 ; i < Drg->vec.TableCount ; i++ )
    {
        f_read( pcb, &thdr, sizeof(TBHDR) );
        switch( thdr.type )
        {
        case T_LAYERS:
            Drg->drg.LayBuf = f_realloc( Drg->drg.LayBuf, thdr.size );
            f_read( pcb, Drg->drg.LayBuf, thdr.size );
            UpdateLayList( thdr.size );
            break;
        case T_FONT:
            if( thdr.size % LNAME_MAX ) p_leave( CORRUPT_FILE );
            size = thdr.size / LNAME_MAX;
            fontsize = sizeof(FONT) * size;
            font = (FONT*) f_realloc( Drg->drg.FontList, fontsize );
            p_bfil( font, fontsize, 0 );
            for( j = 0 ; j < size ; j++ )
            {
                f_read( pcb, font[j].fname, LNAME_MAX );
            }
            Drg->drg.FontList = font;
            Drg->drg.FontSize = size;
            break;
        case T_LIBRARY:
            if( thdr.size % sizeof(TE_LIB) ) p_leave( CORRUPT_FILE );
            size = thdr.size / sizeof(TE_LIB);
            lib = (LIB*) f_realloc( Drg->drg.LibList, sizeof(LIB) * size );
            p_bfil( lib, sizeof(LIB) * size, 0 );
            for( j = 0 ; j < size ; j++ )
            {
                f_read( pcb, lib[j].fname, LNAME_MAX );
                f_read( pcb, &lib[j].set, sizeof(SYMSET) );
            }
            Drg->drg.LibList = lib;
            Drg->drg.LibCount = size;
            break;
        case T_TEXT_STYLE:
            if( thdr.size % sizeof(TSTYLE) ) p_leave( CORRUPT_FILE );
            Drg->drg.TextList = f_realloc( Drg->drg.TextList, thdr.size );
            f_read( pcb, Drg->drg.TextList, thdr.size );
            Drg->drg.TextSize = thdr.size / sizeof(TSTYLE);
            break;
        case T_SYMBOLS:
            if( thdr.size % sizeof(TBENT_SYM) ) p_leave( CORRUPT_FILE );
            /* don't bother reading it in, regenerate it */
            symcount = thdr.size / sizeof(TBENT_SYM);
            /* Fall through */
        case T_CHAR_LIST:
            /* Fall through. Character table is generated */
        default:
            /* Skip anything we don't understand */
            pos = thdr.size;
            f_seek( pcb, P_FCUR, &pos );
        }
    }

    ReadDataDraw( self );

    /* Regerate or clear symbol table */
    p_send2( Drg, O_DG_MAKE_CHAR_TAB );
    p_send3( Drg, O_DG_MAKE_SYM_TAB, symcount );
    return 0;
}

/* End of VECFILE.C */
