#define _WIN32_WINNT 0x500
#include <windows.h>
#include <commctrl.h>
#include <crtdbg.h>

#include "gen.h"
#include "gen_tfx.h"
#include "config.h"
#include "resource.h"

#include "global.h"
#include "gotourl.h"

#include "frontend.h"

const char szAppName[] = "TransparentFX";

const char *const RegKeys[ArraySize] = {
	"MainWindow", "MainWindowMouseOver", "MainWindowInactive",
	"PlaylistEditor", "PlaylistEditorMouseOver", "PlaylistEditorInactive",
	"Equalizer", "EqualizerMouseOver", "EqualizerInactive",
	"MiniBrowser", "MiniBrowserMouseOver", "MiniBrowserInactive",
	"Library", "LibraryMouseOver", "LibraryInactive",
};

const char *const RegKeysFade[3] = {
	"FadeTime", "FadeTimeMouseOver", "FadeTimeInactive",
};

const int CtrlID[ArraySize] = {
	IDC_MAINWINDOWALPHA, IDC_MAINWINDOWALPHAMOUSEOVER,
	IDC_MAINWINDOWALPHAINACTIVE,

	IDC_PLAYLISTEDITORALPHA, IDC_PLAYLISTEDITORALPHAMOUSEOVER,
	IDC_PLAYLISTEDITORALPHAINACTIVE,

	IDC_EQUALIZERALPHA, IDC_EQUALIZERALPHAMOUSEOVER,
	IDC_EQUALIZERALPHAINACTIVE,

	IDC_MINIBROWSERALPHA, IDC_MINIBROWSERALPHAMOUSEOVER,
	IDC_MINIBROWSERALPHAINACTIVE,

	IDC_LIBRARYALPHA, IDC_LIBRARYALPHAMOUSEOVER,
	IDC_LIBRARYALPHAINACTIVE,
};

const int CtrlIDFade[3] = {
	IDC_FADETIMEACTIVE, IDC_FADETIMEMOUSEOVER,
	IDC_FADETIMEINACTIVE,
};

void SetDefaults()
{
	// set default GlobalVariables.Settings etc.
	GlobalVariables.Settings[0].alphavalue(ACTIVE, MAINWINDOW, 255);
	GlobalVariables.Settings[0].alphavalue(MOUSEOVER, MAINWINDOW, 170);
	GlobalVariables.Settings[0].alphavalue(INACTIVE, MAINWINDOW, 130);

	GlobalVariables.Settings[0].alphavalue(ACTIVE, PLAYLISTEDITOR, 255);
	GlobalVariables.Settings[0].alphavalue(MOUSEOVER, PLAYLISTEDITOR, 200);
	GlobalVariables.Settings[0].alphavalue(INACTIVE, PLAYLISTEDITOR, 130);

	GlobalVariables.Settings[0].alphavalue(ACTIVE, EQUALIZER, 255);
	GlobalVariables.Settings[0].alphavalue(MOUSEOVER, EQUALIZER, 170);
	GlobalVariables.Settings[0].alphavalue(INACTIVE, EQUALIZER, 130);

	GlobalVariables.Settings[0].alphavalue(ACTIVE, MINIBROWSER, 255);
	GlobalVariables.Settings[0].alphavalue(MOUSEOVER, MINIBROWSER, 233);
	GlobalVariables.Settings[0].alphavalue(INACTIVE, MINIBROWSER, 200);

	GlobalVariables.Settings[0].alphavalue(ACTIVE, LIBRARY, 255);
	GlobalVariables.Settings[0].alphavalue(MOUSEOVER, LIBRARY, 200);
	GlobalVariables.Settings[0].alphavalue(INACTIVE, LIBRARY, 130);

	GlobalVariables.Settings[0].IniFile = GlobalVariables.PlugInIniFile;

	// set User defaults...
	GlobalVariables.UserSettings.Enable = 1;
	GlobalVariables.UserSettings.Fade = 1;
	GlobalVariables.UserSettings.UseSkinSettings = 1;
	GlobalVariables.UserSettings.AlphaFollowMainWindow = 0;
	GlobalVariables.UserSettings.LinkActivations = 0;
	GlobalVariables.UserSettings.FadeTime[ACTIVE] = 300;
	GlobalVariables.UserSettings.FadeTime[MOUSEOVER] = 100;
	GlobalVariables.UserSettings.FadeTime[INACTIVE] = 1000;
	GlobalVariables.UserSettings.FadeTimerPeriod = 20;
	GlobalVariables.UserSettings.MouseOver = 1;
}

