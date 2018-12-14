/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: MAIN GLOBAL AND MAGIC VARIABLES  *  Date Started: 15 May 1998  *
 *    File: VEC.H           Type: C HEADER   *  Date Revised: 15 May 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1998 Nick Matthews
*/

#ifndef VEC_H
#define VEC_H

GLREF_D WSERV_SPEC* wserv_channel;
GLREF_D PR_HWIMMAN* w_am;
GLREF_D PR_WSERV* w_ws;

GLREF_D TEXT* DatUsedPathNamePtr;
GLREF_D VOID* DatCommandPtr;
GLREF_D VOID* DatDialogPtr;

GLREF_D PR_DRG*   DatApp1;       /* Drg   */
GLREF_D UBYTE*    DatApp2;       /* Data  */
GLREF_D UBYTE*    DatApp3;       /* DBuf  */
/*GLREF_PR_BUILD* DatApp4; */    /* Build */
GLREF_D PR_BAND*  DatApp5;       /* Band  */
GLREF_D PR_UNDO*  DatApp6;       /* Undo  */
/*GLREF_D VOID*   DatApp7; */    /* Extra */

#define Drg   DatApp1
#define Data  DatApp2
#define DBuf  DatApp3
#define Build DatApp4      /* See VecBld.h */
#define Band  DatApp5
#define Undo  DatApp6
#define Extra DatApp7      /* See VecBld.h */


#define PN_PU_UNKNOWN_CTRL       247
#define PN_PU_UNKNOWN_FRAME_CTRL 248
#define PN_DELETE_ITEM_FROM_LIST 249
#define PN_PU_TEXT_TOO_BIG       250
#define PN_CORRUPT_DDATA         251
#define PN_UNKNOWN_BUILD_STEP    252
#define PN_CANNOT_OPEN_BITMAP    253
#define PN_CANNOT_CREATE_BITMAP  254

#define OUTSIDE_AA       -300
#define CORRUPT_FILE     -301
#define USER_ABORT       -302
#define VDL_VERSION_ERR  -303
#define INPUT_FILE_ERR   -304
#define MATHS_ERR        -305

#define AA  0x10000UL    /* Addressable area size, for x and y */

#endif /* VEC_H */