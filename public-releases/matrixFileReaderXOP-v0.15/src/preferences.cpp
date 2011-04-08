/*
	The file preferences.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include <string.h>

#include "preferences.h"

#include "globaldata.h"
#include "utils.h"

void loadXOPPreferences(){

	XOPprefStructHandle prefHandle;

	GetXOPPrefsHandle((Handle*)&prefHandle);

	if( prefHandle && (*prefHandle)->version == XOPprefStruct_VERSION ){

		debugOutputToHistory("Loading preferences from file",true);

		size_t len = sizeof(globDataPtr->openDlgInitialDir);
		strncpy(globDataPtr->openDlgInitialDir,(*prefHandle)->openDlgInitialDir,len);
		globDataPtr->openDlgInitialDir[len-1]='\0';

		globDataPtr->openDlgFileIndex = (*prefHandle)->openDlgFileIndex;

		sprintf(globDataPtr->outputBuffer,"openDlgFileIndex=%d",(*prefHandle)->openDlgFileIndex);
		debugOutputToHistory(globDataPtr->outputBuffer);
		sprintf(globDataPtr->outputBuffer,"openDlgInitialDir=%s",(*prefHandle)->openDlgInitialDir);
		debugOutputToHistory(globDataPtr->outputBuffer);

	}else{
		debugOutputToHistory("No preferences saved or the internal structure has changed, loading defaults",true);
		globDataPtr->openDlgFileIndex=1;
		strcpy(globDataPtr->openDlgInitialDir,"");
	}

	if(prefHandle != NULL){
		DisposeHandle((Handle) prefHandle);
	}
};

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
		strncpy((*prefHandle)->openDlgInitialDir,globDataPtr->openDlgInitialDir,len);
		(*prefHandle)->openDlgInitialDir[len-1]='\0';

		// remember the selected file type in the open dialog from MFR_OpenResultFile
		(*prefHandle)->openDlgFileIndex = globDataPtr->openDlgFileIndex;

		// save struct on disc
		SaveXOPPrefsHandle((Handle)prefHandle);
		DisposeHandle((Handle)prefHandle);
	}
};
