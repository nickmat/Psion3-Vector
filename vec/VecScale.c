/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: SCALE CONVERSION FUNCTIONS       *  Date Started: 28 Oct 1998  *
 *    File: VECSCALE.C      Type: C SOURCE   *  Date Revised:  3 Nov 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

  Copyright (c) 1998 Nick Matthews
*/

#include <hwim.h>
#include <p_math.h>
#include <p_config.h>
#include <limits.h>
#include <vector.g>
#include "vector.h"


static WORD AScale[4] = { 1000, 100, 2540, 35 };
                       /*  cm    mm  Inch  pt */

/***************************************************************************
 **  UpdateDispScale  Update the Drg->drg.Scale value from the Drg->vec.*
 **  ~~~~~~~~~~~~~~~  values
 */

VOID UpdateDispScale( VOID )
{
    DOUBLE tmp;
    DOUBLE ten;
    DOUBLE mag;
    WORD wd;
    DOUBLE* scale;

    scale = &Drg->drg.DScale;
    wd = 1;
    p_itof( scale, &wd );
    p_itof( &tmp, &AScale[ Drg->vec.Units ] );
    p_fdiv( scale, &tmp );
    p_fld( &Drg->drg.PScale, scale );

    p_itof( &tmp, &Drg->vec.Pref.scale.div );
    p_fmul( scale, &tmp );
    p_itof( &tmp, &Drg->vec.Pref.scale.mul );
    p_fdiv( scale, &tmp );

    /* Set drg.MaxDp */
    wd = 1;
    p_itof( &tmp, &wd );
    p_fmul( &tmp, scale );
    wd = 10;
    p_itof( &ten, &wd );
    p_fld( &mag, &ten );
    for( wd = 0 ; wd < 4 ; wd++ )
    {
        if( p_fcmp( &tmp, &mag ) > 0 )
        {
            break;
        }
        p_fdiv( &mag, &ten );
    }
    Drg->drg.MaxDp = wd;
}

/***************************************************************************
 **  LaunitToText  Convert a LAUNIT to a TEXT string
 **  ~~~~~~~~~~~~
 */

static INT LaunitToText( TEXT* buf, LAUNIT val, DOUBLE* scale, UBYTE width, UBYTE dp )
{
    DOUBLE dbl;
    P_DTOB format;
    E_CONFIG cfg;
    INT len;

    p_longtof( &dbl, &val );
    p_fmul( &dbl, scale );

    p_getctd( &cfg );
    format.type = P_DTOB_FIXED;
    format.width = width;
    format.ndec = dp;
    format.point = cfg.decimalSeparator;
    format.trilen = 0;
    len = p_dtob( buf, &dbl, &format );

    /* Remove trailing zeros */
    if( len > 0 )
    {
        buf[len] = '\0';
        if( dp)
        {
            --len;
            while( buf[len] == '0' )
            {
                buf[len] = '\0';
                --len;
            }
            if( buf[len] == cfg.decimalSeparator )
            {
                buf[len] = '\0';
            }
        }
    }
    else
    {
        /* Somethings gone wrong! */
        buf[0] = '\0';
        return 1;
    }
    return 0;
}

/***************************************************************************
 **  LaunitToScaleFixed  Convert a LAUNIT to a TEXT string for scale value
 **  ~~~~~~~~~~~~~~~~~~  display.
 */

INT LaunitToScaleFixed( TEXT* buf, LAUNIT val )
{
    return LaunitToText( buf, val, &Drg->drg.DScale, DIM_FIXED_MAX, Drg->vec.DispDp );
}

/***************************************************************************
 **  LaunitToScaleText  Convert a LAUNIT to a TEXT string for modification
 **  ~~~~~~~~~~~~~~~~~  in a dialog box.
 */

INT LaunitToScaleText( TEXT* buf, LAUNIT val )
{
    return LaunitToText( buf, val, &Drg->drg.DScale, DIM_TEXT_MAX, Drg->drg.MaxDp );
}