void ConfigWrite()
{
	int i;
	char buf[34];

	// clear out the old stuff
	WritePrivateProfileString(szAppName, NULL, NULL, GlobalVariables.PlugInIniFile);

	itoa(GlobalVariables.UserSettings.Enable, buf, 10);
	WritePrivateProfileString(szAppName, "Enable", buf, GlobalVariables.PlugInIniFile);

	wsprintf(buf, "%d", GlobalVariables.UserSettings.Fade);
	WritePrivateProfileString(szAppName, "Fade", buf, GlobalVariables.PlugInIniFile);

	wsprintf(buf, "%d", GlobalVariables.UserSettings.UseSkinSettings);
	WritePrivateProfileString(szAppName, "UseSkinSettings", buf, GlobalVariables.PlugInIniFile);

	wsprintf(buf, "%d", GlobalVariables.UserSettings.LinkActivations);
	WritePrivateProfileString(szAppName, "LinkActivations", buf, GlobalVariables.PlugInIniFile);

	wsprintf(buf, "%d", GlobalVariables.UserSettings.MouseOver);
	WritePrivateProfileString(szAppName, "MouseOver", buf, GlobalVariables.PlugInIniFile);

	itoa(GlobalVariables.UserSettings.AlphaFollowMainWindow, buf, 10);
	WritePrivateProfileString(szAppName, "AlphaFollowMainWindow", buf,
							  GlobalVariables.PlugInIniFile);

	wsprintf(buf, "%d", GlobalVariables.UserSettings.FadeTimerPeriod);
	WritePrivateProfileString(szAppName, "FadeTimerPeriod", buf, GlobalVariables.PlugInIniFile);

	for (i = 0; i < ArraySize; i++)
	{
		itoa(GlobalVariables.Settings[0].alphavalues[i], buf, 10);
		WritePrivateProfileString(szAppName, RegKeys[i], buf,
								  GlobalVariables.Settings[0].IniFile.c_str());
	}

	for (i = 0; i < 3; i++)
	{
		itoa(GlobalVariables.UserSettings.FadeTime[i], buf, 10);
		WritePrivateProfileString(szAppName, RegKeysFade[i], buf, GlobalVariables.PlugInIniFile);
	}
}

void ConfigReadSkinSettings(int s)
{
	int i;

	for (i = 0; i < ArraySize; i++)
	{
		GlobalVariables.Settings[s].alphavalues[i] =
			GetPrivateProfileInt(szAppName, RegKeys[i],
								 GlobalVariables.Settings[s].alphavalues[i],
								 GlobalVariables.Settings[s].IniFile.c_str());
		GlobalVariables.Settings[s].alphavalues[i] =
			AlphaValueClamp(GlobalVariables.Settings[s].alphavalues[i]);
	}

}

void ConfigReadCustomSettings()
{
	// get the path
	char skinpath[MAX_PATH];

	// default to no skin GlobalVariables.Settings
	GlobalVariables.HaveSkinSettings = 0;
	char *e = (char *) SendMessage(GlobalVariables.plugin.hwndParent, WM_USER, (WPARAM) skinpath,
								   IPC_GETSKIN);

	if (e == NULL)
		return;

	GlobalVariables.Settings[1].IniFile = skinpath;
	_RPT1(_CRT_WARN, "IniPath=%s\n", GlobalVariables.Settings[1].IniFile.c_str());

	// is it the base skin?
	if (GlobalVariables.Settings[1].IniFile.length() != 0)
	{
		int exist = -1;

		GlobalVariables.Settings[1].IniFile += "\\plugin.ini";

		exist =
			GetPrivateProfileInt(szAppName, "MainWindow", exist,
								 GlobalVariables.Settings[1].IniFile.c_str());

		_RPT0(_CRT_WARN, "HaveSkinSettings=");
		if (exist != -1)
		{

			_RPT0(_CRT_WARN, "Yes");
			GlobalVariables.HaveSkinSettings = 1;
			ConfigReadSkinSettings(1);
		}

		_RPT0(_CRT_WARN, "\n");
	}
}

