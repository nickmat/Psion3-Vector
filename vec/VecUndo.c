/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: UNDO AND REDO CLASS MEMBERS      *  Date Started: 26 Feb 1997  *
 *    File: VECUNDO.C       Type: C SOURCE   *  Date Revised: 24 Jul 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <hwim.h>
#include <p_math.h>
#include <sa_.rsg>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"

static TEXT* UndoPath = "LOC::M:\\VEC\\TEMP\\undo.tmp";

/***************************************************************************
 **  ReadElement  Read the last saved undo command
 **  ~~~~~~~~~~~
 */

static INT ReadElement( VOID* pcb )
{
    INT ret;

    ret = p_read( pcb, DBuf, 1 );
    if( ret < 1 )
    {
        if( ret == 0 ) return E_FILE_EOF;
        return ret;
    }
    ret = p_read( pcb, &DBuf[TYPE_BYTE], DBuf[SIZE_BYTE]-1 );
    if( ret < DBuf[SIZE_BYTE] - 1 )
    {
        if( ret < 0 ) return ret;
        return E_FILE_EOF;
    }
    DBuf[FLAG_BYTE] &= ~LAYER_FLAGS;          /* Clear temp flags */
    return 0;
}

/***************************************************************************
 **  EDoUndo  Carry out the last saved command block. Returns 0 if completed
 **  ~~~~~~~  ok, else returns a negative error number.
 */

#pragma save, ENTER_CALL

static INT EDoUndo( PR_UNDO* self )
{
    LONG last;
    UNDOSIG sig;
    PR_UNDO* op;      /* Undo/Redo opposite number */
    UBYTE buf[EL_BUFSIZE];
    UINT hand2;
    UWORD DoSymTable, SymCount;
    UWORD DoFontTable;
    INT DoRedraw;
    INT msg;

    op = (PR_UNDO*) self->undo.op;
    op->undo.HoldRedo = TRUE;
    DoSymTable = FALSE;
    DoFontTable = FALSE;
    DoRedraw = FALSE;
    msg = SR_UNDO_DEFAULT;

    p_send3( op, O_UD_BLOCK_BEG, 0 );
    BegDraw();
    for(;;)
    {
        last = self->undo.next - self->undo.plen;
        p_seek( self->undo.pcb, P_FABS, &last );
        p_read( self->undo.pcb, &sig, sizeof(UNDOSIG) );
        if( sig.msg != 0 ) msg = sig.msg;
        p_send3( op, O_UD_SET_MSG, sig.msg );

        switch( sig.com )
        {
        case U_DELETE:
            p_send4( Data, O_DD_READ, sig.hand, DBuf );
            p_send3( Data, O_DD_DELETE, sig.hand );
            p_send4( op, O_UD_SAVE_INSERT, sig.hand, DBuf );
            DoRedraw = TRUE;
            break;
        case U_INSERT:
            if( ReadElement( self->undo.pcb ) != 0 ) break;
            p_send4( Data, O_DD_INSERT, sig.hand, DBuf );
            p_send3( Drg, O_DG_DRAW_EL, DBuf );
            p_send3( op, O_UD_SAVE_DELETE, sig.hand );
            break;
        case U_REPLACE:
            p_send4( Data, O_DD_READ, sig.hand, buf );
            if( ReadElement( self->undo.pcb ) != 0 ) break;
            p_send4( Data, O_DD_SAFE_REPLACE, sig.hand, DBuf );
            p_send4( op, O_UD_SAVE_REPLACE, sig.hand, buf );
            DoRedraw = TRUE;
            break;
        case U_MOVE:
            if( p_read( self->undo.pcb, &hand2, 2 ) < 2 ) break;
            p_send4( Data, O_DD_READ, hand2, DBuf );
            p_send3( Data, O_DD_DELETE, hand2 );
            p_send4( Data, O_DD_INSERT, sig.hand, DBuf );
            p_send4( op, O_UD_SAVE_MOVE, hand2, sig.hand );
            break;
        case U_ADD_SYM:
            DoSymTable = TRUE;
            SymCount = Drg->drg.SymbolCount + 1;
            p_send4( op, O_UD_SAVE, U_DEL_SYM, 0 );
            break;
        case U_DEL_SYM:
            DoSymTable = TRUE;
            SymCount = Drg->drg.SymbolCount - 1;
            p_send4( op, O_UD_SAVE, U_ADD_SYM, 0 );
        case U_FONT_CHAR:
            DoFontTable = TRUE;
            p_send4( op, O_UD_SAVE, U_FONT_CHAR, 0 );
            break;
        }
        self->undo.next = last;
        self->undo.plen = sig.plen;
        p_iow( self->undo.pcb, P_FSETEOF, &last );       /* Truncate file */

        if( ( sig.flags & UNDO_BLOCK ) != 0 )
        {
            break;                                           /* Exit loop */
        }
    }
    hInfoPrint( msg, self->undo.name );
    if( DoRedraw )
    {
        p_send2( Drg, O_DG_REDRAW );
    }
    EndDraw();
    p_send2( op, O_UD_BLOCK_END );
    op->undo.HoldRedo = FALSE;
    if( DoSymTable )
    {
        p_send3( Drg, O_DG_MAKE_SYM_TAB, SymCount );
    }
    if( DoFontTable )
    {
        p_send2( Drg, O_DG_MAKE_CHAR_TAB );
    }
    if( Drg->vec.UndoOff )
    {
        /* Something must be wrong with our opposite number */
        p_send2( self, O_UD_KILL );
    }
    return 0;
}