/***************************************************************************
 **  LaunitToPaperText  Convert a LAUNIT to a paper value TEXT string for
 **  ~~~~~~~~~~~~~~~~~  modification in a dialog box.
 */

INT LaunitToPaperText( TEXT* buf, LAUNIT val )
{
    return LaunitToText( buf, val, &Drg->drg.PScale, DIM_TEXT_MAX, 3 );
}

/***************************************************************************
 **  TextToLaunit  Convert a TEXT string to a LAUNIT
 **  ~~~~~~~~~~~~
 */

static VOID TextToLaunit( LAUNIT* lau, TEXT* buf, DOUBLE* scale )
{
    DOUBLE dbl, half;
    WORD wd;
    E_CONFIG cfg;
    INT ret;

    wd = 1;
    p_itof( &half, &wd );
    wd = 2;
    p_itof( &dbl, &wd );
    p_fdiv( &half, &dbl );

    p_getctd( &cfg );

    ret = p_stod( &buf, &dbl, cfg.decimalSeparator );
    if( ret )
    {
        *lau = 0;
        return;
    }
    p_fdiv( &dbl, scale );
    p_fadd( &dbl, &half );
    p_intl( (LONG*) lau, &dbl );
}

/***************************************************************************
 **  ScaleTextToLaunit  Convert a scale TEXT string to a LAUNIT
 **  ~~~~~~~~~~~~~~~~~
 */

VOID ScaleTextToLaunit( LAUNIT* lau, TEXT* buf )
{
    TextToLaunit( lau, buf, &Drg->drg.DScale );
}

/***************************************************************************
 **  ScaleTextToAunit  Convert a TEXT string to a AUNIT after entry in a
 **  ~~~~~~~~~~~~~~~~  dialog box.
 */

AUNIT ScaleTextToAunit( TEXT* buf )
{
    LAUNIT lau;

    TextToLaunit( &lau, buf, &Drg->drg.DScale );
    if( lau >= AA || lau < 0 )
    {
        return 0;
    }
    return (AUNIT) lau;
}

/***************************************************************************
 **  PaperTextToLaunit  Convert a paper TEXT string to a LAUNIT
 **  ~~~~~~~~~~~~~~~~~
 */

VOID PaperTextToLaunit( LAUNIT* lau, TEXT* buf )
{
    TextToLaunit( lau, buf, &Drg->drg.PScale );
}

/***************************************************************************
 **  PaperTextToAunit  Convert a paper value TEXT string to a AUNIT after
 **  ~~~~~~~~~~~~~~~~  entry in a dialog box.
 */

AUNIT PaperTextToAunit( TEXT* buf )
{
    LAUNIT lau;

    TextToLaunit( &lau, buf, &Drg->drg.PScale );
    if( lau >= AA || lau < 0 )
    {
        return 0;
    }
    return (AUNIT) lau;
}

/***************************************************************************
 **  RptToScaleText  Convert a pPt point, offset by point pOrig to a TEXT
 **  ~~~~~~~~~~~~~~  string for display in a dialog box.
 */

INT RptToScaleText( TEXT* buf, A_PT* pPt, A_PT* pOrig )
{
    LAUNIT lau;
    TEXT xbuf[DIM_TEXT_MAX_Z];
    TEXT ybuf[DIM_TEXT_MAX_Z];
    INT ret;

    lau = (LAUNIT) pPt->x - pOrig->x;
    ret = LaunitToScaleText( xbuf, lau );
    if( ret ) return ret;
    lau = (LAUNIT) pPt->y - pOrig->y;
    lau = Drg->vec.yDir ? -lau : lau;
    ret = LaunitToScaleText( ybuf, lau );
    if( ret ) return ret;
    p_atos( buf, "%s, %s", xbuf, ybuf );
    return 0;
}

/* End of VECSCALE.C file */
