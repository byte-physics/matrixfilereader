/*
	The file operationsinterface_openresultfile.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "header.h"

#include "operationstructs.h"
#include "operationsinterface.h"

#include "globaldata.h"

extern "C" int ExecuteOpenResultFile(OpenResultFileRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	globDataPtr->initialize(p->calledFromMacro,p->calledFromFunction);

	char fullPath[MAX_PATH_LEN+1], fileName[MAX_PATH_LEN+1], dirPath[MAX_PATH_LEN+1], fileNameOrPath[MAX_PATH_LEN+1];
	int ret = 0, i, totalNumBricklets ;
	void* pContext = NULL, *pBricklet = NULL;
	bool loadSuccess;
	
	// /K will close an possibly open result file before opening a new one
	if (p->KFlagEncountered) {
		globDataPtr->closeResultFile();
	}

	if(globDataPtr->resultFileOpen()){
		globDataPtr->setError(ALREADY_FILE_OPEN,globDataPtr->getFileName());
		return 0;
	}

	if(p->fileNameOrPathEncountered && GetHandleSize(p->fileNameOrPath) != 0L){

			ret = GetCStringFromHandle(p->fileNameOrPath,fileNameOrPath,MAX_PATH_LEN);
			if( ret != 0 ){
				globDataPtr->setInternalError(ret);
				return 0;
			}
			// check if we have a symbolic path
			if( p->PFlagEncountered && strlen(p->pathName) > 0L ){
				ret = GetFullPathFromSymbolicPathAndFilePath(p->pathName,fileNameOrPath,fullPath);
				if( ret != 0){
					globDataPtr->setInternalError(ret);
					return 0;
				}
			}
			else{// if not, fileNameOrPath is an absolute path
				// GetNativePath ensure that the path has only either backslashes or colons but not mixed, as
				// GetDirectoryAndFileNameFromFullPath() does not like that
				ret = GetNativePath(fileNameOrPath,fullPath);
				if( ret != 0){
					globDataPtr->setInternalError(ret);
					return 0;
				}
			}
	}
	// an empty or missing fileNameOrPath parameter results in an openfile dialog being displayed
	else{
		sprintf(globDataPtr->outputBuffer,"dir=%s,index=%d",globDataPtr->openDlgInitialDir,globDataPtr->openDlgFileIndex);
		debugOutputToHistory(globDataPtr->outputBuffer);

		ret = XOPOpenFileDialog(dlgPrompt , filterStr, &(globDataPtr->openDlgFileIndex), globDataPtr->openDlgInitialDir, fullPath);
		if(ret == -1){ //the user cancelled the dialog
			globDataPtr->setError(WRONG_PARAMETER,"fileNameOrPath");
			return 0;
		}
		else if(ret != 0){
			globDataPtr->setInternalError(ret);
			return 0;
		}
	}

	ret = GetDirectoryAndFileNameFromFullPath(fullPath,dirPath,fileName);
	if( ret != 0){
		globDataPtr->setInternalError(ret);
		return 0;
	}

	// store the last used directory in globDataPtr->openDlgInitialDir
	strncpy(globDataPtr->openDlgInitialDir,dirPath,MAX_PATH_LEN+1);
	globDataPtr->openDlgInitialDir[MAX_PATH_LEN]='\0';

	// remove suffix \\ in the dirPath because loadResultset does not like that
	if(dirPath[strlen(dirPath)-1] == '\\'){
		dirPath[strlen(dirPath)-1] = '\0';
	}

	// from here on we have
	// filename : myName.test
	// dirPath c:\data
	// fullPath c:\data\myName.test

	sprintf(globDataPtr->outputBuffer,"fullPath %s",fullPath);
	debugOutputToHistory(globDataPtr->outputBuffer);

	sprintf(globDataPtr->outputBuffer,"filename %s",fileName);
	debugOutputToHistory(globDataPtr->outputBuffer);

	sprintf(globDataPtr->outputBuffer,"dirPath %s",dirPath);
	debugOutputToHistory(globDataPtr->outputBuffer);

	if( !FullPathPointsToFolder(dirPath) ){
		globDataPtr->setError(FILE_NOT_READABLE,dirPath);
		return 0;
	}

	if( !FullPathPointsToFile(fullPath)){
		globDataPtr->setError(FILE_NOT_READABLE,fullPath);
		return 0;	
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
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
	globDataPtr->setResultFile(dirPathWString,fileNameWString);

	totalNumBricklets = pSession->getBrickletCount();
	sprintf(globDataPtr->outputBuffer,"totalNumBricklets=%d",totalNumBricklets);
	debugOutputToHistory(globDataPtr->outputBuffer);

	// brickletIDs are 1-based
	for( i=1; i <= totalNumBricklets; i++){
		pBricklet = pSession->getNextBricklet(&pContext);
		ASSERT_RETURN_ZERO(pBricklet);
		try{
			globDataPtr->createBrickletClassObject(i,pBricklet);
		}
		catch(CMemoryException *e){
			e->Delete();
			sprintf(globDataPtr->outputBuffer,"Could not reserve memory for brickletID %d, giving up",i);
			outputToHistory(globDataPtr->outputBuffer);
			break;
		}
	}

	globDataPtr->finalize();
	END_OUTER_CATCH
	return 0;
}
