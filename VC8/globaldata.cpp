/*
  The file globaldata.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "globaldata.hpp"
#include "dllhandler.hpp"
#include "brickletclass.hpp"
#include "utils_generic.hpp"

namespace  {

  bool getSettingOrDefault( DataFolderHandle dataFolderHndl, const char* option_name, bool default_value)
  {
    int objType;
    DataObjectValue objValue;
    bool value = default_value;

    const int ret = GetDataFolderObject(dataFolderHndl, option_name, &objType, &objValue);

    if (ret == 0 && objType == VAR_OBJECT)
    {
      value = doubleToBool(objValue.nv.realValue);
      DEBUGPRINT("debug=%d", value);
    }
    else // variable does not exist or is of wrong type
      DEBUGPRINT("debug=%d (default)", value);

    return value;
  }

} // anonymous namespace 

GlobalData::GlobalData()
  :
  m_VernissageSession(NULL),
  m_lastError(UNKNOWN_ERROR),
  m_debug(debug_default),
  m_doubleWave(double_default),
  m_overwrite(overwrite_default),
  m_datafolder(datafolder_default),
  m_datacache(cache_default),
  m_errorToHistory(false)
{}

GlobalData::~GlobalData()
{}

// store name and path of the open result file
void GlobalData::setResultFile(const std::wstring& dirPath, const std::wstring& fileName)
{
  if (this->resultFileOpen())
  {
    HISTPRINT("BUG: there is already a result file open, please close that first");
    return;
  }

  m_resultDirPath = dirPath;
  m_resultFileName = fileName;
}

/*
  get a pointer to the vernissage session object
  automatically loads the vernissage DLL if there is no such object
*/
Vernissage::Session* GlobalData::getVernissageSession()
{
  if (m_VernissageSession != NULL)
  {
    return m_VernissageSession;
  }
  else
  {
    m_VernissageSession = m_DLLHandler.createSessionObject();
    return m_VernissageSession;
  }
}

/*
  closes a result file, deletes all internal objects associated with that result file
*/
void GlobalData::closeResultFile()
{
  //delete BrickletClass objects
  for (IntBrickletClassPtrMap::iterator it = m_brickletIDBrickletClassMap.begin(); it != m_brickletIDBrickletClassMap.end(); it++)
  {
    delete it->second;
    it->second = NULL;
  }

  // empty bricklet map
  m_brickletIDBrickletClassMap.clear();

  // remove opened result set from internal database
  if (m_VernissageSession)
  {
    m_VernissageSession->eraseResultSets();
  }

  // erase filenames
  m_resultFileName.erase();
  m_resultDirPath.erase();
}

/*
  closes the session and unloads the DLL
  will only be called when we receivce the CLEANUP signal
*/
void GlobalData::closeSession()
{
  this->closeResultFile();
  m_DLLHandler.closeSession();
  m_VernissageSession = NULL;
}

// return a version string identifying the vernissage DLL version
const std::string& GlobalData::getVernissageVersion()
{
  if (m_VernissageSession == NULL)
  {
    this->getVernissageSession();
  }

  return m_DLLHandler.getVernissageVersion();
}

bool GlobalData::resultFileOpen() const
{
  return (!m_resultFileName.empty());
}

template<>
std::string GlobalData::getDirPath<std::string>() const
{
  return toString(m_resultDirPath);
}

template<>
std::wstring GlobalData::getDirPath<std::wstring>() const
{
  return m_resultDirPath;
}

template<>
std::string GlobalData::getFileName<std::string>() const
{
  return toString(m_resultFileName);
}

template<>
std::wstring GlobalData::getFileName<std::wstring>() const
{
  return m_resultFileName;
}

/*
  returns an integer which tells if we should create single or double precision waves
  the integer can be readily used with MDMakeWave
*/
int GlobalData::getIgorWaveType() const
{
  return (m_doubleWave ? NT_FP64 : NT_FP32);
}

/*
  m_brickletIDBrickletClassMap maps a brickletID to a brickletClass pointer
  returns NULL if the bricklet with brickletID can not be found
*/
BrickletClass* GlobalData::getBrickletClassObject(int brickletID) const
{
  IntBrickletClassPtrMap::const_iterator it = m_brickletIDBrickletClassMap.find(brickletID);

  if (it != m_brickletIDBrickletClassMap.end()) // we found the element
  {
    return it->second;
  }

  return NULL;
}

