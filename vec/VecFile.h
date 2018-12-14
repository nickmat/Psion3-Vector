/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Project: Vector - Vector graphics system *  Written by: Nick Matthews  *
 *   Module: Vector file specification       *  Date Started: 20 Mar 1997  *
 *     File: VecFile.h     Type: C Header    *  Date Revised:  6 May 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews

 Vector File Specification
 ~~~~~~~~~~~~~~~~~~~~~~~~~
  A Vector file consists of 4 sections:-
    1.  Header      A fixed size obligatory header
    2.  Settings    Optional, current program settings
    3.  Tables      Zero or more tables of type T_???
    4.  Elements    Zero or more drawing elements of type V_???


 Revision History
 ~~~~~~~~~~~~~~~~
*/

#ifndef VECFILE_H
#define VECFILE_H

#define VECTOR_SIG     "kcVectorDrawing"
#define SIGNITURE_SIZE  16  /* File signiture size */
#define FILE_VERSION   100  /* File format identification */
/*#define SET_VERSION    101 */ /* Program settings identification */
#define BYTES_MAX      200  /* Maximum number of bytes in element */
#define POINTS_MAX      60  /* Maximum number of points in an element */
#define GROUP_MAX      200  /* Maximum number of elements in a group */
#define UNAME_MAX       15  /* User defined name, without trailing zero */
#define UNAME_MAX_Z     16  /* User defined name, zero terminated */
#define LNAME_MAX        8  /* Link file name, without trailing zero */
#define LNAME_MAX_Z     10  /* Link file name, trailing zero, even length */
#define SNAME_MAX       20  /* Symbol name length, without trailing zero */
#define SNAME_MAX_Z     22  /* Symbol name, trailing zero, even length */

#define TRIG_DIV    0x4000
#define FLIP_FLAG   0x8000

#define T_NULL        0  /* Null table */
#define T_SYMBOLS     1  /* TE_SYM  Index to defined symbols */
#define T_LIBRARY     2  /* TE_LIB  Librarys referenced by symbols */
#define T_ATTRIBUTES  3  /* TE_??   Attributes referenced by elements */
#define T_TRANSLATION 4  /* TE_??   Not yet used */
#define T_LAYERS      5  /* Delimited list. List of Layer details */
#define T_CHAR_LIST   6  /* TE_CHAR Index to defined font characters */
#define T_FONT        7  /* TE_FONT Fonts referenced by elements */
#define T_TEXT_STYLE  8  /* TE_TSTYLE Table of available text styles */

#define V_NULL        0  /* Null element */
#define V_LINE        1  /* EL_LINE   Line */
#define V_POLYLINE    2  /* EL_PLINE  Polyline */
#define V_POLYGON     3  /* EL_PLINE  Polygon */
#define V_CIRCLE      4  /* EL_CIRCLE Circle */
#define V_3PT_ARC     5  /* EL_3PT    Circular 3 point Arc */
#define V_BOX         6  /* EL_LINE   Rectangular box (internal only) */
#define V_ARC         7  /* EL_ARC    Circular radius & angles Arc */
#define V_LINK       16  /* EL_LINK   Link to a symbol */
#define V_TEXT       17  /* EL_TEXT   Text line, linked to a font */
#define V_GROUP      32  /* EL_GROUP  Following are part of group */
#define V_SYMBOL     33  /* EL_SYM    Following are part of symbol */
#define V_CHARACTER  34  /* EL_CHAR   Following are part of a font char */
#define V_DIM_HORIZ  64  /* EL_DIM    Horizontal dimension */
#define V_DIM_VERT   65  /* EL_DIM    Vertical dimension */
#define V_DIM_ALIGN  66  /* EL_DIM    Aligned dimension */
#define V_DIM_ANGLE  67  /* EL_ANG    Angular dimension */
#define V_DIM_RADIUS 68  /* EL_RAD    Radius dimension */
#define V_DIM_DIA    69  /* EL_DIA?   Diameter dimension */

#define SIZEOF_SYMHDR    26  /* Header up to boundry rect */
#define SIZEOF_SYMBOLHDR sizeof(EL_SYMBOLHDR)  /* Excludes attach pts */
#define SIZEOF_TEXTHDR   ( sizeof(EL_TEXTHDR) - 1 ) /* Text element excluding text */
#define SIZEOF_DIMHDR    ( sizeof(EL_DIM) - 1 ) /* EL_DIM element without text */

