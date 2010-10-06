/*	TODO author affiliation, license

*/
#include "header.h"

#include "xopinterface.h"
#include "xopinterfacestandard.h"

#include <algorithm>
#include <string>
#include <math.h>

#include "utils.h"
#include "brickletconverter.h"
#include "globaldata.h"

// variable closeResultFile()
static int closeResultFile(closeResultFileParams *p){

	globDataPtr->readSettings();
	globDataPtr->setError(&p->result,UNKNOWN_ERROR);

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(&p->result,NO_FILE_OPEN);
		return 0;
	}
	globDataPtr->closeResultFile();

	globDataPtr->setError(&p->result,SUCCESS);
	return 0;
}

// variable getBrickletRawData(variable brickletID, string dataWave)
static int getBrickletRawData(getBrickletRawDataParams *p){

	globDataPtr->readSettings();
	globDataPtr->setError(&p->result,UNKNOWN_ERROR);
	
	char dataWaveName[MAX_OBJ_NAME+1], dataFolderName[MAX_OBJ_NAME+1];
	const int *pBuffer;
	int* dataPtr = NULL;
	BrickletClass* BrickletClass = NULL;
	int count=0,ret, brickletID;
	DataFolderHandle parentDataFolderHPtr = NULL, newDataFolderHPtr = NULL;

	long dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));
	waveHndl waveHandle;

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	brickletID = (int) p->brickletID;
	const int numberOfBricklets = pSession->getBrickletCount();

	if( numberOfBricklets == 0){
		globDataPtr->setError(&p->result,EMPTY_RESULTFILE);
		return 0;
	}

	if( !isValidBrickletID(brickletID,numberOfBricklets) ){
		globDataPtr->setError(&p->result,NON_EXISTENT_BRICKLET,anyTypeToString<int>(brickletID));
		return 0;
	}

	if( GetHandleSize(p->waveName) == 0L ){
		sprintf(dataWaveName,rawBrickletFormatString,brickletID);
	}
	else
	{
		ret = GetCStringFromHandle(p->waveName,dataWaveName,MAX_OBJ_NAME);
		if(ret != 0){
			globDataPtr->setInternalError(&p->result,ret);
			return 0;
		}
	}

	BrickletClass = globDataPtr->getBrickletClassObject(brickletID);
	ASSERT_RETURN_ZERO(BrickletClass);

	BrickletClass->getBrickletContentsBuffer(&pBuffer,count);

	if(count == 0 || pBuffer == NULL){
		outputToHistory("Could not load bricklet contents.");
		globDataPtr->setError(&p->result,UNKNOWN_ERROR);
		return 0;
	}
	 // create 1D wave with count points
	dimensionSizes[ROWS]=count;

	if( globDataPtr->datafolderEnabled() ){

		ret = GetCurrentDataFolder(&parentDataFolderHPtr);

		if(ret != 0){
			globDataPtr->setInternalError(&p->result,ret);
				return 0;
		}
		sprintf(dataFolderName,dataFolderFormat,brickletID);
		ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);

		// continue if the datafolder alrady exists, abort on all other errors
		if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
			globDataPtr->setInternalError(&p->result,ret);
				return 0;
		}
	}

	ret = MDMakeWave(&waveHandle,dataWaveName,newDataFolderHPtr,dimensionSizes,NT_I32,globDataPtr->overwriteEnabledAsInt());

	if(ret == NAME_WAV_CONFLICT){
		sprintf(globDataPtr->outputBuffer,"Wave %s already exists.",dataWaveName);
		debugOutputToHistory(globDataPtr->outputBuffer);
		globDataPtr->setError(&p->result,WAVE_EXIST,dataWaveName);
		return 0;	
	}

	if(ret != 0 ){
		globDataPtr->setInternalError(&p->result,ret);
		return 0;
	}

	dataPtr = (int*) WaveData(waveHandle);
	// copy data fast :)
	memcpy(dataPtr,pBuffer,count*sizeof(int));

	setDataWaveNote(brickletID,BrickletClass->getRawMin(),BrickletClass->getRawMax(),BrickletClass->getPhysValRawMin(),BrickletClass->getPhysValRawMax(),waveHandle);

	globDataPtr->setError(&p->result,SUCCESS);
	return 0;
}

