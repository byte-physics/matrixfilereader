/*
	The file preferences.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "stdafx.h"

#include "preferences.hpp"
#include "globaldata.hpp"

/*
	Load the preferences from the internal igor storage currently (Igor 6.22) located in
	"C:\Dokumente und Einstellungen\thomas\Anwendungsdaten\WaveMetrics\Igor Pro 6\Igor Preferences.ipr.igr"
*/
void loadXOPPreferences(){

	XOPprefStructHandle prefHandle;

	GetXOPPrefsHandle((Handle*)&prefHandle);

	if( prefHandle && (*prefHandle)->version == XOPprefStruct_VERSION ){

		DEBUGPRINT_SILENT("Loading preferences from file");

		size_t len = sizeof(GlobalData::Instance().openDlgInitialDir);
		strncpy(GlobalData::Instance().openDlgInitialDir,(*prefHandle)->openDlgInitialDir,len);
		GlobalData::Instance().openDlgInitialDir[len-1]='\0';

		GlobalData::Instance().openDlgFileIndex = (*prefHandle)->openDlgFileIndex;

		DEBUGPRINT_SILENT("openDlgFileIndex=%d",(*prefHandle)->openDlgFileIndex);
		DEBUGPRINT_SILENT("openDlgInitialDir=%s",(*prefHandle)->openDlgInitialDir);
		
	}
	else{
		DEBUGPRINT_SILENT("No preferences saved or the internal structure has changed, loading defaults");
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

		DEBUGPRINT_SILENT("Saving preferences to file");

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
