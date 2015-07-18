/*
  The file utils_bricklet.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "utils_bricklet.hpp"
#include "wave.hpp"
#include "globaldata.hpp"
#include "utils_generic.hpp"
#include "bricklet.hpp"

namespace {

  /*
    Write the string waveNote as wave note into waveHandle
  */
  void setWaveNote(const std::string& waveNote, waveHndl waveHandle)
  {
    if (waveNote.empty())
    {
      HISTPRINT("BUG: got empty waveNote in setWaveNote.");
      return;
    }

    Handle noteHandle = NewHandle(waveNote.size()) ;

    if (noteHandle == NULL)
    {
      return;
    }

    int ret = PutCStringInHandle(waveNote.c_str(), noteHandle);
    if (ret != 0)
    {
      HISTPRINT("internal error %d, aborting", ret);
      return;
    }

    ASSERT_RETURN_VOID(waveHandle);
    SetWaveNote(waveHandle, noteHandle);
    // SetWaveNote takes care of diposing the handle
  }

  /*
    Return a string containing the standard wave note part
  */
  void setStandardWaveNote(std::stringstream& sstr, int brickletID /* = -1 */, int traceDir /* = -1 */, std::string suffix /* = std::string() */ )
  {
    sstr.precision(DBL_DIG);

    sstr << RESULT_FILE_NAME_KEY << '=' << unicodeToAnsi(GlobalData::Instance().getFileName()) << CR_CHAR;
    sstr << RESULT_DIR_PATH_KEY  << '=' << unicodeToAnsi(GlobalData::Instance().getDirPath())  << CR_CHAR;

    if (isValidBrickletID(brickletID))
    {
      sstr << BRICKLET_ID_KEY << '=' << brickletID << CR_CHAR;
    }
    else
    {
      sstr << BRICKLET_ID_KEY << '=' << CR_CHAR;
    }

    if (isValidTraceDir(traceDir))
    {
      sstr << TRACEDIR_KEY << '=' << traceDir << CR_CHAR;
    }
    else
    {
      sstr << TRACEDIR_KEY << '=' << CR_CHAR;
    }

    sstr << suffixName           << '=' << suffix                                 << CR_CHAR;
    sstr << "xopVersion="        << MatrixFileReader_XOP_VERSION_STR              << CR_CHAR;
    sstr << "vernissageVersion=" << GlobalData::Instance().getVernissageVersion() << CR_CHAR;
  }

  class IndexToString
  {
  public:
    virtual ~IndexToString(){}

    virtual std::string operator()(unsigned int index) const
    {
      const std::vector<std::string>& data = getData();

      if (index >= data.size())
      {
        DEBUGPRINT("BUG: viewTypeCodeToString got %d as parameter, but it should be between 0 and %d", index, data.size() - 1);
        return std::string();
      }
      else
      {
        return data.at(index);
      }
    }

  private:
    virtual const std::vector<std::string>& getData() const = 0;
  };

  class ViewTypeConverter : public IndexToString
  {
  public:
    ViewTypeConverter()
    {
      m_data.push_back(VTC_OTHER);
      m_data.push_back(VTC_SIMPLE2D);
      m_data.push_back(VTC_SIMPLE1D);
      m_data.push_back(VTC_FWDBWD2D);
      m_data.push_back(VTC_2DOF3D);
      m_data.push_back(VTC_SPECTROSCOPY);
      m_data.push_back(VTC_FORCECURVE);
      m_data.push_back(VTC_1DPROFILE);
      m_data.push_back(VTC_INTERFEROMETER);
      m_data.push_back(VTC_CONTINUOUSCURVE);
      m_data.push_back(VTC_PHASEAMPLITUDECUR);
      m_data.push_back(VTC_CURVESET);
      m_data.push_back(VTC_PARAMETERISEDCUR);
      m_data.push_back(VTC_DISCRETEENERGYMAP);
      m_data.push_back(VTC_ESPIMAGEMAP);
      m_data.push_back(VTC_DOWNWARD2D);
    }

  private:
    std::vector<std::string> m_data;
    const std::vector<std::string>& getData() const { return m_data; }
  };

  class BrickletTypeConverter : public IndexToString
  {
  public:
    BrickletTypeConverter()
    {
      m_data.push_back(BTC_UNKNOWN);
      m_data.push_back(BTC_SPMSPECTROSCOPY);
      m_data.push_back(BTC_ATOMMANIPULATION);
      m_data.push_back(BTC_1DCURVE);
      m_data.push_back(BTC_SPMIMAGE);
      m_data.push_back(BTC_PATHSPECTROSCOPY);
      m_data.push_back(BTC_ESPREGION);
      m_data.push_back(BTC_VOLUMECITS);
      m_data.push_back(BTC_DISCRETEENERGYMAP);
      m_data.push_back(BTC_FORCECURVE);
      m_data.push_back(BTC_PHASEAMPLITUDECUR);
      m_data.push_back(BTC_SIGNALOVERTIME);
      m_data.push_back(BTC_RAWPATHSPEC);
      m_data.push_back(BTC_ESPSNAPSHOTSEQ);
      m_data.push_back(BTC_ESPIMAGEMAP);
      m_data.push_back(BTC_INTERFEROMETERCUR);
      m_data.push_back(BTC_ESPIMAGE);
    }

  private:
    std::vector<std::string> m_data;
    const std::vector<std::string>& getData() const { return m_data; }
  };

  // lower limit of number of bricklets for which we don't bother with multi threading
  const int lowerLimitNumBrickletSingleThreaded = 50;

  void loadBrickletMetaDataRange(GlobalData& globalData, int first, int last)
  {
    for(int i = first; i <= last; i += 1)
    {
      globalData.getBricklet(i).getMetaData();
    }
  }

  void loadBrickletDataRange(GlobalData& globalData, int first, int last)
  {
    for(int i = first; i <= last; i += 1)
    {
      globalData.getBricklet(i).getRawData();
    }
  }

  void loadBrickletDataAndMetaDataRange(GlobalData& globalData, int first, int last)
  {
    for(int i = first; i <= last; i += 1)
    {
      globalData.getBricklet(i).getMetaData();
      globalData.getBricklet(i).getRawData();
    }
  }

  void loadThreadedIfRequired(boost::function<void(GlobalData&, int, int)> func)
  {
    //START_TIMER(1);
    const int numberOfBricklets = getVernissageSession()->getBrickletCount();

    if(numberOfBricklets < lowerLimitNumBrickletSingleThreaded)
    {
      func(GlobalData::Instance(), 1, numberOfBricklets);
      //STOP_TIMER(1);
      return;
    }

    const int numThreads = boost::numeric_cast<int>(boost::thread::hardware_concurrency());
    const int brickletsPerThread = numberOfBricklets / numThreads;

    boost::thread_group threadGroup;

    for (int i = 0; i < numThreads; i++)
    {
      int first = (i == 0 ? 1 : brickletsPerThread * i);
      int last  = (i == numThreads - 1 ? numberOfBricklets : brickletsPerThread * (i + 1) - 1);

      threadGroup.create_thread(boost::bind(func, boost::ref(GlobalData::Instance()), first, last));
    }

    threadGroup.join_all();
    //STOP_TIMER(1);
  }
} // anonymous namespace

