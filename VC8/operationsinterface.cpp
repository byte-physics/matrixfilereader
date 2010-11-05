#include "header.h"

#include "operationstructs.h"
#include "operationsinterface.h"

#include <algorithm>
#include <string>
#include <math.h>

#include "utils.h"
#include "brickletconverter.h"
#include "globaldata.h"

namespace{
	enum TYPE{ RAW_DATA=1, CONVERTED_DATA=2, META_DATA=4};
}

#define BEGIN_OUTER_CATCH	try{
#define END_OUTER_CATCH		}\
							catch(...){\
								sprintf(globDataPtr->outputBuffer,"Unexpected exception caught in line %d, function %s,  file %s\r", __LINE__, __FUNCTION__, __FILE__);\
								XOPNotice(globDataPtr->outputBuffer);\
								globDataPtr->setError(UNKNOWN_ERROR);\
								return 0;\
							}

int ExecuteCheckForNewBricklets(CheckForNewBrickletsRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	globDataPtr->initialize(p->calledFromMacro,p->calledFromFunction);

	void* pContext  = NULL, *pBricklet = NULL;
	BrickletClass *bricklet = NULL;
	std::wstring fileName, dirPath;
	bool loadSuccess;
	int i,ret;

	// save defaults
	SetOperationNumVar(V_startBrickletID,-1.0);
	SetOperationNumVar(V_endBrickletID,-1.0);

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int oldNumberOfBricklets = pSession->getBrickletCount();

	fileName = globDataPtr->getFileNameWString();
	dirPath = globDataPtr->getDirPathWString();

	// true -> result set will be added to the database
	// false -> replaces the current results sets in the internal databse 
	loadSuccess = pSession->loadResultSet(dirPath,fileName,false);

	if(!loadSuccess){
		outputToHistory("Could not check for updates of the result file. Maybe it was moved?");
		outputToHistory("Try closing and opening the result file again.");
		return 0;
	}

	// starting from here we have to
	// - update the pBricklet pointers in the BrickletClass objects
	// - compare old to new totalNumberOfBricklets
	const int numberOfBricklets = pSession->getBrickletCount();

	for(i=1; i <= numberOfBricklets; i++ ){
		pBricklet = pSession->getNextBricklet(&pContext);
		ASSERT_RETURN_ZERO(pBricklet);

		bricklet = globDataPtr->getBrickletClassObject(i);

		if(bricklet == NULL){// this is a new bricklet
			globDataPtr->createBrickletClassObject(i,pBricklet);
		}
		else{	// the bricklet is old and we only have to update *pBricklet
			bricklet->setBrickletPointer(pBricklet);
		}
	}

	// should not happen
	if(numberOfBricklets < oldNumberOfBricklets){
		outputToHistory("Error in udating the result file. Please close and reopen it.");
		return 0;
	}

	// from here on we know that numberOfBricklets >= oldNumberOfBricklets 
	if(oldNumberOfBricklets == numberOfBricklets){
		globDataPtr->setError(NO_NEW_BRICKLETS);
		return 0;
	}

	ret = SetOperationNumVar(V_startBrickletID,oldNumberOfBricklets+1);
	if(ret != 0){
		globDataPtr->setInternalError(ret);
		return 0;
	}

	ret = SetOperationNumVar(V_endBrickletID,numberOfBricklets);
	if(ret != 0){
		globDataPtr->setInternalError(ret);
		return 0;
	}

	globDataPtr->finalize();
	END_OUTER_CATCH
	return 0;
}

