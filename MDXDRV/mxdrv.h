// MXDRV.DLL header
// Copyright (C) 2000 GORRY.

#ifndef __MXDRV_H__
#define __MXDRV_H__

#include "depend.h"

typedef struct tagMXWORK_CH {
	UBYTE volatile *S0000;	// Ptr
	UBYTE S0004_b;	// PCM bank
	UBYTE volatile *S0004;	// voice ptr
	ULONG S0008;	// bend delta
	ULONG S000c;	// bend offset
	UWORD S0010;	// D
	UWORD S0012;	// note+D
	UWORD S0014;	// note+D+bend+Pitch LFO offset
	UBYTE S0016;	// flags b3=keyon/off
	UBYTE S0017;	// flags
	UBYTE S0018;	// ch
	UBYTE S0019;	// carrier slot
	UBYTE S001a;	// len
	UBYTE S001b;	// gate
	UBYTE S001c;	// p
	UBYTE S001d;	// keyon slot
	UBYTE S001e;	// Q
	UBYTE S001f;	// Keyon delay
	UBYTE S0020;	// Keyon delay counter
	UBYTE S0021;	// PMS/AMS
	UBYTE S0022;	// v
	UBYTE S0023;	// v last
	UBYTE S0024;	// LFO delay
	UBYTE S0025;	// LFO delay counter
	UBYTE volatile *S0026;	// Pitch LFO Type
	ULONG S002a;	// Pitch LFO offset start
	ULONG S002e;	// Pitch LFO delta start
	ULONG S0032;	// Pitch LFO delta
	ULONG S0036;	// Pitch LFO offset
	UWORD S003a;	// Pitch LFO length (cooked)
	UWORD S003c;	// Pitch LFO length
	UWORD S003e;	// Pitch LFO length counter
	UBYTE volatile *S0040;	// Volume LFO Type
	UWORD S0044;	// Volume LFO delta start
	UWORD S0046;	// Volume LFO delta (cooked)
	UWORD S0048;	// Volume LFO delta
	UWORD S004a;	// Volume LFO offset
	UWORD S004c;	// Volume LFO length
	UWORD S004e;	// Volume LFO length counter
} MXWORK_CH;

typedef struct tagMXWORK_GLOBAL {
	UWORD L001ba6;
	ULONG L001ba8;
	UBYTE volatile *L001bac;
	UBYTE L001bb4[16];
	UBYTE L001df4;
	UBYTE L001df6[16];
	UWORD L001e06;	// Channel Mask (true)
	UBYTE L001e08;
	UBYTE L001e09;
	UBYTE L001e0a;
	UBYTE L001e0b;
	UBYTE L001e0c;	// @t
	UBYTE L001e0d;
	UBYTE L001e0e;
	UBYTE L001e10;
	UBYTE L001e12;	// Paused
	UBYTE L001e13;	// End
	UBYTE L001e14;	// Fadeout Offset
	UBYTE L001e15;
	UBYTE L001e17;	// Fadeout Enable
	UBYTE L001e18;
	UBYTE L001e19;
	UWORD L001e1a;	// Channel Enable
	UWORD L001e1c;	// Channel Mask
	UWORD L001e1e[2];	// Fadeout Speed
	UWORD L001e22;
	UBYTE volatile *L001e24;
	UBYTE volatile *L001e28;
	UBYTE volatile *L001e2c;
	UBYTE volatile *L001e30;
	UBYTE volatile *L001e34;
	UBYTE volatile *L001e38;
	ULONG L00220c;
	UBYTE volatile *L002218;
	UBYTE volatile *L00221c;
	ULONG L002220; // L_MDXSIZE
	ULONG L002224; // L_PDXSIZE
	UBYTE volatile *L002228;	// voice data
	UBYTE volatile *L00222c;
	UBYTE L002230;
	UBYTE L002231;
	UBYTE L002232;
	UBYTE L002233[9];
	UBYTE L00223c[12];
	UBYTE L002245;
	UWORD L002246; // loop count
	ULONG FATALERROR;
	ULONG FATALERRORADR;
	ULONG PLAYTIME; // ���t����
	UBYTE MUSICTIMER;  // ���t���ԃ^�C�}�[�萔
	UBYTE STOPMUSICTIMER;  // ���t���ԃ^�C�}�[��~
	ULONG MEASURETIMELIMIT; // ���t���Ԍv�����~����
} MXWORK_GLOBAL;

typedef struct tagMXWORK_KEY {
	UBYTE OPT1;
	UBYTE OPT2;
	UBYTE SHIFT;
	UBYTE CTRL;
	UBYTE XF3;
	UBYTE XF4;
	UBYTE XF5;
} MXWORK_KEY;

typedef BYTE MXWORK_OPM[256];

typedef void CALLBACK MXCALLBACK_OPMINTFUNC( void );

enum {
	MXDRV_WORK_FM = 0,		// FM8ch+PCM1ch
	MXDRV_WORK_PCM,			// PCM7ch
	MXDRV_WORK_GLOBAL,
	MXDRV_WORK_KEY,
	MXDRV_WORK_OPM,
	MXDRV_WORK_PCM8,
	MXDRV_WORK_CREDIT,
	MXDRV_CALLBACK_OPMINT,
};

enum {
	MXDRV_ERR_MEMORY = 1,
};

#ifndef DLLEXPORT
#define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MXDRV_LOADMODULE

DLLEXPORT
int MXDRV_Start(
	int samprate,
	int betw,
	int pcmbuf,
	int late,
	int mdxbuf,
	int pdxbuf,
	int opmmode
);

DLLEXPORT
void MXDRV_End(
	void
);

DLLEXPORT
int MXDRV_GetPCM(
	void *buf,
	int len
);

DLLEXPORT
void MXDRV_Play(
	void *mdx,
	DWORD mdxsize,
	void *pdx,
	DWORD pdxsize
);

DLLEXPORT
void volatile *MXDRV_GetWork(
	int i
);

DLLEXPORT
void MXDRV(
	X68REG *reg
);

DLLEXPORT
DWORD MXDRV_MeasurePlayTime(
	void *mdx,
	DWORD mdxsize,
	void *pdx,
	DWORD pdxsize,
	int loop,
	int fadeout
);

DLLEXPORT
void MXDRV_PlayAt(
	DWORD playat,
	int loop,
	int fadeout
);

DLLEXPORT
int MXDRV_TotalVolume(
	int vol
);

#endif // __MXDRV_LOADMODULE

#ifdef __cplusplus
}
#endif // __cplusplus

#define MXDRV_Call( a )				\
{									\
	X68REG reg;						\
									\
	reg.d0 = (a);					\
	reg.d1 = 0x00;					\
	MXDRV( &reg );					\
}									\
									

#define MXDRV_Call_2( a, b )		\
{									\
	X68REG reg;						\
									\
	reg.d0 = (a);					\
	reg.d1 = (b);					\
	MXDRV( &reg );					\
}									\
									

#define MXDRV_Replay() MXDRV_Call( 0x0f )
#define MXDRV_Stop() MXDRV_Call( 0x05 )
#define MXDRV_Pause() MXDRV_Call( 0x06 )
#define MXDRV_Cont() MXDRV_Call( 0x07 )
#define MXDRV_Fadeout() MXDRV_Call_2( 0x0c, 19 )
#define MXDRV_Fadeout2(a) MXDRV_Call_2( 0x0c, (a) )



#endif //__MXDRV_H__