// variable getNumberOfBricklets(variable *totalNumberOfBricklets)
static int getNumberOfBricklets(getNumberOfBrickletsParams *p){

	globDataPtr->readSettings();
	globDataPtr->setError(&p->result,UNKNOWN_ERROR);

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	*p->totalNumberOfBricklets = pSession->getBrickletCount();

	globDataPtr->setError(&p->result,SUCCESS);
	return 0;
}

// variable getResultFile(string *fileName,string *dirPath)
static int getResultFile(getResultFileParams *p){

	globDataPtr->readSettings();
	globDataPtr->setError(&p->result,UNKNOWN_ERROR);

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	//fileName
	if(*p->fileName == NULL){
		*p->fileName = NewHandle(0L);
		if(MemError() || *p->fileName == NULL){
			return NOMEM;
		}
	}

	int ret = PutCStringInHandle(globDataPtr->getFileName().c_str(), *p->fileName);
	if( ret != 0 ){
		globDataPtr->setInternalError(&p->result,ret);
		return 0;
	}

	//dirPath
	if(*p->dirPath == NULL){
		*p->dirPath = NewHandle(0L);
		if(MemError() || *p->dirPath == NULL){
			return NOMEM;
		}
	}

	ret = PutCStringInHandle(globDataPtr->getDirPath().c_str(), *p->dirPath);
	if( ret != 0 ){
		globDataPtr->setInternalError(&p->result,ret);
		return 0;
	}

	globDataPtr->setError(&p->result,SUCCESS);
	return 0;
}

// variable getVernissageVersion(double *vernissageVersion)
static int getVernissageVersion(getVernissageVersionParams *p){

	globDataPtr->readSettings();
	globDataPtr->setError(&p->result,UNKNOWN_ERROR);

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	*p->vernissageVersion = stringToAnyType<double>(globDataPtr->getVernissageVersion());

	globDataPtr->setError(&p->result,SUCCESS);
	return 0;
}

// variable getXOPVersion(double *xopVersion)
static int getXOPVersion(getXOPVersionParams *p){

	globDataPtr->readSettings();
	globDataPtr->setError(&p->result,UNKNOWN_ERROR);

	*p->xopVersion = stringToAnyType<double>(myXopVersion);

	globDataPtr->setError(&p->result,SUCCESS);
	return 0;
}

