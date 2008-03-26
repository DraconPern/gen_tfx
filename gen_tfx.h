#ifndef __GEN_TFX_H_
#define __GEN_TFX_H_

#include <string>

using namespace std;

LRESULT CALLBACK APIProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MainWindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PlayListWindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EqualizerWindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MiniBrowserWindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LibraryWindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void StateLogic (HWND hwnd, UINT message, WPARAM wParamlow, LPARAM lParam, int window_index);
void StateLogicLinked (HWND hwnd, UINT message, WPARAM wParamlow, LPARAM lParam, int window_index);

void ChangeWindowStyle (int window_index);

int init ();
void config ();
void quit ();

int AlphaValueClamp (int value);

void SetTransparency ();

BOOL CALLBACK ConfigureProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// array index
#define MAINWINDOW		0
#define PLAYLISTEDITOR	1
#define EQUALIZER		2
#define MINIBROWSER		3
#define LIBRARY			4
#define NumWindows		5

#define ACTIVE			0
#define MOUSEOVER		1
#define INACTIVE		2

#define ArraySize	NumWindows * 3

// a function that maps items to array
#define idx(state, win) (win * 3 + state)

class CUserSettings
{
public:

	int Enable;
	// fade?
	int Fade;

	int UseSkinSettings;

	int LinkActivations;

	int MouseOver;

	// alpha value follow main window?
	int AlphaFollowMainWindow;

	int FadeTime[3];
	int FadeTimerPeriod;

	CUserSettings()
	{
		Enable = 0;
		Fade = 0;
		UseSkinSettings = 0;
		LinkActivations = 0;
		MouseOver = 0;
		AlphaFollowMainWindow = 0;
		FadeTimerPeriod = 0;
		
		for(int i = 0; i < 3; i++)
			FadeTime[i] = 0;

	}
} ;

class CDialogUtil
{
public:
	// dialog box controls
	HWND sliderctrl[ArraySize];

	HWND fadetimectrl[3];
	HWND fadetimerperiodctrl;

	int m_nNormalHeight;
	int m_nExpandedHeight;
	BOOL m_bExpanded;

	// handle of the dialogbox, used to make sure there is only one
	HWND ConfigDlg;

	CDialogUtil()
	{
		int i;
		for(i = 0; i < ArraySize; i++)
			sliderctrl[i] = 0;

		for(i = 0; i < 3; i++)
			fadetimectrl[i] = NULL;

		fadetimerperiodctrl = NULL;
	
		m_nNormalHeight = m_nExpandedHeight = 0;
		m_bExpanded = FALSE;
		ConfigDlg = NULL;
	}
};

class CSettings
{
  public:
	int alphavalues[ArraySize];
	string IniFile;
	CSettings()
	{
		for(int i = 0; i < ArraySize; i++)
			alphavalues[i] = 0;
	};
	int alphavalue (int state, int win)
	{
		return alphavalues[idx (state, win)];
	};
	void alphavalue (int state, int win, int value)
	{
		alphavalues[idx (state, win)] = AlphaValueClamp (value);
	};
};

class CUIInfo
{
public:
	// the handle to the window
	HWND hwnd;

	// old WndProc
	WNDPROC lpWndProcOld;

	// flag to note if the mouse is inside the window, used for mouseover
	int mouseinside;

	// times of the fade
	DWORD starttime, fadetime;

	// alphas
	int beginalpha, currentalpha, targetalpha;

	// this should correspond with targetalpha
	int targetstate;

	CUIInfo()
	{
		hwnd = NULL;
		lpWndProcOld = NULL;
		mouseinside = 0;
		beginalpha = currentalpha = targetalpha = 0;
		targetstate = 0;
	}
};

#endif