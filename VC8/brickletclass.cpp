/*
	The file brickletclass.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "stdafx.h"

#include "brickletclass.h"
#include "globaldata.h"
#include "utils_bricklet.h"
#include "extremadata.h"
#include "utils_generic.h"

BrickletClass::BrickletClass(void* const pBricklet,int brickletID)
:
m_brickletPtr(pBricklet),
m_rawBufferContents(NULL),
m_brickletID(brickletID),
m_VernissageSession(GlobalData::Instance().getVernissageSession()),
m_extrema(ExtremaData()){

	ASSERT_RETURN_VOID(m_VernissageSession);
}

BrickletClass::~BrickletClass(){
	this->clearCache();
}

/*	
	delete our internally cached data, is called at destruction time and if we want keep memory
	consumption low
*/
void BrickletClass::clearCache(){

	if(m_rawBufferContents != NULL){
		sprintf(GlobalData::Instance().outputBuffer,"Deleting raw data from bricklet %d",m_brickletID);
		debugOutputToHistory(GlobalData::Instance().outputBuffer);
		delete[] m_rawBufferContents;
		m_rawBufferContents=NULL;
		m_rawBufferContentsSize=0;
	}

	// resize to zero elements
	std::vector<std::string>().swap(m_metaDataKeys);
	std::vector<std::string>().swap(m_metaDataValues);
}

/*
	Load the raw data of the bricklet into our own cache
*/
void BrickletClass::getBrickletContentsBuffer(const int** pBuffer, int &count){

	ASSERT_RETURN_VOID(pBuffer);
	ASSERT_RETURN_VOID(m_VernissageSession);
	count=0;

	// we are not called the first time
	if(m_rawBufferContents != NULL){
		debugOutputToHistory("GlobalData::getBrickletContentsBuffer Using cached values");

		sprintf(GlobalData::Instance().outputBuffer,"before: pBuffer=%d,count=%d",*pBuffer,count);
		debugOutputToHistory(GlobalData::Instance().outputBuffer);

		*pBuffer = m_rawBufferContents;
		count    = m_rawBufferContentsSize;

		sprintf(GlobalData::Instance().outputBuffer,"after: pBuffer=%d,count=%d",*pBuffer,count);
		debugOutputToHistory(GlobalData::Instance().outputBuffer);
	}
	else{ // we are called the first time

		try{
			m_VernissageSession->loadBrickletContents(m_brickletPtr,pBuffer,count);
		}
		catch(...){
			sprintf(GlobalData::Instance().outputBuffer,"Could not load the bricklet contents, probably out of memory in getBrickletContentsBuffer() with bricklet %d",m_brickletID);
			outputToHistory(GlobalData::Instance().outputBuffer);
			*pBuffer = NULL;
			count = 0;
			return;	
		}
		// loadBrickletContents either throws an exception or returns pBuffer == 0 || count == 0
		if(*pBuffer == NULL || count == 0){
			sprintf(GlobalData::Instance().outputBuffer,"Out of memory in getBrickletContentsBuffer() with bricklet %d",m_brickletID);
			outputToHistory(GlobalData::Instance().outputBuffer);
			m_VernissageSession->unloadBrickletContents(m_brickletPtr);
			*pBuffer = NULL;
			count=0;
			return;
		}

		sprintf(GlobalData::Instance().outputBuffer,"pBuffer=%d,count=%d",*pBuffer,count);
		debugOutputToHistory(GlobalData::Instance().outputBuffer);

		// these two lines have to be surrounded by loadbrickletContents/unloadBrickletContents, otherwise loadbrickletContents will be called
		// implicitly which is quite expensive
		m_extrema.setMinimum(m_VernissageSession->getRawMin(m_brickletPtr),
							 m_VernissageSession->toPhysical(m_extrema.getRawMin(), m_brickletPtr)
							 );

		m_extrema.setMaximum(m_VernissageSession->getRawMax(m_brickletPtr),
							 m_VernissageSession->toPhysical(m_extrema.getRawMax(), m_brickletPtr)
							 );

		sprintf(GlobalData::Instance().outputBuffer,"rawMin=%d,rawMax=%d,scaledMin=%g,scaledMax=%g",
			m_extrema.getRawMin(),m_extrema.getRawMax(),m_extrema.getPhysValRawMin(),m_extrema.getPhysValRawMax());
		debugOutputToHistory(GlobalData::Instance().outputBuffer);

		// copy the raw data to our own cache
		m_rawBufferContentsSize = count;
		try{
			m_rawBufferContents = new int[m_rawBufferContentsSize];
		}
		catch(CMemoryException* e){
			e->Delete();
			outputToHistory("Out of memory in getBrickletContentsBuffer()");
			*pBuffer = NULL;
			count=0;
			m_VernissageSession->unloadBrickletContents(m_brickletPtr);
			return;
		}
		memcpy(m_rawBufferContents,*pBuffer,sizeof(int)*m_rawBufferContentsSize);
		*pBuffer = m_rawBufferContents;

		// release memory from vernissage DLL
		m_VernissageSession->unloadBrickletContents(m_brickletPtr);
	}
}

