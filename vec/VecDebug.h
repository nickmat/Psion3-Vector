/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: SET DEBUGGING FUNCTIONS          *  Date Started: 29 Jul 1997  *
 *    File: VECDEBUG.H      Type: C HEADER   *  Date Revised: 29 Jul 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#ifndef VECDEBUG_H
#define VECDEBUG_H

#ifdef DEBUG

#define DBX4_Debug(A,B,C)  Debug( (A), (B), (C) )

extern VOID Debug( INT n1, INT n2, TEXT* text );

#else

#define DBX4_Debug(A,B,C)

#endif


#endif /* VECDEBUG_H */
