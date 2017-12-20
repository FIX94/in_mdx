//
// Winamp .MDX (X68000 YM2151 & MSM6258 AUDIO FILE) input plug-in
//
// use-dll    "MXDRV.DLL","X68Sound.DLL"
// use-header "depend.h","mxdrv.h"
//            "in2.h","out.h"
// �Q�l       "mxp.c"
//
#include <windows.h>
#include <shlobj.h>
#include <mbstring.h>

#include "in2.h"
#include "resource.h"
#include "in_mdx.h"

struct env_t Env;						// �������
static int configed;					// �ݒ�ύX����
static TCHAR SaveFile[MAX_PATH];	// �ݒ�t�@�C���t���p�X

#define CONFIG_TITLE "Mdx Plug Setting"

typedef struct {
	char *key;
	int num;
} KeyInit;

static KeyInit freqlist[] = {
	{ "96000",		SOUND_96K },
	{ "88200",		SOUND_88K },
	{ "62500",		SOUND_62K },
	{ "48000",		SOUND_48K },
	{ "44100",		SOUND_44K },
	{ "22050",		SOUND_22K },
	{ NULL,			SOUND_22K },
};

static KeyInit priolist[] = {
	{ "Highest",	THREAD_PRIORITY_HIGHEST },
	{ "Higher",		THREAD_PRIORITY_ABOVE_NORMAL },
	{ "Normal",		THREAD_PRIORITY_NORMAL },
	{ "Lower",		THREAD_PRIORITY_BELOW_NORMAL },
	{ "Lowest",		THREAD_PRIORITY_LOWEST },
	{ NULL,			THREAD_PRIORITY_NORMAL },
};

//�����ݒ�l���[�h
void EnvLoad( char *fn )
{
	lstrcpy( SaveFile, fn );
	configed = 0;
	Env.samprate = GetPrivateProfileInt( "Switch", "SampleRate", 22050, SaveFile );
	Env.pcmbuf = GetPrivateProfileInt( "Switch", "PCMBuf", 5, SaveFile );
	Env.late = GetPrivateProfileInt( "Switch", "Late", 500, SaveFile );
	Env.pcm8use = GetPrivateProfileInt( "Switch", "PCM8", 1, SaveFile );
	Env.priority = GetPrivateProfileInt( "Switch", "Priority", 50, SaveFile );
	Env.Loop = GetPrivateProfileInt( "Switch", "Loop", 3, SaveFile );
	Env.qtime = GetPrivateProfileInt( "Switch", "Qtime", 2, SaveFile );
	Env.TotalVolume = GetPrivateProfileInt( "Switch", "TotalVolume", 100, SaveFile );
	Env.FadeTime = GetPrivateProfileInt( "Switch", "FadeTime", 19, SaveFile );
	Env.agc = GetPrivateProfileInt( "Switch", "AGC", 0, SaveFile );
	Env.ROMEO = GetPrivateProfileInt( "Switch", "ROMEO", 0, SaveFile );
	Env.ignoreErr = GetPrivateProfileInt( "Switch", "IgnoreError", 1, SaveFile );
	Env.enable = (BOOL)GetPrivateProfileInt("Switch", "Enable", 1, SaveFile);
	GetPrivateProfileString( "Path", "PDX", "c:\\", Env.PdxDir, sizeof( Env.PdxDir ), SaveFile );
	// PDX�p�X����⊮
	{
	int l = lstrlen( Env.PdxDir);
	//���{��Ή�
	if ( (Env.PdxDir[l-1] == '\\') &&
		(!_ismbstrail( (const unsigned char *)Env.PdxDir , (const unsigned char *)&Env.PdxDir[l-1]) ) ) {
	} else {
		Env.PdxDir[l] = '\\';
		Env.PdxDir[l+1] = '\0';
	}
	}
	Env.mask = 0x00;

	if (Env.enable == TRUE) mod.FileExtensions = FILE_EXT;
	else					mod.FileExtensions = "\0\0";
}

//�R�[�h�ւ炵
static BOOL WritePrivateProfileInt(LPCSTR lpszSection,LPCSTR lpszKey,int dwValue,LPCSTR lpszFile)
{
	char buf[64];

	wsprintf(buf, "%d", dwValue);
	return(WritePrivateProfileString(lpszSection, lpszKey, buf, lpszFile));
}


