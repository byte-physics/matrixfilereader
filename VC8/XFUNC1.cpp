/*	XFUNC1.c -- illustrates Igor external functions.

*/

#include <string>
#include <set>


#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "XFUNC1.h"

#include "dataclass.h"

#include "globalvariables.h"
#include "utils.h"

#define DEBUG_LEVEL 1

#include "Vernissage.h"

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct closeResultFileParams{
	double  result;	
};
typedef struct closeResultFileParams closeResultFileParams;
#pragma pack()

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

	pSession->eraseResultSets();
	pMyData->closeSession();

	p->result = SUCCESS;
	return 0;
}


#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBrickletRawDataParams{
	Handle  dataWave;
	double  brickletID;
	double  result;	
};
typedef struct getBrickletRawDataParams getBrickletRawDataParams;
#pragma pack()

// variable getBrickletRawData(variable brickletID, string dataWave)
static int getBrickletRawData(getBrickletRawDataParams *p){

	p->result = UNKNOWN_ERROR;
	char buf[ARRAY_SIZE];

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

	if(p->dataWave == NULL ){
		p->result = WRONG_PARAMETER;
		return 0;
	}

	debugOutputToHistory(DEBUG_LEVEL,"Got correct wave, and correct brickletID");

	const int *pBuffer;
	int count=0;

	pMyData->getBrickletContentsBuffer(brickletID,&pBuffer,count);
	ASSERT_RETURN_ZERO(pBuffer);

	if(count == 0){
		outputToHistory("Could not load bricklet contents.");
		p->result = UNKNOWN_ERROR;
		return 0;
	}

	char dataWaveName[MAX_OBJ_NAME+1];
	int ret;
	ret = GetCStringFromHandle(p->dataWave,dataWaveName,MAX_OBJ_NAME);

	if(ret != 0){
		p->result = ret;
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
		sprintf(buf,"Error %d in creating wave.",ret);
		outputToHistory(buf);
		p->result = UNKNOWN_ERROR;
		return 0;
	}

	// lock wave
	int hState=MoveLockHandle(waveHandle);

	int* dataPtr = (int*) WaveData(waveHandle);
	for(int i=0; i < count; i++){
		dataPtr[i] = pBuffer[i];
	}
	HSetState((Handle) waveHandle, hState);

	p->result = SUCCESS;
	return 0;
}



#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getNumberOfBrickletsParams{
	double *totalNumberOfBricklets;
	double  result;	
};
typedef struct getNumberOfBrickletsParams getNumberOfBrickletsParams;
#pragma pack()

// variable getNumberOfBricklets(variable *totalNumberOfBricklets)
static int getNumberOfBricklets(getNumberOfBrickletsParams *p){

	p->result = UNKNOWN_ERROR;
	*p->totalNumberOfBricklets = -1;

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

	if(*p->totalNumberOfBricklets == 0){
		p->result = EMPTY_RESULTFILE;
	}

	p->result = SUCCESS;
	return 0;
}


#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getResultFileNameParams{
	Handle *filename;
	double  result;	
};
typedef struct getResultFileNameParams getResultFileNameParams;
#pragma pack()

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
		//return NULL_STR_VAR;
	}

	PutCStringInHandle(pMyData->getResultFileName().c_str(), *p->filename);

	p->result = SUCCESS;
	return 0;
}


#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getResultFilePathParams{
	Handle *absoluteFilePath;
	double  result;	
};
typedef struct getResultFilePathParams getResultFilePathParams;
#pragma pack()

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




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getVernissageVersionParams{
	Handle *vernissageVersion;
	double  result;	
};
typedef struct getVernissageVersionParams getVernissageVersionParams;
#pragma pack()

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

	PutCStringInHandle(pMyData->getVernissageVersion().c_str(), *p->vernissageVersion);

	p->result = SUCCESS;
	return 0;
}


#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getXOPVersionParams{
	Handle *xopVersion;
	double  result;	
};
typedef struct getXOPVersionParams getXOPVersionParams;
#pragma pack()

// variable getXOPVersion(string *xopVersion)
static int getXOPVersion(getXOPVersionParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	if(p->xopVersion == NULL){
		p->result = WRONG_PARAMETER;
		return 0;
	}

	PutCStringInHandle(myVersion, *p->xopVersion);

	p->result = SUCCESS;
	return 0;
}



#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct openResultFileParams{
	Handle  fileName;
	Handle  absoluteFilePath;
	double  result;	
};
typedef struct openResultFileParams openResultFileParams;
#pragma pack()

