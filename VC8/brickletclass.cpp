/*
  The file brickletclass.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt  in the source folder for details.
*/

#include "stdafx.h"

#include "brickletclass.hpp"
#include "globaldata.hpp"
#include "utils_bricklet.hpp"
#include "extremadata.hpp"
#include "utils_generic.hpp"

BrickletClass::BrickletClass(void* const pBricklet, int brickletID)
  :
  m_brickletPtr(pBricklet),
  m_rawBufferContents(NULL),
  m_brickletID(brickletID),
  m_VernissageSession(GlobalData::Instance().getVernissageSession()),
  m_extrema(ExtremaData())
{

  ASSERT_RETURN_VOID(m_VernissageSession);
}

BrickletClass::~BrickletClass()
{
  this->clearCache();
}

/*
  delete our internally cached data, is called at destruction time and if we want keep memory
  consumption low
*/
void BrickletClass::clearCache()
{

  if (m_rawBufferContents != NULL)
  {
    DEBUGPRINT("Deleting raw data from bricklet %d", m_brickletID);
    delete[] m_rawBufferContents;
    m_rawBufferContents = NULL;
    m_rawBufferContentsSize = 0;
  }

  // resize to zero elements
  std::vector<StringPair>().swap(m_metaData);
}

/*
  Load the raw data of the bricklet into our own cache
*/
void BrickletClass::getBrickletContentsBuffer(const int** pBuffer, int& count)
{

  ASSERT_RETURN_VOID(pBuffer);
  ASSERT_RETURN_VOID(m_VernissageSession);
  count = 0;

  // we are not called the first time
  if (m_rawBufferContents != NULL)
  {
    DEBUGPRINT("GlobalData::getBrickletContentsBuffer Using cached values");

    DEBUGPRINT("before: pBuffer=%d,count=%d", *pBuffer, count);

    *pBuffer = m_rawBufferContents;
    count    = m_rawBufferContentsSize;

    DEBUGPRINT("after: pBuffer=%d,count=%d", *pBuffer, count);
  }
  else  // we are called the first time
  {

    try
    {
      m_VernissageSession->loadBrickletContents(m_brickletPtr, pBuffer, count);
    }
    catch (...)
    {
      HISTPRINT("Could not load the bricklet contents, probably out of memory in getBrickletContentsBuffer() with bricklet %d", m_brickletID);
      *pBuffer = NULL;
      count = 0;
      return;
    }

    // loadBrickletContents either throws an exception or returns pBuffer == 0 || count == 0
    if (*pBuffer == NULL || count == 0)
    {
      HISTPRINT("Out of memory in getBrickletContentsBuffer() with bricklet %d", m_brickletID);
      m_VernissageSession->unloadBrickletContents(m_brickletPtr);
      *pBuffer = NULL;
      count = 0;
      return;
    }

    DEBUGPRINT("pBuffer=%d,count=%d", *pBuffer, count);

    // these two lines have to be surrounded by loadbrickletContents/unloadBrickletContents, otherwise loadbrickletContents will be called
    // implicitly which is quite expensive
    const int rawMin = m_VernissageSession->getRawMin(m_brickletPtr);
    m_extrema.setMinimum(rawMin, m_VernissageSession->toPhysical(rawMin, m_brickletPtr));

    const int rawMax = m_VernissageSession->getRawMax(m_brickletPtr);
    m_extrema.setMaximum(rawMax, m_VernissageSession->toPhysical(rawMax, m_brickletPtr));

    DEBUGPRINT("rawMin=%d,rawMax=%d,scaledMin=%g,scaledMax=%g",
               m_extrema.getRawMin(), m_extrema.getRawMax(), m_extrema.getPhysValRawMin(), m_extrema.getPhysValRawMax());

    // copy the raw data to our own cache
    m_rawBufferContentsSize = count;

    try
    {
      m_rawBufferContents = new int[m_rawBufferContentsSize];
    }
    catch (CMemoryException* e)
    {
      e->Delete();
      HISTPRINT("Out of memory in getBrickletContentsBuffer()");
      *pBuffer = NULL;
      count = 0;
      m_VernissageSession->unloadBrickletContents(m_brickletPtr);
      return;
    }

    memcpy(m_rawBufferContents, *pBuffer, sizeof(int)*m_rawBufferContentsSize);
    *pBuffer = m_rawBufferContents;

    // release memory from vernissage DLL
    m_VernissageSession->unloadBrickletContents(m_brickletPtr);
  }
}

