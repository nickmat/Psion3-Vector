/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: BUILD AGGREGATE CLASS MEMBERS    *  Date Started:  3 Mar 1997  *
 *    File: VECAGGR.C       Type: C SOURCE   *  Date Revised:  2 Jun 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <hwim.h>
#include <p_math.h>
#include <vector.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"

/***************************************************************************
 **  GetSymbolIndex  Return the internal Symbol Table index of the symbol
 **  ~~~~~~~~~~~~~~  matching the given ref. Returns -1 if none found.
 */

static INT GetSymbolIndex( UWORD ref )
{
    INT i;
    EL_SYMBOL* pSym;

    pSym = (EL_SYMBOL*) DBuf;
    for( i = 0 ; i < Drg->drg.SymbolCount ; i++ )
    {
        p_send4( Data, O_DD_READ, Drg->drg.SymbolList[i], DBuf );
        if( pSym->ref == ref )
        {
            return i;
        }
    }
    return -1;
}

/***************************************************************************
 **  CreateAggregate  Create an Aggregate record by moving the members to
 **  ~~~~~~~~~~~~~~~  the end of store. If V_GROUP, group element is
 **  inserted. Returns the start of group position.
 */

static UINT CreateAggregate( PR_SELECT* self, INT element )
{
    EL_GROUP group;
    UINT ehandle, addhand;
    INT count;

    group.grp.size = sizeof(EL_GROUP);
    group.grp.type = V_NULL;
    group.grp.count = 0;
    group.grp.flags = BAND_LAYER;
    count = 0;

    p_bfil( &self->select.Rect, sizeof(ARECT), 0 );
    if( p_enter2( AddToStore, &group ) )
    {
        hInfoPrint( SRM_NO_MEMORY );
        return EL_EOD;
    }
    Rewind();
    for(;;)
    {
        ehandle = p_send3( Drg, O_DG_NEXT_MARKED, DBuf );
        if( ehandle == EL_EOD || DBuf[1] == V_NULL )
        {
            break;
        }
        /* send to end of store */
        p_send3( Data, O_DD_DELETE, ehandle );
        addhand = p_send3( Data, O_DD_ADD, DBuf );
        p_send4( Undo, O_UD_SAVE_MOVE, ehandle, addhand );
        if( element != V_GROUP )
        {
            p_send4( Drg, O_DG_UPDATE_BOUND, &self->select.Rect, DBuf );
        }
        if( DBuf[1] & V_GROUP )
        {
            count -= DBuf[2];
        }
        if( count == GROUP_MAX )
        {
            hInfoPrint( SRM_MAX_GRP_EL );
            break;
        }
        count++;
    }
    for(;;)
    {
        if( ehandle == EL_EOD )
        {
            /* we should leave loop by finding V_NULL */
            p_panic( PN_CORRUPT_DDATA );
        }
        if( DBuf[1] == V_NULL )
        {
            if( element != V_GROUP )
            {
                p_send3( Data, O_DD_DELETE, ehandle );
                p_send4( Undo, O_UD_SAVE_INSERT, ehandle, DBuf );
                self->build.el.grp.count = count;
            }
            else
            {
                group.grp.type = V_GROUP;
                group.grp.count = count;
                p_send4( Data, O_DD_REPLACE, ehandle, &group );
            }
            break; /* out of loop */
        }
        else
        {
            /* There must have been too many */
            /* Set a flag and give user a warning when we've done */
        }
        ehandle = p_send3( Drg, O_DG_NEXT_MARKED, DBuf );
    }
    return ehandle;
}

/*-----------------------------[ egroup - Group ]-------------------------*/

#pragma METHOD_CALL

/***************************************************************************
 **  bld_init  Initiate a Select & Group build. Based on select.
 **  ~~~~~~~~
 */

METHOD VOID egroup_bld_init( PR_EGROUP* self )
{
    self->build.cmd = SR_GROUP;
    p_supersend2( self, O_BLD_INIT );
    self->select.step3 = SR_GROUP_SEL;
}

/***************************************************************************
 **  bld_step  Step method for Group build
 **  ~~~~~~~~
 */

