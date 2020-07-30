/*
  The file globaldata.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "globaldata.hpp"
#include "dllhandler.hpp"
#include "bricklet.hpp"
#include "utils_generic.hpp"
#include "utils_bricklet.hpp"

namespace
{

typedef RawBrickletToIDMap::const_iterator MapCIt;
typedef BrickletPtrVector::iterator BrickletVectorIt;
typedef BrickletPtrVector::const_iterator BrickletVectorCIt;

template <typename T>
T getSettingOrDefault(DataFolderHandle dataFolderHndl, const char *option_name, bool default_value);

template <>
double getSettingOrDefault<double>(DataFolderHandle dataFolderHndl, const char *option_name, bool default_value)
{
  int objType;
  DataObjectValue objValue;
  double value = default_value;

  const int ret = GetDataFolderObject(dataFolderHndl, option_name, &objType, &objValue);

  if(ret == 0 && objType == VAR_OBJECT)
  {
    value = objValue.nv.realValue;
    DEBUGPRINT("%s=%d", option_name, value);
  }
  else
  {
    // variable does not exist or is of wrong type
    DEBUGPRINT("%s=%d (default)", option_name, value);
  }

  return value;
}

template <>
bool getSettingOrDefault<bool>(DataFolderHandle dataFolderHndl, const char *option_name, bool default_value)
{
  return doubleToBool(getSettingOrDefault<double>(dataFolderHndl, option_name, default_value));
}

template <>
int getSettingOrDefault<int>(DataFolderHandle dataFolderHndl, const char *option_name, bool default_value)
{
  return int(getSettingOrDefault<double>(dataFolderHndl, option_name, default_value));
}
} // anonymous namespace

GlobalData::GlobalData()
    : m_VernissageSession(NULL), m_lastError(UNKNOWN_ERROR), m_debug(debug_default), m_doubleWave(double_default),
      m_overwrite(overwrite_default), m_datafolder(datafolder_default), m_datacache(cache_default),
      m_errorToHistory(false)
{
  // initialize encoding conversion class
  EncodingConversion::Instance();
  m_bricklets.reserve(RESERVE_SIZE);
}

GlobalData::~GlobalData()
{
}

// store name and path of the open result file
void GlobalData::setResultFile(const std::wstring &dirPath, const std::wstring &fileName)
{
  if(resultFileOpen())
  {
    HISTPRINT("BUG: there is already a result file open, please close that first");
    return;
  }

  m_resultDirPath  = dirPath;
  m_resultFileName = fileName;
}

const std::wstring &GlobalData::getFileName() const
{
  return m_resultFileName;
}

const std::wstring &GlobalData::getDirPath() const
{
  return m_resultDirPath;
}

/*
  get a pointer to the vernissage session object
  automatically loads the vernissage DLL if there is no such object
*/
Vernissage::Session *GlobalData::getVernissageSession()
{
  if(m_VernissageSession != NULL)
  {
    return m_VernissageSession;
  }
  else
  {
    m_VernissageSession = m_DLLHandler.createSessionObject();
    THROW_IF_NULL(m_VernissageSession);
    return m_VernissageSession;
  }
}

/*
  closes a result file, deletes all internal objects associated with that result file
*/
void GlobalData::closeResultFile()
{
  m_bricklets.clear();
  m_rawToBrickletID.clear();

  // remove opened result set from internal database
  if(m_VernissageSession)
  {
    m_VernissageSession->eraseResultSets();
  }

  // erase filenames
  m_resultFileName.erase();
  m_resultDirPath.erase();
}

/*
  closes the session and unloads the DLL
  will only be called when we receive the CLEANUP signal
*/
void GlobalData::closeSession()
{
  closeResultFile();
  m_DLLHandler.closeSession();
  m_VernissageSession = NULL;
}

/*
  Returns a version string identifying the vernissage DLL version
*/
const std::string &GlobalData::getVernissageVersion()
{
  if(m_VernissageSession == NULL)
  {
    getVernissageSession();
  }

  return m_DLLHandler.getVernissageVersion();
}