/*
  Wrapper function which returns a vector of the meta data keys values
*/
const std::vector<BrickletClass::StringPair>& BrickletClass::getBrickletMetaData()
{
  if (m_metaData.empty())
  {
    try
    {
      loadBrickletMetaDataFromResultFile();
    }
    catch (CMemoryException* e)
    {
      e->Delete();
      HISTPRINT("Out of memory in getBrickletMetaDataValues()");
    }
  }
  return m_metaData;
}

/*
  Reads all meta data into our own structures
*/
void BrickletClass::loadBrickletMetaDataFromResultFile()
{
  std::vector<StringPair> metaData;
  metaData.reserve(METADATA_RESERVE_SIZE);

  // timestamp of creation
  tm ctime = m_VernissageSession->getCreationTimestamp(m_brickletPtr);
  metaData.push_back(std::make_pair("creationTimeStamp", anyTypeToString<time_t>(mktime(&ctime))));

  // viewTypeCodes
  m_viewTypeCodes = m_VernissageSession->getViewTypes(m_brickletPtr);
  std::vector<Vernissage::Session::ViewTypeCode>::const_iterator itViewTypeCode;

  std::string viewTypeCodesAsOneString;
  for (itViewTypeCode = m_viewTypeCodes.begin(); itViewTypeCode != m_viewTypeCodes.end(); itViewTypeCode++)
  {
    viewTypeCodesAsOneString.append(viewTypeCodeToString(*itViewTypeCode) + listSepChar);
  }

  metaData.push_back(std::make_pair("viewTypeCodes",viewTypeCodesAsOneString));
  metaData.push_back(std::make_pair(BRICKLET_ID_KEY,anyTypeToString<int>(m_brickletID)));

  // resultfile
  metaData.push_back(std::make_pair(RESULT_FILE_NAME_KEY,GlobalData::Instance().getFileName<std::string>()));
  metaData.push_back(std::make_pair(RESULT_DIR_PATH_KEY,GlobalData::Instance().getDirPath<std::string>()));

  // introduced with Vernissage 2.0
  metaData.push_back(std::make_pair("sampleName",WStringToString(m_VernissageSession->getSampleName(m_brickletPtr))));

  // dito
  metaData.push_back(std::make_pair("dataSetName",WStringToString(m_VernissageSession->getDataSetName(m_brickletPtr))));

  // dito
  // datacomment is a vector, each entry is from one call to writeDataComment
  std::vector<std::wstring> dataComments = m_VernissageSession->getDataComments(m_brickletPtr);

  // we also write out the number of data comments for convenient access
  metaData.push_back(std::make_pair("dataComment.count",anyTypeToString(dataComments.size())));

  for (unsigned int i = 0; i < dataComments.size(); i++)
  {
    const std::string key = "dataCommentNo" + anyTypeToString(i + 1);
    const std::string value = WStringToString(dataComments[i]);
    metaData.push_back(std::make_pair(key,value));
  }

  // introduced with vernissage 2.1
  metaData.push_back(std::make_pair("visualMateNode",WStringToString(m_VernissageSession->getVisualMateNode(m_brickletPtr))));
  metaData.push_back(std::make_pair("channelGroupName",WStringToString(m_VernissageSession->getChannelGroupName(m_brickletPtr))));

  // introduced with vernissage 2.0 (but forgotten until release 2.1)
  metaData.push_back(std::make_pair("brickletType",brickletTypeToString(m_VernissageSession->getType(m_brickletPtr))));

  // BEGIN m_VernissageSession->getBrickletMetaData
  const Vernissage::Session::BrickletMetaData brickletMetaData = m_VernissageSession->getMetaData(m_brickletPtr);
  metaData.push_back(std::make_pair("brickletMetaData.fileCreatorName",WStringToString(brickletMetaData.fileCreatorName)));
  metaData.push_back(std::make_pair("brickletMetaData.fileCreatorVersion",WStringToString(brickletMetaData.fileCreatorVersion)));
  metaData.push_back(std::make_pair("brickletMetaData.accountName",WStringToString(brickletMetaData.accountName)));
  metaData.push_back(std::make_pair("brickletMetaData.userName",WStringToString(brickletMetaData.userName)));
  // END m_VernissageSession->getBrickletMetaData

  metaData.push_back(std::make_pair("sequenceID",anyTypeToString<int>(m_VernissageSession->getSequenceId(m_brickletPtr))));
  metaData.push_back(std::make_pair("creationComment",WStringToString(m_VernissageSession->getCreationComment(m_brickletPtr))));
  metaData.push_back(std::make_pair("dimension",anyTypeToString<int>(m_VernissageSession->getDimensionCount(m_brickletPtr))));
  metaData.push_back(std::make_pair("rootAxis",WStringToString(m_VernissageSession->getRootAxisName(m_brickletPtr))));

  // new in vernissage 2.1
  metaData.push_back(std::make_pair("rootAxisQualified",WStringToString(m_VernissageSession->getRootAxisQualifiedName(m_brickletPtr))));
  metaData.push_back(std::make_pair("triggerAxis",WStringToString(m_VernissageSession->getTriggerAxisName(m_brickletPtr))));
  metaData.push_back(std::make_pair("triggerAxisQualified",WStringToString(m_VernissageSession->getTriggerAxisQualifiedName(m_brickletPtr))));

  metaData.push_back(std::make_pair("channelName",WStringToString(m_VernissageSession->getChannelName(m_brickletPtr))));
  metaData.push_back(std::make_pair("channelInstanceName",WStringToString(m_VernissageSession->getChannelInstanceName(m_brickletPtr))));

  metaData.push_back(std::make_pair(CHANNEL_UNIT_KEY,WStringToString(m_VernissageSession->getChannelUnit(m_brickletPtr))));
  metaData.push_back(std::make_pair("runCycleCount",anyTypeToString<int>(m_VernissageSession->getRunCycleCount(m_brickletPtr))));

  metaData.push_back(std::make_pair("scanCycleCount",anyTypeToString<int>(m_VernissageSession->getScanCycleCount(m_brickletPtr))));

  // new in vernissage 2.1
  {
    const std::vector<void*> dependentBrickletsVoidVector = m_VernissageSession->getDependingBricklets(m_brickletPtr);
    const std::vector<int> dependentBrickletsIntVector    = GlobalData::Instance().convertBrickletPtr(dependentBrickletsVoidVector);
    std::string dependentBricklets;
    joinString(dependentBrickletsIntVector, listSepChar, dependentBricklets);

    metaData.push_back(std::make_pair("dependentBricklets",dependentBricklets));
  }

  // new in vernissage 2.1
  {
    const std::vector<void*> referencedBrickletsVoidVector = m_VernissageSession->getReferencedBricklets(m_brickletPtr);
    const std::vector<int> referencedBrickletsIntVector    = GlobalData::Instance().convertBrickletPtr(referencedBrickletsVoidVector);
    std::string referencedBricklets;
    joinString(referencedBrickletsIntVector, listSepChar, referencedBricklets);

    metaData.push_back(std::make_pair("referencedBricklets",referencedBricklets));
  }

  // new in vernissage 2.1
  {
    const std::vector<void*> brickletSeriesVoidVector = GlobalData::Instance().getBrickletSeries(m_brickletPtr);
    std::vector<int> brickletSeriesIntVector    = GlobalData::Instance().convertBrickletPtr(brickletSeriesVoidVector);
    // we want to have a sorted list of bricklets
    std::sort(brickletSeriesIntVector.begin(), brickletSeriesIntVector.end());
    std::string brickletSeries;
    joinString(brickletSeriesIntVector, listSepChar, brickletSeries);

    metaData.push_back(std::make_pair("brickletSeries",brickletSeries));
  }

  const std::vector<std::wstring> elementInstanceNames = m_VernissageSession->getExperimentElementInstanceNames(m_brickletPtr, L"");
  for (std::vector<std::wstring>::const_iterator itElem = elementInstanceNames.begin(); itElem != elementInstanceNames.end(); itElem++)
  {
    typedef std::map<std::wstring, Vernissage::Session::Parameter> MapType;
    const MapType elementInstanceParamsMap = m_VernissageSession->getExperimentElementParameters(m_brickletPtr, *itElem);

    for (MapType::const_iterator itParam = elementInstanceParamsMap.begin(); itParam != elementInstanceParamsMap.end(); itParam++)
    {
      const std::string key = WStringToString(*itElem) + std::string(".") + WStringToString(itParam->first) ;
      metaData.push_back(std::make_pair(key + std::string(".value"),WStringToString(itParam->second.value)));
      metaData.push_back(std::make_pair(key + std::string(".unit"),WStringToString(itParam->second.unit)));
    }
  }

  // BEGIN Vernissage::Session::SpatialInfo
  const Vernissage::Session::SpatialInfo spatialInfo = m_VernissageSession->getSpatialInfo(m_brickletPtr);
  for (std::vector<double>::const_iterator it = spatialInfo.physicalX.begin(); it != spatialInfo.physicalX.end(); it++)
  {
    char buf[ARRAY_SIZE];
    int index = it - spatialInfo.physicalX.begin() + 1 ; // one-based Index
    sprintf(buf, "spatialInfo.physicalX.No%d", index);
    metaData.push_back(std::make_pair(buf,anyTypeToString<double>(*it)));
  }
  for (std::vector<double>::const_iterator it = spatialInfo.physicalY.begin(); it != spatialInfo.physicalY.end(); it++)
  {
    char buf[ARRAY_SIZE];
    int index = it - spatialInfo.physicalY.begin() + 1 ; // one-based Index
    sprintf(buf, "spatialInfo.physicalY.No%d", index);
    metaData.push_back(std::make_pair(buf,anyTypeToString<double>(*it)));
  }

  if (spatialInfo.originatorKnown)
  {
    metaData.push_back(std::make_pair("spatialInfo.originatorKnown","true"));
    metaData.push_back(std::make_pair("spatialInfo.channelName",WStringToString(spatialInfo.channelName)));
    metaData.push_back(std::make_pair("spatialInfo.sequenceId",anyTypeToString<int>(spatialInfo.sequenceId)));
    metaData.push_back(std::make_pair("spatialInfo.runCycleCount",anyTypeToString<int>(spatialInfo.runCycleCount)));
    metaData.push_back(std::make_pair("spatialInfo.scanCycleCount",anyTypeToString<int>(spatialInfo.scanCycleCount)));
    metaData.push_back(std::make_pair("spatialInfo.viewName",WStringToString(spatialInfo.viewName)));
    metaData.push_back(std::make_pair("spatialInfo.viewSelectionId",anyTypeToString<int>(spatialInfo.viewSelectionId)));
    metaData.push_back(std::make_pair("spatialInfo.viewSelectionIndex",anyTypeToString<int>(spatialInfo.viewSelectionIndex)));
  }
  else
  {
    metaData.push_back(std::make_pair("spatialInfo.originatorKnown","false"));
    metaData.push_back(std::make_pair("spatialInfo.channelName",""));
    metaData.push_back(std::make_pair("spatialInfo.sequenceId",""));
    metaData.push_back(std::make_pair("spatialInfo.runCycleCount",""));
    metaData.push_back(std::make_pair("spatialInfo.scanCycleCount",""));
    metaData.push_back(std::make_pair("spatialInfo.viewName",""));
    metaData.push_back(std::make_pair("spatialInfo.viewSelectionId",""));
    metaData.push_back(std::make_pair("spatialInfo.viewSelectionIndex",""));
  }
  //END Vernissage::Session::SpatialInfo

  // BEGIN Vernissage::Session::ExperimentInfo
  const Vernissage::Session::ExperimentInfo experimentInfo = m_VernissageSession->getExperimentInfo(m_brickletPtr);
  metaData.push_back(std::make_pair("experimentInfo.Name",WStringToString(experimentInfo.experimentName)));
  metaData.push_back(std::make_pair("experimentInfo.Version",WStringToString(experimentInfo.experimentVersion)));
  metaData.push_back(std::make_pair("experimentInfo.Description",WStringToString(experimentInfo.experimentDescription)));
  metaData.push_back(std::make_pair("experimentInfo.FileSpec",WStringToString(experimentInfo.experimentFileSpec)));
  metaData.push_back(std::make_pair("experimentInfo.projectName",WStringToString(experimentInfo.projectName)));
  metaData.push_back(std::make_pair("experimentInfo.projectVersion",WStringToString(experimentInfo.projectVersion)));
  metaData.push_back(std::make_pair("experimentInfo.projectFileSpec",WStringToString(experimentInfo.projectFileSpec)));
  // END Vernissage::Session::ExperimentInfo

  // create m_allAxes* internally
  getAxes<std::string>();
  std::string allAxesAsOneString;
  joinString(m_allAxesString, listSepChar, allAxesAsOneString);
  metaData.push_back(std::make_pair("allAxes",allAxesAsOneString));

  // BEGIN Vernissage::Session::axisDescriptor
  std::vector<std::wstring>::const_iterator itAllAxes;
  for (itAllAxes = m_allAxesWString.begin(); itAllAxes != m_allAxesWString.end(); itAllAxes++)
  {
    const std::wstring axisNameWString = *itAllAxes;
    const std::string axisNameString  = WStringToString(*itAllAxes);

    const Vernissage::Session::AxisDescriptor axisDescriptor = m_VernissageSession->getAxisDescriptor(m_brickletPtr, axisNameWString);
    metaData.push_back(std::make_pair(axisNameString + ".clocks",anyTypeToString<int>(axisDescriptor.clocks)));
    metaData.push_back(std::make_pair(axisNameString + ".mirrored",(axisDescriptor.mirrored ? "true" : "false")));
    metaData.push_back(std::make_pair(axisNameString + ".physicalUnit",WStringToString(axisDescriptor.physicalUnit)));
    metaData.push_back(std::make_pair(axisNameString + ".physicalIncrement",anyTypeToString<double>(axisDescriptor.physicalIncrement)));
    metaData.push_back(std::make_pair(axisNameString + ".physicalStart",anyTypeToString<double>(axisDescriptor.physicalStart)));
    metaData.push_back(std::make_pair(axisNameString + ".rawIncrement",anyTypeToString<int>(axisDescriptor.rawIncrement)));
    metaData.push_back(std::make_pair(axisNameString + ".rawStart",anyTypeToString<int>(axisDescriptor.rawStart)));
    metaData.push_back(std::make_pair(axisNameString + ".triggerAxisName",WStringToString(axisDescriptor.triggerAxisName)));
    // END Vernissage::Session::axisDescriptor

    // BEGIN Vernissage::Session:AxisTableSet
    const Vernissage::Session::AxisTableSets axisTableSetsMap = m_VernissageSession->getAxisTableSets(m_brickletPtr, axisNameWString);

    // if it is empty, we got the standard table set which is [start=1,step=1,stop=clocks]
    if (axisTableSetsMap.empty())
    {
      metaData.push_back(std::make_pair(axisNameString + ".AxisTableSet.count","1"));
      metaData.push_back(std::make_pair(axisNameString + ".AxisTableSetNo1.axis",""));
      metaData.push_back(std::make_pair(axisNameString + ".AxisTableSetNo1.start","1"));
      metaData.push_back(std::make_pair(axisNameString + ".AxisTableSetNo1.step","1"));
      metaData.push_back(std::make_pair(axisNameString + ".AxisTableSetNo1.stop","1"));
    }
    else
    {
      Vernissage::Session::AxisTableSets::const_iterator itAxisTabelSetsMap;
      Vernissage::Session::TableSet::const_iterator itAxisTableSetsMapStruct;

      unsigned int index = 0;
      for (itAxisTabelSetsMap = axisTableSetsMap.begin(); itAxisTabelSetsMap != axisTableSetsMap.end(); itAxisTabelSetsMap++)
      {
        for (itAxisTableSetsMapStruct = itAxisTabelSetsMap->second.begin(); itAxisTableSetsMapStruct != itAxisTabelSetsMap->second.end(); itAxisTableSetsMapStruct++)
        {
          index++; // 1-based index
          const std::string baseName = axisNameString + ".AxisTableSetNo" + anyTypeToString<int>(index) + ".axis";
          metaData.push_back(std::make_pair(baseName,WStringToString(itAxisTabelSetsMap->first)));
          metaData.push_back(std::make_pair(baseName + ".start",anyTypeToString<int>(itAxisTableSetsMapStruct->start)));
          metaData.push_back(std::make_pair(baseName + ".step",anyTypeToString<int>(itAxisTableSetsMapStruct->step)));
          metaData.push_back(std::make_pair(baseName + ".stop",anyTypeToString<int>(itAxisTableSetsMapStruct->stop)));
        }
      }
      metaData.push_back(std::make_pair(axisNameString + ".AxisTableSet.count",anyTypeToString<int>(index)));
    }
    // END Vernissage::Session:AxisTableSet
  }

  DEBUGPRINT("Loaded %d keys/values as brickletMetaData for bricklet %d", metaData.size(), m_brickletID);

  // ensure that we only use as much memory as we have to
  m_metaData.swap(metaData);
}

