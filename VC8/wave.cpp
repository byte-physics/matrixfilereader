/*
  The file wavedata.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "wave.hpp"
#include "globaldata.hpp"
#include "utils_generic.hpp"

Wave::Wave()
{
  Init();
}

Wave::Wave( const ExtremaData& extremaData )
  :
  m_extrema(extremaData)
{
  Init();
}

// Common initialization routine for all ctors
void Wave::Init()
{
  m_traceDir = NO_TRACE;
  moreData = false;
  m_doublePtr = NULL;
  m_floatPtr = NULL;
  m_waveHandle = NULL;
  m_pixelSize = default_pixelsize;
}

Wave::~Wave(){}

/*
  Make a connection between the waveHandle and this class
*/
void Wave::setWaveHandle(const waveHndl& waveHandle)
{
  THROW_IF_NULL(waveHandle);

  const int waveType = WaveType(waveHandle);
  if ( waveType & NT_FP64)
  {
    m_doublePtr  = getWaveDataPtr<double>(waveHandle);
    m_floatPtr   = NULL;
    moreData     = true;
    m_waveHandle = waveHandle;
  }
  else if (waveType & NT_FP32)
  {
    m_doublePtr  = NULL;
    m_floatPtr   = getWaveDataPtr<float>(waveHandle);
    moreData     = true;
    m_waveHandle = waveHandle;
  }
  else if (waveType & NT_I32)
  {
    m_doublePtr  = NULL;
    m_floatPtr   = NULL;
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
void Wave::setProperties(const std::string& basename, int traceDir, std::string suffix)
{
  switch (traceDir)
  {
  case NO_TRACE:
    m_wavename = basename + suffix;
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
    HISTPRINT("BUG: Wave::setProperties traceDir is out of range");
    return;
  }

  m_traceDir = traceDir;
  m_suffix = suffix;
}

/*
  Output debug info
*/
void Wave::printDebugInfo()
{
  DEBUGPRINT("%s: waveHandle=%p, float=%p, double=%p, moreData=%s, traceDir=%d, specialSuffix=%s",
             m_wavename.empty() ? "empty" : m_wavename.c_str(), m_waveHandle,
             m_floatPtr, m_doublePtr, moreData ? "true" : "false",
             m_traceDir, m_suffix.c_str());
}

/*
  Sets the complete wave to NaN
*/
void Wave::clearWave()
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
void Wave::setWaveScaling(int dimension, const double* sfAPtr, const double* sfBPtr)
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
void Wave::setWaveUnits(int dimension, const std::string& units)
{
  ASSERT_RETURN_VOID(m_waveHandle);

  int ret = MDSetWaveUnits(m_waveHandle, dimension, units.c_str());

  if (ret != 0)
  {
    HISTPRINT("WaveClass::setWaveUnits returned error %d", ret);
  }
}

void Wave::setWaveUnits(int dimension, const std::wstring& units)
{
  setWaveUnits(dimension, unicodeToAnsi(units));
}

const ExtremaData& Wave::getExtrema() const
{
  return m_extrema;
}

bool Wave::isEmpty() const
{
  return m_wavename.empty();
}

int Wave::getTraceDir() const
{
  return m_traceDir;
}

const char* Wave::getWaveName() const
{
  return m_wavename.c_str();
}

waveHndl Wave::getWaveHandle() const
{
  return m_waveHandle;
}

int Wave::GetPixelSize() const
{
  return m_pixelSize;
}

void Wave::SetPixelSize(int pixelSize)
{
  m_pixelSize = pixelSize;
}

std::string Wave::getSuffix() const
{
  return m_suffix;
}
