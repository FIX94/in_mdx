//
// Winamp .MDX (X68000 YM2151 & MSM6258 AUDIO FILE) input plug-in
//
// use-dll    "MXDRV.DLL","X68Sound.DLL"
// use-header "depend.h","mxdrv.h"
//            "in2.h","out.h"
// 参考       "mxp.c"
//
#include <windows.h>
#include "in2.h"
#define __MXDRV_LOADMODULE
#include "mxdrv.h"
#include "resource.h"
#include "in_mdx.h"

extern void volatile *(*MXDRV_GetWork)(int);


//////////////////////////////////////////////////
//クレジット表示
void about(HWND hwndParent)
{
	TCHAR credit[512];
	TCHAR mdxdrv_credit[256];
	size_t ldummy;
	mbstowcs_s(&ldummy, mdxdrv_credit, (char *)MXDRV_GetWork(MXDRV_WORK_CREDIT), 256);

	wsprintf(credit,
			TEXT("X68k MDX PlugIn ") TEXT(DLL_VERSION) TEXT(" By Tanimoto / RuRuRu .\n%s"),
			mdxdrv_credit);

    ::MessageBox(hwndParent, credit, TEXT("About X68000 MDX Decoder"), MB_OK);
}


//////////////////////////////////////////////////
//ファイル種別問合せ
// @param fn in 問い合わせファイル名
// @return 処理対象(!=0)/不明(0)
int isourfile(const wchar_t * /*fn*/)
{
    // 拡張子による振り分けに任せる
	return(0);
}


//////////////////////////////////////////////////
//イコライザ
void eq_set(int /*on*/, char* /*data[10]*/, int /*preamp*/)
{ 
	// 現在の仕様ではイコライザはDSPプラグインの一つとして処理されます
	// この関数は呼ばれません
}

//////////////////////////////////////////////////
//ボリューム変更
// @param volume in 新しいボリューム(0%〜100%->0〜255)
void setvolume(int volume)
{
	mod.outMod->SetVolume(volume);
}

//////////////////////////////////////////////////
//パン変更
// @param pan in 新しいパン(L100%〜R100%->127〜+127)
void setpan(int pan)
{
	mod.outMod->SetPan(pan);
}
