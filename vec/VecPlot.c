/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: PLOTTER CLASS                    *  Date Started: 21 Dec 1997  *
 *    File: VECPLOT.C       Type: C SOURCE   *  Date Revised: 23 Feb 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, 1998, Nick Matthews
*/

#include <p_math.h>
#include <hwim.h>
#include <vector.g>
#include <vector.rsg>
#include "vector.h"

#define ESC  "\033"

static P_SRCHAR parport3c =
{
    P_BAUD_19200, P_BAUD_19200,  /* tbaud, rbaud = 19200 */
    P_DATA_8,                    /* frame = Data 8bits, 1 stop bit, parity none */
    0x00,                        /* parity = none */
    P_OBEY_XOFF | P_IGN_CTS | P_OBEY_DSR | P_FAIL_DSR, /* hand */
    0x11, 0x13,                  /* xon, xoff */
    0x00, 0L                     /* flags, tmask */
};


static VOID PlotBuf( VOID* pcb, TEXT* buf )
{
    UWORD len;

    len = p_slen( buf );
    p_iow( pcb, P_FWRITE, buf, &len );
}

static VOID GetPlotPt( PR_PLOT* self, LA_PT* plpResult, LONG x, LONG y )
{
    if( self->plot.set.Rotate )
    {
        plpResult->x = ( ( Drg->vec.Page.lim.y - y - self->plot.set.OffsetX ) * 2 ) / 5;
        plpResult->y = ( ( Drg->vec.Page.lim.x - x - self->plot.set.OffsetY ) * 2 ) / 5;
    }
    else
    {
        plpResult->x = ( ( x - Drg->vec.Page.pos.x - self->plot.set.OffsetX ) * 2 ) / 5;
        plpResult->y = ( ( Drg->vec.Page.lim.y - y - self->plot.set.OffsetY ) * 2 ) / 5;
    }
}

static LONG GetPlotRel( LONG u )
{
    u = ( u * 2 ) / 5;
    return u;
}

static VOID GetLPolarPt( LA_PT* pPt, LA_PT* pCentrePt, ANGLE* pAngle, LAUNIT radius )
{
    pPt->x = pCentrePt->x + ( (LONG) pAngle->cos * radius ) / TRIG_DIV;
    pPt->y = pCentrePt->y + ( (LONG) pAngle->sin * radius ) / TRIG_DIV;
}

static VOID AngleSub( ANGLE* pResult, ANGLE* pAngA, ANGLE* pAngB )
{
    SWORD temp;

    temp = ( (LONG) pAngA->sin * pAngB->cos - (LONG) pAngA->cos * pAngB->sin ) / TRIG_DIV;
    pResult->cos = ( (LONG) pAngA->cos * pAngB->cos + (LONG) pAngA->sin * pAngB->sin ) / TRIG_DIV;
    pResult->sin = temp;
}

static INT SetCircleStep( LONG rad )
{
    LONG pu;

    pu = GetPlotRel( rad );
    if( pu < 100 )  return 30;
    if( pu < 300 )  return 20;
    if( pu < 1000 ) return 10;
    if( pu < 3000 ) return  5;
    return 3;
}

static VOID PlotLArc( PR_PLOT* self, EL_LARC* pLArc )
{
    TEXT buf[80];
    LA_PT pt;
    LA_PT ptCentre, pt1;
    ANGLE ang;
    TEXT szAng[10];
    INT cord;

    GetPlotPt( self, &ptCentre, pLArc->centre.x, pLArc->centre.y );
    GetLPolarPt( &pt, &pLArc->centre, &pLArc->end, pLArc->radius );
    GetPlotPt( self, &pt1, pt.x, pt.y );
    AngleSub( &ang, &pLArc->end, &pLArc->beg );
    GetAngleStr( szAng, ang.sin, ang.cos );
    cord = SetCircleStep( pLArc->radius );
    p_atos( buf, "PU%ld,%ld;PD;AA%ld,%ld,%s,%d;",
        pt1.x, pt1.y, ptCentre.x, ptCentre.y, szAng, cord );
    PlotBuf( self->plot.pcb, buf );
    self->plot.downat = FALSE;
}

