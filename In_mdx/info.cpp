//
// Winamp .MDX (X68000 YM2151 & MSM6258 AUDIO FILE) input plug-in
//
// use-dll    "MXDRV.DLL","X68Sound.DLL"
// use-header "depend.h","mxdrv.h"
//            "in2.h","out.h"
// �Q�l       "mxp.c"
//
#include <windows.h>
#include "in2.h"
#define __MXDRV_LOADMODULE
#include "mxdrv.h"
#include "resource.h"
#include "in_mdx.h"

extern void volatile *(*MXDRV_GetWork)(int);


//////////////////////////////////////////////////
//�N���W�b�g�\��
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
//�t�@�C����ʖ⍇��
// @param fn in �₢���킹�t�@�C����
// @return �����Ώ�(!=0)/�s��(0)
int isourfile(const wchar_t * /*fn*/)
{
    // �g���q�ɂ��U�蕪���ɔC����
	return(0);
}


//////////////////////////////////////////////////
//�C�R���C�U
void eq_set(int /*on*/, char* /*data[10]*/, int /*preamp*/)
{ 
	// ���݂̎d�l�ł̓C�R���C�U��DSP�v���O�C���̈�Ƃ��ď�������܂�
	// ���̊֐��͌Ă΂�܂���
}

//////////////////////////////////////////////////
//�{�����[���ύX
// @param volume in �V�����{�����[��(0%�`100%->0�`255)
void setvolume(int volume)
{
	mod.outMod->SetVolume(volume);
}

//////////////////////////////////////////////////
//�p���ύX
// @param pan in �V�����p��(L100%�`R100%->127�`+127)
void setpan(int pan)
{
	mod.outMod->SetPan(pan);
}
