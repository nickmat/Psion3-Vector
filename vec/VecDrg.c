/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: VECDRG DRAWING CLASS MEMBERS     *  Date Started:  2 Sep 1996  *
 *    File: VECDRG.C        Type: C SOURCE   *  Date Revised: 13 May 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1996 - 1998, Nick Matthews
*/

#include <p_math.h>
#include <hwim.h>
#include <p_config.h>
#include <p_gen.h>
#include <sa_.rsg>
#include <vector.g>
#include <vecsym.g>
#include <vector.rsg>
#include "vecbld.h"
#include "vector.h"
#include "vecdebug.h"

/*#define DBX5_Debug(A,B,C)    Debug( (A), (B), (C) )*/
#define DBX5_Debug(A,B,C)

static VOID Debug( INT n1, INT n2, TEXT* n3 )
{
    p_send5( Drg, O_DG_DEBUG, n1, n2, n3 );
}


/* Default Configuration settings */
static CONFIG CfgDefault =
{
    {   /* keys; Keyboard map A-Z, 0-9 */
        O_COM_JMP_ABS, O_COM_ZOOM_BOX, O_COM_NOT_ALLOC, O_COM_NOT_ALLOC,        /* A,B,C,D */
        O_COM_ZOOM_EXTENT, O_COM_NOT_ALLOC, O_COM_TOG_B_GRID, O_COM_NOT_ALLOC,  /* E,F,G,H */
        O_COM_TOG_INFO, O_COM_NOT_ALLOC, O_COM_NOT_ALLOC, O_COM_JMP_LAST_CART,  /* I,J,K,L */
        O_COM_NOT_ALLOC, O_COM_NOT_ALLOC, O_COM_NOT_ALLOC, O_COM_JMP_CUR_POLAR, /* M,N,O,P */
        O_COM_TOG_QUICK, O_COM_JMP_CUR_CART, O_COM_TOG_SNAP, O_COM_NOT_ALLOC,   /* Q,R,S,T */
        O_COM_NOT_ALLOC, O_COM_NOT_ALLOC, O_COM_NOT_ALLOC, O_COM_ZOOM_OUT,      /* U,V,W,X */
        O_COM_NOT_ALLOC, O_COM_ZOOM_IN, O_COM_NOT_ALLOC, O_COM_NOT_ALLOC,       /* Y,Z.0,1 */
        O_COM_NOT_ALLOC, O_COM_NOT_ALLOC, O_COM_NOT_ALLOC, O_COM_NOT_ALLOC,     /* 2,3,4,5 */
        O_COM_NOT_ALLOC, O_COM_NOT_ALLOC, O_COM_NOT_ALLOC, O_COM_NOT_ALLOC      /* 6,7,8,9 */
    },
    {   /* move; movement keys */
        0, 2, 4, 3, 10, 1
    },
    {   /* cursor; Cursor style */
        CURSIZE_SMALL,
        CURCOLOR_BLACK,
        CURWIDTH_THIN,
        CURSELBOX_OFF
    },
    {   /* prn; Printer settings */
        0, 0,
        { P_BAUD_9600, P_BAUD_9600, P_DATA_8, 0, 0, 0x11, 0x13, P_IGNORE_PARITY, 0L },
        "LOC::M:\\VECTOR.PRN"
    }
};

static PIC_HEAD PicHeader =
{
    { "PIC", 'P'+'I'+'C', 0x30, 0x30 },            /* sig */
    1,                                             /* count */
    { 0, { 0, 0 }, 0, 0L }                         /* wspic */
};

/* Default settings */
static PRS_VEC VecDefault =
{
    0, 0,     /* DataOffset, TableCount */
    { { 17918U, 22268U }, { 47618U, 43268U } }, /* Page */
    { { 21484U, 27994U }, { 44052U, 37542U } }, /* Scrn */
    { 32918U, 32268U },                         /* Cur */
    0,                                          /* yDir */
    0,                                          /* spare1 */
    2,                                          /* DispDp */
    0,                                          /* spare2 */

    /*1000,*/                                       /* DScale */
    /*2, 100,*/                                     /* DispDp, DispMult */

    UNITS_MM,                                   /* Units */
    0,                                          /* Attr */
    0xffff, 0,                                  /* LayMask, Layer */
    125,                                        /* ZoomPercent */
    VW_CMD | VW_INFO | VW_SCROLL,               /* View */
    VDL_FIRST | VDL_SECOND | VDL_THIRD | VDL_FOURTH, /* DiamondList */
    VDL_FIRST,                                  /* DiamondView */
    0,                                          /* Text */
    {
        { { 21484U, 27994U }, { 44052U, 37542U } }, /* DListScrn[0] */
        { { 21484U, 27994U }, { 44052U, 37542U } }, /* DListScrn[1] */
        { { 21484U, 27994U }, { 44052U, 37542U } }, /* DListScrn[2] */
        { { 21484U, 27994U }, { 44052U, 37542U } }  /* DListScrn[3] */
    },
    {
        { 32918U, 32268U },                     /* DListCur[0] */
        { 32918U, 32268U },                     /* DListCur[1] */
        { 32918U, 32268U },                     /* DListCur[2] */
        { 32918U, 32268U }                      /* DListCur[3] */
    },
    { 62, 62, 62, 62 },                         /* DListUpp[0-3] */
    { 0xffff, 0xffff, 0xffff, 0xffff },         /* DListMask[0-3] */
    { 0, 0, 0, 0 },                             /* DListLay[0-3] */
    { 17918U, 22268U },                         /* Orig = Page.pos */
    62,                                         /* upp */
    33,                                         /* LastChar */
    COM_ACT_COPY | COM_ACT_MIRROR,              /* MoveCopy */
    FALSE,                                      /* UndoOff */
    0x03,                                       /* PgDisp */
    {                                           /* Pref */
        /*0, 0, 0,*/                                /* Pref.spare1,2,3 */
        { 'm', 'm', 0, 0, 0, 0 },               /* Pref.unitname */
        {                                       /* Pref.grid */
            500,                                /* Pref.grid.space */
            GRID_SNAP | GRID_BLACK,             /* Pref.grid.flags */
            1, 5,                               /* minor, major */
            0x30                                /* Pref.grid.showlist */
        },
        { 1, 1 },                               /* Pref.scale */
        { 0, 1, 1, 1 }                          /* Pref.page */
    },
    { { 18918U, 23268U }, { 46618U, 42268U } }, /* Marg */
    0,                                          /* NewText */
    0,                                          /* OffGrid */
    3000,                                       /* Nearest */
    0, 0,                                       /* CurLib, CurSym */
    2,                                          /* SelectBox */
    90                                          /* NewSym */
};

/****************************************************************************
 **  qsSymOrder  Quick sort order function.
 **  ~~~~~~~~~~
 */

typedef INT (*ORDER)();

static INT qsSymOrder( INT n, INT m, TBENT_SYM* list )
{
    UWORD nref;

    p_send4( Data, O_DD_READ, list[n], DBuf );
    nref = ( (EL_SYMBOL*) DBuf )->ref;
    p_send4( Data, O_DD_READ, list[m], DBuf );
    return nref - ( (EL_SYMBOL*) DBuf )->ref;
}

/****************************************************************************
 **  qsSymOrder  Quick sort exchange function.
 **  ~~~~~~~~~~
 */

typedef VOID (*EXCH)();

static VOID qsSymExch( INT n, INT m, TBENT_SYM* list )
{
    TBENT_SYM tmp;

    tmp = list[n];
    list[n] = list[m];
    list[m] = tmp;
}

/***************************************************************************
 **  IsElementPtInRect  Return TRUE if any point in the element is inside
 **  ~~~~~~~~~~~~~~~~~  the given rectangle, else return FALSE.
 */

