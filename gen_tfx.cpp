// TransparentFX general purpose plug-in
// Copyright (C) 2001, Ing-Long Eric Kuo
// Works with Winamp 2.x

// Winamp general purpose plug-in mini-SDK
// Copyright (C) 1997, Justin Frankel/Nullsoft

#define _WIN32_WINNT 0x500

#include <windows.h>
#include <crtdbg.h>

#include "frontend.h"
// #include "Winampcmd.h"

#include "gen.h"
#include "gen_tfx.h"
#include "resource.h"
#include "config.h"
#include "global.h"

// /////////////////////////////////////
// Variables
// /////////////////////////////////////

Global *Global::_instance = NULL;

char szClassName[] = "TransparentFX Class";

char PlugInDescription[MAX_PATH];

// /////////////////////////////////////
// Functions
// /////////////////////////////////////

// this is choosen not to clash
// we use FADER_STARTID - 1 for the mouse timer
// we also use FADER_STARTID + max windows
#define FADER_STARTID	13579

int main(void)
{
	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

	__declspec(dllexport) winampGeneralPurposePlugin *winampGetGeneralPurposePlugin() {
		Global::Initialize();
		return &GlobalVariables.plugin;
	}
#ifdef __cplusplus
}
#endif

bool bIsWindowsVersionOK(void)
{
	OSVERSIONINFO osvi;

	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);

	GetVersionEx(&osvi);		// Assume this function succeeds.

	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 5)
		return TRUE;

	return FALSE;
}

BOOL CALLBACK enumwindowsfunc(HWND hwnd, LPARAM lParam)
{
	char classname[30], windowtitle[30];
	DWORD id;

	// Get the windows' pid
	GetWindowThreadProcessId(hwnd, &id);

	// check the id's
	if (GlobalVariables.ThisProcessID != id)
		return TRUE;

	// get the class name
	GetClassName(hwnd, classname, 30);

	// remember the window handle...
	if (strcmp(classname, "Winamp v1.x") == 0)
		GlobalVariables.UIInfo[MAINWINDOW].hwnd = hwnd;
	else if (strcmp(classname, "Winamp PE") == 0)
		GlobalVariables.UIInfo[PLAYLISTEDITOR].hwnd = hwnd;
	else if (strcmp(classname, "Winamp EQ") == 0)
		GlobalVariables.UIInfo[EQUALIZER].hwnd = hwnd;
	else if (strcmp(classname, "Winamp MB") == 0)
		GlobalVariables.UIInfo[MINIBROWSER].hwnd = hwnd;
	else if (strcmp(classname, "Winamp Gen") == 0)
	{
		GetWindowText(hwnd, windowtitle, 30);
		if (strcmp(windowtitle, "Winamp Library") == 0)
			GlobalVariables.UIInfo[LIBRARY].hwnd = hwnd;
	}
	return TRUE;
}


void ChangeWindowStyle(int window_index)
{
	DWORD dwLong;

	if(window_index < 0 || window_index >= NumWindows)
		return;

	if(GlobalVariables.UIInfo[window_index].hwnd == NULL)
		return;

	dwLong = GetWindowLong(GlobalVariables.UIInfo[window_index].hwnd, GWL_EXSTYLE);

	if (GlobalVariables.UserSettings.Enable)
	{
		if(!(dwLong & WS_EX_LAYERED))
			SetWindowLong(GlobalVariables.UIInfo[window_index].hwnd, GWL_EXSTYLE, dwLong | WS_EX_LAYERED);
		GlobalVariables.UIInfo[window_index].currentalpha = 0;
	}
	else
	{
		if(dwLong & WS_EX_LAYERED)
			SetWindowLong(GlobalVariables.UIInfo[window_index].hwnd, GWL_EXSTYLE, dwLong & ~WS_EX_LAYERED);
	}
}

