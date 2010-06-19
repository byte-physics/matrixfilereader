/*	TODO author affiliation, license

*/

#include <string>
#include <set>


#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "matrixFileReader.h"

#include "dataclass.h"
#include "globalvariables.h"
#include "utils.h"

#define DEBUG_LEVEL 1

#include "Vernissage.h"

// variable closeResultFile()
static int closeResultFile(closeResultFileParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	pMyData->closeSession();

	p->result = SUCCESS;
	return 0;
}

// variable getBrickletRawData(variable brickletID, string dataWave)
static int getBrickletRawData(getBrickletRawDataParams *p){

	p->result = UNKNOWN_ERROR;
	int ret;
	char buf[ARRAY_SIZE];
	char dataWaveName[MAX_OBJ_NAME+1];
	const int *pBuffer;
	int count=0;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	int brickletID = (int) p->brickletID;

	if(pSession->getBrickletCount() == 0){
		p->result = EMPTY_RESULTFILE;
		return 0;
	}

	if( brickletID < 1 || brickletID > pSession->getBrickletCount() ){
		p->result = NON_EXISTENT_BRICKLET;
		return 0;
	}

	if( GetHandleSize(p->baseName) == 0L ){
		sprintf(dataWaveName,"brickletRawData_%03d",brickletID);
	}
	else
	{
		ret = GetCStringFromHandle(p->baseName,dataWaveName,MAX_OBJ_NAME);
		if(ret != 0){
			p->result = ret;
			return 0;
		}
	}

	MyBricklet* bricklet = pMyData->getBrickletClassFromMap(brickletID);

	bricklet->getBrickletContentsBuffer(&pBuffer,count);
	ASSERT_RETURN_ZERO(pBuffer);

	if(count == 0){
		outputToHistory("Could not load bricklet contents.");
		p->result = UNKNOWN_ERROR;
		return 0;
	}

	long dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));
	dimensionSizes[ROWS]=count; // create 1D wave with count points
	waveHndl waveHandle;
	int overwrite=0;

	ret = MDMakeWave(&waveHandle,dataWaveName,NULL,dimensionSizes,NT_I32,overwrite);

	if(ret == NAME_WAV_CONFLICT){
		sprintf(buf,"Wave %s already exists.",dataWaveName);
		debugOutputToHistory(DEBUG_LEVEL,buf);
		p->result = WAVE_EXIST;
		return 0;	
	}

	if(ret != 0 ){
		sprintf(buf,"Error %d in creating wave %s.",ret, dataWaveName);
		outputToHistory(buf);
		p->result = UNKNOWN_ERROR;
		return 0;
	}

	// lock wave
	int hState=MoveLockHandle(waveHandle);

	int* dataPtr = (int*) WaveData(waveHandle);

	// copy data fast :)
	memcpy(dataPtr,pBuffer,count*sizeof(int));

	HSetState((Handle) waveHandle, hState);

	pMyData->setOtherWaveNote(brickletID,waveHandle);

	p->result = SUCCESS;
	return 0;
}

// variable getNumberOfBricklets(variable *totalNumberOfBricklets)
static int getNumberOfBricklets(getNumberOfBrickletsParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	if(p->totalNumberOfBricklets == NULL){
		p->result = WRONG_PARAMETER;
	}

	*p->totalNumberOfBricklets = pSession->getBrickletCount();

	p->result = SUCCESS;
	return 0;
}

// variable getResultFileName(string *filename)
static int getResultFileName(getResultFileNameParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	if(p->filename == NULL){
		p->result = WRONG_PARAMETER;
		return 0;
	}

	PutCStringInHandle(pMyData->getResultFileName().c_str(), *p->filename);

	p->result = SUCCESS;
	return 0;
}

// variable getResultFilePath(string *absoluteFilePath)
static int getResultFilePath(getResultFilePathParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	if(p->absoluteFilePath == NULL){
		p->result = WRONG_PARAMETER;
		return 0;
	}

	PutCStringInHandle(pMyData->getResultFilePath().c_str(), *p->absoluteFilePath);

	p->result = SUCCESS;
	return 0;
}

// variable getVernissageVersion(string *vernissageVersion)
static int getVernissageVersion(getVernissageVersionParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	if(p->vernissageVersion == NULL){
		p->result = WRONG_PARAMETER;
		return 0;
	}

	*p->vernissageVersion = stringToAnyType<double>(pMyData->getVernissageVersion());

	p->result = SUCCESS;
	return 0;
}

// variable getXOPVersion(string *xopVersion)
static int getXOPVersion(getXOPVersionParams *p){

	p->result = UNKNOWN_ERROR;

	if(p->xopVersion == NULL){
		p->result = WRONG_PARAMETER;
		return 0;
	}

	*p->xopVersion = stringToAnyType<double>(myXopVersion);

	p->result = SUCCESS;
	return 0;
}

