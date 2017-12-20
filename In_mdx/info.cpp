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
	char credit[256];

    wsprintf(credit,
             "X68k MDX PlugIn " DLL_VERSION " By Tanimoto / RuRuRu .\n%s",
             (char *)MXDRV_GetWork( MXDRV_WORK_CREDIT ));

    ::MessageBox(hwndParent, credit, "About X68000 MDX Decorder", MB_OK);
}


//////////////////////////////////////////////////
//ファイル種別問合せ
// @param fn in 問い合わせファイル名
// @return 処理対象(!=0)/不明(0)
int isourfile(char * /*fn*/)
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
// @param volume in 新しいボリューム(0%～100%->0～255)
void setvolume(int volume)
{
	mod.outMod->SetVolume(volume);
}

//////////////////////////////////////////////////
//パン変更
// @param pan in 新しいパン(L100%～R100%->127～+127)
void setpan(int pan)
{
	mod.outMod->SetPan(pan);
}
