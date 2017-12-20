// MXDRV.DLL X68000-depend header
// Copyright (C) 2000 GORRY.

#ifndef __DEPEND_H__
#define __DEPEND_H__

typedef unsigned char UBYTE;
typedef unsigned short UWORD;
typedef unsigned long ULONG;
typedef signed char SBYTE;
typedef signed short SWORD;
typedef signed long SLONG;

#define FALSE 0
#define TRUE 1

typedef struct __X68REG {
    ULONG d0;
    ULONG d1;
    ULONG d2;
    ULONG d3;
    ULONG d4;
    ULONG d5;
    ULONG d6;
    ULONG d7;
    UBYTE *a0;
    UBYTE *a1;
    UBYTE *a2;
    UBYTE *a3;
    UBYTE *a4;
    UBYTE *a5;
    UBYTE *a6;
    UBYTE *a7;
} X68REG;

#define SET 255
#define CLR 0
#define GETBWORD(a) ((((UBYTE *)(a))[0]*256)+(((UBYTE *)(a))[1]))
#define GETBLONG(a) ((((UBYTE *)(a))[0]*16777216)+(((UBYTE *)(a))[1]*65536)+(((UBYTE *)(a))[2]*256)+(((UBYTE *)(a))[3]))
#define PUTBWORD(a,b) ((((UBYTE *)(a))[0]=((b)>> 8)),(((UBYTE *)(a))[1]=((b)>> 0)))
#define PUTBLONG(a,b) ((((UBYTE *)(a))[0]=((b)>>24)),(((UBYTE *)(a))[1]=((b)>>16)), (((UBYTE *)(a))[2]=((b)>> 8)), (((UBYTE *)(a))[3]=((b)>> 0)))










#endif //__DEPEND_H__
