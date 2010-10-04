/*	TODO author affiliation, license

*/
#include "header.h"

#include "matrixFileReader.h"

#include <algorithm>
#include <string>
#include <math.h>

#include "utils.h"
#include "datahandling.h"
#include "dataclass.h"

// variable closeResultFile()
static int closeResultFile(closeResultFileParams *p){

	setError(&p->result,UNKNOWN_ERROR);

	if(!pMyData->resultFileOpen()){
		setError(&p->result,NO_FILE_OPEN);
		return 0;
	}
	pMyData->closeSession();

	setError(&p->result,SUCCESS);
	return 0;
}

// variable getBrickletRawData(variable brickletID, string dataWave)
static int getBrickletRawData(getBrickletRawDataParams *p){

	setError(&p->result,UNKNOWN_ERROR);
	
	char dataWaveName[MAX_OBJ_NAME+1], dataFolderName[MAX_OBJ_NAME+1];
	const int *pBuffer;
	int* dataPtr = NULL;
	MyBricklet* myBricklet = NULL;
	int count=0,ret, brickletID;
	DataFolderHandle parentDataFolderHPtr = NULL, newDataFolderHPtr = NULL;

	long dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));
	waveHndl waveHandle;

	if(!pMyData->resultFileOpen()){
		setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	brickletID = (int) p->brickletID;
	const int numberOfBricklets = pSession->getBrickletCount();

	if( numberOfBricklets == 0){
		setError(&p->result,EMPTY_RESULTFILE);
		return 0;
	}

	if( !isValidBrickletID(brickletID,numberOfBricklets) ){
		setError(&p->result,NON_EXISTENT_BRICKLET,anyTypeToString<int>(brickletID));
		return 0;
	}

	if( GetHandleSize(p->waveName) == 0L ){
		sprintf(dataWaveName,rawBrickletFormatString,brickletID);
	}
	else
	{
		ret = GetCStringFromHandle(p->waveName,dataWaveName,MAX_OBJ_NAME);
		if(ret != 0){
			setInternalError(&p->result,ret);
			return 0;
		}
	}

	myBricklet = pMyData->getMyBrickletObject(brickletID);
	ASSERT_RETURN_ZERO(myBricklet);

	myBricklet->getBrickletContentsBuffer(&pBuffer,count);

	if(count == 0 || pBuffer == NULL){
		outputToHistory("Could not load bricklet contents.");
		setError(&p->result,UNKNOWN_ERROR);
		return 0;
	}
	 // create 1D wave with count points
	dimensionSizes[ROWS]=count;

	if( pMyData->datafolderEnabled() ){

		ret = GetCurrentDataFolder(&parentDataFolderHPtr);

		if(ret != 0){
			setInternalError(&p->result,ret);
				return 0;
		}
		sprintf(dataFolderName,dataFolderFormat,brickletID);
		ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);

		// continue if the datafolder alrady exists, abort on all other errors
		if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
			setInternalError(&p->result,ret);
				return 0;
		}
	}

	ret = MDMakeWave(&waveHandle,dataWaveName,newDataFolderHPtr,dimensionSizes,NT_I32,pMyData->overwriteEnabledAsInt());

	if(ret == NAME_WAV_CONFLICT){
		sprintf(pMyData->outputBuffer,"Wave %s already exists.",dataWaveName);
		debugOutputToHistory(pMyData->outputBuffer);
		setError(&p->result,WAVE_EXIST,dataWaveName);
		return 0;	
	}

	if(ret != 0 ){
		setInternalError(&p->result,ret);
		return 0;
	}

	dataPtr = (int*) WaveData(waveHandle);
	// copy data fast :)
	memcpy(dataPtr,pBuffer,count*sizeof(int));

	setDataWaveNote(brickletID,myBricklet->getRawMin(),myBricklet->getRawMax(),myBricklet->getPhysValRawMin(),myBricklet->getPhysValRawMax(),waveHandle);

	setError(&p->result,SUCCESS);
	return 0;
}

