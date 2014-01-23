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

namespace  {

  typedef std::map<std::wstring, Vernissage::Session::Parameter> ParameterMap;
  typedef ParameterMap::const_iterator ParameterMapIt;
  typedef std::vector<BrickletClass::StringPair> StringPairVector;
  typedef std::vector<std::wstring> WstringVector;
  typedef WstringVector::const_iterator WstringVectorIt;

  template<typename T>
  inline void AddMetaData(StringPairVector& metaData, std::string key, T value )
  {
    metaData.push_back(std::make_pair(key, toString(value)));
  }

  void AddParameterMap( StringPairVector& metaData, const std::string& parentName, const ParameterMap& paramMap )
  {
    for (ParameterMap::const_iterator it = paramMap.begin(); it != paramMap.end(); it++)
    {
      const std::string key = parentName + std::string(".") + toString(it->first);
      AddMetaData(metaData,key + std::string(".value"),it->second.value);
      AddMetaData(metaData,key + std::string(".unit"),it->second.unit);
    }
  }

} // anonymous namespace

BrickletClass::BrickletClass(int brickletID, void* const vernissageBricklet)
  :
  m_brickletPtr(vernissageBricklet),
  m_rawBufferContents(NULL),
  m_rawBufferContentsSize(0),
  m_brickletID(brickletID)
{
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
  std::vector<StringPair>().swap(m_deployParams);
  std::vector<std::string>().swap(m_allAxesString);
  std::vector<std::wstring>().swap(m_allAxesWString);
}