/*
  Create a two column text wave from two string vectors
*/
int createAndFillTextWave(DataFolderHandle baseFolderHandle, const StringPairVector& data, DataFolderHandle dataFolderHandle, const char* waveName, int brickletID, std::string& waveNameList)
{
  // create 2D textwave with firstColumn.size() rows and 2 columns
  if (data.empty())
  {
    HISTPRINT("BUG: list size may not be zero");
    return UNKNOWN_ERROR;
  }

  CountInt dimensionSizes[MAX_DIMENSIONS + 1];
  MemClear(dimensionSizes, sizeof(dimensionSizes));

  dimensionSizes[ROWS] = data.size();
  dimensionSizes[COLUMNS] = 2;

  waveHndl waveHandle;
  int ret = MDMakeWave(&waveHandle, waveName, dataFolderHandle, dimensionSizes, TEXT_WAVE_TYPE, isOverwriteEnabled());

  if (ret == NAME_WAV_CONFLICT)
  {
    DEBUGPRINT("Wave %s already exists.", waveName);
    return WAVE_EXIST;
  }
  else if (ret != 0)
  {
    HISTPRINT("Error %d in creating wave %s.", ret, waveName);
    return UNKNOWN_ERROR;
  }

  // copy the strings of both columns into a new vector
  // so that they are then 1D
  // first: first, then: second
  std::vector<std::string> allColumns;
  try
  {
    allColumns.resize(data.size()*2);
  }
  catch (CMemoryException* e)
  {
    e->Delete();
    HISTPRINT("Out of memory in createAndFillTextWave()");
    return UNKNOWN_ERROR;
  }

  for (unsigned int i = 0; i < data.size(); i++)
  {
    allColumns[i] = data[i].first;
    allColumns[data.size()+i] = data[i].second;
  }

  ret = stringVectorToTextWave(allColumns, waveHandle);

  if (ret != 0)
  {
    HISTPRINT("stringVectorToTextWave returned %d", ret);
    return ret;
  }
  setOtherWaveNote(waveHandle, brickletID);
  appendToWaveList(baseFolderHandle, waveHandle, waveNameList);

  return 0;
}