// variable getNumberOfBricklets(variable *totalNumberOfBricklets)
static int getNumberOfBricklets(getNumberOfBrickletsParams *p){

	setError(&p->result,UNKNOWN_ERROR);

	if(!pMyData->resultFileOpen()){
		setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	*p->totalNumberOfBricklets = pSession->getBrickletCount();

	setError(&p->result,SUCCESS);
	return 0;
}

// variable getResultFile(string *fileName,string *dirPath)
static int getResultFile(getResultFileParams *p){

	setError(&p->result,UNKNOWN_ERROR);

	if(!pMyData->resultFileOpen()){
		setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	//fileName
	if(*p->fileName == NULL){
		*p->fileName = NewHandle(0L);
		if(MemError() || *p->fileName == NULL){
			return NOMEM;
		}
	}

	int ret = PutCStringInHandle(pMyData->getFileName().c_str(), *p->fileName);
	if( ret != 0 ){
		setInternalError(&p->result,ret);
		return 0;
	}

	//dirPath
	if(*p->dirPath == NULL){
		*p->dirPath = NewHandle(0L);
		if(MemError() || *p->dirPath == NULL){
			return NOMEM;
		}
	}

	ret = PutCStringInHandle(pMyData->getDirPath().c_str(), *p->dirPath);
	if( ret != 0 ){
		setInternalError(&p->result,ret);
		return 0;
	}

	setError(&p->result,SUCCESS);
	return 0;
}

// variable getVernissageVersion(double *vernissageVersion)
static int getVernissageVersion(getVernissageVersionParams *p){

	setError(&p->result,UNKNOWN_ERROR);

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	*p->vernissageVersion = stringToAnyType<double>(pMyData->getVernissageVersion());

	setError(&p->result,SUCCESS);
	return 0;
}

// variable getXOPVersion(double *xopVersion)
static int getXOPVersion(getXOPVersionParams *p){

	setError(&p->result,UNKNOWN_ERROR);

	*p->xopVersion = stringToAnyType<double>(myXopVersion);

	setError(&p->result,SUCCESS);
	return 0;
}

// variable openResultFile(string absoluteFilePath, string fileName)
static int openResultFile(openResultFileParams *p){

	setError(&p->result,UNKNOWN_ERROR);

	char fullPath[MAX_PATH_LEN+1], fileName[MAX_PATH_LEN+1], dirPath[MAX_PATH_LEN+1];
	int ret = 0,i, offset=0,count=0, maxCount=100;
	void* pContext  = NULL, *pBricklet = NULL;
	
	if(pMyData->resultFileOpen()){
		setError(&p->result,ALREADY_FILE_OPEN,pMyData->getFileName());
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	if( GetHandleSize(p->absoluteFilePath) == 0L){
		setError(&p->result,WRONG_PARAMETER,"absoluteFilePath");
		return 0;
	}

	if( GetHandleSize(p->fileName) != 0L){
		ret = GetCStringFromHandle(p->fileName,fileName,MAX_PATH_LEN);
		if( ret != 0 ){
			setInternalError(&p->result,ret);
				return 0;
		}

		ret = GetCStringFromHandle(p->absoluteFilePath,dirPath,MAX_PATH_LEN);
		if( ret != 0 ){
			setInternalError(&p->result,ret);
		}

		ret = MacToWinPath(dirPath);
		if( ret != 0){
				setInternalError(&p->result,ret);
				return 0;
		}
		ConcatenatePaths(dirPath,fileName,fullPath);
	}
	// empty filename, so p->absoluteFilePath has the path including the filename
	else{

		ret = GetCStringFromHandle(p->absoluteFilePath,fullPath,MAX_PATH_LEN);
		if( ret != 0 ){
				setInternalError(&p->result,ret);
				return 0;
		}

		ret = MacToWinPath(fullPath);
		if( ret != 0){
				setInternalError(&p->result,ret);
				return 0;
		}

		ret = GetDirectoryAndFileNameFromFullPath(fullPath,dirPath,fileName);
		if( ret != 0){
				setInternalError(&p->result,ret);
				return 0;
		}
	}

	// from here on we have
	// filename : myName.test
	// dirPath c:\data
	// fullPath c:\data\myName.test

	if( !FullPathPointsToFolder(dirPath) ){
		setError(&p->result,FILE_NOT_READABLE,dirPath);
		return 0;	
	}

	if( !FullPathPointsToFile(fullPath)){
		setError(&p->result,FILE_NOT_READABLE,fullPath);
		return 0;	
	}

	// remove suffix \\ in the dirPath because loadResultset does not like that
	if(dirPath[strlen(dirPath)-1] == '\\'){
		dirPath[strlen(dirPath)-1] = '\0';
	}

	sprintf(pMyData->outputBuffer,"filename %s",fileName);
	debugOutputToHistory(pMyData->outputBuffer);

	sprintf(pMyData->outputBuffer,"dirPath %s",dirPath);
	debugOutputToHistory(pMyData->outputBuffer);

	// true -> result set will be added to the database
	// false -> replaces the current results sets in the internal databse 
	bool loadSuccess = pSession->loadResultSet(CharPtrToWString(dirPath),CharPtrToWString(fileName),false);

	if(!loadSuccess){
		outputToHistory("Could not load the result file");
		setError(&p->result,UNKNOWN_ERROR);
		return 0;
	}

	//starting from here the result file is valid
	pMyData->setResultFile(dirPath,fileName);

	for(i=1; i <= pSession->getBrickletCount(); i++ ){
		pBricklet = pSession->getNextBricklet(&pContext);
		ASSERT_RETURN_ZERO(pBricklet);
		pMyData->createMyBrickletObject(i,pBricklet);
	}

	setError(&p->result,SUCCESS);
	return 0;
}

// variable checkForNewBricklets(variable *startBrickletID,variable *endBrickletID)
static int checkForNewBricklets(checkForNewBrickletsParams *p){

	setError(&p->result,UNKNOWN_ERROR);

	// save defaults
	*p->endBrickletID   = -1;
	*p->startBrickletID = -1;

	if(!pMyData->resultFileOpen()){
		setError(&p->result,NO_FILE_OPEN);
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
		setError(&p->result,UNKNOWN_ERROR);
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
		setError(&p->result,NO_NEW_BRICKLETS);
		return 0;
	}
	// from here on we know that numberOfBricklets > oldNumberOfBricklets 

	// happened once so be prepared
	if(numberOfBricklets < oldNumberOfBricklets){
		setError(&p->result,UNKNOWN_ERROR);
		return 0;
	}

	*p->endBrickletID   = numberOfBricklets;
	*p->startBrickletID = oldNumberOfBricklets+1;

	setError(&p->result,SUCCESS);
	return 0;
}

// variable getAllBrickletData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletData(getAllBrickletDataParams *p){

	if(!pMyData->resultFileOpen()){
		setError(&p->result,NO_FILE_OPEN);
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
		setError(&p->result,NO_FILE_OPEN);
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
	
	int brickletID=-1, ret=-1;

	setError(&p->result,UNKNOWN_ERROR);

	if(!pMyData->resultFileOpen()){
		setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		setError(&p->result,EMPTY_RESULTFILE);
		return 0;
	}

	if(	!isValidBrickletRange(p->startBrickletID,p->endBrickletID,numberOfBricklets) ){
			setError(&p->result,INVALID_RANGE);
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
			setInternalError(&p->result,ret);
			return 0;
		}
	}

	// now we got a valid baseName
	for(brickletID=int(p->startBrickletID); brickletID <= int(p->endBrickletID); brickletID++){

		myBricklet = pMyData->getMyBrickletObject(brickletID);
		ASSERT_RETURN_ZERO(myBricklet);

		sprintf(dataName,brickletDataFormat,dataBaseName,brickletID);

		if( pMyData->datafolderEnabled() ){

			ret = GetCurrentDataFolder(&parentDataFolderHPtr);
			if(ret != 0){
				setInternalError(&p->result,ret);
				return 0;
			}

			sprintf(dataFolderName,dataFolderFormat,brickletID);
			ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);
	
			// continue if the datafolder alrady exists, abort on all other errors
			if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
				setInternalError(&p->result,ret);
				return 0;
			}
		}

		ret = createAndFillDataWave(newDataFolderHPtr,dataName,brickletID);
		if(ret == WAVE_EXIST){
			setError(&p->result,ret,dataName);
			return 0;
		}
		else if(ret == INTERNAL_ERROR_CONVERTING_DATA){
			setError(&p->result,ret);
			return 0;
		}
		else if(ret != SUCCESS){
			setInternalError(&p->result,ret);
			return 0;
		}
	}

	setError(&p->result,SUCCESS);
	return 0;
}

