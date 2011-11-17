/*
	The file preferences.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include <string.h>

#include "preferences.h"

#include "globaldata.h"

/*
	Load the preferences from the internal igor storage currently (Igor 6.22) located in
	"C:\Dokumente und Einstellungen\thomas\Anwendungsdaten\WaveMetrics\Igor Pro 6\Igor Preferences.ipr.igr"
*/
void loadXOPPreferences(){

	XOPprefStructHandle prefHandle;

	GetXOPPrefsHandle((Handle*)&prefHandle);

	if( prefHandle && (*prefHandle)->version == XOPprefStruct_VERSION ){

		debugOutputToHistory("Loading preferences from file",true);

		size_t len = sizeof(GlobalData::Instance().openDlgInitialDir);
		strncpy(GlobalData::Instance().openDlgInitialDir,(*prefHandle)->openDlgInitialDir,len);
		GlobalData::Instance().openDlgInitialDir[len-1]='\0';

		GlobalData::Instance().openDlgFileIndex = (*prefHandle)->openDlgFileIndex;

		sprintf(GlobalData::Instance().outputBuffer,"openDlgFileIndex=%d",(*prefHandle)->openDlgFileIndex);
		debugOutputToHistory(GlobalData::Instance().outputBuffer);
		sprintf(GlobalData::Instance().outputBuffer,"openDlgInitialDir=%s",(*prefHandle)->openDlgInitialDir);
		debugOutputToHistory(GlobalData::Instance().outputBuffer);

	}
	else{
		debugOutputToHistory("No preferences saved or the internal structure has changed, loading defaults",true);
		GlobalData::Instance().openDlgFileIndex=1;
		strcpy(GlobalData::Instance().openDlgInitialDir,"");
	}

	if(prefHandle != NULL){
		DisposeHandle((Handle) prefHandle);
	}
};

// Save the preferences to the internal igor storage
void saveXOPPreferences(){

	XOPprefStructHandle prefHandle;

	prefHandle = (XOPprefStructHandle) NewHandle((BCInt) sizeof(XOPprefStruct));
	if(prefHandle != NULL){
		MemClear((char*) *prefHandle,sizeof(XOPprefStruct));

		debugOutputToHistory("Saving preferences to file",true);

		//write structure version
		(*prefHandle)->version = XOPprefStruct_VERSION;
		
		// write the starting directory of MFR_OpenResultFile
		size_t len = sizeof((*prefHandle)->openDlgInitialDir);
		strncpy((*prefHandle)->openDlgInitialDir,GlobalData::Instance().openDlgInitialDir,len);
		(*prefHandle)->openDlgInitialDir[len-1]='\0';

		// remember the selected file type in the open dialog from MFR_OpenResultFile
		(*prefHandle)->openDlgFileIndex = GlobalData::Instance().openDlgFileIndex;

		// save struct on disc
		SaveXOPPrefsHandle((Handle)prefHandle);
		DisposeHandle((Handle)prefHandle);
	}
};