int ExecuteGetResultFileMetaData(GetResultFileMetaDataRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	globDataPtr->initialize(p->calledFromMacro,p->calledFromFunction);
	SetOperationStrVar(S_waveNames,"");

	std::string waveName, fullPathOfCreatedWaves;
	std::vector<std::string> keys,values;
	char buf[ARRAY_SIZE];
	int ret;
	void *pBricklet = NULL;
	BrickletClass *bricklet = NULL;
	tm ctime;
	Vernissage::Session::BrickletMetaData brickletMetaData;

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();

	// check waveName parameter
	if (p->NFlagEncountered){
		if( GetHandleSize(p->waveName) == 0L ){
			globDataPtr->setError(WRONG_PARAMETER,"waveName");
		}
		else{
			convertHandleToString(p->waveName,waveName);
		}
	}
	else{
		waveName = resultMetaDefault;
	}

	keys.push_back("resultFilePath");
	values.push_back(globDataPtr->getDirPath());

	keys.push_back("resultFileName");
	values.push_back(globDataPtr->getFileName());

	keys.push_back("totalNumberOfBricklets");
	values.push_back(anyTypeToString<int>(numberOfBricklets));

	if(numberOfBricklets > 0){
		bricklet = globDataPtr->getBrickletClassObject(numberOfBricklets);
		ASSERT_RETURN_ZERO(bricklet);	
		pBricklet  = bricklet->getBrickletPointer();
		ASSERT_RETURN_ZERO(pBricklet);

		// use the timestamp of the last bricklet as dateOfLastChange
		ctime = pSession->getCreationTimestamp(pBricklet);

		brickletMetaData = pSession->getMetaData(pBricklet);

		keys.push_back("dateOfLastChange");
		sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d",ctime.tm_mon+1,ctime.tm_mday,ctime.tm_year+1900, ctime.tm_hour,ctime.tm_min,ctime.tm_sec);
		values.push_back(buf);

		keys.push_back("timeStampOfLastChange");
		values.push_back(anyTypeToString<time_t>(mktime(&ctime)));

		keys.push_back("BrickletMetaData.fileCreatorName");
		values.push_back(WStringToString(brickletMetaData.fileCreatorName));

		keys.push_back("BrickletMetaData.fileCreatorVersion");
		values.push_back(WStringToString(brickletMetaData.fileCreatorVersion));

		keys.push_back("BrickletMetaData.userName");
		values.push_back(WStringToString(brickletMetaData.userName));

		keys.push_back("BrickletMetaData.accountName");
		values.push_back(WStringToString(brickletMetaData.accountName));
	}
	else{
		keys.push_back("dateOfLastChange");
		values.push_back("");

		keys.push_back("timeStampOfLastChange");
		values.push_back("");

		keys.push_back("BrickletMetaData.fileCreatorName");
		values.push_back("");

		keys.push_back("BrickletMetaData.fileCreatorVersion");
		values.push_back("");

		keys.push_back("BrickletMetaData.userName");
		values.push_back("");

		keys.push_back("BrickletMetaData.accountName");
		values.push_back("");
	}

	// brickletID=0 because we are handling resultfile metadata
	ret = createAndFillTextWave(keys,values,NULL,waveName.c_str(),0,fullPathOfCreatedWaves);

	if(ret == WAVE_EXIST){
		globDataPtr->setError(ret,waveName);
		return 0;
	}
	else if(ret != 0){
		globDataPtr->setInternalError(ret);
		return 0;
	}

	SetOperationStrVar(S_waveNames,fullPathOfCreatedWaves.c_str());
	bool clearCache=true;
	globDataPtr->finalize(clearCache);
	END_OUTER_CATCH
	return 0;
}