int init()
{
	WNDCLASS wc;

	// Set up the description for Winamp to display
	char filename[MAX_PATH],
	*pluginname;

	_RPT0(_CRT_WARN, "init()\n");

	GlobalVariables.WinampVersion = SendMessage(GlobalVariables.plugin.hwndParent, WM_WA_IPC, 0, IPC_GETVERSION);

	GetModuleFileName(GlobalVariables.plugin.hDllInstance, filename, MAX_PATH);
	pluginname = strrchr(filename, '\\');
	pluginname++;				// skip the slash
	wsprintf(PlugInDescription, "%s Plug-In (%s)", GENTFXNAMEVERSION, pluginname);
	GlobalVariables.plugin.description = PlugInDescription;

	// Set up the pathname to the ini file
	strcpy(GlobalVariables.PlugInIniFile, filename);
	pluginname = strrchr(GlobalVariables.PlugInIniFile, '\\');
	pluginname++;				// skip the slash
	*pluginname = 0;			// null terminate
	strcat(GlobalVariables.PlugInIniFile, "plugin.ini");

	GlobalVariables.HaveSkinSettings = 0;

	SetDefaults();

	// get the processid for enumwindowsfunc
	GetWindowThreadProcessId(GlobalVariables.plugin.hwndParent, &GlobalVariables.ThisProcessID);

	// get winamp main window handle
	if(!GlobalVariables.UIInfo[MAINWINDOW].hwnd)
		GlobalVariables.UIInfo[MAINWINDOW].hwnd = GlobalVariables.plugin.hwndParent;

	// get winamp's child windows
	EnumWindows(enumwindowsfunc, 0);

	// read configuration, override the defaults
	ConfigReadUserSettings();
	ConfigReadCustomSettings();

	GlobalVariables.WindowCanBeLayered = bIsWindowsVersionOK();

	// check that we can actually change things...
	if (!GlobalVariables.WindowCanBeLayered)
		return 0;

	// remember the old Main Window Procedures and add ours
	if(GlobalVariables.UIInfo[MAINWINDOW].hwnd != NULL)
	{
		GlobalVariables.UIInfo[MAINWINDOW].lpWndProcOld =
			(WNDPROC) GetWindowLong(GlobalVariables.UIInfo[MAINWINDOW].hwnd, GWL_WNDPROC);
		SetWindowLong(GlobalVariables.UIInfo[MAINWINDOW].hwnd, GWL_WNDPROC, (LONG) MainWindowProc);
	}

	if(GlobalVariables.UIInfo[EQUALIZER].hwnd != NULL)
	{
		GlobalVariables.UIInfo[EQUALIZER].lpWndProcOld =
			(WNDPROC) GetWindowLong(GlobalVariables.UIInfo[EQUALIZER].hwnd, GWL_WNDPROC);
		SetWindowLong(GlobalVariables.UIInfo[EQUALIZER].hwnd, GWL_WNDPROC, (LONG) EqualizerWindowProc);
	}

	if(GlobalVariables.UIInfo[PLAYLISTEDITOR].hwnd != NULL)
	{
		GlobalVariables.UIInfo[PLAYLISTEDITOR].lpWndProcOld =
			(WNDPROC) GetWindowLong(GlobalVariables.UIInfo[PLAYLISTEDITOR].hwnd, GWL_WNDPROC);
		SetWindowLong(GlobalVariables.UIInfo[PLAYLISTEDITOR].hwnd, GWL_WNDPROC,
					  (LONG) PlayListWindowProc);
	}

	if(GlobalVariables.UIInfo[MINIBROWSER].hwnd != NULL)
	{
		GlobalVariables.UIInfo[MINIBROWSER].lpWndProcOld =
			(WNDPROC) GetWindowLong(GlobalVariables.UIInfo[MINIBROWSER].hwnd, GWL_WNDPROC);
		SetWindowLong(GlobalVariables.UIInfo[MINIBROWSER].hwnd, GWL_WNDPROC, (LONG) MiniBrowserWindowProc);
	}
	
	if(GlobalVariables.UIInfo[LIBRARY].hwnd != NULL)
	{	
		GlobalVariables.UIInfo[LIBRARY].lpWndProcOld =
			(WNDPROC) GetWindowLong(GlobalVariables.UIInfo[LIBRARY].hwnd, GWL_WNDPROC);
		SetWindowLong(GlobalVariables.UIInfo[LIBRARY].hwnd, GWL_WNDPROC, (LONG) LibraryWindowProc);
	}

	// create API window
	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC) APIProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GlobalVariables.plugin.hDllInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szClassName;
	RegisterClass(&wc);

	CreateWindow(szClassName, szClassName, WS_OVERLAPPEDWINDOW | WS_DISABLED,
				 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				 GlobalVariables.plugin.hwndParent, NULL, GlobalVariables.plugin.hDllInstance,
				 NULL);

	_RPT0(_CRT_WARN, "init() - done\n");
	return 0;
}