void ConfigReadUserSettings()
{
	// read user GlobalVariables.Settings
	GlobalVariables.UserSettings.Enable =
		GetPrivateProfileInt(szAppName, "Enable", GlobalVariables.UserSettings.Enable,
							 GlobalVariables.PlugInIniFile);

	GlobalVariables.UserSettings.Fade =
		GetPrivateProfileInt(szAppName, "Fade", GlobalVariables.UserSettings.Fade,
							 GlobalVariables.PlugInIniFile);
	GlobalVariables.UserSettings.UseSkinSettings =
		GetPrivateProfileInt(szAppName, "UseSkinSettings",
							 GlobalVariables.UserSettings.UseSkinSettings,
							 GlobalVariables.PlugInIniFile);

	GlobalVariables.UserSettings.AlphaFollowMainWindow =
		GetPrivateProfileInt(szAppName, "AlphaFollowMainWindow",
							 GlobalVariables.UserSettings.AlphaFollowMainWindow,
							 GlobalVariables.PlugInIniFile);

	GlobalVariables.UserSettings.LinkActivations =
		GetPrivateProfileInt(szAppName, "LinkActivations",
							 GlobalVariables.UserSettings.LinkActivations,
							 GlobalVariables.PlugInIniFile);

	GlobalVariables.UserSettings.MouseOver =
		GetPrivateProfileInt(szAppName, "MouseOver", GlobalVariables.UserSettings.MouseOver,
							 GlobalVariables.PlugInIniFile);

	GlobalVariables.UserSettings.FadeTimerPeriod =
		GetPrivateProfileInt(szAppName, "FadeTimerPeriod",
							 GlobalVariables.UserSettings.FadeTimerPeriod,
							 GlobalVariables.PlugInIniFile);

	for (int i = 0; i < 3; i++)
	{
		GlobalVariables.UserSettings.FadeTime[i] =
			GetPrivateProfileInt(szAppName, RegKeysFade[i],
								 GlobalVariables.UserSettings.FadeTime[i],
								 GlobalVariables.PlugInIniFile);
	}

	ConfigReadSkinSettings(0);
}


void Update_UseSkinSettings_UI(HWND hwndDlg)
{
	int i,
	 state;

	state = !ActiveSetting;

	for (i = 0; i < ArraySize; i++)
	{
		EnableWindow(GlobalVariables.DialogUtil.sliderctrl[i], state);
	}

	EnableWindow(GetDlgItem(hwndDlg, IDC_STATICMAINWINDOW), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATICPLAYLIST), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATICEQUALIZER), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATICMINIBROWSER), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATICLIBRARY), state);

	EnableWindow(GetDlgItem(hwndDlg, IDC_STATICACTIVEWINDOW), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATICINACTIVEWINDOW), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATICMOUSEOVER), state);

	EnableWindow(GetDlgItem(hwndDlg, IDC_ALPHAFOLLOW), state);

	for (i = 0; i < ArraySize; i++)
	{
		SendMessage(GlobalVariables.DialogUtil.sliderctrl[i], TBM_SETPOS, TRUE,
					GlobalVariables.Settings[ActiveSetting].alphavalues[i]);
	}
}

void UpdateAlphaFollowStatus(HWND hwndDlg)
{
	RECT rect;
	SetRect(&rect, 155, 55, 157, 125); OffsetRect(&rect, -1, -1); InflateRect(&rect, 2, 2);	
	MapDialogRect(hwndDlg, &rect);
	InvalidateRect(hwndDlg, &rect, TRUE);
	SetRect(&rect, 265, 55, 267, 125); OffsetRect(&rect, -1, -1); InflateRect(&rect, 2, 2);	
	MapDialogRect(hwndDlg, &rect);
	InvalidateRect(hwndDlg, &rect, TRUE);
	SetRect(&rect, 375, 55, 377, 125); OffsetRect(&rect, -1, -1); InflateRect(&rect, 2, 2);	
	MapDialogRect(hwndDlg, &rect);
	InvalidateRect(hwndDlg, &rect, TRUE);
}

