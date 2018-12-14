/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION EPOC16      *  Written by: Nick Matthews  *
 *  Module: EXCHANGE FILES DYL DEFINES       *  Date Started:  4 Apr 1998  *
 *    File: VECXCH.H        Type: C HEADER   *  Date Revised:  4 Apr 1998  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1998, Nick Matthews

Note:-
 The xchange class must be the 1st class in libray and the first 3 member
 functions must be the same.

*/

#ifndef VECXCH_H
#define VECXCH_H

#define C_XCHANGE     0
#define O_XCH_VERSION 1
#define O_XCH_IMPORT  2
#define O_XCH_EXPORT  3

typedef struct
{
    TEXT* fname;
} EXPORT_DATA;



typedef struct
{
    TEXT* fname;
    UINT  res;
} EXPORT_DATA_QCD;

#endif /* VECXCH_H */