METHOD VOID egroup_bld_step( PR_EGROUP* self, AUNIT ax, AUNIT ay )
{
    switch( self->build.state )
    {
    case BLD_1ST:
    case BLD_2ND:
        SelectStep( (PR_SELECT*) self, ax, ay );
        break;
    case BLD_3RD:
        BegUndo( SR_GROUP_UNDONE );
        CreateAggregate( (PR_SELECT*) self, V_GROUP );
        EndUndo();
        p_send2( self, O_BLD_CANCEL );
        break;
    }
}

/*-------------------------[ ecsym - Create Symbol ]----------------------*/

/***************************************************************************
 **  bld_init  Initiate a Select & Create Symbol build. Based on select.
 **  ~~~~~~~~
 */

METHOD VOID ecsym_bld_init( PR_ECSYM* self )
{
    self->build.cmd = SR_CREATE_SYM;
    p_supersend2( self, O_BLD_INIT );
    self->select.step3 = SR_HOTSPOT_PT;
}

/***************************************************************************
 **  bld_step  Step method for Create Symbol build
 **  ~~~~~~~~
 */

METHOD VOID ecsym_bld_step( PR_ECSYM* self, AUNIT ax, AUNIT ay )
{
    UINT hand;

    switch( self->build.state )
    {
    case BLD_1ST:
    case BLD_2ND:
        SelectStep( (PR_SELECT*) self, ax, ay );
        break;
    case BLD_3RD:
        BegUndo( SR_CREATE_SYMBOL_UNDONE );
        if( BldLaunchDialog( &self->build.el, CREATE_SYM_DIALOG, C_CSYMDLG ) )
        {
            self->build.el.sym.grp.size = SIZEOF_SYMBOLHDR;
            self->build.el.sym.grp.type = V_SYMBOL;
            self->build.el.sym.grp.flags = 0;
            self->build.el.sym.hot.x = ax;
            self->build.el.sym.hot.y = ay;
            hand = CreateAggregate( (PR_SELECT*) self, V_SYMBOL );
            if( hand != EL_EOD )
            {
                self->build.el.sym.bound = self->select.Rect;
                if( p_enter3( InsertInStore, hand, &self->build.el ) )
                {
                    hInfoPrint( SRM_NO_MEMORY );
                }
                else
                {
                    p_send3( Drg, O_DG_MAKE_SYM_TAB, Drg->drg.SymbolCount+1 );
                    p_send4( Undo, O_UD_SAVE, U_DEL_SYM, 0 );
                }
            }
        }
        else
        {
            p_send2( self, O_BLD_BACK );
            return;
        }
        EndUndo();
        p_send2( self, O_BLD_CANCEL );
        break;
    }
}

/*--------------------[ ecfont - Create Font character ]------------------*/

/***************************************************************************
 **  bld_init  Initiate a Select & Create Character build. Based on select.
 **  ~~~~~~~~
 */

METHOD VOID ecchar_bld_init( PR_ECCHAR* self )
{
    self->build.cmd = SR_CREATE_FONT;
    p_supersend2( self, O_BLD_INIT );
    self->select.step3 = SR_HOTSPOT_PT;
}

/***************************************************************************
 **  bld_step  Step method for Create Character build
 **  ~~~~~~~~
 */

METHOD VOID ecchar_bld_step( PR_ECCHAR* self, AUNIT ax, AUNIT ay )
{
    UINT hand;

    switch( self->build.state )
    {
    case BLD_1ST:
    case BLD_2ND:
        SelectStep( (PR_SELECT*) self, ax, ay );
        break;
    case BLD_3RD:
        self->build.el.ch.hot.x = ax;
        self->build.el.ch.hot.y = ay;
        Drg->drg.StepID = SR_SET_PITCH;
        self->build.state = BLD_4TH;
        break;
    case BLD_4TH:
        self->build.el.ch.pitch = ax - self->build.el.ch.hot.x;
        BegUndo( SR_CREATE_CHAR_UNDONE );
        if( BldLaunchDialog(
            &self->build.el, CREATE_FONT_CH_DIALOG, C_CFTCHDLG ) )
        {
            self->build.el.ch.grp.size = sizeof(EL_CHAR);
            self->build.el.ch.grp.type = V_CHARACTER;
            self->build.el.ch.grp.flags = 0;
            hand = CreateAggregate( (PR_SELECT*) self, V_CHARACTER );
            if( hand != EL_EOD )
            {
                self->build.el.ch.bound = self->select.Rect;
                if( p_enter3( InsertInStore, hand, &self->build.el ) )
                {
                    hInfoPrint( SRM_NO_MEMORY );
                }
                else
                {
                    p_send2( Drg, O_DG_MAKE_CHAR_TAB );
                    p_send4( Undo, O_UD_SAVE, U_FONT_CHAR, 0 );
                }
            }
        }
        else
        {
            EndUndo();
            p_send2( self, O_BLD_BACK );
            return;
        }
        EndUndo();
        p_send2( self, O_BLD_CANCEL );
        break;
    }
}

