/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: DDATA - DRAWING DATA STORE CLASS *  Date Started: 24 Sep 1996  *
 *    File: VECDATA.C       Type: C SOURCE   *  Date Revised: 31 May 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1996, 1997, Nick Matthews

  The following are implimented as #defines to sgbuf class members
  VOID dd_init( UINT len ); Initiate and set segment size
  UINT dd_size( VOID );     Size in bytes of current data within store
*/

#include <hwim.h>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"

#pragma METHOD_CALL

/***************************************************************************
 **  dd_reset  Delete all data
 **  ~~~~~~~~
 */

METHOD VOID ddata_dd_reset( PR_DDATA* self )
{
    p_send4( self, O_SB_DELETE, 0, self->sgbuf.nbytes );
    self->ddata.pos = 0;
    self->ddata.rcount = 0;
}

/***************************************************************************
 **  dd_rewind  Set current position to first record and return its position
 **  ~~~~~~~~~
 */

METHOD UINT ddata_dd_rewind( PR_DDATA* self )
{
    self->ddata.pos = 0;
    return 0;
}

/***************************************************************************
 **  dd_add  Add an element to the end of the store. Current position is not
 **  ~~~~~~  altered. Returns the position of the new record.
 **  Calls p_leave( E_GEN_NOMEMORY ) if insufficient memory.
 */

METHOD UINT ddata_dd_add( PR_DDATA* self, UBYTE* data )
{
    UINT cpos;

    cpos = self->sgbuf.nbytes;
    p_send5( self, O_SB_INSERT, self->sgbuf.nbytes, data, (UINT) data[0] );
    self->ddata.rcount++;
    return cpos;
}

/***************************************************************************
 **  dd_add_1st  Add an element to the beginning of the store. Current
 **  ~~~~~~~~~~  position is adjusted to allow for the additional data.
 **  Returns the position of the new record (which is always 0).
 **  Calls p_leave( E_GEN_NOMEMORY ) if insufficient memory.
 */

METHOD UINT ddata_dd_add_1st( PR_DDATA* self, UBYTE* data )
{
    p_send5( self, O_SB_INSERT, 0, data, (UINT) data[0] );
    self->ddata.rcount++;
    self->ddata.pos += (UINT) data[0];
    return 0;
}

/***************************************************************************
 **  dd_insert  Insert an element at the given position in the store.
 **  ~~~~~~~~~  Current position is set to the one following the inserted
 **  one. Calls p_leave( E_GEN_NOMEMORY ) if insufficient memory.
 */

METHOD VOID ddata_dd_insert( PR_DDATA* self, UINT pos, UBYTE* data )
{
    p_send5( self, O_SB_INSERT, pos, data, (UINT) data[0] );
    self->ddata.pos += (UINT) data[0];
    self->ddata.rcount++;
}

/***************************************************************************
 **  dd_replace  Replace an element at the given position in the store.
 **  ~~~~~~~~~~  Current position is set to the one following the replaced
 **  one. Calls p_leave( E_GEN_NOMEMORY ) if insufficient memory and the old
 **  element is deleted. Cannot fail if size of new element is <= old.
 */

METHOD VOID ddata_dd_replace( PR_DDATA* self, UINT pos, UBYTE* data )
{
    UBYTE len;

    p_send5( self, O_SB_EXTRACT, pos, &len, 1 );
    p_send4( self, O_SB_DELETE, pos, (UINT) len );
    --self->ddata.rcount;
    p_send5( self, O_SB_INSERT, pos, data, (UINT) data[0] );
    self->ddata.rcount++;
    self->ddata.pos = pos + (UINT) data[0];
}

/***************************************************************************
 **  dd_safe_replace  Replace an element at the given position in the store.
 **  ~~~~~~~~~~~~~~~  Current position is set to the one following the
 **  replaced one. Calls p_leave( E_GEN_NOMEMORY ) if insufficient memory
 **  but the old element is left in place. This function may fail even if
 **  the replacement element is smaller than the original.
 */

METHOD VOID ddata_dd_safe_replace( PR_DDATA* self, UINT pos, UBYTE* data )
{
    UBYTE len;

    p_send5( self, O_SB_INSERT, pos, data, (UINT) data[0] );
    self->ddata.pos = pos + (UINT) data[0];
    p_send5( self, O_SB_EXTRACT, pos, &len, 1 );
    p_send4( self, O_SB_DELETE, self->ddata.pos, (UINT) len );
}

/***************************************************************************
 **  dd_delete  Remove the given record. Current position set to the
 **  ~~~~~~~~~  following data element.
 */

METHOD VOID ddata_dd_delete( PR_DDATA* self, UINT pos )
{
    UBYTE len;

    p_send5( self, O_SB_EXTRACT, pos, &len, 1 );
    p_send4( self, O_SB_DELETE, pos, (UINT) len );
    --self->ddata.rcount;
    self->ddata.pos = pos;
}

/***************************************************************************
 **  dd_read  Copy the given record to the given buffer. Current position
 **  ~~~~~~~  set to the following data element.
 */

METHOD VOID ddata_dd_read( PR_DDATA* self, UINT pos, VOID* data )
{
    UBYTE len;

    p_send5( self, O_SB_EXTRACT, pos, &len, 1 );
    p_send5( self, O_SB_EXTRACT, pos, data, (UINT) len );
    self->ddata.pos = (UINT) len + pos;
}

/***************************************************************************
 **  dd_next  Copy the current record to the given buffer if not at end of
 **  ~~~~~~~  data store. Return position of retrieved element or EL_EOD.
 **  Current position set to the following data element.
 */

METHOD UINT ddata_dd_next( PR_DDATA* self, VOID* data )
{
    UINT cpos;
    UBYTE len;

    if( self->ddata.pos >= self->sgbuf.nbytes )
    {
        return EL_EOD;
    }
    cpos = self->ddata.pos;
    p_send5( self, O_SB_EXTRACT, self->ddata.pos, &len, 1 );
    p_send5( self, O_SB_EXTRACT, self->ddata.pos, data, (UINT) len );
    self->ddata.pos += (UINT) len;
    return cpos;
}

/***************************************************************************
 **  dd_count  Return the number of element records held.
 **  ~~~~~~~~
 */

METHOD UINT ddata_dd_count( PR_DDATA* self )
{
    return self->ddata.rcount;
}

/***************************************************************************
 **  dd_pos  Return the current record position.
 **  ~~~~~~
 */

METHOD UINT ddata_dd_pos( PR_DDATA* self )
{
    return self->ddata.pos;
}

/***************************************************************************
 **  dd_set_pos  Set the current record position.
 **  ~~~~~~~~~~
 */

METHOD VOID ddata_dd_set_pos( PR_DDATA* self, UINT pos )
{
    self->ddata.pos = pos;
}

/* End of VECDATA.C File */