int ExecuteCreateOverviewTable(CreateOverviewTableRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	globDataPtr->initialize(p->calledFromMacro,p->calledFromFunction);
	SetOperationStrVar(S_waveNames,"");

	int ret=-1;
	std::string keyList, key, value, waveName;
	waveHndl waveHandle;
	DataFolderHandle parentDataFolderHPtr = NULL;
	BrickletClass *bricklet=NULL;
	int i, j;

	CountInt dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));

	std::vector<std::string> keys, textWaveContents;

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		globDataPtr->setError(EMPTY_RESULTFILE);
		return 0;
	}

	// check keyList parameter
	if (p->KEYSFlagEncountered) {
		if( GetHandleSize(p->keyList) == 0L ){
			globDataPtr->setError(WRONG_PARAMETER,"keyList");
			return 0;
		}
		else{
			convertHandleToString(p->keyList,keyList);
		}
	}
	else{
		keyList = keyList_default;
	}

	// check waveName parameter
	if (p->NFlagEncountered){
		if( GetHandleSize(p->waveName) == 0L ){
			globDataPtr->setError(WRONG_PARAMETER,"waveName");
		}
		else{
			convertHandleToString(p->waveName,waveName);
		}
	}
	else{
		waveName = overViewTableDefault;
	}

	char sepChar[] = ";";
	splitString(keyList,sepChar,keys);

	if( keys.size() == 0 ){
		globDataPtr->setError(WRONG_PARAMETER,"keyList");
		return 0;
	}

	dimensionSizes[ROWS] = numberOfBricklets;
	dimensionSizes[COLUMNS] = keys.size();

	ret = MDMakeWave(&waveHandle,waveName.c_str(),NULL,dimensionSizes,TEXT_WAVE_TYPE,globDataPtr->overwriteEnabledAsInt());
	if(ret == NAME_WAV_CONFLICT){
		sprintf(globDataPtr->outputBuffer,"Wave %s already exists.",waveName);
		debugOutputToHistory(globDataPtr->outputBuffer);
		globDataPtr->setError(WAVE_EXIST,waveName);
		return 0;
	}
	else if(ret != 0 ){
		globDataPtr->setInternalError(ret);
		return 0;
	}

	ASSERT_RETURN_ZERO(waveHandle);

	for(j=0; j < keys.size(); j++){

		key = keys.at(j);
		MDSetDimensionLabel(waveHandle,COLUMNS,j,const_cast<char *> (key.c_str()));
		sprintf(globDataPtr->outputBuffer,"key=%s",key.c_str());
		debugOutputToHistory(globDataPtr->outputBuffer);

		for(i=1; i <= numberOfBricklets; i++){
			bricklet = globDataPtr->getBrickletClassObject(i);
			ASSERT_RETURN_ZERO(bricklet);
			value = bricklet->getMetaDataValueAsString(key);
			textWaveContents.push_back(value);

			sprintf(globDataPtr->outputBuffer,"   value=%s",value.c_str());
			debugOutputToHistory(globDataPtr->outputBuffer);
		}
	}

	ret = stringVectorToTextWave(textWaveContents,waveHandle);
	if(ret != 0){
		globDataPtr->setInternalError(ret);
		return 0;
	}

	// brickletID equals 0 because the wave note is for a resultfile kind wave
	setOtherWaveNote(0,waveHandle);

	ret = GetCurrentDataFolder(&parentDataFolderHPtr);
	if(ret != 0){
		globDataPtr->setInternalError(ret);
		return 0;
	}

	SetOperationStrVar(S_waveNames,getFullWavePath(parentDataFolderHPtr,waveHandle).c_str());
	bool clearCache=true;
	globDataPtr->finalize(clearCache);
	END_OUTER_CATCH
	return 0;
}

int ExecuteGetReportTemplate(GetReportTemplateRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	SetOperationStrVar(S_value,"");

	std::string str;

	// get windows version
	// see http://msdn.microsoft.com/en-us/library/ms724451%28VS.85%29.aspx
	OSVERSIONINFO osvi;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	str.append("####\r");

	BOOL retValue = GetVersionEx(&osvi);
	// non zero is success here
	if(retValue != 0){
		str.append("Windows version: " + anyTypeToString<int>(osvi.dwMajorVersion) + "." + anyTypeToString<int>(osvi.dwMinorVersion) + " (Build " + anyTypeToString<int>(osvi.dwBuildNumber) + ")\r" );
	}
	else{
		str.append("Windows version: unknown\r");
	}

#if defined(_MSC_VER)
	str.append("Visual Studio version: " + anyTypeToString<int>(_MSC_VER) + "\r");
#elif defined(__GNUC__)
	str.append("GCC version: " __VERSION__ "\r");
#else
	str.append("Unknown compiler version\r");
#endif

	str.append("Igor Pro Version: " + anyTypeToString<int>(igorVersion) + "\r");
	str.append("Vernissage version: " + globDataPtr->getVernissageVersion() + "\r");
	str.append("XOP version: " + std::string(myXopVersion) + "\r");
	str.append("Compilation date and time: " __DATE__ " " __TIME__ "\r");
	str.append("\r");
	str.append("Your Name:\r");
	str.append("Bug description:\r");
	str.append("####\r");

	SetOperationStrVar(S_value,str.c_str());

	outputToHistory(str.c_str());
	globDataPtr->finalize();
	END_OUTER_CATCH
	return 0;
}