// variable getRangeBrickletMetaData(string baseName,variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletMetaData(getRangeBrickletMetaDataParams *p){

	MyBricklet* myBricklet = NULL;
	std::vector<std::string> keys,values;
	char metaDataBaseName[MAX_OBJ_NAME+1], dataFolderName[MAX_OBJ_NAME+1], metaDataName[MAX_OBJ_NAME+1];
	DataFolderHandle parentDataFolderHPtr = NULL, newDataFolderHPtr = NULL;
	
	int brickletID=-1, ret=-1;

	setError(&p->result,UNKNOWN_ERROR);

	if(!pMyData->resultFileOpen()){
		setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		setError(&p->result,EMPTY_RESULTFILE);
		return 0;
	}

	if( !isValidBrickletRange(p->startBrickletID,p->endBrickletID,numberOfBricklets) ){
		setError(&p->result,INVALID_RANGE);
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
			setInternalError(&p->result,ret);
			return 0;
		}
	}
	// now we got a valid baseName

	for(brickletID=int(p->startBrickletID); brickletID <= int(p->endBrickletID); brickletID++){


		myBricklet = pMyData->getMyBrickletObject(brickletID);
		ASSERT_RETURN_ZERO(myBricklet);

		sprintf(metaDataName,brickletMetaDataFormat,metaDataBaseName,brickletID);

		if( pMyData->datafolderEnabled() ){

			ret = GetCurrentDataFolder(&parentDataFolderHPtr);

			if(ret != 0){
				setInternalError(&p->result,ret);
				return 0;
			}
			sprintf(dataFolderName,dataFolderFormat,brickletID);
			ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);
	
			// continue if the datafolder alrady exists, abort on all other errors
			if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
				setInternalError(&p->result,ret);
				return 0;
			}
		}
		myBricklet->getBrickletMetaData(keys,values);

		ret = createAndFillTextWave(keys,values,newDataFolderHPtr,metaDataName,brickletID);

		if(ret == WAVE_EXIST){
			setError(&p->result,ret,metaDataName);
			return 0;
		}
		else if(ret != SUCCESS){
			setInternalError(&p->result,ret);
			return 0;
		}
	}
	setError(&p->result,SUCCESS);
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
	if(MemError() || p->result == NULL){
		return NOMEM;
	}

	int ret = PutCStringInHandle(str.c_str(),p->result);
	if(ret != 0){
		sprintf(pMyData->outputBuffer,"internal error %d, aborting",ret);
		outputToHistory(pMyData->outputBuffer);
		return 0;
	}

	outputToHistory(str.c_str());
	return 0;
}