// variable openResultFile(string absoluteFilePath, string fileName)
static int openResultFile(openResultFileParams *p){

	globDataPtr->readSettings();
	globDataPtr->setError(&p->result,UNKNOWN_ERROR);

	char fullPath[MAX_PATH_LEN+1], fileName[MAX_PATH_LEN+1], dirPath[MAX_PATH_LEN+1];
	int ret = 0,i, offset=0,count=0, maxCount=100;
	void* pContext  = NULL, *pBricklet = NULL;
	
	if(globDataPtr->resultFileOpen()){
		globDataPtr->setError(&p->result,ALREADY_FILE_OPEN,globDataPtr->getFileName());
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	if( GetHandleSize(p->absoluteFilePath) == 0L){
		globDataPtr->setError(&p->result,WRONG_PARAMETER,"absoluteFilePath");
		return 0;
	}

	if( GetHandleSize(p->fileName) != 0L){
		ret = GetCStringFromHandle(p->fileName,fileName,MAX_PATH_LEN);
		if( ret != 0 ){
			globDataPtr->setInternalError(&p->result,ret);
				return 0;
		}

		ret = GetCStringFromHandle(p->absoluteFilePath,dirPath,MAX_PATH_LEN);
		if( ret != 0 ){
			globDataPtr->setInternalError(&p->result,ret);
		}

		ret = MacToWinPath(dirPath);
		if( ret != 0){
				globDataPtr->setInternalError(&p->result,ret);
				return 0;
		}
		ConcatenatePaths(dirPath,fileName,fullPath);
	}
	// empty filename, so p->absoluteFilePath has the path including the filename
	else{

		ret = GetCStringFromHandle(p->absoluteFilePath,fullPath,MAX_PATH_LEN);
		if( ret != 0 ){
				globDataPtr->setInternalError(&p->result,ret);
				return 0;
		}

		ret = MacToWinPath(fullPath);
		if( ret != 0){
				globDataPtr->setInternalError(&p->result,ret);
				return 0;
		}

		ret = GetDirectoryAndFileNameFromFullPath(fullPath,dirPath,fileName);
		if( ret != 0){
				globDataPtr->setInternalError(&p->result,ret);
				return 0;
		}
	}

	// from here on we have
	// filename : myName.test
	// dirPath c:\data
	// fullPath c:\data\myName.test

	if( !FullPathPointsToFolder(dirPath) ){
		globDataPtr->setError(&p->result,FILE_NOT_READABLE,dirPath);
		return 0;	
	}

	if( !FullPathPointsToFile(fullPath)){
		globDataPtr->setError(&p->result,FILE_NOT_READABLE,fullPath);
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

	// true -> result set will be added to the database
	// false -> replaces the current results sets in the internal databse 
	bool loadSuccess = pSession->loadResultSet(CharPtrToWString(dirPath),CharPtrToWString(fileName),false);

	if(!loadSuccess){
		outputToHistory("Could not load the result file");
		globDataPtr->setError(&p->result,UNKNOWN_ERROR);
		return 0;
	}

	//starting from here the result file is valid
	globDataPtr->setResultFile(dirPath,fileName);

	for(i=1; i <= pSession->getBrickletCount(); i++ ){
		pBricklet = pSession->getNextBricklet(&pContext);
		ASSERT_RETURN_ZERO(pBricklet);
		globDataPtr->createBrickletClassObject(i,pBricklet);
	}

	globDataPtr->setError(&p->result,SUCCESS);
	return 0;
}

// variable checkForNewBricklets(variable *startBrickletID,variable *endBrickletID)
static int checkForNewBricklets(checkForNewBrickletsParams *p){

	globDataPtr->readSettings();
	globDataPtr->setError(&p->result,UNKNOWN_ERROR);

	// save defaults
	*p->endBrickletID   = -1;
	*p->startBrickletID = -1;

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int oldNumberOfBricklets = pSession->getBrickletCount();
	void* pContext  = NULL, *pBricklet = NULL;
	BrickletClass *BrickletClass = NULL;
	int i;

	std::wstring fileName = StringToWString(globDataPtr->getFileName());
	std::wstring dirPath = StringToWString(globDataPtr->getDirPath());

	// true -> result set will be added to the database
	// false -> replaces the current results sets in the internal databse 
	bool loadSuccess = pSession->loadResultSet(dirPath,fileName,false);

	if(!loadSuccess){
		outputToHistory("Could not check for updates of the result file. Maybe it was moved?");
		outputToHistory("Try closing and opening the result file again.");
		globDataPtr->setError(&p->result,UNKNOWN_ERROR);
		return 0;
	}

	// starting from here we have to
	// - update the pBricklet pointers in the BrickletClass objects
	// - compare old to new totalNumberOfBricklets
	const int numberOfBricklets = pSession->getBrickletCount();

	for(i=1; i <= numberOfBricklets; i++ ){
		pBricklet = pSession->getNextBricklet(&pContext);
		ASSERT_RETURN_ZERO(pBricklet);

		BrickletClass = globDataPtr->getBrickletClassObject(i);

		if(BrickletClass == NULL){// this is a new bricklet
			globDataPtr->createBrickletClassObject(i,pBricklet);
		}else{	// the bricklet is old and we only have to update *pBricklet
			BrickletClass->setBrickletPointer(pBricklet);
		}
	}

	if(oldNumberOfBricklets == numberOfBricklets){
		globDataPtr->setError(&p->result,NO_NEW_BRICKLETS);
		return 0;
	}
	// from here on we know that numberOfBricklets > oldNumberOfBricklets 

	// happened once so be prepared
	if(numberOfBricklets < oldNumberOfBricklets){
		globDataPtr->setError(&p->result,UNKNOWN_ERROR);
		return 0;
	}

	*p->endBrickletID   = numberOfBricklets;
	*p->startBrickletID = oldNumberOfBricklets+1;

	globDataPtr->setError(&p->result,SUCCESS);
	return 0;
}

// variable getAllBrickletData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletData(getAllBrickletDataParams *p){

	globDataPtr->readSettings();

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
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
	
	globDataPtr->readSettings();

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
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
	
	globDataPtr->readSettings();

	getRangeBrickletDataParams rangeParams;
	rangeParams.baseName = p->baseName;
	rangeParams.startBrickletID = p->brickletID;
	rangeParams.endBrickletID = p->brickletID;

	getRangeBrickletData(&rangeParams);
	return 0;

}

// variable getBrickletMetaData(string metaData, variable brickletID)
static int getBrickletMetaData(getBrickletMetaDataParams *p){

	globDataPtr->readSettings();

	getRangeBrickletMetaDataParams rangeParams;
	rangeParams.baseName = p->baseName;
	rangeParams.startBrickletID = p->brickletID;
	rangeParams.endBrickletID = p->brickletID;

	getRangeBrickletMetaData(&rangeParams);
	return 0;
}

// variable getRangeBrickletData(string baseName, variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletData(getRangeBrickletDataParams *p){

	globDataPtr->readSettings();

	BrickletClass* BrickletClass = NULL;
	std::vector<std::string> keys,values;
	char dataBaseName[MAX_OBJ_NAME+1], dataFolderName[MAX_OBJ_NAME+1], dataName[MAX_OBJ_NAME+1];
	DataFolderHandle parentDataFolderHPtr = NULL, newDataFolderHPtr = NULL;
	
	int brickletID=-1, ret=-1;

	globDataPtr->setError(&p->result,UNKNOWN_ERROR);

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		globDataPtr->setError(&p->result,EMPTY_RESULTFILE);
		return 0;
	}

	if(	!isValidBrickletRange(p->startBrickletID,p->endBrickletID,numberOfBricklets) ){
			globDataPtr->setError(&p->result,INVALID_RANGE);
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
			globDataPtr->setInternalError(&p->result,ret);
			return 0;
		}
	}

	// now we got a valid baseName
	for(brickletID=int(p->startBrickletID); brickletID <= int(p->endBrickletID); brickletID++){

		BrickletClass = globDataPtr->getBrickletClassObject(brickletID);
		ASSERT_RETURN_ZERO(BrickletClass);

		sprintf(dataName,brickletDataFormat,dataBaseName,brickletID);

		if( globDataPtr->datafolderEnabled() ){

			ret = GetCurrentDataFolder(&parentDataFolderHPtr);
			if(ret != 0){
				globDataPtr->setInternalError(&p->result,ret);
				return 0;
			}

			sprintf(dataFolderName,dataFolderFormat,brickletID);
			ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);
	
			// continue if the datafolder alrady exists, abort on all other errors
			if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
				globDataPtr->setInternalError(&p->result,ret);
				return 0;
			}
		}

		ret = createWaves(newDataFolderHPtr,dataName,brickletID);
		if(ret == WAVE_EXIST){
			globDataPtr->setError(&p->result,ret,dataName);
			return 0;
		}
		else if(ret == INTERNAL_ERROR_CONVERTING_DATA){
			globDataPtr->setError(&p->result,ret);
			return 0;
		}
		else if(ret != SUCCESS){
			globDataPtr->setInternalError(&p->result,ret);
			return 0;
		}
	}

	globDataPtr->setError(&p->result,SUCCESS);
	return 0;
}

