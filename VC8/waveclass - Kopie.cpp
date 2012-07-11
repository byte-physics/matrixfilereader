/*
  The file wavedata.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "waveclass.hpp"
#include "globaldata.hpp"
#include "utils_generic.hpp"

template<typename T>
WaveClass<T>::WaveClass()
  :
  m_extrema(ExtremaData())
{
  Init();
}

template<typename T>
WaveClass<T>::WaveClass( const ExtremaData& extremaData )
  :
  m_extrema(extremaData)
{
  Init();
}

// Common initialization routine for all ctors
template<typename T>
void WaveClass<T>::Init()
{
  m_traceDir = NO_TRACE;
  moreData = false;
  m_doublePtr = NULL;
  m_floatPtr = NULL;
  m_waveHandle = NULL;
  pixelSize = 1;
}

template<typename T>
WaveClass<T>::~WaveClass(){}

/*
  Make a connection between the waveHandle and this class
*/
template<typename T>
void WaveClass<T>::setWaveHandle(const waveHndl& waveHandle)
{
  if (WaveType(waveHandle) & NT_FP64)
  {
    m_doublePtr = getWaveDataPtr<double>(waveHandle);
    m_floatPtr = NULL;

    if (m_doublePtr == NULL)
    {
      HISTPRINT("BUG: setWaveHandle(...) m_doublePtr is NULL, this should not happen...");
    }
    else
    {
      moreData = true;
      m_waveHandle = waveHandle;
    }
  }
  else if (WaveType(waveHandle) & NT_FP32)
  {
    m_doublePtr = NULL;
    m_floatPtr = getWaveDataPtr<float>(waveHandle);

    if (m_floatPtr == NULL)
    {
      HISTPRINT("BUG: setWaveHandle(...) m_floatPtr is NULL, this should not happen...");
    }
    else
    {
      moreData = true;
      m_waveHandle = waveHandle;
    }
  }
  else if (WaveType(waveHandle) & NT_I32)
  {
    m_doublePtr = NULL;
    m_floatPtr = NULL;

    m_waveHandle = waveHandle;
  }
  else
  {
    HISTPRINT("wrong datatype in setWaveHandle");
  }
}

/*
  Set the name and trace direction of the wave, adds then the appropriate suffix to the wave name
*/
template<typename T>
void WaveClass<T>::setNameAndTraceDir(const std::string& basename, int traceDir)
{
  m_traceDir = traceDir;

  switch (m_traceDir)
  {
  case NO_TRACE:
    m_wavename = basename;
    break;

  case TRACE_UP:
    m_wavename = basename + TRACE_UP_STRING;
    break;

  case RE_TRACE_UP:
    m_wavename = basename + RE_TRACE_UP_STRING;
    break;

  case TRACE_DOWN:
    m_wavename = basename + TRACE_DOWN_STRING;
    break;

  case RE_TRACE_DOWN:
    m_wavename = basename + RE_TRACE_DOWN_STRING;
    break;

  default:
    HISTPRINT("BUG: setNameAndTraceDir() tracedir is out of range");
    break;
  }
}

/*
  Output debug info
*/
template<typename T>
void WaveClass<T>::printDebugInfo()
{
  DEBUGPRINT("%s: waveHandle=%p, float=%p, double=%p, moreData=%s",
             m_wavename.empty() ? "empty" : m_wavename.c_str(), m_waveHandle, m_floatPtr, m_doublePtr, moreData ? "true" : "false");
}

/*
  Sets the complete wave to NaN
*/
template<typename T>
void WaveClass<T>::clearWave()
{
  if (m_doublePtr)
  {
    waveClearNaN64(m_doublePtr, WavePoints(m_waveHandle));
  }
  else if (m_floatPtr)
  {
    waveClearNaN32(m_floatPtr, WavePoints(m_waveHandle));
  }
}

/*
  convenience wrapper
*/
template<typename T>
void WaveClass<T>::setWaveScaling(int dimension, const double* sfAPtr, const double* sfBPtr)
{
  ASSERT_RETURN_VOID(m_waveHandle);

  int ret = MDSetWaveScaling(m_waveHandle, dimension, sfAPtr, sfBPtr);

  if (ret != 0)
  {
    HISTPRINT("WaveClass::setWaveScaling returned error %d", ret);
  }
}

/*
  convenience wrapper
*/
template<typename T>
void WaveClass<T>::setWaveUnits(int dimension, const std::string& units)
{
  ASSERT_RETURN_VOID(m_waveHandle);

  int ret = MDSetWaveUnits(m_waveHandle, dimension, units.c_str());

  if (ret != 0)
  {
    HISTPRINT("WaveClass::setWaveUnits returned error %d", ret);
  }
}

template<typename T>
void WaveClass<T>::setWaveUnits(int dimension, const std::wstring& units)
{
  setWaveUnits(dimension, WStringToString(units));
}

template<typename T>
const ExtremaData& WaveClass<T>::getExtrema() const
{
  return m_extrema;
}

template<typename T>
bool WaveClass<T>::isEmpty() const
{
  return m_wavename.empty();
}

template<typename T>
int WaveClass<T>::getTraceDir() const
{
  return m_traceDir;
}

template<typename T>
const char* WaveClass<T>::getWaveName() const
{
  return m_wavename.c_str();
}

template<typename T>
waveHndl WaveClass<T>::getWaveHandle() const
{
  return m_waveHandle;
}

// eplicit template instantiations
template<> class WaveClass<double>;
template<> class WaveClass<float>;