int ExecuteGetBrickletData(GetBrickletDataRuntimeParamsPtr p){

	GenericGetBrickletParams params;

	params.calledFromFunction	= p->calledFromFunction;
	params.calledFromMacro		= p->calledFromMacro;

	params.NFlagEncountered		= p->NFlagEncountered;
	params.NFlagParamsSet[0]	= p->NFlagParamsSet[0];
	params.baseName				= p->baseName;

	params.RFlagEncountered		= p->RFlagEncountered;
	params.RFlagParamsSet[0]	= p->RFlagParamsSet[0];
	params.RFlagParamsSet[1]	= p->RFlagParamsSet[1];
	params.startBrickletID		= p->startBrickletID;
	params.endBrickletID		= p->endBrickletID;

	params.SFlagEncountered		= p->SFlagEncountered;
	params.SFlagParamsSet[0]	= p->SFlagParamsSet[0];
	params.pixelSize					= p->pixelSize;

	return GenericGetBricklet(&params,CONVERTED_DATA);
}

int ExecuteGetBrickletMetaData(GetBrickletMetaDataRuntimeParamsPtr p){

	GenericGetBrickletParams params;

	params.calledFromFunction	= p->calledFromFunction;
	params.calledFromMacro		= p->calledFromMacro;

	params.NFlagEncountered		= p->NFlagEncountered;
	params.NFlagParamsSet[0]	= p->NFlagParamsSet[0];
	params.baseName				= p->baseName;

	params.RFlagEncountered		= p->RFlagEncountered;
	params.RFlagParamsSet[0]	= p->RFlagParamsSet[0];
	params.RFlagParamsSet[1]	= p->RFlagParamsSet[1];
	params.startBrickletID		= p->startBrickletID;
	params.endBrickletID		= p->endBrickletID;

	params.SFlagEncountered		= 0;
	params.SFlagParamsSet[0]	= 0;
	params.pixelSize			= 0.0;

	return GenericGetBricklet(&params,META_DATA);
}

int ExecuteGetBrickletRawData(GetBrickletRawDataRuntimeParamsPtr p){

	GenericGetBrickletParams params;

	params.calledFromFunction	= p->calledFromFunction;
	params.calledFromMacro		= p->calledFromMacro;

	params.NFlagEncountered		= p->NFlagEncountered;
	params.NFlagParamsSet[0]	= p->NFlagParamsSet[0];
	params.baseName				= p->baseName;

	params.RFlagEncountered		= p->RFlagEncountered;
	params.RFlagParamsSet[0]	= p->RFlagParamsSet[0];
	params.RFlagParamsSet[1]	= p->RFlagParamsSet[1];
	params.startBrickletID		= p->startBrickletID;
	params.endBrickletID		= p->endBrickletID;

	params.SFlagEncountered		= 0;
	params.SFlagParamsSet[0]	= 0;
	params.pixelSize			= 0.0;

	return GenericGetBricklet(&params,RAW_DATA);
}