/***************************************************************************
 **  PlotPrefix  Output setup strings to plotter
 **  ~~~~~~~~~~~
 */

static VOID PlotPrefix( PR_PLOT* self )
{
    TEXT buf[120];
    LONG t, l, r, b;
    LA_PT pt1, pt2;

    l = Drg->vec.Marg.pos.x;
    b = Drg->vec.Marg.lim.y;
    r = Drg->vec.Marg.lim.x;
    t = Drg->vec.Marg.pos.y;

    GetPlotPt( self, &pt1, l, b );
    GetPlotPt( self, &pt2, r, t );
    p_atos( buf, "IW%ld,%ld,%ld,%ld;SP1;PA;", pt1.x, pt1.y, pt2.x, pt2.y );
    PlotBuf( self->plot.pcb, buf );
}

#pragma METHOD_CALL

/***************************************************************************
 **  plot_init  Initialise plotter
 **  ~~~~~~~~~
 */

METHOD INT plot_plot_init( PR_PLOT* self )
{
    self->plot.set.OffsetX = 1000;
    self->plot.set.OffsetY = 400;
    self->plot.set.Rotate = ! Drg->vec.Pref.page.orient;
    return TRUE;
}

/***************************************************************************
 **  plot_open  Open plotter ready for output
 **  ~~~~~~~~~
 */

METHOD INT plot_plot_open( PR_PLOT* self )
{
    PRN* pPrn;
    P_SRCHAR* pSerSet;
    INT ret;

    /* open timer */
#if 0
    ret = p_open( &self->plot.tcb, "TIM:", -1 );
    if( ret )
    {
        return PLOTERR_INIT_FAILED;
    }
#endif
    pPrn = &Drg->drg.Cfg.prn;
    pSerSet = &pPrn->serport;
    switch( pPrn->device )
    {
    case PRNDEV_PARA:
        ret = p_open( &self->plot.pcb, "PAR:A", -1 );
        if( ret != E_FILE_DEVICE )
        {
            /* PAR:A device exists, must be 3a */
            if( ret == 0 )
            {
                break;
            }
            /* failed */
            self->plot.pcb = NULL;
            break;
        }
        /* PAR:A device doesn't exists, must be 3c */
        pSerSet = &parport3c;
        /* Fall thru to serial setting */
    case PRNDEV_SERIAL:
        ret = p_open( &self->plot.pcb, "TTY:A", -1 );
        if( ret == 0 )
        {
            ret = p_iow( self->plot.pcb, P_FSET, pSerSet );
            if( ret == 0 )
            {
                break;
            }
            p_close( self->plot.pcb );
        }
        self->plot.pcb = NULL;
        break;
    case PRNDEV_FILE:
        ret = p_open( &self->plot.pcb, pPrn->fname, P_FSTREAM | P_FREPLACE | P_FUPDATE );
        if( ret == 0 )
        {
            break;
        }
        self->plot.pcb = NULL;
        break;
    case PRNDEV_IRLINK:
        ret = p_open( &self->plot.pcb, "IRP:A", -1 );
        if( ret == 0 )
        {
            break;
        }
        self->plot.pcb = NULL;
        break;
    default:
        ret = -1;
        self->plot.pcb = NULL;
    }
    if( self->plot.pcb )
    {
        return 0;
    }
    hErrorDialog( ret, SR_PRINT_OPEN_FAIL );
    return PLOTERR_INIT_FAILED;
}

/***************************************************************************
 **  plot_close  Close down plotter
 **  ~~~~~~~~~~
 */

METHOD VOID plot_plot_close( PR_PLOT* self )
{
    p_sleep( 20L );
    p_close( self->plot.pcb );
    /*p_close( self->plot.tcb );*/
}

/***************************************************************************
 **  plot_prefix  Output setup strings to plotter
 **  ~~~~~~~~~~~
 */