void UpdateUI(HWND hwndDlg)
{
	int i;
	_ASSERT(hwndDlg == GlobalVariables.DialogUtil.ConfigDlg);

	CheckDlgButton(hwndDlg, IDC_ENABLE, GlobalVariables.UserSettings.Enable);

	CheckDlgButton(hwndDlg, IDC_FADE, GlobalVariables.UserSettings.Fade);

	CheckDlgButton(hwndDlg, IDC_ALPHAFOLLOW, GlobalVariables.UserSettings.AlphaFollowMainWindow);

	CheckDlgButton(hwndDlg, IDC_USESKINSETTINGS, GlobalVariables.UserSettings.UseSkinSettings);

	CheckDlgButton(hwndDlg, IDC_CHECKHAVESKINSETTINGS, GlobalVariables.HaveSkinSettings);

	CheckDlgButton(hwndDlg, IDC_LINKACTIVATIONS, GlobalVariables.UserSettings.LinkActivations);

	CheckDlgButton(hwndDlg, IDC_MOUSEOVER, GlobalVariables.UserSettings.MouseOver);

	for (i = 0; i < 3; i++)
	{
		SendMessage(GlobalVariables.DialogUtil.fadetimectrl[i], TBM_SETPOS, TRUE,
					GlobalVariables.UserSettings.FadeTime[i]);
	}

	SendMessage(GlobalVariables.DialogUtil.fadetimerperiodctrl, TBM_SETPOS, TRUE,
				GlobalVariables.UserSettings.FadeTimerPeriod);

	Update_UseSkinSettings_UI(hwndDlg);

	UpdateAlphaFollowStatus(hwndDlg);
}

void EnableExpandedControls(HWND m_hWnd, BOOL bEnabled)
{
	HWND hWndChild = GetDlgItem(m_hWnd, IDC_MODEBREAK);

	while (hWndChild != NULL)
	{
		EnableWindow(hWndChild, bEnabled);
		hWndChild = GetNextWindow(hWndChild, GW_HWNDNEXT);
	}
}

void ExpandContractDlg(HWND hwndDlg)
{
	RECT rcDlg;

	GetWindowRect(hwndDlg, &rcDlg);

	if (GlobalVariables.DialogUtil.m_bExpanded)
	{
		MoveWindow(hwndDlg, rcDlg.left, rcDlg.top,
				   rcDlg.right - rcDlg.left, GlobalVariables.DialogUtil.m_nNormalHeight, TRUE);
	}
	else
	{
		MoveWindow(hwndDlg, rcDlg.left, rcDlg.top,
				   rcDlg.right - rcDlg.left, GlobalVariables.DialogUtil.m_nExpandedHeight, TRUE);
	}

	GlobalVariables.DialogUtil.m_bExpanded = !GlobalVariables.DialogUtil.m_bExpanded;

	EnableExpandedControls(hwndDlg, GlobalVariables.DialogUtil.m_bExpanded);
}

void InitContractDialog(HWND hwndDlg)
{
	RECT rcDlg,
	 rcMarker;
	GetWindowRect(hwndDlg, &rcDlg);

	GlobalVariables.DialogUtil.m_nExpandedHeight = rcDlg.bottom - rcDlg.top;

	GetWindowRect(GetDlgItem(hwndDlg, IDC_MODEBREAK), &rcMarker);

	GlobalVariables.DialogUtil.m_nNormalHeight = (rcMarker.top - rcDlg.top);

	MoveWindow(hwndDlg, rcDlg.left, rcDlg.top, rcDlg.right - rcDlg.left,
			   GlobalVariables.DialogUtil.m_nNormalHeight, TRUE);

	GlobalVariables.DialogUtil.m_bExpanded = FALSE;

	EnableExpandedControls(hwndDlg, GlobalVariables.DialogUtil.m_bExpanded);
}