// variable getRangeBrickletMetaData(string baseName,variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletMetaData(getRangeBrickletMetaDataParams *p){

	globDataPtr->readSettings();

	BrickletClass* BrickletClass = NULL;
	std::vector<std::string> keys,values;
	char metaDataBaseName[MAX_OBJ_NAME+1], dataFolderName[MAX_OBJ_NAME+1], metaDataName[MAX_OBJ_NAME+1];
	DataFolderHandle parentDataFolderHPtr = NULL, newDataFolderHPtr = NULL;
	
	int brickletID=-1, ret=-1;

	globDataPtr->setError(&p->result,UNKNOWN_ERROR);

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		globDataPtr->setError(&p->result,EMPTY_RESULTFILE);
		return 0;
	}

	if( !isValidBrickletRange(p->startBrickletID,p->endBrickletID,numberOfBricklets) ){
		globDataPtr->setError(&p->result,INVALID_RANGE);
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
			globDataPtr->setInternalError(&p->result,ret);
			return 0;
		}
	}
	// now we got a valid baseName

	for(brickletID=int(p->startBrickletID); brickletID <= int(p->endBrickletID); brickletID++){


		BrickletClass = globDataPtr->getBrickletClassObject(brickletID);
		ASSERT_RETURN_ZERO(BrickletClass);

		sprintf(metaDataName,brickletMetaDataFormat,metaDataBaseName,brickletID);

		if( globDataPtr->datafolderEnabled() ){

			ret = GetCurrentDataFolder(&parentDataFolderHPtr);

			if(ret != 0){
				globDataPtr->setInternalError(&p->result,ret);
				return 0;
			}
			sprintf(dataFolderName,dataFolderFormat,brickletID);
			ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);
	
			// continue if the datafolder alrady exists, abort on all other errors
			if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
				globDataPtr->setInternalError(&p->result,ret);
				return 0;
			}
		}
		BrickletClass->getBrickletMetaData(keys,values);

		ret = createAndFillTextWave(keys,values,newDataFolderHPtr,metaDataName,brickletID);

		if(ret == WAVE_EXIST){
			globDataPtr->setError(&p->result,ret,metaDataName);
			return 0;
		}
		else if(ret != SUCCESS){
			globDataPtr->setInternalError(&p->result,ret);
			return 0;
		}
	}
	globDataPtr->setError(&p->result,SUCCESS);
	return 0;
}