LRESULT CALLBACK APIProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int alphasettingchanged = FALSE;

	if (message == WM_USER)
	{
		switch (lParam)
		{
		case 0:				// Get Version
			return GENTFXVERSION;

			// version 1.3
		case 1:				// Get Fading
			return GlobalVariables.UserSettings.Fade;
		case 2:
			return GlobalVariables.UserSettings.UseSkinSettings;
		case 3:
			return GlobalVariables.UserSettings.AlphaFollowMainWindow;
		case 4:
			return GlobalVariables.Settings[0].alphavalue(ACTIVE, MAINWINDOW);
		case 5:
			return GlobalVariables.Settings[0].alphavalue(INACTIVE, MAINWINDOW);
		case 6:
			return GlobalVariables.Settings[0].alphavalue(ACTIVE, PLAYLISTEDITOR);
		case 7:
			return GlobalVariables.Settings[0].alphavalue(INACTIVE, PLAYLISTEDITOR);
		case 8:
			return GlobalVariables.Settings[0].alphavalue(ACTIVE, EQUALIZER);
		case 9:
			return GlobalVariables.Settings[0].alphavalue(INACTIVE, EQUALIZER);
		case 10:
			return GlobalVariables.Settings[0].alphavalue(ACTIVE, MINIBROWSER);
		case 11:
			return GlobalVariables.Settings[0].alphavalue(INACTIVE, MINIBROWSER);

			// version 1.4
		case 12:
			return GlobalVariables.UserSettings.MouseOver;
		case 13:
			return GlobalVariables.UserSettings.LinkActivations;
		case 14:
			return GlobalVariables.UserSettings.FadeTimerPeriod;
		case 15:
			return GlobalVariables.Settings[0].alphavalue(MOUSEOVER, MAINWINDOW);
		case 16:
			return GlobalVariables.Settings[0].alphavalue(MOUSEOVER, PLAYLISTEDITOR);
		case 17:
			return GlobalVariables.Settings[0].alphavalue(MOUSEOVER, EQUALIZER);
		case 18:
			return GlobalVariables.Settings[0].alphavalue(MOUSEOVER, MINIBROWSER);
		case 19:
			return GlobalVariables.UserSettings.FadeTime[ACTIVE];
		case 20:
			return GlobalVariables.UserSettings.FadeTime[MOUSEOVER];
		case 21:
			return GlobalVariables.UserSettings.FadeTime[INACTIVE];

			// version 1.3
		case 101:
			GlobalVariables.UserSettings.Fade = wParam;
			break;
		case 102:
			GlobalVariables.UserSettings.UseSkinSettings = wParam;
			alphasettingchanged = TRUE;
			break;
		case 103:
			GlobalVariables.UserSettings.AlphaFollowMainWindow = wParam;
			alphasettingchanged = TRUE;
			break;
		case 104:
			GlobalVariables.Settings[0].alphavalue(ACTIVE, MAINWINDOW, wParam);
			alphasettingchanged = TRUE;
			break;
		case 105:
			GlobalVariables.Settings[0].alphavalue(INACTIVE, MAINWINDOW, wParam);
			alphasettingchanged = TRUE;
			break;
		case 106:
			GlobalVariables.Settings[0].alphavalue(ACTIVE, PLAYLISTEDITOR, wParam);
			alphasettingchanged = TRUE;
			break;
		case 107:
			GlobalVariables.Settings[0].alphavalue(INACTIVE, PLAYLISTEDITOR, wParam);
			alphasettingchanged = TRUE;
			break;
		case 108:
			GlobalVariables.Settings[0].alphavalue(ACTIVE, EQUALIZER, wParam);
			alphasettingchanged = TRUE;
			break;
		case 109:
			GlobalVariables.Settings[0].alphavalue(INACTIVE, EQUALIZER, wParam);
			alphasettingchanged = TRUE;
			break;
		case 110:
			GlobalVariables.Settings[0].alphavalue(ACTIVE, MINIBROWSER, wParam);
			alphasettingchanged = TRUE;
			break;
		case 111:
			GlobalVariables.Settings[0].alphavalue(INACTIVE, MINIBROWSER, wParam);
			alphasettingchanged = TRUE;
			break;

			// version 1.4
		case 112:
			GlobalVariables.UserSettings.MouseOver = wParam;
			break;
		case 113:
			GlobalVariables.UserSettings.LinkActivations = wParam;
			alphasettingchanged = TRUE;
			break;
		case 114:
			GlobalVariables.UserSettings.FadeTimerPeriod = wParam;
			break;
		case 115:
			GlobalVariables.Settings[0].alphavalue(MOUSEOVER, MAINWINDOW, wParam);
			alphasettingchanged = TRUE;
			break;
		case 116:
			GlobalVariables.Settings[0].alphavalue(MOUSEOVER, PLAYLISTEDITOR, wParam);
			alphasettingchanged = TRUE;
			break;
		case 117:
			GlobalVariables.Settings[0].alphavalue(MOUSEOVER, EQUALIZER, wParam);
			alphasettingchanged = TRUE;
			break;
		case 118:
			GlobalVariables.Settings[0].alphavalue(MOUSEOVER, MINIBROWSER, wParam);
			alphasettingchanged = TRUE;
			break;
		case 119:
			GlobalVariables.UserSettings.FadeTime[ACTIVE] = wParam;
			break;
		case 120:
			GlobalVariables.UserSettings.FadeTime[MOUSEOVER] = wParam;
			break;
		case 121:
			GlobalVariables.UserSettings.FadeTime[INACTIVE] = wParam;
			break;

			// version 1.3
		case 1000:
			ConfigWrite();
			break;
		case 1001:
			ConfigReadUserSettings();
			ConfigReadCustomSettings();
			alphasettingchanged = TRUE;
			break;
		}

		if (alphasettingchanged)
			SetTransparency();

		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

int AlphaValueClamp(int value)
{
	if (value < 15)
		return 15;
	else if (value > 255)
		return 255;
	else
		return value;
}

void quit()
{
	ConfigWrite();
	/*
	 * if(!WindowCanBeLayered) return;
	 * 
	 * 
	 */
	Global::ShutdownVariables();
}

// //////////////////////////////////////////////////////
// Functions that actually change appearances
// //////////////////////////////////////////////////////

VOID CALLBACK FaderProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	int window_index;			// index of information
	int timeused;				// in miliseconds
	int fadetime;


	//_RPT1(_CRT_WARN, "FaderProc(%i)\n", idEvent);

	window_index = idEvent - FADER_STARTID;
	if (window_index < 0 || window_index > 4)
		return;

	fadetime = GlobalVariables.UIInfo[window_index].fadetime;
	timeused = dwTime - GlobalVariables.UIInfo[window_index].starttime;

	if (timeused >= fadetime)
	{
		timeused = fadetime;	// so that currentalpha will be calc as targetalpha
		KillTimer(hwnd, idEvent);
		_RPT1(_CRT_WARN, "FaderProc killtimer window=%i\n", window_index);
	}

	if (fadetime > 0)			// calculate alpha ramp based on fadetime
	{
		// use current and fade time to calculate an alpha value
		GlobalVariables.UIInfo[window_index].currentalpha =
			GlobalVariables.UIInfo[window_index].beginalpha +
			(GlobalVariables.UIInfo[window_index].targetalpha -
			 GlobalVariables.UIInfo[window_index].beginalpha) * timeused / fadetime;
	}
	else						// hum.. fadetime is zero, they want instant result
	{
		GlobalVariables.UIInfo[window_index].currentalpha =
			GlobalVariables.UIInfo[window_index].targetalpha;
	}

	_RPT2(_CRT_WARN, "SetLayeredWindowAttributes window=%i to %i\n", window_index,
		  GlobalVariables.UIInfo[window_index].currentalpha);
	
	if(IsWindowVisible(GlobalVariables.UIInfo[window_index].hwnd))
		SetLayeredWindowAttributes(GlobalVariables.UIInfo[window_index].hwnd, 0,
							   GlobalVariables.UIInfo[window_index].currentalpha, LWA_ALPHA);


}

