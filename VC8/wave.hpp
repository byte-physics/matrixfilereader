/*
  The file waveclass.h is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/
#pragma once

#include <string>
#include "ForwardDecl.hpp"
#include "extremadata.hpp"

/*
  Internal representation of a igor wave
*/
class Wave
{
public:
  Wave();
  Wave(const ExtremaData &extremaData);
  ~Wave();

  void clearWave();
  void setWaveHandle(waveHndl waveHandle);
  void setProperties(const std::string &basename, int traceDir, std::string suffix = std::string());
  void printDebugInfo();
  void setWaveScaling(int dimension, const double *sfAPtr, const double *sfBPtr);
  void setWaveUnits(int dimension, const std::wstring &units);
  void setWaveUnits(int dimension, const std::string &units);

  const char *getWaveName() const;
  waveHndl getWaveHandle() const;
  int getTraceDir() const;
  bool isEmpty() const;
  const ExtremaData &getExtrema() const;
  int GetPixelSize() const;
  void SetPixelSize(int pixelSize);
  std::string getSuffix() const;

  /*
  Writes the data into the wave
  It will _not_ be checked if index is out-of-range
  Here we also determine the extrema values
  */
  void fillWave(CountInt index, int rawValue, double scaledValue)
  {
    if(m_floatPtr)
    {
      m_floatPtr[index] = static_cast<float>(scaledValue);
    }
    else if(m_doublePtr)
    {
      m_doublePtr[index] = scaledValue;
    }

    // check if it is a new minimum
    if(rawValue < m_extrema.getRawMin())
    {
      m_extrema.setMinimum(rawValue, scaledValue);
    }

    // check if it is a new maximum
    if(rawValue > m_extrema.getRawMax())
    {
      m_extrema.setMaximum(rawValue, scaledValue);
    }
  }

public:
  bool moreData;

private:
  void Init();
  ExtremaData m_extrema;
  std::string m_wavename;
  int m_traceDir;
  waveHndl m_waveHandle;
  float *m_floatPtr;
  double *m_doublePtr;
  int m_pixelSize;
  std::string m_suffix;
};