/*
	Wrapper function which returns a vector of the meta data keys values
*/
const std::vector<std::string>& BrickletClass::getBrickletMetaDataValues(){

	if(m_metaDataKeys.empty() ||  m_metaDataValues.empty()){
		try{
			loadBrickletMetaDataFromResultFile();
		}
		catch(CMemoryException* e){
			e->Delete();
			XOPNotice("Out of memory in getBrickletMetaDataValues()");
			return m_metaDataValues;
		}
	}
	return m_metaDataValues;
}

/*
	Wrapper function which returns a vector of the meta data keys
*/
const std::vector<std::string>& BrickletClass::getBrickletMetaDataKeys(){

	if(m_metaDataKeys.empty() ||  m_metaDataValues.empty()){
		try{
			loadBrickletMetaDataFromResultFile();
		}
		catch(CMemoryException* e){
			e->Delete();
			XOPNotice("Out of memory in getBrickletMetaData()");
			return m_metaDataKeys;
		}
	}
	return m_metaDataKeys;
}

/*
	Reads all meta data into our own structures
*/
void BrickletClass::loadBrickletMetaDataFromResultFile(){

	std::vector<std::wstring> elementInstanceNames;
	std::map<std::wstring, Vernissage::Session::Parameter> elementInstanceParamsMap;
	Vernissage::Session::ExperimentInfo experimentInfo;
	Vernissage::Session::SpatialInfo spatialInfo;
	Vernissage::Session::BrickletMetaData brickletMetaData; 
	
	std::string allAxesAsOneString;
	Vernissage::Session::AxisDescriptor axisDescriptor;
	Vernissage::Session::AxisTableSets axisTableSetsMap;
	std::wstring axisNameWString;
	std::string axisNameString, baseName, viewTypeCodesAsOneString;
	int index;
	char buf[ARRAY_SIZE];

	std::vector<std::string> metaDataKeys, metaDataValues;
	metaDataKeys.reserve(METADATA_RESERVE_SIZE);
	metaDataValues.reserve(METADATA_RESERVE_SIZE);

	// timestamp of creation
	tm ctime = m_VernissageSession->getCreationTimestamp(m_brickletPtr);
	metaDataKeys.push_back("creationTimeStamp");
	metaDataValues.push_back(anyTypeToString<time_t>(mktime(&ctime)));

	// viewTypeCodes
	m_viewTypeCodes = m_VernissageSession->getViewTypes(m_brickletPtr);
	std::vector<Vernissage::Session::ViewTypeCode>::const_iterator itViewTypeCode;
	for(itViewTypeCode = m_viewTypeCodes.begin(); itViewTypeCode != m_viewTypeCodes.end(); itViewTypeCode++){
		viewTypeCodesAsOneString.append( viewTypeCodeToString(*itViewTypeCode) + listSepChar);
	}
	metaDataKeys.push_back("viewTypeCodes");
	metaDataValues.push_back(viewTypeCodesAsOneString);

	metaDataKeys.push_back(BRICKLET_ID_KEY);
	metaDataValues.push_back(anyTypeToString<int>(m_brickletID));

	// resultfile name
	metaDataKeys.push_back(RESULT_FILE_NAME_KEY);
	metaDataValues.push_back(GlobalData::Instance().getFileName());

	// resultfile path
	metaDataKeys.push_back(RESULT_DIR_PATH_KEY);
	metaDataValues.push_back(GlobalData::Instance().getDirPath());

	// introduced with Vernissage 2.0
	metaDataKeys.push_back("sampleName");
	metaDataValues.push_back(WStringToString(m_VernissageSession->getSampleName(m_brickletPtr)));

	// dito
	metaDataKeys.push_back("dataSetName");
	metaDataValues.push_back(WStringToString(m_VernissageSession->getDataSetName(m_brickletPtr)));

	// dito
	// datacomment is a vector, each entry is from one call to writeDataComment
	std::vector<std::wstring> dataComments = m_VernissageSession->getDataComments(m_brickletPtr);

	// we also write out the number of data comments for convenient access
	metaDataKeys.push_back("dataComment.count");
	metaDataValues.push_back(anyTypeToString(dataComments.size()));

	for(unsigned int i = 0; i < dataComments.size(); i++){
		metaDataKeys.push_back("dataCommentNo" + anyTypeToString(i+1));
		metaDataValues.push_back(WStringToString(dataComments[i]));
	}

	// BEGIN m_VernissageSession->getBrickletMetaData
	brickletMetaData = m_VernissageSession->getMetaData(m_brickletPtr);
	metaDataKeys.push_back("brickletMetaData.fileCreatorName");
	metaDataValues.push_back(WStringToString(brickletMetaData.fileCreatorName));

	metaDataKeys.push_back("brickletMetaData.fileCreatorVersion");
	metaDataValues.push_back(WStringToString(brickletMetaData.fileCreatorVersion));

	metaDataKeys.push_back("brickletMetaData.accountName");
	metaDataValues.push_back(WStringToString(brickletMetaData.accountName));

	metaDataKeys.push_back("brickletMetaData.userName");
	metaDataValues.push_back(WStringToString(brickletMetaData.userName));
	// END m_VernissageSession->getBrickletMetaData 

	metaDataKeys.push_back("sequenceID");
	metaDataValues.push_back(anyTypeToString<int>(m_VernissageSession->getSequenceId(m_brickletPtr)));

	metaDataKeys.push_back("creationComment");
	metaDataValues.push_back(WStringToString(m_VernissageSession->getCreationComment(m_brickletPtr)));

	metaDataKeys.push_back("dimension");
	metaDataValues.push_back(anyTypeToString<int>(m_VernissageSession->getDimensionCount(m_brickletPtr)));

	metaDataKeys.push_back("rootAxis");
	metaDataValues.push_back(WStringToString(m_VernissageSession->getRootAxisName(m_brickletPtr)));

	metaDataKeys.push_back("triggerAxis");
	metaDataValues.push_back(WStringToString(m_VernissageSession->getTriggerAxisName(m_brickletPtr)));

	metaDataKeys.push_back("channelName");
	metaDataValues.push_back(WStringToString(m_VernissageSession->getChannelName(m_brickletPtr)));

	metaDataKeys.push_back("channelInstanceName");
	metaDataValues.push_back(WStringToString(m_VernissageSession->getChannelInstanceName(m_brickletPtr)));

	metaDataKeys.push_back(CHANNEL_UNIT_KEY);
	metaDataValues.push_back(WStringToString(m_VernissageSession->getChannelUnit(m_brickletPtr)));

	metaDataKeys.push_back("runCycleCount");
	metaDataValues.push_back(anyTypeToString<int>(m_VernissageSession->getRunCycleCount(m_brickletPtr)));

	metaDataKeys.push_back("scanCycleCount");
	metaDataValues.push_back(anyTypeToString<int>(m_VernissageSession->getScanCycleCount(m_brickletPtr)));

	elementInstanceNames = m_VernissageSession->getExperimentElementInstanceNames(m_brickletPtr,L"");

	std::vector<std::wstring>::iterator itElementInstanceNames;
	std::map<std::wstring, Vernissage::Session::Parameter>::iterator itMap;

	for(itElementInstanceNames = elementInstanceNames.begin(); itElementInstanceNames != elementInstanceNames.end(); itElementInstanceNames++){

		elementInstanceParamsMap = m_VernissageSession->getExperimentElementParameters(m_brickletPtr,*itElementInstanceNames);

		for(itMap = elementInstanceParamsMap.begin(); itMap != elementInstanceParamsMap.end(); itMap++){

			//metaDataKeys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first));
			//metaDataValues.push_back("");

			metaDataKeys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first) + std::string(".value") );
			metaDataValues.push_back( WStringToString(itMap->second.value));

			metaDataKeys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first) + std::string(".unit") );
			metaDataValues.push_back( WStringToString(itMap->second.unit));

			//metaDataKeys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first) + std::string(".valueType") );
			//metaDataValues.push_back( valueTypeToString(itMap->second.valueType) );
		}
	}

	// BEGIN Vernissage::Session::SpatialInfo
	spatialInfo = m_VernissageSession->getSpatialInfo(m_brickletPtr);

	std::vector<double>::const_iterator it;
	for( it = spatialInfo.physicalX.begin(); it != spatialInfo.physicalX.end(); it++){
		index = it - spatialInfo.physicalX.begin() + 1 ; // one-based Index
		sprintf(buf,"spatialInfo.physicalX.No%d",index);
		metaDataKeys.push_back(buf);
		metaDataValues.push_back(anyTypeToString<double>(*it));
	}

	for( it = spatialInfo.physicalY.begin(); it != spatialInfo.physicalY.end(); it++){
		index = it - spatialInfo.physicalY.begin() + 1 ; // one-based Index
		sprintf(buf,"spatialInfo.physicalY.No%d",index);
		metaDataKeys.push_back(buf);
		metaDataValues.push_back(anyTypeToString<double>(*it));
	}

	metaDataKeys.push_back("spatialInfo.originatorKnown");
	metaDataValues.push_back(spatialInfo.originatorKnown ? "true" : "false");

	metaDataKeys.push_back("spatialInfo.channelName");
	if(spatialInfo.originatorKnown){
		metaDataValues.push_back(WStringToString(spatialInfo.channelName));
	}else{
		metaDataValues.push_back("");
	}

	metaDataKeys.push_back("spatialInfo.sequenceId");
	if(spatialInfo.originatorKnown){
		metaDataValues.push_back(anyTypeToString<int>(spatialInfo.sequenceId));
	}else{
		metaDataValues.push_back("");
	}

	metaDataKeys.push_back("spatialInfo.runCycleCount");
	if(spatialInfo.originatorKnown){
		metaDataValues.push_back(anyTypeToString<int>(spatialInfo.runCycleCount));
	}else{
		metaDataValues.push_back("");
	}

	metaDataKeys.push_back("spatialInfo.scanCycleCount");
	if(spatialInfo.originatorKnown){
		metaDataValues.push_back(anyTypeToString<int>(spatialInfo.scanCycleCount));
	}else{
		metaDataValues.push_back("");
	}

	metaDataKeys.push_back("spatialInfo.viewName");
	if(spatialInfo.originatorKnown){
		metaDataValues.push_back(WStringToString(spatialInfo.viewName));
	}else{
		metaDataValues.push_back("");
	}

	metaDataKeys.push_back("spatialInfo.viewSelectionId");
	if(spatialInfo.originatorKnown){
		metaDataValues.push_back(anyTypeToString<int>(spatialInfo.viewSelectionId));
	}else{
		metaDataValues.push_back("");
	}

	metaDataKeys.push_back("spatialInfo.viewSelectionIndex");
	if(spatialInfo.originatorKnown){
		metaDataValues.push_back(anyTypeToString<int>(spatialInfo.viewSelectionIndex));		
	}else{
		metaDataValues.push_back("");
	}
	//END Vernissage::Session::SpatialInfo

	// BEGIN Vernissage::Session::ExperimentInfo
	experimentInfo = m_VernissageSession->getExperimentInfo(m_brickletPtr);

	metaDataKeys.push_back("experimentInfo.Name");
	metaDataValues.push_back(WStringToString(experimentInfo.experimentName));

	metaDataKeys.push_back("experimentInfo.Version");
	metaDataValues.push_back(WStringToString(experimentInfo.experimentVersion));

	metaDataKeys.push_back("experimentInfo.Description");
	metaDataValues.push_back(WStringToString(experimentInfo.experimentDescription));

	metaDataKeys.push_back("experimentInfo.FileSpec");
	metaDataValues.push_back(WStringToString(experimentInfo.experimentFileSpec));

	metaDataKeys.push_back("experimentInfo.projectName");
	metaDataValues.push_back(WStringToString(experimentInfo.projectName));

	metaDataKeys.push_back("experimentInfo.projectVersion");
	metaDataValues.push_back(WStringToString(experimentInfo.projectVersion));

	metaDataKeys.push_back("experimentInfo.projectFileSpec");
	metaDataValues.push_back(WStringToString(experimentInfo.projectFileSpec));
	// END Vernissage::Session::ExperimentInfo

	// create m_allAxes* internally
	getAxes();
	joinString(m_allAxesString,listSepChar,allAxesAsOneString);

	metaDataKeys.push_back("allAxes");
	metaDataValues.push_back(allAxesAsOneString);

	// BEGIN Vernissage::Session::axisDescriptor
	std::vector<std::wstring>::const_iterator itAllAxes;
	for(itAllAxes = m_allAxesWString.begin(); itAllAxes != m_allAxesWString.end(); itAllAxes++){
		axisNameWString = *itAllAxes;
		axisNameString  = WStringToString(*itAllAxes);

		axisDescriptor = m_VernissageSession->getAxisDescriptor(m_brickletPtr,axisNameWString);

		metaDataKeys.push_back(axisNameString + ".clocks");
		metaDataValues.push_back(anyTypeToString<int>(axisDescriptor.clocks));

		metaDataKeys.push_back(axisNameString + ".mirrored");
		metaDataValues.push_back((axisDescriptor.mirrored)? "true" : "false");

		metaDataKeys.push_back(axisNameString + ".physicalUnit");
		metaDataValues.push_back(WStringToString(axisDescriptor.physicalUnit));

		metaDataKeys.push_back(axisNameString + ".physicalIncrement");
		metaDataValues.push_back(anyTypeToString<double>(axisDescriptor.physicalIncrement));

		metaDataKeys.push_back(axisNameString + ".physicalStart");
		metaDataValues.push_back(anyTypeToString<double>(axisDescriptor.physicalStart));

		metaDataKeys.push_back(axisNameString + ".rawIncrement");
		metaDataValues.push_back(anyTypeToString<int>(axisDescriptor.rawIncrement));

		metaDataKeys.push_back(axisNameString + ".rawStart");
		metaDataValues.push_back(anyTypeToString<int>(axisDescriptor.rawStart));

		metaDataKeys.push_back(axisNameString + ".triggerAxisName");
		metaDataValues.push_back(WStringToString(axisDescriptor.triggerAxisName));
		// END Vernissage::Session::axisDescriptor

		// BEGIN Vernissage::Session:AxisTableSet
		axisTableSetsMap = m_VernissageSession->getAxisTableSets(m_brickletPtr,*itAllAxes);

		// if it is empty, we got the standard table set which is [start=1,step=1,stop=clocks]
		if(axisTableSetsMap.empty()){

			metaDataKeys.push_back( axisNameString + ".AxisTableSet.count");
			metaDataValues.push_back(anyTypeToString<int>(1));

			metaDataKeys.push_back( axisNameString + ".AxisTableSetNo1.axis");
			metaDataValues.push_back(std::string());

			metaDataKeys.push_back( axisNameString + ".AxisTableSetNo1.start");
			metaDataValues.push_back(anyTypeToString<int>(1));

			metaDataKeys.push_back( axisNameString + ".AxisTableSetNo1.step");
			metaDataValues.push_back(anyTypeToString<int>(1));

			metaDataKeys.push_back( axisNameString + ".AxisTableSetNo1.stop");
			metaDataValues.push_back(anyTypeToString<int>(axisDescriptor.clocks));
		}
		else{

			Vernissage::Session::AxisTableSets::const_iterator itAxisTabelSetsMap;
			Vernissage::Session::TableSet::const_iterator itAxisTableSetsMapStruct;

			index=0;
			for( itAxisTabelSetsMap = axisTableSetsMap.begin(); itAxisTabelSetsMap != axisTableSetsMap.end(); itAxisTabelSetsMap++ ){
				for(itAxisTableSetsMapStruct= itAxisTabelSetsMap->second.begin(); itAxisTableSetsMapStruct != itAxisTabelSetsMap->second.end(); itAxisTableSetsMapStruct++){

				index++; // 1-based index
				baseName = axisNameString + ".AxisTableSetNo" + anyTypeToString<int>(index) + ".axis";
				metaDataKeys.push_back(baseName);
				metaDataValues.push_back(WStringToString(itAxisTabelSetsMap->first));

				metaDataKeys.push_back(baseName + ".start");
				metaDataValues.push_back(anyTypeToString<int>(itAxisTableSetsMapStruct->start));

				metaDataKeys.push_back(baseName + ".step");
				metaDataValues.push_back(anyTypeToString<int>(itAxisTableSetsMapStruct->step));

				metaDataKeys.push_back(baseName + ".stop");
				metaDataValues.push_back(anyTypeToString<int>(itAxisTableSetsMapStruct->stop));
				}
			}

			metaDataKeys.push_back( axisNameString + ".AxisTableSet.count");
			metaDataValues.push_back(anyTypeToString<int>(index));

		}
		// END Vernissage::Session:AxisTableSet
	}

	sprintf(GlobalData::Instance().outputBuffer,"Loaded %d keys and %d values as brickletMetaData for bricklet %d",metaDataKeys.size(), metaDataValues.size(),m_brickletID);
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	if(metaDataKeys.size() != metaDataValues.size()){
		outputToHistory("BUG: key value lists don't have the same size, aborting");
		metaDataKeys.clear();
		metaDataValues.clear();
	}

	// ensure that we only use as much memory as we have to
	m_metaDataKeys.swap(metaDataKeys);
	m_metaDataValues.swap(metaDataValues);
}
	