/*
 * Add a timer to hwnd, the ID is FADER_STARTID + index.
 * 
 * DoAlpha applies the effect on a window
 * 
 */
void DoAlpha(int state, int window_index)
{

	_RPT2(_CRT_WARN, "DoAlpha, s=%i,w=%i\n", state, window_index);
	if (GlobalVariables.UserSettings.Fade)
	{
		GlobalVariables.UIInfo[window_index].starttime = GetTickCount();
		
		GlobalVariables.UIInfo[window_index].fadetime = GlobalVariables.UserSettings.FadeTime[state];
		GlobalVariables.UIInfo[window_index].targetstate = state;

		// create the alpha ramp using the current alpha as the start
		GlobalVariables.UIInfo[window_index].beginalpha =
			GlobalVariables.UIInfo[window_index].currentalpha;
		GlobalVariables.UIInfo[window_index].targetalpha =
			GlobalVariables.Settings[ActiveSetting].alphavalue(state, window_index);

		_RPT2(_CRT_WARN, "DoAlpha fade (%i, %i)\n", state, window_index);

		SetTimer(GlobalVariables.UIInfo[window_index].hwnd, FADER_STARTID + window_index,
				 GlobalVariables.UserSettings.FadeTimerPeriod, FaderProc);
	}
	else						// they don't want fade.. do it right here...
	{
		_RPT2(_CRT_WARN, "DoAlpha (%i, %i)\n", state, window_index);
		GlobalVariables.UIInfo[window_index].currentalpha =
			GlobalVariables.Settings[ActiveSetting].alphavalue(state, window_index);
		GlobalVariables.UIInfo[window_index].targetstate = state;
		if(IsWindowVisible(GlobalVariables.UIInfo[window_index].hwnd))
			SetLayeredWindowAttributes(GlobalVariables.UIInfo[window_index].hwnd, 0,
								   GlobalVariables.UIInfo[window_index].currentalpha, LWA_ALPHA);
	}
}

