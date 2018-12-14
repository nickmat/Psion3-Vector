/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: BUILD DEFINITION                 *  Date Started: 10 Apr 1997  *
 *    File: VECBLD.H        Type: C HEADER   *  Date Revised:  2 Jun 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#ifndef VECBLD_H
#define VECBLD_H

#ifndef VECBLD_G
#include <vecbld.g>
#endif

GLREF_D PR_BUILD* DatApp4;
GLREF_D PR_EXTRA* DatApp7;

extern VOID BegDrawClear( VOID );
extern VOID BegDraw( VOID );
extern VOID EndDraw( VOID );
extern VOID Rewind( VOID );
extern VOID BegUndo( INT msg );
extern VOID EndUndo( VOID );
extern VOID AddUndo( VOID* pBuf );
extern INT  ComparePt( A_PT* pt1, A_PT* pt2 );

extern INT BldLaunchDialog( VOID* buf, INT id, INT class );
extern VOID SelectStep( PR_SELECT* self, AUNIT ax, AUNIT ay );

#pragma save, ENTER_CALL
extern INT AddToStore( VOID* pBuf );
extern INT InsertInStore( UINT pos, VOID* pBuf );
#pragma restore

#endif /* VECBLD_H */