// variable getResultFileMetaData(string waveName)
static int getResultFileMetaData(getResultFileMetaDataParams *p){

	setError(&p->result,UNKNOWN_ERROR);

	char metaDataWaveName[MAX_OBJ_NAME+1];
	std::vector<std::string> keys,values;
	char buf[ARRAY_SIZE];
	int ret;
	void *pBricklet = NULL;
	tm ctime;
	Vernissage::Session::BrickletMetaData brickletMetaData;

	if(!pMyData->resultFileOpen()){
		setError(&p->result,NO_FILE_OPEN);
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
			setInternalError(&p->result,ret);
			return 0;
		}
	}

	const int numberOfBricklets = pSession->getBrickletCount();

	if(numberOfBricklets == 0){
		setError(&p->result,EMPTY_RESULTFILE);
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
		setError(&p->result,ret,metaDataWaveName);
		return 0;
	}

	if(ret != 0){
		setInternalError(&p->result,ret);
		return 0;
	}

	setError(&p->result,SUCCESS);
	return 0;
}


// variable createOverViewTable(string waveName, string keyList)
static int createOverViewTable(createOverViewTableParams *p){

	setError(&p->result,UNKNOWN_ERROR);
	char keyListChar[ARRAY_SIZE+1];
	int ret=-1,count=0, countMax=1000;
	std::string keyList, key, value;
	waveHndl waveHandle;
	long dimensionSizes[MAX_DIMENSIONS+1];

	MemClear(dimensionSizes, sizeof(dimensionSizes));

	std::vector<std::string> keys, textWaveContents;
	char waveName[MAX_OBJ_NAME+1];
	MyBricklet *myBricklet=NULL;
	unsigned int i, j;

	if(!pMyData->resultFileOpen()){
		setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const unsigned int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		setError(&p->result,EMPTY_RESULTFILE);
		return 0;
	}

	// check keyList parameter
	if( p->keyList == NULL || GetHandleSize(p->keyList) == 0L ){
		setError(&p->result,WRONG_PARAMETER,"keyList");
		return 0;
	}
	else{
		ret = GetCStringFromHandle(p->keyList,keyListChar,ARRAY_SIZE);
		if(ret != 0){
			setInternalError(&p->result,ret);
			return 0;
		}
	}

	char sepChar[] = ";";
	splitString(keyListChar,sepChar,keys);

	if( keys.size() == 0 ){
		setError(&p->result,WRONG_PARAMETER,"keyList");
		return 0;
	}

	// check waveName parameter
	if( GetHandleSize(p->waveName) == 0L ){
		sprintf(waveName,overViewTableDefault);
	}
	else{
		ret = GetCStringFromHandle(p->waveName,waveName,MAX_OBJ_NAME);
		if(ret != 0){
			setInternalError(&p->result,ret);
			return 0;
		}
	}

	dimensionSizes[ROWS] = numberOfBricklets;
	dimensionSizes[COLUMNS] = keys.size();
	ret = MDMakeWave(&waveHandle,waveName,NULL,dimensionSizes,TEXT_WAVE_TYPE,pMyData->overwriteEnabledAsInt());

	if(ret == NAME_WAV_CONFLICT){
		sprintf(pMyData->outputBuffer,"Wave %s already exists.",waveName);
		debugOutputToHistory(pMyData->outputBuffer);
		setError(&p->result,WAVE_EXIST,waveName);
		return 0;
	}

	if(ret != 0 ){
		setInternalError(&p->result,ret);
		return 0;
	}

	ASSERT_RETURN_MINUSONE(waveHandle);

	for(j=0; j < keys.size(); j++){

		key = keys.at(j);
		MDSetDimensionLabel(waveHandle,COLUMNS,j,const_cast<char *> (key.c_str()));
		sprintf(pMyData->outputBuffer,"key=%s",key.c_str());
		debugOutputToHistory(pMyData->outputBuffer);

		for(i=1; i <= numberOfBricklets; i++){
			myBricklet = pMyData->getMyBrickletObject(i);
			value = myBricklet->getMetaDataValueAsString(key);
			textWaveContents.push_back(value);

			sprintf(pMyData->outputBuffer,"   value=%s",value.c_str());
			debugOutputToHistory(pMyData->outputBuffer);
		}
	}

	ret = stringVectorToTextWave(textWaveContents,waveHandle);

	if(ret != 0){
		setInternalError(&p->result,ret);
		return 0;
	}

	// brickletID equals 0 because the wave note is for a resultfile kind wave
	setOtherWaveNote(0,waveHandle);

	setError(&p->result,SUCCESS);
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
	if(MemError() || p->result == NULL){
		return NOMEM;
	}
	
	int ret = PutCStringInHandle(lastErrorMsg.c_str(),p->result);
	if(ret != 0){
		sprintf(pMyData->outputBuffer,"internal error %d, aborting",ret);
		outputToHistory(pMyData->outputBuffer);
		return 0;
	}
	return 0;
}

