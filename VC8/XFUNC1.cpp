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

	if(p->baseName == NULL ){
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
	ret = GetCStringFromHandle(p->baseName,dataWaveName,MAX_OBJ_NAME);

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
		sprintf(buf,"Error %d in creating wave %s.",ret, dataWaveName);
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

	PutCStringInHandle(pMyData->getVernissageVersion().c_str(), *p->vernissageVersion);

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

	PutCStringInHandle(myVersion, *p->xopVersion);

	p->result = SUCCESS;
	return 0;
}

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

	//if(pSession->getBrickletCount() == 0){
	//	p->result = EMPTY_RESULTFILE;
	//	return 0;
	//}

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

	p->result = SUCCESS;
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

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	getRangeBrickletDataParams rangeParams;
	rangeParams.baseName = p->baseName;
	rangeParams.startBrickletID = p->brickletID;
	rangeParams.endBrickletID = p->brickletID;
	rangeParams.separateFolderForEachBricklet=0;

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

	if(p->baseName == NULL){
		p->result = WRONG_PARAMETER;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);

	getRangeBrickletMetaDataParams rangeParams;
	rangeParams.baseName = p->baseName;
	rangeParams.startBrickletID = p->brickletID;
	rangeParams.endBrickletID = p->brickletID;
	rangeParams.separateFolderForEachBricklet=0;

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