int GenericGetBricklet(GenericGetBrickletParamsPtr p,int typeOfData){
	BEGIN_OUTER_CATCH
	globDataPtr->initialize(p->calledFromMacro,p->calledFromFunction);
	SetOperationStrVar(S_waveNames,"");

	const char *sepChar = ";";
	std::string fullPathOfCreatedWaves, baseName;
	std::vector<std::string> keys, values;
	BrickletClass *bricklet = NULL;
	char waveName[ARRAY_SIZE], dataFolderName[MAX_OBJ_NAME+1];
	DataFolderHandle parentDataFolderHPtr = NULL, newDataFolderHPtr = NULL;
	int brickletID=-1, ret=-1;
	int startBrickletID=-1, endBrickletID=-1;
	int pixelSize;
	bool resampleData;

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		globDataPtr->setError(EMPTY_RESULTFILE);
		return 0;
	}

	// By default the range is 1 to totalNumberOfBricklets (aka all bricklets)
	startBrickletID = 1;
	endBrickletID = numberOfBricklets;	

	// the /R flag specifies a single brickletID or a brickletID range
	// Parameter: p->startBrickletID
	// Parameter: p->endBrickletID [optional]
	if(p->RFlagEncountered){
		startBrickletID = int(p->startBrickletID);
		if (p->RFlagParamsSet[1]){// endBrickletID is set
			endBrickletID   = int(p->endBrickletID);
		}
		else{
			endBrickletID  = int(p->startBrickletID); // the range is restricted to one bricklet given by startBrickletID
		}
	}

	sprintf(globDataPtr->outputBuffer,"startBrickletID=%d, endBrickletID=%d",startBrickletID,endBrickletID);
	debugOutputToHistory(globDataPtr->outputBuffer);

	if(!isValidBrickletRange(startBrickletID,endBrickletID,numberOfBricklets)){
		globDataPtr->setError(INVALID_RANGE);
		return 0;
	}

	// from here on we have a none empty result set open and a valid bricklet range
	if( p->NFlagEncountered ){
		if( GetHandleSize(p->baseName) == 0L ){
			globDataPtr->setError(WRONG_PARAMETER,"\\N=");
			return 0;
		}
		else{
			convertHandleToString(p->baseName,baseName);
		}
	}
	else{
		// use the default name for the waves
		switch(typeOfData){
			case RAW_DATA:
				baseName = brickletRawDefault;
				break;
			case CONVERTED_DATA:
				baseName = brickletDataDefault;
				break;
			case META_DATA:
				baseName = brickletMetaDefault;
				break;
			default:
				outputToHistory("BUG: Error in GenericGetBricklet");
				return 0;
				break;
		}
	}

	pixelSize=1;
	// check for possible resample flag in case we are dealing with converted data
	if( p->SFlagEncountered ){
		resampleData = true;
		pixelSize = int( floor(p->pixelSize) );
		if(pixelSize < 2 || pixelSize > maximum_pixelSize){
			globDataPtr->setError(WRONG_PARAMETER,"pixelSize");
			return 0;
		}
	}
	else{
		resampleData = false;
	}
	sprintf(globDataPtr->outputBuffer,"pixelSize=%d",pixelSize);
	debugOutputToHistory(globDataPtr->outputBuffer);

	for(brickletID=startBrickletID; brickletID <= endBrickletID; brickletID++){

		bricklet = globDataPtr->getBrickletClassObject(brickletID);
		ASSERT_RETURN_ZERO(bricklet);

		// check the length of the wave baseName
		// if we don't do it here we can not know an upper limit for char waveName[]
		// if it is too long we abort
		ret = CheckName(NULL,WAVE_OBJECT,baseName.c_str());
		if(ret == NAME_TOO_LONG){
			globDataPtr->setInternalError(ret);
			return ret;
		}

		sprintf(waveName,brickletDataFormat,baseName.c_str(),brickletID);

		// datafolder handling
		if( globDataPtr->datafolderEnabled() ){

			ret = GetCurrentDataFolder(&parentDataFolderHPtr);
			if(ret != 0){
				globDataPtr->setInternalError(ret);
				return 0;
			}

			sprintf(dataFolderName,dataFolderFormat,brickletID);
			ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);

			// continue if the datafolder alrady exists, abort on all other errors
			if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
				globDataPtr->setInternalError(ret);
				return 0;
			}
		}
		
		switch(typeOfData){
			case RAW_DATA:
				ret = createRawDataWave(newDataFolderHPtr,waveName,brickletID,fullPathOfCreatedWaves);
				break;
			case CONVERTED_DATA:
				ret = createWaves(newDataFolderHPtr,waveName,brickletID,resampleData,pixelSize,fullPathOfCreatedWaves);		
				break;
			case META_DATA:
				bricklet->getBrickletMetaData(keys,values);
				ret = createAndFillTextWave(keys,values,newDataFolderHPtr,waveName,brickletID,fullPathOfCreatedWaves);
				break;
			default:
				outputToHistory("Error in GenericGetBricklet");
				return 0;
				break;
		}
		if(!globDataPtr->dataCacheEnabled()){
			bricklet->clearCache();
		}

		if(ret == WAVE_EXIST){
			globDataPtr->setError(ret,waveName);
			return 0;
		}
		else if(ret == INTERNAL_ERROR_CONVERTING_DATA || ret == UNKNOWN_ERROR){
			globDataPtr->setError(ret);
			return 0;
		}
		else if(ret != SUCCESS){
			globDataPtr->setInternalError(ret);
			return 0;
		}

		//check for user abort
		if( SpinProcess() != 0 ){
			break;
		}
	}

	SetOperationStrVar(S_waveNames,fullPathOfCreatedWaves.c_str());

	bool clearCache=true;
	globDataPtr->finalize(clearCache,ret);
	END_OUTER_CATCH
	return 0;
}