/*
  for each bricklet we have to call this function and make the connection between the pBricklet pointer
  from the vernissage DLL and our brickletClass objects
*/
void GlobalData::createBrickletClassObject(int brickletID, void* const pBricklet)
{
  BrickletClass* bricklet = NULL;

  try
  {
    bricklet = new BrickletClass(pBricklet, brickletID);
  }
  catch (CMemoryException* e)
  {
    e->Delete();
    HISTPRINT("Out of memory in createBrickletClassObject\r");
    throw e;
  }
  m_brickletIDBrickletClassMap[brickletID] = bricklet;

  DEBUGPRINT("setBrickletPointerMap brickletID=%d,pBricklet=%p", brickletID, pBricklet);
}

/*
  errors which are not recoverable should be told the user by calling setInternalError
*/
void GlobalData::setInternalError(int errorValue)
{
  DEBUGPRINT("BUG: xop internal error %d returned.", errorValue);

  char errorMessage[256];   // 256 is taken from the GetIgorErrorMessage declaration
  const int ret = GetIgorErrorMessage(errorValue, errorMessage);

  if (ret == 0)
  {
    HISTPRINT(errorMessage);
  }
}
/*
  Must be called by every operation which sets V_flag before returning
  In case some internal cache got filled by the operation, filledCache=true has to be passed
*/
void GlobalData::finalize(bool filledCache /* = false */, int errorCode /* = SUCCESS */)
{
  this->setError(errorCode);

  if (!isDataCacheEnabled() && filledCache)
  {
    int totalBrickletCount = m_VernissageSession->getBrickletCount();

    for (int i = 1; i <= totalBrickletCount; i++)
    {
      BrickletClass* bricklet = GlobalData::Instance().getBrickletClassObject(i);
      ASSERT_RETURN_VOID(bricklet);
      bricklet->clearCache();
    }
  }
}

/*
  Must be called by every operation which sets V_flag and does not want to read the V_MatrixFileReader*
  variables defined in constans.h. This is the case if these variables are not relevant or if the
  operation has the /DEST parameter, in that case the readsettings call must be delayed to use the
  correct datafolder for the variables.
*/
void GlobalData::initializeWithoutReadSettings(int calledFromMacro, int calledFromFunction)
{
  // set it temporary to false, otherwise the setError in the next line causes the error message to be printed
  m_errorToHistory = false;

  this->setError(UNKNOWN_ERROR);
  m_errorToHistory = (calledFromMacro == 0 && calledFromFunction == 0);
}

/*
  Must be called by every operation which sets V_flag and does depend on the V_MatrixFileReader*
  variables defined in constans.h
*/
void GlobalData::initialize(int calledFromMacro, const int calledFromFunction)
{
  this->readSettings();
  this->initializeWithoutReadSettings(calledFromMacro, calledFromFunction);
}

/*
  Takes care of setting V_flag to the current error value
  Should be called immediately before calling return in a operation
*/
void GlobalData::setError(int errorCode, const std::string& argument /*= std::string()*/)
{
  if (errorCode < SUCCESS || errorCode > WAVE_EXIST)
  {
    HISTPRINT("BUG: errorCode is out of range");
    m_lastError = UNKNOWN_ERROR;
    return;
  }

  m_lastError = errorCode;

  int ret = SetOperationNumVar(V_flag, static_cast<double>(errorCode));

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return;
  }

  m_lastErrorArgument = argument.empty() ? "(missing argument)" : argument;

  DEBUGPRINT("lastErrorCode %d, argument %s", errorCode, argument.c_str());

  if (m_errorToHistory && errorCode != SUCCESS)
  {
    HISTPRINT(getLastErrorMessage().c_str());
  }
}

/*
  Get a string with the last error message
*/
std::string GlobalData::getLastErrorMessage() const
{
  return getErrorMessage(m_lastError);
}

/*
  Translate a error code to a human readable error message
*/
std::string GlobalData::getErrorMessage(int errorCode) const
{
  std::string msg;

  switch (errorCode)
  {
  case SUCCESS:
    msg = "No error, everything went nice and smooth.";
    break;

  case UNKNOWN_ERROR:
    msg = "A strange and unknown error happened. It might be appropriate to file a bug report at " + std::string(PROJECTURL) + ".";
    break;

  case ALREADY_FILE_OPEN:
    msg = "A file is already open and it can only be one file open at a time.";
    break;

  case EMPTY_RESULTFILE:
    msg = "The result file is empty, so there is little one can do here...";
    break;

  case FILE_NOT_READABLE:
    msg = "The file/folder " + m_lastErrorArgument + " is not readable.";
    break;

  case NO_NEW_BRICKLETS:
    msg = "There are no new bricklets in the result file.";
    break;

  case WRONG_PARAMETER:
    msg = "The paramter " + m_lastErrorArgument + " is missing or wrong. Please consult the documentation.";
    break;

  case INTERNAL_ERROR_CONVERTING_DATA:
    msg = "The rawdata could not be interpreted. You can try using getRawBrickleData() instead and consulting the vernissage documentation. Please file also a bug report and attach your data.";
    break;

  case NO_FILE_OPEN:
    msg = "There is no result file open.";
    break;

  case INVALID_RANGE:
    msg = "The brickletID range was wrong. brickletIDs have to lie between 1 and numberOfBricklets, and startBrickletID may not be bigger than endBrickletID.";
    break;

  case WAVE_EXIST:
    msg = "The wave " + m_lastErrorArgument + " already exists. Please move/delete it first.";
    break;

  default:
    msg = "BUG: unknown error code";
    break;
  }

  return msg;
}