/*
 * Applies an effect on all the windows 
 */
_inline void DoAlphaLink(int state)
{
	int i;

	_RPT1(_CRT_WARN, "DoAlphaLink, s=%i\n", state);

	for (i = 0; i < NumWindows; i++)
		DoAlpha(state, i);
}

// track the mouse by testing window handles
// this will also include the window's child unlike TrackMouseEvent()
// notice we don't use PtInRect since windows will overlap
VOID CALLBACK TrackMouseTimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	POINT pt;

	GetCursorPos(&pt);
	HWND inhwnd = WindowFromPoint(pt);

	// see if we can find our own window... 
	// this is needed for nested windows, eg. mini browser
	while(inhwnd != NULL)
	{
		// if found do nothing
		if(inhwnd == hWnd)
			return;		

		inhwnd = GetParent(inhwnd);
	}

	// not found, so it's not in hWnd anymore
	KillTimer(hWnd, idEvent);
	SendMessage(hWnd, WM_MOUSELEAVE, 0, 0);	
}

_inline BOOL TrackMouse(HWND hwnd)
{
	return SetTimer(hwnd, FADER_STARTID - 1, 50, (TIMERPROC) TrackMouseTimerProc);
}


_inline void StateLogic(HWND hwnd, UINT message, WPARAM wParamlow, LPARAM lParam, int window_index)
{
	if(hwnd != GlobalVariables.UIInfo[window_index].hwnd)
		return;

	if(!GlobalVariables.UserSettings.Enable)
		return;

	switch (message)
	{
	case WM_SHOWWINDOW:
		SetTransparency();
		break;
	case WM_ACTIVATE:
		{
			if (wParamlow)		// (WA_ACTIVE || WA_CLICKACTIVE)
			{
				_RPT1(_CRT_WARN, "  Activate - w=%i\n", window_index);
				DoAlpha(ACTIVE, window_index);

			}
			else
			{
				_RPT1(_CRT_WARN, "   Deactivate - w=%i\n", window_index);
				DoAlpha(INACTIVE, window_index);

			}
		}
		break;
	case WM_SETCURSOR:
	case WM_MOUSEMOVE:
		{
			if (!GlobalVariables.UserSettings.MouseOver)
				break;

			if (GlobalVariables.UIInfo[window_index].mouseinside == 0)
			{
				_RPT2(_CRT_WARN, "   MouseEnters - w=%i, %i\n", window_index,
					  GlobalVariables.UIInfo[window_index].targetstate);
				
				// use our tracker
				TrackMouse (hwnd);				

				// only change if it is not active
				if (GlobalVariables.UIInfo[window_index].targetstate != ACTIVE)
					DoAlpha(MOUSEOVER, window_index);

				// mouse is in
				GlobalVariables.UIInfo[window_index].mouseinside = 1;
			}
		}
		break;
	case WM_MOUSELEAVE:
		{
			if (!GlobalVariables.UserSettings.MouseOver)
				break;

			_RPT1(_CRT_WARN, "   MouseLeave - w=%i\n", window_index);
			GlobalVariables.UIInfo[window_index].mouseinside = 0;

			// only change if it is not active
			if (GlobalVariables.UIInfo[window_index].targetstate != ACTIVE)
				DoAlpha(INACTIVE, window_index);
		}
		break;
	}
}