int ExecuteGetXOPErrorMessage(GetXOPErrorMessageRuntimeParamsPtr p){
	std::string errorMessage;

	// return requested error message
	if (p->errorCodeEncountered && p->errorCodeParamsSet[0]) {
		errorMessage = globDataPtr->getErrorMessage(int(p->errorCode));
	}
	else{// get last error message
		errorMessage = globDataPtr->getLastErrorMessage();
	}
	outputToHistory(errorMessage.c_str());
	return 0;
}

int ExecuteGetResultFileName(GetResultFileNameRuntimeParamsPtr p){

	globDataPtr->initialize(p->calledFromMacro,p->calledFromFunction);
	SetOperationStrVar(S_fileName,"");
	SetOperationStrVar(S_dirPath,"");

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(NO_FILE_OPEN);
		return 0;
	}

	SetOperationStrVar(S_fileName,globDataPtr->getFileName().c_str());
	SetOperationStrVar(S_dirPath,globDataPtr->getDirPath().c_str());

	globDataPtr->finalize();
	return 0;
}

int ExecuteGetVernissageVersion(GetVernissageVersionRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	SetOperationNumVar(V_DLLversion,0);

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	SetOperationNumVar(V_DLLversion,stringToAnyType<double>(globDataPtr->getVernissageVersion()));
	END_OUTER_CATCH
	return 0;
}

int ExecuteGetVersion(GetVersionRuntimeParamsPtr p){

	SetOperationNumVar(V_XOPversion,stringToAnyType<double>(myXopVersion));
	return 0;
}