// string getBugReportTemplate();
static int getBugReportTemplate(getBugReportTemplateParams *p){
	
	globDataPtr->readSettings();

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
	str.append("Vernissage version: " + globDataPtr->getVernissageVersion() + "\r");
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
		sprintf(globDataPtr->outputBuffer,"internal error %d, aborting",ret);
		outputToHistory(globDataPtr->outputBuffer);
		return 0;
	}

	outputToHistory(str.c_str());
	return 0;
}

// variable getResultFileMetaData(string waveName)
static int getResultFileMetaData(getResultFileMetaDataParams *p){

	globDataPtr->readSettings();
	globDataPtr->setError(&p->result,UNKNOWN_ERROR);

	char metaDataWaveName[MAX_OBJ_NAME+1];
	std::vector<std::string> keys,values;
	char buf[ARRAY_SIZE];
	int ret;
	void *pBricklet = NULL;
	tm ctime;
	Vernissage::Session::BrickletMetaData brickletMetaData;

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	if( GetHandleSize(p->waveName) == 0L ){
		sprintf(metaDataWaveName,resultMetaDefault);
	}
	else{
		ret = GetCStringFromHandle(p->waveName,metaDataWaveName,MAX_OBJ_NAME);
		if(ret != 0){
			globDataPtr->setInternalError(&p->result,ret);
			return 0;
		}
	}

	const int numberOfBricklets = pSession->getBrickletCount();

	if(numberOfBricklets == 0){
		globDataPtr->setError(&p->result,EMPTY_RESULTFILE);
		return 0;
	}

	pBricklet = globDataPtr->getBrickletClassObject(numberOfBricklets)->getBrickletPointer();
	ASSERT_RETURN_ZERO(pBricklet);

	// use the timestamp of the last bricklet as dateOfLastChange
	ctime = pSession->getCreationTimestamp(pBricklet);

	brickletMetaData = pSession->getMetaData(pBricklet);

	keys.push_back("resultFilePath");
	values.push_back(globDataPtr->getDirPath());

	keys.push_back("resultFileName");
	values.push_back(globDataPtr->getFileName());

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
		globDataPtr->setError(&p->result,ret,metaDataWaveName);
		return 0;
	}

	if(ret != 0){
		globDataPtr->setInternalError(&p->result,ret);
		return 0;
	}

	globDataPtr->setError(&p->result,SUCCESS);
	return 0;
}


