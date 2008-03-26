
#pragma message("Version is here, Dialog, and VERSIONINFO block")
#define GENTFXNAMEVERSION "TransparentFX v1.12"
#define GENTFXVERSION 0x0111

#define GlobalVariables Global::Variables()

class Global
{

public:
	static void Initialize() {if (_instance == NULL) _instance = new Global ();}
	inline static Global &Variables() 
		{return *_instance;};
	static void ShutdownVariables()
		{if (_instance != NULL) delete _instance; _instance = NULL;};
	virtual ~Global() {};

protected:
	Global()
	{		
		plugin.version = GPPHDR_VER;
		plugin.description = GENTFXNAMEVERSION;
		plugin.init = init;
		plugin.config = config;
		plugin.quit = quit;
		plugin.hwndParent = NULL;
		plugin.hDllInstance = NULL;
		PlugInIniFile[0] = 0;

		WinampVersion = 0;
	}
	static Global *_instance;	
public:
	
	CSettings Settings[2];
	CUserSettings UserSettings;
	int HaveSkinSettings;
	
	CDialogUtil DialogUtil;	
	
	CUIInfo UIInfo[NumWindows];

	// flag that the OS supports layered windows...
	int WindowCanBeLayered;

	// only control windows of this process
	DWORD ThisProcessID;

	winampGeneralPurposePlugin plugin;
	DWORD WinampVersion;

	char PlugInIniFile[MAX_PATH];
};