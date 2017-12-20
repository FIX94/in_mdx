//
// Winamp .MDX (X68000 YM2151 & MSM6258 AUDIO FILE) input plug-in
//
// use-dll    "MXDRV.DLL","X68Sound.DLL"
// use-header "depend.h","mxdrv.h"
//            "in2.h","out.h"
// �Q�l       "mxp.c"
//

//////////////////////////////////////////////////
// include
#include <windows.h>
#include <shlwapi.h>
#include <mbstring.h>

#include "in2.h"
#define __MXDRV_LOADMODULE
#include "mxdrv.h"
#include "resource.h"
#include "in_mdx.h"
#include "lzx042.h"

//////////////////////////////////////////////////
// define
#define FILLPOLL 20					// ��荞�ݎ���[ms]
#define SEC 1000					// second[ms]
#define MIN 60000					// minute[ms]
#define WAVEBUF 4096				// WAVE�p�o�b�t�@�T�C�Y[byte]
#define MDX_TITLE_MAX 256			// MDX�^�C�g���ő啶����
#define MXDRV_DLL_NAME "mxdrv.dll"	// DLL��
#define WM_WA_EOF WM_USER+2			// �I�[�ɒB��������Winamp�ɒʒm���郁�b�Z�[�W

//////////////////////////////////////////////////
// mxdrv work
#define GetLoopCount() (g_mxwork->L002246)
#define TimerB2msec(n) ((n)*(1024.0*(256-g_mxwork->L001e0c)/4000.0))
#define IsPlayEnd() (g_mxwork->L001e13 != 0)
#define PlayTime() (g_mxwork->PLAYTIME)

//////////////////////////////////////////////////
// DLL�Ăяo���p( "mxdrv.dll" )
int (*MXDRV_Start)( int, int, int, int, int, int, int);
void (*MXDRV_End)( void );
void (*MXDRV_Play)( void *mdx, DWORD mdxsize, void *pdx, DWORD pdxsize );
void (*MXDRV)( X68REG *reg );
void volatile *(*MXDRV_GetWork)( int i );
int (*MXDRV_GetPCM)( void *, int );
DWORD (*MXDRV_MeasurePlayTime)( void *mdx, DWORD mdxsize, void *pdx, DWORD pdxsize, int loop, int fadeout );
void (*MXDRV_PlayAt)( DWORD playat, int loop, int fadeout );
int (*MXDRV_TotalVolume)( int );

//////////////////////////////////////////////////
// �O���[�o���ϐ�
TCHAR g_MDXTitle[MDX_TITLE_MAX];				//
MXWORK_GLOBAL *g_mxwork;						// MXDRV.DLL's ���[�N�G���A
int g_paused = 0;								// are we paused?
DWORD g_PlayTime = 0;							//
int g_seek_time = 0;							//
HINSTANCE g_hinstMXDRV = NULL;					//
HANDLE g_hSeekEvent = NULL;						// �V�[�N�p�C�x���g�n���h��
BOOL g_bMxdrvStarted = FALSE;					// �풓����
BOOL g_bDecodeThread = FALSE;					// �Đ��X���b�h�����t���O
HANDLE g_hThreadHandle = NULL;					// the handle to the decode thread

//////////////////////////////////////////////////
// WINAMP�N������1�񂾂�������
static void init()
{
	TCHAR aIniFileName[MAX_PATH];

	//ini�t�@�C���������
	::GetModuleFileName(mod.hDllInstance, aIniFileName, MAX_PATH);
	PathRemoveFileSpec(aIniFileName);
	PathAppend(aIniFileName, "in_mdx.ini");

	//ini�t�@�C������ݒ�����[�h
	EnvLoad(aIniFileName);

	g_bMxdrvStarted = FALSE;

	// DLL�A�^�b�`
	g_hinstMXDRV = ::LoadLibrary(MXDRV_DLL_NAME);
	if (g_hinstMXDRV == NULL) {
		::MessageBox(::GetDesktopWindow(),
		"mxdrv.dll�̓ǂݍ��݂Ɏ��s���܂����B", DLL_NAME, MB_ICONSTOP|MB_OK);
		return;
	}
	MXDRV_Start   = (int (*)(int, int, int, int, int, int, int))GetProcAddress(g_hinstMXDRV, "MXDRV_Start");
	MXDRV_End     = (void (*)(void))GetProcAddress(g_hinstMXDRV, "MXDRV_End");
	MXDRV_GetPCM  = (int (*)(void *, int))GetProcAddress(g_hinstMXDRV, "MXDRV_GetPCM");
	MXDRV_Play    = (void (*)(void *, DWORD, void *, DWORD))GetProcAddress(g_hinstMXDRV, "MXDRV_Play");
	MXDRV         = (void (*)(X68REG *))GetProcAddress(g_hinstMXDRV, "MXDRV");
	MXDRV_GetWork = (void volatile *(*)(int))GetProcAddress(g_hinstMXDRV, "MXDRV_GetWork");
	MXDRV_MeasurePlayTime = (DWORD(*)(void *, DWORD, void *, DWORD, int, int))GetProcAddress(g_hinstMXDRV, "MXDRV_MeasurePlayTime");
	MXDRV_PlayAt  = (void(*)(DWORD, int, int))GetProcAddress(g_hinstMXDRV, "MXDRV_PlayAt");
	MXDRV_TotalVolume = (int(*)(int))GetProcAddress(g_hinstMXDRV, "MXDRV_TotalVolume");

	// MXDRV�̃��[�N�G���A�擾
	g_mxwork = (MXWORK_GLOBAL *)MXDRV_GetWork(MXDRV_WORK_GLOBAL);

	//�C�x���g�쐬
	g_hSeekEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	return;
}

