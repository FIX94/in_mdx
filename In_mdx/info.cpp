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
	char credit[256];

    wsprintf(credit,
             "X68k MDX PlugIn " DLL_VERSION " By Tanimoto / RuRuRu .\n%s",
             (char *)MXDRV_GetWork( MXDRV_WORK_CREDIT ));

    ::MessageBox(hwndParent, credit, "About X68000 MDX Decorder", MB_OK);
}


//////////////////////////////////////////////////
//�t�@�C����ʖ⍇��
// @param fn in �₢���킹�t�@�C����
// @return �����Ώ�(!=0)/�s��(0)
int isourfile(char * /*fn*/)
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
