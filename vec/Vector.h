/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
 *  Module: MAIN CLASS MEMBER FUNCTIONS      *  Date Started: 27 Aug 1996  *
 *    File: VECTOR.H        Type: C HEADER   *  Date Revised: 15 Apr 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1996, 1997, Nick Matthews
*/

#ifndef VECTOR_H
#define VECTOR_H

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
/*GLREF_D VOID* DatApp7;*/       /* Extra */

#define Drg   DatApp1
#define Data  DatApp2
#define DBuf  DatApp3
#define Build DatApp4      /* See VecBld.h */
#define Band  DatApp5
#define Undo  DatApp6
#define Extra DatApp7      /* See VecBld.h */

/* Built in dyl's, order is the same as vector.dfl, see vector.mak */
#define VECSYM_DYL  0
#define VECBLD_DYL  1

GLREF_D UBYTE*    FBuf;
GLREF_D PR_UNDO*  Redo;

extern HANDLE hBlackBM;
extern HANDLE hGreyBM;
extern INT idBlackBM;
extern INT idGreyBM;
extern HANDLE BuildDyl;
extern TEXT* CfgFileName;

#define ERROR_STRINGS           ESR_RETRY
#define ES_RETRY                ( ESR_RETRY - ERROR_STRINGS )
#define ES_CANCEL               ( ESR_CANCEL - ERROR_STRINGS )
#define ES_DISCARD              ( ESR_DISCARD - ERROR_STRINGS )
#define ES_CANT_DISPLAY_SYM_WIN ( ESR_CANT_DISPLAY_SYM_WIN - ERROR_STRINGS )
#define ES_NOT_VEC_FILE         ( ESR_NOT_VEC_FILE - ERROR_STRINGS )
#define ES_CORRUPT_FILE         ( ESR_CORRUPT_FILE - ERROR_STRINGS )
#define ES_CANT_READ_FILE       ( ESR_CANT_READ_FILE - ERROR_STRINGS )
#define ES_CANT_SAVE_FILE       ( ESR_CANT_SAVE_FILE - ERROR_STRINGS )

#define MAX_ERROR_STRINGS 6
extern TEXT* ErrorStrings[MAX_ERROR_STRINGS];

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

/* Info Window Width */
#ifdef PSION_3A
#define VIW_WIDTH    110
#else /* PSION_SI */
#define VIW_WIDTH     80
#endif /* PSION_?? */

/* Scroll windows width */
#define VSW_WIDTH    6

/* Ruler window vertical width & horizontal height */
#define VVRW_WIDTH   32
#define VHRW_HEIGHT  14

/* Command window height */
#define VCW_HEIGHT   20

#define PI_Q   1686629713L
#define PI_D   0x20000000L

#define CENTRE(P,L) ( (P) + ( ( (L)-(P) ) >> 1 ) )

/*#define ABS(X)   ( (X) > 0 ? (X) : -(X) )*/

#define AA  0x10000UL    /* Addressable area size, for x and y */

/* VecAbout.c */
extern INT CheckReg( VOID );

/* VecCom.c */
extern INT LaunchDialog( VOID* buf, INT id, INT class );

/* */
extern void ExtentToRect( P_RECT* rect, const P_EXTENT* extent );

/* Process.asm */
extern VOID CDECL kcAddPt( A_PT* dest, A_PT* source, INT ax, INT ay );

/* DrawLine.asm */
extern VOID CDECL kcDrawLine( INT* data, SRECT* clip, DATTR* attr );
extern INT  CDECL kcIsLineInRect( INT* data, SRECT* rect, DATTR* attr );

/* Clear.asm */
extern VOID CDECL kcClearBitmap( HANDLE bitmap );
extern VOID CDECL kcClearRect( HANDLE bitmap, ARECT* pRect );
extern VOID CDECL kcScrollBitmap( HANDLE bitmap, INT sx, INT sy );

/* Circle.asm */
extern VOID CDECL kcDrawCircle( INT* data, SRECT* clip, DATTR* attr );
extern INT  CDECL kcIsCircleInRect( INT* data, SRECT* rect, DATTR* attr );
extern VOID CDECL kcDraw2AngArc( VOID* data, SRECT* clip, DATTR* attr );
extern INT  CDECL kcIs2AngArcInRect( VOID* data, SRECT* rect, DATTR* attr );

/* CalcArc.c */
extern VOID CDECL kcDraw3PtArc( INT* data, SRECT* clip, DATTR* attr );
extern INT  CDECL kcIs3PtArcInRect( INT* data, SRECT* rect, DATTR* attr );