//////////////////////////////////////////////////
// WINAMP�I�����ɌĂяo�������
static void quit()
{
	if (g_bMxdrvStarted == TRUE) {
		MXDRV_Stop();
		MXDRV_End();
	}
	if (g_hinstMXDRV != NULL) {
		::FreeLibrary(g_hinstMXDRV);
	}
	// ini�t�@�C���ւ̕ۑ�
	EnvSave();
}

#if 1
//////////////////////////////////////////////////
//
int GetDwordBE(char *p)
{
	int ret;
	ret  = ((int)(unsigned char)p[0]) << 0x18;
	ret |= ((int)(unsigned char)p[1]) << 0x10;
	ret |= ((int)(unsigned char)p[2]) << 0x08;
	ret |= ((int)(unsigned char)p[3]) << 0x00;
	return ret;
}

//////////////////////////////////////////////////
//
int GetWordBE(char *p)
{
	int ret;
	ret  = ((int)(unsigned char)p[0]) << 0x08;
	ret |= ((int)(unsigned char)p[1]) << 0x00;
	return ret;
}

//////////////////////////////////////////////////
//
int IsLzxEncoded(char *p)
{
	if (GetDwordBE(p + 4) != 0x4c5a5820) return 0;
	return GetDwordBE(p + 0x12);
}

//////////////////////////////////////////////////
// MXP���q��
void SetChannelPlayFlagMDX(
	DWORD status
) {
	g_mxwork->L001e1c = (UWORD)status;
}

//////////////////////////////////////////////////
// MXP���q��
static void ExtractPath(
	TCHAR *p
) {
	TCHAR c;
	TCHAR *p2 = NULL;

	do {
		c = *(p);
		if ( c == '\0' ) break;
		if ( _ismbblead(c) ) {
			p = (TCHAR *)_mbsinc( (const unsigned char *)p );
		} else {
			if ( c == '\\' ) {
				p2 = p;
			}
			p++;
		}
	} while (!0);
	p2[1] = '\0';
}


