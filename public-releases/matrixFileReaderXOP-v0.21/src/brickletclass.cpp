/*
  The file brickletclass.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/
#include "stdafx.h"

#include "brickletclass.hpp"
#include "globaldata.hpp"
#include "utils_bricklet.hpp"
#include "extremadata.hpp"
#include "utils_generic.hpp"

BrickletClass::BrickletClass(int brickletID, void* const vernissageBricklet)
  :
  m_brickletPtr(vernissageBricklet),
  m_rawBufferContents(NULL),
  m_rawBufferContentsSize(0),
  m_brickletID(brickletID),
  m_vernissageSession(GlobalData::Instance().getVernissageSession())
{
  ASSERT_RETURN_VOID(m_vernissageSession);
  ASSERT_RETURN_VOID(vernissageBricklet);
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
  count = 0;
  ASSERT_RETURN_VOID(pBuffer);
  ASSERT_RETURN_VOID(m_vernissageSession);

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
      m_vernissageSession->loadBrickletContents(m_brickletPtr, pBuffer, count);
    }
    catch (...)
    {
      HISTPRINT("Could not load the bricklet contents, probably out of memory in getBrickletContentsBuffer() with bricklet %d", m_brickletID);
      *pBuffer = NULL;
      count = 0;
      return;
    }

    // loadBrickletContents either throws an exception or returns pBuffer == 0 || count == 0 in the error case
    if (*pBuffer == NULL || count == 0)
    {
      HISTPRINT("Out of memory in getBrickletContentsBuffer() with bricklet %d", m_brickletID);
      m_vernissageSession->unloadBrickletContents(m_brickletPtr);
      *pBuffer = NULL;
      count = 0;
      return;
    }

    DEBUGPRINT("pBuffer=%d,count=%d", *pBuffer, count);

    // these two lines have to be surrounded by loadbrickletContents/unloadBrickletContents, otherwise loadbrickletContents will be called
    // implicitly which is quite expensive
    const int rawMin = m_vernissageSession->getRawMin(m_brickletPtr);
    m_extrema.setMinimum(rawMin, m_vernissageSession->toPhysical(rawMin, m_brickletPtr));

    const int rawMax = m_vernissageSession->getRawMax(m_brickletPtr);
    m_extrema.setMaximum(rawMax, m_vernissageSession->toPhysical(rawMax, m_brickletPtr));

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
      m_vernissageSession->unloadBrickletContents(m_brickletPtr);
      return;
    }

    memcpy(m_rawBufferContents, *pBuffer, sizeof(int)*m_rawBufferContentsSize);
    *pBuffer = m_rawBufferContents;

    // release memory from vernissage DLL
    m_vernissageSession->unloadBrickletContents(m_brickletPtr);
  }
}

/*
  Wrapper function which returns a vector of the meta data keys (.first) and values (.second)
*/
const std::vector<BrickletClass::StringPair>& BrickletClass::getMetaData()
{
  if (m_metaData.empty())
  {
    try
    {
      loadMetaData();
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
void BrickletClass::loadMetaData()
{
  std::vector<StringPair> metaData;
  metaData.reserve(METADATA_RESERVE_SIZE);

  // timestamp of creation
  tm ctime = m_vernissageSession->getCreationTimestamp(m_brickletPtr);
  metaData.push_back(std::make_pair("creationTimeStamp", toString(mktime(&ctime))));

  // viewTypeCodes
  typedef std::vector<Vernissage::Session::ViewTypeCode> ViewTypeCodeVector;
  const ViewTypeCodeVector viewTypeCodes = m_vernissageSession->getViewTypes(m_brickletPtr);

  std::string viewTypeCodesAsOneString;
  for (ViewTypeCodeVector::const_iterator it = viewTypeCodes.begin(); it != viewTypeCodes.end(); it++)
  {
    viewTypeCodesAsOneString.append(viewTypeCodeToString(*it) + listSepChar);
  }

  metaData.push_back(std::make_pair("viewTypeCodes",viewTypeCodesAsOneString));
  metaData.push_back(std::make_pair(BRICKLET_ID_KEY,toString(m_brickletID)));

  // resultfile
  metaData.push_back(std::make_pair(RESULT_FILE_NAME_KEY,GlobalData::Instance().getFileName<std::string>()));
  metaData.push_back(std::make_pair(RESULT_DIR_PATH_KEY,GlobalData::Instance().getDirPath<std::string>()));

  // introduced with Vernissage 2.0
  metaData.push_back(std::make_pair("sampleName",toString(m_vernissageSession->getSampleName(m_brickletPtr))));

  // dito
  metaData.push_back(std::make_pair("dataSetName",toString(m_vernissageSession->getDataSetName(m_brickletPtr))));

  // dito
  // datacomment is a vector, each entry is from one call to writeDataComment
  std::vector<std::wstring> dataComments = m_vernissageSession->getDataComments(m_brickletPtr);

  // we also write out the number of data comments for convenient access
  metaData.push_back(std::make_pair("dataComment.count",toString(dataComments.size())));

  for (unsigned int i = 0; i < dataComments.size(); i++)
  {
    const std::string key = "dataCommentNo" + toString(i + 1);
    const std::string value = toString(dataComments[i]);
    metaData.push_back(std::make_pair(key,value));
  }

  // introduced with vernissage 2.1
  metaData.push_back(std::make_pair("visualMateNode",toString(m_vernissageSession->getVisualMateNode(m_brickletPtr))));
  metaData.push_back(std::make_pair("channelGroupName",toString(m_vernissageSession->getChannelGroupName(m_brickletPtr))));

  // introduced with vernissage 2.0 (but forgotten until release 2.1)
  metaData.push_back(std::make_pair("brickletType",brickletTypeToString(m_vernissageSession->getType(m_brickletPtr))));

  // BEGIN m_VernissageSession->getBrickletMetaData
  const Vernissage::Session::BrickletMetaData brickletMetaData = m_vernissageSession->getMetaData(m_brickletPtr);
  metaData.push_back(std::make_pair("brickletMetaData.fileCreatorName",toString(brickletMetaData.fileCreatorName)));
  metaData.push_back(std::make_pair("brickletMetaData.fileCreatorVersion",toString(brickletMetaData.fileCreatorVersion)));
  metaData.push_back(std::make_pair("brickletMetaData.accountName",toString(brickletMetaData.accountName)));
  metaData.push_back(std::make_pair("brickletMetaData.userName",toString(brickletMetaData.userName)));
  // END m_VernissageSession->getBrickletMetaData

  metaData.push_back(std::make_pair("sequenceID",toString(m_vernissageSession->getSequenceId(m_brickletPtr))));
  metaData.push_back(std::make_pair("creationComment",toString(m_vernissageSession->getCreationComment(m_brickletPtr))));
  metaData.push_back(std::make_pair("dimension",toString(m_vernissageSession->getDimensionCount(m_brickletPtr))));
  metaData.push_back(std::make_pair("rootAxis",toString(m_vernissageSession->getRootAxisName(m_brickletPtr))));

  // new in vernissage 2.1
  metaData.push_back(std::make_pair("rootAxisQualified",toString(m_vernissageSession->getRootAxisQualifiedName(m_brickletPtr))));
  metaData.push_back(std::make_pair("triggerAxis",toString(m_vernissageSession->getTriggerAxisName(m_brickletPtr))));
  metaData.push_back(std::make_pair("triggerAxisQualified",toString(m_vernissageSession->getTriggerAxisQualifiedName(m_brickletPtr))));

  metaData.push_back(std::make_pair("channelName",toString(m_vernissageSession->getChannelName(m_brickletPtr))));
  metaData.push_back(std::make_pair("channelInstanceName",toString(m_vernissageSession->getChannelInstanceName(m_brickletPtr))));

  metaData.push_back(std::make_pair(CHANNEL_UNIT_KEY,toString(m_vernissageSession->getChannelUnit(m_brickletPtr))));
  metaData.push_back(std::make_pair("runCycleCount",toString(m_vernissageSession->getRunCycleCount(m_brickletPtr))));

  metaData.push_back(std::make_pair("scanCycleCount",toString(m_vernissageSession->getScanCycleCount(m_brickletPtr))));

  // new in vernissage 2.1
  {
    const std::vector<void*> dependentBrickletsVoidVector = m_vernissageSession->getDependingBricklets(m_brickletPtr);
    const std::vector<int> dependentBrickletsIntVector    = GlobalData::Instance().convertBrickletPtr(dependentBrickletsVoidVector);
    std::string dependentBricklets;
    joinString(dependentBrickletsIntVector, listSepChar, dependentBricklets);

    metaData.push_back(std::make_pair("dependentBricklets",dependentBricklets));
  }

  // new in vernissage 2.1
  {
    const std::vector<void*> referencedBrickletsVoidVector = m_vernissageSession->getReferencedBricklets(m_brickletPtr);
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

  const std::vector<std::wstring> elementInstanceNames = m_vernissageSession->getExperimentElementInstanceNames(m_brickletPtr, L"");
  for (std::vector<std::wstring>::const_iterator itInstance = elementInstanceNames.begin(); itInstance != elementInstanceNames.end(); itInstance++)
  {
    typedef std::map<std::wstring, Vernissage::Session::Parameter> MapType;

    const MapType elementInstanceParamsMap = m_vernissageSession->getExperimentElementParameters(m_brickletPtr, *itInstance);

    for (MapType::const_iterator itParam = elementInstanceParamsMap.begin(); itParam != elementInstanceParamsMap.end(); itParam++)
    {
      const std::string key = toString(*itInstance) + std::string(".") + toString(itParam->first) ;
      metaData.push_back(std::make_pair(key + std::string(".value"),toString(itParam->second.value)));
      metaData.push_back(std::make_pair(key + std::string(".unit"),toString(itParam->second.unit)));
    }
  }

  // BEGIN Vernissage::Session::SpatialInfo
  const Vernissage::Session::SpatialInfo spatialInfo = m_vernissageSession->getSpatialInfo(m_brickletPtr);
  for (std::vector<double>::const_iterator it = spatialInfo.physicalX.begin(); it != spatialInfo.physicalX.end(); it++)
  {
    char buf[ARRAY_SIZE];
    int index = it - spatialInfo.physicalX.begin() + 1 ; // one-based Index
    sprintf(buf, "spatialInfo.physicalX.No%d", index);
    metaData.push_back(std::make_pair(buf,toString(*it)));
  }
  for (std::vector<double>::const_iterator it = spatialInfo.physicalY.begin(); it != spatialInfo.physicalY.end(); it++)
  {
    char buf[ARRAY_SIZE];
    int index = it - spatialInfo.physicalY.begin() + 1 ; // one-based Index
    sprintf(buf, "spatialInfo.physicalY.No%d", index);
    metaData.push_back(std::make_pair(buf,toString(*it)));
  }

  if (spatialInfo.originatorKnown)
  {
    metaData.push_back(std::make_pair("spatialInfo.originatorKnown","true"));
    metaData.push_back(std::make_pair("spatialInfo.channelName",toString(spatialInfo.channelName)));
    metaData.push_back(std::make_pair("spatialInfo.sequenceId",toString(spatialInfo.sequenceId)));
    metaData.push_back(std::make_pair("spatialInfo.runCycleCount",toString(spatialInfo.runCycleCount)));
    metaData.push_back(std::make_pair("spatialInfo.scanCycleCount",toString(spatialInfo.scanCycleCount)));
    metaData.push_back(std::make_pair("spatialInfo.viewName",toString(spatialInfo.viewName)));
    metaData.push_back(std::make_pair("spatialInfo.viewSelectionId",toString(spatialInfo.viewSelectionId)));
    metaData.push_back(std::make_pair("spatialInfo.viewSelectionIndex",toString(spatialInfo.viewSelectionIndex)));
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
  const Vernissage::Session::ExperimentInfo experimentInfo = m_vernissageSession->getExperimentInfo(m_brickletPtr);
  metaData.push_back(std::make_pair("experimentInfo.Name",toString(experimentInfo.experimentName)));
  metaData.push_back(std::make_pair("experimentInfo.Version",toString(experimentInfo.experimentVersion)));
  metaData.push_back(std::make_pair("experimentInfo.Description",toString(experimentInfo.experimentDescription)));
  metaData.push_back(std::make_pair("experimentInfo.FileSpec",toString(experimentInfo.experimentFileSpec)));
  metaData.push_back(std::make_pair("experimentInfo.projectName",toString(experimentInfo.projectName)));
  metaData.push_back(std::make_pair("experimentInfo.projectVersion",toString(experimentInfo.projectVersion)));
  metaData.push_back(std::make_pair("experimentInfo.projectFileSpec",toString(experimentInfo.projectFileSpec)));
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
    const std::string axisNameString  = toString(*itAllAxes);

    const Vernissage::Session::AxisDescriptor axisDescriptor = m_vernissageSession->getAxisDescriptor(m_brickletPtr, axisNameWString);
    metaData.push_back(std::make_pair(axisNameString + ".clocks",toString(axisDescriptor.clocks)));
    metaData.push_back(std::make_pair(axisNameString + ".mirrored",(axisDescriptor.mirrored ? "true" : "false")));
    metaData.push_back(std::make_pair(axisNameString + ".physicalUnit",toString(axisDescriptor.physicalUnit)));
    metaData.push_back(std::make_pair(axisNameString + ".physicalIncrement",toString(axisDescriptor.physicalIncrement)));
    metaData.push_back(std::make_pair(axisNameString + ".physicalStart",toString(axisDescriptor.physicalStart)));
    metaData.push_back(std::make_pair(axisNameString + ".rawIncrement",toString(axisDescriptor.rawIncrement)));
    metaData.push_back(std::make_pair(axisNameString + ".rawStart",toString(axisDescriptor.rawStart)));
    metaData.push_back(std::make_pair(axisNameString + ".triggerAxisName",toString(axisDescriptor.triggerAxisName)));
    // END Vernissage::Session::axisDescriptor

    // BEGIN Vernissage::Session:AxisTableSet
    const Vernissage::Session::AxisTableSets axisTableSetsMap = m_vernissageSession->getAxisTableSets(m_brickletPtr, axisNameWString);

    // if it is empty, we got the standard table set which is [start=1,step=1,stop=clocks]
    if (axisTableSetsMap.empty())
    {
      metaData.push_back(std::make_pair(axisNameString + ".AxisTableSet.count","1"));
      metaData.push_back(std::make_pair(axisNameString + ".AxisTableSetNo1.axis",""));
      metaData.push_back(std::make_pair(axisNameString + ".AxisTableSetNo1.start","1"));
      metaData.push_back(std::make_pair(axisNameString + ".AxisTableSetNo1.step","1"));
      metaData.push_back(std::make_pair(axisNameString + ".AxisTableSetNo1.stop",toString(axisDescriptor.clocks)));
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
          const std::string baseName = axisNameString + ".AxisTableSetNo" + toString(index) + ".axis";
          metaData.push_back(std::make_pair(baseName,toString(itAxisTabelSetsMap->first)));
          metaData.push_back(std::make_pair(baseName + ".start",toString(itAxisTableSetsMapStruct->start)));
          metaData.push_back(std::make_pair(baseName + ".step",toString(itAxisTableSetsMapStruct->step)));
          metaData.push_back(std::make_pair(baseName + ".stop",toString(itAxisTableSetsMapStruct->stop)));
        }
      }
      metaData.push_back(std::make_pair(axisNameString + ".AxisTableSet.count",toString(index)));
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
// The value of maxRuns is strictly speaking wrong becaUse the Matrix Software supports an unlimited number of axes, but due to pragmativ and safe coding reasons this has ben set to 100.
// The returned list will have the entries "triggerAxisName;axisNameWhichTriggeredTheTriggerAxis;...;rootAxisName"
void BrickletClass::generateAllAxesVector()
{
  std::vector<std::wstring> allAxesWString;
  std::vector<std::string> allAxesString;

  unsigned int numRuns = 0;
  const unsigned int maxRuns = 100;

  const std::wstring rootAxis = m_vernissageSession->getRootAxisName(m_brickletPtr);
  const std::wstring triggerAxis = m_vernissageSession->getTriggerAxisName(m_brickletPtr);

  std::wstring axisName = triggerAxis;
  allAxesWString.push_back(triggerAxis);
  allAxesString.push_back(toString(triggerAxis));

  while (axisName != rootAxis)
  {
    axisName = m_vernissageSession->getAxisDescriptor(m_brickletPtr, axisName).triggerAxisName;
    allAxesWString.push_back(axisName);
    allAxesString.push_back(toString(axisName));
    numRuns++;

    if (numRuns > maxRuns)
    {
      HISTPRINT("Found more than 100 axes in the axis hierachy. This is highly unlikely and therefore a bug in this XOP or in Vernissage.");
      break;
    }
  }

  m_allAxesWString.swap(allAxesWString);
  m_allAxesString.swap(allAxesString);
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
  Resetting *vernissageBricklet is only needed after the same result file is loaded again to check for new bricklets
*/
void BrickletClass::setBrickletPointer( void* const vernissageBricklet )
{
  ASSERT_RETURN_VOID(vernissageBricklet);
  m_brickletPtr = vernissageBricklet;
}

void* BrickletClass::getBrickletPointer() const
{
  return m_brickletPtr;
}

const ExtremaData& BrickletClass::getExtrema() const
{
  return m_extrema;
}
