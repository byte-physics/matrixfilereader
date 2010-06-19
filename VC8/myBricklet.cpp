#include "myBricklet.h"

#include "globalvariables.h"
#include <algorithm>
#include "utils.h"

#define DEBUG_LEVEL 1

MyBricklet::MyBricklet(void* pBricklet,Vernissage::Session *pSession):m_brickletPtr(pBricklet),m_rawBufferContents(NULL),m_VernissageSession(pSession)
{
	m_metaDataKeys.reserve(1000);
	m_metaDataValues.reserve(1000);
}

MyBricklet::~MyBricklet(void)
{
	if(m_VernissageSession){
		if(m_rawBufferContents != NULL){
			debugOutputToHistory(DEBUG_LEVEL,"Unloading Bricklet Contents");
			m_VernissageSession->unloadBrickletContents(m_brickletPtr);
			m_rawBufferContents=NULL;
		}
	}
}


void MyBricklet::getBrickletContentsBuffer(const int** pBuffer, int &count){

	char buf[ARRAY_SIZE];
	
	ASSERT_RETURN_VOID(pBuffer);
	
	count=0;

	// we are not called the first time
	if(m_rawBufferContents != NULL){
		debugOutputToHistory(DEBUG_LEVEL,"myData::getBrickletContentsBuffer Using cached values");

		sprintf(buf,"before: pBuffer=%d,count=%d",*pBuffer,count);
		debugOutputToHistory(DEBUG_LEVEL,buf);

		*pBuffer = m_rawBufferContents;
		count   = m_rawBufferContentsSize;

		sprintf(buf,"after: pBuffer=%d,count=%d",*pBuffer,count);
		debugOutputToHistory(DEBUG_LEVEL,buf);
	}
	else{ // we are called the first time
		m_VernissageSession->loadBrickletContents(m_brickletPtr,pBuffer,count);

		sprintf(buf,"pBuffer=%d,count=%d",*pBuffer,count);
		debugOutputToHistory(DEBUG_LEVEL,buf);

		m_rawBufferContents = *pBuffer;
		m_rawBufferContentsSize = count;
	}
}

void MyBricklet::getBrickletMetaData(std::vector<std::string> &keys, std::vector<std::string> &values){

	char buf[ARRAY_SIZE];

	if(m_metaDataKeys.size() == 0 ||  m_metaDataValues.size() == 0){

		loadBrickletMetaDataFromResultFile();

	}

	keys = m_metaDataKeys;
	values = m_metaDataValues;
}