// see Vernissage Manual page 20/21
// the idea here is to first get the root and triggerAxis for the Bricklet
// Then we know the starting point of the axis hierachy (rootAxis) and the endpoint (triggerAxis)
// In this way we can then traverse from the endpoint (triggerAxis) to the starting point (rootAxis) and record all axis names
// In more than 99% of the cases this routine will return one to three axes
// The value of maxRuns is strictly speaking wrong becase the Matrix Software supports an unlimited number of axes, but due to pragmativ and safe coding reasons this has ben set to 100.
// The returned list will have the entries "triggerAxisName;axisNameWhichTriggeredTheTriggerAxis;...;rootAxisName" 
void BrickletClass::generateAllAxesVector(){

	std::wstring axisName, rootAxis, triggerAxis;
	std::vector<std::wstring> allAxesWString;
	std::vector<std::string> allAxesString;

	int numRuns=0,maxRuns=100;

	rootAxis= m_VernissageSession->getRootAxisName(m_brickletPtr);
	triggerAxis = m_VernissageSession->getTriggerAxisName(m_brickletPtr);

	axisName = triggerAxis;
	allAxesWString.push_back(triggerAxis);
	allAxesString.push_back(WStringToString(triggerAxis));

	while(axisName != rootAxis){
		axisName = m_VernissageSession->getAxisDescriptor(m_brickletPtr,axisName).triggerAxisName;
		allAxesWString.push_back(axisName);
		allAxesString.push_back(WStringToString(axisName));

		numRuns++;
		if(numRuns > maxRuns){
			outputToHistory("Found more than 100 axes in the axis hierachy. This is highly unlikely and therefore a bug in this XOP or in Vernissage.");
			break;
		}
	}
	m_allAxesWString = allAxesWString;
	m_allAxesString  = allAxesString;
}