METHOD VOID plot_plot_prefix( PR_PLOT* self )
{
    PlotBuf( self->plot.pcb, "IN;" );
    PlotPrefix( self );
}

/***************************************************************************
 **  plot_postfix  Output shutdown, paper eject strings to plotter
 **  ~~~~~~~~~~~~
 */

METHOD VOID plot_plot_postfix( PR_PLOT* self )
{
    PlotBuf( self->plot.pcb, "PU;PG;" );
}

/***************************************************************************
 **  plot_draw  Output an element
 **  ~~~~~~~~~
 */

METHOD VOID plot_plot_draw( PR_PLOT* self )
{
    UINT hand;
    VOID* idle;

    idle = f_newsend( CAT_VECTOR_OLIB, C_AIDLE, O_AO_INIT );

    if( self->plot.esc == NULL )
    {
        LaunchDialog( &self->plot.cancel, ESC_PLOT_DIALOG, C_ESCPLOTDLG );
        self->plot.esc = DatDialogPtr;
    }

    p_send2( self, O_PLOT_PREFIX );
    Rewind();
    for(;;)
    {
        hand = p_send3( Data, O_DD_NEXT, DBuf );
        if( hand == EL_EOD ) break;
        /* Yield to allow dialogs to update */
        p_send2( idle, O_AO_QUEUE );
        p_send2( w_am, O_AM_START );
        if( self->plot.cancel ) break;
        if( DBuf[TYPE_BYTE] & V_AGGR_BIT ) continue;
        if( Drg->vec.LayMask & LayerBit( DBuf[LAYER_BYTE] ) )
        {
            Draw( DBuf );
        }
    }
    p_send2( self, O_PLOT_POSTFIX );
    hDestroy( idle );
}

/***************************************************************************
 **  plot_line  Plot line
 **  ~~~~~~~~~
 */

METHOD VOID plot_plot_line( PR_PLOT* self, A_PT* data )
{
    TEXT buf[80];
    LA_PT pt1, pt2;

    GetPlotPt( self, &pt1, data[0].x, data[0].y );
    GetPlotPt( self, &pt2, data[1].x, data[1].y );
    if( ! self->plot.downat || pt1.x != self->plot.last.x || pt1.y != self->plot.last.y )
    {
        p_atos( buf, "PU%ld,%ld;", pt1.x, pt1.y );
        PlotBuf( self->plot.pcb, buf );
    }
    p_atos( buf, "PD%ld,%ld;", pt2.x, pt2.y );
    PlotBuf( self->plot.pcb, buf );
    self->plot.downat = TRUE;
    self->plot.last = pt2;
}

/***************************************************************************
 **  plot_circle  Plot circle
 **  ~~~~~~~~~~~
 */

METHOD VOID plot_plot_circle( PR_PLOT* self, A_PT* data )
{
    TEXT buf[80];
    LONG r;
    LA_PT pt;
    INT cord;

    GetPlotPt( self, &pt, data[0].x, data[0].y );
    r = GetPlotRel( data[1].x );
    cord = SetCircleStep( data[1].x );
    p_atos( buf, "PU%ld,%ld;CI%ld,%d;", pt.x, pt.y, r, cord );
    PlotBuf( self->plot.pcb, buf );
    self->plot.downat = FALSE;
}

/***************************************************************************
 **  plot_arc  Plot arc (circle)
 **  ~~~~~~~~
 */

METHOD VOID plot_plot_arc( PR_PLOT* self, EL_ARC* pArc )
{
    EL_LARC larc;

    larc.hdr = pArc->hdr;
    larc.centre.x = pArc->centre.x;
    larc.centre.y = pArc->centre.y;
    larc.radius = pArc->radius;
    larc.beg = pArc->beg;
    larc.end = pArc->end;
    PlotLArc( self, &larc );
}

/***************************************************************************
 **  plot_3pt_arc  Plot arc (3 pt)
 **  ~~~~~~~~~~~~
 */