// variable openResultFile(string absoluteFilePath, string fileName)
static int openResultFile(openResultFileParams *p){

	p->result = UNKNOWN_ERROR;

	char filePath[MAX_PATH_LEN+1];
	char fileName[MAX_PATH_LEN+1];
	std::wstring wstringFilePath,wstringFileName;
	char buf[ARRAY_SIZE];
	int ret = 0,i;
	void* pContext  = NULL;
	void* pBricklet = NULL;

	ASSERT_RETURN_ZERO(pMyData);
	if(pMyData->resultFileOpen()){
		p->result = ALREADY_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	ret = GetCStringFromHandle(p->absoluteFilePath,filePath,MAX_PATH_LEN);
	if( ret != 0 ){
		return ret;
	}

	ret = GetCStringFromHandle(p->fileName,fileName,MAX_PATH_LEN);
	if( ret != 0 ){
		return ret;
	}

	wstringFilePath = CharPtrToWString(filePath);
	wstringFileName = CharPtrToWString(fileName);

	sprintf(buf,"filename %s",WStringToString(wstringFileName).c_str());
	debugOutputToHistory(DEBUG_LEVEL,buf);

	sprintf(buf,"filepath %s",WStringToString(wstringFilePath).c_str());
	debugOutputToHistory(DEBUG_LEVEL,buf);

	if( !pSession->directoryExists(wstringFilePath)){
		sprintf(buf,"The folder %s is not readable.",WStringToString(wstringFilePath).c_str());
		outputToHistory(buf);
		p->result = FILE_NOT_READABLE;
		return 0;	
	}
	if( !fileExists(WStringToString(wstringFilePath), WStringToString(wstringFileName))){
		sprintf(buf,"The file %s is not readable.",WStringToString(wstringFileName).c_str());
		outputToHistory(buf);
		p->result = FILE_NOT_READABLE;
		return 0;	
	}

	pSession->loadResultSet(wstringFilePath,wstringFileName,true);

	//starting from here the result file is valid
	pMyData->setResultFile(WStringToString(wstringFilePath),WStringToString(wstringFileName));

	for(i=1; i <= pSession->getBrickletCount(); i++ ){
		pBricklet = pSession->getNextBricklet(&pContext);
		ASSERT_RETURN_ZERO(pBricklet);
		pMyData->setBrickletClassMap(i,pBricklet);
	}

	p->result = SUCCESS;
	return 0;
}

// TODO
// variable checkForNewBricklets(variable *startBrickletID,variable *endBrickletID,variable rememberCalls)
static int checkForNewBricklets(checkForNewBrickletsParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	p->result = SUCCESS;
	return 0;
}

// variable getAllBrickletData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletData(getAllBrickletDataParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	getRangeBrickletDataParams rangeParams;
	rangeParams.baseName = p->baseName;
	rangeParams.startBrickletID = 1;
	rangeParams.endBrickletID = pSession->getBrickletCount();
	rangeParams.separateFolderForEachBricklet=p->separateFolderForEachBricklet;

	getRangeBrickletData(&rangeParams);
	
	p->result = rangeParams.result;
	return 0;
}

// variable getAllBrickletMetaData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletMetaData(getAllBrickletMetaDataParams *p){

	p->result = UNKNOWN_ERROR;
	int ret;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	getRangeBrickletMetaDataParams rangeParams;
	rangeParams.baseName = p->baseName;
	rangeParams.startBrickletID = 1;
	rangeParams.endBrickletID = pSession->getBrickletCount();
	rangeParams.separateFolderForEachBricklet=p->separateFolderForEachBricklet;

	ret = getRangeBrickletMetaData(&rangeParams);

	p->result = rangeParams.result;
	return ret;
}

// variable getBrickletData(string baseName, variable separateFolderForEachBricklet, variable brickletID)
static int getBrickletData(getBrickletDataParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	getRangeBrickletDataParams rangeParams;
	rangeParams.baseName = p->baseName;
	rangeParams.startBrickletID = p->brickletID;
	rangeParams.endBrickletID = p->brickletID;
	rangeParams.separateFolderForEachBricklet=p->separateFolderForEachBricklet;

	getRangeBrickletData(&rangeParams);
	
	p->result = rangeParams.result;
	return 0;

}