/* VecCalc.c */
extern VOID CalcQuadrant( EL_ARC* arc, A_PT* pt );
extern AUNIT CalcAngle( ANGLE* angle, A_PT* cpt, A_PT* pt );
extern INT  GetIntersect( A_PT* result, A_PT* aline, A_PT* bline );
extern INT  CalcArcCentreAPt( A_PT* pt, EL_3PT* arc );
extern INT  CalcPerpendicularPt( A_PT* result, A_PT* line, A_PT* pt, INT OnLine );
extern INT  GetLCircleSeg(
    ANGLE* ang1, ANGLE* ang2, A_PT* line, LA_PT* cpt, LAUNIT* rad );
extern INT  GetCircleBreak( A_PT* cord, A_PT* apt, A_PT* line, A_PT* cpt, AUNIT rad );
extern INT  IsAngleInArc( ANGLE* ang, ANGLE* beg, ANGLE* end );
extern INT  Convert3PtArcToArc( EL_ARC* result, EL_3PT* arc3pt );
extern INT  Convert3PtArcToLArc( EL_LARC* result, EL_3PT* arc3pt );
extern INT  ConvertLArcTo3PtArc( EL_3PT* arc3pt, EL_LARC* larc );
extern INT  GetArcRect( ARECT* pRect, ELEM* el );

/* DrawGrid.asm */
extern VOID CDECL kcDrawGrid( GRID_DATA* data, SRECT* clip, DATTR* attr );

/* Hypot.asm */
extern UINT CDECL kcHypotenuse( INT side1, INT side2 );
extern UINT CDECL kcSquareRoot( unsigned long number );

/* Trig.asm */
extern VOID CDECL kcTrig2A( INT dx,INT dy,INT* cos2q,INT* sin2q,UINT* div );

/* VecDrg.c */
extern VOID AdjustCur( A_PT* result, A_PT* source );
extern VOID AdjustScrn( AUNIT posx, AUNIT posy );
extern INT IsElementInRect( SRECT* rect, UBYTE* el );
extern INT OpenAddFile4( FONT* pFont );

/* VecDWin.c */
extern VOID DrawGrid( GRIDSET* pGrid, SRECT* pRect );

/* VecIWin.c */
extern VOID UpdateInfo( INT index );

/* VecJump.c */
extern VOID JumpToNearest( INT nearest );
#define NEAR_LINE_END  0
#define NEAR_LINE_MID  1
#define NEAR_CENTRE    2
#define NEAR_QUADRANT  3
#define NEAR_PERP      4
#define NEAR_TANGENT   5
#define NEAR_INTERSECT 6

/* VecMisc.c */
#define  AtoSdim(A) ( (SUNIT) ( (A) / Drg->vec.upp ) )
#define  StoAdim(S) ( (S) * Drg->vec.upp )
#define DATASIZE(H) ( ( (H)[SIZE_BYTE] - sizeof(ELHDR) ) / sizeof(A_PT) )

extern void NormaliseRect( ARECT* rect );
extern INT  PointInRect( const A_PT* pt, const ARECT* rect );
extern INT  RectOverlap( const ARECT* pRectRaw, const ARECT* pRectNorm );
extern VOID AddPointToBound( ARECT* rect, A_PT* pt );
extern INT  BitToIndex( UINT flag );
extern INT  ComparePt( A_PT* pt1, A_PT* pt2 );
extern VOID AddAngle( ANGLE* pAng1, ANGLE* pAng2 );
extern INT  RotatePt( A_PT* pPt, A_PT* pPtCtr, ANGLE* pAng );

extern VOID  GetRelPolarPt( R_PT* rpt, SWORD asin, SWORD acos, AUNIT radius );
extern VOID  GetPolarPt( A_PT* pt, A_PT* centre, ANGLE* angle, AUNIT radius );
extern INT   IsLayerOn( INT layer );
#define LayerBit(L) ( 1 << ( (L) & 0x0f ) )
#define IsSnapOn()  ( Drg->vec.Pref.grid.flags & GRID_SNAP )

extern VOID BegDrawClear( VOID );
extern VOID BegDraw( VOID );
extern VOID EndDraw( VOID );
extern VOID Rewind( VOID );
extern VOID BegUndo( INT msg );
extern VOID EndUndo( VOID );
extern VOID AddUndo( VOID* pBuf );
extern VOID KillUndo( VOID );
extern VOID GetDWinSize( P_EXTENT* pext );

extern UINT GetHeaderSize( VOID* pfcb );
extern UINT SeekTable( VOID* pfcb, UWORD offset, UWORD type, TBHDR* thdr );

extern VOID GetTextHtStr( TEXT* buf, UWORD smul, UWORD sdiv, INT units );
extern INT SetTextHt( UWORD* smul, UWORD* sdiv, TEXT* pstr, INT units );
extern VOID SetAngle( SWORD* asin, SWORD* acos, DOUBLE* angle );
extern VOID GetAngleDbl( DOUBLE* d, SWORD asin, SWORD acos );
extern VOID GetAngleStr( TEXT* buf, SWORD asin, SWORD acos );
extern VOID SetScale( LKSET* set, DOUBLE* dbl );
extern VOID GetScaleDbl( DOUBLE* d, LKSET* set );