#pragma restore

/***************************************************************************
 **  DoUndo  Carry out the last saved command block
 **  ~~~~~~
 */

static VOID DoUndo( PR_UNDO* self )
{
    PR_UNDO* op;      /* Undo/Redo opposite number */

    if( p_enter2( EDoUndo, self ) )
    {
        /* An error, command block was not completed */
        KillUndo(); /* Kill both files */
        op = (PR_UNDO*) self->undo.op;
        op->undo.HoldRedo = FALSE;
        Drg->vec.UndoOff = TRUE;
        hInfoPrint( SRM_LO_MEM_UNDO_OFF );
    }
}

/***************************************************************************
 **  ESaveData  Save the undo command. Returns 0 if saved ok, or a negative
 **  ~~~~~~~~~  error value if a problem
 */

#pragma save, ENTER_CALL

static INT ESaveData( PR_UNDO* self, UNDOSIG* sig, VOID* data, UINT size )
{
    Drg->drg.ChangedFlag = TRUE;
    if( Drg->vec.UndoOff )
    {
        return 0;
    }
    if( self->undo.pcb == NULL )
    {
        p_scpy( self->undo.fn, UndoPath );
        hEnsurePath( self->undo.fn );
        f_open( &self->undo.pcb, self->undo.fn,
            P_FSTREAM | P_FUNIQUE | P_FRANDOM );
        self->undo.next = 0;
        self->undo.plen = 0;
    }
    f_seek( self->undo.pcb, P_FABS, &self->undo.next );

    sig->plen = self->undo.plen;
    if( self->undo.block && ! self->undo.first )
    {
        sig->flags = 0;
    }
    else
    {
        sig->flags = UNDO_BLOCK;
        self->undo.first = FALSE;
    }
    if( self->undo.mark )
    {
        sig->flags |= UNDO_MARK;
        self->undo.mark = FALSE;
    }
    sig->msg = self->undo.nextmsg;
    self->undo.nextmsg = 0;

    f_write( self->undo.pcb, sig, sizeof(UNDOSIG ) );
    if( data )
    {
        f_write( self->undo.pcb, data, size );
    }

    size += sizeof(UNDOSIG);
    self->undo.next += size;
    self->undo.plen = size;
    if( Redo->undo.pcb && ! self->undo.HoldRedo )
    {
        p_send2( Redo, O_UD_KILL );
    }
    return 0;
}

#pragma restore

/***************************************************************************
 **  SaveData  Save the undo command. Kills all undo files if there is a
 **  ~~~~~~~~  problem
 */

static VOID SaveData( PR_UNDO* self, UNDOSIG* sig, VOID* data, UINT size )
{
    PR_UNDO* op;      /* Undo/Redo opposite number */

    if( p_enter5( ESaveData, self, sig, data, size ) )
    {
        /* Save failed, so switch off Undo */
        p_send2( self, O_UD_KILL );
        /* What to do about our opposite number */
        op = (PR_UNDO*) self->undo.op;
        if( self->undo.HoldRedo == FALSE )
        {
            /* Not in the middle of an undo, so just kill it */
            p_send2( op, O_UD_KILL );
        }
        Drg->vec.UndoOff = TRUE;
        hInfoPrint( SRM_LO_MEM_UNDO_OFF );
    }
}

/***************************************************************************
 **  ud_init  Initialise as either Undo or Redo
 **  ~~~~~~~
 */

#pragma METHOD_CALL

METHOD VOID undo_ud_init( PR_UNDO* self, PR_ROOT* op, UINT rid )
{
    self->undo.op = op;
    hLoadResBuf( rid, self->undo.name );
}

/***************************************************************************
 **  ud_set_msg  Set a message for the next (non block) save entry.
 **  ~~~~~~~~~~
 */

METHOD VOID undo_ud_set_msg( PR_UNDO* self, INT msg )
{
    self->undo.nextmsg = msg;
}

/***************************************************************************
 **  ud_save  Record a command with a word of data
 **  ~~~~~~~
 */

METHOD VOID undo_ud_save( PR_UNDO* self, UINT command, UINT word )
{
    UNDOSIG sig;

    if( command == U_NULL )
    {
        return;
    }
    sig.com = command;
    sig.hand = word;
    SaveData( self, &sig, NULL, 0 );
}