// string readXOPSettings();
static int readXOPSettings(readXOPSettingsParams *p){

	
	std::string config;
	std::stringstream stream;

	stream << debug_option_name << "=" << pMyData->debuggingEnabled() << ";";
	stream << folder_option_name << "=" << pMyData->datafolderEnabled() << ";";
	stream << overwrite_option_name << "=" << pMyData->overwriteEnabled() << ";";
	stream << double_option_name << "=" << pMyData->doubleWaveEnabled();

	config = stream.str();

	p->result = NewHandle(config.size());
	if(MemError() || p->result == NULL){
			return NOMEM;
	}

	int ret = PutCStringInHandle(config.c_str(), p->result);
	if(ret != 0){
		sprintf(pMyData->outputBuffer,"internal error %d, aborting",ret);
		outputToHistory(pMyData->outputBuffer);
		return 0;
	}
	return 0;
}

// variable writeXOPSettings(string config)
static int writeXOPSettings(writeXOPSettingsParams *p){

	setError(&p->result,UNKNOWN_ERROR);

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
		setError(&p->result,WRONG_PARAMETER,"config");
		return 0;
	}

	ret = GetCStringFromHandle(p->config,configChar,ARRAY_SIZE);
	if(ret != 0){
		setInternalError(&p->result,ret);
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
			setError(&p->result,WRONG_PARAMETER,"config");
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
			setError(&p->result,WRONG_PARAMETER,"config");
			return 0;
		}
		
		// check if the value is either 1 or 0
		if(value != 0 && value != 1){
			setError(&p->result,WRONG_PARAMETER,"config");
			return 0;
		}

		// now we got a valid key/value pair
		pMyData->setOption(key, value);
	}

	setError(&p->result,SUCCESS);
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


HOST_IMPORT int
main(IORecHandle ioRecHandle)
{	
	XOPInit(ioRecHandle);							/* do standard XOP initialization */
	SetXOPEntry(XOPEntry);							/* set entry point for future calls */

	if (igorVersion < 620){
		SetXOPResult(REQUIRES_IGOR_620);
		return EXIT_FAILURE;
	}

	try{
		pMyData = new myData();
	}
	catch(CException *e){
		SetXOPResult(OUT_OF_MEMORY);
		e->Delete();
		return EXIT_FAILURE;
	}

	SetXOPResult(0L);

	return EXIT_SUCCESS;
}

void doCleanup(){

	// in case the user has forgotten to close the session
	closeResultFileParams p;
	closeResultFile(&p);
}

bool isValidBrickletRange(double startID, double endID,int numberOfBricklets){

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

void setInternalError(double *result, int errorValue){
	
	*result = errorValue;
	pMyData->setLastError(errorValue);

	sprintf(pMyData->outputBuffer,"BUG: xop internal error %d returned.",errorValue);
	outputToHistory(pMyData->outputBuffer);
}

void setError(double *result, int errorValue, std::string msgArgument){

	*result = errorValue;
	pMyData->setLastError(errorValue,msgArgument);

}