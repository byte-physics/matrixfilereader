
#include "header.h"

#include "BrickletClass.h"

#include "globaldata.h"
#include "utils.h"

BrickletClass::BrickletClass(void* pBricklet,int brickletID):m_brickletPtr(pBricklet),m_rawBufferContents(NULL),m_brickletID(brickletID){

	m_VernissageSession = globDataPtr->getVernissageSession();
	ASSERT_RETURN_VOID(m_VernissageSession);
}

BrickletClass::~BrickletClass(void){
	this->clearCache();
}

void BrickletClass::clearCache(void){

	if(m_rawBufferContents != NULL){
		sprintf(globDataPtr->outputBuffer,"Deleting raw data from bricklet %d",m_brickletID);
		debugOutputToHistory(globDataPtr->outputBuffer);
		delete[] m_rawBufferContents;
		m_rawBufferContents=NULL;
		m_rawBufferContentsSize=0;
	}

	// resize to zero elements, clear() would only delete them, but not reduce the memory consumption
	m_metaDataKeys.resize(0);
	m_metaDataValues.resize(0);
}

void BrickletClass::getBrickletContentsBuffer(const int** pBuffer, int &count){

	ASSERT_RETURN_VOID(pBuffer);
	ASSERT_RETURN_VOID(m_VernissageSession);
	count=0;

	// we are not called the first time
	if(m_rawBufferContents != NULL){
		debugOutputToHistory("GlobalData::getBrickletContentsBuffer Using cached values");

		sprintf(globDataPtr->outputBuffer,"before: pBuffer=%d,count=%d",*pBuffer,count);
		debugOutputToHistory(globDataPtr->outputBuffer);

		*pBuffer = m_rawBufferContents;
		count    = m_rawBufferContentsSize;

		sprintf(globDataPtr->outputBuffer,"after: pBuffer=%d,count=%d",*pBuffer,count);
		debugOutputToHistory(globDataPtr->outputBuffer);
	}
	else{ // we are called the first time

		try{
			// load raw data from vernissage DLL
			m_VernissageSession->loadBrickletContents(m_brickletPtr,pBuffer,count);
		}
		catch(...){
			sprintf(globDataPtr->outputBuffer,"Out of memory in getBrickletContentsBuffer() with bricklet %d",m_brickletID);
			outputToHistory(globDataPtr->outputBuffer);
			*pBuffer = NULL;
			count = 0;
			return;	
		}

		if(*pBuffer == NULL || count == 0){
			sprintf(globDataPtr->outputBuffer,"Out of memory in getBrickletContentsBuffer() with bricklet %d",m_brickletID);
			outputToHistory(globDataPtr->outputBuffer);
			m_VernissageSession->unloadBrickletContents(m_brickletPtr);
			*pBuffer = NULL;
			count=0;
			return;
		}

		sprintf(globDataPtr->outputBuffer,"pBuffer=%d,count=%d",*pBuffer,count);
		debugOutputToHistory(globDataPtr->outputBuffer);

		// these two lines have to be surrounded by loadbrickletContents/unloadBrickletContents, otherwise the getRaw* routines will be called by
		// loadbrickletContents implicitly which is quite expensive
		m_minRawValue = m_VernissageSession->getRawMin(m_brickletPtr);
		m_maxRawValue = m_VernissageSession->getRawMax(m_brickletPtr);

		m_minScaledValue = m_VernissageSession->toPhysical(m_minRawValue, m_brickletPtr);
		m_maxScaledValue = m_VernissageSession->toPhysical(m_maxRawValue, m_brickletPtr);

		sprintf(globDataPtr->outputBuffer,"rawMin=%d,rawMax=%d,scaledMin=%g,scaledMax=%g",m_minRawValue,m_maxRawValue,m_minScaledValue,m_maxScaledValue);
		debugOutputToHistory(globDataPtr->outputBuffer);

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

void BrickletClass::getBrickletMetaData(std::vector<std::string> &keys, std::vector<std::string> &values){

	if(m_metaDataKeys.size() == 0 ||  m_metaDataValues.size() == 0){

		m_metaDataKeys.reserve(METADATA_RESERVE_SIZE);
		m_metaDataValues.reserve(METADATA_RESERVE_SIZE);
		loadBrickletMetaDataFromResultFile();
		m_metaDataKeys.resize(m_metaDataKeys.size());
		m_metaDataValues.resize(m_metaDataValues.size());
	}

	keys = m_metaDataKeys;
	values = m_metaDataValues;
}

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
	std::vector<std::wstring> allAxesAsWString;
	std::string axisNameString, baseName, viewTypeCodesAsOneString;
	int index;
	char buf[ARRAY_SIZE];

	elementInstanceNames.clear();
	m_allAxes.clear();
	allAxesAsOneString.clear();
	m_metaDataKeys.clear();
	m_metaDataValues.clear();

	// timestamp of creation
	tm ctime = m_VernissageSession->getCreationTimestamp(m_brickletPtr);
	m_metaDataKeys.push_back("creationTimeStamp");
	m_metaDataValues.push_back(anyTypeToString<time_t>(mktime(&ctime)));

	// viewTypeCodes
	m_viewTypeCodes = m_VernissageSession->getViewTypes(m_brickletPtr);
	std::vector<Vernissage::Session::ViewTypeCode>::const_iterator itViewTypeCode;
	for(itViewTypeCode = m_viewTypeCodes.begin(); itViewTypeCode != m_viewTypeCodes.end(); itViewTypeCode++){
		viewTypeCodesAsOneString.append( viewTypeCodeToString(*itViewTypeCode) + ";");
	}
	m_metaDataKeys.push_back("viewTypeCodes");
	m_metaDataValues.push_back(viewTypeCodesAsOneString);

	m_metaDataKeys.push_back("brickletID");
	m_metaDataValues.push_back(anyTypeToString<int>(m_brickletID));

	// resultfile name
	m_metaDataKeys.push_back("resultFileName");
	m_metaDataValues.push_back(globDataPtr->getFileName());

	// resultfile path
	m_metaDataKeys.push_back("resultDirPath");
	m_metaDataValues.push_back(globDataPtr->getDirPath());

	// introduced with Vernissage 2.0
	m_metaDataKeys.push_back("sampleName");
	m_metaDataValues.push_back(WStringToString(m_VernissageSession->getSampleName(m_brickletPtr)));

	// dito
	m_metaDataKeys.push_back("dataSetName");
	m_metaDataValues.push_back(WStringToString(m_VernissageSession->getDataSetName(m_brickletPtr)));

	// dito
	std::vector<std::wstring> dataComments = m_VernissageSession->getDataComments(m_brickletPtr);

	for(unsigned int i = 0; i < dataComments.size(); i++){
		m_metaDataKeys.push_back("dataCommentNo" + anyTypeToString(i+1));
		m_metaDataValues.push_back(WStringToString(dataComments[i]));
	}

	// BEGIN m_VernissageSession->getBrickletMetaData
	brickletMetaData = m_VernissageSession->getMetaData(m_brickletPtr);
	m_metaDataKeys.push_back("brickletMetaData.fileCreatorName");
	m_metaDataValues.push_back(WStringToString(brickletMetaData.fileCreatorName));

	m_metaDataKeys.push_back("brickletMetaData.fileCreatorVersion");
	m_metaDataValues.push_back(WStringToString(brickletMetaData.fileCreatorVersion));

	m_metaDataKeys.push_back("brickletMetaData.accountName");
	m_metaDataValues.push_back(WStringToString(brickletMetaData.accountName));

	m_metaDataKeys.push_back("brickletMetaData.userName");
	m_metaDataValues.push_back(WStringToString(brickletMetaData.userName));
	// END m_VernissageSession->getBrickletMetaData 

	m_metaDataKeys.push_back("sequenceID");
	m_metaDataValues.push_back(anyTypeToString<int>(m_VernissageSession->getSequenceId(m_brickletPtr)));

	m_metaDataKeys.push_back("creationComment");
	m_metaDataValues.push_back(WStringToString(m_VernissageSession->getCreationComment(m_brickletPtr)));

	m_metaDataKeys.push_back("dimension");
	m_metaDataValues.push_back(anyTypeToString<int>(m_VernissageSession->getDimensionCount(m_brickletPtr)));

	m_metaDataKeys.push_back("rootAxis");
	m_metaDataValues.push_back(WStringToString(m_VernissageSession->getRootAxisName(m_brickletPtr)));

	m_metaDataKeys.push_back("triggerAxis");
	m_metaDataValues.push_back(WStringToString(m_VernissageSession->getTriggerAxisName(m_brickletPtr)));

	m_metaDataKeys.push_back("channelName");
	m_metaDataValues.push_back(WStringToString(m_VernissageSession->getChannelName(m_brickletPtr)));

	m_metaDataKeys.push_back("channelInstanceName");
	m_metaDataValues.push_back(WStringToString(m_VernissageSession->getChannelInstanceName(m_brickletPtr)));

	m_metaDataKeys.push_back("channelUnit");
	m_metaDataValues.push_back(WStringToString(m_VernissageSession->getChannelUnit(m_brickletPtr)));

	m_metaDataKeys.push_back("runCycleCount");
	m_metaDataValues.push_back(anyTypeToString<int>(m_VernissageSession->getRunCycleCount(m_brickletPtr)));

	m_metaDataKeys.push_back("scanCycleCount");
	m_metaDataValues.push_back(anyTypeToString<int>(m_VernissageSession->getScanCycleCount(m_brickletPtr)));

	elementInstanceNames = m_VernissageSession->getExperimentElementInstanceNames(m_brickletPtr,L"");

	std::vector<std::wstring>::iterator itElementInstanceNames;
	std::map<std::wstring, Vernissage::Session::Parameter>::iterator itMap;

	for(itElementInstanceNames = elementInstanceNames.begin(); itElementInstanceNames != elementInstanceNames.end(); itElementInstanceNames++){

		elementInstanceParamsMap = m_VernissageSession->getExperimentElementParameters(m_brickletPtr,*itElementInstanceNames);

		for(itMap = elementInstanceParamsMap.begin(); itMap != elementInstanceParamsMap.end(); itMap++){

			//m_metaDataKeys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first));
			//m_metaDataValues.push_back("");

			m_metaDataKeys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first) + std::string(".value") );
			m_metaDataValues.push_back( WStringToString(itMap->second.value));

			m_metaDataKeys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first) + std::string(".unit") );
			m_metaDataValues.push_back( WStringToString(itMap->second.unit));

			//m_metaDataKeys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first) + std::string(".valueType") );
			//m_metaDataValues.push_back( valueTypeToString(itMap->second.valueType) );
		}
	}

	// BEGIN Vernissage::Session::SpatialInfo
	spatialInfo = m_VernissageSession->getSpatialInfo(m_brickletPtr);

	std::vector<double>::const_iterator it;
	for( it = spatialInfo.physicalX.begin(); it != spatialInfo.physicalX.end(); it++){
		index = it - spatialInfo.physicalX.begin() + 1 ; // one-based Index
		sprintf(buf,"spatialInfo.physicalX.No%d",index);
		m_metaDataKeys.push_back(buf);
		m_metaDataValues.push_back(anyTypeToString<double>(*it));
	}

	for( it = spatialInfo.physicalY.begin(); it != spatialInfo.physicalY.end(); it++){
		index = it - spatialInfo.physicalY.begin() + 1 ; // one-based Index
		sprintf(buf,"spatialInfo.physicalY.No%d",index);
		m_metaDataKeys.push_back(buf);
		m_metaDataValues.push_back(anyTypeToString<double>(*it));
	}

	m_metaDataKeys.push_back("spatialInfo.originatorKnown");
	m_metaDataValues.push_back(spatialInfo.originatorKnown ? "true" : "false");

	m_metaDataKeys.push_back("spatialInfo.channelName");
	if(spatialInfo.originatorKnown){
		m_metaDataValues.push_back(WStringToString(spatialInfo.channelName));
	}else{
		m_metaDataValues.push_back("");
	}

	m_metaDataKeys.push_back("spatialInfo.sequenceId");
	if(spatialInfo.originatorKnown){
		m_metaDataValues.push_back(anyTypeToString<int>(spatialInfo.sequenceId));
	}else{
		m_metaDataValues.push_back("");
	}

	m_metaDataKeys.push_back("spatialInfo.runCycleCount");
	if(spatialInfo.originatorKnown){
		m_metaDataValues.push_back(anyTypeToString<int>(spatialInfo.runCycleCount));
	}else{
		m_metaDataValues.push_back("");
	}

	m_metaDataKeys.push_back("spatialInfo.scanCycleCount");
	if(spatialInfo.originatorKnown){
		m_metaDataValues.push_back(anyTypeToString<int>(spatialInfo.scanCycleCount));
	}else{
		m_metaDataValues.push_back("");
	}

	m_metaDataKeys.push_back("spatialInfo.viewName");
	if(spatialInfo.originatorKnown){
		m_metaDataValues.push_back(WStringToString(spatialInfo.viewName));
	}else{
		m_metaDataValues.push_back("");
	}

	m_metaDataKeys.push_back("spatialInfo.viewSelectionId");
	if(spatialInfo.originatorKnown){
		m_metaDataValues.push_back(anyTypeToString<int>(spatialInfo.viewSelectionId));
	}else{
		m_metaDataValues.push_back("");
	}

	m_metaDataKeys.push_back("spatialInfo.viewSelectionIndex");
	if(spatialInfo.originatorKnown){
		m_metaDataValues.push_back(anyTypeToString<int>(spatialInfo.viewSelectionIndex));		
	}else{
		m_metaDataValues.push_back("");
	}
	//END Vernissage::Session::SpatialInfo

	// BEGIN Vernissage::Session::ExperimentInfo
	experimentInfo = m_VernissageSession->getExperimentInfo(m_brickletPtr);

	m_metaDataKeys.push_back("experimentInfo.Name");
	m_metaDataValues.push_back(WStringToString(experimentInfo.experimentName));

	m_metaDataKeys.push_back("experimentInfo.Version");
	m_metaDataValues.push_back(WStringToString(experimentInfo.experimentVersion));

	m_metaDataKeys.push_back("experimentInfo.Description");
	m_metaDataValues.push_back(WStringToString(experimentInfo.experimentDescription));

	m_metaDataKeys.push_back("experimentInfo.FileSpec");
	m_metaDataValues.push_back(WStringToString(experimentInfo.experimentFileSpec));

	m_metaDataKeys.push_back("experimentInfo.projectName");
	m_metaDataValues.push_back(WStringToString(experimentInfo.projectName));

	m_metaDataKeys.push_back("experimentInfo.projectVersion");
	m_metaDataValues.push_back(WStringToString(experimentInfo.projectVersion));

	m_metaDataKeys.push_back("experimentInfo.projectFileSpec");
	m_metaDataValues.push_back(WStringToString(experimentInfo.projectFileSpec));
	// END Vernissage::Session::ExperimentInfo

	// store a list of all axes
	allAxesAsWString = generateAllAxesVector();
	m_metaDataKeys.push_back("allAxes");

	std::vector<std::wstring>::const_iterator itAllAxes;
	for(itAllAxes = allAxesAsWString.begin(); itAllAxes != allAxesAsWString.end(); itAllAxes++){
		allAxesAsOneString.append(WStringToString(*itAllAxes) + ";");
		m_allAxes.push_back(WStringToString(*itAllAxes));
	}
	m_metaDataValues.push_back(allAxesAsOneString);

	// BEGIN Vernissage::Session::axisDescriptor
	for(itAllAxes = allAxesAsWString.begin(); itAllAxes != allAxesAsWString.end(); itAllAxes++){
		axisNameWString = *itAllAxes;
		axisNameString  = WStringToString(*itAllAxes);

		axisDescriptor = m_VernissageSession->getAxisDescriptor(m_brickletPtr,axisNameWString);

		m_metaDataKeys.push_back(axisNameString + ".clocks");
		m_metaDataValues.push_back(anyTypeToString<int>(axisDescriptor.clocks));

		m_metaDataKeys.push_back(axisNameString + ".mirrored");
		m_metaDataValues.push_back((axisDescriptor.mirrored)? "true" : "false");

		m_metaDataKeys.push_back(axisNameString + ".physicalUnit");
		m_metaDataValues.push_back(WStringToString(axisDescriptor.physicalUnit));

		m_metaDataKeys.push_back(axisNameString + ".physicalIncrement");
		m_metaDataValues.push_back(anyTypeToString<double>(axisDescriptor.physicalIncrement));

		m_metaDataKeys.push_back(axisNameString + ".physicalStart");
		m_metaDataValues.push_back(anyTypeToString<double>(axisDescriptor.physicalStart));

		m_metaDataKeys.push_back(axisNameString + ".rawIncrement");
		m_metaDataValues.push_back(anyTypeToString<int>(axisDescriptor.rawIncrement));

		m_metaDataKeys.push_back(axisNameString + ".rawStart");
		m_metaDataValues.push_back(anyTypeToString<int>(axisDescriptor.rawStart));

		m_metaDataKeys.push_back(axisNameString + ".triggerAxisName");
		m_metaDataValues.push_back(WStringToString(axisDescriptor.triggerAxisName));
		// END Vernissage::Session::axisDescriptor

		// BEGIN Vernissage::Session:AxisTableSet
		axisTableSetsMap = m_VernissageSession->getAxisTableSets(m_brickletPtr,*itAllAxes);

		// if it is empty, we got the standard table set which is [start=1,step=1,stop=clocks]
		if(axisTableSetsMap.size() == 0){

			m_metaDataKeys.push_back( axisNameString + ".AxisTableSet.count");
			m_metaDataValues.push_back(anyTypeToString<int>(1));

			m_metaDataKeys.push_back( axisNameString + ".AxisTableSetNo1.axis");
			m_metaDataValues.push_back(std::string());

			m_metaDataKeys.push_back( axisNameString + ".AxisTableSetNo1.start");
			m_metaDataValues.push_back(anyTypeToString<int>(1));

			m_metaDataKeys.push_back( axisNameString + ".AxisTableSetNo1.step");
			m_metaDataValues.push_back(anyTypeToString<int>(1));

			m_metaDataKeys.push_back( axisNameString + ".AxisTableSetNo1.stop");
			m_metaDataValues.push_back(anyTypeToString<int>(axisDescriptor.clocks));
		}
		else{

			Vernissage::Session::AxisTableSets::const_iterator itAxisTabelSetsMap;
			Vernissage::Session::TableSet::const_iterator itAxisTableSetsMapStruct;

			index=0;
			for( itAxisTabelSetsMap = axisTableSetsMap.begin(); itAxisTabelSetsMap != axisTableSetsMap.end(); itAxisTabelSetsMap++ ){
				for(itAxisTableSetsMapStruct= itAxisTabelSetsMap->second.begin(); itAxisTableSetsMapStruct != itAxisTabelSetsMap->second.end(); itAxisTableSetsMapStruct++){

				index++; // 1-based index
				baseName = axisNameString + ".AxisTableSetNo" + anyTypeToString<int>(index) + ".axis";
				m_metaDataKeys.push_back(baseName);
				m_metaDataValues.push_back(WStringToString(itAxisTabelSetsMap->first));

				m_metaDataKeys.push_back(baseName + ".start");
				m_metaDataValues.push_back(anyTypeToString<int>(itAxisTableSetsMapStruct->start));

				m_metaDataKeys.push_back(baseName + ".step");
				m_metaDataValues.push_back(anyTypeToString<int>(itAxisTableSetsMapStruct->step));

				m_metaDataKeys.push_back(baseName + ".stop");
				m_metaDataValues.push_back(anyTypeToString<int>(itAxisTableSetsMapStruct->stop));
				}
			}

			m_metaDataKeys.push_back( axisNameString + ".AxisTableSet.count");
			m_metaDataValues.push_back(anyTypeToString<int>(index));

		}
		// END Vernissage::Session:AxisTableSet
	}

	sprintf(globDataPtr->outputBuffer,"Loaded %d keys and %d values as brickletMetaData",m_metaDataKeys.size(), m_metaDataValues.size());
	debugOutputToHistory(globDataPtr->outputBuffer);

	if(m_metaDataKeys.size() != m_metaDataValues.size()){
		outputToHistory("BUG: key value lists don't have the same size, aborting");
		m_metaDataKeys.clear();
		m_metaDataValues.clear();
	}
}
	