/*
 * StateLogicLinked is responsible for figuring out if a state change will actually require a
 * change in transparent value.  Eg. if another window is active, there's no reason to do active
 * again, since the values do not need to be changed.
 * 
 */
_inline void
StateLogicLinked(HWND hwnd, UINT message, WPARAM wParamlow, LPARAM lParam, int window_index)
{
	if(hwnd != GlobalVariables.UIInfo[window_index].hwnd)
		return;

	if(!GlobalVariables.UserSettings.Enable)
		return;

	int i, s;

	switch (message)
	{
	case WM_SHOWWINDOW:
		SetTransparency();
		break;

		// WM_ACTIVATE seems to be enough..
		// case WM_NCACTIVATE: // this is sent when the application starts
		// lParam = NULL; // no previous window
		// continue to WM_ACTIVATE
	case WM_ACTIVATE:
		{
			if (wParamlow)		// (WA_ACTIVE || WA_CLICKACTIVE)
			{
				_RPT1(_CRT_WARN, "   Activate - w=%i\n", window_index);
				// check to see if the old window is another winamp window
				s = 1;
				for (i = 0; i < NumWindows; i++)
				{
					if(GlobalVariables.UIInfo[i].hwnd == NULL)
						continue;

					if (GlobalVariables.UIInfo[i].hwnd == (HWND) lParam)
					{
						s = 0;
						_RPT1(_CRT_WARN, "   %i is old active\n", i);
					}
				}

				if (s)
					DoAlphaLink(ACTIVE);
			}
			else
			{
				_RPT1(_CRT_WARN, "   Deactivate - w=%i\n", window_index);
				// check to see if the old window is another winamp window
				s = 1;
				for (i = 0; i < NumWindows; i++)
				{
					if(GlobalVariables.UIInfo[i].hwnd == NULL)
						continue;

					if (GlobalVariables.UIInfo[i].hwnd == (HWND) lParam)
					{
						s = 0;
						_RPT1(_CRT_WARN, "   %i is old inactive\n", i);
					}
				}

				if (s)
					DoAlphaLink(INACTIVE);
			}
		}
		break;
	case WM_SETCURSOR:
	case WM_MOUSEMOVE:
		{
			if (!GlobalVariables.UserSettings.MouseOver)
				break;

			if (GlobalVariables.UIInfo[window_index].mouseinside == 0)
			{

				_RPT1(_CRT_WARN, "   MouseEnter - w=%i\n", window_index);

				TrackMouse (hwnd);
				/*
				TRACKMOUSEEVENT trackmouseevent;
				trackmouseevent.cbSize = sizeof(trackmouseevent);
				trackmouseevent.dwFlags = TME_LEAVE;
				trackmouseevent.hwndTrack = hwnd;
				trackmouseevent.dwHoverTime = HOVER_DEFAULT;

				TrackMouseEvent(&trackmouseevent);
				*/

				// if one of the window is active don't do anything
				s = 1;
				for (i = 0; i < NumWindows; i++)
				{
					if(GlobalVariables.UIInfo[i].hwnd == NULL)
						continue;

					if (GlobalVariables.UIInfo[i].targetstate == ACTIVE)
					{
						s = 0;
						_RPT1(_CRT_WARN, "   %i is still active\n", i);
					}
				}

				if (s)
					DoAlphaLink(MOUSEOVER);

				// mouse is in
				GlobalVariables.UIInfo[window_index].mouseinside = 1;
			}
		}
		break;
	case WM_MOUSELEAVE:
		{
			if (!GlobalVariables.UserSettings.MouseOver)
				break;

			_RPT1(_CRT_WARN, "   MouseLeave - w=%i\n", window_index);

			// mouse is outta here
			GlobalVariables.UIInfo[window_index].mouseinside = 0;

			s = 1;
			for (i = 0; i < NumWindows; i++)
			{
				if(GlobalVariables.UIInfo[i].hwnd == NULL)
					continue;

				// if one is active, don't do anything
				if (GlobalVariables.UIInfo[i].targetstate == ACTIVE)
					s = 0;

				// if the mouse is in any window, don't do anything
				if (GlobalVariables.UIInfo[i].mouseinside)
					s = 0;
			}

			if (s)
				DoAlphaLink(INACTIVE);

		}
		break;
	}
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD wParamlow = LOWORD(wParam);

	static BOOL changedstyle = FALSE;
	if (!changedstyle)
	{
		ChangeWindowStyle(MAINWINDOW);
		changedstyle = true;
	}

	if (GlobalVariables.UserSettings.LinkActivations)
		StateLogicLinked(hwnd, message, wParamlow, lParam, MAINWINDOW);
	else
		StateLogic(hwnd, message, wParamlow, lParam, MAINWINDOW);

	// skin change ....
	// 32769(away from base)

	// no need to check WindowCanBeLayered, since this is only hooked if it is true
	LRESULT res = CallWindowProc(GlobalVariables.UIInfo[MAINWINDOW].lpWndProcOld, hwnd, message, wParam,
					   lParam);

	// fix the 'leave ugly stuff on desktop' bug
	if (message == WM_NCCALCSIZE && wParam == TRUE)
		return 0;

	if (message == WM_COMMAND)
	{
		switch (wParamlow)
		{
		case 40291:			// change skin from menu?
		//case 40258:			// change skin from menu?
		case WINAMP_BUTTON5:	// next
		case WINAMP_BUTTON1:	// previous
		case WINAMP_BUTTON2:	// play
			{
				ConfigReadCustomSettings();
				SetTransparency();
			}
			break;
		}
	}
	/*
	 * else if (message == WM_USER + 2) // some sort of internal message { ConfigReadCustomSettings 
	 * (); SetTransparency (); } 
	 */

	return res;
}

