//
// Winamp .MDX (X68000 YM2151 & MSM6258 AUDIO FILE) input plug-in
//
// use-dll    "MXDRV.DLL","X68Sound.DLL"
// use-header "depend.h","mxdrv.h"
//            "in2.h","out.h"
// Reference       "mxp.c"
//
#include <windows.h>
#include <shlobj.h>
#include <mbstring.h>
#include <tchar.h>
#include <stdio.h>

#include "in2.h"
#include "resource.h"
#include "in_mdx.h"

struct env_t Env;						// Operating Conditions
static int configed;					// Yes configuration changes
static TCHAR SaveFile[MAX_PATH];		// Configuration file full path

#define CONFIG_TITLE "Mdx Plug Setting"

typedef struct {
	TCHAR *key;
	int num;
} KeyInit;

static KeyInit freqlist[] = {
	{ TEXT("96000"),	SOUND_96K },
	{ TEXT("88200"),	SOUND_88K },
	{ TEXT("62500"),	SOUND_62K },
	{ TEXT("48000"),	SOUND_48K },
	{ TEXT("44100"),	SOUND_44K },
	{ TEXT("22050"),	SOUND_22K },
	{ NULL,				SOUND_22K },
};

static KeyInit priolist[] = {
	{ TEXT("Highest"),	THREAD_PRIORITY_HIGHEST },
	{ TEXT("Higher"),	THREAD_PRIORITY_ABOVE_NORMAL },
	{ TEXT("Normal"),	THREAD_PRIORITY_NORMAL },
	{ TEXT("Lower"),	THREAD_PRIORITY_BELOW_NORMAL },
	{ TEXT("Lowest"),	THREAD_PRIORITY_LOWEST },
	{ NULL,				THREAD_PRIORITY_NORMAL },
};

//Condition setting value load
void EnvLoad( TCHAR *fn )
{
	lstrcpy( SaveFile, fn );
	FILE *stream = NULL;
	// make sure the file does not already exist
	if (_tfopen_s(&stream, SaveFile, TEXT("r")) != 0)
	{
		// create file with encoding UTF-16LE
		_tfopen_s(&stream, SaveFile, TEXT("w, ccs=UTF-16LE"));
	}
	if (stream != NULL) fclose(stream);

	configed = 0;
	Env.samprate = GetPrivateProfileInt( TEXT("Switch"), TEXT("SampleRate"), 22050, SaveFile );
	Env.pcmbuf = GetPrivateProfileInt( TEXT("Switch"), TEXT("PCMBuf"), 5, SaveFile );
	Env.late = GetPrivateProfileInt( TEXT("Switch"), TEXT("Late"), 500, SaveFile );
	Env.pcm8use = GetPrivateProfileInt( TEXT("Switch"), TEXT("PCM8"), 1, SaveFile );
	Env.priority = GetPrivateProfileInt( TEXT("Switch"), TEXT("Priority"), 50, SaveFile );
	Env.Loop = GetPrivateProfileInt( TEXT("Switch"), TEXT("Loop"), 3, SaveFile );
	Env.qtime = GetPrivateProfileInt( TEXT("Switch"), TEXT("Qtime"), 2, SaveFile );
	Env.TotalVolume = GetPrivateProfileInt( TEXT("Switch"), TEXT("TotalVolume"), 100, SaveFile );
	Env.FadeTime = GetPrivateProfileInt( TEXT("Switch"), TEXT("FadeTime"), 19, SaveFile );
	Env.agc = GetPrivateProfileInt( TEXT("Switch"), TEXT("AGC"), 0, SaveFile );
	Env.ROMEO = GetPrivateProfileInt( TEXT("Switch"), TEXT("ROMEO"), 0, SaveFile );
	Env.ignoreErr = GetPrivateProfileInt( TEXT("Switch"), TEXT("IgnoreError"), 1, SaveFile );
	Env.enable = (BOOL)GetPrivateProfileInt( TEXT("Switch"), TEXT("Enable"), 1, SaveFile );
	GetPrivateProfileString( TEXT("Path"), TEXT("PDX"), TEXT("c:\\"), Env.PdxDir, sizeof( Env.PdxDir ), SaveFile );
	// PDX Complements the path name
	{
	int l = lstrlen( Env.PdxDir);
	//Japanese support
	if ( (Env.PdxDir[l-1] == '\\') ) {
	} else {
		Env.PdxDir[l] = '\\';
		Env.PdxDir[l+1] = '\0';
	}
	}
	Env.mask = 0x00;

	if (Env.enable == TRUE) mod.FileExtensions = FILE_EXT;
	else					mod.FileExtensions = "\0\0";
}

