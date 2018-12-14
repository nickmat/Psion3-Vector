/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: BUILD SYMBOL CLASS MEMBERS       *  Date Started:  7 Feb 1997  *
 *    File: BLDSYM.C        Type: C SOURCE   *  Date Revised: 12 Nov 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997 - 1998, Nick Matthews

 See VecBuild.c file for base class members.
*/

#include <hwim.h>
#include <p_gen.h>
#include <vector.g>
#include <vecsym.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"

/***************************************************************************
 **  CreateSymWin  Load and run the Symbol select window. Returns 0 if
 **  ~~~~~~~~~~~~  successful or neg error number if not.
 */

static INT CreateSymWin( PR_NSYM* self )
{
    NSYM_DATA data;
    INT err;
    UINT quick;

    err = p_openlib( &self->nsym.dcb, DatCommandPtr );
    if( err < 0 )
    {
        return err;
    }
    err = p_loadfilelib( self->nsym.dcb, VECSYM_DYL, &self->nsym.dyl, TRUE );
    if( err < 0 )
    {
        p_close( self->nsym.dcb );
        return err;
    }
    self->nsym.symwin = p_newlibh( self->nsym.dyl, C_SYMW );
    if( self->nsym.symwin == NULL )
    {
        p_unloadlib( self->nsym.dyl );
        p_close( self->nsym.dcb );
        return E_GEN_NOMEMORY;
    }
    BegDrawClear();
    data.blackbm = p_send2( Drg, O_DG_GET_BLACK_BMID );
    data.greybm = p_send2( Drg, O_DG_GET_GREY_BMID );
    data.link = (EL_LINK*) &self->build.el;
    data.ret = &self->nsym.ret;
    quick = Drg->drg.Quick;
    Drg->drg.Quick = 0;
    err = p_entersend3( self->nsym.symwin, O_WN_INIT, &data );
    if( err == 0 )
    {
        p_send3( w_ws, O_WS_ADD_DIAL, self->nsym.symwin );
        p_send2( w_am, O_AM_START );
        /* Wait for Symbol Select to return */
        p_send3( w_ws, O_WS_REMOVE_DIAL, self->nsym.symwin );
        err = self->nsym.ret;
    }
    hDestroy( self->nsym.symwin );
    self->nsym.symwin = NULL;
    p_unloadlib( self->nsym.dyl );
    p_close( self->nsym.dcb );
    Drg->drg.Quick = quick;
    p_send2( Drg, O_DG_CLEAR_BITMAP );
    p_send3( Drg, O_DG_DRAW_ALL, NULL );
    EndDraw();
    return err;
}

/***************************************************************************
 **  SelectSymbol  Allow symbol selection and set next step. Returns 0 if
 **  ~~~~~~~~~~~~  successful or neg error number if not.
 */

static INT SelectSymbol( PR_NSYM* self )
{
    INT err;

    err = CreateSymWin( self );

    p_send3( w_ws->wserv.cli, O_DW_SET_DISABLE, FALSE );
    if( err != 0 )
    {
        if( err < 0 )
        {
            p_send4( Drg, O_DG_ALERT, ES_CANT_DISPLAY_SYM_WIN, err );
        }
        Drg->drg.StepID = SR_SELECT;
        self->build.state = BLD_1ST;
        return err;
    }
    Drg->drg.StepID = SR_POSITION;
    self->build.state = BLD_2ND;
    return 0;
}

/***************************************************************************
 **  CopyLink  Copy the supplied link from library to store
 **  ~~~~~~~~
 */

VOID CopyLink( EL_LINK* pLink )
{
    LIB* pLib;
    LONG pos;
    TE_SYM loc;
    EL_SYMBOLHDR sym;
    int i;

    pLib = &Drg->drg.LibList[ pLink->lib ];

    pos = pLib->symlist + pLink->sym * sizeof(TE_SYM);
    f_seek( pLib->pfcb, P_FABS, &pos );
    f_read( pLib->pfcb, &loc, sizeof(TE_SYM) );
    pos = pLib->data + loc;
    f_seek( pLib->pfcb, P_FABS, &pos );
    f_read( pLib->pfcb, DBuf, 1 );
    f_read( pLib->pfcb, &DBuf[1], DBuf[0]-1 );
    sym = *(EL_SYMBOLHDR*) DBuf;

    BegUndo( SR_SYMBOL_COPY_UNDONE );
    DBuf[SIZE_BYTE] = sizeof(ELGRP);
    DBuf[TYPE_BYTE] = V_GROUP;
    AddUndo( DBuf );

    BegDraw();
    for( i = sym.grp.count ; i > 0 ; --i )
    {
        f_read( pLib->pfcb, DBuf, 1 );
        f_read( pLib->pfcb, &DBuf[1], DBuf[0]-1 );
        if( DBuf[TYPE_BYTE] == V_GROUP )
        {
            i += DBuf[COUNT_BYTE];
        }
        else
        {
            p_send5( Drg, O_DG_ADJUST_LKEL, DBuf, pLink, &sym.hot );
            DBuf[LAYER_BYTE] = Drg->vec.Layer;
            p_send3( Drg, O_DG_DRAW_EL, DBuf );
        }
        AddUndo( DBuf );
    }
    EndDraw();
    EndUndo();
}

/*---------------------[ nsymbol - Symbol Link or Copy ]------------------*/

#pragma METHOD_CALL

/***************************************************************************
 **  nsym_bld_init  Put up Symbol selector window. If a symbol is not
 **  ~~~~~~~~~~~~~  selected (error or esc) then cancel build.
 */

METHOD VOID nsym_bld_init( PR_NSYM* self )
{
    self->build.cmd = Drg->drg.CmdID = SR_SYMBOL;
    self->build.step1 = SR_SELECT;
    Drg->drg.StepID = SR_SELECT;
    self->build.state = BLD_1ST;
    self->build.UndoMsg = SR_SYMBOL_LINK_UNDONE;
    p_send4( self, O_BLD_STEP, Drg->vec.Cur.x, Drg->vec.Cur.y );
}

/***************************************************************************
 **  bld_step  Carry out the build steps for creating a symbol link or copy.
 **  ~~~~~~~~
 */

METHOD VOID nsym_bld_step( PR_NSYM* self, AUNIT ax, AUNIT ay )
{
    EL_LINK* link;
    INT ret;

    link = (EL_LINK*) &self->build.el;
    switch( self->build.state )
    {
    case BLD_1ST:
        ret = SelectSymbol( self );
        if( ret < 0 )
        {
            p_send2( self, O_BLD_CANCEL );
            return;
        }
        if( ret > 0 )
        {
            return;
        }
        /* link header bytes are filled by SelectSymbol */
        link->hdr.layer = BAND_LAYER | NEW_LAYER;
        link->pos.x = ax;
        link->pos.y = ay;
        BegDraw();
        hDestroy( Band );
        p_send3( Drg, O_DG_CREATE_BAND, C_BAND );
        p_send5( Band, O_BND_INIT, link, &link->pos, NULL );
        EndDraw();
        Drg->drg.StepID = SR_POSITION;
        self->build.state = BLD_2ND;
        break;
    case BLD_2ND:
        link->pos.x = ax;
        link->pos.y = ay;
        if( Drg->drg.LibList[ Drg->vec.CurLib ].set.create == 0 )
        {
            /* Create Link */
            p_send2( self, O_BLD_SAVE );
            break;
        }
        /* Copy Symbol */
        CopyLink( link );
        p_send2( self, O_BLD_CANCEL );
    }
}

/* End of BLDSYM.C File */