// variable getBrickletMetaData(string metaData, variable brickletID)
static int getBrickletMetaData(getBrickletMetaDataParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	getRangeBrickletMetaDataParams rangeParams;
	rangeParams.baseName = p->baseName;
	rangeParams.startBrickletID = p->brickletID;
	rangeParams.endBrickletID = p->brickletID;
	rangeParams.separateFolderForEachBricklet=p->separateFolderForEachBricklet;

	getRangeBrickletMetaData(&rangeParams);
	
	p->result = rangeParams.result;
	return 0;
}

// TODO
// string getErrorMessage(variable errorCode)
static int getErrorMessage(getErrorMessageParams *p){

	//p->result = NU;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		//p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	//p->result = SUCCESS;
	return 0;
}

// TODO
// variable getRangeBrickletData(string baseName, variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletData(getRangeBrickletDataParams *p){

	MyBricklet* myBricklet = NULL;
	std::vector<std::string> keys,values;
	char dataBaseName[MAX_OBJ_NAME+1], dataFolderName[MAX_OBJ_NAME+1], dataName[MAX_OBJ_NAME+1];
	DataFolderHandle parentDataFolderHPtr = NULL, newDataFolderHPtr = NULL;
	char buf[ARRAY_SIZE];
	int brickletID=-1, ret=-1;

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		p->result = EMPTY_RESULTFILE;
		return 0;
	}

	if( ( p->separateFolderForEachBricklet - 0.0) > 1e-5
		&& ( p->separateFolderForEachBricklet - 1.0) > 1e-5){
			p->result = WRONG_PARAMETER;
			debugOutputToHistory(DEBUG_LEVEL,"Paramteter separateFolderForEachBricklet must be 0 or 1");
			return 0;
	}

	if(	p->startBrickletID >  p->endBrickletID
		|| p->startBrickletID <  1 // brickletIDs are 1-based
		|| p->endBrickletID   <  1 
		|| p->startBrickletID > numberOfBricklets
		|| p->endBrickletID   > numberOfBricklets){
			sprintf(buf,"Paramteter brickletID must lie between 1 and %d (both included)",numberOfBricklets);
			debugOutputToHistory(DEBUG_LEVEL,buf);
			p->result = INVALID_RANGE;
			return 0;
	}
	// from here on we have a none empty result set open and valid start- and end bricklet IDs

	if( GetHandleSize(p->baseName) == 0L ){
		sprintf(dataBaseName,"brickletData");
	}
	else
	{
		ret = GetCStringFromHandle(p->baseName,dataBaseName,MAX_OBJ_NAME);
		if(ret != 0){
			p->result = ret;
			return 0;
		}
	}
	// now we got a valid baseName

	for(brickletID=p->startBrickletID; brickletID <= p->endBrickletID; brickletID++){

		myBricklet = pMyData->getBrickletClassFromMap(brickletID);
		ASSERT_RETURN_ZERO(myBricklet);

		sprintf(dataName,"%s_%03d",dataBaseName,brickletID);

		if(p->separateFolderForEachBricklet != 0.0){

			ret = GetCurrentDataFolder(&parentDataFolderHPtr);

			if(ret != 0){
				p->result = ret;
				return 0;
			}
			sprintf(dataFolderName,"X_%03d",brickletID);
			ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);
	
			// continue if the datafolder alrady exists, abort on all other errors
			if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
				p->result = ret;
				return 0;
			}
		}
			ret = createAndFillDataWave(newDataFolderHPtr,dataName,brickletID);

			if(ret != 0){
				sprintf(buf,"BUG: internal error (%d) in createAndFillDataWave ",ret);
				outputToHistory(buf);
				p->result = ret;
				return 0;
			}
	}
	p->result = SUCCESS;
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

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();
	if(numberOfBricklets == 0){
		p->result = EMPTY_RESULTFILE;
		return 0;
	}

	if( ( p->separateFolderForEachBricklet - 0.0) > 1e-5
		&& ( p->separateFolderForEachBricklet - 1.0) > 1e-5){
			p->result = WRONG_PARAMETER;
			debugOutputToHistory(DEBUG_LEVEL,"Paramteter separateFolderForEachBricklet must be 0 or 1");
			return 0;
	}

	if(	p->startBrickletID >  p->endBrickletID
		|| p->startBrickletID <  1 // brickletIDs are 1-based
		|| p->endBrickletID   <  1 
		|| p->startBrickletID > numberOfBricklets
		|| p->endBrickletID   > numberOfBricklets){
			sprintf(buf,"Paramteter brickletID must lie between 1 and %d (both included)",numberOfBricklets);
			debugOutputToHistory(DEBUG_LEVEL,buf);
			p->result = INVALID_RANGE;
			return 0;
	}
	// from here on we have a none empty result set open and valid start- and end bricklet IDs

	if( GetHandleSize(p->baseName) == 0L ){
		sprintf(metaDataBaseName,"brickletMetaData");
	}
	else
	{
		ret = GetCStringFromHandle(p->baseName,metaDataBaseName,MAX_OBJ_NAME);
		if(ret != 0){
			p->result = ret;
			return 0;
		}
	}
	// now we got a valid baseName

	for(brickletID=p->startBrickletID; brickletID <= p->endBrickletID; brickletID++){


		myBricklet = pMyData->getBrickletClassFromMap(brickletID);
		ASSERT_RETURN_ZERO(myBricklet);

		sprintf(metaDataName,"%s_%03d",metaDataBaseName,brickletID);

		if(p->separateFolderForEachBricklet != 0.0){

			ret = GetCurrentDataFolder(&parentDataFolderHPtr);

			if(ret != 0){
				p->result = ret;
				return 0;
			}
			sprintf(dataFolderName,"X_%03d",brickletID);
			ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);
	
			// continue if the datafolder alrady exists, abort on all other errors
			if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
				p->result = ret;
				return 0;
			}
		}
			myBricklet->getBrickletMetaData(keys,values);

			ret = createAndFillTextWave(keys,values,newDataFolderHPtr,metaDataName,brickletID);

			if(ret != 0){
				sprintf(buf,"BUG: internal error (%d) in createAndFillTextWave ",ret);
				outputToHistory(buf);
				p->result = ret;
				return 0;
			}
	}
	p->result = SUCCESS;
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

    BOOL retValue = GetVersionEx(&osvi);

	str.append("####\r");
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
		str.append("Unknown compiler\r");
	#endif
	str.append("Igor Pro Version: " + anyTypeToString<int>(igorVersion) + "\r");
	if(pMyData != NULL){
		str.append("Vernissage version: " + pMyData->getVernissageVersion() + "\r");
	}
	else{
		str.append("Venissage version: Can't access internal data\r");
	}
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

	char metaDataWaveName[MAX_OBJ_NAME+1];
	int ret;
	std::vector<std::string> keys,values;
	char buf[ARRAY_SIZE];
	int numberOfBricklets;
	void *pBricklet;
	tm ctime;
	Vernissage::Session::BrickletMetaData brickletMetaData;

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	if( GetHandleSize(p->waveName) == 0L ){
		sprintf(metaDataWaveName,"resultFileMetaData");
	}
	else{
		ret = GetCStringFromHandle(p->waveName,metaDataWaveName,MAX_OBJ_NAME);
		if(ret != 0){
			p->result = ret;
			return 0;
		}
	}

	// use the timestamp of the last bricklet as dateOfLastChange
	numberOfBricklets = pSession->getBrickletCount();

	pBricklet = pMyData->getBrickletClassFromMap(numberOfBricklets)->getBrickletPointer();
	ASSERT_RETURN_ZERO(pBricklet);

	ctime = pSession->getCreationTimestamp(pBricklet);

	brickletMetaData = pSession->getMetaData(pBricklet);

	keys.push_back("filePath");
	values.push_back(pMyData->getResultFilePath());

	keys.push_back("fileName");
	values.push_back(pMyData->getResultFileName());

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

	// brickletID=0 because the waveNote of the resultfile metadata does hold an empty brickletID wavenote property
	ret = createAndFillTextWave(keys,values,NULL,metaDataWaveName,0);

	if(ret != 0){
		sprintf(buf,"BUG: internal error (%d) in createAndFillTextWave ",ret);
		outputToHistory(buf);
		p->result = ret;
		return 0;
	}

	p->result = SUCCESS;
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
			returnValue = (long) getAllBrickletData;
			break;
		case 3:						
			returnValue = (long) getAllBrickletMetaData;
			break;
		case 4:						
			returnValue = (long) getBrickletData;
			break;
		case 5:						
			returnValue = (long) getBrickletMetaData;
			break;
		case 6:						
			returnValue = (long) getBrickletRawData;
			break;
		case 7:						
			returnValue = (long) getBugReportTemplate;
			break;
		case 8:						
			returnValue = (long) getErrorMessage;
			break;
		case 9:						
			returnValue = (long) getNumberOfBricklets;
			break;
		case 10:						
			returnValue = (long) getRangeBrickletData;
			break;
		case 11:						
			returnValue = (long) getRangeBrickletMetaData;
			break;
		case 12:						
			returnValue = (long) getResultFileMetaData;
			break;
		case 13:						
			returnValue = (long) getResultFileName;
			break;
		case 14:						
			returnValue = (long) getResultFilePath;
			break;
		case 15:						
			returnValue = (long) getVernissageVersion;
			break;
		case 16:						
			returnValue = (long) getXOPVersion;
			break;
		case 17:						
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

	delete pMyData;
	pMyData = NULL;
}
