/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: REGCODE - SHAREWARE REGISTRATION *  Written by: Nick Matthews  *
 *  Module: READ FUNCTION FOR TARGET PROGRAM *  Date Started: 15 Jul 1997  *
 *    File: REGREAD.H       Type: C HEADER   *  Date Revised: 16 Jul 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews
*/

#ifndef REGREAD_H
#define REGREAD_H

#define RC_NAME_MAX_Z  20
#define RC_NAME_MAX    16
#define RC_NAME_MIN     5
#define RC_PROG_MAX_Z   4
#define RC_PROG_MAX     2
#define RC_CODE_MAX_Z  40
#define RC_CODE_MAX    38

#define RC_CH_SPACE   26
#define RC_CH_HYPHAN  27
#define RC_CH_SHIFT   28

#define RC_MASK      0x001f
#define RC_SHUFFLE   32
#define RC_PRE_XOR   0x8a2bee47L
#define RC_POST_XOR  0x837b0451L
#define RC_CODE_KEY  0x5b9a

extern int rcReadCode( char* name, char* prog, const char* code );

#endif /* REGREAD_H */