/*
	returns a vector with the axis hierachy (std::wstring)
*/
const std::vector<std::wstring>& BrickletClass::getAxes(){

	if(m_allAxesString.empty() || m_allAxesWString.empty()){
		generateAllAxesVector();
	}
	return m_allAxesWString;
}

/*
	returns a vector with the axis hierachy (std::string)
*/
const std::vector<std::string>& BrickletClass::getAxesString(){

	if(m_allAxesString.empty() || m_allAxesWString.empty()){
		generateAllAxesVector();
	}
	return m_allAxesString;
}

/*
	Wrapper function which returns a metadata value of type int for a given key
*/
int	BrickletClass::getMetaDataValueAsInt(const std::string &key){
	return stringToAnyType<int>(this->getMetaDataValueAsString(key));
};

/*
	Wrapper function which returns a metadata value of type double for a given key
*/
double BrickletClass::getMetaDataValueAsDouble(const std::string &key){
	return stringToAnyType<double>(this->getMetaDataValueAsString(key));
};

/*
	Return a given metadata value as string
*/
std::string BrickletClass::getMetaDataValueAsString(const std::string &key){

	std::string value;

	if(key.empty()){
		outputToHistory("BUG: getMetaDataValueAsString called with empty parameter");
		return value;
	}

	if(m_metaDataKeys.empty() || m_metaDataValues.empty()){
		loadBrickletMetaDataFromResultFile();
	}

	for(unsigned int i=0; i < m_metaDataKeys.size(); i++){
		if( m_metaDataKeys[i] == key ){
			value = m_metaDataValues[i];
			break;
		}
	}
	return value;
}