/*
  Convert a vernissage viewtype code to a string
*/
std::string viewTypeCodeToString(unsigned int idx)
{
  ViewTypeConverter conv;
  return conv(idx);
}

/*
  Convert the bricklet type enumeration value into a human readable string
*/
std::string brickletTypeToString(unsigned int idx)
{
  BrickletTypeConverter conv;
  return conv(idx);
}

/*
  Set the appropriate wave note for data waves
*/
void setDataWaveNote( int brickletID, const Wave& waveData )
{
  std::stringstream note;
  setStandardWaveNote(note, brickletID, waveData.getTraceDir(), waveData.getSuffix());

  note << "rawMin="                << waveData.getExtrema().getRawMin()        << CR_CHAR;
  note << "rawMax="                << waveData.getExtrema().getRawMax()        << CR_CHAR;
  note << "physicalValueOfRawMin=" << waveData.getExtrema().getPhysValRawMin() << CR_CHAR;
  note << "physicalValueOfRawMax=" << waveData.getExtrema().getPhysValRawMax() << CR_CHAR;
  note << "pixelSize="             << waveData.GetPixelSize()                  << CR_CHAR;

  setWaveNote(note.str(), waveData.getWaveHandle());
}

/*
  Set the appropriate wave note for the other waves (bricklet metadata, resultfile meta data, overviewtable)
*/
void setOtherWaveNote(waveHndl waveHandle, int brickletID /*= -1*/, int traceDir  /*= -1*/, std::string suffix /* = std::string() */ )
{
  std::stringstream note;
  setStandardWaveNote(note, brickletID, traceDir, suffix);
  setWaveNote(note.str(), waveHandle);
}

/*
  Check if the bricklet range startID-endID is valid
*/
bool isValidBrickletRange(double startID, double endID, int numberOfBricklets)
{
  // brickletIDs are 1-based
  return (startID <=  endID
          && startID >=  1
          && endID   >=  1
          && startID <= numberOfBricklets
          && endID   <= numberOfBricklets);
}

/*
  A valid brickletID is a positive and non-zero number
*/
bool isValidBrickletID(int brickletID)
{
  return (brickletID > 0);
}

/*
  The trace direction knows four different states defined in constants.h.
  In accordance with vernissage the range is from TRACE_UP(=0) to RE_TRACE_DOWN(3)
*/
bool isValidTraceDir(int traceDir)
{
  return (traceDir >= TRACE_UP && traceDir <= RE_TRACE_DOWN);
}

/*
  Convenience helper
*/
Vernissage::Session* getVernissageSession()
{
  return GlobalData::Instance().getVernissageSession();
}

int isOverwriteEnabled()
{
  return static_cast<int>(GlobalData::Instance().isOverwriteEnabled());
}

/*
  Returns an integer which tells if we should create single or double precision waves
  the integer can be readily used with MDMakeWave
*/
int getIgorWaveType()
{
  return (GlobalData::Instance().isDoubleWaveEnabled() ? NT_FP64 : NT_FP32);
}

/*
  Returns a vector of all bricklets which are part of the series of rawBrickletPtr (also included).
  The returned vector is not sorted.
*/
std::vector<void*> getBrickletSeries( void* rawBrickletPtr )
{
  std::vector<void*> brickeltSeries;

  if (rawBrickletPtr == NULL)
  {
    return brickeltSeries;
  }

  // get all predecessors
  void* p =  rawBrickletPtr;

  while ((p = getVernissageSession()->getPredecessorBricklet(p)) != NULL)
  {
    brickeltSeries.push_back(p);
  }

  // add the bricklet itself
  brickeltSeries.push_back(rawBrickletPtr);

  // get all successors
  p =  rawBrickletPtr;

  while ((p = getVernissageSession()->getSuccessorBricklet(p)) != NULL)
  {
    brickeltSeries.push_back(p);
  }

  return brickeltSeries;
}

/*
  Load the meta data of all bricklets maybe using threads
*/
void loadAllBrickletMetaData()
{
  loadThreadedIfRequired(&loadBrickletMetaDataRange);
}

/*
  Load the raw data of all bricklets maybe using threads
*/
void loadAllBrickletData()
{
  loadThreadedIfRequired(&loadBrickletDataRange);
}

/*
  Load the raw and meta data of all bricklets maybe using threads
*/
void loadAllBrickletDataAndMetaData()
{
  loadThreadedIfRequired(&loadBrickletDataAndMetaDataRange);
}