bool GlobalData::resultFileOpen() const
{
  return (!m_resultFileName.empty());
}

Bricklet &GlobalData::getBricklet(int brickletID)
{
  if(!isValidBrickletID(brickletID) || brickletID >= boost::numeric_cast<int>(m_bricklets.size()) ||
     !m_bricklets[brickletID])
  {
    throw std::runtime_error("The requested bricklet " + toString(brickletID) + " does not exist");
  }

  return *m_bricklets[brickletID];
}

/*
  For each bricklet we have to call this function and make the connection between the vernissageBricklet pointer
  from the vernissage DLL and our Bricklet objects
*/
void GlobalData::createBricklet(int brickletID, void *const vernissageBricklet)
{
  THROW_IF_NULL(vernissageBricklet);

  try
  {
    const int numBricklets      = boost::numeric_cast<int>(m_bricklets.size());
    const int totalNumBricklets = getVernissageSession()->getBrickletCount();

    if(brickletID >= numBricklets)
    {
      if(totalNumBricklets < numBricklets)
      {
        throw std::runtime_error("BUG: Number of bricklets must not shrink.");
      }

      DEBUGPRINT("createBricklet: Increasing size of m_bricklets");

      // we reserve more than we currently need
      // m_bricklets[0] is always unused, so we need to create a bye one larger vector
      m_bricklets.reserve(totalNumBricklets * 2);
      m_bricklets.resize(totalNumBricklets + 1);
    }

    if(m_bricklets[brickletID])
    {
      throw std::runtime_error("Trying to overwrite brickletID " + toString((brickletID)));
    }

    m_bricklets[brickletID]               = boost::make_shared<Bricklet>(brickletID, vernissageBricklet);
    m_rawToBrickletID[vernissageBricklet] = brickletID;

    DEBUGPRINT("createBricklet brickletID=%d,vernissageBricklet=%p", brickletID, vernissageBricklet);
  }
  catch(CMemoryException *e)
  {
    HISTPRINT("Out of memory in createBrickletClassObject\r");
    throw e;
  }
}

/*
  Update the vernissage bricklet pointer for Bricklet brickletID to vernissageBricklet
*/
void GlobalData::updateBricklet(int brickletID, void *const vernissageBricklet)
{
  Bricklet &bricklet                = getBricklet(brickletID);
  void *const oldVernissageBricklet = bricklet.getBrickletPointer();

  if(oldVernissageBricklet == vernissageBricklet)
  {
    return;
  }

  DEBUGPRINT("Updating vernissage bricklet pointer from %p to %p", oldVernissageBricklet, vernissageBricklet);

  m_rawToBrickletID.erase(oldVernissageBricklet);
  m_rawToBrickletID[vernissageBricklet] = brickletID;
  bricklet.setBrickletPointer(vernissageBricklet);
}

/*
  errors which are not recoverable should be told the user by calling setInternalError
*/
void GlobalData::setInternalError(int errorValue)
{
  DEBUGPRINT("BUG: xop internal error %d returned.", errorValue);

  char errorMessage[256]; // 256 is taken from the GetIgorErrorMessage declaration
  const int ret = GetIgorErrorMessage(errorValue, errorMessage);

  if(ret == 0)
  {
    HISTPRINT(errorMessage);
  }
}

void GlobalData::finalizeWithFilledCache()
{
  finalize();

  if(!isDataCacheEnabled())
  {
    for(BrickletVectorIt it = m_bricklets.begin() + 1; it != m_bricklets.end(); it++)
    {
      (*it)->clearCache();
    }
  }
}

/*
  Must be called by every operation which sets V_flag before returning
*/
void GlobalData::finalize()
{
  setError(SUCCESS);
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

  setError(UNKNOWN_ERROR);
  m_errorToHistory = (calledFromMacro == 0 && calledFromFunction == 0);
}