/***************************************************************************
 **  sel_filter  Filter to select only specified aggreate type
 **  ~~~~~~~~~~
 */

METHOD INT ecchar_sel_filter( PR_ECCHAR* self, ELEM* pEl )
{
    switch( pEl->hdr.type )
    {
    case V_GROUP:
    case V_CHARACTER:
    case V_SYMBOL:
    case V_LINK:
    case V_TEXT:
        return FALSE;
    }
    return TRUE;
}

/*----------------------------[ eungrp - Ungroup ]------------------------*/

/***************************************************************************
 **  bld_init  Initiate a Select & Ungroup build. Based on select.
 **  ~~~~~~~~
 */

METHOD VOID eungrp_bld_init( PR_EUNGRP* self )
{
    if( self->build.cmd == 0 )
    {
        self->build.cmd = SR_UNGROUP;
    }
    p_supersend2( self, O_BLD_INIT );
    self->select.step3 = SR_UNGROUP_SEL;
    self->eungrp.AggrType = V_GROUP;
    self->build.UndoMsg = SR_UNGROUP_UNDONE;
    self->eungrp.UndoCom = U_NULL;
}

/***************************************************************************
 **  bld_step  Step method for Ungroup of all aggregate types build
 **  ~~~~~~~~
 */

METHOD VOID eungrp_bld_step( PR_EUNGRP* self, AUNIT ax, AUNIT ay )
{
    UINT ehandle;
    INT i;

    switch( self->build.state )
    {
    case BLD_1ST:
    case BLD_2ND:
        SelectStep( (PR_SELECT*) self, ax, ay );
        break;
    case BLD_3RD:
        /* Remove the group markers */
        Rewind();
        BegUndo( self->build.UndoMsg );
        p_send4( Undo, O_UD_SAVE, self->eungrp.UndoCom, 0 );
        for(;;)
        {
            ehandle = p_send3( Drg, O_DG_NEXT_MARKED, DBuf );
            if( ehandle == EL_EOD ) break;
            if( DBuf[TYPE_BYTE] & V_AGGR_BIT )
            {
                p_send3( Data, O_DD_DELETE, ehandle );
                p_send4( Undo, O_UD_SAVE_INSERT, ehandle, DBuf );
                for( i = DBuf[COUNT_BYTE] ; i > 0 ; --i )
                {
                    ehandle = p_send3( Data, O_DD_NEXT, DBuf );
                    if( DBuf[TYPE_BYTE] == V_GROUP )
                    {
                        i += DBuf[COUNT_BYTE];
                    }
                }
            }
        }
        EndUndo();
        if( self->eungrp.AggrType == V_SYMBOL )
        {
            p_send3( Drg, O_DG_MAKE_SYM_TAB, Drg->drg.SymbolCount-1 );
        }
        if( self->eungrp.AggrType == V_CHARACTER )
        {
            p_send2( Drg, O_DG_MAKE_CHAR_TAB );
        }
        p_send2( self, O_BLD_CANCEL );
        break;
    }
}

/***************************************************************************
 **  sel_filter  Filter to select only specified aggreate type
 **  ~~~~~~~~~~
 */

METHOD INT eungrp_sel_filter( PR_EUNGRP* self, ELEM* pEl )
{
    if( pEl->hdr.type == self->eungrp.AggrType )
    {
        return TRUE;
    }
    return FALSE;
}

/*------------------------[ eunsym - Ungroup Symbol ]---------------------*/

/***************************************************************************
 **  bld_init  Initiate a Select & Ungroup symbol build. Based on eungrp.
 **  ~~~~~~~~
 */