//Likelihood to the code
static BOOL WritePrivateProfileInt(LPCTSTR lpszSection, LPCTSTR lpszKey,int dwValue, LPCTSTR lpszFile)
{
	TCHAR buf[64];

	wsprintf(buf, TEXT("%d"), dwValue);
	return(WritePrivateProfileString(lpszSection, lpszKey, buf, lpszFile));
}


//Condition setting value save
void EnvSave()
{
	if ( configed ) {
		//Only when you have changed
		WritePrivateProfileInt( TEXT("Switch"), TEXT("SampleRate"), Env.samprate, SaveFile );
		WritePrivateProfileInt( TEXT("Switch"), TEXT("PCMBuf"), Env.pcmbuf, SaveFile );
		WritePrivateProfileInt( TEXT("Switch"), TEXT("Late"), Env.late, SaveFile );
		WritePrivateProfileInt( TEXT("Switch"), TEXT("PCM8"), Env.pcm8use, SaveFile );
		WritePrivateProfileInt( TEXT("Switch"), TEXT("Priority"), Env.priority, SaveFile );
		WritePrivateProfileInt( TEXT("Switch"), TEXT("Loop"), Env.Loop, SaveFile );
		WritePrivateProfileInt( TEXT("Switch"), TEXT("Qtime"), Env.qtime, SaveFile );
		WritePrivateProfileInt( TEXT("Switch"), TEXT("TotalVolume"), Env.TotalVolume, SaveFile );
		WritePrivateProfileInt( TEXT("Switch"), TEXT("FadeTime"), Env.FadeTime, SaveFile );
		WritePrivateProfileInt( TEXT("Switch"), TEXT("AGC"), Env.agc, SaveFile );
		WritePrivateProfileInt( TEXT("Switch"), TEXT("ROMEO"), Env.ROMEO, SaveFile );
		WritePrivateProfileInt( TEXT("Switch"), TEXT("IgnoreError"), Env.ignoreErr, SaveFile );
		WritePrivateProfileInt( TEXT("Switch"), TEXT("Enable"), Env.enable, SaveFile );
		WritePrivateProfileString( TEXT("Path"), TEXT("PDX"), Env.PdxDir, SaveFile );
	}
}