int ExecuteOpenResultFile(OpenResultFileRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	globDataPtr->initialize(p->calledFromMacro,p->calledFromFunction);

	char fullPath[MAX_PATH_LEN+1]="", fileName[MAX_PATH_LEN+1], dirPath[MAX_PATH_LEN+1], fileNameOrPath[MAX_PATH_LEN+1];
	int ret = 0,i, offset=0,count=0, maxCount=100;
	void* pContext  = NULL, *pBricklet = NULL;
	bool loadSuccess;
	
	// /K will close an possibly open result file before opening a new one
	if (p->KFlagEncountered) {
		globDataPtr->closeResultFile();
	}

	if(globDataPtr->resultFileOpen()){
		globDataPtr->setError(ALREADY_FILE_OPEN,globDataPtr->getFileName());
		return 0;
	}

	if(p->fileNameOrPathEncountered && p->fileNameOrPathParamsSet[0] \
		&& p->fileNameOrPath != NULL && GetHandleSize(p->fileNameOrPath) != 0L){

			ret = GetCStringFromHandle(p->fileNameOrPath,fileNameOrPath,MAX_PATH_LEN);
			if( ret != 0 ){
				globDataPtr->setInternalError(ret);
				return 0;
			}
			ret = GetFullPathFromSymbolicPathAndFilePath(p->pathName,fileNameOrPath,fullPath);
			if( ret != 0){
				globDataPtr->setInternalError(ret);
				return 0;
			}
	}
	else{
		// an empty or missing fileNameOrPath parameter result in an openfile dialog being displayed

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

	// from here on we have
	// filename : myName.test
	// dirPath c:\data
	// fullPath c:\data\myName.test

	if( !FullPathPointsToFolder(dirPath) ){
		globDataPtr->setError(FILE_NOT_READABLE,dirPath);
		return 0;	
	}

	if( !FullPathPointsToFile(fullPath)){
		globDataPtr->setError(FILE_NOT_READABLE,fullPath);
		return 0;	
	}

	// remove suffix \\ in the dirPath because loadResultset does not like that
	if(dirPath[strlen(dirPath)-1] == '\\'){
		dirPath[strlen(dirPath)-1] = '\0';
	}

	sprintf(globDataPtr->outputBuffer,"filename %s",fileName);
	debugOutputToHistory(globDataPtr->outputBuffer);

	sprintf(globDataPtr->outputBuffer,"dirPath %s",dirPath);
	debugOutputToHistory(globDataPtr->outputBuffer);

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	// true -> result set will be added to the database
	// false -> replaces the current results sets in the internal databse 
	loadSuccess = pSession->loadResultSet(CharPtrToWString(dirPath),CharPtrToWString(fileName),false);

	if(!loadSuccess){
		outputToHistory("Could not load the result file");
		return 0;
	}

	//starting from here the result file is valid
	globDataPtr->setResultFile(CharPtrToWString(dirPath),CharPtrToWString(fileName));

	for(i=1; i <= pSession->getBrickletCount(); i++ ){
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

int ExecuteGetBrickletCount(GetBrickletCountRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	globDataPtr->initialize(p->calledFromMacro,p->calledFromFunction);

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	SetOperationNumVar(V_count,pSession->getBrickletCount());
	globDataPtr->finalize();
	END_OUTER_CATCH
	return 0;
}

int ExecuteCloseResultFile(CloseResultFileRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	globDataPtr->initialize(p->calledFromMacro,p->calledFromFunction);

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(NO_FILE_OPEN);
		return 0;
	}
	globDataPtr->closeResultFile();

	globDataPtr->finalize();
	END_OUTER_CATCH
	return 0;
}

/*	XOPEntry()

This is the entry point from the host application to the XOP for all messages after the
INIT message.
*/
void XOPEntry(void){	
	long result = 0;

	try{
		switch (GetXOPMessage()) {
		case CLEANUP:
			// in case the user has forgotten to close the result file
			if(globDataPtr->resultFileOpen()){
				globDataPtr->closeResultFile();
			}
			// close the session and unload the DLL
			globDataPtr->closeSession();
			delete globDataPtr;
			globDataPtr = NULL;
			break;
		}
	}
	catch(...){
		XOPNotice("Unexpected exception in XOPEntry");
	}
	SetXOPResult(result);
}

/*	main(ioRecHandle)

This is the initial entry point at which the host application calls XOP.
The message sent by the host must be INIT.
main() does any necessary initialization and then sets the XOPEntry field of the
ioRecHandle to the address to be called for future messages.
*/

HOST_IMPORT int main(IORecHandle ioRecHandle){	
	XOPInit(ioRecHandle);							/* do standard XOP initialization */
	SetXOPEntry(XOPEntry);							/* set entry point for future calls */
	int errorCode;

	if (igorVersion < 620){
		SetXOPResult(REQUIRES_IGOR_620);
		return EXIT_FAILURE;
	}

	try{
		globDataPtr = new GlobalData();
	}
	catch(...){
		SetXOPResult(OUT_OF_MEMORY);
		return EXIT_FAILURE;
	}

	if (errorCode = RegisterOpenResultFile()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterCloseResultFile()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetBrickletCount()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetResultFileName()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetVernissageVersion()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetMtrxFileReaderVersion()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetBrickletData()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetBrickletMetaData()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetBrickletRawData()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetReportTemplate()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterCreateOverviewTable()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetResultFileMetaData()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterCheckForNewBricklets()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetXOPErrorMessage()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}

	SetXOPResult(0L);
	return EXIT_SUCCESS;
}