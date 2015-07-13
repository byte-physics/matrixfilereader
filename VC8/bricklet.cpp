/*
  The file brickletclass.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/
#include "stdafx.h"

#include "bricklet.hpp"
#include "globaldata.hpp"
#include "utils_bricklet.hpp"
#include "extremadata.hpp"
#include "utils_generic.hpp"

namespace  {

  typedef std::map<std::wstring, Vernissage::Session::Parameter> ParameterMap;
  typedef ParameterMap::const_iterator ParameterMapIt;
  typedef std::vector<std::wstring> WstringVector;
  typedef WstringVector::const_iterator WstringVectorIt;

  void AddParameterMap( StringPairVector& metaData, const std::string& parentName, const ParameterMap& paramMap )
  {
    for (ParameterMap::const_iterator it = paramMap.begin(); it != paramMap.end(); it++)
    {
      const std::string key = parentName + std::string(".") + toString(it->first);
      AddEntry(metaData,key + std::string(".value"),it->second.value);
      AddEntry(metaData,key + std::string(".unit"),it->second.unit);
    }
  }

  // Ensure that we only use as much memory as we have to
  void ShrinkToFit( StringPairVector& vec )
  {
    StringPairVector(vec.begin(),vec.end()).swap(vec);

    std::size_t size = 0;
    std::size_t capacity = 0;

    if(GlobalData::Instance().isDebuggingEnabled())
    {
      for (StringPairVectorCIt it = vec.begin(); it != vec.end(); it++)
      {
        //For std::strings shrink to fit does not do anything in VC8
        size     += it->first.size() + it->second.size();
        capacity += it->first.capacity() + it->second.capacity();
      }
      DEBUGPRINT("vector:  size=%d, capacity=%d",vec.size(), vec.capacity());
      DEBUGPRINT("strings: size=%d, capacity=%d",size, capacity);
    }
  }

  int convertBrickletPtr(void* rawBrickletPtr)
  {
    return GlobalData::Instance().convertBrickletPtr(rawBrickletPtr);
  }

  std::vector<int> convertRawBrickletVector(std::vector<void*> rawVec)
  {
    std::vector<int> idVec(rawVec.size());
    std::transform(rawVec.begin(),rawVec.end(),idVec.begin(),convertBrickletPtr);
    return idVec;
  }

} // anonymous namespace

Bricklet::Bricklet(int brickletID, void* const vernissageBricklet)
  :
  m_brickletPtr(vernissageBricklet),
  m_rawBufferContentsSize(0),
  m_brickletID(brickletID)
{
  if (!isValidBrickletID(brickletID))
  {
    throw std::runtime_error("Invalid brickletID: " + toString(brickletID));
  }

  THROW_IF_NULL(vernissageBricklet);
}

Bricklet::~Bricklet()
{}

/*
  delete our internally cached data, is called at destruction time and if we want keep memory
  consumption low
*/
void Bricklet::clearCache()
{
  if (m_rawBufferContents)
  {
    DEBUGPRINT("Deleting raw data from bricklet %d", m_brickletID);
    m_rawBufferContents.reset();
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
int* Bricklet::getRawData()
{
  // we are not called the first time
  if (m_rawBufferContents)
  {
    DEBUGPRINT("BrickletClass::getBrickletContentsBuffer Using cached values");
    DEBUGPRINT("m_rawBufferContents=%p,size=%d", m_rawBufferContents.get(), m_rawBufferContentsSize);
    return m_rawBufferContents.get();
  }

  int count = 0;
  const int *pBuffer;
  Vernissage::Session* session = getVernissageSession();

  try
  {
    session->loadBrickletContents(m_brickletPtr, &pBuffer, count);
  }
  catch (...)
  {
    HISTPRINT("Could not load the bricklet contents, probably out of memory in getBrickletContentsBuffer() with bricklet %d", m_brickletID);
    return NULL;
  }

  // loadBrickletContents either throws an exception or returns pBuffer == 0 || count == 0 in the error case
  if (pBuffer == NULL || count <= 0)
  {
    HISTPRINT("Out of memory in getBrickletContentsBuffer() with bricklet %d", m_brickletID);
    session->unloadBrickletContents(m_brickletPtr);
    return NULL;
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
  try
  {
    m_rawBufferContents.reset(new int[count]);
  }
  catch (CMemoryException* e)
  {
    e->Delete();
    HISTPRINT("Out of memory in getBrickletContentsBuffer()");
    session->unloadBrickletContents(m_brickletPtr);
    return NULL;
  }

  memcpy(m_rawBufferContents.get(), pBuffer, sizeof(int)*count);
  m_rawBufferContentsSize = count;

  DEBUGPRINT("m_rawBufferContents=%p,size=%d", m_rawBufferContents.get(), m_rawBufferContentsSize);

  // release memory from vernissage DLL
  session->unloadBrickletContents(m_brickletPtr);
  return m_rawBufferContents.get();
}

/*
  Wrapper function which returns a vector of the meta data keys (.first) and values (.second)
*/
const std::vector<StringPair>& Bricklet::getMetaData()
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
      HISTPRINT("Out of memory in getMetaData()");
      m_metaData.clear();
    }
  }
  return m_metaData;
}