/***************************************************************************
 **  ud_save_delete  Record handle to be deleted on undo
 **  ~~~~~~~~~~~~~~
 */

METHOD VOID undo_ud_save_delete( PR_UNDO* self, UINT hand )
{
    UNDOSIG sig;

    sig.com = U_DELETE;
    sig.hand = hand;
    SaveData( self, &sig, NULL, 0 );
}

/***************************************************************************
 **  ud_save_insert  Record handle and element to be inserted on undo
 **  ~~~~~~~~~~~~~~
 */

METHOD VOID undo_ud_save_insert( PR_UNDO* self, UINT hand, UBYTE* element )
{
    UNDOSIG sig;

    sig.com = U_INSERT;
    sig.hand = hand;
    SaveData( self, &sig, element, element[SIZE_BYTE] );
}

/***************************************************************************
 **  ud_save_replace  Record handle and element to be replaced on undo
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID undo_ud_save_replace( PR_UNDO* self, UINT hand, UBYTE* element )
{
    UNDOSIG sig;

    sig.com = U_REPLACE;
    sig.hand = hand;
    SaveData( self, &sig, element, element[SIZE_BYTE] );
}

/***************************************************************************
 **  ud_save_move  Record from and to handles to be moved on undo
 **  ~~~~~~~~~~~~
 */

METHOD VOID undo_ud_save_move( PR_UNDO* self, UINT hand1, UINT hand2 )
{
    UNDOSIG sig;

    sig.com = U_MOVE;
    sig.hand = hand1;
    SaveData( self, &sig, &hand2, sizeof(UINT) );
}

/***************************************************************************
 **  ud_undo  Carry out an undo instruction
 **  ~~~~~~~
 */

METHOD VOID undo_ud_undo( PR_UNDO* self )
{
    if( Drg->vec.UndoOff )
    {
        hInfoPrint( SR_UNDO_NOT_ACTIVE, self->undo.name );
        return;
    }
    if( self->undo.plen == 0 )
    {
        hInfoPrint( SR_NOTHING_TO_UNDO, self->undo.name );
        return;
    }
    DoUndo( self );
}

/***************************************************************************
 **  ud_kill  Close and delete the file
 **  ~~~~~~~
 */

METHOD VOID undo_ud_kill( PR_UNDO* self )
{
    if( self->undo.pcb )
    {
        p_close( self->undo.pcb );
        self->undo.pcb = NULL;
        self->undo.next = 0;
        self->undo.plen = 0;
        p_delete( self->undo.fn );
    }
    self->undo.block = FALSE;
    self->undo.first = FALSE;
}

/***************************************************************************
 **  ud_block_beg  Start of block with message required on playback
 **  ~~~~~~~~~~~~
 */

METHOD VOID undo_ud_block_beg( PR_UNDO* self, INT msg )
{
    self->undo.block = TRUE;
    self->undo.first = TRUE;
    self->undo.nextmsg = msg;
}

/***************************************************************************
 **  ud_block_end  Mark end of block.
 **  ~~~~~~~~~~~~
 */

METHOD VOID undo_ud_block_end( PR_UNDO* self )
{
    self->undo.block = FALSE;
    self->undo.first = FALSE;
}

/***************************************************************************
 **  ud_mark  Add a mark to the next record saved
 **  ~~~~~~~
 */

METHOD VOID undo_ud_mark( PR_UNDO* self )
{
    self->undo.mark = TRUE;
}

/***************************************************************************
 **  ud_roll_back  Remove the current block, or entries from last mark, from
 **  ~~~~~~~~~~~~  the undo file. Flag must be either UNDO_BLOCK or UNDO_MARK
 */

METHOD VOID undo_ud_roll_back( PR_UNDO* self, UINT flag )
{
    LONG last;
    UNDOSIG sig;

    if( Drg->vec.UndoOff )
    {
        return;
    }
    switch( flag )
    {
    case UNDO_BLOCK:
        if( self->undo.block == FALSE || self->undo.first == TRUE )
        {
            return;                           /* Not started on block yet */
        }
        self->undo.first = TRUE;          /* In case we restart the block */
        break;
    case UNDO_MARK:
        if( self->undo.mark == TRUE )
        {
            return;
        }
        self->undo.mark = TRUE;
    }
    for(;;)
    {
        last = self->undo.next - self->undo.plen;
        p_seek( self->undo.pcb, P_FABS, &last );
        p_read( self->undo.pcb, &sig, sizeof(UNDOSIG) );
        self->undo.next = last;
        self->undo.plen = sig.plen;
        if( ( sig.flags & flag ) != 0 ) break;
    }
    p_iow( self->undo.pcb, P_FSETEOF, &last );           /* Truncate file */
}

/* End of VecUndo.c file */