// TODO
// variable getRangeBrickletMetaData(string baseName,variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletMetaData(getRangeBrickletMetaDataParams *p){

	void* pBricklet = NULL;
	std::vector<std::string> keys,values;
	std::vector<std::wstring> elementInstanceNames;
	std::map<std::wstring, Vernissage::Session::Parameter> elementInstanceParamsMap;
	Vernissage::Session::SpatialInfo spatialInfo;
	Vernissage::Session::ExperimentInfo experimentInfo;

	char metaDataBaseName[MAX_OBJ_NAME+1], dataFolderName[MAX_OBJ_NAME+1], metaDataName[MAX_OBJ_NAME+1];
	DataFolderHandle parentDataFolderHPtr = NULL, newDataFolderHPtr = NULL;
	char buf[ARRAY_SIZE];
	int brickletID=-1, ret=-1, index=-1;

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

	if(	p->startBrickletID >  p->endBrickletID
		|| p->startBrickletID <  1 // brickletIDs are 1-based
		|| p->endBrickletID   <  1 
		|| p->startBrickletID > numberOfBricklets
		|| p->endBrickletID   > numberOfBricklets){
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


		pBricklet = pMyData->getBrickletPointerFromMap(brickletID);
		ASSERT_RETURN_ZERO(pBricklet);

		sprintf(metaDataName,"%s_%03d",metaDataBaseName,brickletID);

		if(p->separateFolderForEachBricklet != 0.0){

			ret = GetCurrentDataFolder(&parentDataFolderHPtr);

			if(ret != 0){
				p->result = ret;
				return 0;
			}
			sprintf(dataFolderName,"X_%03d",brickletID);
			ret = NewDataFolder(parentDataFolderHPtr, dataFolderName, &newDataFolderHPtr);
		
			if(ret == FOLDER_NAME_EXISTS){
				p->result = DATAFOLDER_EXIST;
				return 0;
			}
			else if(ret != 0){
				p->result = ret;
				return 0;
			}
		}

		keys.clear();
		values.clear();
		elementInstanceNames.clear();

		if( !pMyData->gotCachedBrickletMetaData(brickletID) ){

			// timestamp of creation
			tm ctime = pSession->getCreationTimestamp(pBricklet);

			keys.push_back("creationTimeStamp");
			values.push_back(anyTypeToString<time_t>(mktime(&ctime)));

			// BEGIN pSession->getBrickletMetaData
			Vernissage::Session::BrickletMetaData brickletMetaData = pSession->getMetaData(pBricklet);
			keys.push_back("brickletMetaData.fileCreatorName");
			values.push_back(WStringToString(brickletMetaData.fileCreatorName));

			keys.push_back("brickletMetaData.fileCreatorVersion");
			values.push_back(WStringToString(brickletMetaData.fileCreatorVersion));
			
			keys.push_back("brickletMetaData.accountName");
			values.push_back(WStringToString(brickletMetaData.accountName));

			keys.push_back("brickletMetaData.userName");
			values.push_back(WStringToString(brickletMetaData.userName));
			// END pSession->getBrickletMetaData 
			
			keys.push_back("sequenceID");
			values.push_back(anyTypeToString<int>(pSession->getSequenceId(pBricklet)));

			keys.push_back("creationComment");
			values.push_back(WStringToString(pSession->getCreationComment(pBricklet)));

			keys.push_back("dimension");
			values.push_back(anyTypeToString<int>(pSession->getDimensionCount(pBricklet)));

			keys.push_back("rootAxis");
			values.push_back(WStringToString(pSession->getRootAxisName(pBricklet)));

			keys.push_back("triggerAxis");
			values.push_back(WStringToString(pSession->getTriggerAxisName(pBricklet)));

			keys.push_back("channelName");
			values.push_back(WStringToString(pSession->getChannelName(pBricklet)));

			keys.push_back("channelInstanceName");
			values.push_back(WStringToString(pSession->getChannelInstanceName(pBricklet)));

			keys.push_back("runCycleCount");
			values.push_back(anyTypeToString<int>(pSession->getRunCycleCount(pBricklet)));

			keys.push_back("scanCycleCount");
			values.push_back(anyTypeToString<int>(pSession->getScanCycleCount(pBricklet)));

			elementInstanceNames = pSession->getExperimentElementInstanceNames(pBricklet,L"");

			std::vector<std::wstring>::iterator itElementInstanceNames;
			std::map<std::wstring, Vernissage::Session::Parameter>::iterator itMap;
			
			for(itElementInstanceNames = elementInstanceNames.begin(); itElementInstanceNames != elementInstanceNames.end(); itElementInstanceNames++){

				elementInstanceParamsMap = pSession->getExperimentElementParameters(pBricklet,*itElementInstanceNames);

				for(itMap = elementInstanceParamsMap.begin(); itMap != elementInstanceParamsMap.end(); itMap++){

					keys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first));
					values.push_back("");

					keys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first) + std::string(".value") );
					values.push_back( WStringToString(itMap->second.value));

					keys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first) + std::string(".unit") );
					values.push_back( WStringToString(itMap->second.unit));

					keys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first) + std::string(".valueType") );
					values.push_back( anyTypeToString<int>(itMap->second.valueType));
				}
			}

			// BEGIN Vernissage::Session::SpatialInfo
			//spatialInfo = pSession->getSpatialInfo(pBricklet);
			// Vernissage crashes here with getSpatialInfo ??

			//std::vector<double>::const_iterator it;
			//for( it = spatialInfo.physicalX.begin(); it != spatialInfo.physicalX.end(); it++){
			//	index = it - spatialInfo.physicalX.begin() + 1 ; // one-based Index
			//	sprintf(buf,"spatialInfo.physicalX.No%d",index);
			//	keys.push_back(buf);
			//	values.push_back(anyTypeToString<double>(*it));
			//};

			//for( it = spatialInfo.physicalY.begin(); it != spatialInfo.physicalY.end(); it++){
			//	index = it - spatialInfo.physicalY.begin() + 1 ; // one-based Index
			//	sprintf(buf,"spatialInfo.physicalY.No%d",index);
			//	keys.push_back(buf);
			//	values.push_back(anyTypeToString<double>(*it));
			//};

			//keys.push_back("spatialInfo.originatorKnown");
			//values.push_back(spatialInfo.originatorKnown ? "true" : "false");

			//keys.push_back("spatialInfo.channelName");
			//values.push_back(WStringToString(spatialInfo.channelName));
			//
			//keys.push_back("spatialInfo.sequenceId");
			//values.push_back(anyTypeToString<int>(spatialInfo.sequenceId));

			//keys.push_back("spatialInfo.runCycleCount");
			//values.push_back(anyTypeToString<int>(spatialInfo.runCycleCount));
			//
			//keys.push_back("spatialInfo.scanCycleCount");
			//values.push_back(anyTypeToString<int>(spatialInfo.scanCycleCount));
			//
			//keys.push_back("spatialInfo.viewName");
			//values.push_back(WStringToString(spatialInfo.viewName));
			//
			//keys.push_back("spatialInfo.viewSelectionId");
			//values.push_back(anyTypeToString<int>(spatialInfo.viewSelectionId));

			//keys.push_back("spatialInfo.viewSelectionIndex");
			//values.push_back(anyTypeToString<int>(spatialInfo.viewSelectionIndex));		
			//// END Vernissage::Session::SpatialInfo

			// BEGIN Vernissage::Session::ExperimentInfo
			experimentInfo = pSession->getExperimentInfo(pBricklet);

			keys.push_back("experimentInfo.Name");
			values.push_back(WStringToString(experimentInfo.experimentName));

			keys.push_back("experimentInfo.Version");
			values.push_back(WStringToString(experimentInfo.experimentVersion));

			keys.push_back("experimentInfo.Description");
			values.push_back(WStringToString(experimentInfo.experimentDescription));
			
			keys.push_back("experimentInfo.FileSpec");
			values.push_back(WStringToString(experimentInfo.experimentFileSpec));
			
			keys.push_back("experimentInfo.projectName");
			values.push_back(WStringToString(experimentInfo.projectName));
		
			keys.push_back("experimentInfo.projectVersion");
			values.push_back(WStringToString(experimentInfo.projectVersion));
			
			keys.push_back("experimentInfo.projectFileSpec");
			values.push_back(WStringToString(experimentInfo.projectFileSpec));
			// END Vernissage::Session::ExperimentInfo

			//std::vector<std::wstring> allAxes = getAllAxesNames(pSession,pBricklet);

			//Vernissage::Session::AxisDescriptor axisDescriptor;

			//for(vector<wstring>::iterator itAllAxes = allAxes.begin(); itAllAxes != allAxes.end(); itAllAxes++){
			//	wprintf(L"##Axis name: %s\n",itAllAxes->c_str());

			//	axisDescriptor = pSession->getAxisDescriptor(pBricklet,*itAllAxes);
			//	wprintf(L"\tclocks: %d\n", axisDescriptor.clocks);
			//	wprintf(L"\tmirrored: %s\n", (axisDescriptor.mirrored)? L"true" : L"false");
			//	wprintf(L"\tphysicalUnit: %s\n", axisDescriptor.physicalUnit.c_str());
			//	wprintf(L"\tphysicalIncrement: %g\n", axisDescriptor.physicalIncrement);
			//	wprintf(L"\tphysicalStart: %g\n", axisDescriptor.physicalStart);
			//	wprintf(L"\trawIncrement: %d\n",axisDescriptor.rawIncrement);
			//	wprintf(L"\trawStart: %d\n",axisDescriptor.rawStart);
			//	wprintf(L"\ttriggerAxisName: %s\n",axisDescriptor.triggerAxisName.c_str());

			//	axisTableSets = pSession->getAxisTableSets(pBricklet,*itAllAxes);

			//	printf("Number of axis table sets: %d\n",axisTableSets.size());


			//	for(Vernissage::Session::AxisTableSets::iterator itTabelSets = axisTableSets.begin(); itTabelSets != axisTableSets.end(); itTabelSets++ ){
			//		wprintf(L"\tAxis name: %s (start, stop, step)=",itTabelSets->first);
			//		for(Vernissage::Session::TableSet::iterator itOneTableSet = itTabelSets->second.begin(); itOneTableSet != itTabelSets->second.end(); itOneTableSet++){
			//			printf("\t(%g, %g, %g,),",itOneTableSet->start,itOneTableSet->step,itOneTableSet->stop);
			//		}
			//		printf("\n");
			//	}

			//	// if it is empty, we got the standard table set which is [1,clocks,1]
			//	if(axisTableSets.size() == 0){
			//		printf("Standard axis table set: [start,stop,step] = [1,%d,1]\n",axisDescriptor.clocks); 
			//	}


			ret = createAndFillTextWave(keys,values,newDataFolderHPtr,metaDataName);

			if(ret != 0){
				sprintf(buf,"BUG: internal error (%d) in createAndFillTextWave ",ret);
				outputToHistory(buf);
				p->result = ret;
				return 0;
			}

			 //the metaData could be saved to a textwave successfully, so we can cache it
			pMyData->storeBrickletMetaData(brickletID,keys,values);
		}
		else{ // we got the metaData cached
			pMyData->loadCachedBrickletMetaData(brickletID,keys,values);
			
			ret = createAndFillTextWave(keys,values,newDataFolderHPtr,metaDataName);

			if(ret != 0){
				sprintf(buf,"BUG: internal error (%d) in createAndFillTextWave ",ret);
				outputToHistory(buf);
				p->result = ret;
				return 0;
			}
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
	str.append("XOP version: " + std::string(myVersion) + "\r");
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

	std::vector<std::string> keys,values;
	char buf[ARRAY_SIZE];

	// use the timestamp of the last bricklet as dateOfLastChange
	int numberOfBricklets = pSession->getBrickletCount();
	void* pBricklet = pMyData->getBrickletPointerFromMap(numberOfBricklets);
	tm ctime = pSession->getCreationTimestamp(pBricklet);

	Vernissage::Session::BrickletMetaData brickletMetaData = pSession->getMetaData(pBricklet);

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

	keys.push_back("Brickletkeys.fileCreatorName");
	values.push_back(WStringToString(brickletMetaData.fileCreatorName));

	keys.push_back("Brickletkeys.fileCreatorVersion");
	values.push_back(WStringToString(brickletMetaData.fileCreatorVersion));

	keys.push_back("Brickletkeys.userName");
	values.push_back(WStringToString(brickletMetaData.userName));

	keys.push_back("Brickletkeys.accountName");
	values.push_back(WStringToString(brickletMetaData.accountName));

	ret = createAndFillTextWave(keys,values,NULL,metaDataWaveName);

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
