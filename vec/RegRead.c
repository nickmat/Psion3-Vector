/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: REGCODE - SHAREWARE REGISTRATION *  Written by: Nick Matthews  *
 *  Module: READ FUNCTION FOR TARGET PROGRAM *  Date Started: 15 Jul 1997  *
 *    File: REGREAD.C       Type: C SOURCE   *  Date Revised: 25 Aug 1997  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Copyright (c) 1997, Nick Matthews

 25aug97  Silently convert '0' to 'O' and '1' to 'I'.

*/

#include "regread.h"

/***************************************************************************
 **  XorBuf  XOR the buffer of length len with the 4 byte code.
 **  ~~~~~~
 */

static void XorBuf( unsigned char* buf, int len, unsigned char* code )
{
    int i;

    for( i = 0 ; i < len ; i++ )
    {
        buf[i] ^= code[ i % 4 ];
    }
}


/***************************************************************************
 **  UnmakeCode  Convert the code string to a byte array, return the length.
 **  ~~~~~~~~~~
 */

static int UnmakeCode( unsigned char* buf, const char* code )
{
    unsigned a, shift;
    char ch;
    int i, n;

    shift = 11;
    n = 0;
    a = 0;
    for( i = 0 ; code[i] ; i++ )
    {
        ch = code[i];
        if( ch == '0' ) ch = 'O';
        if( ch == '1' ) ch = 'I';
        a |= (unsigned) ( ch - ( ( ch < ';' ) ? '2' : ';' ) ) << shift;
        if( shift <= 8 )
        {
            buf[n++] = a >> 8;
            a <<= 8;
            shift += 8;
        }
        shift -= 5;
    }
    return n;
}

/***************************************************************************
 **  Unshuffle  Unshuffle the byte array and XOR with fixed keys
 **  ~~~~~~~~~
 */

static void Unshuffle( unsigned char* buf, int len )
{
    long code;
    int i, m;
    unsigned char tmp;
    int l = len - 1;
    int size = len - 2;

    code = RC_POST_XOR;
    XorBuf( buf, len, (unsigned char*) &code );
    for( i = RC_SHUFFLE - 1 ; i >= 0 ; --i )
    {
        m = ( ( buf[0] + i ) % size ) + 1;
        tmp = buf[0];
        buf[0] = buf[m];
        buf[m] = buf[l];
        buf[l] = tmp;
    }
    code = RC_PRE_XOR;
    XorBuf( buf, len, (unsigned char*) &code );
}

/***************************************************************************
 **  UnpackTokens  Convert the byte array to 5 bit tokens.
 **  ~~~~~~~~~~~~
 */

static void UnpackTokens( unsigned char* buf2, const unsigned char* buf1, unsigned len )
{
    unsigned int shift, ch, i, c, a;

    shift = ch = i = c = 0;
    for(;;)
    {
        while( shift <= 8 )
        {
            ch <<= 8;
            if( i < len )
            {
                ch |= buf1[i++];
            }
            shift += 8;
        }
        shift -= 5;
        a = ch & ( RC_MASK << shift );
        buf2[c++] = ( a >> shift );
        if( c > buf2[0] ) break;
    }
}

/***************************************************************************
 **  TokensToString  Convert the 5 bit tokens to a zero terminated string
 **  ~~~~~~~~~~~~~~
 */

static int TokensToString( char* name, unsigned char* tok, int len )
{
    int i, c;
    int ucase = 1;

    for( c = i = 0 ; c < len ; c++ )
    {
        switch( tok[c] )
        {
        case RC_CH_SHIFT:
            ucase ^= 1;
            continue;
        case RC_CH_SPACE:
            name[i] = ' ';
            break;
        case RC_CH_HYPHAN:
            name[i] = '-';
            break;
        default:
            name[i] = tok[c] + ( ucase ? 'A' : 'a' );
        }
        i++;
        if( i == RC_NAME_MAX_Z ) break;
    }
    name[i] = '\0';
    return i;
}

/***************************************************************************
 **  rcReadCode  Convert the Code string to a name and program string.
 **  ~~~~~~~~~~  Returns the alt code value if a valid Code, 0 if not valid.
 */

int rcReadCode( char* name, char* prog, const char* code )
{
    unsigned char buf1[RC_CODE_MAX_Z], buf2[RC_CODE_MAX_Z];
    int len;

    len = UnmakeCode( buf1, code );
    Unshuffle( buf1, len );
    if( *(unsigned*) buf1 != RC_CODE_KEY )
    {
        return 0;
    }
    UnpackTokens( buf2, &buf1[2], len - 2 );
    len = TokensToString( name, &buf2[1], buf2[0] ) - 3;
    prog[0] = name[len];
    prog[1] = name[len+1];
    prog[2] = '\0';
    name[len] = '\0';
    return name[len+2] - 'a' + 1;
}

/* End of RegRead.c file */