LRESULT CALLBACK PlayListWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD wParamlow = LOWORD(wParam);

	static BOOL changedstyle = FALSE;
	if (!changedstyle)
	{
		if(GlobalVariables.WinampVersion < 0x5011)
			ChangeWindowStyle(PLAYLISTEDITOR);

		changedstyle = true;
	}

	if (GlobalVariables.UserSettings.LinkActivations)
		StateLogicLinked(hwnd, message, wParamlow, lParam, PLAYLISTEDITOR);
	else
		StateLogic(hwnd, message, wParamlow, lParam, PLAYLISTEDITOR);

	// since Winamp5 embeds this window, we need to detect this and disable alpha
	if(message == WM_SHOWWINDOW && GlobalVariables.UserSettings.Enable)
	{		
		if(GetAncestor(hwnd, GA_PARENT) == GetDesktopWindow())
		{			
			DWORD dwLong = GetWindowLong(hwnd, GWL_EXSTYLE);

			// turn on alpha?
			if(!(dwLong & WS_EX_LAYERED))
				SetWindowLong(hwnd, GWL_EXSTYLE, dwLong | WS_EX_LAYERED);
		}
		else
		{
			DWORD dwLong = GetWindowLong(hwnd, GWL_EXSTYLE);

			// turn off alpha
			if(dwLong & WS_EX_LAYERED)
				SetWindowLong(hwnd, GWL_EXSTYLE, dwLong & ~WS_EX_LAYERED);
		}
	}

	// no need to check WindowCanBeLayered, since this is only hooked if it is true
	LRESULT res = CallWindowProc(GlobalVariables.UIInfo[PLAYLISTEDITOR].lpWndProcOld, hwnd, message,
						  wParam, lParam);

	return res;
}

LRESULT CALLBACK EqualizerWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD wParamlow = LOWORD(wParam);

	static BOOL changedstyle = FALSE;
	if (!changedstyle)
	{
		ChangeWindowStyle(EQUALIZER);
		changedstyle = true;
	}

	if (GlobalVariables.UserSettings.LinkActivations)
		StateLogicLinked(hwnd, message, wParamlow, lParam, EQUALIZER);
	else
		StateLogic(hwnd, message, wParamlow, lParam, EQUALIZER);

	// no need to check WindowCanBeLayered, since this is only hooked if it is true
	LRESULT res =  CallWindowProc(GlobalVariables.UIInfo[EQUALIZER].lpWndProcOld, hwnd, message, wParam,
						  lParam);
	
	return res;
}