void MyBricklet::loadBrickletMetaDataFromResultFile(){

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

	// resultfile name
	m_metaDataKeys.push_back("fileName");
	m_metaDataValues.push_back(pMyData->getResultFileName());

	// resultfile path
	m_metaDataKeys.push_back("filePath");
	m_metaDataValues.push_back(pMyData->getResultFilePath());

	// total number of bricklets
	m_metaDataKeys.push_back("totalNumberOfBricklets");
	m_metaDataValues.push_back(anyTypeToString<int>(m_VernissageSession->getBrickletCount()));

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

	m_dimension = m_VernissageSession->getDimensionCount(m_brickletPtr);
	m_metaDataKeys.push_back("dimension");
	m_metaDataValues.push_back(anyTypeToString<int>(m_dimension));

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

			m_metaDataKeys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first));
			m_metaDataValues.push_back("");

			m_metaDataKeys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first) + std::string(".value") );
			m_metaDataValues.push_back( WStringToString(itMap->second.value));

			m_metaDataKeys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first) + std::string(".unit") );
			m_metaDataValues.push_back( WStringToString(itMap->second.unit));

			m_metaDataKeys.push_back( WStringToString(*itElementInstanceNames) + std::string(".") + WStringToString(itMap->first) + std::string(".valueType") );
			m_metaDataValues.push_back( valueTypeToString(itMap->second.valueType) );
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

		// if it is empty, we got the standard table set which is [start=1,step=clocks,stop=1]
		if(axisTableSetsMap.size() == 0){

			m_metaDataKeys.push_back(axisNameString + ".axisTableSetNo1.axisName");
			m_metaDataValues.push_back(axisNameString);

			m_metaDataKeys.push_back(axisNameString + ".axisTableSetNo1.start");
			m_metaDataValues.push_back(anyTypeToString<int>(1));

			m_metaDataKeys.push_back(axisNameString + ".axisTableSetNo1.step");
			m_metaDataValues.push_back(anyTypeToString<int>(1));

			m_metaDataKeys.push_back(axisNameString + ".axisTableSetNo1.stop");
			m_metaDataValues.push_back(anyTypeToString<int>(axisDescriptor.clocks));
		}
		else{

			Vernissage::Session::AxisTableSets::const_iterator itAxisTabelSetsMap;
			Vernissage::Session::TableSet::const_iterator itAxisTableSetsMapStruct;
			index=0;
			for( itAxisTabelSetsMap = axisTableSetsMap.begin(); itAxisTabelSetsMap != axisTableSetsMap.end(); itAxisTabelSetsMap++ ){
				index++; // 1-based index
				baseName = axisNameString + "axisTableSetNo" + anyTypeToString<int>(index) + ".";
				m_metaDataKeys.push_back(baseName + axisNameString);
				m_metaDataValues.push_back(WStringToString(itAxisTabelSetsMap->first));

				for(itAxisTableSetsMapStruct= itAxisTabelSetsMap->second.begin(); itAxisTableSetsMapStruct != itAxisTabelSetsMap->second.end(); itAxisTableSetsMapStruct++){
					baseName = baseName + WStringToString(itAxisTabelSetsMap->first) + ".";
					m_metaDataKeys.push_back(baseName + "start");
					m_metaDataValues.push_back(anyTypeToString<int>(itAxisTableSetsMapStruct->start));

					m_metaDataKeys.push_back(baseName + "step");
					m_metaDataValues.push_back(anyTypeToString<int>(itAxisTableSetsMapStruct->step));

					m_metaDataKeys.push_back(baseName + "stop");
					m_metaDataValues.push_back(anyTypeToString<int>(itAxisTableSetsMapStruct->stop));
				}
			}
		}
		// END Vernissage::Session:AxisTableSet
	}

	sprintf(buf,"Loaded %d keys and %d values as brickletMetaData",m_metaDataKeys.size(), m_metaDataValues.size());
	debugOutputToHistory(DEBUG_LEVEL,buf);

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
// In more than 99% of the cases this routine will return one or two axes
// The value of maxRuns is strictly speaking wrong becase the Matrix Software supports an unlimited number of axes, but due to pragmativ and safe coding reasons this has ben set to 100.
// The returned list will have the entries "triggerAxisName;axisNameWhichTriggeredTheTriggerAxis;...;rootAxisName" 
std::vector<std::wstring> MyBricklet::generateAllAxesVector(){

	std::wstring axisName, rootAxis, triggerAxis;
	std::vector<std::wstring> allAxes;

	char buf[ARRAY_SIZE];

	int numRuns=0,maxRuns=100, index;

	rootAxis= m_VernissageSession->getRootAxisName(m_brickletPtr);
	triggerAxis = m_VernissageSession->getTriggerAxisName(m_brickletPtr);

	axisName = triggerAxis;
	allAxes.push_back(triggerAxis);

	while(axisName != rootAxis){
		axisName= m_VernissageSession->getAxisDescriptor(m_brickletPtr,axisName).triggerAxisName;
		allAxes.push_back(axisName);

		numRuns++;
		if(numRuns > maxRuns){
			outputToHistory("Found more than 100 axes in the axis hierachy. This is highliy unlikely and therefore a bug in this XOP or in Vernissage.");
			break;
		}
	}

	return allAxes;
}

int	MyBricklet::getMetaDataValueAsInt(std::string key){
	return stringToAnyType<int>(this->getMetaDataValueAsString(key));
};

double MyBricklet::getMetaDataValueAsDouble(std::string key){
	return stringToAnyType<double>(this->getMetaDataValueAsString(key));
};


std::string MyBricklet::getMetaDataValueAsString(std::string key){

	std::string value;
	char buf[ARRAY_SIZE];

	if(key.size() == 0){
		outputToHistory("BUG: getMetaDataValueAsString called with empty parameter");
		return value;
	}

	if(m_metaDataKeys.size() == 0 || m_metaDataValues.size() == 0){

		loadBrickletMetaDataFromResultFile();
	}

	for(int i=0; i < m_metaDataKeys.size(); i++){
		if( m_metaDataKeys[i] == key){
			value = m_metaDataValues[i];
			break;
		}
	}

	// crashes
	//std::vector<std::string>::iterator it = std::find(m_metaDataKeys.begin(), m_metaDataKeys.end(), key);
	//if(it != m_metaDataValues.end()){ // we found it
		//sprintf(buf,"column of key %s found",*it);
		//debugOutputToHistory(DEBUG_LEVEL,buf);
		//value = m_metaDataValues.at(it-m_metaDataKeys.begin());
	//}

	return value;
}