// variable openResultFile(string absoluteFilePath, string fileName)
static int openResultFile(openResultFileParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(pMyData->resultFileOpen()){
		p->result = ALREADY_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();

	if(pSession == NULL){
		outputToHistory("Could not create session object.");
		p->result = UNKNOWN_ERROR;
		return 0;
	}

	char filePath[MAX_PATH_LEN+1];
	char fileName[MAX_PATH_LEN+1];
	std::wstring wstringFilePath,wstringFileName;

	int returnValueInt = 0;
	
	returnValueInt = GetCStringFromHandle(p->absoluteFilePath,filePath,MAX_PATH_LEN);
	if( returnValueInt != 0 ){
		return returnValueInt;
	}

	returnValueInt = GetCStringFromHandle(p->fileName,fileName,MAX_PATH_LEN);
	if( returnValueInt != 0 ){
		return returnValueInt;
	}

	wstringFilePath = CharPtrToWString(filePath);
	wstringFileName = CharPtrToWString(fileName);

	char buf[1000];
	sprintf(buf,"filename %s",WStringToString(wstringFileName).c_str());
	debugOutputToHistory(DEBUG_LEVEL,buf);

	sprintf(buf,"filepath %s",WStringToString(wstringFilePath).c_str());
	debugOutputToHistory(DEBUG_LEVEL,buf);

	// TODO check for file existence

	bool retValue = pSession->loadResultSet(wstringFilePath,wstringFileName,true);


	if(!retValue){
		sprintf(buf,"File %s/%s is not readable.",WStringToString(wstringFilePath).c_str(),WStringToString(wstringFileName).c_str());
		outputToHistory(buf);
		p->result = FILE_NOT_READABLE;
		return 0;
	}

	if(pSession->getBrickletCount() == 0){
		p->result = EMPTY_RESULTFILE;
		return 0;
	}

	//starting from here the result file is valid
	pMyData->setResultFile(WStringToString(wstringFilePath),WStringToString(wstringFileName));

	void* pContext  = NULL;
	void* pBricklet = NULL;

	for(int i=1; i <= pSession->getBrickletCount(); i++ ){
		pBricklet = pSession->getNextBricklet(&pContext);
		ASSERT_RETURN_ZERO(pBricklet);
		pMyData->setBrickletPointerMap(i,pBricklet);
	}

	p->result = SUCCESS;
	return 0;
}



#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct checkForNewBrickletsParams{
	double  rememberCalls;
	double *endBrickletID;
	double *startBrickletID;
	double  result;	
};
typedef struct checkForNewBrickletsParams checkForNewBrickletsParams;
#pragma pack()

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







#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getAllBrickletDataParams{
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;	
};
typedef struct getAllBrickletDataParams getAllBrickletDataParams;
#pragma pack()

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



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getAllBrickletMetaDataParams{
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;	
};
typedef struct getAllBrickletMetaDataParams getAllBrickletMetaDataParams;
#pragma pack()

// variable getAllBrickletMetaData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletMetaData(getAllBrickletMetaDataParams *p){

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




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBrickletDataParams{
	double  brickletID;
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;	
};
typedef struct getBrickletDataParams getBrickletDataParams;
#pragma pack()

// variable getBrickletData(string baseName, variable separateFolderForEachBricklet, variable brickletID)
static int getBrickletData(getBrickletDataParams *p){

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




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBrickletMetaDataParams{
	Handle  metaData;
	double  brickletID;
	double  result;	
};
typedef struct getBrickletMetaDataParams getBrickletMetaDataParams;
#pragma pack()

// variable getBrickletMetaData(variable brickletID, string metaData)
static int getBrickletMetaData(getBrickletMetaDataParams *p){

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





#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getErrorMessageParams{
	double  errorCode;
	Handle  result;	
};
typedef struct getErrorMessageParams getErrorMessageParams;
#pragma pack()

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



#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getRangeBrickletDataParams{
	double  endBrickletID;
	double  startBrickletID;
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;	
};
typedef struct getRangeBrickletDataParams getRangeBrickletDataParams;
#pragma pack()

// variable getRangeBrickletData(string baseName, variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletData(getRangeBrickletDataParams *p){

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




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getRangeBrickletMetaDataParams{
	double  endBrickletID;
	double  startBrickletID;
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;	
};
typedef struct getRangeBrickletMetaDataParams getRangeBrickletMetaDataParams;
#pragma pack()