/*
  Reads all meta data into our own structures
*/
void Bricklet::loadMetaData()
{
  typedef std::vector<Vernissage::Session::ViewTypeCode> ViewTypeCodeVector;
  typedef ViewTypeCodeVector::const_iterator ViewTypeCodeVectorIt;

  m_metaData.clear();
  m_metaData.reserve(RESERVE_SIZE);

  Vernissage::Session* session = getVernissageSession();

  // timestamp of creation
  tm ctime = session->getCreationTimestamp(m_brickletPtr);
  AddEntry(m_metaData,"creationTimeStamp", mktime(&ctime));

  // viewTypeCodes
  const ViewTypeCodeVector viewTypeCodes = session->getViewTypes(m_brickletPtr);

  std::string viewTypeCodesAsOneString;
  for (ViewTypeCodeVectorIt it = viewTypeCodes.begin(); it != viewTypeCodes.end(); it++)
  {
    viewTypeCodesAsOneString.append(viewTypeCodeToString(*it) + listSepChar);
  }

  AddEntry(m_metaData,"viewTypeCodes",viewTypeCodesAsOneString);
  AddEntry(m_metaData,BRICKLET_ID_KEY,m_brickletID);

  // resultfile
  AddEntry(m_metaData,RESULT_FILE_NAME_KEY,unicodeToAnsi(GlobalData::Instance().getFileName()));
  AddEntry(m_metaData,RESULT_DIR_PATH_KEY,unicodeToAnsi(GlobalData::Instance().getDirPath()));

  // introduced with Vernissage 2.0
  AddEntry(m_metaData,"sampleName",session->getSampleName(m_brickletPtr));

  // dito
  AddEntry(m_metaData,"dataSetName",session->getDataSetName(m_brickletPtr));

  // dito
  // datacomment is a vector, each entry is from one call to writeDataComment
  const WstringVector dataComments = session->getDataComments(m_brickletPtr);

  // we also write out the number of data comments for convenient access
  AddEntry(m_metaData,"dataComment.count",dataComments.size());

  for (unsigned int i = 0; i < dataComments.size(); i++)
  {
    const std::string key = "dataCommentNo" + toString(i + 1);
    const std::string value = toString(dataComments[i]);
    AddEntry(m_metaData,key,value);
  }

  // introduced with vernissage 2.1
  AddEntry(m_metaData,"visualMateNode",session->getVisualMateNode(m_brickletPtr));
  AddEntry(m_metaData,"channelGroupName",session->getChannelGroupName(m_brickletPtr));

  // introduced with vernissage 2.0 (but forgotten until release 2.1)
  AddEntry(m_metaData,"brickletType",brickletTypeToString(session->getType(m_brickletPtr)));

  // BEGIN session->getBrickletMetaData
  const Vernissage::Session::BrickletMetaData brickletMetaData = session->getMetaData(m_brickletPtr);
  AddEntry(m_metaData,"brickletMetaData.fileCreatorName",brickletMetaData.fileCreatorName);
  AddEntry(m_metaData,"brickletMetaData.fileCreatorVersion",brickletMetaData.fileCreatorVersion);
  AddEntry(m_metaData,"brickletMetaData.accountName",brickletMetaData.accountName);
  AddEntry(m_metaData,"brickletMetaData.userName",brickletMetaData.userName);
  // END session->getBrickletMetaData

  AddEntry(m_metaData,"sequenceID",session->getSequenceId(m_brickletPtr));
  AddEntry(m_metaData,"creationComment",session->getCreationComment(m_brickletPtr));
  AddEntry(m_metaData,"dimension",session->getDimensionCount(m_brickletPtr));
  AddEntry(m_metaData,"rootAxis",session->getRootAxisName(m_brickletPtr));

  // new in vernissage 2.1
  AddEntry(m_metaData,"rootAxisQualified",session->getRootAxisQualifiedName(m_brickletPtr));
  AddEntry(m_metaData,"triggerAxis",session->getTriggerAxisName(m_brickletPtr));
  AddEntry(m_metaData,"triggerAxisQualified",session->getTriggerAxisQualifiedName(m_brickletPtr));

  AddEntry(m_metaData,"channelName",session->getChannelName(m_brickletPtr));
  AddEntry(m_metaData,"channelInstanceName",session->getChannelInstanceName(m_brickletPtr));

  AddEntry(m_metaData,CHANNEL_UNIT_KEY,session->getChannelUnit(m_brickletPtr));
  AddEntry(m_metaData,"runCycleCount",session->getRunCycleCount(m_brickletPtr));

  AddEntry(m_metaData,"scanCycleCount",session->getScanCycleCount(m_brickletPtr));

  // new in vernissage 2.1
  // FIXME vernissage takes ages for the getDependingBricklets and friends calls
  // *and* internally locks the accesses so that we can't access it concurrently
  {
    const std::vector<int> idVec = convertRawBrickletVector(session->getDependingBricklets(m_brickletPtr));
    std::string dependentBricklets;
    joinString(idVec, listSepChar, dependentBricklets);

    AddEntry(m_metaData,"dependentBricklets",dependentBricklets);
  }

  // new in vernissage 2.1
  {
    const std::vector<int> idVec = convertRawBrickletVector(session->getReferencedBricklets(m_brickletPtr));
    std::string referencedBricklets;
    joinString(idVec, listSepChar, referencedBricklets);

    AddEntry(m_metaData,"referencedBricklets",referencedBricklets);
  }

  // new in vernissage 2.1
  {
    std::vector<int> idVec = convertRawBrickletVector(getBrickletSeries(m_brickletPtr));

    // we want to have a sorted list of bricklets
    std::sort(idVec.begin(), idVec.end());
    std::string brickletSeries;
    joinString(idVec, listSepChar, brickletSeries);

    AddEntry(m_metaData,"brickletSeries",brickletSeries);
  }

  const std::vector<std::wstring> elementInstanceNames = session->getExperimentElementInstanceNames(m_brickletPtr, L"");
  for (std::vector<std::wstring>::const_iterator it = elementInstanceNames.begin(); it != elementInstanceNames.end(); it++)
  {
   const ParameterMap elementInstanceParamsMap = session->getExperimentElementParameters(m_brickletPtr, *it);
   AddParameterMap(m_metaData,toString(*it),elementInstanceParamsMap);
  }

  // BEGIN Vernissage::Session::SpatialInfo
  const Vernissage::Session::SpatialInfo spatialInfo = session->getSpatialInfo(m_brickletPtr);
  for (std::vector<double>::const_iterator it = spatialInfo.physicalX.begin(); it != spatialInfo.physicalX.end(); it++)
  {
    const int index = it - spatialInfo.physicalX.begin() + 1 ; // 1-based index
    const std::string key = "spatialInfo.physicalX.No" + toString(index);
    AddEntry(m_metaData,key,*it);
  }
  for (std::vector<double>::const_iterator it = spatialInfo.physicalY.begin(); it != spatialInfo.physicalY.end(); it++)
  {
    const int index = it - spatialInfo.physicalY.begin() + 1 ; // 1-based index
    const std::string key = "spatialInfo.physicalY.No" + toString(index);
    AddEntry(m_metaData,key,*it);
  }

  if (spatialInfo.originatorKnown)
  {
    AddEntry(m_metaData,"spatialInfo.originatorKnown","true");
    AddEntry(m_metaData,"spatialInfo.channelName",spatialInfo.channelName);
    AddEntry(m_metaData,"spatialInfo.sequenceId",spatialInfo.sequenceId);
    AddEntry(m_metaData,"spatialInfo.runCycleCount",spatialInfo.runCycleCount);
    AddEntry(m_metaData,"spatialInfo.scanCycleCount",spatialInfo.scanCycleCount);
    AddEntry(m_metaData,"spatialInfo.viewName",spatialInfo.viewName);
    AddEntry(m_metaData,"spatialInfo.viewSelectionId",spatialInfo.viewSelectionId);
    AddEntry(m_metaData,"spatialInfo.viewSelectionIndex",spatialInfo.viewSelectionIndex);
  }
  else
  {
    AddEntry(m_metaData,"spatialInfo.originatorKnown","false");
    AddEntry(m_metaData,"spatialInfo.channelName","");
    AddEntry(m_metaData,"spatialInfo.sequenceId","");
    AddEntry(m_metaData,"spatialInfo.runCycleCount","");
    AddEntry(m_metaData,"spatialInfo.scanCycleCount","");
    AddEntry(m_metaData,"spatialInfo.viewName","");
    AddEntry(m_metaData,"spatialInfo.viewSelectionId","");
    AddEntry(m_metaData,"spatialInfo.viewSelectionIndex","");
  }
  //END Vernissage::Session::SpatialInfo

  // BEGIN Vernissage::Session::ExperimentInfo
  const Vernissage::Session::ExperimentInfo experimentInfo = session->getExperimentInfo(m_brickletPtr);
  AddEntry(m_metaData,"experimentInfo.Name",experimentInfo.experimentName);
  AddEntry(m_metaData,"experimentInfo.Version",experimentInfo.experimentVersion);
  AddEntry(m_metaData,"experimentInfo.Description",experimentInfo.experimentDescription);
  AddEntry(m_metaData,"experimentInfo.FileSpec",experimentInfo.experimentFileSpec);
  AddEntry(m_metaData,"experimentInfo.projectName",experimentInfo.projectName);
  AddEntry(m_metaData,"experimentInfo.projectVersion",experimentInfo.projectVersion);
  AddEntry(m_metaData,"experimentInfo.projectFileSpec",experimentInfo.projectFileSpec);
  // END Vernissage::Session::ExperimentInfo

  // create m_allAxes* internally
  getAxes<std::string>();
  std::string allAxesAsOneString;
  joinString(m_allAxesString, listSepChar, allAxesAsOneString);
  AddEntry(m_metaData,"allAxes",allAxesAsOneString);

  // BEGIN Vernissage::Session::axisDescriptor
  for (WstringVectorIt itAllAxes = m_allAxesWString.begin(); itAllAxes != m_allAxesWString.end(); itAllAxes++)
  {
    const std::wstring axisNameWString = *itAllAxes;
    const std::string axisNameString  = toString(*itAllAxes);

    const Vernissage::Session::AxisDescriptor axisDescriptor = session->getAxisDescriptor(m_brickletPtr, axisNameWString);
    AddEntry(m_metaData,axisNameString + ".clocks",axisDescriptor.clocks);
    AddEntry(m_metaData,axisNameString + ".mirrored",(axisDescriptor.mirrored ? "true" : "false"));
    AddEntry(m_metaData,axisNameString + ".physicalUnit",axisDescriptor.physicalUnit);
    AddEntry(m_metaData,axisNameString + ".physicalIncrement",axisDescriptor.physicalIncrement);
    AddEntry(m_metaData,axisNameString + ".physicalStart",axisDescriptor.physicalStart);
    AddEntry(m_metaData,axisNameString + ".rawIncrement",axisDescriptor.rawIncrement);
    AddEntry(m_metaData,axisNameString + ".rawStart",axisDescriptor.rawStart);
    AddEntry(m_metaData,axisNameString + ".triggerAxisName",axisDescriptor.triggerAxisName);
    // END Vernissage::Session::axisDescriptor

    // BEGIN Vernissage::Session:AxisTableSet
    const Vernissage::Session::AxisTableSets axisTableSetsMap = session->getAxisTableSets(m_brickletPtr, axisNameWString);

    // if it is empty, we got the standard table set which is [start=1,step=1,stop=clocks]
    if (axisTableSetsMap.empty())
    {
      AddEntry(m_metaData,axisNameString + ".AxisTableSet.count","1");
      AddEntry(m_metaData,axisNameString + ".AxisTableSetNo1.axis","");
      AddEntry(m_metaData,axisNameString + ".AxisTableSetNo1.start","1");
      AddEntry(m_metaData,axisNameString + ".AxisTableSetNo1.step","1");
      AddEntry(m_metaData,axisNameString + ".AxisTableSetNo1.stop",axisDescriptor.clocks);
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
          AddEntry(m_metaData,baseName,itMap->first);
          AddEntry(m_metaData,baseName + ".start",it->start);
          AddEntry(m_metaData,baseName + ".step",it->step);
          AddEntry(m_metaData,baseName + ".stop",it->stop);
        }
      }
      AddEntry(m_metaData,axisNameString + ".AxisTableSet.count",index);
    }
    // END Vernissage::Session:AxisTableSet

    // BEGIN Vernissage::AxisParameters introduced with Vernissage 2.2
    const ParameterMap axisParams = session->getAxisParameters(m_brickletPtr, axisNameWString);
    AddParameterMap(m_metaData,axisNameString,axisParams);
    // END Vernissage::AxisParameters
  }

  // BEGIN Vernissage::CalibrationInfo introduced with Vernissage 2.2
  {
    typedef std::map<std::wstring,Vernissage::Session::CalibrationInfo> CalibMap;
    typedef CalibMap::const_iterator CalibMapIt;
    const std::map<std::wstring,Vernissage::Session::CalibrationInfo> calibMap = session->getCalibrationInformation(m_brickletPtr);

    for (CalibMapIt it = calibMap.begin(); it != calibMap.end(); it++)
    {
      const std::string baseName            = "calibration." + toString(it->first);
      const std::string dataSet             = toString(it->second.dataSet);
      const std::string dataSetVariant      = toString(it->second.dataSetVariant);
      const std::string parameterSet        = toString(it->second.parameterSet);
      const std::string parameterSetVariant = toString(it->second.parameterSetVariant);
      AddEntry(m_metaData, baseName + ".dataSet", dataSet);
      AddEntry(m_metaData, baseName + ".dataSetVariant", dataSetVariant);
      AddEntry(m_metaData, baseName + ".parameterSetVariant", parameterSet);
      AddEntry(m_metaData, baseName + ".parameterSet", parameterSetVariant);
    }
  }
  // END Vernissage::CalibrationInfo

  DEBUGPRINT("Loaded %d keys/values as brickletMetaData for bricklet %d", m_metaData.size(), m_brickletID);
  ShrinkToFit(m_metaData);
}