/* Basic types, as used in Vector Files */
#ifdef PSION_3A
#define PSION
#endif

#ifdef PSION_SI
#define PSION
#endif

#ifdef PSION
  typedef signed char         SBYTE;    /* Signed 8 bit */
  typedef short               SWORD;    /* Signed 16 bit */
  typedef long               SDWORD;    /* Signed 32 bit */
#else
  typedef signed char         SBYTE;    /* Signed 8 bit */
  typedef unsigned char       UBYTE;    /* Unsigned 8 bit */
  typedef short               SWORD;    /* Signed 16 bit */
  typedef unsigned short      UWORD;    /* Unsigned 16 bit */
  typedef long               SDWORD;    /* Signed 32 bit */
#endif

typedef UWORD   AUNIT;     /* Absolute unit */
typedef SDWORD LAUNIT;     /* Large absolute unit */
typedef SWORD   RUNIT;     /* Relative unit */

typedef struct A_PT      /* Absolute point */
{
    AUNIT x;
    AUNIT y;
} A_PT;

typedef struct LA_PT      /* Large Absolute point */
{
    LAUNIT x;
    LAUNIT y;
} LA_PT;

typedef struct R_PT      /* Relative point */
{
    RUNIT x;
    RUNIT y;
} R_PT;

typedef struct ARECT     /* Absolute rectangle */
{
    A_PT pos;
    A_PT lim;
} ARECT;

typedef struct RRECT     /* Relative rectangle */
{
    R_PT pos;
    R_PT lim;
} RRECT;

typedef struct SCALE
{
    UBYTE q;
    UBYTE d;
} SCALE;

typedef struct ANGLE      /* Angle as Sin and Cos * TRIG_DIV */
{
    SWORD sin;
    SWORD cos;
} ANGLE;

typedef struct LKSET      /* Link Settings */
{
    UWORD smul;
    UWORD sdiv;   /* or'ed with FLIP_FLAG */
    ANGLE a;
} LKSET;

typedef struct VECHDR    /* File header */
{
    UBYTE sig[SIGNITURE_SIZE];  /* = kcVectorDrawing/0 */
    UWORD version;      /* Vector File specification number */
    UWORD hdrsize;      /* Sizeof(VECHDR) + Settings section */
    UWORD program;      /* Program version reference */
    UWORD dataoffset;   /* hdrsize + Tables section (check value) */
    UWORD tablecount;   /* Number of tables in Table section */
} VECHDR;


typedef struct TBHDR      /* Table header */
{
    UWORD size;
    UWORD type;
} TBHDR;

typedef UWORD TE_SYM;     /* T_SYMBOLS Symbol definition table entry */

typedef struct SYMSET     /* Symbol library default settings */
{
    SCALE scale;
    ANGLE rotate;
    UBYTE mirror;
    UBYTE create;
} SYMSET;

/* Flip flags for SYMSET.mirror */
#define FLIP_HORIZONTAL  0x01
#define FLIP_VERTICAL    0x02

typedef struct TE_LIB     /* T_LIBRARY Library table indexed by V_LINK */
{
    UBYTE name[LNAME_MAX];
    SYMSET setting;
} TE_LIB;

typedef UWORD TE_CHAR;    /* Font Character definition table entry */

typedef struct TE_FONT    /* T_FONT Font file table entry index by V_TEXT */
{
    UBYTE name[LNAME_MAX];
} TE_FONT;

typedef struct TE_TSTYLE  /* T_TEXT_STYLE Text style table entry */
{
    UBYTE font;     /* Index to Font table */
    UBYTE units;    /* Units used to set size */
    LKSET set;      /* Link settings to use */
    SWORD slant;    /* Oblique angle */
    UBYTE name[UNAME_MAX_Z];   /* Users identification name */
} TE_TSTYLE;

/* Data Store Elements */

typedef struct ELHDR   /* Element header */
{
    UBYTE size;
    UBYTE type;
    UBYTE attr;
    UBYTE layer;
} ELHDR;

typedef struct EL_LINE   /* Element with two points */
{
    ELHDR hdr;
    A_PT  beg;
    A_PT  end;
} EL_LINE;

typedef struct EL_PLINE  /* Multiple point element */
{
    ELHDR hdr;
    A_PT  beg;
    A_PT  pt[POINTS_MAX-1];
} EL_PLINE;