/*
  Reads variables in the data folder dataFolderHndl. In case dataFolderHndl is
  null the current data folder is used.
*/
void GlobalData::readSettings(DataFolderHandle dataFolderHndl /* = NULL */)
{
  m_debug = getSettingOrDefault(dataFolderHndl,debug_option,debug_default);

  if (m_debug)
  {
    char dataFolderPath[MAXCMDLEN + 1];
    // flags=3 returns the full path to the datafolder and including quotes if needed
    const int ret = GetDataFolderNameOrPath(dataFolderHndl, 3, dataFolderPath);

    if (ret == 0)
    {
      DEBUGPRINT("V_MatrixFileReader* variables in the folder %s:", dataFolderPath);
    }
  }

  m_overwrite  = getSettingOrDefault(dataFolderHndl,overwrite_option,overwrite_default);
  m_doubleWave = getSettingOrDefault(dataFolderHndl,double_option,double_default);
  m_datafolder = getSettingOrDefault(dataFolderHndl,datafolder_option,datafolder_default);
  m_datacache  = getSettingOrDefault(dataFolderHndl,cache_option,cache_default);
}

/*
  Converts a vector of vernissage APIs raw bricklet pointers to a vector of brickletIDs
*/
std::vector<int> GlobalData::convertBrickletPtr(const std::vector<void*>& rawBrickletPtrs)
{
  std::vector<int> brickletIDs;
  brickletIDs.reserve(rawBrickletPtrs.size());

  typedef std::vector<void*>::const_iterator ItType;
  for (ItType it = rawBrickletPtrs.begin(); it != rawBrickletPtrs.end(); it++)
  {
    const int brickletID = convertBrickletPtr(*it);
    brickletIDs.push_back(brickletID);
  }

  return brickletIDs;
}

/*
  Returns a brickletID for the vernissage APIs raw bricklet pointer
*/
int GlobalData::convertBrickletPtr(void* rawBrickletPtr)
{
  for (IntBrickletClassPtrMap::const_iterator it = m_brickletIDBrickletClassMap.begin(); it != m_brickletIDBrickletClassMap.end(); it++)
  {
    if (it->second->getBrickletPointer() == rawBrickletPtr)
    {
      return it->first;
    }
  }

  HISTPRINT(outputBuffer, "BUG: Could not find a corresponding brickletID for the raw pointer %p", rawBrickletPtr);
  return INVALID_BRICKLETID;
}

/*
  Returns a vector of all bricklets which are part of the series of rawBrickletPtr (also included).
  The returned vector is not sorted.
*/
std::vector<void*> GlobalData::getBrickletSeries(void* rawBrickletPtr)
{
  void* p = NULL;
  std::vector<void*> brickeltSeries;

  if (rawBrickletPtr == NULL)
  {
    return brickeltSeries;
  }

  // get all predecessors
  p =  rawBrickletPtr;

  while ((p = m_VernissageSession->getPredecessorBricklet(p)) != NULL)
  {
    brickeltSeries.push_back(p);
  }

  // add the bricklet itself
  brickeltSeries.push_back(rawBrickletPtr);

  // get all successors
  p =  rawBrickletPtr;

  while ((p = m_VernissageSession->getSuccessorBricklet(p)) != NULL)
  {
    brickeltSeries.push_back(p);
  }

  return brickeltSeries;
}

bool GlobalData::isDataCacheEnabled() const
{
  return m_datacache;
}

bool GlobalData::isDatafolderEnabled() const
{
  return m_datafolder;
}

bool GlobalData::isDebuggingEnabled() const
{
  return m_debug;
}

bool GlobalData::isDoubleWaveEnabled() const
{
  return m_doubleWave;
}