// variable getRangeBrickletMetaData(string baseName,variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletMetaData(getRangeBrickletMetaDataParams *p){

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




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getResultFileMetaDataParams{
	Handle  waveName;
	double  result;	
};
typedef struct getResultFileMetaDataParams getResultFileMetaDataParams;
#pragma pack()

// variable getResultFileMetaData(string waveName)
static int getResultFileMetaData(getResultFileMetaDataParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	if(p->waveName == NULL){
		p->result = WRONG_PARAMETER;
		return 0;
	}

	char metaDataWaveName[MAX_OBJ_NAME+1];
	int ret = GetCStringFromHandle(p->waveName,metaDataWaveName,MAX_OBJ_NAME);

	if(ret != 0){
		p->result = ret;
		return 0;
	}

	std::vector<std::string> metaData;

 //*		- filename
 //*		- filepath
 //*		- BrickletMetaData.fileCreatorName
 //*		- BrickletMetaData.fileCreatorVersion
 //*		- BrickletMetaData.userName
 //*		- BrickletMetaData.accountName
 //*		- totalNumberOfBricklets
 //*		- changeDate (this will have the timestamp of the newest bricklet)

	int numberOfBricklets = pSession->getBrickletCount();
	void* pBricklet = pMyData->getBrickletPointerFromMap(numberOfBricklets);
	tm ctime = pSession->getCreationTimestamp(pBricklet);
	char buf[ARRAY_SIZE];

	Vernissage::Session::BrickletMetaData brickletMetaData = pSession->getMetaData(pBricklet);

	metaData.push_back("filePath");
	metaData.push_back("fileName");
	metaData.push_back("totalNumberOfBricklets");
	metaData.push_back("changeDate");
	metaData.push_back("BrickletMetaData.fileCreatorName");
	metaData.push_back("BrickletMetaData.fileCreatorVersion");
	metaData.push_back("BrickletMetaData.userName");
	metaData.push_back("BrickletMetaData.accountName");

	metaData.push_back(pMyData->getResultFilePath());
	metaData.push_back(pMyData->getResultFileName());
	metaData.push_back(anyTypeToString<int>(numberOfBricklets));
	
	sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d",ctime.tm_mon+1,ctime.tm_mday,ctime.tm_year+1900, ctime.tm_hour,ctime.tm_min,ctime.tm_sec);
	metaData.push_back(buf);

	metaData.push_back(WStringToString(brickletMetaData.fileCreatorName));
	metaData.push_back(WStringToString(brickletMetaData.fileCreatorVersion));
	metaData.push_back(WStringToString(brickletMetaData.userName));
	metaData.push_back(WStringToString(brickletMetaData.accountName));

	long dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));

	// create 2D textwave wave with count points
	if(metaData.size() % 2 == 0){
		dimensionSizes[ROWS]=metaData.size()/2;
	}
	else{
		outputToHistory("BUG: wrong size of textwave contents");
		return 0;
	}
	dimensionSizes[COLUMNS]=2;

	waveHndl waveHandle;
	int overwrite=0;

	ret = MDMakeWave(&waveHandle,metaDataWaveName,NULL,dimensionSizes,TEXT_WAVE_TYPE,overwrite);

	if(ret == NAME_WAV_CONFLICT){
		sprintf(buf,"Wave %s already exists.",metaDataWaveName);
		debugOutputToHistory(DEBUG_LEVEL,buf);
		p->result = WAVE_EXIST;
		return 0;	
	}

	ret = stringVectorToTextWave(metaData,waveHandle);

	if(ret != 0){
		sprintf(buf,"stringVectorToTextWave returned %d",ret);
		outputToHistory(buf);
		p->result=ret;
	}
	else{
		p->result = SUCCESS;
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
			returnValue = (long) getErrorMessage;
			break;
		case 8:						
			returnValue = (long) getNumberOfBricklets;
			break;
		case 9:						
			returnValue = (long) getRangeBrickletData;
			break;
		case 10:						
			returnValue = (long) getRangeBrickletMetaData;
			break;
		case 11:						
			returnValue = (long) getResultFileMetaData;
			break;
		case 12:						
			returnValue = (long) getResultFileName;
			break;
		case 13:						
			returnValue = (long) getResultFilePath;
			break;
		case 14:						
			returnValue = (long) getVernissageVersion;
			break;
		case 15:						
			returnValue = (long) getXOPVersion;
			break;
		case 16:						
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