BOOL CALLBACK ConfigureProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			int i;

			GlobalVariables.DialogUtil.ConfigDlg = hwndDlg;

			for (i = 0; i < ArraySize; i++)
			{
				// remember the handles for later use
				GlobalVariables.DialogUtil.sliderctrl[i] = GetDlgItem(hwndDlg, CtrlID[i]);

				// set range, tick frequency
				SendMessage(GlobalVariables.DialogUtil.sliderctrl[i], TBM_SETRANGE, FALSE,
							MAKELONG(15, 255));
				SendMessage(GlobalVariables.DialogUtil.sliderctrl[i], TBM_SETTICFREQ, 24, 0);

				// position is updated in Update_UseSkinSettings_UI
			}

			for (i = 0; i < 3; i++)
			{
				// remember the handles for later use
				GlobalVariables.DialogUtil.fadetimectrl[i] = GetDlgItem(hwndDlg, CtrlIDFade[i]);

				// set range, tick frequency
				SendMessage(GlobalVariables.DialogUtil.fadetimectrl[i], TBM_SETRANGE,
							FALSE, MAKELONG(0, 1500));
				SendMessage(GlobalVariables.DialogUtil.fadetimectrl[i], TBM_SETTICFREQ, 100, 0);

			}

			GlobalVariables.DialogUtil.fadetimerperiodctrl =
				GetDlgItem(hwndDlg, IDC_FADETIMERPERIOD);

			// set range, tick frequency
			SendMessage(GlobalVariables.DialogUtil.fadetimerperiodctrl, TBM_SETRANGE, FALSE,
						MAKELONG(10, 100));
			SendMessage(GlobalVariables.DialogUtil.fadetimerperiodctrl, TBM_SETTICFREQ, 10, 0);

			UpdateUI(hwndDlg);
			InitContractDialog(hwndDlg);

		}
		return FALSE;
	case WM_HSCROLL:
		{
			int i;
			HWND wnd = (HWND) lParam;
			int value = SendMessage(wnd, TBM_GETPOS, 0, 0);

			_ASSERT(ActiveSetting == 0);	// only when the user can change
			// GlobalVariables.Settings

			// remember which Winamp window
			int WinampWindow = -1;
			int State = -1;

			// check alpha GlobalVariables.Settings for change
			for (i = 0; i < ArraySize; i++)
			{
				if (wnd == GlobalVariables.DialogUtil.sliderctrl[i])
				{
					WinampWindow = i / 3;
					State = i % 3;
					GlobalVariables.Settings[ActiveSetting].alphavalues[i] = value;
					SetLayeredWindowAttributes(GlobalVariables.UIInfo
											   [WinampWindow].hwnd, 0, value, LWA_ALPHA);
					GlobalVariables.UIInfo[WinampWindow].currentalpha = value;
					break;
				}
			}

			// see if we actually changed a alpha
			if (WinampWindow != -1)
			{
				// is follow on?
				if (GlobalVariables.UserSettings.AlphaFollowMainWindow)
				{
					if (WinampWindow == MAINWINDOW)
					{
						// move all the other sliders
						for (i = 1; i < NumWindows; i++)
						{
							GlobalVariables.Settings[ActiveSetting].alphavalue(State, i, value);							
							SendMessage(GlobalVariables.DialogUtil.sliderctrl
										[idx(State, i)], TBM_SETPOS, TRUE, value);
						}
						SetLayeredWindowAttributes
							(GlobalVariables.UIInfo[PLAYLISTEDITOR].hwnd, 0, value, LWA_ALPHA);
						GlobalVariables.UIInfo[PLAYLISTEDITOR].currentalpha = value;

						SetLayeredWindowAttributes
							(GlobalVariables.UIInfo[EQUALIZER].hwnd, 0, value, LWA_ALPHA);
						GlobalVariables.UIInfo[EQUALIZER].currentalpha = value;
						
						SetLayeredWindowAttributes
							(GlobalVariables.UIInfo[MINIBROWSER].hwnd, 0, value, LWA_ALPHA);
						GlobalVariables.UIInfo[MINIBROWSER].currentalpha = value;
						
						SetLayeredWindowAttributes
							(GlobalVariables.UIInfo[LIBRARY].hwnd, 0, value, LWA_ALPHA);
						GlobalVariables.UIInfo[LIBRARY].currentalpha = value;

					}
					else
					{
						// turn off follow
						if (GlobalVariables.Settings[ActiveSetting].alphavalue(State, MAINWINDOW) !=
							value)
						{
							GlobalVariables.UserSettings.AlphaFollowMainWindow = 0;

							CheckDlgButton
								(hwndDlg, IDC_ALPHAFOLLOW,
								 GlobalVariables.UserSettings.AlphaFollowMainWindow);

							UpdateAlphaFollowStatus(hwndDlg);
						}

					}
				}
			}
			else if (wnd == GlobalVariables.DialogUtil.fadetimerperiodctrl)
				GlobalVariables.UserSettings.FadeTimerPeriod = value;
			else if (wnd == GlobalVariables.DialogUtil.fadetimectrl[ACTIVE])
				GlobalVariables.UserSettings.FadeTime[ACTIVE] = value;
			else if (wnd == GlobalVariables.DialogUtil.fadetimectrl[MOUSEOVER])
				GlobalVariables.UserSettings.FadeTime[MOUSEOVER] = value;
			else if (wnd == GlobalVariables.DialogUtil.fadetimectrl[INACTIVE])
				GlobalVariables.UserSettings.FadeTime[INACTIVE] = value;
		}
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			ConfigWrite();
			SetTransparency();
			GlobalVariables.DialogUtil.ConfigDlg = NULL;
			DestroyWindow(hwndDlg);
			return FALSE;
		case IDC_ENABLE:
			{
				GlobalVariables.UserSettings.Enable =
					IsDlgButtonChecked(hwndDlg, IDC_ENABLE) == BST_CHECKED;
				for(int i = 0; i < NumWindows; i++)
					ChangeWindowStyle(i);

				UpdateUI(hwndDlg);
				SetTransparency();
			}
			return FALSE;
		case IDC_FADE:
			{
				GlobalVariables.UserSettings.Fade =
					IsDlgButtonChecked(hwndDlg, IDC_FADE) == BST_CHECKED;
			}
			return FALSE;
		case IDC_USESKINSETTINGS:
			{
				GlobalVariables.UserSettings.UseSkinSettings =
					IsDlgButtonChecked(hwndDlg, IDC_USESKINSETTINGS) == BST_CHECKED;

				Update_UseSkinSettings_UI(hwndDlg);
				SetTransparency();
			}
			return FALSE;
		case IDC_ALPHAFOLLOW:
			{
				_ASSERT(ActiveSetting == 0);	// only when the user can change
				// GlobalVariables.Settings
				GlobalVariables.UserSettings.AlphaFollowMainWindow =
					IsDlgButtonChecked(hwndDlg, IDC_ALPHAFOLLOW) == BST_CHECKED;

				UpdateAlphaFollowStatus(hwndDlg);
			}
			return FALSE;
		case IDC_LINKACTIVATIONS:
			{
				GlobalVariables.UserSettings.LinkActivations =
					IsDlgButtonChecked(hwndDlg, IDC_LINKACTIVATIONS) == BST_CHECKED;
			}
			return FALSE;
		case IDC_OPENWEBPAGE:
			GotoURL("http://www.draconpern.com", SW_SHOWDEFAULT);
			return FALSE;
		case IDC_CONFIGMODE:
			{
				ExpandContractDlg(hwndDlg);
			}
			return FALSE;
		case IDC_SETDEFAULTS:
			{
				SetDefaults();
				UpdateUI(hwndDlg);
			}
			return FALSE;
		case IDC_MOUSEOVER:
			{
				GlobalVariables.UserSettings.MouseOver =
					IsDlgButtonChecked(hwndDlg, IDC_MOUSEOVER) == BST_CHECKED;
			}
			return FALSE;
		}
		return TRUE;
		/*
		 * case WM_HELP: { HELPINFO *lphi = (LPHELPINFO) lParam; RECT rect; HH_POPUP hPop; // HTML 
		 * Help popup structure // Initialize structure to NULLs memset (&hPop, 0, sizeof (hPop));
		 * // Set size of structure hPop.cbStruct = sizeof (hPop); hPop.hinst = NULL; hPop.idString
		 * = 0;//lphi->dwContextId; hPop.pszText = "Hello"; GetWindowRect((HWND)lphi->hItemHandle,
		 * &rect); hPop.pt.x = rect.left; hPop.pt.y = rect.top; hPop.clrBackground = -1;
		 * hPop.clrForeground = -1; hPop.rcMargins.top = -1; hPop.rcMargins.left = -1;
		 * hPop.rcMargins.bottom = -1; hPop.rcMargins.right = -1; hPop.pszFont = NULL; HWND popup =
		 * HtmlHelp (hwndDlg, NULL, HH_DISPLAY_TEXT_POPUP, (DWORD) & hPop); //GetWindowRect (popup,
		 * &rect); //MoveWindow(popup, lphi->MousePos.x, lphi->MousePos.y, rect.right - rect.left,
		 * rect.bottom - rect.top, TRUE); } return FALSE; 
		 */
	case WM_PAINT:
		{
			RECT rect;
			if (GetUpdateRect(hwndDlg, &rect, TRUE) != 0
				&& GlobalVariables.UserSettings.AlphaFollowMainWindow)
			{
				PAINTSTRUCT ps;
				HPEN hPen, hPenOld;
				LOGBRUSH lb;
				RECT rect;
				HDC hdc = BeginPaint(hwndDlg, &ps);				

				// Initialize the pen's brush.
				lb.lbStyle = BS_SOLID;
				lb.lbColor = GetSysColor(COLOR_BTNTEXT);
				lb.lbHatch = 0;

				hPen = ExtCreatePen(PS_GEOMETRIC, 3, &lb, 0, NULL);
				hPenOld = (HPEN) SelectObject(hdc, hPen);

				SetRect(&rect, 155, 55, 157, 125);				
				MapDialogRect(hwndDlg, &rect);
				MoveToEx(hdc, rect.left, rect.top, NULL);
				LineTo(hdc, rect.right, rect.top);
				LineTo(hdc, rect.right, rect.bottom);
				LineTo(hdc, rect.left, rect.bottom);

				SetRect(&rect, 265, 55, 267, 125);
				MapDialogRect(hwndDlg, &rect);
				MoveToEx(hdc, rect.left, rect.top, NULL);
				LineTo(hdc, rect.right, rect.top);
				LineTo(hdc, rect.right, rect.bottom);
				LineTo(hdc, rect.left, rect.bottom);

				SetRect(&rect, 375, 55, 377, 125);
				MapDialogRect(hwndDlg, &rect);
				MoveToEx(hdc, rect.left, rect.top, NULL);
				LineTo(hdc, rect.right, rect.top);
				LineTo(hdc, rect.right, rect.bottom);
				LineTo(hdc, rect.left, rect.bottom);

				SelectObject(hdc, hPenOld);
				DeleteObject(hPen);
				EndPaint(hwndDlg, &ps);
			}
			// must return false here, otherwise nothing gets drawn
			return FALSE;
		}
	}
	return FALSE;
}

void config()
{
	if (GlobalVariables.DialogUtil.ConfigDlg == NULL)
	{
		// GlobalVariables.DialogUtil.ConfigDlg is set in WM_INITDIALOG
		CreateDialog(GlobalVariables.plugin.hDllInstance,
					 MAKEINTRESOURCE(IDD_CONFIGURE), GlobalVariables.plugin.hwndParent,
					 ConfigureProc);

		if (GlobalVariables.DialogUtil.ConfigDlg != NULL)
			ShowWindow(GlobalVariables.DialogUtil.ConfigDlg, SW_SHOW);

	}

	else
	{
		ShowWindow(GlobalVariables.DialogUtil.ConfigDlg, SW_RESTORE);
		BringWindowToTop(GlobalVariables.DialogUtil.ConfigDlg);
		SetForegroundWindow(GlobalVariables.DialogUtil.ConfigDlg);
	}
}