//�����ݒ�l�Z�[�u
void EnvSave()
{
	if ( configed ) {
		//�ύX�������̂�
		WritePrivateProfileInt( "Switch", "SampleRate", Env.samprate, SaveFile );
		WritePrivateProfileInt( "Switch", "PCMBuf", Env.pcmbuf, SaveFile );
		WritePrivateProfileInt( "Switch", "Late", Env.late, SaveFile );
		WritePrivateProfileInt( "Switch", "PCM8", Env.pcm8use, SaveFile );
		WritePrivateProfileInt( "Switch", "Priority", Env.priority, SaveFile );
		WritePrivateProfileInt( "Switch", "Loop", Env.Loop, SaveFile );
		WritePrivateProfileInt( "Switch", "Qtime", Env.qtime, SaveFile );
		WritePrivateProfileInt( "Switch", "TotalVolume", Env.TotalVolume, SaveFile );
		WritePrivateProfileInt( "Switch", "FadeTime", Env.FadeTime, SaveFile );
		WritePrivateProfileInt( "Switch", "AGC", Env.agc, SaveFile );
		WritePrivateProfileInt( "Switch", "ROMEO", Env.ROMEO, SaveFile );
		WritePrivateProfileInt( "Switch", "IgnoreError", Env.ignoreErr, SaveFile );
		WritePrivateProfileInt( "Switch", "Enable", Env.enable, SaveFile );
		WritePrivateProfileString( "Path", "PDX", Env.PdxDir, SaveFile );
	}
}

// �v���p�e�B�V�[�g�p�R�[���o�b�N(X68Sound.dll�p)
static BOOL CALLBACK ConfigDlgFunc(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char strtemp[MAX_PATH];
	int i;

	switch(message) {
	  case WM_INITDIALOG:
		// ���ڂ̏�����(samprate)
		::SendDlgItemMessage(hdwnd, IDC_FREQUENCY, CB_SETEXTENDEDUI, TRUE, 0);
		for (i = 0; freqlist[i].key != NULL; i++) {
			::SendDlgItemMessage(hdwnd, IDC_FREQUENCY, CB_ADDSTRING, 0, (LPARAM)freqlist[i].key);
		}
		for (i = 0; freqlist[i].key != NULL; i++) {
			if (Env.samprate == freqlist[i].num) break;
		}
		if (freqlist[i].key == NULL) i = 0;
		::SendDlgItemMessage(hdwnd, IDC_FREQUENCY, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)freqlist[i].key);
		// ���ڂ̏�����(pcmbuf)
		wsprintf(strtemp, "%d", Env.pcmbuf);
		::SetDlgItemText(hdwnd, IDC_EDIT5, strtemp);
		// ���ڂ̏�����(late)
		wsprintf(strtemp, "%d", Env.late);
		::SetDlgItemText(hdwnd, IDC_EDIT6, strtemp);
		// �g�[�^���{�����[��
		::SendDlgItemMessage(hdwnd, IDC_SLIDER2, TBM_SETRANGE, 0, (LPARAM)MAKELONG(50, 300));
		::SendDlgItemMessage(hdwnd, IDC_SLIDER2, TBM_SETTICFREQ, 50, 0);
		::SendDlgItemMessage(hdwnd, IDC_SLIDER2, TBM_SETPOS, TRUE, Env.TotalVolume );
		// ���ڂ̏�����(PCM8�g�p)
		::SendDlgItemMessage(hdwnd, IDC_CHECK1, BM_SETCHECK, Env.pcm8use, 0);
		// ���ڂ̏�����(ROMEO�g�p)
		::SendDlgItemMessage(hdwnd, IDC_CHECK19, BM_SETCHECK, Env.ROMEO, 0);
		// MIN/MAX �̐ݒ�
		::SendDlgItemMessage(hdwnd, IDC_SPIN4, UDM_SETRANGE, 0, (LPARAM)MAKELONG(200, 1));
		::SendDlgItemMessage(hdwnd, IDC_SPIN5, UDM_SETRANGE, 0, (LPARAM)MAKELONG(500, 1));
		return 1;
	  case WM_NOTIFY:
		PSHNOTIFY *PSHNotify =(PSHNOTIFY*)lParam;
		switch (PSHNotify->hdr.code) {
		  case PSN_APPLY:
			{
				char buf[MAX_PATH];
				// ���ڂ̕ۑ�(samprate)
				::GetDlgItemText(hdwnd, IDC_FREQUENCY, buf, sizeof(buf));
				for (i = 0; freqlist[i].key != NULL; i++) {
					if (lstrcmpi(buf, freqlist[i].key) == 0) break;
				}
				Env.samprate = freqlist[i].num;

				// ���ڂ̕ۑ�(pcmbuf)
				::GetDlgItemText(hdwnd, IDC_EDIT5, strtemp, MAX_PATH);
				Env.pcmbuf = atoi(strtemp);
				if(Env.pcmbuf <   1)  Env.pcmbuf =   1;
				if(Env.pcmbuf > 200)  Env.pcmbuf = 200;

				// ���ڂ̕ۑ�(late)
				::GetDlgItemText(hdwnd, IDC_EDIT6, strtemp, MAX_PATH);
				Env.late = atoi(strtemp);
				if(Env.late <   1)  Env.late =   1;
				if(Env.late > 500)  Env.late = 500;

				// �g�[�^���{�����[��
				Env.TotalVolume = ::SendDlgItemMessage(hdwnd, IDC_SLIDER2, TBM_GETPOS,0,0 );

				// ���ڂ̕ۑ�(pcm8use)
				Env.pcm8use = ::SendDlgItemMessage(hdwnd, IDC_CHECK1, BM_GETCHECK, 0, 0);

				// ���ڂ̕ۑ�(ROMEO)
				Env.ROMEO = ::SendDlgItemMessage(hdwnd, IDC_CHECK19, BM_GETCHECK, 0, 0);
				configed = 1;
				return(1);
			}
			break;
		  case WM_COMMAND:
			switch (LOWORD(wParam)) {
			  case IDC_FREQUENCY:
				if (HIWORD(wParam) == CBN_SELCHANGE) {
					PropSheet_Changed(hdwnd, PropSheet_GetCurrentPageHwnd(hdwnd));
					return TRUE;
				}
				break;
			}
			break;
		}
	}
	return(0);
}

