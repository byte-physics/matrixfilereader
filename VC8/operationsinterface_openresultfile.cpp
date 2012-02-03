/*
	The file operationsinterface_openresultfile.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.h"
#include "operationsinterface.h"
#include "globaldata.h"

#include "utils_generic.h"

extern "C" int ExecuteOpenResultFile(OpenResultFileRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	GlobalData::Instance().initialize(p->calledFromMacro,p->calledFromFunction);

	char fullPath[MAX_PATH_LEN+1], fileName[MAX_FILENAME_LEN+1], dirPath[MAX_PATH_LEN+1], fileNameOrPath[MAX_PATH_LEN+1];
	int ret = 0, i, totalNumBricklets ;
	void* pContext = NULL, *pBricklet = NULL;
	bool loadSuccess;
	
	// /K will close an possibly open result file before opening a new one
	if (p->KFlagEncountered) {
		GlobalData::Instance().closeResultFile();
	}

	if(GlobalData::Instance().resultFileOpen()){
		GlobalData::Instance().setError(ALREADY_FILE_OPEN,GlobalData::Instance().getFileName());
		return 0;
	}

	if(p->fileNameOrPathEncountered && GetHandleSize(p->fileNameOrPath) != 0L){

			ret = GetCStringFromHandle(p->fileNameOrPath,fileNameOrPath,MAX_PATH_LEN);
			if( ret != 0 ){
				GlobalData::Instance().setInternalError(ret);
				return 0;
			}
			// check if we have a symbolic path
			if( p->PFlagEncountered && strlen(p->pathName) > 0L ){
				ret = GetFullPathFromSymbolicPathAndFilePath(p->pathName,fileNameOrPath,fullPath);
				if( ret != 0){
					GlobalData::Instance().setInternalError(ret);
					return 0;
				}
			}
			else{// if not, fileNameOrPath is an absolute path
				// GetNativePath ensure that the path has only either backslashes or colons but not mixed, as
				// GetDirectoryAndFileNameFromFullPath() does not like that
				ret = GetNativePath(fileNameOrPath,fullPath);
				if( ret != 0){
					GlobalData::Instance().setInternalError(ret);
					return 0;
				}
			}
	}
	// an empty or missing fileNameOrPath parameter results in an openfile dialog being displayed
	else{
		sprintf(GlobalData::Instance().outputBuffer,"dir=%s,index=%d",GlobalData::Instance().openDlgInitialDir,GlobalData::Instance().openDlgFileIndex);
		debugOutputToHistory(GlobalData::Instance().outputBuffer);

		// empty initial filename
		fullPath[0]='\0';
		ret = XOPOpenFileDialog(dlgPrompt , filterStr, &(GlobalData::Instance().openDlgFileIndex), GlobalData::Instance().openDlgInitialDir, fullPath);
		if(ret == -1){ //the user cancelled the dialog
			GlobalData::Instance().setError(WRONG_PARAMETER,"fileNameOrPath");
			return 0;
		}
		else if(ret != 0){
			GlobalData::Instance().setInternalError(ret);
			return 0;
		}
	}

	ret = GetDirectoryAndFileNameFromFullPath(fullPath,dirPath,fileName);
	if( ret == WM_BAD_FILE_NAME){
		GlobalData::Instance().setError(FILE_NOT_READABLE,fullPath);
		return 0;
	}
	else if( ret != 0){
		GlobalData::Instance().setInternalError(ret);
		return 0;
	}

	// from here on we have
	// filename : myName.test
	// dirPath c:\data
	// fullPath c:\data\myName.test

	sprintf(GlobalData::Instance().outputBuffer,"fullPath %s",fullPath);
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	sprintf(GlobalData::Instance().outputBuffer,"filename %s",fileName);
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	sprintf(GlobalData::Instance().outputBuffer,"dirPath %s",dirPath);
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	if( !FullPathPointsToFolder(dirPath) ){
		GlobalData::Instance().setError(FILE_NOT_READABLE,dirPath);
		return 0;
	}

	if( !FullPathPointsToFile(fullPath)){
		GlobalData::Instance().setError(FILE_NOT_READABLE,fullPath);
		return 0;	
	}

	RemoveAllBackslashesAtTheEnd(dirPath);

	// store the last used directory in GlobalData::Instance().openDlgInitialDir
	strncpy(GlobalData::Instance().openDlgInitialDir,dirPath,MAX_PATH_LEN+1);
	GlobalData::Instance().openDlgInitialDir[MAX_PATH_LEN]='\0';

	Vernissage::Session *pSession = GlobalData::Instance().getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	// now we convert to wide strings
	std::wstring  dirPathWString = CharPtrToWString(dirPath);
	std::wstring  fileNameWString = CharPtrToWString(fileName);

	// true -> result set will be added to the database
	// false -> replaces the current results sets in the internal databse 
	loadSuccess = pSession->loadResultSet(dirPathWString,fileNameWString,false);

	if(!loadSuccess){
		outputToHistory("Could not load the result file");
		return 0;
	}

	//starting from here the result file is valid
	GlobalData::Instance().setResultFile(dirPathWString,fileNameWString);

	totalNumBricklets = pSession->getBrickletCount();
	sprintf(GlobalData::Instance().outputBuffer,"totalNumBricklets=%d",totalNumBricklets);
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	// brickletIDs are 1-based
	for( i=1; i <= totalNumBricklets; i++){
		pBricklet = pSession->getNextBricklet(&pContext);
		ASSERT_RETURN_ZERO(pBricklet);
		try{
			GlobalData::Instance().createBrickletClassObject(i,pBricklet);
		}
		catch(CMemoryException* e){
			e->Delete();
			sprintf(GlobalData::Instance().outputBuffer,"Could not reserve memory for brickletID %d, giving up",i);
			outputToHistory(GlobalData::Instance().outputBuffer);
			break;
		}
	}

	GlobalData::Instance().finalize();
	END_OUTER_CATCH
	return 0;
}