// Property sheet callback(X68Sound.dllUse)
static BOOL CALLBACK ConfigDlgFunc(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR strtemp[MAX_PATH];
	int i;

	switch(message) {
	  case WM_INITDIALOG:
		// Initialization of the item(samprate)
		::SendDlgItemMessage(hdwnd, IDC_FREQUENCY, CB_SETEXTENDEDUI, TRUE, 0);
		for (i = 0; freqlist[i].key != NULL; i++) {
			::SendDlgItemMessage(hdwnd, IDC_FREQUENCY, CB_ADDSTRING, 0, (LPARAM)freqlist[i].key);
		}
		for (i = 0; freqlist[i].key != NULL; i++) {
			if (Env.samprate == freqlist[i].num) break;
		}
		if (freqlist[i].key == NULL) i = 0;
		::SendDlgItemMessage(hdwnd, IDC_FREQUENCY, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)freqlist[i].key);
		// Initialization of the item(pcmbuf)
		wsprintf(strtemp, TEXT("%d"), Env.pcmbuf);
		::SetDlgItemText(hdwnd, IDC_EDIT5, strtemp);
		// Initialization of the item(late)
		wsprintf(strtemp, TEXT("%d"), Env.late);
		::SetDlgItemText(hdwnd, IDC_EDIT6, strtemp);
		// Total volume
		::SendDlgItemMessage(hdwnd, IDC_SLIDER2, TBM_SETRANGE, 0, (LPARAM)MAKELONG(50, 300));
		::SendDlgItemMessage(hdwnd, IDC_SLIDER2, TBM_SETTICFREQ, 50, 0);
		::SendDlgItemMessage(hdwnd, IDC_SLIDER2, TBM_SETPOS, TRUE, Env.TotalVolume );
		// Initialization of the item(PCM8Use)
		::SendDlgItemMessage(hdwnd, IDC_CHECK1, BM_SETCHECK, Env.pcm8use, 0);
		// Initialization of the item(ROMEOUse)
		::SendDlgItemMessage(hdwnd, IDC_CHECK19, BM_SETCHECK, Env.ROMEO, 0);
		// MIN/MAX Setting of
		::SendDlgItemMessage(hdwnd, IDC_SPIN4, UDM_SETRANGE, 0, (LPARAM)MAKELONG(200, 1));
		::SendDlgItemMessage(hdwnd, IDC_SPIN5, UDM_SETRANGE, 0, (LPARAM)MAKELONG(500, 1));
		return 1;
	  case WM_NOTIFY:
		PSHNOTIFY *PSHNotify =(PSHNOTIFY*)lParam;
		switch (PSHNotify->hdr.code) {
		  case PSN_APPLY:
			{
				TCHAR buf[MAX_PATH];
				// Save item(samprate)
				::GetDlgItemText(hdwnd, IDC_FREQUENCY, buf, sizeof(buf));
				for (i = 0; freqlist[i].key != NULL; i++) {
					if (lstrcmpi(buf, freqlist[i].key) == 0) break;
				}
				Env.samprate = freqlist[i].num;

				// Save item(pcmbuf)
				::GetDlgItemText(hdwnd, IDC_EDIT5, strtemp, MAX_PATH);
				Env.pcmbuf = _wtoi(strtemp);
				if(Env.pcmbuf <   1)  Env.pcmbuf =   1;
				if(Env.pcmbuf > 200)  Env.pcmbuf = 200;

				// Save item(late)
				::GetDlgItemText(hdwnd, IDC_EDIT6, strtemp, MAX_PATH);
				Env.late = _wtoi(strtemp);
				if(Env.late <   1)  Env.late =   1;
				if(Env.late > 500)  Env.late = 500;

				// Total volume
				Env.TotalVolume = ::SendDlgItemMessage(hdwnd, IDC_SLIDER2, TBM_GETPOS,0,0 );

				// Save item(pcm8use)
				Env.pcm8use = ::SendDlgItemMessage(hdwnd, IDC_CHECK1, BM_GETCHECK, 0, 0);

				// Save item(ROMEO)
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

// Property sheet callback(PlugInUse)
static BOOL CALLBACK ConfigDlg2Func(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR strtemp[MAX_PATH];
	int i;

	switch(message) {
	case WM_INITDIALOG:

		// 項目の初期化(PdxDir)
		wsprintf(strtemp, TEXT("%s"), Env.PdxDir);
		::SetDlgItemText(hdwnd, IDC_EDIT7, strtemp);
		// 項目の初期化(ループ回数)
		wsprintf(strtemp, TEXT("%d"), Env.Loop);
		::SetDlgItemText(hdwnd, IDC_EDIT2, strtemp);
		// 項目の初期化(qtime)
		wsprintf(strtemp, TEXT("%d"), Env.qtime);
		::SetDlgItemText(hdwnd, IDC_EDIT8, strtemp);
		// 項目の初期化(AGC使用)
		::SendDlgItemMessage(hdwnd, IDC_CHECK18, BM_SETCHECK, Env.agc, 0);
		// 項目の初期化(エラー無視)
		::SendDlgItemMessage(hdwnd, IDC_CHECK20, BM_SETCHECK, Env.ignoreErr, 0);
		// 項目の初期化(有効)
		::SendDlgItemMessage(hdwnd, IDC_ENABLE, BM_SETCHECK, Env.enable, 0);
		// 再生優先度
		::SendDlgItemMessage(hdwnd, IDC_PRIORITY, CB_SETEXTENDEDUI, TRUE, 0);
		for (i = 0; priolist[i].key != NULL; i++) {
			::SendDlgItemMessage(hdwnd, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)priolist[i].key);
		}
		for (i = 0; priolist[i].key != NULL; i++) {
			if (Env.priority == priolist[i].num) break;
		}
		if (priolist[i].key == NULL) i = 2;
		::SendDlgItemMessage(hdwnd, IDC_PRIORITY, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)priolist[i].key);
		// フェードタイム
		::SendDlgItemMessage(hdwnd, IDC_SLIDER3, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 50));
		::SendDlgItemMessage(hdwnd, IDC_SLIDER3, TBM_SETTICFREQ, 10, 0);
		::SendDlgItemMessage(hdwnd, IDC_SLIDER3, TBM_SETPOS, TRUE, Env.FadeTime );
		// MIN/MAX の設定
		::SendDlgItemMessage(hdwnd, IDC_SPIN1, UDM_SETRANGE, 0, (LPARAM)MAKELONG(99, 1));
		::SendDlgItemMessage(hdwnd, IDC_SPIN6, UDM_SETRANGE, 0, (LPARAM)MAKELONG(60, 1));
		return(1);

	  case WM_COMMAND :
		switch(LOWORD(wParam)) {
		  case IDC_BUTTON1:
			{
				TCHAR aPath[MAX_PATH];
				BROWSEINFO oBrInfo;
				LPITEMIDLIST idlist;
				::ZeroMemory(&oBrInfo, sizeof(BROWSEINFO));
				oBrInfo.hwndOwner = hdwnd;
				oBrInfo.pszDisplayName = aPath;
				oBrInfo.lpszTitle = TEXT("Select PDX directory");
				oBrInfo.ulFlags = (BIF_EDITBOX | BIF_RETURNONLYFSDIRS);

				idlist = ::SHBrowseForFolder(&oBrInfo);
				if (idlist != NULL) {
					::SHGetPathFromIDList(idlist, Env.PdxDir);
					::CoTaskMemFree(idlist);

					// 項目の初期化(PdxDir)
					wsprintf(strtemp, TEXT("%s"), Env.PdxDir);
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
				// 再生優先度
				TCHAR buf[MAX_PATH];
				::GetDlgItemText(hdwnd, IDC_PRIORITY, buf, sizeof(buf));
				for (i = 0; priolist[i].key != NULL; i++) {
					if (lstrcmpi(buf, priolist[i].key) == 0) break;
				}
				Env.priority = priolist[i].num;

				// フェードタイム
				Env.FadeTime = ::SendDlgItemMessage(hdwnd, IDC_SLIDER3, TBM_GETPOS,0,0 );

				// 項目の保存(agc)
				Env.agc = ::SendDlgItemMessage(hdwnd, IDC_CHECK18, BM_GETCHECK, 0, 0);

				// 項目の保存(エラー無視)
				Env.ignoreErr = ::SendDlgItemMessage(hdwnd, IDC_CHECK20, BM_GETCHECK, 0, 0);

				// 項目の保存(エラー無視)
				Env.enable = ::SendDlgItemMessage(hdwnd, IDC_ENABLE, BM_GETCHECK, 0, 0);
				if (Env.enable == TRUE) mod.FileExtensions = FILE_EXT;
				else					mod.FileExtensions = "\0\0";

				// 項目の保存(PCMディレクトリ)
				::GetDlgItemText(hdwnd, IDC_EDIT7, strtemp, MAX_PATH);
				lstrcpy( Env.PdxDir, strtemp );
				// PDXパス名を補完
				{
					TCHAR *p;
					int l = lstrlen( Env.PdxDir);
					p = &Env.PdxDir[l];
					if ( p == Env.PdxDir ) {
						*(p++) = '.';
						*(p++) = '\\';
						*(p) = '\0';
					} else {
						//日本語対応
						if ( (Env.PdxDir[l-1] == '\\') &&
							 (!_ismbstrail( (const unsigned char *)Env.PdxDir , (const unsigned char *)&Env.PdxDir[l-1]) ) ) {
						} else {
							Env.PdxDir[l] = '\\';
							Env.PdxDir[l+1] = '\0';
						}
					}
				}

				// 項目の保存(ループ回数)
				::GetDlgItemText(hdwnd, IDC_EDIT2, strtemp, MAX_PATH);
				Env.Loop = _wtoi(strtemp);
				if(Env.Loop <  1)  Env.Loop =  1;
				if(Env.Loop > 99)  Env.Loop = 99;

				// 項目の保存(q時間)
				::GetDlgItemText(hdwnd, IDC_EDIT8, strtemp, MAX_PATH);
				Env.qtime = _wtoi(strtemp);
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

// プロパティシート用コールバック(サウンドマスク用)
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

// 設定用プロパティシート作成
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
	oPSHeader.pszCaption  = TEXT(CONFIG_TITLE);
	oPSHeader.nPages      = sizeof(aPSPage) / sizeof(PROPSHEETPAGE);
	oPSHeader.ppsp        = (LPCPROPSHEETPAGE) &aPSPage;

	::PropertySheet(&oPSHeader);
}

