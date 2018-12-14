/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: PROPERTY BUILD & DIALOG CLASSES  *  Date Started: 16 Apr 1997  *
 *    File: VECPROP.C       Type: C SOURCE   *  Date Revised:  1 May 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#include <hwim.h>
#include <vector.g>
#include <vecbld.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"

static VOID ShowAggrProperty( PR_EPROP* self, UINT hand );

static VOID UpdateLayer( UINT hand, ELEM* el, INT layer )
{
    INT layerflags;

    if( ( el->hdr.layer & ~LAYER_FLAGS ) != layer )
    {
        layerflags = el->hdr.layer & LAYER_FLAGS;
        p_send4( Undo, O_UD_SAVE_REPLACE, hand, el );
        el->hdr.layer = layer | layerflags;
        p_send4( Data, O_DD_REPLACE, hand, el );
    }
}

static VOID UpdateAggrLayer( ELEM* el, INT layer )
{
    INT i;
    UINT hand;

    for( i = el->grp.count ; i > 0 ; --i )
    {
        hand = p_send3( Data, O_DD_NEXT, el );
        if( hand == EL_EOD )
        {
            p_panic( PN_CORRUPT_DDATA );
        }
        if( el->hdr.type & V_AGGR_BIT )
        {
            i += el->grp.count;
        }
        else
        {
            UpdateLayer( hand, el, layer );
        }
    }
}

static VOID UpdateListLayer( PROP_ITEM* list, INT size, INT layer )
{
    INT i;

    for( i = 0 ; i < size ; i++ )
    {
        p_send4( Data, O_DD_READ, list[i].hand, DBuf );
        if( DBuf[TYPE_BYTE] & V_AGGR_BIT )
        {
            UpdateAggrLayer( (ELEM*) DBuf, layer );
        }
        else
        {
            UpdateLayer( list[i].hand, (ELEM*) DBuf, layer );
        }
    }
}

static VOID ShowProperty( PR_EPROP* self, UINT hand )
{
    ELEM* el;
    INT len;

    Drg->drg.PropChanged = FALSE;
    el = &self->build.el;
    p_send4( Data, O_DD_READ, hand, el );
    switch( el->hdr.type )
    {
    case V_LINE:
        BldLaunchDialog( el, LINE_PROP_DIALOG, C_PROPDLG );
        break;
    case V_POLYLINE:
    case V_POLYGON:
        BldLaunchDialog( el, PLINE_PROP_DIALOG, C_PLINPDLG );
        break;
    case V_CIRCLE:
        BldLaunchDialog( el, CIRCLE_PROP_DIALOG, C_CIRPDLG );
        break;
    case V_ARC:
        BldLaunchDialog( el, ARC_PROP_DIALOG, C_ARCPDLG );
        break;
    case V_3PT_ARC:
        BldLaunchDialog( el, ARC3PT_PROP_DIALOG, C_PROPDLG );
        break;
    case V_TEXT:
        BldLaunchDialog( el, TEXT_PROP_DIALOG, C_TEXTPDLG );
        break;
    case V_LINK:
        BldLaunchDialog( el, LINK_PROP_DIALOG, C_LINKPDLG );
        break;
    case V_GROUP:
    case V_CHARACTER:
    case V_SYMBOL:
        ShowAggrProperty( self, hand );
        break;
    }
    if( Drg->drg.PropChanged )
    {
        p_send4( Data, O_DD_READ, hand, DBuf );
        len = DBuf[SIZE_BYTE];
        if( p_bcmp( DBuf, len, el, len ) != 0 )
        {
            /* Really has changed */
            p_send4( Data, O_DD_REPLACE, hand, el );
            p_send4( Undo, O_UD_SAVE_REPLACE, hand, DBuf );
        }
    }
}

static VOID CheckLayer( PR_EPROP* self, INT layer )
{
    if( self->eprop.layer == -2 )
    {
        self->eprop.layer = layer;
    }
    else if( self->eprop.layer != layer )
    {
        self->eprop.layer = -1;
    }
}

static VOID SkipAggregate( PR_EPROP* self, ELEM* el )
{
    INT i;
    UINT h;

    for( i = el->grp.count ; i > 0 ; --i )
    {
        h = p_send3( Data, O_DD_NEXT, el );
        if( h == EL_EOD )
        {
            p_panic( PN_CORRUPT_DDATA );
        }
        if( el->hdr.type & V_AGGR_BIT )
        {
            i += el->grp.count;
        }
        else
        {
            CheckLayer( self, el->hdr.layer & ~LAYER_FLAGS );
        }
    }
}