// see Vernissage Manual page 20/21
// the idea here is to first get the root and triggerAxis for the Bricklet
// Then we know the starting point of the axis hierachy (rootAxis) and the endpoint (triggerAxis)
// In this way we can then traverse from the endpoint (triggerAxis) to the starting point (rootAxis) and record all axis names
// In more than 99% of the cases this routine will return one to three axes
// The value of maxRuns is strictly speaking wrong becase the Matrix Software supports an unlimited number of axes, but due to pragmativ and safe coding reasons this has ben set to 100.
// The returned list will have the entries "triggerAxisName;axisNameWhichTriggeredTheTriggerAxis;...;rootAxisName" 
std::vector<std::wstring> BrickletClass::generateAllAxesVector(){

	std::wstring axisName, rootAxis, triggerAxis;
	std::vector<std::wstring> allAxes;

	int numRuns=0,maxRuns=100;

	rootAxis= m_VernissageSession->getRootAxisName(m_brickletPtr);
	triggerAxis = m_VernissageSession->getTriggerAxisName(m_brickletPtr);

	axisName = triggerAxis;
	allAxes.push_back(triggerAxis);

	while(axisName != rootAxis){
		axisName= m_VernissageSession->getAxisDescriptor(m_brickletPtr,axisName).triggerAxisName;
		allAxes.push_back(axisName);

		numRuns++;
		if(numRuns > maxRuns){
			outputToHistory("Found more than 100 axes in the axis hierachy. This is highly unlikely and therefore a bug in this XOP or in Vernissage.");
			break;
		}
	}

	return allAxes;
}

int	BrickletClass::getMetaDataValueAsInt(std::string key){
	return stringToAnyType<int>(this->getMetaDataValueAsString(key));
};

double BrickletClass::getMetaDataValueAsDouble(std::string key){
	return stringToAnyType<double>(this->getMetaDataValueAsString(key));
};


std::string BrickletClass::getMetaDataValueAsString(std::string key){

	std::string value;

	if(key.size() == 0){
		outputToHistory("BUG: getMetaDataValueAsString called with empty parameter");
		return value;
	}

	if(m_metaDataKeys.size() == 0 || m_metaDataValues.size() == 0){
		loadBrickletMetaDataFromResultFile();
	}

	for(unsigned int i=0; i < m_metaDataKeys.size(); i++){
		if( m_metaDataKeys[i] == key){
			value = m_metaDataValues[i];
			break;
		}
	}
	return value;
}