/*
  Load the raw data of the bricklet into our own cache
*/
void BrickletClass::getBrickletContentsBuffer(const int** pBuffer, int& count)
{
  count = 0;
  ASSERT_RETURN_VOID(pBuffer);
  Vernissage::Session* session = getVernissageSession();
  ASSERT_RETURN_VOID(session);

  // we are not called the first time
  if (m_rawBufferContents != NULL)
  {
    DEBUGPRINT("BrickletClass::getBrickletContentsBuffer Using cached values");
    *pBuffer = m_rawBufferContents;
    count    = m_rawBufferContentsSize;

    DEBUGPRINT("m_rawBufferContents=%p,size=%d", m_rawBufferContents, m_rawBufferContentsSize);
  }
  else  // we are called the first time
  {
    try
    {
      session->loadBrickletContents(m_brickletPtr, pBuffer, count);
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
      session->unloadBrickletContents(m_brickletPtr);
      *pBuffer = NULL;
      count = 0;
      return;
    }

    // these two lines have to be surrounded by loadbrickletContents/unloadBrickletContents, otherwise loadbrickletContents will be called
    // implicitly which is quite expensive
    const int rawMin = session->getRawMin(m_brickletPtr);
    m_extrema.setMinimum(rawMin, session->toPhysical(rawMin, m_brickletPtr));

    const int rawMax = session->getRawMax(m_brickletPtr);
    m_extrema.setMaximum(rawMax, session->toPhysical(rawMax, m_brickletPtr));

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
      session->unloadBrickletContents(m_brickletPtr);
      return;
    }

    memcpy(m_rawBufferContents, *pBuffer, sizeof(int)*m_rawBufferContentsSize);
    *pBuffer = m_rawBufferContents;

    DEBUGPRINT("m_rawBufferContents=%p,size=%d", m_rawBufferContents, m_rawBufferContentsSize);

    // release memory from vernissage DLL
    session->unloadBrickletContents(m_brickletPtr);
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
  typedef std::vector<Vernissage::Session::ViewTypeCode> ViewTypeCodeVector;
  typedef ViewTypeCodeVector::const_iterator ViewTypeCodeVectorIt;

  StringPairVector metaData;
  metaData.reserve(METADATA_RESERVE_SIZE);

  Vernissage::Session* session = getVernissageSession();
  ASSERT_RETURN_VOID(session);

  // timestamp of creation
  tm ctime = session->getCreationTimestamp(m_brickletPtr);
  AddMetaData(metaData,"creationTimeStamp", mktime(&ctime));

  // viewTypeCodes
  const ViewTypeCodeVector viewTypeCodes = session->getViewTypes(m_brickletPtr);

  std::string viewTypeCodesAsOneString;
  for (ViewTypeCodeVectorIt it = viewTypeCodes.begin(); it != viewTypeCodes.end(); it++)
  {
    viewTypeCodesAsOneString.append(viewTypeCodeToString(*it) + listSepChar);
  }

  AddMetaData(metaData,"viewTypeCodes",viewTypeCodesAsOneString);
  AddMetaData(metaData,BRICKLET_ID_KEY,m_brickletID);

  // resultfile
  AddMetaData(metaData,RESULT_FILE_NAME_KEY,GlobalData::Instance().getFileName<std::string>());
  AddMetaData(metaData,RESULT_DIR_PATH_KEY,GlobalData::Instance().getDirPath<std::string>());

  // introduced with Vernissage 2.0
  AddMetaData(metaData,"sampleName",session->getSampleName(m_brickletPtr));

  // dito
  AddMetaData(metaData,"dataSetName",session->getDataSetName(m_brickletPtr));

  // dito
  // datacomment is a vector, each entry is from one call to writeDataComment
  const WstringVector dataComments = session->getDataComments(m_brickletPtr);

  // we also write out the number of data comments for convenient access
  AddMetaData(metaData,"dataComment.count",dataComments.size());

  for (unsigned int i = 0; i < dataComments.size(); i++)
  {
    const std::string key = "dataCommentNo" + toString(i + 1);
    const std::string value = toString(dataComments[i]);
    AddMetaData(metaData,key,value);
  }

  // introduced with vernissage 2.1
  AddMetaData(metaData,"visualMateNode",session->getVisualMateNode(m_brickletPtr));
  AddMetaData(metaData,"channelGroupName",session->getChannelGroupName(m_brickletPtr));

  // introduced with vernissage 2.0 (but forgotten until release 2.1)
  AddMetaData(metaData,"brickletType",brickletTypeToString(session->getType(m_brickletPtr)));

  // BEGIN session->getBrickletMetaData
  const Vernissage::Session::BrickletMetaData brickletMetaData = session->getMetaData(m_brickletPtr);
  AddMetaData(metaData,"brickletMetaData.fileCreatorName",brickletMetaData.fileCreatorName);
  AddMetaData(metaData,"brickletMetaData.fileCreatorVersion",brickletMetaData.fileCreatorVersion);
  AddMetaData(metaData,"brickletMetaData.accountName",brickletMetaData.accountName);
  AddMetaData(metaData,"brickletMetaData.userName",brickletMetaData.userName);
  // END session->getBrickletMetaData

  AddMetaData(metaData,"sequenceID",session->getSequenceId(m_brickletPtr));
  AddMetaData(metaData,"creationComment",session->getCreationComment(m_brickletPtr));
  AddMetaData(metaData,"dimension",session->getDimensionCount(m_brickletPtr));
  AddMetaData(metaData,"rootAxis",session->getRootAxisName(m_brickletPtr));

  // new in vernissage 2.1
  AddMetaData(metaData,"rootAxisQualified",session->getRootAxisQualifiedName(m_brickletPtr));
  AddMetaData(metaData,"triggerAxis",session->getTriggerAxisName(m_brickletPtr));
  AddMetaData(metaData,"triggerAxisQualified",session->getTriggerAxisQualifiedName(m_brickletPtr));

  AddMetaData(metaData,"channelName",session->getChannelName(m_brickletPtr));
  AddMetaData(metaData,"channelInstanceName",session->getChannelInstanceName(m_brickletPtr));

  AddMetaData(metaData,CHANNEL_UNIT_KEY,session->getChannelUnit(m_brickletPtr));
  AddMetaData(metaData,"runCycleCount",session->getRunCycleCount(m_brickletPtr));

  AddMetaData(metaData,"scanCycleCount",session->getScanCycleCount(m_brickletPtr));

  // new in vernissage 2.1
  {
    const std::vector<void*> dependentBrickletsVoidVector = session->getDependingBricklets(m_brickletPtr);
    const std::vector<int> dependentBrickletsIntVector    = GlobalData::Instance().convertBrickletPtr(dependentBrickletsVoidVector);
    std::string dependentBricklets;
    joinString(dependentBrickletsIntVector, listSepChar, dependentBricklets);

    AddMetaData(metaData,"dependentBricklets",dependentBricklets);
  }

  // new in vernissage 2.1
  {
    const std::vector<void*> referencedBrickletsVoidVector = session->getReferencedBricklets(m_brickletPtr);
    const std::vector<int> referencedBrickletsIntVector    = GlobalData::Instance().convertBrickletPtr(referencedBrickletsVoidVector);
    std::string referencedBricklets;
    joinString(referencedBrickletsIntVector, listSepChar, referencedBricklets);

    AddMetaData(metaData,"referencedBricklets",referencedBricklets);
  }

  // new in vernissage 2.1
  {
    const std::vector<void*> brickletSeriesVoidVector = GlobalData::Instance().getBrickletSeries(m_brickletPtr);
    std::vector<int> brickletSeriesIntVector    = GlobalData::Instance().convertBrickletPtr(brickletSeriesVoidVector);
    // we want to have a sorted list of bricklets
    std::sort(brickletSeriesIntVector.begin(), brickletSeriesIntVector.end());
    std::string brickletSeries;
    joinString(brickletSeriesIntVector, listSepChar, brickletSeries);

    AddMetaData(metaData,"brickletSeries",brickletSeries);
  }

  const std::vector<std::wstring> elementInstanceNames = session->getExperimentElementInstanceNames(m_brickletPtr, L"");
  for (std::vector<std::wstring>::const_iterator it = elementInstanceNames.begin(); it != elementInstanceNames.end(); it++)
  {
   const ParameterMap elementInstanceParamsMap = session->getExperimentElementParameters(m_brickletPtr, *it);
   AddParameterMap(metaData,toString(*it),elementInstanceParamsMap);
  }

  // BEGIN Vernissage::Session::SpatialInfo
  const Vernissage::Session::SpatialInfo spatialInfo = session->getSpatialInfo(m_brickletPtr);
  for (std::vector<double>::const_iterator it = spatialInfo.physicalX.begin(); it != spatialInfo.physicalX.end(); it++)
  {
    const int index = it - spatialInfo.physicalX.begin() + 1 ; // 1-based index
    const std::string key = "spatialInfo.physicalX.No" + toString(index);
    AddMetaData(metaData,key,*it);
  }
  for (std::vector<double>::const_iterator it = spatialInfo.physicalY.begin(); it != spatialInfo.physicalY.end(); it++)
  {
    const int index = it - spatialInfo.physicalY.begin() + 1 ; // 1-based index
    const std::string key = "spatialInfo.physicalY.No" + toString(index);
    AddMetaData(metaData,key,*it);
  }

  if (spatialInfo.originatorKnown)
  {
    AddMetaData(metaData,"spatialInfo.originatorKnown","true");
    AddMetaData(metaData,"spatialInfo.channelName",spatialInfo.channelName);
    AddMetaData(metaData,"spatialInfo.sequenceId",spatialInfo.sequenceId);
    AddMetaData(metaData,"spatialInfo.runCycleCount",spatialInfo.runCycleCount);
    AddMetaData(metaData,"spatialInfo.scanCycleCount",spatialInfo.scanCycleCount);
    AddMetaData(metaData,"spatialInfo.viewName",spatialInfo.viewName);
    AddMetaData(metaData,"spatialInfo.viewSelectionId",spatialInfo.viewSelectionId);
    AddMetaData(metaData,"spatialInfo.viewSelectionIndex",spatialInfo.viewSelectionIndex);
  }
  else
  {
    AddMetaData(metaData,"spatialInfo.originatorKnown","false");
    AddMetaData(metaData,"spatialInfo.channelName","");
    AddMetaData(metaData,"spatialInfo.sequenceId","");
    AddMetaData(metaData,"spatialInfo.runCycleCount","");
    AddMetaData(metaData,"spatialInfo.scanCycleCount","");
    AddMetaData(metaData,"spatialInfo.viewName","");
    AddMetaData(metaData,"spatialInfo.viewSelectionId","");
    AddMetaData(metaData,"spatialInfo.viewSelectionIndex","");
  }
  //END Vernissage::Session::SpatialInfo

  // BEGIN Vernissage::Session::ExperimentInfo
  const Vernissage::Session::ExperimentInfo experimentInfo = session->getExperimentInfo(m_brickletPtr);
  AddMetaData(metaData,"experimentInfo.Name",experimentInfo.experimentName);
  AddMetaData(metaData,"experimentInfo.Version",experimentInfo.experimentVersion);
  AddMetaData(metaData,"experimentInfo.Description",experimentInfo.experimentDescription);
  AddMetaData(metaData,"experimentInfo.FileSpec",experimentInfo.experimentFileSpec);
  AddMetaData(metaData,"experimentInfo.projectName",experimentInfo.projectName);
  AddMetaData(metaData,"experimentInfo.projectVersion",experimentInfo.projectVersion);
  AddMetaData(metaData,"experimentInfo.projectFileSpec",experimentInfo.projectFileSpec);
  // END Vernissage::Session::ExperimentInfo

  // create m_allAxes* internally
  getAxes<std::string>();
  std::string allAxesAsOneString;
  joinString(m_allAxesString, listSepChar, allAxesAsOneString);
  AddMetaData(metaData,"allAxes",allAxesAsOneString);

  // BEGIN Vernissage::Session::axisDescriptor
  for (WstringVectorIt itAllAxes = m_allAxesWString.begin(); itAllAxes != m_allAxesWString.end(); itAllAxes++)
  {
    const std::wstring axisNameWString = *itAllAxes;
    const std::string axisNameString  = toString(*itAllAxes);

    const Vernissage::Session::AxisDescriptor axisDescriptor = session->getAxisDescriptor(m_brickletPtr, axisNameWString);
    AddMetaData(metaData,axisNameString + ".clocks",axisDescriptor.clocks);
    AddMetaData(metaData,axisNameString + ".mirrored",(axisDescriptor.mirrored ? "true" : "false"));
    AddMetaData(metaData,axisNameString + ".physicalUnit",axisDescriptor.physicalUnit);
    AddMetaData(metaData,axisNameString + ".physicalIncrement",axisDescriptor.physicalIncrement);
    AddMetaData(metaData,axisNameString + ".physicalStart",axisDescriptor.physicalStart);
    AddMetaData(metaData,axisNameString + ".rawIncrement",axisDescriptor.rawIncrement);
    AddMetaData(metaData,axisNameString + ".rawStart",axisDescriptor.rawStart);
    AddMetaData(metaData,axisNameString + ".triggerAxisName",axisDescriptor.triggerAxisName);
    // END Vernissage::Session::axisDescriptor

    // BEGIN Vernissage::Session:AxisTableSet
    const Vernissage::Session::AxisTableSets axisTableSetsMap = session->getAxisTableSets(m_brickletPtr, axisNameWString);

    // if it is empty, we got the standard table set which is [start=1,step=1,stop=clocks]
    if (axisTableSetsMap.empty())
    {
      AddMetaData(metaData,axisNameString + ".AxisTableSet.count","1");
      AddMetaData(metaData,axisNameString + ".AxisTableSetNo1.axis","");
      AddMetaData(metaData,axisNameString + ".AxisTableSetNo1.start","1");
      AddMetaData(metaData,axisNameString + ".AxisTableSetNo1.step","1");
      AddMetaData(metaData,axisNameString + ".AxisTableSetNo1.stop",axisDescriptor.clocks);
    }
    else
    {
      unsigned int index = 0;
      typedef Vernissage::Session::AxisTableSets::const_iterator AxisTableSetsIt;
      for (AxisTableSetsIt itMap = axisTableSetsMap.begin(); itMap != axisTableSetsMap.end(); itMap++)
      {
        typedef Vernissage::Session::TableSet::const_iterator TableSetIt;
        for (TableSetIt it = itMap->second.begin(); it != itMap->second.end(); it++)
        {
          index++; // 1-based index
          const std::string baseName = axisNameString + ".AxisTableSetNo" + toString(index) + ".axis";
          AddMetaData(metaData,baseName,itMap->first);
          AddMetaData(metaData,baseName + ".start",it->start);
          AddMetaData(metaData,baseName + ".step",it->step);
          AddMetaData(metaData,baseName + ".stop",it->stop);
        }
      }
      AddMetaData(metaData,axisNameString + ".AxisTableSet.count",index);
    }
    // END Vernissage::Session:AxisTableSet

    // BEGIN Vernissage::AxisParameters introduced with Vernissage 2.2
    const ParameterMap axisParams = session->getAxisParameters(m_brickletPtr, axisNameWString);
    AddParameterMap(metaData,axisNameString,axisParams);
    // END Vernissage::AxisParameters
  }

  DEBUGPRINT("Loaded %d keys/values as brickletMetaData for bricklet %d", metaData.size(), m_brickletID);

  // ensure that we only use as much memory as we have to
  m_metaData.swap(metaData);
}