/*
Wrapper function which returns a vector with the deployment parameters and their values
*/
const std::vector<StringPair>& Bricklet::getDeploymentParameter()
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
      HISTPRINT("Out of memory in getDeploymentParameter()");
      m_deployParams.clear();
    }
  }
  return m_deployParams;
}

/*
  Reads all deployment parameters into our own structures
*/
void Bricklet::loadDeploymentParameters()
{
  m_deployParams.clear();
  m_deployParams.reserve(RESERVE_SIZE);

  Vernissage::Session* session = getVernissageSession();

  const WstringVector elementInstanceNames = session->getExperimentElementInstanceNames(m_brickletPtr, L"");
  for (WstringVectorIt itInstance = elementInstanceNames.begin(); itInstance != elementInstanceNames.end(); itInstance++)
  {
    typedef std::map<std::wstring,std::wstring> ParamMap;
    typedef ParamMap::const_iterator ParamMapIt;
    const ParamMap paramMap = session->getExperimentElementDeploymentParameters(m_brickletPtr, *itInstance);
    for (ParamMapIt it = paramMap.begin(); it != paramMap.end(); it++)
    {
      const std::string key = toString(*itInstance) + "." + toString(it->first);
      AddEntry(m_deployParams,key,it->second);
    }
  }

  ShrinkToFit(m_deployParams);
}


