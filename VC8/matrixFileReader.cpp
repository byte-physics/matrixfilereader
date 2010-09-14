/*	TODO author affiliation, license

*/

#include "matrixFileReader.h"

#include <algorithm>
#include <string>
#include <math.h>

#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

#include "dataclass.h"
#include "utils.h"
#include "datahandling.h"

#include "globals.h"
#include "version.h"

#include "Vernissage.h"

#define SET_ERROR(A){ p->result = A; pMyData->setLastError(A); }
#define SET_ERROR_MSG(A,B){ p->result = A; pMyData->setLastError(A,B); }
#define SET_INTERNAL_ERROR(A) { SET_ERROR(UNKNOWN_ERROR); char buf[ARRAY_SIZE]; sprintf(buf,"BUG: xop internal error %d returned.",A); outputToHistory(buf);} 

// variable closeResultFile()
static int closeResultFile(closeResultFileParams *p){

	SET_ERROR(UNKNOWN_ERROR)

	if(!pMyData->resultFileOpen()){
		SET_ERROR(NO_FILE_OPEN)
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	pMyData->closeSession();

	SET_ERROR(SUCCESS)
	return 0;
}

// variable getBrickletRawData(variable brickletID, string dataWave)
static int getBrickletRawData(getBrickletRawDataParams *p){

	SET_ERROR(UNKNOWN_ERROR)

	char buf[ARRAY_SIZE];
	char dataWaveName[MAX_OBJ_NAME+1], dataFolderName[MAX_OBJ_NAME+1];
	const int *pBuffer;
	int* dataPtr = NULL;
	MyBricklet* myBricklet = NULL;
	int count=0,ret, brickletID, hState;
	DataFolderHandle parentDataFolderHPtr = NULL, newDataFolderHPtr = NULL;

	long dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));
	waveHndl waveHandle;

	if(!pMyData->resultFileOpen()){
		SET_ERROR(NO_FILE_OPEN)
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	brickletID = (int) p->brickletID;
	const int numberOfBricklets = pSession->getBrickletCount();

	if( numberOfBricklets == 0){
		SET_ERROR(EMPTY_RESULTFILE)
		return 0;
	}

	if( !isValidBrickletID(brickletID,numberOfBricklets) ){
		SET_ERROR_MSG(NON_EXISTENT_BRICKLET,anyTypeToString<int>(brickletID))
		return 0;
	}

	if( GetHandleSize(p->waveName) == 0L ){
		sprintf(dataWaveName,rawBrickletFormatString,brickletID);
	}
	else
	{
		ret = GetCStringFromHandle(p->waveName,dataWaveName,MAX_OBJ_NAME);
		if(ret != 0){
			SET_INTERNAL_ERROR(ret)
			return 0;
		}
	}

	myBricklet = pMyData->getMyBrickletObject(brickletID);
	ASSERT_RETURN_ZERO(myBricklet);

	myBricklet->getBrickletContentsBuffer(&pBuffer,count);
	ASSERT_RETURN_ZERO(pBuffer);

	if(count == 0){
		outputToHistory("BUG: Could not load bricklet contents.");
		SET_ERROR(UNKNOWN_ERROR)
		return 0;
	}
	 // create 1D wave with count points
	dimensionSizes[ROWS]=count;

	if( pMyData->datafolderEnabled() ){

		ret = GetCurrentDataFolder(&parentDataFolderHPtr);

		if(ret != 0){
			SET_INTERNAL_ERROR(ret)
				return 0;
		}
		sprintf(dataFolderName,dataFolderFormat,brickletID);
		ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);

		// continue if the datafolder alrady exists, abort on all other errors
		if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
			SET_INTERNAL_ERROR(ret)
				return 0;
		}
	}

	ret = MDMakeWave(&waveHandle,dataWaveName,newDataFolderHPtr,dimensionSizes,NT_I32,pMyData->overwriteEnabledAsInt());

	if(ret == NAME_WAV_CONFLICT){
		sprintf(buf,"Wave %s already exists.",dataWaveName);
		debugOutputToHistory(buf);
		SET_ERROR_MSG(WAVE_EXIST,dataWaveName)
		return 0;	
	}

	if(ret != 0 ){
		SET_INTERNAL_ERROR(ret)
		return 0;
	}

	// lock wave
	hState=MoveLockHandle(waveHandle);

	dataPtr = (int*) WaveData(waveHandle);

	// copy data fast :)
	memcpy(dataPtr,pBuffer,count*sizeof(int));

	HSetState((Handle) waveHandle, hState);

	setDataWaveNote(brickletID,myBricklet->getRawMin(),myBricklet->getRawMax(),myBricklet->getPhysValRawMin(),myBricklet->getPhysValRawMax(),waveHandle);

	SET_ERROR(SUCCESS)
	return 0;
}