static UINT IsElementPtInRect( ARECT* rect, UBYTE* el )
{
    INT i, size;
    A_PT* data;

    data = (A_PT*) &el[DATA_BYTE];

    switch( el[TYPE_BYTE] )
    {
    case V_POLYLINE:
    case V_POLYGON:
        size = DATASIZE( el );
        break;
    case V_CIRCLE:
    case V_ARC:
    case V_TEXT:
    case V_LINK:
        size = 1;
        break;
    case V_LINE:
        size = 2;
        break;
    case V_3PT_ARC:
        size = 3;
        break;
    default:
        return FALSE;
    }
    for( i = 0 ; i < size ; i++ )
    {
        if( PointInRect( &data[i], rect ) )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/***************************************************************************
 **  IsElementEnclosed  Return TRUE if all of the element is inside the
 **  ~~~~~~~~~~~~~~~~~  given rectangle, else return FALSE.
 */

LOCAL_C UINT IsElementEnclosed( const ARECT* rect, UBYTE* el )
{
    A_PT* data;
    INT size;
    int i;
    A_PT pt[4];
    AUNIT xc, yc, rad;

    data = (A_PT*) &el[DATA_BYTE];
    size = DATASIZE( el );

    switch( el[TYPE_BYTE] )
    {
    case V_ARC:
    case V_3PT_ARC:
        GetArcRect( (ARECT*) pt, (ELEM*) el );
        data = pt;
        size = 2;
        break;
    case V_CIRCLE:
        xc = data[0].x;
        yc = data[0].y;
        rad = data[1].x;
        pt[0].x = xc - rad;
        pt[0].y = yc - rad;
        pt[1].x = xc + rad;
        pt[1].y = yc + rad;
        data = pt;
        size = 2;
        break;
    case V_TEXT:
    case V_LINK:
        GetLinkRect( pt, (EL_LINK*) el );
        data = pt;
        size = 4;
        break;
    }
    for( i = 0 ; i < size ; i++ )
    {
        if( ! PointInRect( &data[i], rect ) )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/***************************************************************************
 **  AddRect  Set rect1 to the rectangle which just encloses rect2 andd rec3
 **  ~~~~~~~
 */

LOCAL_C VOID AddRect( ARECT* rect1, ARECT* rect2, ARECT* rect3 )
{
    rect3->pos.x = MIN( rect1->pos.x, rect2->pos.x );
    rect3->pos.y = MIN( rect1->pos.y, rect2->pos.y );
    rect3->lim.x = MAX( rect1->lim.x, rect2->lim.x );
    rect3->lim.y = MAX( rect1->lim.y, rect2->lim.y );
}

/***************************************************************************
 **  AdjustUnitToPage   Ensure that the unit is positioned on the current
 **  ~~~~~~~~~~~~~~~~   page
 */

static AUNIT AdjustUnitToPage( AUNIT au, AUNIT pos, AUNIT lim, AUNIT orig )
{
    AUNIT spc;

    spc = Drg->vec.Pref.grid.space;
    if( spc )
    {
        orig %= spc;
    }
    --lim;
    if( au < pos )
    {
        if( IsSnapOn() )
        {
            au = ( ( ( pos - orig - 1 ) / spc ) + 1 ) * spc + orig;
        }
        else
        {
            au = pos;
        }
    }
    if( au > lim )
    {
        if( IsSnapOn() )
        {
            au = ( ( lim - orig ) / spc ) * spc + orig;
        }
        else
        {
            au = lim;
        }
    }
    return au;
}

/***************************************************************************
 **  SnapUnitToGrid   Move unit to the nearest grid posiotion
 **  ~~~~~~~~~~~~~~
 */

static AUNIT SnapUnitToGrid( AUNIT au, AUNIT orig )
{
    AUNIT da, nda;
    AUNIT spc;

    spc = Drg->vec.Pref.grid.space;
    orig %= spc;
    da = au - orig;
    nda = ( da / spc ) * spc;
    if( da - nda > spc / 2 )
    {
        nda += spc;
    }
    return orig + nda;
}

/***************************************************************************
 **  AdjustCur  Return new Cursor position so that it is on the page and is
 **  ~~~~~~~~~  snapped to the grid if Snap is on.
 */

VOID AdjustCur( A_PT* newcur, A_PT* old )
{
    AUNIT au, orig;

    au = old->x;
    orig = Drg->vec.Orig.x;
    if( IsSnapOn() )
    {
        au = SnapUnitToGrid( au, orig );
    }
    newcur->x = AdjustUnitToPage( au, Drg->vec.Page.pos.x, Drg->vec.Page.lim.x, orig );

    au = old->y;
    orig = Drg->vec.Orig.y;
    if( IsSnapOn() )
    {
        au = SnapUnitToGrid( au, orig );
    }
    newcur->y = AdjustUnitToPage( au, Drg->vec.Page.pos.y, Drg->vec.Page.lim.y, orig );
}

/***************************************************************************
 **  AdjustScrn  From the proposed Scrn position posx and posy, set Drg Scrn
 **  ~~~~~~~~~~  and Clip variables.
 */

VOID AdjustScrn( AUNIT posx, AUNIT posy )
{
    P_EXTENT extent;
    SWORD upp;
    SUNIT spos, slim;
    SUNIT sppos, splim;

    p_send3( w_ws->wserv.cli, O_DW_GET_SIZE, &extent );
    upp = Drg->vec.upp;

    spos = posx / upp;
    sppos = Drg->vec.Page.pos.x / upp;
    splim = Drg->vec.Page.lim.x / upp + 1;

    if( spos + extent.width > splim )
    {
        spos = splim - extent.width;
    }
    if( splim - sppos < extent.width )
    {
        /* Width of page is less than screen size */
        spos = sppos;
        slim = splim;
    }
    else
    {
        if( spos < sppos )
        {
            spos = sppos;
        }
        slim = spos + extent.width;
    }
    Drg->vec.Scrn.pos.x = spos * upp;
    Drg->vec.Scrn.lim.x = slim * upp;
    Drg->drg.Clip.pos.x = 0;
    Drg->drg.Clip.lim.x = slim - spos;

    spos = posy / upp;
    sppos = Drg->vec.Page.pos.y / upp;
    splim = Drg->vec.Page.lim.y / upp + 1;

    if( spos + extent.height > splim )
    {
        spos = splim - extent.height;
    }
    if( splim - sppos < extent.height )
    {
        /* Height of page is less than screen size */
        spos = sppos;
        slim = splim;
    }
    else
    {
        if( spos < sppos )
        {
            spos = sppos;
        }
        slim = spos + extent.height;
    }
    Drg->vec.Scrn.pos.y = spos * upp;
    Drg->vec.Scrn.lim.y = slim * upp;
    Drg->drg.Clip.pos.y = 0;
    Drg->drg.Clip.lim.y = slim - spos;
    Drg->drg.pClip = &Drg->drg.Clip;
}

/***************************************************************************
 **  MoveScrnToCur  Adjust the current screen so that the cursor is in view.
 **  ~~~~~~~~~~~~~  Only Drg->vec.Scrn is changed. If the cursor is already
 **  on the screen, nothing is changed.
 */

static VOID MoveScrnToCur( VOID )
{
    SWORD upp;
    AUNIT cu, au;
    SUNIT su;

    upp = Drg->vec.upp;

    cu = Drg->vec.Cur.x;
    su = cu / upp;
    au = 0;
    if( cu >= Drg->vec.Scrn.lim.x )
    {
        au = ( su + 1 ) * upp - Drg->vec.Scrn.lim.x;
    }
    else if( cu < Drg->vec.Scrn.pos.x )
    {
        au = su * upp - Drg->vec.Scrn.pos.x;
    }
    Drg->vec.Scrn.pos.x += au;
    Drg->vec.Scrn.lim.x += au;

    cu = Drg->vec.Cur.y;
    su = cu / upp;
    au = 0;
    if( cu >= Drg->vec.Scrn.lim.y )
    {
        au = ( su + 1 ) * upp - Drg->vec.Scrn.lim.y;
    }
    else if( cu < Drg->vec.Scrn.pos.y )
    {
        au = su * upp - Drg->vec.Scrn.pos.y;
    }
    Drg->vec.Scrn.pos.y += au;
    Drg->vec.Scrn.lim.y += au;
}

/***************************************************************************
 **  MoveCurToScrn  Adjust the current cursor so that it is in view.
 **  ~~~~~~~~~~~~~  Only Drg->vec.Cur is changed. If the cursor is already
 **  on the screen, nothing is changed.
 */

static AUNIT MoveCurToScrnUnit( AUNIT spos, AUNIT slim, AUNIT cu, AUNIT orig )
{
    if( cu >= slim )
    {
        cu = slim - 1;
        if( IsSnapOn() )
        {
            cu = SnapUnitToGrid( cu, orig );
            if( cu >= slim )
            {
                cu -= Drg->vec.Pref.grid.space;
            }
        }
    }
    if( cu < spos )
    {
        cu = spos;
        if( IsSnapOn() )
        {
            cu = SnapUnitToGrid( cu, orig );
            if( cu < spos )
            {
                cu += Drg->vec.Pref.grid.space;
            }
        }
    }
    return cu;
}

static VOID MoveCurToScrn( VOID )
{
    Drg->vec.Cur.x = MoveCurToScrnUnit(
        Drg->vec.Scrn.pos.x, Drg->vec.Scrn.lim.x, Drg->vec.Cur.x, Drg->vec.Orig.x );
    Drg->vec.Cur.y = MoveCurToScrnUnit(
        Drg->vec.Scrn.pos.y, Drg->vec.Scrn.lim.y, Drg->vec.Cur.y, Drg->vec.Orig.y );
    /* Just in case there is no grid point on the screen */
    MoveScrnToCur();
}

/***************************************************************************
 **  SetView  From the View flags, ensure that the view windows
 **  ~~~~~~~  are opened or closed if necessary and adjust to the new
 **  screen size. Note: Cur is not altered.
 */

static VOID SetView( VOID )
{
    SUNIT nsw, nsh;
    INT sstate;
    P_EXTENT ext;

    /* Start with Drawing Window full screen */
    nsw = wserv_channel->conn.info.pixels.x;
    nsh = wserv_channel->conn.info.pixels.y;

    /* System Status Window */
    if( Drg->vec.View & VW_SSTATUS )
    {
        sstate = W_STATUS_WINDOW_SMALL;
    }
    else if( Drg->vec.View & VW_LSTATUS )
    {
        sstate = W_STATUS_WINDOW_BIG;
    }
    else
    {
        sstate = W_STATUS_WINDOW_OFF;
    }
    wStatusWindow( sstate );
    wInquireStatusWindow( sstate, &ext );
    nsw -= ext.width;

    /* Infomation Window */
    if( Drg->vec.View & VW_INFO )
    {
        if( Drg->drg.IWin == NULL )
        {
            Drg->drg.IWin = f_new( CAT_VECTOR_VECTOR, C_VECIW );
            p_send3( Drg->drg.IWin, O_WN_INIT, nsw - VIW_WIDTH );
            hInitVis( Drg->drg.IWin );
        }
        else
        {
            p_send3( Drg->drg.IWin, O_IW_SET_POS, nsw - VIW_WIDTH );
        }
        nsw -= VIW_WIDTH;
    }
    else
    {
        hDestroy( Drg->drg.IWin );
        Drg->drg.IWin = NULL;
    }

    /* Right Scroll Window */
    if( Drg->vec.View & VW_VSCROLL )
    {
        if( Drg->drg.VSWin == NULL )
        {
            Drg->drg.VSWin = f_new( CAT_VECTOR_VECTOR, C_VECVSW );
            p_send4( Drg->drg.VSWin, O_WN_INIT, nsw - VSW_WIDTH, nsh );
            hInitVis( Drg->drg.VSWin );
        }
        else
        {
            p_send4( Drg->drg.VSWin, O_SW_SET_POS, nsw - VSW_WIDTH, nsh );
        }
        nsw -= VSW_WIDTH;
    }
    else
    {
        hDestroy( Drg->drg.VSWin );
        Drg->drg.VSWin = NULL;
    }

    /* Bottom Scroll Window */
    if( Drg->vec.View & VW_HSCROLL )
    {
        if( Drg->drg.HSWin == NULL )
        {
            Drg->drg.HSWin = f_new( CAT_VECTOR_VECTOR, C_VECHSW );
            p_send4( Drg->drg.HSWin, O_WN_INIT, nsh - VSW_WIDTH, nsw );
            hInitVis( Drg->drg.HSWin );
        }
        else
        {
            p_send4( Drg->drg.HSWin, O_SW_SET_POS, nsh - VSW_WIDTH, nsw );
        }
        nsh -= VSW_WIDTH;
    }
    else
    {
        hDestroy( Drg->drg.HSWin );
        Drg->drg.HSWin = NULL;
    }

    ext.tl.x = ext.tl.y = 0;
    ext.width = nsw;
    ext.height = nsh;
    p_send3( w_ws->wserv.cli, O_DW_SET_SIZE, &ext );

    AdjustScrn( Drg->vec.Scrn.pos.x, Drg->vec.Scrn.pos.y );
    MoveScrnToCur();
}

/***************************************************************************
 **  OpenAddFile4  Open the default font file bound to app as add file 4.
 **  ~~~~~~~~~~~~
 */

INT OpenAddFile4( FONT* pFont )
{
    ImgHeader head;
    LONG pos;
    UWORD offset, uword;
    UINT fhdr, ret;
    TBHDR thdr;

    f_open( &pFont->pfcb, DatCommandPtr, P_FRANDOM | P_FSTREAM | P_FSHARE );
    f_read( pFont->pfcb, &head, sizeof(ImgHeader) );
    offset = head.Add[3].offset;
    if( offset == 0 )
    {
        return FALSE;
    }
    pos = offset;
    f_seek( pFont->pfcb, P_FABS, &pos );
    fhdr = GetHeaderSize( pFont->pfcb );
    if( fhdr == 0 ) p_panic( 200 );
    f_read( pFont->pfcb, &uword, sizeof(UWORD) );
    pFont->data = offset + uword;
    ret = SeekTable( pFont->pfcb, offset, T_CHAR_LIST, &thdr );
    if( ret == 0 ) p_panic( 201 );
    pFont->chlist = ret;
    p_scpy( pFont->fname, "*Default" );
    return TRUE;
}

/***************************************************************************
 **  InitText  Initiate the font list with the default font
 **  ~~~~~~~~
 */

static VOID InitText( VOID )
{
    FONT* pFont;
    TSTYLE* pText;
    int i;

    /* Close any existing files */
    for( i = 0 ; i < Drg->drg.FontSize ; i++ )
    {
        p_close( Drg->drg.FontList[i].pfcb );
    }
    /* Default Font File entry */
    pFont = f_realloc( Drg->drg.FontList, sizeof(FONT) );
    Drg->drg.FontList = pFont;
    Drg->drg.FontSize = 1;
    OpenAddFile4( pFont );

    /* Default Text Style entry */
    pText = f_realloc( Drg->drg.TextList, sizeof(TSTYLE) );
    Drg->drg.TextList = pText;
    Drg->drg.TextSize = 1;
    Drg->vec.Text = 0;

    pText->font = 0;
    pText->units = UNITS_PT;
    pText->set.smul = 1;
    pText->set.sdiv = 5;
    pText->set.a.sin = 0;
    pText->set.a.cos = TRIG_DIV;
    p_scpy( (TEXT*) pText->name, "Default text" );
}

/***************************************************************************
 **  dg_move_cursor  Cursor is adjusted to be on the page. The current Scrn
 **  ~~~~~~~~~~~~~~  is assumed to be valid.
 */

#pragma METHOD_CALL

METHOD VOID vec_dg_move_cursor( PR_VEC* self, AUNIT ax, AUNIT ay )
{
    INT xs, ys;    /* Scroll distances */
    SRECT sr, rsr; /* Screen rect, Redraw rect */
    SUNIT xp, yp;
    INT redraw;

    DBX5_Debug( 200, 0, NULL );
    xs = ys = 0;
    p_send3( w_ws->wserv.cli, O_DW_GET_SRECT, &sr );
    rsr = sr;
    redraw = FALSE;

    DBX5_Debug( 201, 0, NULL );
    ax = AdjustUnitToPage( ax, self->vec.Page.pos.x, self->vec.Page.lim.x, self->vec.Orig.x );
    DBX5_Debug( 202, 0, NULL );
    ay = AdjustUnitToPage( ay, self->vec.Page.pos.y, self->vec.Page.lim.y, self->vec.Orig.y );

    DBX5_Debug( 210, 0, NULL );
    if( ax >= self->vec.Scrn.lim.x )
    {
        xs = ( ax / self->vec.upp + 1 ) * self->vec.upp - self->vec.Scrn.lim.x;
        xp = xs / self->vec.upp;
        if( sr.lim.x > xp )
        {
            rsr.pos.x = rsr.lim.x - xp;
        }
        else
        {
            redraw = TRUE;
        }
    }
    if( ay >= self->vec.Scrn.lim.y )
    {
        ys = ( ay / self->vec.upp + 1 ) * self->vec.upp - self->vec.Scrn.lim.y;
        yp = ys / self->vec.upp;
        if( sr.lim.y > yp )
        {
            rsr.pos.y = rsr.lim.y - yp;
        }
        else
        {
            redraw = TRUE;
        }
    }
    DBX5_Debug( 220, 0, NULL );
    if( ax < self->vec.Scrn.pos.x )
    {
        xs = ( ax / self->vec.upp ) * self->vec.upp - self->vec.Scrn.pos.x;
        xp = -xs / self->vec.upp;
        if( sr.lim.x > xp )
        {
            rsr.lim.x = xp;
        }
        else
        {
            redraw = TRUE;
        }
    }
    if( ay < self->vec.Scrn.pos.y )
    {
        ys = ( ay / self->vec.upp ) * self->vec.upp - self->vec.Scrn.pos.y;
        yp = -ys / self->vec.upp;
        if( sr.lim.y > yp )
        {
            rsr.lim.y = yp;
        }
        else
        {
            redraw = TRUE;
        }
    }

    DBX5_Debug( 230, 0, NULL );
    BegDraw();
    self->vec.Cur.x = ax;
    self->vec.Cur.y = ay;
    if( xs || ys )
    {
        if( xs && ys )
        {
            redraw = TRUE;
        }

        if( redraw )
        {
            kcClearBitmap( hBlackBM );
            kcClearBitmap( hGreyBM );
            rsr = sr;
        }
        else
        {
            xp = -xs / self->vec.upp;
            yp = -ys / self->vec.upp;
            kcScrollBitmap( hBlackBM, xp, yp );
            kcScrollBitmap( hGreyBM, xp, yp );
        }
        p_send2( w_ws->wserv.cli, O_WN_DODRAW );

        self->vec.Scrn.pos.x += xs;
        self->vec.Scrn.pos.y += ys;
        self->vec.Scrn.lim.x += xs;
        self->vec.Scrn.lim.y += ys;
        p_send3( Drg, O_DG_DRAW_ALL, &rsr );
        p_send3( Drg, O_DG_DRAW_GREY, &rsr );
    }
    EndDraw();
    DBX5_Debug( 299, 0, NULL );
}

/***************************************************************************
 **  dg_snap_to_grid  If snap is on, move cursor to a point on the grid.
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID vec_dg_snap_to_grid( PR_VEC* self )
{
    AUNIT ax, ay;

    if( IsSnapOn() )
    {
        ax = SnapUnitToGrid( self->vec.Cur.x, self->vec.Orig.x );
        ay = SnapUnitToGrid( self->vec.Cur.y, self->vec.Orig.y );
        vec_dg_move_cursor( self, ax, ay );
    }
}

/***************************************************************************
 **  dg_move_screen  Screen is adjusted to be on the page and cursor is
 **  ~~~~~~~~~~~~~~  moved to be in view.
 */

METHOD VOID vec_dg_move_screen( PR_VEC* self, AUNIT ax, AUNIT ay )
{
    BegDraw();
    AdjustScrn( ax, ay );
    MoveCurToScrn();
    vec_dg_snap_to_grid( self );
    p_send2( Drg, O_DG_REDRAW );
    EndDraw();
}

/***************************************************************************
 **  dg_set_zoom  Check and amend the upp zoom factor and redraw.
 **  ~~~~~~~~~~~
 */

METHOD VOID vec_dg_set_zoom( PR_VEC* self, SWORD upp )
{
    AUNIT cx, cy;
    SUNIT scurx, scury;

    upp = TestUpp( upp );
    if( upp == self->vec.upp )
    {
        return;
    }
    cx = self->vec.Cur.x;
    cy = self->vec.Cur.y;
    scurx = ( cx - self->vec.Scrn.pos.x ) / self->vec.upp;
    scury = ( cy - self->vec.Scrn.pos.y ) / self->vec.upp;

    BegDrawClear();

    self->vec.upp = upp;
    AdjustScrn( cx - scurx * upp, cy - scury * upp );
    p_send3( self, O_DG_DRAW_ALL, NULL );

    EndDraw();
}

/***************************************************************************
 **  dg_reset_data  Clear the current drawing and reset settings to program
 **  ~~~~~~~~~~~~~  defaults.
 */

METHOD VOID drg_dg_reset_data_( PR_DRG* self, INT ClrSettings )
{
    int i;

    BegDrawClear();
    hDestroy( Build );
    hDestroy( Build ); /* Twice to destroy a temp Build */
    hDestroy( Band );
    p_send2( Data, O_DD_RESET );
    p_send2( Undo, O_UD_KILL );
    p_send2( Redo, O_UD_KILL );
    if( ClrSettings )
    {
        self->vec = VecDefault;
    }

    InitText();      /* Set up default font and text style */
    self->drg.LastPt = self->vec.Cur;

    /* Clear Symbol and Char List */
    p_free( self->drg.CharList );
    self->drg.CharList = NULL;
    p_free( self->drg.SymbolList );
    self->drg.SymbolList = NULL;
    self->drg.SymbolCount = 0;

    /* Clear Library table, close files first */
    for( i = 0 ; i < self->drg.LibCount ; i++ )
    {
        p_close( self->drg.LibList[i].pfcb );
    }
    p_free( self->drg.LibList );
    self->drg.LibList = NULL;
    self->drg.LibCount = 0;

    /* Clear Layer names */
    p_free( self->drg.LayBuf );
    self->drg.LayBuf = NULL;
    p_bfil( self->drg.LayStr, sizeof(TEXT*) * 16, 0 );
    self->drg.LayBufSize = 0;

    self->drg.GridDisp = ( Drg->vec.Pref.grid.flags & GRID_GREY ) != 0;
    self->drg.GridAuto = 1;
    self->drg.Quick = 0;
    self->drg.ChangedFlag = 0;
    self->drg.CmdID = SR_VIEWING;
    self->drg.StepID = SR_BLANK;
    self->drg.ProcessOp = PO_NONE;

    UpdateDispScale();

    p_send2( self, O_DG_SET_DLIST );
    EndDraw();
    UpdateInfo( IW_ALL );
}

/***************************************************************************
 **  dg_init  One and only initialisation of the drg class.
 **  ~~~~~~~
 */

METHOD VOID drg_dg_init( PR_DRG* self )
{
    VOID* pcb;
    INT i;

    self->drg.DrgFile = f_new( CAT_VECTOR_VECTOR, C_DRGFILE );
    p_send2( self->drg.DrgFile, O_DF_INIT );
    p_send3( self->drg.DrgFile, O_DF_SET_FNAME, DatUsedPathNamePtr );

    /*p_bcpy( self->drg.Cfg.keys, KeyDefault, KEY_MAP_SIZE );*/
    self->drg.Cfg = CfgDefault;
    if( p_open( &pcb, CfgFileName, P_FSTREAM | P_FOPEN | P_FSHARE ) == 0 )
    {
        p_read( pcb, &i, sizeof(INT) );
        if( i == VERSION_CFG )
        {
            p_read( pcb, &self->drg.Cfg, sizeof(CONFIG) );
        }
        p_close( pcb );
    }

    drg_dg_reset_data_( self, DG_CLEAR_SETTINGS );
}

/***************************************************************************
 **  dg_update_cfg  Update section and save the configuration file.
 **  ~~~~~~~~~~~~~
 */

METHOD VOID drg_dg_update_cfg( PR_DRG* self, INT section )
{
    VOID* pcb;
    CONFIG cfg;
    INT ver;

    cfg = Drg->drg.Cfg;
    if( p_open( &pcb, CfgFileName, P_FSTREAM | P_FOPEN | P_FSHARE ) == 0 )
    {
        p_read( pcb, &ver, sizeof(INT) );
        if( ver == VERSION_CFG )
        {
            p_read( pcb, &cfg, sizeof(CONFIG) );
        }
        p_close( pcb );
    }

    ver = VERSION_CFG;
    switch( section )
    {
    case CFG_KEYS:
        p_bcpy( cfg.keys, Drg->drg.Cfg.keys, sizeof(UBYTE) * KEY_MAP_SIZE );
        break;
    case CFG_MOVE:
        cfg.move = Drg->drg.Cfg.move;
        break;
    case CFG_CURSOR:
        cfg.cursor = Drg->drg.Cfg.cursor;
        break;
    case CFG_PRN:
        cfg.prn = Drg->drg.Cfg.prn;
        break;
    default:
        return;
    }

    if( p_open( &pcb, CfgFileName, P_FSTREAM | P_FREPLACE | P_FUPDATE ) == 0 )
    {
        p_write( pcb, &ver, sizeof(INT) );
        p_write( pcb, &cfg, sizeof(CONFIG) );
        p_close( pcb );
    }
}

/***************************************************************************
 **  dg_set_fname  Set up a new filename and file type
 **  ~~~~~~~~~~~~
 */

METHOD VOID drg_dg_set_fname( PR_DRG* self, TEXT* fn )
{
    TEXT* ext;
    INT len;
    UINT ftype;

    p_send3( w_am, O_AM_NEW_FILENAME, fn );
    len = p_slen( fn );
    ext = fn + len - 4;
    if( len < 4 )
    {
        ftype = FTYPE_DRAWING;
    }
    else if( p_scmpi( ext, ".vsl" ) == 0 )
    {
        ftype = FTYPE_LIBRARY;
    }
    else if( p_scmpi( ext, ".vft" ) == 0 )
    {
        ftype = FTYPE_FONT;
    }
    else if( p_scmpi( ext, ".vtp" ) == 0 )
    {
        ftype = FTYPE_TEMPLATE;
    }
    else
    {
        ftype = FTYPE_DRAWING;
    }
    Drg->drg.FileType = ftype;
}

/***************************************************************************
 **  dg_read_file  Read the current (named in drg.DrgFile) file and update
 **  ~~~~~~~~~~~~  settings. If unable to read file, present alert and
 **  return FALSE, otherwise return TRUE.
 */

METHOD INT drg_dg_read_file( PR_DRG* self )
{
    INT msg, err;

    if( p_send2( self->drg.DrgFile, O_DF_CHECK_HEADER ) == FALSE )
    {
        msg = ES_NOT_VEC_FILE;
        err = E_GEN_FAIL;
    }
    else
    {
        err = p_entersend2( self->drg.DrgFile, O_DF_READ_FILE );
        if( err == CORRUPT_FILE )
        {
            msg = ES_CORRUPT_FILE;
            err = E_GEN_FAIL;
        }
        else
        {
            msg = ES_CANT_READ_FILE;
        }
    }
    if( err )
    {
        p_send4( Drg, O_DG_ALERT, msg, err );
        return FALSE;
    }
    self->drg.LastPt = self->vec.Cur;
    UpdateDispScale();
    return TRUE;
}

METHOD UINT drg_dg_next_marked( PR_DRG* self, UBYTE* buf )
{
    UINT ehand;

    for(;;)
    {
        ehand = p_send3( Data, O_DD_NEXT, buf );
        if( ehand == EL_EOD || buf[3] & BAND_LAYER )
        {
            return ehand;
        }
    }
}

/***************************************************************************
 **  dg_next_pt_inrect  Return the handle and data of the next element has a
 **  ~~~~~~~~~~~~~~~~~  point inside the given rectangle. If a member of a
 **  group is found the group handle and data is returned and next will be
 **  left indeterminate. (If the entries are to be marked, the marking
 **  function will reset next to the record following the group.)
 */

METHOD UINT drg_dg_next_pt_inrect( PR_DRG* self, ARECT* rect, UBYTE* pbuf )
{
    UINT handle, h;
    EL_GROUP grp;
    int i;

    for(;;)
    {
        handle = p_send3( Data, O_DD_NEXT, pbuf );
        if( handle == EL_EOD )
        {
            return EL_EOD;
        }
        if( pbuf[TYPE_BYTE] & V_AGGR_BIT )
        {
            grp = *(EL_GROUP*) pbuf;
            for( i = grp.grp.count ; i > 0 ; --i )
            {
                h = p_send3( Data, O_DD_NEXT, pbuf );
                if( h == EL_EOD )
                {
                    p_panic( PN_CORRUPT_DDATA );
                }
                if( pbuf[TYPE_BYTE] == V_GROUP )
                {
                    i += pbuf[COUNT_BYTE];
                    continue;
                }
                if( IsLayerOn( pbuf[LAYER_BYTE] ) == FALSE )
                {
                    continue;
                }
                if( IsElementPtInRect( rect, pbuf ) )
                {
                    p_send4( Data, O_DD_READ, handle, pbuf );
                    return handle;
                }
            }
            continue;
        }
        if( IsLayerOn( pbuf[LAYER_BYTE] ) == FALSE )
        {
            continue;
        }
        if( IsElementPtInRect( rect, pbuf ) )
        {
            return handle;
        }
    }
}

/***************************************************************************
 **  dg_next_inrect  Return the handle and data of the next element that is
 **  ~~~~~~~~~~~~~~  partly or wholly enclosed by the give rectangle. If a
 **  group record is found the group handle and data is returned and next
 **  will be left indeterminate. (If the entries are to be marked, the
 **  marking function will reset next to the record following the group.)
 */

METHOD UINT drg_dg_next_inrect( PR_DRG* self, ARECT* arect, UBYTE* pbuf )
{
    UINT handle, h;
    SRECT srect;
    EL_GROUP grp;
    int i;

    srect.pos.x = AtoSdim( arect->pos.x );
    srect.pos.y = AtoSdim( arect->pos.y );
    srect.lim.x = AtoSdim( arect->lim.x );
    srect.lim.y = AtoSdim( arect->lim.y );
    Drg->drg.psInRect = &srect;
    Drg->drg.paInRect = arect;
    Drg->drg.Primative = PRIM_ISINRECT;
    Drg->drg.sScrnPosX = 0;
    Drg->drg.sScrnPosY = 0;
    Drg->drg.IsInRect = 0;

    for(;;)
    {
        handle = p_send3( Data, O_DD_NEXT, pbuf );
        if( handle == EL_EOD )
        {
            break;
        }
        if( pbuf[TYPE_BYTE] & V_AGGR_BIT )
        {
            grp = *(EL_GROUP*) pbuf;
            for( i = grp.grp.count ; i > 0 ; --i )
            {
                h = p_send3( Data, O_DD_NEXT, pbuf );
                if( h == EL_EOD )
                {
                    p_panic( PN_CORRUPT_DDATA );
                }
                if( pbuf[TYPE_BYTE] == V_GROUP )
                {
                    i += pbuf[COUNT_BYTE];
                    continue;
                }
                if( IsLayerOn( pbuf[LAYER_BYTE] ) == FALSE )
                {
                    continue;
                }
                Draw( pbuf );
                if( Drg->drg.IsInRect )
                {
                    p_send4( Data, O_DD_READ, handle, pbuf );
                    break;
                }
            }
            if( Drg->drg.IsInRect )
            {
                break;
            }
            continue;
        }
        if( IsLayerOn( pbuf[LAYER_BYTE] ) == FALSE )
        {
            continue;
        }
        Draw( pbuf );
        if( Drg->drg.IsInRect )
        {
            break;
        }
    }
    Drg->drg.Primative = PRIM_DRAW;
    return handle;

#if 0
    UINT handle, h;
    SRECT srect;
    EL_GROUP grp;
    int i;

    srect.pos.x = AtoSdim( arect->pos.x );
    srect.pos.y = AtoSdim( arect->pos.y );
    srect.lim.x = AtoSdim( arect->lim.x );
    srect.lim.y = AtoSdim( arect->lim.y );

    for(;;)
    {
        handle = p_send3( Data, O_DD_NEXT, pbuf );
        if( handle == EL_EOD )
        {
            return EL_EOD;
        }
        if( pbuf[TYPE_BYTE] & V_AGGR_BIT )
        {
            grp = *(EL_GROUP*) pbuf;
            for( i = grp.grp.count ; i > 0 ; --i )
            {
                h = p_send3( Data, O_DD_NEXT, pbuf );
                if( h == EL_EOD )
                {
                    p_panic( PN_CORRUPT_DDATA );
                }
                if( pbuf[TYPE_BYTE] == V_GROUP )
                {
                    i += pbuf[COUNT_BYTE];
                    continue;
                }
                if( IsLayerOn( pbuf[LAYER_BYTE] ) == FALSE )
                {
                    continue;
                }
                if( IsElementInRect( &srect, pbuf ) )
                {
                    p_send4( Data, O_DD_READ, handle, pbuf );
                    return handle;
                }
            }
            continue;
        }
        if( IsLayerOn( pbuf[LAYER_BYTE] ) == FALSE )
        {
            continue;
        }
        if( IsElementInRect( &srect, pbuf ) )
        {
            return handle;
        }
    }
#endif
}

/***************************************************************************
 **  dg_next_enclosed  Return the handle and data of the next element that
 **  ~~~~~~~~~~~~~~~~  is fully enclosed by the give rectangle. If a group
 **  record is found the group handle and data is returned but next will be
 **  set to the element following the last group entry.
 */

METHOD UINT drg_dg_next_enclosed( PR_DRG* self, ARECT* rect, UBYTE* pbuf )
{
    ELHDR* hdr = (ELHDR*) pbuf;
    UINT handle, h, giveup;
    int i;
    EL_GROUP grp;

    for(;;)
    {
        handle = p_send3( Data, O_DD_NEXT, hdr );
        if( handle == EL_EOD )
        {
            return EL_EOD;
        }
        if( hdr->type & V_AGGR_BIT )
        {
            grp = *(EL_GROUP*) pbuf;
            giveup = FALSE;
            for( i = grp.grp.count ; i > 0 ; --i )
            {
                h = p_send3( Data, O_DD_NEXT, pbuf );
                if( h == EL_EOD )
                {
                    p_panic( PN_CORRUPT_DDATA );
                }
                if( hdr->type == V_GROUP )
                {
                    i += hdr->attr; /* hdr->attr == grp->count */
                    continue;
                }
                if( IsLayerOn( hdr->layer ) == FALSE )
                {
                    giveup = TRUE;
                    continue;
                }
                if( giveup == FALSE )
                {
                    if( ! IsElementEnclosed( rect, pbuf ) )
                    {
                        giveup = TRUE;
                    }
                }
            }
            if( giveup == FALSE )
            {
                p_send4( Data, O_DD_READ, handle, pbuf );
                return handle;
            }
            continue;
        }
        if( IsLayerOn( hdr->layer ) == FALSE )
        {
            continue;
        }
        if( IsElementEnclosed( rect, pbuf ) )
        {
            return handle;
        }
    }
}

/***************************************************************************
 **  dg_mark_el  Mark and display the elements as selected. Note this may
 **  ~~~~~~~~~~  overwrite the DBuf buffer, so if pdata uses DBuf, don't
 **  expect to use it again afterwards. If handle is a group, next data
 **  element will be set to the one following all the group elements.
 */

METHOD VOID drg_dg_mark_el( PR_DRG* self, UINT handle, UBYTE* pdata, INT as )
{
    UINT count;

    if( as == FALSE ) /* Just step thru aggregate without marking */
    {
        if( pdata[TYPE_BYTE] & V_AGGR_BIT )
        {
            count = pdata[COUNT_BYTE];
            while( count )
            {
                handle = p_send3( Data, O_DD_NEXT, DBuf );
                if( DBuf[TYPE_BYTE] == V_GROUP )
                {
                    count += DBuf[COUNT_BYTE];
                }
                --count;
            }
        }
        return;
    }
    if( pdata[TYPE_BYTE] & V_AGGR_BIT )
    {
        pdata[FLAG_BYTE] |= BAND_LAYER;
        p_send4( Data, O_DD_REPLACE, handle, pdata );
        count = pdata[COUNT_BYTE];
        while( count )
        {
            handle = p_send3( Data, O_DD_NEXT, DBuf );
            if( DBuf[TYPE_BYTE] == V_GROUP )
            {
                DBuf[FLAG_BYTE] |= BAND_LAYER;
                count += DBuf[COUNT_BYTE];
            }
            else
            {
                p_send3( self, O_DG_UNDRAW_EL, DBuf );
                DBuf[FLAG_BYTE] |= BAND_LAYER;
                p_send3( self, O_DG_DRAW_EL, DBuf );
            }
            p_send4( Data, O_DD_REPLACE, handle, DBuf );
            --count;
        }
    }
    else
    {
        p_send3( self, O_DG_UNDRAW_EL, pdata );
        pdata[LAYER_BYTE] |= BAND_LAYER;
        p_send3( self, O_DG_DRAW_EL, pdata );
        p_send4( Data, O_DD_REPLACE, handle, pdata );
    }
}

/***************************************************************************
 **  dg_make_char_tab  Create a new or refresh an existing font character
 **  ~~~~~~~~~~~~~~~~  table.
 */

METHOD VOID drg_dg_make_char_tab( PR_DRG* self )
{
    EL_CHAR* ch;
    UINT ehand;
    INT found = FALSE;

    ch = (EL_CHAR*) DBuf;

    if( self->drg.CharList == NULL )
    {
        self->drg.CharList = f_alloc( sizeof(UINT) * 256 );
    }
    p_bfil( self->drg.CharList, sizeof(UINT) * 256, 0xff );
    Rewind();
    for(;;)
    {
        ehand = p_send3( Data, O_DD_NEXT, DBuf );
        if( ehand == EL_EOD ) break; /* Done */
        if( DBuf[1] == V_CHARACTER )
        {
            self->drg.CharList[ ch->ref ] = ehand;
            found = TRUE;
        }
    }
    if( found == FALSE )
    {
        /* None found */
        p_free( self->drg.CharList );
        self->drg.CharList = NULL;
        return;
    }
}

/***************************************************************************
 **  dg_make_sym_tab  Create a new or refresh an existing symbol table.
 **  ~~~~~~~~~~~~~~~  Table is cleared if symcount is zero.
 */

METHOD VOID drg_dg_make_sym_tab( PR_DRG* self, UWORD symcount )
{
    UINT ehand;

    self->drg.SymbolCount = 0;
    if( symcount == 0 )
    {
        p_free( self->drg.SymbolList );
        self->drg.SymbolList = NULL;
        return;
    }
    self->drg.SymbolList = f_realloc(
        self->drg.SymbolList, sizeof(UINT) * symcount );
    Rewind();
    for(;;)
    {
        ehand = p_send3( Data, O_DD_NEXT, DBuf );
        if( ehand == EL_EOD ) break; /* Done */
        if( DBuf[1] == V_SYMBOL )
        {
            if( self->drg.SymbolCount == symcount )
            {
                symcount++;
                self->drg.SymbolList = f_realloc(
                    self->drg.SymbolList, sizeof(UINT) * symcount );
            }
            self->drg.SymbolList[ self->drg.SymbolCount++ ] = ehand;
        }
    }
    if( self->drg.SymbolCount == 0 )
    {
        /* None found */
        p_free( self->drg.SymbolList );
        self->drg.SymbolList = NULL;
        return;
    }
    p_qsort( self->drg.SymbolCount, (ORDER) qsSymOrder, (EXCH) qsSymExch, self->drg.SymbolList );
}

/***************************************************************************
 **  dg_update_bound  Update the bounding rect to include the new elem. If
 **  ~~~~~~~~~~~~~~~  rect is all zero's create a bounding rect from scratch.
 */

METHOD VOID drg_dg_update_bound( PR_DRG* self, ARECT* rect, UBYTE* elem )
{
    INT i, size;
    A_PT* pt;
    A_PT apt[4];
    AUNIT radius;

    pt = (A_PT*) &elem[ sizeof(ELHDR) ];
    switch( elem[1] )
    {
    case V_LINE:
    case V_POLYLINE:
    case V_POLYGON:
    case V_BOX:
        size = ( elem[0] - sizeof(ELHDR) ) / sizeof(A_PT);
        break;
    case V_ARC:
    case V_3PT_ARC:
        GetArcRect( (ARECT*) apt, (ELEM*) elem );
        pt = apt;
        size = 2;
        break;
    case V_CIRCLE:
        radius = pt[1].x;
        apt[0].x = pt[0].x - radius;
        apt[0].y = pt[0].y - radius;
        apt[1].x = pt[0].x + radius;
        apt[1].y = pt[0].y + radius;
        pt = apt;
        size = 2;
        break;
    case V_TEXT:
        GetTextRect( apt, (EL_TEXT*) elem );
        pt = apt;
        size = 4;
        break;
    case V_LINK:
        GetLinkRect( apt, (EL_LINK*) elem );
        pt = apt;
        size = 4;
        break;
    default:
        return;
    }
    for( i = 0 ; i < size ; i++ )
    {
        if( rect->lim.x == 0 && rect->lim.y == 0 )
        {
            rect->pos = rect->lim = pt[i];
        }
        else
        {
            AddPointToBound( rect, &pt[i] );
        }
    }
}

/***************************************************************************
 **  dg_process_el
 **  ~~~~~~~~~~~~~
 */

METHOD VOID drg_dg_process_el( PR_DRG* self, UBYTE* buf, UINT process )
{
    int i, size;
    A_PT* pt;
    EL_CHAR* ch;
    EL_TEXT* txt;
    EL_LINK* pLink;
    EL_ARC* arc;
    ANGLE ang;

    pt = (A_PT*) &buf[DATA_BYTE];
    Drg->drg.ProcessOp = process;

    switch( buf[TYPE_BYTE] )
    {
    case V_LINE:
    case V_POLYLINE:
    case V_POLYGON:
    case V_3PT_ARC:
        size = DATASIZE( buf );
        break;
    case V_ARC:
        arc = (EL_ARC*) buf;
        if( Drg->drg.ProcessOp == PO_MIRROR && Drg->drg.Divisor != 0 )
        {
            /* Swop beg and end */
            ang = arc->beg; arc->beg = arc->end; arc->end = ang;
        }
        ProcessAng( &arc->beg, &arc->beg );
        ProcessAng( &arc->end, &arc->end );
        /* Fall through */
    case V_CIRCLE:
        ProcessDim( &pt[1].x, &pt[1].x ); /* Radius */
        size = 1;
        break;
    case V_SYMBOL:
        /* process bound rect, hot point and any attach points */
        pt = (A_PT*) &buf[SIZEOF_SYMHDR];
        size = ( buf[SIZE_BYTE] - SIZEOF_SYMHDR ) / sizeof(A_PT);
        break;
    case V_CHARACTER:
        ch = (EL_CHAR*) buf;
        ProcessDim( &ch->pitch, &ch->pitch );
        pt = (A_PT*) &ch->bound;
        size = 3; /* bound rect and hot point */
        break;
    case V_TEXT:
        /* Process the adjustment settings */
        txt = (EL_TEXT*) buf;
        ProcessLink( &txt->set, &txt->set );
        size = 1; /* Process the position point */
        break;
    case V_LINK:
        /* Process the adjustment settings */
        pLink = (EL_LINK*) buf;
        ProcessLink( &pLink->set, &pLink->set );
        size = 1; /* Process the position point */
        break;
    case V_DIM_HORIZ:
        size = 3;
        break;
    default:
        /* Ignore all others */
        Drg->drg.ProcessOp = PO_NONE;
        return;
    }
    for( i = 0 ; i < size ; i++ )
    {
        ProcessPt( &pt[i], &pt[i] );
    }
    Drg->drg.ProcessOp = PO_NONE;
}

/***************************************************************************
 **  dg_clear_bitmap  Clear both black and grey bitmaps
 **  ~~~~~~~~~~~~~~~
 */

METHOD VOID drg_dg_clear_bitmap( PR_DRG* self )
{
    kcClearBitmap( hBlackBM );
    kcClearBitmap( hGreyBM );
}

/***************************************************************************
 **  dg_draw_el
 **  ~~~~~~~~~~
 */

METHOD VOID drg_dg_draw_el( PR_DRG* self, ELHDR* elem )
{
    if( elem->type & V_AGGR_BIT ) return;

    self->drg.DAttr.mode = DRAW_MODE_SET;
    self->drg.pClip = &self->drg.Clip;

    switch( IsLayerOn( elem->layer ) )
    {
    case BL_BLACK:
        self->drg.DAttr.bitmap = hBlackBM;
        break;
    case BL_GREY:
        self->drg.DAttr.bitmap = hGreyBM;
        break;
    default:
        return;
    }
    self->drg.sScrnPosX = AtoSdim( self->vec.Scrn.pos.x );
    self->drg.sScrnPosY = AtoSdim( self->vec.Scrn.pos.y );
    Draw( (UBYTE*) elem );
}

METHOD VOID drg_dg_undraw_el( PR_DRG* self, ELHDR* elem )
{
    if( elem->type & V_AGGR_BIT ) return;
    if( IsLayerOn( elem->layer ) != BL_BLACK ) return;

    self->drg.DAttr.mode = DRAW_MODE_CLR;
    self->drg.DAttr.bitmap = hBlackBM;
    self->drg.sScrnPosX = AtoSdim( self->vec.Scrn.pos.x );
    self->drg.sScrnPosY = AtoSdim( self->vec.Scrn.pos.y );
    self->drg.pClip = &self->drg.Clip;
    Draw( (UBYTE*) elem );
}

METHOD VOID drg_dg_draw_all( PR_DRG* self, SRECT* pRect )
{
    UWORD cnt, i;

    hBusyPrint( 2, SR_DRAWING );
    BegDraw();
    p_send2( Data, O_DD_REWIND );
    cnt = p_send2( Data, O_DD_COUNT );
    self->drg.sScrnPosX = AtoSdim( self->vec.Scrn.pos.x );
    self->drg.sScrnPosY = AtoSdim( self->vec.Scrn.pos.y );
    if( pRect )
    {
        self->drg.pClip = pRect;
    }
    else
    {
        self->drg.pClip = &self->drg.Clip;
    }
    self->drg.DAttr.mode = DRAW_MODE_SET;

    for( i = 0 ; i < cnt ; i++ )
    {
        p_send3( Data, O_DD_NEXT, DBuf );
        if( DBuf[TYPE_BYTE] & V_AGGR_BIT )
        {
            continue;
        }
        switch( IsLayerOn( DBuf[LAYER_BYTE] ) )
        {
        case BL_BLACK:
            self->drg.DAttr.bitmap = hBlackBM;
            break;
        case BL_GREY:
            self->drg.DAttr.bitmap = hGreyBM;
            break;
        default:
            continue;
        }
        Draw( DBuf );
    }
    self->drg.pClip = &self->drg.Clip;
    EndDraw();
    wCancelBusyMsg();
}

/***************************************************************************
 **  dg_draw_grey  Draw all grey elements. Only draw if no Band.
 **  ~~~~~~~~~~~~
 */

METHOD VOID drg_dg_draw_grey( PR_DRG* self, SRECT* pRect )
{
    if( Band ) return;

    if( self->vec.Pref.grid.flags & GRID_GREY )
    {
        DrawGrid( &self->vec.Pref.grid, pRect );
    }
}

/***************************************************************************
 **  dg_redraw  Clear the screen and redraw all elements
 **  ~~~~~~~~~
 */

METHOD VOID drg_dg_redraw( PR_DRG* self )
{
    BegDrawClear();
    self->drg.GridDisp = ( self->vec.Pref.grid.flags & GRID_GREY ) != 0;
    drg_dg_draw_all( self, NULL );
    EndDraw();
}

/***************************************************************************
 **  dg_get_layer_str  Copy the name for layer into the given buffer
 **  ~~~~~~~~~~~~~~~~
 */

METHOD VOID drg_dg_get_layer_str( PR_DRG* self, TEXT* buf, INT layer )
{
    TEXT* str;

    str = Drg->drg.LayStr[ layer ];
    if( str != NULL && str[0] != '\0' )
    {
        p_scpy( buf, str );
    }
    else
    {
        hAtos( buf, SR_LAYER_NAME_FMT, layer + 'A' );
    }
}

/***************************************************************************
 **  dg_set_view  Create, remove or resize windows to match the vec.View
 **  ~~~~~~~~~~~  flags.
 */

METHOD VOID drg_dg_set_view( PR_DRG* self )
{
    BegDraw();
    SetView();
    drg_dg_redraw( self );
    EndDraw();
}

/***************************************************************************
 **  dg_update_page  Update the Page and margin sizes, and adjust or values
 **  ~~~~~~~~~~~~~~  to fit new page. Should be called between Beg/EndDraw.
 */

METHOD VOID drg_dg_update_page( PR_DRG* self, AUNIT wd, AUNIT ht, AUNIT* mg )
{
    self->vec.Page.pos.x = ( AA - wd ) >> 1;
    self->vec.Page.lim.x = self->vec.Page.pos.x + wd;
    self->vec.Page.pos.y = ( AA - ht ) >> 1;
    self->vec.Page.lim.y = self->vec.Page.pos.y + ht;

    if( mg[MARGIN_LEFT] + mg[MARGIN_RIGHT] >= wd )
    {
        mg[MARGIN_LEFT] = mg[MARGIN_RIGHT] = wd/20;
    }
    if( mg[MARGIN_TOP] + mg[MARGIN_BOTTOM] >= ht )
    {
        mg[MARGIN_TOP] = mg[MARGIN_BOTTOM] = ht/20;
    }

    self->vec.Marg.pos.x = self->vec.Page.pos.x + mg[MARGIN_LEFT];
    self->vec.Marg.pos.y = self->vec.Page.pos.y + mg[MARGIN_TOP];
    self->vec.Marg.lim.x = self->vec.Page.lim.x - mg[MARGIN_RIGHT];
    self->vec.Marg.lim.y = self->vec.Page.lim.y - mg[MARGIN_BOTTOM];

    AdjustCur( &self->vec.Cur, &self->vec.Cur );
    AdjustScrn( self->vec.Scrn.pos.x, self->vec.Scrn.pos.y );
    MoveScrnToCur();
}

/***************************************************************************
 **  dg_set_page  Reset the Page and margin sizes, then redraw to suit
 **  ~~~~~~~~~~~
 */

METHOD VOID drg_dg_set_page( PR_DRG* self, AUNIT wd, AUNIT ht, AUNIT* mg )
{
    BegDraw();
    drg_dg_update_page( self, wd, ht, mg );
    drg_dg_redraw( self );
    EndDraw();
    UpdateInfo( IW_ALL );
}

/***************************************************************************
 **  dg_set_snap_grid  Update the drawing with the given settings
 **  ~~~~~~~~~~~~~~~~
 */

METHOD VOID drg_dg_set_snap_grid( PR_DRG* self, GRIDSET* pGrid )
{
    if( pGrid->space == 0 )
    {
        pGrid->flags = 0;
    }

    BegDraw();
    Drg->vec.Pref.grid = *pGrid;
    Drg->drg.GridDisp = ( pGrid->flags & GRID_GREY ) != 0;
    Drg->drg.GridAuto = 1;
    if( pGrid->flags & GRID_SNAP )
    {
        p_send2( Drg, O_DG_SNAP_TO_GRID );
    }
    EndDraw();

    UpdateInfo( IW_SNAP );
    UpdateInfo( IW_GRID );
}

METHOD VOID drg_dg_set_dlist( PR_DRG* self )
{
    TEXT list[VDL_MAX][10];
    TEXT* alist[VDL_MAX];
    int i;
    UBYTE flag = 1;
    UINT count = 0;
    UINT pos = W_STATUS_WIN_NO_DIAMOND;

    p_send4( w_am, O_AM_LOAD_RES_BUF, SR_FIRST, list[0] );
    p_send4( w_am, O_AM_LOAD_RES_BUF, SR_SECOND, list[1] );
    p_send4( w_am, O_AM_LOAD_RES_BUF, SR_THIRD, list[2] );
    p_send4( w_am, O_AM_LOAD_RES_BUF, SR_FOURTH, list[3] );

    for( i = 0 ; i < VDL_MAX ; i++ )
    {
        if( self->vec.DiamondList & ( flag << i ) )
        {
            alist[count] = list[i];
            if( self->vec.DiamondView & ( flag << i ) )
            {
                pos = count;
            }
            count++;
        }
    }
    if( count == 0 )
    {
        count = W_STATUS_WINDOW_ICON;
    }
    wsSetList( count, alist, pos );
}

/***************************************************************************
 **  dg_change_view  From the given view bit pattern (VDL_*) set up and
 **  ~~~~~~~~~~~~~~  display the new view.
 */

METHOD VOID drg_dg_change_view( PR_DRG* self, UINT view )
{
    UINT oldview, newview;

    if( Drg->vec.DiamondView == view ) return; /* Not changed */
    oldview = BitToIndex( Drg->vec.DiamondView );
    newview = BitToIndex( view );

    if( oldview >= VDL_MAX || newview >= VDL_MAX ) return;
    Drg->vec.DiamondView = view;
    self->vec.DListScrn[oldview] = self->vec.Scrn;
    self->vec.DListCur[oldview] = self->vec.Cur;
    self->vec.DListUpp[oldview] = self->vec.upp;
    self->vec.DListMask[oldview] = self->vec.LayMask;
    self->vec.DListLay[oldview] = self->vec.Layer;

    BegDrawClear();
    self->vec.Scrn = self->vec.DListScrn[newview];
    self->vec.Cur = self->vec.DListCur[newview];
    self->vec.upp = self->vec.DListUpp[newview];
    self->vec.LayMask = self->vec.DListMask[newview];
    self->vec.Layer = self->vec.DListLay[newview];

    if( IsSnapOn() )
    {
        vec_dg_snap_to_grid( (PR_VEC*) self );
    }

    AdjustScrn( Drg->vec.Scrn.pos.x, Drg->vec.Scrn.pos.y );
    MoveScrnToCur();
    drg_dg_redraw( self );
    EndDraw();

    UpdateInfo( IW_LAYER );
    drg_dg_set_dlist( self );
}

/***************************************************************************
 **  dg_zoom_to_box  Zoom screen to fit box.
 **  ~~~~~~~~~~~~~~
 */

METHOD VOID drg_dg_zoom_to_box( PR_DRG* self, ARECT* pRect )
{
    AUNIT dx, dy, temp;
    SWORD uppx, uppy, upp;
    P_EXTENT extent;

    NormaliseRect( pRect );
    dx = pRect->lim.x - pRect->pos.x;
    dy = pRect->lim.y - pRect->pos.y;
    GetDWinSize( &extent );

    BegDrawClear();

    uppx = 1 + dx / extent.width;
    uppy = 1 + dy / extent.height;
    if( uppx > uppy )
    {
        upp = uppx;
        temp = p_send5( self, O_DG_CENTRE_EDGE, extent.height, upp, dy );
        if( temp > pRect->pos.y ) temp = pRect->pos.y;
        pRect->pos.y -= temp;
    }
    else
    {
        upp = uppy;
        temp = p_send5( self, O_DG_CENTRE_EDGE, extent.width, upp, dx );
        if( temp > pRect->pos.x ) temp = pRect->pos.x;
        pRect->pos.x -= temp;
    }
    self->vec.upp = p_send3( self, O_DG_TEST_UPP, upp );
    AdjustScrn( pRect->pos.x, pRect->pos.y );
    MoveCurToScrn();
    p_send3( self, O_DG_DRAW_ALL, NULL );
    EndDraw();
}

/***************************************************************************
 **  dg_get_extent  Get the boundary of the current drawing
 **  ~~~~~~~~~~~~~
 */

METHOD VOID drg_dg_get_extent( PR_DRG* self, ARECT* pRect )
{
    p_bfil( pRect, sizeof(ARECT), 0 );
    p_send2( Data, O_DD_REWIND );
    while( p_send3( Data, O_DD_NEXT, DBuf ) != EL_EOD )
    {
        if( IsLayerOn( DBuf[LAYER_BYTE] ) )
        {
            p_send4( self, O_DG_UPDATE_BOUND, pRect, DBuf );
        }
    }
}

/***************************************************************************
 **  dg_get_font  Return the index for the given font name. If the font does
 **  ~~~~~~~~~~~  not exist, add it to the table. Return -1 if there is not
 **  sufficient memory.
 */

METHOD INT drg_dg_get_font( PR_DRG* self, TEXT* fontname )
{
    INT i;
    FONT* list;  /* The FontList */
    FONT* font;   /* The new Font entry */

    for( i = 0 ; i < self->drg.FontSize ; i++ )
    {
        if( p_scmpi( self->drg.FontList[i].fname, fontname ) == 0 )
        {
            return i;
        }
    }
    /* Not found, so add it. Note; i = self->drg.FontSize */
    list = p_realloc( self->drg.FontList, sizeof(FONT) * (i+1) );
    if( list == NULL )
    {
        /* Out of memory */
        return -1;
    }
    Drg->drg.FontList = list;
    Drg->drg.FontSize = i + 1;
    font = &list[i];
    p_bfil( font, sizeof(FONT), 0 );
    p_scpy( font->fname, fontname );
    return i;
}

/***************************************************************************
 **  dg_get_lib  Return the index for the given lib name. If the lib does
 **  ~~~~~~~~~~  not exist, add it to the table. Return -1 if there is not
 **  sufficient memory.
 */

METHOD INT drg_dg_get_lib( PR_DRG* self, TEXT* libname )
{
    LIB* list;    /* The LibList */
    LIB* pLib;   /* The new Lib entry */
    INT i;

    for( i = 0 ; i < self->drg.LibCount ; i++ )
    {
        if( p_scmpi( self->drg.LibList[i].fname, libname ) == 0 )
        {
            return i;
        }
    }
    /* Not found, so add it. Note; i = self->drg.LibCount */
    list = p_realloc( self->drg.LibList, sizeof(LIB) * (i+1) );
    if( list == NULL )
    {
        /* Out of memory */
        return -1;
    }
    self->drg.LibList = list;
    self->drg.LibCount = i + 1;
    pLib = &list[i];
    p_bfil( pLib, sizeof(LIB), 0 );
    p_scpy( pLib->fname, libname );
    pLib->set.scale.q = pLib->set.scale.d = 1;
    pLib->set.rotate.cos = TRIG_DIV;
    return i;
}

/***************************************************************************
 **  dg_open_lib  Open the library file for the supplied LIB struct. The
 **  ~~~~~~~~~~~  struct has only the lib name field, this function will
 **  complete the remaining fields.
 */

METHOD VOID drg_dg_open_lib( PR_DRG* self, LIB* pLib )
{
    PR_LKOPEN* lkopen;

    lkopen = f_new( CAT_VECTOR_VECTOR, C_LKOPEN );
    p_send3( lkopen, O_LKO_OPEN, pLib );
    hDestroy( lkopen );
}

/***************************************************************************
 **  dg_alert  Put up a safe error alert using preloaded error message
 **  ~~~~~~~~
 */

METHOD VOID drg_dg_alert( PR_DRG* self, INT errmess, INT errnum )
{
    TEXT errcode[3];

    errcode[0] = '\0';
    errcode[1] = 0xfe;
    errcode[2] = errnum;

    wsAlertW( WS_ALERT_CLIENT, ErrorStrings[errmess], errcode, NULL );
}

/***************************************************************************
 **  dg_discard_retry  Put up a safe error alert using preloaded error
 **  ~~~~~~~~~~~~~~~~  message. Returns 0=Cancel, 1=Retry
 */

METHOD INT drg_dg_cancel_retry( PR_DRG* self, INT errmess, INT errnum )
{
    TEXT errcode[3];

    errcode[0] = '\0';
    errcode[1] = 0xfe;
    errcode[2] = errnum;

    return wsAlertW( WS_ALERT_CLIENT, ErrorStrings[errmess], errcode,
        ErrorStrings[ES_CANCEL],
        ErrorStrings[ES_RETRY], NULL );
}

/* DYL helpers */

METHOD AUNIT drg_dg_su2au( PR_DRG* self, SUNIT su  )
{
    return StoAdim( su );
}

METHOD UINT drg_dg_hypot( PR_DRG* self, INT side1, INT side2 )
{
    return kcHypotenuse( side1, side2 );
}

METHOD UINT drg_dg_sq_root( PR_DRG* self, LONG* num )
{
    return kcSquareRoot( *num );
}

METHOD VOID drg_dg_adjust_scrn( PR_DRG* self, AUNIT posx, AUNIT posy )
{
    AdjustScrn( posx, posy );
}

METHOD SWORD drg_dg_test_upp( PR_DRG* self, SWORD upp )
{
    return TestUpp( upp );
}

METHOD UINT drg_dg_calc_intersect(
    PR_DRG* self, A_PT* result, A_PT* aline, A_PT* bline )
{
    return GetIntersect( result, aline, bline );
}

METHOD INT drg_dg_calc_arc_cntr( PR_DRG* self, A_PT* pPt, EL_3PT* p3PtArc )
{
    return CalcArcCentreAPt( pPt, p3PtArc );
}

METHOD INT drg_dg_calc_perp( PR_DRG* self, A_PT* result, A_PT* line, A_PT* pt )
{
    return CalcPerpendicularPt( result, line, pt, TRUE );
}

METHOD UINT drg_dg_break_circle( PR_DRG* self,
    ANGLE* aAng, EL_LARC* pLarc, EL_LINE* pLine )
{
    return GetLCircleSeg(
        &aAng[0], &aAng[1], &pLine->beg, &pLarc->centre, &pLarc->radius );
}

METHOD UINT drg_dg_larc_to_3pt( PR_DRG* self, EL_3PT* p3PtArc, EL_LARC* pLarc )
{
    return ConvertLArcTo3PtArc( p3PtArc, pLarc );
}

METHOD UINT drg_dg_3pt_to_larc( PR_DRG* self, EL_LARC* pLarc, EL_3PT* p3PtArc )
{
    return Convert3PtArcToLArc( pLarc, p3PtArc );
}

METHOD UBYTE* drg_dg_get_fbuf( PR_DRG* self )
{
    return FBuf;
}

METHOD VOID drg_dg_normalise_rect( PR_DRG* self, ARECT* rect )
{
    NormaliseRect( rect );
}

METHOD INT drg_dg_get_grey_bmid( PR_DRG* self )
{
    return idGreyBM;
}

METHOD INT drg_dg_get_black_bmid( PR_DRG* self )
{
    return idBlackBM;
}

METHOD HANDLE drg_dg_get_bmhand( PR_DRG* self, INT flag )
{
    if( flag == BITMAP_BLACK )
    {
        return hBlackBM;
    }
    /* flag == BITMAP_GREY */
    return hGreyBM;
}

METHOD VOID drg_dg_swap_bitmap( PR_DRG* self )
{
    HANDLE tmp;

    tmp = hBlackBM;
    hBlackBM = hGreyBM;
    hGreyBM = tmp;
}

METHOD PIC_HEAD* drg_dg_get_pic_head( PR_DRG* self )
{
    return &PicHeader;
}

METHOD AUNIT drg_dg_average( PR_DRG* self, AUNIT au1, AUNIT au2 )
{
    return (AUNIT) ( ( (LONG) au1 + au2 ) / 2 );
}

METHOD AUNIT drg_dg_long_div_uint( PR_DRG* self, LONG* pl, UINT ui )
{
    return (AUNIT) ( *pl / ui );
}

METHOD VOID drg_dg_l_div_l_eq_l( PR_DRG* self, LONG* result, LONG* pl1, LONG* pl2 )
{
    *result = ( *pl1 / *pl2 );
}

METHOD AUNIT drg_dg_centre_edge( PR_DRG* self, INT width, SWORD upp, AUNIT dx )
{
    LONG i;

    i = (LONG) width * upp;
    if( i <= dx )
    {
        return 0;
    }
    return (AUNIT) ( ( i - dx ) / 2 );
}

METHOD VOID drg_dg_create_band( PR_DRG* self, INT bandclass )
{
    Band = f_new( CAT_VECTOR_VECTOR, bandclass );
}

METHOD PR_VASTR* drg_dg_make_laylist( PR_DRG* self )
{
    PR_VASTR* list;
    int i;
    TEXT buf[LAYER_NAME_MAX+4];
    INT  ch;

    list = f_new( CAT_VECTOR_OLIB, C_VASTR );
    p_send3( list, O_VA_INIT, (LAYER_NAME_MAX+4) * 2 );
    for( i = 0 ; i < 16 ; i++ )
    {
        ch = 'A' + i;
        if( self->drg.LayStr[i] == NULL )
        {
            hAtos( buf, SR_LAYER_DNAME_FMT, ch, ch );
        }
        else
        {
            hAtos( buf, SR_LAYER_ANAME_FMT, ch, self->drg.LayStr[i] );
        }
        p_send4( list, O_VA_INSERT, i, buf );
    }
    return list;
}

METHOD PR_VASTR* drg_dg_make_fontlist( PR_DRG* self )
{
    PR_VASTR* list;
    INT i;

    list = f_new( CAT_VECTOR_OLIB, C_VASTR );
    p_send3( list, O_VA_INIT, LNAME_MAX_Z * 4 );
    for( i = 0 ; i < self->drg.FontSize ; i++ )
    {
        p_send4( list, O_VA_INSERT, i, self->drg.FontList[i].fname );
    }
    if( i == 0 ) return NULL;
    return list;
}

METHOD PR_VASTR* drg_dg_make_liblist( PR_DRG* self )
{
    PR_VASTR* list;
    INT i;

    list = f_new( CAT_VECTOR_OLIB, C_VASTR );
    p_send3( list, O_VA_INIT, LNAME_MAX_Z * 4 );
    for( i = 0 ; i < self->drg.LibCount ; i++ )
    {
        p_send4( list, O_VA_INSERT, i, self->drg.LibList[i].fname );
    }
    if( i == 0 ) return NULL;
    return list;
}

METHOD PR_VASTR* drg_dg_make_symlist( PR_DRG* self, INT iLib )
{
    PR_VASTR* list;
    LIB* pLib;
    EL_SYMBOLHDR sym;
    LONG pos;
    TE_SYM loc;
    INT i;

    pLib = &self->drg.LibList[iLib];

    list = f_new( CAT_VECTOR_OLIB, C_VASTR );
    p_send3( list, O_VA_INIT, LNAME_MAX_Z * 4 );
    for( i = 0 ; i < pLib->symcount ; i++ )
    {
        pos = pLib->symlist + i * sizeof(TE_SYM);
        f_seek( pLib->pfcb, P_FABS, &pos );
        f_read( pLib->pfcb, &loc, sizeof(TE_SYM) );
        pos = pLib->data + loc;
        f_seek( pLib->pfcb, P_FABS, &pos );
        f_read( pLib->pfcb, &sym, sizeof(EL_SYMBOLHDR) );
        /* Ensure name is zero terminated */
        sym.bound.pos.x = 0; /* Pretend you didn't see this! */
        p_send4( list, O_VA_INSERT, i, sym.name );
    }
    if( i == 0 ) return NULL;
    return list;
}

METHOD INT drg_dg_get_sym_index( PR_DRG* self, INT iLib, UWORD ref )
{
    LIB* pLib;
    EL_SYMBOLHDR sym;
    LONG pos;
    TE_SYM loc;
    INT i;

    pLib = &self->drg.LibList[iLib];
    if( pLib->pfcb == NULL )
    {
        p_send3( self, O_DG_OPEN_LIB, pLib );
        if( pLib->pfcb == NULL )
        {
            return -1;
        }
    }

    for( i = 0 ; i < pLib->symcount ; i++ )
    {
        pos = pLib->symlist + i * sizeof(TE_SYM);
        f_seek( pLib->pfcb, P_FABS, &pos );
        f_read( pLib->pfcb, &loc, sizeof(TE_SYM) );
        pos = pLib->data + loc;
        f_seek( pLib->pfcb, P_FABS, &pos );
        f_read( pLib->pfcb, &sym, sizeof(EL_SYMBOLHDR) );
        if( ref == sym.ref )
        {
            return i;
        }
    }
    return -1;
}

METHOD UWORD drg_dg_get_sym_ref( PR_DRG* self, INT iLib, INT ind )
{
    LIB* pLib;
    LONG pos;
    TE_SYM loc;
    EL_SYMBOLHDR sym;

    pLib = &self->drg.LibList[iLib];
    if( pLib->pfcb == NULL )
    {
        return 0;
    }
    pos = pLib->symlist + ind * sizeof(TE_SYM);
    f_seek( pLib->pfcb, P_FABS, &pos );
    f_read( pLib->pfcb, &loc, sizeof(TE_SYM) );
    pos = pLib->data + loc;
    f_seek( pLib->pfcb, P_FABS, &pos );
    f_read( pLib->pfcb, &sym, sizeof(EL_SYMBOLHDR) );
    return sym.ref;
}

METHOD VOID drg_dg_get_textht_str( PR_DRG* self, TEXT* buf, LKSET* set )
{
    UWORD div;

    div = set->sdiv & ~FLIP_FLAG;
    GetTextHtStr( buf, set->smul, div, self->vec.Units );
}

METHOD INT drg_dg_set_textht( PR_DRG* self, TEXT* buf, LKSET* set )
{
    UWORD flip;
    INT ret;

    flip = set->sdiv & FLIP_FLAG;
    ret = SetTextHt( &set->smul, &set->sdiv, buf, self->vec.Units );
    set->sdiv |= flip;
    return ret;
}

METHOD VOID drg_dg_get_angle_dbl( PR_DRG* self, DOUBLE* dbl, ANGLE* ang )
{
    GetAngleDbl( dbl, ang->sin, ang->cos );
}

METHOD VOID drg_dg_set_angle( PR_DRG* self, ANGLE* ang, DOUBLE* dbl )
{
    SetAngle( &ang->sin, &ang->cos, dbl );
}

METHOD VOID drg_dg_get_scale_dbl( PR_DRG* self, DOUBLE* dbl, LKSET* set )
{
    GetScaleDbl( dbl, set );
}

METHOD VOID drg_dg_set_scale( PR_DRG* self, LKSET* set, DOUBLE* dbl )
{
    SetScale( set, dbl );
}

METHOD AUNIT drg_dg_calc_angle( PR_DRG* self, ANGLE* ang, A_PT* cpt, A_PT* pt )
{
    return CalcAngle( ang, cpt, pt );
}

METHOD VOID drg_dg_adjust_lkdim( PR_DRG* self, AUNIT* au, LKSET* set )
{
    AdjustLkDim( au, set );
}

METHOD VOID drg_dg_adjust_lkang( PR_DRG* self, ANGLE* ang, LKSET* set )
{
    AdjustLkAng( ang, set );
}

METHOD VOID drg_dg_adjust_lkpt( PR_DRG* self, A_PT* pt, EL_LINK* lk, A_PT* hot )
{
    AdjustLkPt( pt, &lk->pos, &lk->set, hot, NULL );
}

METHOD VOID* drg_dg_new_class( PR_DRG* self, INT classnum )
{
    return f_new( CAT_VECTOR_VECTOR, classnum );
}

METHOD VOID drg_dg_adjust_lkel( PR_DRG* self, UBYTE* pBuf, EL_LINK* pLink,
    A_PT* pHotPt )
{
    AdjustFileEl( pBuf, &pLink->pos, &pLink->set, pHotPt, NULL );
}

METHOD INT drg_dg_set_text_rect( PR_DRG* self, EL_TEXT* pText )
{
    return SetTextRect( pText );
}

METHOD INT drg_dg_angle_in_arc( PR_DRG* self, ANGLE* pAng, ANGLE* aArc )
{
    return IsAngleInArc( pAng, &aArc[0], &aArc[1] );
}

METHOD VOID drg_dg_get_polar_pt( PR_DRG* self, A_PT* pt, AUNIT* array, ANGLE* ang )
{
    GetPolarPt( pt, (A_PT*) array, ang, array[2] );
}

METHOD VOID drg_dg_rel_polar_pt( PR_DRG* self, R_PT* rpt, ANGLE* ang, AUNIT rad )
{
    GetRelPolarPt( rpt, ang->sin, ang->cos, rad );
}

METHOD INT drg_dg_rect_overlap( PR_DRG* self, ARECT* rect1, ARECT* rect2 )
{
    return RectOverlap( rect1, rect2 );
}

METHOD INT drg_dg_is_layer_on( PR_DRG* self, INT layer )
{
    return IsLayerOn( layer );
}

METHOD VOID drg_dg_dlg_title_s( PR_DRG* self, UINT id )
{
    SetTitleScaleUnits( id );
}

METHOD VOID drg_dg_dlg_set_s_au( PR_DRG* self, UINT index, AUNIT au )
{
    DlgSetScaleAunit( index, au );
}

METHOD AUNIT drg_dg_dlg_get_s_au( PR_DRG* self, UINT index )
{
    return DlgSenseScaleAunit( index );
}

METHOD VOID drg_dg_dlg_set_s_lau( PR_DRG* self, UINT index, LAUNIT* lau )
{
    DlgSetScaleLaunit( index, lau );
}

METHOD VOID drg_dg_dlg_get_s_lau( PR_DRG* self, UINT index, LAUNIT* lau )
{
    DlgSenseScaleLaunit( index, lau );
}

METHOD VOID drg_dg_dlg_set_s_rpt( PR_DRG* self, UINT index, A_PT* pPt, A_PT* pOrig )
{
    DlgSetScaleRpt( index, pPt, pOrig );
}

METHOD INT drg_dg_lau_to_s_text( PR_DRG* self, TEXT* buf, LAUNIT* lau )
{
    return LaunitToScaleText( buf, *lau );
}

METHOD AUNIT drg_dg_s_text_to_au( PR_DRG* self, TEXT* buf )
{
    return ScaleTextToAunit( buf );
}

METHOD INT drg_dg_rpt_to_s_text( PR_DRG* self, TEXT* buf, A_PT* pPt, A_PT* pOrig )
{
    return RptToScaleText( buf, pPt, pOrig );
}

METHOD VOID drg_dg_update_scale( PR_DRG* self )
{
    UpdateDispScale();
}

METHOD VOID drg_dg_snap_cursor( PR_DRG* self )
{
    AdjustCur( &self->vec.Cur, &self->vec.Cur );
}

METHOD VOID drg_dg_debug( PR_DRG* self, INT n1, INT n2, TEXT* text )
{
    DB_DATA data;

    data.rid = SR_DEBUG_3;
    data.code1 = n1;
    data.code2 = n2;
    data.text = text;

    LaunchDialog( &data, DEBUG_DIALOG, C_DBDLG );
}

/* End of VECDRG.C */