extern VOID DeleteFromList( VOID* list, INT* size, INT unit, INT item );
extern SWORD TestUpp( SWORD upp );
extern INT IsKeyDown( VOID );
extern AUNIT GetMean( AUNIT au1, AUNIT au2 );

/* VecScale.c */
#define DIM_TEXT_MAX      9
#define DIM_TEXT_MAX_Z   10
#define DIM_FIXED_MAX     7
#define DIM_FIXED_MAX_Z  DIM_TEXT_MAX_Z
extern VOID  UpdateDispScale( VOID );
extern INT   LaunitToScaleFixed( TEXT* buf, LAUNIT val );
extern INT   LaunitToScaleText( TEXT* buf, LAUNIT val );
extern AUNIT ScaleTextToAunit( TEXT* buf );
extern VOID  ScaleTextToLaunit( LAUNIT* lau, TEXT* buf );
extern INT   LaunitToPaperText( TEXT* buf, LAUNIT val );
extern AUNIT PaperTextToAunit( TEXT* buf );
extern VOID  PaperTextToLaunit( LAUNIT* lau, TEXT* buf );

#define RPT_TEXT_MAX_Z   25
extern INT   RptToScaleText( TEXT* buf, A_PT* pPt, A_PT* pOrig );

/* VecLocs.c */
extern VOID DeleteTempFiles( VOID );
extern VOID ReadTemplate( TEXT* fname );

/* VecDraw.c */
extern VOID Draw( UBYTE* element );
extern VOID DrawLine( A_PT* data );
extern VOID DrawCircle( A_PT* data );
extern VOID DrawArc( EL_ARC* arc );
extern VOID Draw3PtArc( EL_3PT* arc );
extern VOID DrawBox( A_PT* data );
extern VOID DrawPolyline( A_PT* data, INT size );
extern VOID DrawPolygon( A_PT* data, INT size );
extern void ProcessPt( A_PT* result, A_PT* source );
extern void ProcessDim( AUNIT* result, AUNIT* source );
extern void ProcessAng( ANGLE* result, ANGLE* source );
extern void ProcessLink( LKSET* result, LKSET* source );

extern INT IsElementInRect( SRECT* rect, UBYTE* el );

/* VecText.c */
extern VOID AdjustLkDim( AUNIT* dim, LKSET* set );
extern VOID AdjustLkAng( ANGLE* ang, LKSET* set );
extern VOID AdjustLkPt(
    A_PT* pt, A_PT* pos, LKSET* set, A_PT* hot, LONG* offset );
extern VOID AdjustFileEl(
    UBYTE* el, A_PT* pos, LKSET* set, A_PT* hot, LONG* offset );
extern INT  SetTextRect( EL_TEXT* pText );
extern VOID GetLinkRrect( RRECT* pRrect, EL_LINK* pLink );
extern VOID GetTextRect( A_PT* pt, EL_TEXT* pText );
extern VOID GetLinkRect( A_PT* pt, EL_LINK* pLink );
extern VOID DrawText( EL_TEXT* pText );
extern VOID DrawTextBox( EL_TEXT* pText );
extern INT DeleteTextStyle( UINT select );
extern VOID DrawLinkBox( EL_LINK* pLink );
extern VOID DrawLink( EL_LINK* pLink );
extern INT IsLinkInRect( SRECT* rect, EL_LINK* pLink );

/* VecDim.c */
extern VOID DrawDimHoriz( EL_DIM* pDim );
extern VOID UpdateDimHorizText( EL_DIM* pDim );

/* VecSADlg.c */
extern VOID SetExtension( TEXT* buf, INT ftype, INT export );
/* Flags for export arg */
#define FTYPE_EXPORT  TRUE
#define FTYPE_IMPORT  FALSE

/* VecDlg.c */
extern VOID SetTitleScaleUnits( UINT format );
extern VOID DlgSetScaleAunit( UBYTE index, AUNIT au );
extern AUNIT DlgSenseScaleAunit( UBYTE index );
extern VOID DlgSetScaleLaunit( UBYTE index, LAUNIT* lau );
extern VOID DlgSenseScaleLaunit( UBYTE index, LAUNIT* lau );

extern VOID SetTitlePaperUnits( UINT format );
extern VOID DlgSetPaperAunit( UBYTE index, AUNIT au );
extern AUNIT DlgSensePaperAunit( UBYTE index );
extern VOID DlgSetPaperRAunit( UBYTE index, AUNIT au, AUNIT orig );
extern AUNIT DlgSensePaperRAunit( UBYTE index, AUNIT orig );

extern VOID DlgSetScaleRpt( UBYTE index, A_PT* pPt, A_PT* pOrig );

/* VecPrint.c */
extern VOID DoPrint( VOID );

#endif /* VECTOR_H */