// �v���p�e�B�V�[�g�p�R�[���o�b�N(PlugIn�p)
static BOOL CALLBACK ConfigDlg2Func(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char strtemp[MAX_PATH];
	int i;

	switch(message) {
	case WM_INITDIALOG:

		// ���ڂ̏�����(PdxDir)
		wsprintf(strtemp, "%s", Env.PdxDir);
		::SetDlgItemText(hdwnd, IDC_EDIT7, strtemp);
		// ���ڂ̏�����(���[�v��)
		wsprintf(strtemp, "%d", Env.Loop);
		::SetDlgItemText(hdwnd, IDC_EDIT2, strtemp);
		// ���ڂ̏�����(qtime)
		wsprintf(strtemp, "%d", Env.qtime);
		::SetDlgItemText(hdwnd, IDC_EDIT8, strtemp);
		// ���ڂ̏�����(AGC�g�p)
		::SendDlgItemMessage(hdwnd, IDC_CHECK18, BM_SETCHECK, Env.agc, 0);
		// ���ڂ̏�����(�G���[����)
		::SendDlgItemMessage(hdwnd, IDC_CHECK20, BM_SETCHECK, Env.ignoreErr, 0);
		// ���ڂ̏�����(�L��)
		::SendDlgItemMessage(hdwnd, IDC_ENABLE, BM_SETCHECK, Env.enable, 0);
		// �Đ��D��x
		::SendDlgItemMessage(hdwnd, IDC_PRIORITY, CB_SETEXTENDEDUI, TRUE, 0);
		for (i = 0; priolist[i].key != NULL; i++) {
			::SendDlgItemMessage(hdwnd, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)priolist[i].key);
		}
		for (i = 0; priolist[i].key != NULL; i++) {
			if (Env.priority == priolist[i].num) break;
		}
		if (priolist[i].key == NULL) i = 2;
		::SendDlgItemMessage(hdwnd, IDC_PRIORITY, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)priolist[i].key);
		// �t�F�[�h�^�C��
		::SendDlgItemMessage(hdwnd, IDC_SLIDER3, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 50));
		::SendDlgItemMessage(hdwnd, IDC_SLIDER3, TBM_SETTICFREQ, 10, 0);
		::SendDlgItemMessage(hdwnd, IDC_SLIDER3, TBM_SETPOS, TRUE, Env.FadeTime );
		// MIN/MAX �̐ݒ�
		::SendDlgItemMessage(hdwnd, IDC_SPIN1, UDM_SETRANGE, 0, (LPARAM)MAKELONG(99, 1));
		::SendDlgItemMessage(hdwnd, IDC_SPIN6, UDM_SETRANGE, 0, (LPARAM)MAKELONG(60, 1));
		return(1);

	  case WM_COMMAND :
		switch(LOWORD(wParam)) {
		  case IDC_BUTTON1:
			{
				char aPath[MAX_PATH];
				BROWSEINFO oBrInfo;
				LPITEMIDLIST idlist;
				::ZeroMemory(&oBrInfo, sizeof(BROWSEINFO));
				oBrInfo.hwndOwner = hdwnd;
				oBrInfo.pszDisplayName = aPath;
				oBrInfo.lpszTitle = "PDX�̃f�B���N�g���[���w��";
				oBrInfo.ulFlags = (BIF_EDITBOX | BIF_RETURNONLYFSDIRS);

				idlist = ::SHBrowseForFolder(&oBrInfo);
				if (idlist != NULL) {
					::SHGetPathFromIDList(idlist, Env.PdxDir);
					::CoTaskMemFree(idlist);

					// ���ڂ̏�����(PdxDir)
					wsprintf(strtemp, "%s", Env.PdxDir);
					::SetDlgItemText(hdwnd, IDC_EDIT7, strtemp);
				}
			}
			return(1);
		  case IDC_PRIORITY:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				PropSheet_Changed(hdwnd, PropSheet_GetCurrentPageHwnd(hdwnd));
				return TRUE;
			}
			break;
		}
		break;

	  case WM_NOTIFY:
		PSHNOTIFY *PSHNotify =(PSHNOTIFY*)lParam;
		switch (PSHNotify->hdr.code) {
		  case PSN_APPLY:
			{
				// �Đ��D��x
				char buf[MAX_PATH];
				::GetDlgItemText(hdwnd, IDC_PRIORITY, buf, sizeof(buf));
				for (i = 0; priolist[i].key != NULL; i++) {
					if (lstrcmpi(buf, priolist[i].key) == 0) break;
				}
				Env.priority = priolist[i].num;

				// �t�F�[�h�^�C��
				Env.FadeTime = ::SendDlgItemMessage(hdwnd, IDC_SLIDER3, TBM_GETPOS,0,0 );

				// ���ڂ̕ۑ�(agc)
				Env.agc = ::SendDlgItemMessage(hdwnd, IDC_CHECK18, BM_GETCHECK, 0, 0);

				// ���ڂ̕ۑ�(�G���[����)
				Env.ignoreErr = ::SendDlgItemMessage(hdwnd, IDC_CHECK20, BM_GETCHECK, 0, 0);

				// ���ڂ̕ۑ�(�G���[����)
				Env.enable = ::SendDlgItemMessage(hdwnd, IDC_ENABLE, BM_GETCHECK, 0, 0);
				if (Env.enable == TRUE) mod.FileExtensions = FILE_EXT;
				else					mod.FileExtensions = "\0\0";

				// ���ڂ̕ۑ�(PCM�f�B���N�g��)
				::GetDlgItemText(hdwnd, IDC_EDIT7, strtemp, MAX_PATH);
				lstrcpy( Env.PdxDir, strtemp );
				// PDX�p�X����⊮
				{
					TCHAR *p;
					int l = lstrlen( Env.PdxDir);
					p = &Env.PdxDir[l];
					if ( p == Env.PdxDir ) {
						*(p++) = '.';
						*(p++) = '\\';
						*(p) = '\0';
					} else {
						//���{��Ή�
						if ( (Env.PdxDir[l-1] == '\\') &&
							 (!_ismbstrail( (const unsigned char *)Env.PdxDir , (const unsigned char *)&Env.PdxDir[l-1]) ) ) {
						} else {
							Env.PdxDir[l] = '\\';
							Env.PdxDir[l+1] = '\0';
						}
					}
				}

				// ���ڂ̕ۑ�(���[�v��)
				::GetDlgItemText(hdwnd, IDC_EDIT2, strtemp, MAX_PATH);
				Env.Loop = atoi(strtemp);
				if(Env.Loop <  1)  Env.Loop =  1;
				if(Env.Loop > 99)  Env.Loop = 99;

				// ���ڂ̕ۑ�(q����)
				::GetDlgItemText(hdwnd, IDC_EDIT8, strtemp, MAX_PATH);
				Env.qtime = atoi(strtemp);
				if(Env.qtime <  1)  Env.qtime =  1;
				if(Env.qtime > 60)  Env.qtime = 60;

				configed = 1;
				return(1);
			}
		}
		break;
	}
	return(0);
}