static VOID AddToList( PR_EPROP* self, PROP_ITEM* item, UINT hand, ELEM* el )
{
    item->hand = hand;
    item->type = el->hdr.type;
    if( el->hdr.type & V_AGGR_BIT )
    {
        SkipAggregate( self, el );
    }
    else
    {
        CheckLayer( self, el->hdr.layer & ~LAYER_FLAGS );
    }
}

static PROP_ITEM* MakeSelItemList( PR_EPROP* self )
{
    INT i;
    UINT hand;
    PROP_ITEM* list;

    list = (PROP_ITEM*) f_alloc( self->select.Count * sizeof(PROP_ITEM) );
    Rewind();
    self->eprop.layer = -2;
    for( i = 0 ; i < self->select.Count ; i++ )
    {
        hand = p_send3( Drg, O_DG_NEXT_MARKED, DBuf );
        if( hand == EL_EOD ) break;
        AddToList( self, &list[i], hand, (ELEM*) DBuf );
    }
    return list;
}

static PROP_ITEM* MakeItemList( PR_EPROP* self, ELEM* el )
{
    INT i;
    UINT hand;
    PROP_ITEM* list;

    /* Assumes that the element has just been read, so next is ready */
    if( el->grp.count == 0 )
    {
        return NULL;
    }
    list = (PROP_ITEM*) f_alloc( el->grp.count * sizeof(PROP_ITEM) );
    self->eprop.layer = -2;
    for( i = 0 ; i < el->grp.count ; i++ )
    {
        hand = p_send3( Data, O_DD_NEXT, DBuf );
        if( hand == EL_EOD ) break;
        AddToList( self, &list[i], hand, (ELEM*) DBuf );
    }
    return list;
}

static VOID ShowAggrProperty( PR_EPROP* self, UINT hand )
{
    APROP_DATA apd;
    UINT item;
    INT dlgrid;
    INT dlgclass;

    apd.curitem = 0;
    for(;;)
    {
        if( hand == EL_EOD )
        {
            apd.itemlist = MakeSelItemList( self );
            apd.itemsize = self->select.Count;
            apd.elem = NULL;
            apd.type = -1;
            dlgrid = AGGR_PROP_DIALOG;
            dlgclass = C_GRPPDLG;
        }
        else
        {
            p_send4( Data, O_DD_READ, hand, &self->build.el );
            apd.itemlist = MakeItemList( self, &self->build.el );
            apd.itemsize = self->build.el.grp.count;
            apd.elem = &self->build.el;
            apd.type = self->build.el.grp.type;
            switch( apd.type )
            {
            case V_GROUP:
                dlgrid = AGGR_PROP_DIALOG;
                dlgclass = C_GRPPDLG;
                break;
            case V_CHARACTER:
                dlgrid = CHAR_PROP_DIALOG;
                dlgclass = C_CHARPDLG;
                break;
            case V_SYMBOL:
                dlgrid = SYM_PROP_DIALOG;
                dlgclass = C_SYMPDLG;
                break;
            default:
                return; /* Shouldn't be here then */
            }
        }
        apd.nlayer = apd.layer = self->eprop.layer;
        BldLaunchDialog( &apd, dlgrid, dlgclass );
        item = apd.itemlist[apd.curitem].hand;
        switch( apd.key )
        {
        case 'I': case 'i':
            ShowProperty( self, item );
            break;
        case W_KEY_RETURN: case 'A': case 'a':
            if( apd.nlayer != apd.layer )
            {
                UpdateListLayer( apd.itemlist, apd.itemsize, apd.nlayer );
            }
            /* Fall through */
        default:
            p_free( apd.itemlist );
            return;
        }
        p_free( apd.itemlist );
    }
}


#pragma METHOD_CALL

/***************************************************************************
 **  bld_init  Initiate a Select and Modify Property build. Based on select.
 **  ~~~~~~~~
 */

METHOD VOID eprop_bld_init( PR_EPROP* self )
{
    self->build.cmd = SR_PROPERTY;
    p_supersend2( self, O_BLD_INIT );
    self->select.step3 = SR_VIEW_PROPERTY;
}

METHOD VOID eprop_bld_step( PR_EPROP* self, AUNIT ax, AUNIT ay )
{
    UINT hand;

    switch( self->build.state )
    {
    case BLD_1ST:
    case BLD_2ND:
        SelectStep( (PR_SELECT*) self, ax, ay );
        break;
    case BLD_3RD:
        BegUndo( SR_MODIFY_PROPERTY_UNDONE );
        if( self->select.Count > 1 )
        {
            ShowAggrProperty( self, EL_EOD );
        }
        else
        {
            Rewind();
            hand = p_send3( Drg, O_DG_NEXT_MARKED, DBuf );
            ShowProperty( self, hand );
        }
        EndUndo();
        p_send2( self, O_BLD_CANCEL );
        break;
    }
}

/* End of VecProp.c file */