// variable getNumberOfBricklets(variable *totalNumberOfBricklets)
static int getNumberOfBricklets(getNumberOfBrickletsParams *p){

	SET_ERROR(UNKNOWN_ERROR)

	if(!pMyData->resultFileOpen()){
		SET_ERROR(NO_FILE_OPEN)
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	*p->totalNumberOfBricklets = pSession->getBrickletCount();

	SET_ERROR(SUCCESS)
	return 0;
}

// variable getResultFile(string *fileName,string *dirPath)
static int getResultFile(getResultFileParams *p){

	SET_ERROR(UNKNOWN_ERROR)

	if(!pMyData->resultFileOpen()){
		SET_ERROR(NO_FILE_OPEN)
		return 0;
	}

	//filename
	if(*p->fileName == NULL){
		*p->fileName = NewHandle(0L);
	}

	int ret = PutCStringInHandle(pMyData->getFileName().c_str(), *p->fileName);
	if( ret != 0 ){
		SET_INTERNAL_ERROR(ret)
		return 0;
	}

	//dirname
	if(*p->dirPath == NULL){
		*p->dirPath = NewHandle(0L);
	}

	ret = PutCStringInHandle(pMyData->getDirPath().c_str(), *p->dirPath);
	if( ret != 0 ){
		SET_INTERNAL_ERROR(ret)
		return 0;
	}

	SET_ERROR(SUCCESS)
	return 0;
}

// variable getVernissageVersion(double *vernissageVersion)
static int getVernissageVersion(getVernissageVersionParams *p){

	SET_ERROR(UNKNOWN_ERROR)

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	*p->vernissageVersion = stringToAnyType<double>(pMyData->getVernissageVersion());

	SET_ERROR(SUCCESS)
	return 0;
}

// variable getXOPVersion(double *xopVersion)
static int getXOPVersion(getXOPVersionParams *p){

	SET_ERROR(UNKNOWN_ERROR)

	*p->xopVersion = stringToAnyType<double>(myXopVersion);

	SET_ERROR(SUCCESS)
	return 0;
}

// variable openResultFile(string absoluteFilePath, string fileName)
static int openResultFile(openResultFileParams *p){

	SET_ERROR(UNKNOWN_ERROR)

	char fullPath[MAX_PATH_LEN+1], fileName[MAX_PATH_LEN+1], dirPath[MAX_PATH_LEN+1];
	int ret = 0,i, pos, offset=0,count=0, maxCount=100;
	void* pContext  = NULL, *pBricklet = NULL;
	char buf[ARRAY_SIZE];

	if(pMyData->resultFileOpen()){
		SET_ERROR_MSG(ALREADY_FILE_OPEN,pMyData->getFileName())
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	if( GetHandleSize(p->absoluteFilePath) == 0L){
		SET_ERROR_MSG(WRONG_PARAMETER,"absoluteFilePath")
		return 0;
	}

	if( GetHandleSize(p->fileName) != 0L){
		ret = GetCStringFromHandle(p->fileName,fileName,MAX_PATH_LEN);
		if( ret != 0 ){
			SET_INTERNAL_ERROR(ret)
				return 0;
		}

		ret = GetCStringFromHandle(p->absoluteFilePath,dirPath,MAX_PATH_LEN);
		if( ret != 0 ){
			SET_INTERNAL_ERROR(ret)
		}

		ret = MacToWinPath(dirPath);
		if( ret != 0){
				SET_INTERNAL_ERROR(ret)
				return 0;
		}
		ConcatenatePaths(dirPath,fileName,fullPath);
	}
	// empty filename, so p->absoluteFilePath has the path including the filename
	else{

		ret = GetCStringFromHandle(p->absoluteFilePath,fullPath,MAX_PATH_LEN);
		if( ret != 0 ){
				SET_INTERNAL_ERROR(ret)
				return 0;
		}

		ret = MacToWinPath(fullPath);
		if( ret != 0){
				SET_INTERNAL_ERROR(ret)
				return 0;
		}

		ret = GetDirectoryAndFileNameFromFullPath(fullPath,dirPath,fileName);
		if( ret != 0){
				SET_INTERNAL_ERROR(ret)
				return 0;
		}
	}

	// from here on we have
	// filename : myName.test
	// dirPath c:\data
	// fullPath c:\data\myName.test

	if( !FullPathPointsToFolder(dirPath) ){
		SET_ERROR_MSG(FILE_NOT_READABLE,dirPath)
		return 0;	
	}

	if( !FullPathPointsToFile(fullPath)){
		SET_ERROR_MSG(FILE_NOT_READABLE,fullPath)
		return 0;	
	}

	// remove suffix \\ in the dirPath because loadResultset does not like that
	if(dirPath[strlen(dirPath)-1] == '\\'){
		dirPath[strlen(dirPath)-1] = '\0';
	}

	sprintf(buf,"filename %s",fileName);
	debugOutputToHistory(buf);

	sprintf(buf,"dirPath %s",dirPath);
	debugOutputToHistory(buf);

	// true -> result set will be added to the database
	// false -> replaces the current results sets in the internal databse 
	bool loadSuccess = pSession->loadResultSet(CharPtrToWString(dirPath),CharPtrToWString(fileName),false);

	if(!loadSuccess){
		outputToHistory("Could not load the result file");
		SET_ERROR(UNKNOWN_ERROR)
		return 0;
	}

	//starting from here the result file is valid
	pMyData->setResultFile(dirPath,fileName);

	for(i=1; i <= pSession->getBrickletCount(); i++ ){
		pBricklet = pSession->getNextBricklet(&pContext);
		ASSERT_RETURN_ZERO(pBricklet);
		pMyData->createMyBrickletObject(i,pBricklet);
	}

	SET_ERROR(SUCCESS)
	return 0;
}

// variable checkForNewBricklets(variable *startBrickletID,variable *endBrickletID)
static int checkForNewBricklets(checkForNewBrickletsParams *p){

	SET_ERROR(UNKNOWN_ERROR)

	// save defaults
	*p->endBrickletID   = -1;
	*p->startBrickletID = -1;

	if(!pMyData->resultFileOpen()){
		SET_ERROR(NO_FILE_OPEN)
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int oldNumberOfBricklets = pSession->getBrickletCount();
	void* pContext  = NULL, *pBricklet = NULL;
	MyBricklet *myBricklet = NULL;
	int i;

	std::wstring fileName = StringToWString(pMyData->getFileName());
	std::wstring dirPath = StringToWString(pMyData->getDirPath());

	// true -> result set will be added to the database
	// false -> replaces the current results sets in the internal databse 
	bool loadSuccess = pSession->loadResultSet(dirPath,fileName,false);

	if(!loadSuccess){
		outputToHistory("Could not check for updates of the result file. Maybe it was moved?");
		outputToHistory("Try closing and opening the result file again.");
		SET_ERROR(UNKNOWN_ERROR)
		return 0;
	}

	// starting from here we have to
	// - update the pBricklet pointers in the MyBricklet objects
	// - compare old to new totalNumberOfBricklets

	const int numberOfBricklets = pSession->getBrickletCount();

	for(i=1; i <= numberOfBricklets; i++ ){
		pBricklet = pSession->getNextBricklet(&pContext);
		ASSERT_RETURN_ZERO(pBricklet);

		myBricklet = pMyData->getMyBrickletObject(i);

		if(myBricklet == NULL){// this is a new bricklet
			pMyData->createMyBrickletObject(i,pBricklet);
		}else{	// the bricklet is old and we only have to update *pBricklet
			myBricklet->setBrickletPointer(pBricklet);
		}
	}

	if(oldNumberOfBricklets == numberOfBricklets){
		SET_ERROR(NO_NEW_BRICKLETS)
		return 0;
	}
	// from here on we know that numberOfBricklets > oldNumberOfBricklets 

	// happened once so be prepared
	if(numberOfBricklets < oldNumberOfBricklets){
		SET_ERROR(UNKNOWN_ERROR)
		return 0;
	}

	*p->endBrickletID   = numberOfBricklets;
	*p->startBrickletID = oldNumberOfBricklets+1;

	SET_ERROR(SUCCESS)
	return 0;
}

// variable getAllBrickletData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletData(getAllBrickletDataParams *p){

	if(!pMyData->resultFileOpen()){
		SET_ERROR(NO_FILE_OPEN)
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	getRangeBrickletDataParams rangeParams;
	rangeParams.baseName = p->baseName;
	rangeParams.startBrickletID = 1;
	rangeParams.endBrickletID = pSession->getBrickletCount();

	getRangeBrickletData(&rangeParams);
	return 0;
}

// variable getAllBrickletMetaData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletMetaData(getAllBrickletMetaDataParams *p){

	if(!pMyData->resultFileOpen()){
		SET_ERROR(NO_FILE_OPEN)
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	getRangeBrickletMetaDataParams rangeParams;
	rangeParams.baseName = p->baseName;
	rangeParams.startBrickletID = 1;
	rangeParams.endBrickletID = pSession->getBrickletCount();

	getRangeBrickletMetaData(&rangeParams);
	return 0;
}

// variable getBrickletData(string baseName, variable separateFolderForEachBricklet, variable brickletID)
static int getBrickletData(getBrickletDataParams *p){

	getRangeBrickletDataParams rangeParams;
	rangeParams.baseName = p->baseName;
	rangeParams.startBrickletID = p->brickletID;
	rangeParams.endBrickletID = p->brickletID;

	getRangeBrickletData(&rangeParams);
	return 0;

}

// variable getBrickletMetaData(string metaData, variable brickletID)
static int getBrickletMetaData(getBrickletMetaDataParams *p){

	getRangeBrickletMetaDataParams rangeParams;
	rangeParams.baseName = p->baseName;
	rangeParams.startBrickletID = p->brickletID;
	rangeParams.endBrickletID = p->brickletID;

	getRangeBrickletMetaData(&rangeParams);
	return 0;
}

// variable getRangeBrickletData(string baseName, variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletData(getRangeBrickletDataParams *p){

	MyBricklet* myBricklet = NULL;
	std::vector<std::string> keys,values;
	char dataBaseName[MAX_OBJ_NAME+1], dataFolderName[MAX_OBJ_NAME+1], dataName[MAX_OBJ_NAME+1];
	DataFolderHandle parentDataFolderHPtr = NULL, newDataFolderHPtr = NULL;
	char buf[ARRAY_SIZE];
	int brickletID=-1, ret=-1;

	SET_ERROR(UNKNOWN_ERROR)

	if(!pMyData->resultFileOpen()){
		SET_ERROR(NO_FILE_OPEN)
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		SET_ERROR(EMPTY_RESULTFILE)
		return 0;
	}

	if(	!isValidBrickletRange(p->startBrickletID,p->endBrickletID,numberOfBricklets) ){
			SET_ERROR(INVALID_RANGE)
			return 0;
	}

	// from here on we have a none empty result set open and valid start- and end bricklet IDs
	if( GetHandleSize(p->baseName) == 0L ){
		sprintf(dataBaseName,brickletDataDefault);
	}
	else
	{
		ret = GetCStringFromHandle(p->baseName,dataBaseName,MAX_OBJ_NAME);
		if(ret != 0){
			SET_INTERNAL_ERROR(ret)
			return 0;
		}
	}

	// now we got a valid baseName
	for(brickletID=p->startBrickletID; brickletID <= p->endBrickletID; brickletID++){

		myBricklet = pMyData->getMyBrickletObject(brickletID);
		ASSERT_RETURN_ZERO(myBricklet);

		sprintf(dataName,brickletDataFormat,dataBaseName,brickletID);

		if( pMyData->datafolderEnabled() ){

			ret = GetCurrentDataFolder(&parentDataFolderHPtr);

			if(ret != 0){
				SET_INTERNAL_ERROR(ret)
				return 0;
			}
			sprintf(dataFolderName,dataFolderFormat,brickletID);
			ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);
	
			// continue if the datafolder alrady exists, abort on all other errors
			if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
				SET_INTERNAL_ERROR(ret)
				return 0;
			}
		}

		ret = createAndFillDataWave(newDataFolderHPtr,dataName,brickletID);

		if(ret == WAVE_EXIST){
			SET_ERROR_MSG(ret,dataName)
			return 0;
		}
		else if(ret == INTERNAL_ERROR_CONVERTING_DATA){
			SET_ERROR(ret)
			return 0;
		}
		else if(ret != SUCCESS){
			SET_INTERNAL_ERROR(ret)
			return 0;
		}
	}

	SET_ERROR(SUCCESS)
	return 0;
}