// variable createOverViewTable(string waveName, string keyList)
static int createOverViewTable(createOverViewTableParams *p){

	globDataPtr->readSettings();
	globDataPtr->setError(&p->result,UNKNOWN_ERROR);

	char keyListChar[ARRAY_SIZE+1];
	int ret=-1,count=0, countMax=1000;
	std::string keyList, key, value;
	waveHndl waveHandle;
	long dimensionSizes[MAX_DIMENSIONS+1];

	MemClear(dimensionSizes, sizeof(dimensionSizes));

	std::vector<std::string> keys, textWaveContents;
	char waveName[MAX_OBJ_NAME+1];
	BrickletClass *BrickletClass=NULL;
	unsigned int i, j;

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(&p->result,NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const unsigned int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		globDataPtr->setError(&p->result,EMPTY_RESULTFILE);
		return 0;
	}

	// check keyList parameter
	if( p->keyList == NULL || GetHandleSize(p->keyList) == 0L ){
		globDataPtr->setError(&p->result,WRONG_PARAMETER,"keyList");
		return 0;
	}
	else{
		ret = GetCStringFromHandle(p->keyList,keyListChar,ARRAY_SIZE);
		if(ret != 0){
			globDataPtr->setInternalError(&p->result,ret);
			return 0;
		}
	}

	char sepChar[] = ";";
	splitString(keyListChar,sepChar,keys);

	if( keys.size() == 0 ){
		globDataPtr->setError(&p->result,WRONG_PARAMETER,"keyList");
		return 0;
	}

	// check waveName parameter
	if( GetHandleSize(p->waveName) == 0L ){
		sprintf(waveName,overViewTableDefault);
	}
	else{
		ret = GetCStringFromHandle(p->waveName,waveName,MAX_OBJ_NAME);
		if(ret != 0){
			globDataPtr->setInternalError(&p->result,ret);
			return 0;
		}
	}

	dimensionSizes[ROWS] = numberOfBricklets;
	dimensionSizes[COLUMNS] = keys.size();
	ret = MDMakeWave(&waveHandle,waveName,NULL,dimensionSizes,TEXT_WAVE_TYPE,globDataPtr->overwriteEnabledAsInt());

	if(ret == NAME_WAV_CONFLICT){
		sprintf(globDataPtr->outputBuffer,"Wave %s already exists.",waveName);
		debugOutputToHistory(globDataPtr->outputBuffer);
		globDataPtr->setError(&p->result,WAVE_EXIST,waveName);
		return 0;
	}

	if(ret != 0 ){
		globDataPtr->setInternalError(&p->result,ret);
		return 0;
	}

	ASSERT_RETURN_MINUSONE(waveHandle);

	for(j=0; j < keys.size(); j++){

		key = keys.at(j);
		MDSetDimensionLabel(waveHandle,COLUMNS,j,const_cast<char *> (key.c_str()));
		sprintf(globDataPtr->outputBuffer,"key=%s",key.c_str());
		debugOutputToHistory(globDataPtr->outputBuffer);

		for(i=1; i <= numberOfBricklets; i++){
			BrickletClass = globDataPtr->getBrickletClassObject(i);
			value = BrickletClass->getMetaDataValueAsString(key);
			textWaveContents.push_back(value);

			sprintf(globDataPtr->outputBuffer,"   value=%s",value.c_str());
			debugOutputToHistory(globDataPtr->outputBuffer);
		}
	}

	ret = stringVectorToTextWave(textWaveContents,waveHandle);

	if(ret != 0){
		globDataPtr->setInternalError(&p->result,ret);
		return 0;
	}

	// brickletID equals 0 because the wave note is for a resultfile kind wave
	setOtherWaveNote(0,waveHandle);

	globDataPtr->setError(&p->result,SUCCESS);
	return 0;
}

// variable getLastError()
static int getLastError(getLastErrorParams *p){
	
	globDataPtr->readSettings();

	p->result = globDataPtr->getLastError();
	return 0;
}

// string getLastErrorMessage()
static int getLastErrorMessage(getLastErrorMessageParams *p){
	
	globDataPtr->readSettings();
	std::string lastErrorMsg = globDataPtr->getLastErrorMessage();
	
	p->result = NewHandle(lastErrorMsg.size());
	if(MemError() || p->result == NULL){
		return NOMEM;
	}
	
	int ret = PutCStringInHandle(lastErrorMsg.c_str(),p->result);
	if(ret != 0){
		sprintf(globDataPtr->outputBuffer,"internal error %d, aborting",ret);
		outputToHistory(globDataPtr->outputBuffer);
		return 0;
	}
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
			// in case the user has forgotten to close the result file
			closeResultFileParams p;
			closeResultFile(&p);

			// close the session and unload the DLL
			globDataPtr->closeSession();
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
		globDataPtr = new GlobalData();
	}
	catch(CException *e){
		SetXOPResult(OUT_OF_MEMORY);
		e->Delete();
		return EXIT_FAILURE;
	}

	SetXOPResult(0L);

	return EXIT_SUCCESS;
}