/*
Wrapper function which returns a vector with the deployment parameters and their values
*/
const std::vector<BrickletClass::StringPair>& BrickletClass::getDeploymentParameter()
{
  if (m_deployParams.empty())
  {
    try
    {
      loadDeploymentParameters();
    }
    catch (CMemoryException* e)
    {
      e->Delete();
      HISTPRINT("Out of memory in getBrickletMetaDataValues()");
    }
  }
  return m_deployParams;
}

/*
  Reads all deployment parameters into our own structures
*/
void BrickletClass::loadDeploymentParameters()
{
  StringPairVector deployParams;
  deployParams.reserve(METADATA_RESERVE_SIZE);

  Vernissage::Session* session = getVernissageSession();
  ASSERT_RETURN_VOID(session);

  const WstringVector elementInstanceNames = session->getExperimentElementInstanceNames(m_brickletPtr, L"");
  for (WstringVectorIt itInstance = elementInstanceNames.begin(); itInstance != elementInstanceNames.end(); itInstance++)
  {
    typedef std::map<std::wstring,std::wstring> ParamMap;
    typedef ParamMap::const_iterator ParamMapIt;
    const ParamMap paramMap = session->getExperimentElementDeploymentParameters(m_brickletPtr, *itInstance);
    for (ParamMapIt it = paramMap.begin(); it != paramMap.end(); it++)
    {
      const std::string key = toString(*itInstance) + "." + toString(it->first);
      AddMetaData(deployParams,key,it->second);
    }
  }

  m_deployParams.swap(deployParams);
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

  Vernissage::Session* session = getVernissageSession();
  ASSERT_RETURN_VOID(session);

  const std::wstring rootAxis = session->getRootAxisName(m_brickletPtr);
  const std::wstring triggerAxis = session->getTriggerAxisName(m_brickletPtr);

  std::wstring axisName = triggerAxis;
  allAxesWString.push_back(triggerAxis);
  allAxesString.push_back(toString(triggerAxis));

  while (axisName != rootAxis)
  {
    axisName = session->getAxisDescriptor(m_brickletPtr, axisName).triggerAxisName;
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