// see Vernissage Manual page 20/21
// the idea here is to first get the root and triggerAxis for the Bricklet
// Then we know the starting point of the axis hierachy (rootAxis) and the endpoint (triggerAxis)
// In this way we can then traverse from the endpoint (triggerAxis) to the starting point (rootAxis) and record all axis names
// In more than 99% of the cases this routine will return one to three axes
// The value of maxRuns is strictly speaking wrong becase the Matrix Software supports an unlimited number of axes, but due to pragmativ and safe coding reasons this has ben set to 100.
// The returned list will have the entries "triggerAxisName;axisNameWhichTriggeredTheTriggerAxis;...;rootAxisName"
void BrickletClass::generateAllAxesVector()
{
  std::wstring axisName, rootAxis, triggerAxis;
  std::vector<std::wstring> allAxesWString;
  std::vector<std::string> allAxesString;

  unsigned int numRuns = 0;
  const unsigned int maxRuns = 100;

  rootAxis = m_VernissageSession->getRootAxisName(m_brickletPtr);
  triggerAxis = m_VernissageSession->getTriggerAxisName(m_brickletPtr);

  axisName = triggerAxis;
  allAxesWString.push_back(triggerAxis);
  allAxesString.push_back(WStringToString(triggerAxis));

  while (axisName != rootAxis)
  {
    axisName = m_VernissageSession->getAxisDescriptor(m_brickletPtr, axisName).triggerAxisName;
    allAxesWString.push_back(axisName);
    allAxesString.push_back(WStringToString(axisName));
    numRuns++;

    if (numRuns > maxRuns)
    {
      HISTPRINT("Found more than 100 axes in the axis hierachy. This is highly unlikely and therefore a bug in this XOP or in Vernissage.");
      break;
    }
  }

  m_allAxesWString = allAxesWString;
  m_allAxesString  = allAxesString;
}

/*
  Returns a vector with the axis hierarchy (std::wstring)
*/
template<>
const std::vector<std::wstring>& BrickletClass::getAxes<std::wstring>()
{
  if (m_allAxesString.empty() || m_allAxesWString.empty())
  {
    generateAllAxesVector();
  }

  return m_allAxesWString;
}

/*
  Returns a vector with the axis hierarchy (std::string)
*/
template<>
const std::vector<std::string>& BrickletClass::getAxes<std::string>()
{
  if (m_allAxesString.empty() || m_allAxesWString.empty())
  {
    generateAllAxesVector();
  }

  return m_allAxesString;
}

/*
  resetting *pBricklet is only needed after the same result file is loaded again to check for new bricklets
*/
void BrickletClass::setBrickletPointer( void* const pBricklet )
{
  m_brickletPtr = pBricklet;
}

const std::vector<Vernissage::Session::ViewTypeCode>& BrickletClass::getViewTypeCodes() const
{
  return m_viewTypeCodes;
}

void* BrickletClass::getBrickletPointer() const
{
  return m_brickletPtr;
}

const ExtremaData& BrickletClass::getExtrema() const
{
  return m_extrema;
}