/*
  Must be called by every operation which sets V_flag and does depend on the V_MatrixFileReader*
  variables defined in constants.h
*/
void GlobalData::initialize(int calledFromMacro, const int calledFromFunction)
{
  readSettings();
  initializeWithoutReadSettings(calledFromMacro, calledFromFunction);
}

/*
  Takes care of setting V_flag to the current error value
  Should be called immediately before calling return in a operation
*/
void GlobalData::setError(int errorCode, const std::string &argument /*= std::string()*/)
{
  if(errorCode < SUCCESS || errorCode > WAVE_EXIST)
  {
    HISTPRINT("BUG: errorCode is out of range");
    m_lastError = UNKNOWN_ERROR;
    return;
  }

  m_lastError = errorCode;

  int ret = SetOperationNumVar(V_flag, static_cast<double>(errorCode));

  if(ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return;
  }

  m_lastErrorArgument = argument.empty() ? "(missing argument)" : argument;

  DEBUGPRINT("lastErrorCode %d, argument %s", errorCode, argument.c_str());

  if(m_errorToHistory && errorCode != SUCCESS)
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

  switch(errorCode)
  {
  case SUCCESS:
    msg = "No error, everything went nice and smooth.";
    break;

  case UNKNOWN_ERROR:
    msg = "A strange and unknown error happened. It might be appropriate to file a bug report at " +
          std::string(PROJECTURL) + ".";
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
    msg = "The rawdata could not be interpreted. You can try using getRawBrickleData() instead and consulting the "
          "vernissage documentation. Please file also a bug report and attach your data.";
    break;

  case NO_FILE_OPEN:
    msg = "There is no result file open.";
    break;

  case INVALID_RANGE:
    msg = "The brickletID range was wrong. brickletIDs have to lie between 1 and numberOfBricklets, and "
          "startBrickletID may not be bigger than endBrickletID.";
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
  m_debug = getSettingOrDefault<bool>(dataFolderHndl, debug_option, debug_default);

  if(m_debug)
  {
    char dataFolderPath[MAXCMDLEN + 1];
    // flags=3 returns the full path to the datafolder and including quotes if needed
    const int ret = GetDataFolderNameOrPath(dataFolderHndl, 3, dataFolderPath);

    if(ret == 0)
    {
      DEBUGPRINT("Variables in the folder %s:", dataFolderPath);
    }
  }

  m_overwrite  = getSettingOrDefault<bool>(dataFolderHndl, overwrite_option, overwrite_default);
  m_doubleWave = getSettingOrDefault<bool>(dataFolderHndl, double_option, double_default);
  m_datafolder = getSettingOrDefault<bool>(dataFolderHndl, datafolder_option, datafolder_default);
  m_datacache  = getSettingOrDefault<bool>(dataFolderHndl, cache_option, cache_default);
  m_magic      = getSettingOrDefault<int>(dataFolderHndl, magic_option, magic_default);
}

/*
  Returns a brickletID for the vernissage APIs raw bricklet pointer
*/
int GlobalData::convertBrickletPtr(void *rawBrickletPtr) const
{
  MapCIt it = m_rawToBrickletID.find(rawBrickletPtr);
  if(it != m_rawToBrickletID.end())
  {
    return it->second;
  }

  HISTPRINT("BUG: Could not find a corresponding brickletID for the raw pointer %p", rawBrickletPtr);
  return INVALID_BRICKLETID;
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

bool GlobalData::isOverwriteEnabled() const
{
  return m_overwrite;
}

int GlobalData::magicSetting() const
{
  return m_magic;
}

std::size_t GlobalData::getUsedMemory() const
{
  std::size_t usedMemInBytes = sizeof(*this);

  for(std::size_t i = 1; i < m_bricklets.size(); i++)
  {
    usedMemInBytes += m_bricklets[i]->getUsedMemory();
  }

  return usedMemInBytes;
}