// see Vernissage Manual page 20/21
// the idea here is to first get the root and triggerAxis for the Bricklet
// Then we know the starting point of the axis hierachy (rootAxis) and the endpoint (triggerAxis)
// In this way we can then traverse from the endpoint (triggerAxis) to the starting point (rootAxis) and record all axis names
// In more than 99% of the cases this routine will return one to three axes
// The value of maxRuns is strictly speaking wrong because the Matrix Software supports an unlimited number of axes, but due to pragmativ and safe coding reasons this has ben set to 100.
// The returned list will have the entries "triggerAxisName;axisNameWhichTriggeredTheTriggerAxis;...;rootAxisName"
void Bricklet::generateAllAxesVector()
{
  std::vector<std::wstring> allAxesWString;
  std::vector<std::string> allAxesString;

  unsigned int numRuns = 0;
  const unsigned int maxRuns = 100;

  Vernissage::Session* session = getVernissageSession();
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
const std::vector<std::wstring>& Bricklet::getAxes<std::wstring>()
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
const std::vector<std::string>& Bricklet::getAxes<std::string>()
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
void Bricklet::setBrickletPointer( void* const vernissageBricklet )
{
  THROW_IF_NULL(vernissageBricklet);
  m_brickletPtr = vernissageBricklet;
}

/*
  Does never return NULL
*/
void* Bricklet::getBrickletPointer() const
{
  return m_brickletPtr;
}

const ExtremaData& Bricklet::getExtrema() const
{
  return m_extrema;
}

int Bricklet::getRawDataSize()
{
  THROW_IF_NULL(m_rawBufferContents);
  return m_rawBufferContentsSize;
}

std::size_t Bricklet::getUsedMemory() const
{
  std::size_t usedMemMetaData = 0;
  std::size_t usedMemRawData = m_rawBufferContentsSize * sizeof(int);

  DEBUGPRINT("Memory usage of bricklet %d:", m_brickletID)
  DEBUGPRINT("Raw data %d KiB:", usedMemRawData / 1024)

  for (StringPairVectorCIt it = m_metaData.begin(); it != m_metaData.end(); it++)
  {
    usedMemMetaData += it->first.capacity();
    usedMemMetaData += it->second.capacity();
  }

  for (StringPairVectorCIt it = m_deployParams.begin(); it != m_deployParams.end(); it++)
  {
    usedMemMetaData += it->first.capacity();
    usedMemMetaData += it->second.capacity();
  }

  DEBUGPRINT("Meta data %d KiB:", usedMemMetaData / 1024)

  return sizeof(*this) + usedMemRawData + usedMemMetaData;
}