// variable getRangeBrickletMetaData(string baseName,variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletMetaData(getRangeBrickletMetaDataParams *p){

	MyBricklet* myBricklet = NULL;
	std::vector<std::string> keys,values;
	char metaDataBaseName[MAX_OBJ_NAME+1], dataFolderName[MAX_OBJ_NAME+1], metaDataName[MAX_OBJ_NAME+1];
	DataFolderHandle parentDataFolderHPtr = NULL, newDataFolderHPtr = NULL;
	char buf[ARRAY_SIZE];
	int brickletID=-1, ret=-1;

	SET_ERROR(UNKNOWN_ERROR)

	if(!pMyData->resultFileOpen()){
		SET_ERROR(NO_FILE_OPEN)
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		SET_ERROR(EMPTY_RESULTFILE)
		return 0;
	}

	if( !isValidBrickletRange(p->startBrickletID,p->endBrickletID,numberOfBricklets) ){
		SET_ERROR(INVALID_RANGE)
		return 0;
	}

	// from here on we have a none empty result set open and valid start- and end bricklet IDs
	if( GetHandleSize(p->baseName) == 0L ){
		sprintf(metaDataBaseName,brickletMetaDefault);
	}
	else
	{
		ret = GetCStringFromHandle(p->baseName,metaDataBaseName,MAX_OBJ_NAME);
		if(ret != 0){
			SET_INTERNAL_ERROR(ret)
			return 0;
		}
	}
	// now we got a valid baseName

	for(brickletID=p->startBrickletID; brickletID <= p->endBrickletID; brickletID++){


		myBricklet = pMyData->getMyBrickletObject(brickletID);
		ASSERT_RETURN_ZERO(myBricklet);

		sprintf(metaDataName,brickletMetaDataFormat,metaDataBaseName,brickletID);

		if( pMyData->datafolderEnabled() ){

			ret = GetCurrentDataFolder(&parentDataFolderHPtr);

			if(ret != 0){
				SET_INTERNAL_ERROR(ret)
				return 0;
			}
			sprintf(dataFolderName,dataFolderFormat,brickletID);
			ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);
	
			// continue if the datafolder alrady exists, abort on all other errors
			if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
				SET_INTERNAL_ERROR(ret)
				return 0;
			}
		}
		myBricklet->getBrickletMetaData(keys,values);

		ret = createAndFillTextWave(keys,values,newDataFolderHPtr,metaDataName,brickletID);

		if(ret == WAVE_EXIST){
			SET_ERROR_MSG(ret,metaDataName)
			return 0;
		}
		else if(ret != SUCCESS){
			SET_INTERNAL_ERROR(ret)
			return 0;
		}
	}
	SET_ERROR(SUCCESS)
	return 0;
}