METHOD VOID eunsym_bld_init( PR_EUNSYM* self )
{
    self->build.cmd = SR_UNGROUP_SYM;
    p_supersend2( self, O_BLD_INIT );
    self->select.step3 = SR_UNGROUP_SEL;
    self->eungrp.AggrType = V_SYMBOL;
    self->build.UndoMsg = SR_UNGROUP_SYMBOL_UNDONE;
    self->eungrp.UndoCom = U_DEL_SYM;
}

/*----------------------[ eunchar - Ungroup Character ]-------------------*/

/***************************************************************************
 **  bld_init  Initiate a Select & Ungroup character build. Based on eungrp.
 **  ~~~~~~~~
 */

METHOD VOID eunchar_bld_init( PR_EUNCHAR* self )
{
    self->build.cmd = SR_UNGROUP_CHAR;
    p_supersend2( self, O_BLD_INIT );
    self->select.step3 = SR_UNGROUP_SEL;
    self->eungrp.AggrType = V_CHARACTER;
    self->build.UndoMsg = SR_UNGROUP_CHAR_UNDONE;
    self->eungrp.UndoCom = U_FONT_CHAR;
}

/*-------------------------[ Create Symbol Dialog ]-----------------------*/

/***************************************************************************
 **  dl_dyn_init  Initiate Create Symbol Dialog
 **  ~~~~~~~~~~~
 */

METHOD VOID csymdlg_dl_dyn_init( PR_CSYMDLG* self )
{
    hDlgSetNcedit( 2, Drg->vec.NewSym + 10 );
}

/***************************************************************************
 **  dl_key  Complete Create Symbol Dialog
 **  ~~~~~~
 */

METHOD INT csymdlg_dl_key( PR_CSYMDLG* self, INT index, INT keycode,
    INT actbut )
{
    EL_SYMBOL* sym;
    INT ind;
    UINT hand;

    /* We only get here if Enter is pressed */
    sym = self->dlgbox.rbuf;
    p_scpy( (TEXT*) sym->name, hDlgSenseEdwin( 1 ) );
    sym->ref = hDlgSenseNcedit( 2 );
    ind = GetSymbolIndex( sym->ref );

    if( ind != -1 )
    {
        if( hConfirm( SR_REPLACE_EXIST_SYM, sym->ref ) )
        {
            hand = Drg->drg.SymbolList[ind];
            p_send4( Data, O_DD_READ, hand, DBuf );
            p_send3( Data, O_DD_DELETE, hand );
            p_send4( Undo, O_UD_SAVE_INSERT, hand, DBuf );
        }
        else
        {
            return WN_KEY_NO_CHANGE;
        }
    }
    if( sym->ref > Drg->vec.NewSym )
    {
        Drg->vec.NewSym = sym->ref;
    }

    return WN_KEY_CHANGED;
}

/*---------------------[ Create Font Character Dialog ]-------------------*/

/***************************************************************************
 **  dl_dyn_init  Initiate Create Character Dialog
 **  ~~~~~~~~~~~
 */

METHOD VOID cftchdlg_dl_dyn_init( PR_CFTCHDLG* self )
{
    hDlgSetNcedit( 1, Drg->vec.LastChar );
}

/***************************************************************************
 **  dl_key  Complete Create Character Dialog
 **  ~~~~~~
 */

METHOD INT cftchdlg_dl_key( PR_CFTCHDLG* self, INT index, INT keycode,
    INT actbut )
{
    EL_CHAR* pch;
    UINT hand;

    /* We only get here if Enter is pressed */
    pch = self->dlgbox.rbuf;
    pch->ref = hDlgSenseNcedit( 1 );
    if( Drg->drg.CharList )
    {
        hand = Drg->drg.CharList[pch->ref];
        if( hand != 0xffff )
        {
            if( hConfirm( SR_REPLACE_EXIST_CHAR, pch->ref ) )
            {
                p_send4( Data, O_DD_READ, hand, DBuf );
                p_send3( Data, O_DD_DELETE, hand );
                p_send4( Undo, O_UD_SAVE_INSERT, hand, DBuf );
            }
            else
            {
                return WN_KEY_NO_CHANGE;
            }
        }
    }

    Drg->vec.LastChar = pch->ref + 1;

    return WN_KEY_CHANGED;
}

/* End of VECAGGR.C File */
