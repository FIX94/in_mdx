//
// Winamp .MDX (X68000 YM2151 & MSM6258 AUDIO FILE) input plug-in
//
// use-dll    "MXDRV.DLL","X68Sound.DLL"
// use-header "depend.h","mxdrv.h"
//            "in2.h","out.h"
// �Q�l       "mxp.c"
//

#define DLL_NAME    "in_mdx.dll"
#define DLL_VERSION "v1.14.4"
#define FILE_EXT "MDX\0MDX Audio File (*.MDX)\0"

//�o��wave�t�H�[�}�b�g
#define NCH (2)				// �X�e���I
#define BPS (16)			// bit/size
#define MAX_FREQ (96000)
#define SOUND_96K (96000)
#define SOUND_88K (88200)
#define SOUND_62K (62500)
#define SOUND_48K (48000)
#define SOUND_44K (44100)
#define SOUND_22K (22050)
#define SOUND_11K (11025)

//�����ݒ�p�����^
struct env_t {
	int available;
	int samprate;
	int pcmbuf;
	int late;
	int pcm8use;
	int Loop;
	int qtime;
	int priority;
	int TotalVolume;
	int FadeTime;
	int ROMEO;
	unsigned int mask;
	int agc;
	int ignoreErr;
    BOOL enable;
    TCHAR PdxDir[MAX_PATH];
};

//�����ݒ�
extern struct env_t Env;

void SetChannelPlayFlagMDX( DWORD status );

//config
void EnvLoad( char *fn );
void EnvSave();

//WINAMP�G���g��
void about(HWND hwndParent);
void config(HWND hwndParent);
static void init(void);
static void quit(void);
int isourfile(char *fn);
int play(char *fn);
void pause();
void unpause();
int ispaused();
void stop();
int getlength();
int getoutputtime();
void setoutputtime(int time_in_ms);
void setvolume(int volume);
void setpan(int pan);
int infoDlg(char *fn, HWND hwnd);
void getfileinfo(char *filename, char *title, int *length_in_ms);
void eq_set(int on, char data[10], int preamp);

//winamp���񑩃e�[�u��
extern In_Module mod;