// �v���p�e�B�V�[�g�p�R�[���o�b�N(�T�E���h�}�X�N�p)
static BOOL CALLBACK ConfigDlg3Func(HWND hdwnd, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
	unsigned short mask;
	switch(message) {
	  case WM_INITDIALOG:
		//mask
		::SendDlgItemMessage(hdwnd, IDC_MUTE_FM1,  BM_SETCHECK, (Env.mask >>  0)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_FM2,  BM_SETCHECK, (Env.mask >>  1)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_FM3,  BM_SETCHECK, (Env.mask >>  2)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_FM4,  BM_SETCHECK, (Env.mask >>  3)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_FM5,  BM_SETCHECK, (Env.mask >>  4)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_FM6,  BM_SETCHECK, (Env.mask >>  5)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_FM7,  BM_SETCHECK, (Env.mask >>  6)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_FM8,  BM_SETCHECK, (Env.mask >>  7)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM1, BM_SETCHECK, (Env.mask >>  8)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM2, BM_SETCHECK, (Env.mask >>  9)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM3, BM_SETCHECK, (Env.mask >> 10)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM4, BM_SETCHECK, (Env.mask >> 11)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM5, BM_SETCHECK, (Env.mask >> 12)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM6, BM_SETCHECK, (Env.mask >> 13)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM7, BM_SETCHECK, (Env.mask >> 14)&1, 0);
		::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM8, BM_SETCHECK, (Env.mask >> 15)&1, 0);
		return(1);
	  case WM_COMMAND :
		switch(LOWORD(wParam)) {
		  case IDC_MUTE_FM1: case IDC_MUTE_FM2:
		  case IDC_MUTE_FM3: case IDC_MUTE_FM4:
		  case IDC_MUTE_FM5: case IDC_MUTE_FM6:
		  case IDC_MUTE_FM7: case IDC_MUTE_FM8:
		  case IDC_MUTE_PCM1: case IDC_MUTE_PCM2:
		  case IDC_MUTE_PCM3: case IDC_MUTE_PCM4:
		  case IDC_MUTE_PCM5: case IDC_MUTE_PCM6:
		  case IDC_MUTE_PCM7: case IDC_MUTE_PCM8:
			mask = 0;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_FM1,  BM_GETCHECK, 0, 0) << 0;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_FM2,  BM_GETCHECK, 0, 0) << 1;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_FM3,  BM_GETCHECK, 0, 0) << 2;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_FM4,  BM_GETCHECK, 0, 0) << 3;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_FM5,  BM_GETCHECK, 0, 0) << 4;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_FM6,  BM_GETCHECK, 0, 0) << 5;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_FM7,  BM_GETCHECK, 0, 0) << 6;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_FM8,  BM_GETCHECK, 0, 0) << 7;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM1, BM_GETCHECK, 0, 0) << 8;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM2, BM_GETCHECK, 0, 0) << 9;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM3, BM_GETCHECK, 0, 0) << 10;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM4, BM_GETCHECK, 0, 0) << 11;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM5, BM_GETCHECK, 0, 0) << 12;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM6, BM_GETCHECK, 0, 0) << 13;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM7, BM_GETCHECK, 0, 0) << 14;
			mask |= ::SendDlgItemMessage(hdwnd, IDC_MUTE_PCM8, BM_GETCHECK, 0, 0) << 15;
			Env.mask = mask;
			SetChannelPlayFlagMDX(Env.mask);
			return(1);
		}
	}
	return(0);
}