//////////////////////////////////////////////////
// PDX��T����΃p�X��Ԃ�
char *SearchPdx(char *mdxpath , char *pdxpath , unsigned char *fn)
{
	static TCHAR pdxfilename[MAX_PATH+1];

	HANDLE hPdxFile = INVALID_HANDLE_VALUE;
	for (int i=1; hPdxFile == INVALID_HANDLE_VALUE; i++) {
		switch(i) {
		  case 1:
			// mdx�p�X
			  wsprintf(pdxfilename, "%s%s", mdxpath, fn);
			break;
		  case 2:
			// mdx�p�X(.pdx�t����)
			wsprintf(pdxfilename, "%s%s.pdx", mdxpath, fn);
			break;
		  case 3:
			// pdx�p�X
			wsprintf(pdxfilename, "%s%s", pdxpath, fn);
			break;
		  case 4:
			// pdx�p�X(.pdx�t����)
			wsprintf(pdxfilename, "%s%s.pdx", pdxpath, fn);
		  default:
			// �����ł���
			return(NULL);
		}

		hPdxFile = ::CreateFile(
			pdxfilename,
			GENERIC_READ,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	}
	::CloseHandle(hPdxFile);

	return(pdxfilename);
}

//////////////////////////////////////////////////
// X68�����R�[�h�ϊ�
void ConvTitle( unsigned char *mdxbuf )
{
	int i , j;

	j = 0;
	for (i = 0 ; ;) {
		if (mdxbuf[i+j] == 0) break;
		if (_ismbslead( (const unsigned char *)mdxbuf , (const unsigned char *)(mdxbuf+i+j))) {
			if ( (mdxbuf[i+j] == 0x80) || // 1/2�p
			     (mdxbuf[i+j] == 0xf0) || // 1/4��t��
			     (mdxbuf[i+j] == 0xf1) ||
			     (mdxbuf[i+j] == 0xf2) || // 1/4���t��
				 (mdxbuf[i+j] == 0xf3) ) {
				j++;
				continue;
			}
		} else if ( !_ismbstrail( (const unsigned char *)mdxbuf , (const unsigned char *)(mdxbuf+i+j)) ) {
			if (mdxbuf[ i+j ] < 0x20) mdxbuf[ i+j ] = 0x20;
		}
		mdxbuf[i] = mdxbuf[i+j];
		i++;
	}
	mdxbuf[i]=0;
}

enum ePlay_err{
	err_readmdx,
	err_readpdx,
	err_brokenmdx,
	err_brokenpdx,
	err_mdxbuf,
	err_pdxbuf,
	err_mxdrv
};

//////////////////////////////////////////////////
// MXP���q��
static int PlayMDX(
	HWND hwnd,
	TCHAR *mdxname
) {
	TCHAR mdxfilename[MAX_PATH+1];
	TCHAR mdxpath[MAX_PATH+1];
	BYTE *p2 = NULL;
	int mdxsize;
	int pdxsize;
	DWORD mdxbodyptr;
	DWORD pdxbodyptr;
	int havepdx;
	TCHAR *dummy;
	int mdxbufsize;
	int pdxbufsize;
	char *pdxfn;
	int mdxfsiz;
	int ret;
	BYTE *mdxbuf;
	BYTE *pdxbuf;
	HANDLE hMdxFile;
	HANDLE hPdxFile;

try
{
	// ������
	mdxbuf = pdxbuf = p2 = NULL;
	pdxfn = NULL;
	dummy = NULL;
	ret = 0;
	havepdx = 0;
	mdxsize = 0;
	pdxsize = 0;
	mdxfsiz = 0;
	pdxbufsize = 0;
	mdxbodyptr = 0;
	pdxbodyptr = 0;
	hMdxFile = NULL;
	hPdxFile = NULL;

	// MDX�p�X���𐶐�
	GetFullPathName(mdxname, sizeof(mdxfilename), mdxfilename, &dummy);
	lstrcpy(mdxpath, mdxfilename);
	ExtractPath(mdxpath);

	// MDX��ǂݍ���
	hMdxFile = ::CreateFile(
						mdxfilename,
						GENERIC_READ,
						FILE_SHARE_READ|FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	if (hMdxFile == NULL) throw err_readmdx;

	DWORD oMdxLen = ::SetFilePointer(hMdxFile, 0, NULL, FILE_END);
	if (oMdxLen != INVALID_SET_FILE_POINTER) {
		// �Œ�64KByte�擾����
		mdxbufsize = max(oMdxLen+(8*1024), 64*1024);
	} else {
		throw err_readmdx;
	}
	mdxbuf = (BYTE*)::GlobalAlloc(GMEM_FIXED, mdxbufsize);
	if (mdxbuf == NULL) throw err_mdxbuf;
	::ZeroMemory(mdxbuf, mdxbufsize);

	::SetFilePointer(hMdxFile, 0, NULL, FILE_BEGIN);
	DWORD oReadSize;
	if (::ReadFile(hMdxFile, mdxbuf+8, mdxbufsize-8, &oReadSize, NULL)) {
		mdxfsiz = mdxsize = oReadSize;
	} else {
		throw err_readmdx;
	}
	::CloseHandle(hMdxFile);
	hMdxFile = NULL;

	// �^�C�g�����X�L�b�v
	{
		BYTE c = 0;
		BYTE *p;
		p = mdxbuf+8;
		while ( mdxsize-- ) {
			c = *(p++);
			if ( c == 0x0d ) break;
			if ( c == 0x0a ) break;
			// "�einal �eantasy �u   = �M���K���b�V���̃e�[�}(^^;) =  	By �֓� ���� (PCM8)"
			// ���̃f�[�^�[���Đ��ł��Ȃ��Ȃ�̂ł��̃`�F�b�N�͎~�߂�
			/*
			if ( c < 0x20 ) {
				if ( c != 0x1b ) throw err_brokenmdx;
			}
			*/
		}
		if ( mdxsize <= 0 ) throw err_brokenmdx;
		p--;
		*(p++) = 0x00;
		if ( ((DWORD)(p))&0x01 ) {
			*(p++) = 0x00;
			mdxsize--;
			if ( mdxsize <= 0 ) throw err_brokenmdx;
		}
		p2 = p;
		if ( c != 0x0d ) {
			while ( mdxsize--, *(p2++) != 0x0d ) {
				if ( mdxsize <= 0 ) throw err_brokenmdx;
			}
		}
	}

	//�^�C�g���I���܂œǂݔ�΂�
	while ( mdxsize--, *(p2++) != 0x1a ) {
		if ( mdxsize <= 0 ) throw err_brokenmdx;
	}
	
	// PDX��ǂݍ���
	if ( *(p2) ) {
		pdxfn = SearchPdx( mdxpath , Env.PdxDir , p2 );
		if ( pdxfn != NULL ) {
			hPdxFile = ::CreateFile(
							pdxfn,
							GENERIC_READ,
							FILE_SHARE_READ|FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
			havepdx = 1;
			DWORD oPdxLen = ::SetFilePointer(hPdxFile, 0, NULL, FILE_END);
			if (oPdxLen != INVALID_SET_FILE_POINTER) {
				// �Œ�1024KByte�擾����
				pdxbufsize = max(oPdxLen+(8*1024), 1024*1024);
			} else {
				throw err_readpdx;
			}
			pdxbuf = (BYTE*)::GlobalAlloc(GMEM_FIXED, pdxbufsize);
			if (pdxbuf == NULL) throw err_pdxbuf;
			::ZeroMemory(pdxbuf, pdxbufsize);

			::SetFilePointer(hPdxFile, 0, NULL, FILE_BEGIN);
			pdxbodyptr = 10;

			DWORD oReadSize;
			if (::ReadFile(hPdxFile, pdxbuf+10, pdxbufsize-10, &oReadSize, NULL)) {
				pdxsize = oReadSize;
			} else {
				throw err_readpdx;
			}
			::CloseHandle(hPdxFile);
			hPdxFile = NULL;

			// PDX��LZX������
			{
				int lzxsize = IsLzxEncoded( (char *)pdxbuf+10 );
				if ( lzxsize != 0 ) {
					char *tmp = new char[ pdxsize ];
					memcpy( tmp , pdxbuf + 10 , pdxsize );
					lzx042( tmp, pdxbuf + 10 );
					delete [] tmp;
					pdxsize = lzxsize + 256;
				}
			}

			// PDX���e�`�F�b�N
			// �w�b�_�[�̍ŏ���PCM�̃A�h���X�ƒ����̊Ԃ�0x0000�𗘗p����
			// ...�̂������\�����ɒl�̓����Ă���PDX���݂���̂Ń`�F�b�N���~�߂�
			// MDX���ŎQ�Ƃ��Ȃ��ԍ��ɃR�����g����Ă�PDX�����邵(��
//			if (((WORD)pdxbuf[10 + 4]) != 0x0000) {
//				throw err_brokenpdx;
//			}

			// PDX��MXDRV�֓n����悤���H����
			pdxbuf[0] = 0x00;
			pdxbuf[1] = 0x00;
			pdxbuf[2] = 0x00;
			pdxbuf[3] = 0x00;
			pdxbuf[4] = (unsigned char)(pdxbodyptr>>8);
			pdxbuf[5] = (unsigned char)pdxbodyptr;
			pdxbuf[6] = 0; //pdxnamelen>>8;
			pdxbuf[7] = 0; //pdxnamelen;
			pdxbuf[8] = 0; //pdxname[0]
			pdxbuf[9] = 0; // |
			pdxsize += 10;
		} else {
			// PDX������Ȃ�����
		}
	}

	// MDX��LZX������
	{
		// PDX FileName Skip
		if (havepdx) {
			while ( mdxsize--, *(p2++) ) {
				if ( mdxsize <= 0 ) throw err_brokenmdx;
			}
			p2--;
		}

		int lzxsize = IsLzxEncoded( (char *)p2+1 );
		if (lzxsize != 0) {
			char *tmp = new char[ mdxsize ];
			memcpy( tmp , p2+1 , mdxsize );
			lzx042( tmp, p2+1 );
			delete [] tmp;
			mdxfsiz = mdxsize = lzxsize + 256;
		}
	}

	// MXDRV�N��
	if ( MXDRV_Start(Env.samprate, 0, Env.pcmbuf, Env.late, mdxbufsize, pdxbufsize, Env.ROMEO) ) {
		throw err_mxdrv;
	}
	//�g�[�^���{�����[���ݒ�(MXDRV v1.30�ȍ~)
//	if ( MXDRV_TotalVolume != NULL )
		MXDRV_TotalVolume( (int)(256.0 * Env.TotalVolume / 100.0) );
	g_bMxdrvStarted = TRUE;

	// PCM8�̎g�p������
	UBYTE *pcm8ptr;
	pcm8ptr = (UBYTE *)MXDRV_GetWork(MXDRV_WORK_PCM8);
	*pcm8ptr = ( Env.pcm8use ? 1 : 0 );

	// PDX�t�@�C�������X�L�b�v����
	while ( mdxsize--, *(p2++) ) {
		if ( mdxsize <= 0 ) throw err_brokenmdx;
	}
	// MDX��MXDRV�֓n����悤���H����
	mdxbodyptr = p2 - &mdxbuf[0];
	mdxbuf[0] = 0x00;
	mdxbuf[1] = 0x00;
	mdxbuf[2] = (havepdx ? 0 : 0xff);
	mdxbuf[3] = (havepdx ? 0 : 0xff);
	mdxbuf[4] = (unsigned char)(mdxbodyptr>>8);
	mdxbuf[5] = (unsigned char)mdxbodyptr;
	mdxbuf[6] = 0x00;
	mdxbuf[7] = 0x08;
	mdxsize += 8;
	lstrcpy( g_MDXTitle, (TCHAR *)&mdxbuf[8] );
	ConvTitle( (unsigned char *)g_MDXTitle );

	// MXDRV�Ăяo��(�Ȏ��Ԏ擾)
	g_PlayTime = MXDRV_MeasurePlayTime( mdxbuf, mdxfsiz+8, pdxbuf, pdxsize, Env.Loop, TRUE );

	// MXDRV�Ăяo��(�Đ��J�n)
	MXDRV_Play( mdxbuf, mdxfsiz+8, pdxbuf, pdxsize );

	// �`�����l���}�X�N�ݒ�
	SetChannelPlayFlagMDX(Env.mask);

} catch(ePlay_err iREASON) {
	TCHAR errmsg[MAX_PATH+256];
	switch ( iREASON ) {
	case err_readmdx:
		wsprintf( errmsg, "MDX�t�@�C��(%s)�̓ǂݍ��݂Ɏ��s���܂����B", mdxfilename );
		break;
	case err_readpdx:
		wsprintf( errmsg, "PDX�t�@�C��(%s)�̓ǂݍ��݂Ɏ��s���܂����B", pdxfn);
		break;
	case err_brokenmdx:
		wsprintf( errmsg, "�t�@�C��(%s)��MDX�łȂ����A���Ă��܂��B", mdxfilename );
		break;
	case err_brokenpdx:
		wsprintf( errmsg, "�t�@�C��(%s)��PDX�łȂ����A���Ă��܂��B", pdxfn );
		break;
	case err_mdxbuf:
		wsprintf( errmsg, "�t�@�C��(%s)��ǂ�MDX�o�b�t�@������܂���B", mdxfilename );
		break;
	case err_pdxbuf:
		wsprintf( errmsg, "�t�@�C��(%s)��ǂ�PDX�o�b�t�@������܂���B", pdxfn );
		break;
	case err_mxdrv:
		wsprintf( errmsg, "mxdrv.dll�̏������Ɏ��s���܂����B" );
		break;
	default:
		wsprintf( errmsg, "�����G���[�B" );
		break;
	}
	if (Env.ignoreErr == false) {
		::MessageBox( hwnd, errmsg, DLL_NAME, MB_ICONSTOP|MB_OK );
	}
	ret = -1;	//�G���[�I��
}
	// ���������
	if (mdxbuf != NULL) ::GlobalFree(mdxbuf);
	if (pdxbuf != NULL) ::GlobalFree(pdxbuf);

	// �n���h���J��
	if (hMdxFile != NULL) ::CloseHandle(hMdxFile);
	if (hPdxFile != NULL) ::CloseHandle(hPdxFile);

	return(ret);
}
#endif

DWORD WINAPI __stdcall PlayThread(void *b);

//////////////////////////////////////////////////
//�Đ�
// @param fn in �Ώۂ̃t�@�C����
// @return ����(=0)/�Ώۃt�@�C������(=-1)/���̑��̃G���[(>0,<-1)
int play(char *fn) 
{ 
	//�A�E�g�v�b�g�v���O�C���ݒ�
	int maxlatency;
	maxlatency = mod.outMod->Open(Env.samprate, NCH, BPS, 0, 0);
	if (maxlatency < 0) {	// error opening device
		MessageBox(GetDesktopWindow(),
			"outMod open error", DLL_NAME, MB_ICONSTOP|MB_OK);
		return(1);
	}

	g_paused = 0;

	// Info�\���X�V
	mod.SetInfo((Env.samprate*BPS*NCH)/1000, Env.samprate/1000, NCH, 1);

	// Vis�̏�����
	mod.SAVSAInit(maxlatency, Env.samprate);
	mod.VSASetInfo(Env.samprate, NCH);

	// set the output plug-ins default volume
	mod.outMod->SetVolume(-666);

	//���t�J�n
	if (PlayMDX(::GetDesktopWindow(), fn) == -1) return(1);

	// �X���b�h�쐬
	unsigned long thread_id;
	g_bDecodeThread = TRUE;
	g_hThreadHandle = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PlayThread,
										 (void *) &g_bDecodeThread, 0, &thread_id);
	::SetThreadPriority(g_hThreadHandle, Env.priority);

	return(0); 
}

//////////////////////////////////////////////////
//�ꎞ��~
void pause()
{
	g_paused = 1;
	//	MXDRV_Pause(); �Đ��X���b�h�ŏ���
	mod.outMod->Pause(1);
}

//////////////////////////////////////////////////
//�ꎞ��~����
void unpause()
{
	g_paused = 0;
	//	MXDRV_Cont(); �Đ��X���b�h�ŏ���
	mod.outMod->Pause(0);
}

//////////////////////////////////////////////////
//�ꎞ��~���
int ispaused()
{
	return(g_paused);
}

//////////////////////////////////////////////////
//�Đ����~
void stop()
{ 
	if (g_hThreadHandle != NULL) {
		g_bDecodeThread = FALSE;
		if (::WaitForSingleObject(g_hThreadHandle, INFINITE) == WAIT_TIMEOUT) {
			::MessageBox(mod.hMainWindow, "error asking thread to die!\n", "error killing decode thread", 0);
			::TerminateThread(g_hThreadHandle, 0);
		}
		::CloseHandle(g_hThreadHandle);
		g_hThreadHandle = NULL;
	}
	MXDRV_Stop();
	MXDRV_End();	//���̃v���O�C���ׂ̈ɉ������
	g_bMxdrvStarted = FALSE;
	mod.outMod->Close();
	mod.SAVSADeInit();
}

//////////////////////////////////////////////////
//�Ȃ̎��Ԃ�[ms]�ŕԂ�
int getlength()
{
	//MXDRV���擾��������[ms]��Ԃ�
	return(g_PlayTime);
}

int decode_pos_ms; // current decoding position, in milliseconds

//////////////////////////////////////////////////
//�o�ߎ��Ԃ�[ms]�ŕԂ�
int getoutputtime()
{
	//�K���Ɏ���
//	return(decode_pos_ms + (mod.outMod->GetOutputTime() - mod.outMod->GetWrittenTime()));
	return(decode_pos_ms - (mod.outMod->GetWrittenTime() - mod.outMod->GetOutputTime()));

}

//////////////////////////////////////////////////
//�Đ��ʒu�w��
void setoutputtime(int time_in_ms)
{
	g_seek_time = time_in_ms;
	::SetEvent(g_hSeekEvent);
}

//////////////////////////////////////////////////
// �ȃ^�C�g���擾
// @return 0:����/-1:���s
int GetFileMDXTitle( char *mdxfilename, char *title, char *pdx )
{
	BYTE *mdxbuf = NULL;
	int mdxsize, mdxbufsize = 1*1024;

	mdxbuf = (BYTE*)::GlobalAlloc(GMEM_FIXED, mdxbufsize);
	if (mdxbuf == NULL) return(-1);
	::ZeroMemory(mdxbuf, mdxbufsize);

	HANDLE hMdxFile = ::CreateFile(
						mdxfilename,
						GENERIC_READ,
						FILE_SHARE_READ|FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	if (hMdxFile == NULL) {
		::GlobalFree(mdxbuf);
		return(-1);
	}
	::SetFilePointer(hMdxFile, 0, NULL, FILE_BEGIN);
	DWORD oReadSize;
	if (::ReadFile(hMdxFile, mdxbuf, mdxbufsize, &oReadSize, NULL)) {
		mdxsize = oReadSize;
	} else {
		::CloseHandle(hMdxFile);
		return(-1);
	}
	::CloseHandle(hMdxFile);

	//�^�C�g��
	int i;
	for ( i = 0 ; i < mdxsize ; i++ ) {
		if ( (mdxbuf[i] == 0x0d) || (mdxbuf[i] == 0) ) break;
		title[i] = mdxbuf[i];
	}
	title[i]=0;
	if ( i == 0 ) lstrcpy( title , mdxfilename );
//title[40] = 0;	//@@@
	ConvTitle( (unsigned char *)title );
	//�^�C�g���I���܂ŃX�L�b�v
	for ( ; ; i++ ) {
		if ( mdxbuf[i] == 0 ) break;
		if ( mdxbuf[i] == 0x1a ) {
			i++;
			break;
		}
	}

	//PDX�t�@�C����
	int j;
	for ( j= 0 ; i < mdxsize ; i++, j++ ) {
		if ( mdxbuf[i] == 0 ) break;
		pdx[j] = mdxbuf[i];
	}
	pdx[j]=0;
	::GlobalFree(mdxbuf);
	return(0);
}

//////////////////////////////////////////////////
//�t�@�C�����ڍו\��
int infoDlg(char *fn, HWND hwnd)
{
	char title[1024] , pdx[1024] , *p;

	p = fn + lstrlen(fn);
	for (;;) {
		p--;
		if (p < fn) break;
		if (!_ismbstrail( (const unsigned char *)fn , (const unsigned char *)p ) && *p == '\\') break;
	}
	p++;
	if (GetFileMDXTitle(fn, title, pdx) == 0) {
		lstrcat( title , "\n\nMDXfile " );
		lstrcat( title , fn );
		lstrcat( title , "\nPDXfile " );
		if ( pdx[0] ) {
			lstrcat( title, pdx );
		} else {
			lstrcat( title, "<none>" );
		}
		::MessageBox(hwnd, title, p, MB_OK);
		return(0);
	} else {
		// �^�C�g���擾�G���[
		return(1);
	}
}

//////////////////////////////////////////////////
//�t�@�C�����擾(�v���C�A�V�[�N��)
void getfileinfo(char *filename, char *title, int *length_in_ms)
{
	if (!filename || !*filename) {	// currently playing file
		if (title) lstrcpy( title , g_MDXTitle );
		if (length_in_ms) *length_in_ms = g_PlayTime;
	} else {
		char pdx[1024];
		GetFileMDXTitle( filename , title, pdx );
		if (length_in_ms) *length_in_ms=-1000;
	}
}

//////////////////////////////////////////////////
// �I���`�F�b�N
// @param fade out 0xFFFF(�I��)/1(���t�I������)/0(���t��)
void FinishCheck(int *fade)
{
	// mxdrv�͉��t���H
	if (IsPlayEnd()) {
		*fade = 0xffff;
		// �����v�̐؂�ւ�(��)
		mod.SetInfo((Env.samprate*BPS*NCH)/1000, Env.samprate/1000, NCH, 0);
	} else {
		//���[�v�J�E���g���`�F�b�N
		unsigned int n = GetLoopCount();
		if ( (n >= (unsigned)Env.Loop) ||
		     (getoutputtime() > Env.qtime*MIN) ) {
			// n�񃋁[�v���P�񃋁[�vm���ȏ�
			if ( *fade == 0 ) {
                // �����v�̐؂�ւ�(��)
				mod.SetInfo((Env.samprate*BPS*NCH)/1000, Env.samprate/1000, NCH, 0);
				MXDRV_Fadeout2(Env.FadeTime);
				(*fade) = 1;
			}
		}
	}
}

//////////////////////////////////////////////////
// �Đ��X���b�h
DWORD WINAPI __stdcall PlayThread(void *p_bDecode)
{
	// ����9000�Ƃ����l��44.1KHz����200ms���̃o�b�t�@��z�肵�Ă����͗l
	// 96KHz�ɑΉ��������݂͒l��ύX����
	// �X��x2���Ă���̂�DSP��ʂ������������l��
	char aSampleBuffer[(MAX_FREQ/1000)*30*NCH*(BPS/8)*2]; // Sound data buffer (30ms)

	// ������
	decode_pos_ms = 0;
	::ZeroMemory(aSampleBuffer, sizeof(aSampleBuffer));

#define WRITE_LENGTH (WAVEBUF*NCH*(BPS/8))

#if 0
	//�I�[�g�Q�C���R���g���[��
#define PCM_DFLT 0x1000
	if (Env.agc) {
		MXDRV_Stop();
		int mpcm = PCM_DFLT;
		// �擪��20sec��1.5sec�P�ʂŃ`�F�b�N
		for (int x = 0 ; (x < (int)(g_PlayTime-1500)) && (x < 20000) ; x += 1500) {
			MXDRV_PlayAt(x , Env.Loop, TRUE);
			MXDRV_GetPCM(aSampleBuffer , (Env.samprate/1000)*30);	// �����܂�200ms�҂��ɂ���
			MXDRV_GetPCM(aSampleBuffer , 1024 / 4);
			MXDRV_Stop();
			short *p = (short*)&aSampleBuffer[0];
			for (int i = 0 ; i < 1024 ; i += 2) {	//4096byte 1024ko
				if ( *p > mpcm ) mpcm = *p;
				p += 3;	//= 2*8;	//1ko
			}
		}
#define PCMLVL 0x8000		// �ݒ�\�ɂ����ق�������?
		MXDRV_TotalVolume((int)(256.0*PCMLVL/mpcm));
		MXDRV_Replay();
	}
#endif

	int fade = 0;

	while (*(BOOL*)p_bDecode) {

		if (fade == 0xffff) {
			//�ȏI�����o��
			mod.outMod->CanWrite();
			if (mod.outMod->IsPlaying() == 0) {
				// �I�[�ɒB��������Winamp�ɒʒm
				::PostMessage(mod.hMainWindow, WM_WA_EOF, 0, 0);
				break;
			}
		} else if (mod.outMod->CanWrite() >= (WRITE_LENGTH << (mod.dsp_isactive()?1:0))) {
			//�Đ���
			FinishCheck(&fade);
			int l = WRITE_LENGTH;
			MXDRV_GetPCM(aSampleBuffer , WRITE_LENGTH / 4);	//��荞��

			if (mod.dsp_isactive()) {
				l = mod.dsp_dosamples((short *)aSampleBuffer, l/NCH/(BPS/8), BPS, NCH, Env.samprate)*(NCH*(BPS/8));
			}

			mod.SAAddPCMData((char *)aSampleBuffer, NCH, BPS, decode_pos_ms);
			mod.VSAAddPCMData((char *)aSampleBuffer, NCH, BPS, decode_pos_ms);

			decode_pos_ms += (WAVEBUF*1000)/Env.samprate;

			mod.outMod->Write(aSampleBuffer, l);	//����
		}

		// �o�b�t�@�����O�\�҂�(20ms)
		DWORD ret;
		ret = ::WaitForSingleObject(g_hSeekEvent, FILLPOLL);

		if (ret != WAIT_TIMEOUT) {
			//�V�[�N�v������
			::ResetEvent(g_hSeekEvent);
			MXDRV_Stop();
			fade = 0;
			decode_pos_ms = g_seek_time - (g_seek_time%SEC);
			mod.SetInfo((Env.samprate*BPS*NCH)/1000, Env.samprate/1000, NCH, 1);
			mod.outMod->Flush(g_seek_time);
			MXDRV_PlayAt(g_seek_time, Env.Loop, TRUE);
		}
	}

	return(0);
}

//////////////////////////////////////////////////
//In_Module�\����
In_Module mod = 
{
	IN_VER,
	"X68000 MDX Decorder " DLL_VERSION " (x86)",
	0,	// hMainWindow
	0,  // hDllInstance
	FILE_EXT,
	1,	// is_seekable
	1,	// uses output
	config,
	about,
	init,
	quit,
	getfileinfo,
	infoDlg,
	isourfile,
	play,
	pause,
	unpause,
	ispaused,
	stop,
	
	getlength,
	getoutputtime,
	setoutputtime,

	setvolume,
	setpan,

	0,0,0,0,0,0,0,0,0, // vis stuff

	0,0, // dsp

	eq_set,

	NULL,		// setinfo

	0 // out_mod
};

//////////////////////////////////////////////////
//In_Module�擾�֐�
extern "C" {
__declspec( dllexport ) In_Module * winampGetInModule2()
{
	  return(&mod);
}
}

