#ifndef __CONFIG_H_
#define __CONFIG_H_

#define ActiveSetting (GlobalVariables.HaveSkinSettings && GlobalVariables.GlobalVariables.UserSettings.UseSkinSettings)

void SetDefaults ();
void ConfigWrite ();
void ConfigReadCustomSettings ();
void ConfigReadUserSettings ();



#endif