LRESULT CALLBACK MiniBrowserWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD wParamlow = LOWORD(wParam);

	static BOOL changedstyle = FALSE;
	if (!changedstyle)
	{
		ChangeWindowStyle(MINIBROWSER);
		changedstyle = true;
	}

	if (GlobalVariables.UserSettings.LinkActivations)
		StateLogicLinked(hwnd, message, wParamlow, lParam, MINIBROWSER);
	else
		StateLogic(hwnd, message, wParamlow, lParam, MINIBROWSER);

	// since Winamp5 embeds this window, we need to detect this and disable alpha
	if(message == WM_SHOWWINDOW && GlobalVariables.UserSettings.Enable)
	{		
		if(GetAncestor(hwnd, GA_PARENT) == GetDesktopWindow())
		{			
			DWORD dwLong = GetWindowLong(hwnd, GWL_EXSTYLE);

			// turn on alpha?
			if(!(dwLong & WS_EX_LAYERED))
				SetWindowLong(hwnd, GWL_EXSTYLE, dwLong | WS_EX_LAYERED);
		}
		else
		{
			DWORD dwLong = GetWindowLong(hwnd, GWL_EXSTYLE);

			// turn off alpha
			if(dwLong & WS_EX_LAYERED)
				SetWindowLong(hwnd, GWL_EXSTYLE, dwLong & ~WS_EX_LAYERED);
		}
	}

	// no need to check WindowCanBeLayered, since this is only hooked if it is true
	LRESULT res = CallWindowProc(GlobalVariables.UIInfo[MINIBROWSER].lpWndProcOld, hwnd, message, wParam,
						  lParam);

	return res;
}

LRESULT CALLBACK LibraryWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD wParamlow = LOWORD(wParam);

	static BOOL changedstyle = FALSE;
	if (!changedstyle)
	{
		ChangeWindowStyle(LIBRARY);
		changedstyle = true;
	}

	if (GlobalVariables.UserSettings.LinkActivations)
		StateLogicLinked(hwnd, message, wParamlow, lParam, LIBRARY);
	else
		StateLogic(hwnd, message, wParamlow, lParam, LIBRARY);

	// since Winamp5 embeds this window, we need to detect this and disable alpha
	if(message == WM_NCCALCSIZE && GlobalVariables.UserSettings.Enable)
	{		
		if(GetAncestor(hwnd, GA_PARENT) == GetDesktopWindow())
		{			
			DWORD dwLong = GetWindowLong(hwnd, GWL_EXSTYLE);

			// turn on alpha?
			if(!(dwLong & WS_EX_LAYERED))
				SetWindowLong(hwnd, GWL_EXSTYLE, dwLong | WS_EX_LAYERED);
		}
		else
		{
			DWORD dwLong = GetWindowLong(hwnd, GWL_EXSTYLE);

			// turn off alpha
			if(dwLong & WS_EX_LAYERED)
				SetWindowLong(hwnd, GWL_EXSTYLE, dwLong & ~WS_EX_LAYERED);
		}
	}

	// no need to check WindowCanBeLayered, since this is only hooked if it is true
	LRESULT res = CallWindowProc(GlobalVariables.UIInfo[LIBRARY].lpWndProcOld, hwnd, message, wParam,
						  lParam);

	return res;
}

void SetTransparency()
{
	int s;

	_RPT0(_CRT_WARN, "SetTransparency\n");

	HWND activehwnd = GetActiveWindow();
	if (GlobalVariables.UserSettings.LinkActivations)
	{
		// set state depend on if a window is active
		s = 0;
		for (int i = 0; i < NumWindows; i++)
			if (activehwnd == GlobalVariables.UIInfo[i].hwnd)
				s = 1;

		StateLogicLinked(GlobalVariables.UIInfo[MAINWINDOW].hwnd, WM_ACTIVATE, s, NULL, MAINWINDOW);
	}
	else
	{
		for (int i = 0; i < NumWindows; i++)
		{
			// set depending on if it is active
			s = 0;
			if (activehwnd == GlobalVariables.UIInfo[i].hwnd)
				s = 1;
			StateLogic(GlobalVariables.UIInfo[i].hwnd, WM_ACTIVATE, s, NULL, i);
		}
	}
}