typedef struct EL_CIRCLE  /* Circular Element */
{
    ELHDR hdr;
    A_PT  centre;
    AUNIT radius;
} EL_CIRCLE;

typedef struct EL_ARC    /* Circular Arc (2 angles) element */
{
    ELHDR hdr;
    A_PT  centre;
    AUNIT radius;
    ANGLE beg;
    ANGLE end;
} EL_ARC;

typedef struct EL_LARC  /* Large arc (internal only) */
{
    ELHDR  hdr;
    LA_PT  centre;
    LAUNIT radius;
    ANGLE  beg;
    ANGLE  end;
} EL_LARC;

typedef struct EL_3PT    /* Three point Element (arc) */
{
    ELHDR hdr;
    A_PT  beg;
    A_PT  mid;
    A_PT  end;
} EL_3PT;

typedef struct EL_4PT    /* 4 point Element (Box polygon) */
{
    ELHDR hdr;
    A_PT  pt[4];
} EL_4PT;


typedef struct EL_TEXT   /* Line of text element */
{
    ELHDR hdr;
    A_PT  pos;      /* Position of first char */
    RRECT box;      /* Bounding box, relative to pos */
    LKSET set;      /* Size, angle & flip settings */
    SWORD slant;    /* Oblique angle */
    UBYTE font;     /* Index to font table */
    UBYTE text[BYTES_MAX]; /* Character array (NOT zero terminated) */
} EL_TEXT;

typedef struct EL_TEXTHDR /* Just the header part of an EL_TEXT element */
{
    ELHDR hdr;
    A_PT  pos;
    RRECT box;   /* Bounding box, relative to pos */
    LKSET set;
    SWORD slant;
    UBYTE font;
    UBYTE text; /* 1st char only */
} EL_TEXTHDR;

#define DIM_OVERSHOOT  250

typedef struct EL_DIM    /* Line dimension */
{
    ELHDR hdr;
    A_PT   pt1;
    A_PT   pt2;
    A_PT   pos;         /* Position of text */
    UBYTE  dstyle;      /* Dimension style (future) */
    UBYTE  spare;       /* Spare, packing */
    EL_TEXTHDR text;    /* Text details */
} EL_DIM;

typedef struct EL_LINK
{
    ELHDR hdr;
    A_PT  pos;   /* Position of hot spot */
    RRECT box;   /* Bounding box, relative to pos */
    LKSET set;   /* Size, angle & flip settings */
    UBYTE lib;   /* Index into library table */
    UBYTE sym;   /* Zero in file, index into table in library file */
    UWORD ref;   /* Reference to symbol in library file */
} EL_LINK;

typedef struct ELGRP
{
    UBYTE size;
    UBYTE type;
    UBYTE count;
    UBYTE flags;
} ELGRP;

typedef struct EL_GROUP  /* Start of group elements */
{
    ELGRP grp;
} EL_GROUP;

typedef struct EL_SYMBOL   /* Start of symbol elements */
{
    ELGRP grp;
    UWORD ref;
    UBYTE name[SNAME_MAX];
    ARECT bound;
    A_PT  hot;
    A_PT  attach[POINTS_MAX-10];
} EL_SYMBOL;

typedef struct EL_SYMBOLHDR   /* Just the header part of a symbol */
{
    ELGRP grp;
    UWORD ref;
    UBYTE name[SNAME_MAX];
    ARECT bound;
    A_PT  hot;
} EL_SYMBOLHDR;

typedef struct EL_CHAR  /* Start of Character elements */
{
    ELGRP grp;
    UBYTE ref;       /* ASCII number */
    UBYTE spare;
    AUNIT pitch;     /* Distance to move the text cursor on by */
    ARECT bound;
    A_PT  hot;
} EL_CHAR;

/* Combined element union */

typedef union ELEM
{
    UBYTE     size;
    ELHDR     hdr;
    ELGRP     grp;
    EL_LINE   line;
    EL_PLINE  pline;
    EL_CIRCLE circle;
    EL_ARC    arc;
    EL_LARC   larc;
    EL_3PT    el3pt;
    EL_4PT    el4pt;
    EL_LINK   link;
    EL_TEXT   text;
    EL_SYMBOL sym;
    EL_CHAR   ch;
    EL_DIM    dim;
    UBYTE     buf[256];
} ELEM;

#endif  /* VECFILE_H */
