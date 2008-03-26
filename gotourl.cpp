// from http://www.wdj.com/archive/0808/feature.html

#define _WIN32_WINNT 0x500

#include <windows.h>
#include <shellapi.h>
#include <string.h>
#include "gotourl.h"


long
GetRegKey (HKEY key, char *subkey, char *retdata)
{
	long retval;
	HKEY hkey;

	retval = RegOpenKeyEx (key, subkey, 0, KEY_QUERY_VALUE, &hkey);

	if (retval == ERROR_SUCCESS)
	  {
		  long datasize = MAX_PATH;
		  char data[MAX_PATH];

		  RegQueryValue (hkey, NULL, (LPSTR) data, &datasize);

		  lstrcpy (retdata, data);
		  RegCloseKey (hkey);
	  }

	return retval;
}

BOOL
GotoURL (const char *url, int showcmd)
{
	char key[MAX_PATH + MAX_PATH];
	BOOL retflag = FALSE;

	/* if the ShellExecute() fails          */
	if ((long) ShellExecute (NULL, "open", url, NULL, NULL, showcmd) <= 32)
	  {
		  /* get the .htm regkey and lookup the program */
		  if (GetRegKey (HKEY_CLASSES_ROOT, ".htm", key) == ERROR_SUCCESS)
			{
				lstrcat (key, "\\shell\\open\\command");
				if (GetRegKey (HKEY_CLASSES_ROOT, key, key) == ERROR_SUCCESS)
				  {
					  char *pos;
					  pos = strstr (key, "\"%1\"");
					  if (pos == NULL)	/* if no quotes */
						{
							/* now check for %1, without the quotes */
							pos = strstr (key, "%1");
							if (pos == NULL)	/* if no parameter */
								pos = key + lstrlen (key) - 1;
							else
								*pos = '\0';	/* remove the parameter */
						}
					  else
						  *pos = '\0';	/* remove the parameter */

					  lstrcat (pos, " ");
					  lstrcat (pos, url);
					  if (WinExec (key, showcmd) > 31)
						  retflag = TRUE;
				  }
			}
	  }
	else
		retflag = TRUE;

	return retflag;
}

/* End of File */