METHOD VOID plot_plot_3pt_arc( PR_PLOT* self, EL_3PT* pArc )
{
    EL_LARC larc;

    if( Convert3PtArcToLArc( &larc, pArc ) == FALSE )
    {
        DrawPolyline( &pArc->beg, 3 );
        return;
    }
    PlotLArc( self, &larc );
}

/*------------------------------------------------------------------------*/
/***************************************************************************
 **  plot_init  Initialise plotter
 **  ~~~~~~~~~
 */

METHOD INT pcl5plt_plot_init( PR_PCL5PLT* self )
{
    self->plot.set.OffsetX = 600;
    self->plot.set.OffsetY = 1450;
    self->plot.set.Rotate = Drg->vec.Pref.page.orient;
    return TRUE;
}

/***************************************************************************
 **  plot_prefix  Output setup strings to plotter
 **  ~~~~~~~~~~~
 */

METHOD VOID pcl5plt_plot_prefix( PR_PCL5PLT* self )
{
    PlotBuf( self->plot.pcb, ESC "E" ESC "%0B" );
    p_supersend2( self, O_PLOT_PREFIX );
}

/***************************************************************************
 **  plot_postfix  Output shutdown, paper eject strings to plotter
 **  ~~~~~~~~~~~~
 */

METHOD VOID pcl5plt_plot_postfix( PR_PCL5PLT* self )
{
    p_supersend2( self, O_PLOT_POSTFIX );
    PlotBuf( self->plot.pcb, ESC "%0A" ESC "E" );
}

/*------------------------------------------------------------------------*/
/***************************************************************************
 **  plot_init  Initialise custom plotter
 **  ~~~~~~~~~
 */

#if 0
METHOD INT cushpglplt_plot_init( PR_PCL5PLT* self )
{
    p_supersend2( self, O_PLOT_INIT );
    return LaunchDialog( &self->plot.set, CUSTOM_PLOT_DIALOG, C_CUSPLOTDLG );
}
#endif

/*------------------------------------------------------------------------*/
/***************************************************************************
 **  cusplotdlg  Custom plotting dialog
 **  ~~~~~~~~~~
 */

#if 0
METHOD VOID cusplotdlg_dl_dyn_init( PR_CUSPLOTDLG* self )
{
    PLOTCUS* cus;

    cus = (PLOTCUS*) self->dlgbox.rbuf;

    SetTitleUnits( SR_CUSTOM_PLOT_TITLE_FMT );
    hDlgSetChlist( CUSPLOTDLG_ROTATE, cus->Rotate );
    DlgSetAunit( CUSPLOTDLG_OFFSETX, cus->OffsetX );
    DlgSetAunit( CUSPLOTDLG_OFFSETY, cus->OffsetY );
}

METHOD INT cusplotdlg_dl_key( PR_CUSPLOTDLG* self, INT index, INT keycode, INT actbut )
{
    PLOTCUS* cus;

    cus = (PLOTCUS*) self->dlgbox.rbuf;

    cus->Rotate = hDlgSenseChlist( CUSPLOTDLG_ROTATE );
    cus->OffsetX = DlgSenseAunit( CUSPLOTDLG_OFFSETX );
    cus->OffsetY = DlgSenseAunit( CUSPLOTDLG_OFFSETY );
    return WN_KEY_NO_CHANGE;
}
#endif

/*------------------------------------------------------------------------*/
/***************************************************************************
 **  escplotdlg  Cancel printing dialog
 **  ~~~~~~~~~~
 */

METHOD VOID escplotdlg_dl_dyn_init( PR_ESCPLOTDLG* self )
{
    /* set title */
}

METHOD INT escplotdlg_dl_key( PR_ESCPLOTDLG* self, INT index, INT keycode, INT actbut )
{
    INT* esc;

    esc = (INT*) self->dlgbox.rbuf;

    *esc = TRUE;
    return WN_KEY_NO_CHANGE;
}

/* End of VecPlot.c file */