// �ݒ�p�v���p�e�B�V�[�g�쐬
void config(HWND hwndParent)
{
	PROPSHEETPAGE aPSPage[3];
	PROPSHEETHEADER oPSHeader;

	::ZeroMemory(aPSPage, sizeof(aPSPage));
	for (int i=0; i<3; i++) {
		aPSPage[i].dwSize      = sizeof(PROPSHEETPAGE);
		aPSPage[i].dwFlags     = PSP_DEFAULT;
		aPSPage[i].hInstance   = mod.hDllInstance;
	}
	aPSPage[0].pfnDlgProc = ConfigDlgFunc;
	aPSPage[1].pfnDlgProc = ConfigDlg2Func;
	aPSPage[2].pfnDlgProc = ConfigDlg3Func;
	aPSPage[0].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG1);
	aPSPage[1].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG2);
	aPSPage[2].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG3);

	::ZeroMemory(&oPSHeader, sizeof(oPSHeader));
	oPSHeader.dwSize      = sizeof(PROPSHEETHEADER);
	oPSHeader.dwFlags     = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
	oPSHeader.hwndParent  = hwndParent;
	oPSHeader.hInstance   = mod.hDllInstance;
	oPSHeader.pszCaption  = (LPSTR)CONFIG_TITLE;
	oPSHeader.nPages      = sizeof(aPSPage) / sizeof(PROPSHEETPAGE);
	oPSHeader.ppsp        = (LPCPROPSHEETPAGE) &aPSPage;

	::PropertySheet(&oPSHeader);
}