// string getBugReportTemplate();
static int getBugReportTemplate(getBugReportTemplateParams *p){
	
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
	#else
		str.append("BUG: Unknown compiler\r");
	#endif
	str.append("Igor Pro Version: " + anyTypeToString<int>(igorVersion) + "\r");
	str.append("Vernissage version: " + pMyData->getVernissageVersion() + "\r");
	str.append("XOP version: " + std::string(myXopVersion) + "\r");
	str.append("Compilation date and time: " __DATE__ " " __TIME__ "\r");
	str.append("\r");
	str.append("Your Name:\r");
	str.append("Bug description:\r");
	str.append("####\r");

	p->result = NewHandle(str.size());
	PutCStringInHandle(str.c_str(),p->result);

	outputToHistory(str.c_str());
	return 0;
}

// variable getResultFileMetaData(string waveName)
static int getResultFileMetaData(getResultFileMetaDataParams *p){

	SET_ERROR(UNKNOWN_ERROR)

	char metaDataWaveName[MAX_OBJ_NAME+1];
	std::vector<std::string> keys,values;
	char buf[ARRAY_SIZE];
	int ret;
	void *pBricklet;
	tm ctime;
	Vernissage::Session::BrickletMetaData brickletMetaData;

	if(!pMyData->resultFileOpen()){
		SET_ERROR(NO_FILE_OPEN)
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	if( GetHandleSize(p->waveName) == 0L ){
		sprintf(metaDataWaveName,resultMetaDefault);
	}
	else{
		ret = GetCStringFromHandle(p->waveName,metaDataWaveName,MAX_OBJ_NAME);
		if(ret != 0){
			SET_INTERNAL_ERROR(ret)
			return 0;
		}
	}

	const int numberOfBricklets = pSession->getBrickletCount();

	if(numberOfBricklets == 0){
		SET_ERROR(EMPTY_RESULTFILE)
		return 0;
	}

	pBricklet = pMyData->getMyBrickletObject(numberOfBricklets)->getBrickletPointer();
	ASSERT_RETURN_ZERO(pBricklet);

	// use the timestamp of the last bricklet as dateOfLastChange
	ctime = pSession->getCreationTimestamp(pBricklet);

	brickletMetaData = pSession->getMetaData(pBricklet);

	keys.push_back("resultFilePath");
	values.push_back(pMyData->getDirPath());

	keys.push_back("resultFileName");
	values.push_back(pMyData->getFileName());

	keys.push_back("totalNumberOfBricklets");
	values.push_back(anyTypeToString<int>(numberOfBricklets));

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

	// brickletID=0 because we are handling resultfile metadata
	ret = createAndFillTextWave(keys,values,NULL,metaDataWaveName,0);

	if(ret == WAVE_EXIST){
		SET_ERROR_MSG(ret,metaDataWaveName)
		return 0;
	}

	if(ret != 0){
		SET_INTERNAL_ERROR(ret)
		return 0;
	}

	SET_ERROR(SUCCESS)
	return 0;
}


// variable createOverViewTable(string waveName, string keyList)
static int createOverViewTable(createOverViewTableParams *p){

	SET_ERROR(UNKNOWN_ERROR)
	char buf[ARRAY_SIZE], keyListChar[ARRAY_SIZE+1];
	int pos, offset, ret=-1,count=0, countMax=1000;
	std::string keyList, key, value;
	waveHndl waveHandle;
	long dimensionSizes[MAX_DIMENSIONS+1];

	MemClear(dimensionSizes, sizeof(dimensionSizes));

	std::vector<std::string> keys, textWaveContents;
	char waveName[MAX_OBJ_NAME+1];
	MyBricklet *myBricklet=NULL;
	int i, j;

	if(!pMyData->resultFileOpen()){
		SET_ERROR(NO_FILE_OPEN)
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		SET_ERROR(EMPTY_RESULTFILE)
		return 0;
	}

	// check keyList parameter
	if( p->keyList == NULL || GetHandleSize(p->keyList) == 0L ){
		SET_ERROR_MSG(WRONG_PARAMETER,"keyList")
		return 0;
	}
	else{
		ret = GetCStringFromHandle(p->keyList,keyListChar,ARRAY_SIZE);
		if(ret != 0){
			SET_INTERNAL_ERROR(ret)
			return 0;
		}
	}

	char sepChar[] = ";";
	splitString(keyListChar,sepChar,keys);

	if( keys.size() == 0 ){
		SET_ERROR_MSG(WRONG_PARAMETER,"keyList")
		return 0;
	}

	// check waveName parameter
	if( GetHandleSize(p->waveName) == 0L ){
		sprintf(waveName,overViewTableDefault);
	}
	else{
		ret = GetCStringFromHandle(p->waveName,waveName,MAX_OBJ_NAME);
		if(ret != 0){
			SET_INTERNAL_ERROR(ret)
			return 0;
		}
	}

	dimensionSizes[ROWS] = numberOfBricklets;
	dimensionSizes[COLUMNS] = keys.size();
	ret = MDMakeWave(&waveHandle,waveName,NULL,dimensionSizes,TEXT_WAVE_TYPE,pMyData->overwriteEnabledAsInt());

	if(ret == NAME_WAV_CONFLICT){
		sprintf(buf,"Wave %s already exists.",waveName);
		debugOutputToHistory(buf);
		SET_ERROR_MSG(WAVE_EXIST,waveName)
		return 0;
	}

	if(ret != 0 ){
		SET_INTERNAL_ERROR(ret)
		return 0;
	}

	ASSERT_RETURN_MINUSONE(waveHandle);

	for(j=0; j < keys.size(); j++){

		key = keys.at(j);
		MDSetDimensionLabel(waveHandle,COLUMNS,j,const_cast<char *> (key.c_str()));
		sprintf(buf,"key=%s",key.c_str());
		debugOutputToHistory(buf);

		for(i=1; i <= numberOfBricklets; i++){
			myBricklet = pMyData->getMyBrickletObject(i);
			value = myBricklet->getMetaDataValueAsString(key);
			textWaveContents.push_back(value);

			sprintf(buf,"   value=%s",value.c_str());
			debugOutputToHistory(buf);
		}
	}

	ret = stringVectorToTextWave(textWaveContents,waveHandle);

	if(ret != 0){
		SET_INTERNAL_ERROR(ret)
		return 0;
	}

	// brickletID equals 0 because the wave note is for a resultfile kind wave
	setOtherWaveNote(0,waveHandle);

	SET_ERROR(SUCCESS)
	return 0;
}

// variable getLastError()
static int getLastError(getLastErrorParams *p){

	p->result = pMyData->getLastError();
	return 0;
}

// string getLastErrorMessage()
static int getLastErrorMessage(getLastErrorMessageParams *p){

	std::string lastErrorMsg = pMyData->getLastErrorMessage();
	p->result = NewHandle(lastErrorMsg.size());
	
	PutCStringInHandle(lastErrorMsg.c_str(),p->result);

	return 0;
}

// string readXOPSettings();
static int readXOPSettings(readXOPSettingsParams *p){

	char buf[ARRAY_SIZE];
	std::string config;
	std::stringstream stream;

	stream << debug_option_name << "=" << pMyData->debuggingEnabled() << ";";
	stream << folder_option_name << "=" << pMyData->datafolderEnabled() << ";";
	stream << overwrite_option_name << "=" << pMyData->overwriteEnabled() << ";";
	stream << double_option_name << "=" << pMyData->doubleWaveEnabled();

	config = stream.str();

	p->result = NewHandle(config.size());

	if(p->result == NULL){
		return NOMEM;
	}

	int ret = PutCStringInHandle(config.c_str(), p->result);

	if( ret != 0 ){
		sprintf(buf,"internal error %d",ret);
		outputToHistory(buf);
	}
	return 0;
}

// variable writeXOPSettings(string config)
static int writeXOPSettings(writeXOPSettingsParams *p){

	SET_ERROR(UNKNOWN_ERROR)

	char configChar[ARRAY_SIZE];
	int ret;
	bool isValidKey;
	int value;

	std::vector<std::string> configList, splittedOption, validOptions;
	std::string optionString, key;

	validOptions.push_back(debug_option_name);
	validOptions.push_back(folder_option_name);
	validOptions.push_back(overwrite_option_name);
	validOptions.push_back(double_option_name);

	if(*p->config == NULL || GetHandleSize(p->config) == 0L){
		SET_ERROR_MSG(WRONG_PARAMETER,"config")
		return 0;
	}

	ret = GetCStringFromHandle(p->config,configChar,ARRAY_SIZE);
	if(ret != 0){
		SET_INTERNAL_ERROR(ret)
		return 0;
	}

	// an option string looks like "key1=value1;key2=value2"
	char listSepChar[] = ";";
	char optionSepChar[] = "=";

	splitString(configChar,listSepChar,configList);

	std::vector<std::string>::const_iterator it, itValidOptions;
	for(it = configList.begin(); it != configList.end(); it++){
		optionString = *it;

		splitString(const_cast<char*>(optionString.c_str()),optionSepChar,splittedOption);

		if(splittedOption.size() != 2){
			SET_ERROR_MSG(WRONG_PARAMETER,"config")
			return 0;
		}

		key   = splittedOption[0];
		value = stringToAnyType<int>(splittedOption[1]);

		// check if the key is in the list of valid keys
		isValidKey = false;
		for(itValidOptions = validOptions.begin(); itValidOptions != validOptions.end(); itValidOptions++){
			if( *itValidOptions == key ){
				isValidKey = true;
			}
		}

		if(!isValidKey){
			SET_ERROR_MSG(WRONG_PARAMETER,"config")
			return 0;
		}
		
		// check if the value is either 1 or 0
		if(value != 0 && value != 1){
			SET_ERROR_MSG(WRONG_PARAMETER,"config")
			return 0;
		}

		// now we got a valid key/value pair
		pMyData->setOption(key, value);
	}

	SET_ERROR(SUCCESS)
	return 0;
}

static long RegisterFunction()
{
	/*	NOTE:
		Some XOPs should return a result of NIL in response to the FUNCADDRS message.
		See XOP manual "Restrictions on Direct XFUNCs" section.
	*/

	int funcIndex = GetXOPItem(0);		/* which function invoked ? */
	long returnValue = NIL;

	switch (funcIndex) {
		case 0:						
			returnValue = (long) checkForNewBricklets;
			break;
		case 1:						
			returnValue = (long) closeResultFile;
			break;
		case 2:						
			returnValue = (long) createOverViewTable;
			break;
		case 3:						
			returnValue = (long) getAllBrickletData;
			break;
		case 4:						
			returnValue = (long) getAllBrickletMetaData;
			break;
		case 5:						
			returnValue = (long) getBrickletData;
			break;
		case 6:						
			returnValue = (long) getBrickletMetaData;
			break;
		case 7:						
			returnValue = (long) getBrickletRawData;
			break;
		case 8:						
			returnValue = (long) getBugReportTemplate;
			break;
		case 9:						
			returnValue = (long) getLastError;
			break;
		case 10:						
			returnValue = (long) getLastErrorMessage;
			break;
		case 11:						
			returnValue = (long) getNumberOfBricklets;
			break;
		case 12:						
			returnValue = (long) getRangeBrickletData;
			break;
		case 13:						
			returnValue = (long) getRangeBrickletMetaData;
			break;
		case 14:						
			returnValue = (long) getResultFile;
			break;
		case 15:						
			returnValue = (long) getResultFileMetaData;
			break;
		case 16:						
			returnValue = (long) getVernissageVersion;
			break;
		case 17:						
			returnValue = (long) getXOPVersion;
			break;
		case 18:						
			returnValue = (long) openResultFile;
			break;
		case 19:						
			returnValue = (long) readXOPSettings;
			break;
		case 20:						
			returnValue = (long) writeXOPSettings;
			break;

	}
	return returnValue;
}

/*	XOPEntry()

	This is the entry point from the host application to the XOP for all messages after the
	INIT message.
*/
static void XOPEntry(void)
{	
	long result = 0;

	switch (GetXOPMessage()) {
		case FUNCADDRS:
			result = RegisterFunction();
			break;
		case CLEANUP:
			doCleanup();
			break;
	}
	SetXOPResult(result);
}

/*	main(ioRecHandle)

	This is the initial entry point at which the host application calls XOP.
	The message sent by the host must be INIT.
	main() does any necessary initialization and then sets the XOPEntry field of the
	ioRecHandle to the address to be called for future messages.
*/


HOST_IMPORT void
main(IORecHandle ioRecHandle)
{	
	XOPInit(ioRecHandle);							/* do standard XOP initialization */
	SetXOPEntry(XOPEntry);							/* set entry point for future calls */

	if (igorVersion < 504){
		SetXOPResult(REQUIRES_IGOR_504);
		return;
	}

	pMyData = new myData();
	if(pMyData == NULL){ // out of memory
		SetXOPResult(OUT_OF_MEMORY);
		return;
	}

	SetXOPResult(0L);

	return;
}

void doCleanup(){

	// in case the user has forgotten to close the session
	closeResultFileParams p;
	closeResultFile(&p);
}

bool isValidBrickletRange(int startID, int endID,int numberOfBricklets){

	// brickletIDs are 1-based
	return ( startID <=  endID
		&& startID >=  1
		&& endID   >=  1
		&& startID <= numberOfBricklets
		&& endID   <= numberOfBricklets );
}

bool isValidBrickletID(int brickletID, int numberOfBricklets){

	return brickletID >= 1 && brickletID <= numberOfBricklets;
}