/*
  The file brickletconverter.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "brickletconverter.hpp"
#include "utils_bricklet.hpp"
#include "globaldata.hpp"
#include "wave.hpp"
#include "bricklet.hpp"
#include "extremadata.hpp"
#include "utils_generic.hpp"

namespace
{
typedef std::vector<Wave> WaveVec;
typedef std::vector<Wave>::iterator WaveIt;
typedef std::vector<std::string> StringVector;
typedef std::vector<std::string>::const_iterator StringVectorCIt;

// Vernissage typedef
typedef std::vector<Vernissage::Session::ViewTypeCode> ViewTypeCodeVector;
typedef ViewTypeCodeVector::const_iterator ViewTypeCodeVectorCIt;
typedef Vernissage::Session::TableSet TableSet;
typedef Vernissage::Session::AxisTableSets AxisTableSets;
typedef TableSet::const_iterator TableSetCIt;
typedef Vernissage::Session::AxisDescriptor AxisDescriptor;

// Create data for the raw->scaled transformation
typedef std::pair<double, double> TransData;
TransData CalculateTransformationParameter(const Bricklet &bricklet)
{
  double slope, yIntercept;

  // the min and max values here are for the complete bricklet data and not only for one wave
  const int x1    = bricklet.getExtrema().getRawMin();
  const int x2    = bricklet.getExtrema().getRawMax();
  const double y1 = bricklet.getExtrema().getPhysValRawMin();
  const double y2 = bricklet.getExtrema().getPhysValRawMax();

  if(GlobalData::Instance().magicSetting() & identity_transformation)
  {
    slope      = 1.0;
    yIntercept = 0.0;
  }
  else
  {
    // usually xOne is not equal to xTwo
    if(x1 != x2)
    {
      slope      = (y1 - y2) / (x1 * 1.0 - x2 * 1.0);
      yIntercept = y1 - slope * x1;
    }
    else
    {
      // but if it is we have to do something different
      // xOne == xTwo means that the minimum is equal to the maximum, so the data is everywhere yOne == yTwo aka
      // constant
      slope      = 0.0;
      yIntercept = y1;
    }
  }
  DEBUGPRINT("raw->scaled transformation: xOne=%d,xTwo=%d,yOne=%.15g,yTwo=%.15g", x1, x2, y1, y2);
  DEBUGPRINT("raw->scaled transformation: slope=%.15g,yIntercept=%.15g", slope, yIntercept);

  return std::make_pair(slope, yIntercept);
}

int createEmptyWaves(WaveVec &waves, DataFolderHandle waveFolderHandle, CountInt *dimensionSizes)
{
  for(WaveIt it = waves.begin(); it != waves.end(); it++)
  {
    // skip empty entries
    if(it->isEmpty())
    {
      continue;
    }

    waveHndl waveHandle;
    int ret = MDMakeWave(&waveHandle, it->getWaveName(), waveFolderHandle, dimensionSizes, getIgorWaveType(),
                         isOverwriteEnabled());

    if(ret == NAME_WAV_CONFLICT)
    {
      DEBUGPRINT("Wave %s already exists.", it->getWaveName());
      return WAVE_EXIST;
    }
    else if(ret != 0)
    {
      HISTPRINT("Error %d in creating wave %s.", ret, it->getWaveName());
      return UNKNOWN_ERROR;
    }

    it->setWaveHandle(waveHandle);
    it->clearWave();
  }

  for(WaveIt it = waves.begin(); GlobalData::Instance().isDebuggingEnabled() && it != waves.end(); it++)
  {
    if(it->isEmpty())
    {
      continue;
    }

    it->printDebugInfo();
  }

  return 0;
}

int HandleResamplingIfRequested(DataFolderHandle waveFolderHandle, Wave &wave, int dimension, bool resampleData,
                                int pixelSize)
{
  if(!resampleData)
  {
    return 0;
  }

  wave.SetPixelSize(pixelSize);

  DEBUGPRINT("Resampling wave %s with pixelSize=%d", wave.getWaveName(), pixelSize);

  char dataFolderPath[MAXCMDLEN + 1];
  // flag=3 results in the full path being returned including a trailing colon
  int ret = GetDataFolderNameOrPath(waveFolderHandle, 3, dataFolderPath);

  if(ret != 0)
  {
    return ret;
  }

  char cmd[ARRAY_SIZE];
  if(dimension == 1)
  {
    // Command: "Resample/DOWN= [...] wave"
    sprintf(cmd, "Resample/DOWN={%d} %s", pixelSize, dataFolderPath);
    CatPossiblyQuotedName(cmd, wave.getWaveName());

    ret = XOPSilentCommand(cmd);

    if(ret != 0)
    {
      HISTPRINT("The command _%s_ failed to execute. So the XOP has to be fixed...", cmd);
      return ret;
    }
  }
  else if(dimension == 2)
  {
    // Command: "ImageInterpolate [...] Pixelate"
    sprintf(cmd, "ImageInterpolate/PXSZ={%d,%d}/DEST=%sM_PixelatedImage Pixelate %s", pixelSize, pixelSize,
            dataFolderPath, dataFolderPath);
    CatPossiblyQuotedName(cmd, wave.getWaveName());

    ret = XOPSilentCommand(cmd);

    if(ret != 0)
    {
      HISTPRINT("The command _%s_ failed to execute. So the XOP has to be fixed...", cmd);
      return ret;
    }

    // kill the un-interpolated wave
    ret = KillWave(wave.getWaveHandle());

    if(ret != 0)
    {
      return ret;
    }

    // rename wave from M_PixelatedImage to waveNameVector[i] both sitting in dataFolderPath
    ret = RenameDataFolderObject(waveFolderHandle, WAVE_OBJECT, "M_PixelatedImage", wave.getWaveName());

    if(ret != 0)
    {
      return ret;
    }
  }

  wave.setWaveHandle(FetchWaveFromDataFolder(waveFolderHandle, wave.getWaveName()));

  return 0;
}

int createWaves1D(DataFolderHandle baseFolderHandle, DataFolderHandle waveFolderHandle, const char *waveBaseName,
                  int brickletID, bool resampleData, int pixelSize, std::string &waveNameList)
{
  CountInt dimensionSizes[MAX_DIMENSIONS + 1];
  MemClear(dimensionSizes, sizeof(dimensionSizes));

  Bricklet &bricklet           = GlobalData::Instance().getBricklet(brickletID);
  void *vernissageBricklet     = bricklet.getBrickletPointer();
  Vernissage::Session *session = getVernissageSession();

  const int *rawBrickletDataPtr = bricklet.getRawData();
  if(rawBrickletDataPtr == NULL)
  {
    HISTPRINT("Could not load bricklet contents.");
    return UNKNOWN_ERROR;
  }
  const int rawBrickletSize = bricklet.getRawDataSize();

  const int brickletType             = session->getType(vernissageBricklet);
  const std::wstring triggerAxisName = session->getTriggerAxisName(vernissageBricklet);
  const AxisDescriptor triggerAxis   = session->getAxisDescriptor(vernissageBricklet, triggerAxisName);
  int numPointsTriggerAxis           = triggerAxis.clocks;
  // axis length and data length differ in case this is an aborted I(t) scan or similar
  int dataSize = rawBrickletSize;

  DEBUGPRINT("rawBrickletSize=%d, numPointsTriggerAxis=%d, triggerAxis.mirrored=%s", rawBrickletSize,
             numPointsTriggerAxis, boolToCString(triggerAxis.mirrored));

  std::vector<Wave> waves(MAX_NUM_WAVES);
  Wave &wave1 = waves[0];
  Wave &wave2 = waves[1];

  if(triggerAxis.mirrored)
  {
    if(numPointsTriggerAxis != rawBrickletSize)
    {
      HISTPRINT("BUG: Axis length differs from data length. Keep fingers crossed!");
    }
    if(brickletType != Vernissage::Session::btc_SPMSpectroscopy)
    {
      HISTPRINT("BUG: Unexpected mirrored trigger axis with non btc_SPMSpectroscopy dataset.");
    }

    wave1.setProperties(waveBaseName, NO_TRACE, suffixRampFwd);
    wave2.setProperties(waveBaseName, NO_TRACE, suffixRampBwd);
    numPointsTriggerAxis /= 2;
    dataSize = numPointsTriggerAxis;
  }
  else
  {
    // at the moment the special suffix makes only sense with 1D SPS data
    if(brickletType == Vernissage::Session::btc_SPMSpectroscopy)
    {
      wave1.setProperties(waveBaseName, NO_TRACE, suffixRampFwd);
    }
    else
    {
      wave1.setProperties(waveBaseName, NO_TRACE);
    }
  }

  dimensionSizes[ROWS] = numPointsTriggerAxis;

  int ret = createEmptyWaves(waves, waveFolderHandle, dimensionSizes);
  if(ret != 0)
  {
    return ret;
  }

  const TransData transData = CalculateTransformationParameter(bricklet);
  const double slope        = transData.first;
  const double yIntercept   = transData.second;

  for(int i = 0; i < dataSize; i++)
  {
    const int rawValue = rawBrickletDataPtr[i];
    wave1.fillWave(i, rawValue, rawValue * slope + yIntercept);
  }

  if(triggerAxis.mirrored)
  {
    const int firstBlockOffset = numPointsTriggerAxis;

    for(int i = 0; i < numPointsTriggerAxis; i++)
    {
      const int rawValue = rawBrickletDataPtr[firstBlockOffset + i];
      wave2.fillWave(firstBlockOffset - i - 1, rawValue, rawValue * slope + yIntercept);
    }
  }

  for(WaveIt it = waves.begin(); it != waves.end(); it++)
  {
    // skip empty entries
    if(it->isEmpty())
    {
      continue;
    }

    ret = HandleResamplingIfRequested(waveFolderHandle, *it, 1, resampleData, pixelSize);

    if(ret != 0)
    {
      return ret;
    }

    double xAxisDelta;

    // also the wave scaling changes if we have resampled the data
    if(resampleData)
    {
      CountInt interpolatedDimSizes[MAX_DIMENSIONS + 1];
      MemClear(interpolatedDimSizes, sizeof(interpolatedDimSizes));

      int numDimensions;
      ret = MDGetWaveDimensions(it->getWaveHandle(), &numDimensions, interpolatedDimSizes);

      if(ret != 0)
      {
        return ret;
      }

      xAxisDelta = triggerAxis.physicalIncrement * double(dimensionSizes[ROWS]) / double(interpolatedDimSizes[ROWS]);
    }
    else // original spectrum
    {
      xAxisDelta = triggerAxis.physicalIncrement;
    }

    setDataWaveNote(brickletID, *it);
    it->setWaveScaling(ROWS, &xAxisDelta, &triggerAxis.physicalStart);
    it->setWaveUnits(DATA, bricklet.getMetaDataValue<std::string>(CHANNEL_UNIT_KEY));
    it->setWaveUnits(ROWS, triggerAxis.physicalUnit);

    appendToWaveList(baseFolderHandle, it->getWaveHandle(), waveNameList);
  }

  return SUCCESS;
}

int createWaves2D(DataFolderHandle baseFolderHandle, DataFolderHandle waveFolderHandle, const char *waveBaseName,
                  int brickletID, bool resampleData, int pixelSize, std::string &waveNameList)
{
  CountInt dimensionSizes[MAX_DIMENSIONS + 1];
  MemClear(dimensionSizes, sizeof(dimensionSizes));

  Bricklet &bricklet           = GlobalData::Instance().getBricklet(brickletID);
  void *vernissageBricklet     = bricklet.getBrickletPointer();
  Vernissage::Session *session = getVernissageSession();

  const int *rawBrickletDataPtr = bricklet.getRawData();
  if(rawBrickletDataPtr == NULL)
  {
    HISTPRINT("Could not load bricklet contents.");
    return UNKNOWN_ERROR;
  }
  const int rawBrickletSize = bricklet.getRawDataSize();

  const AxisDescriptor triggerAxis =
      session->getAxisDescriptor(vernissageBricklet, session->getTriggerAxisName(vernissageBricklet));

  // Determine the length of one "line" of data
  int numPointsTriggerAxis = triggerAxis.clocks;

  if(triggerAxis.mirrored)
  {
    // The axis has the "mirrored" characteristic, thus it has a
    // "forward" and a "backward" section. Thus, the length of one line
    // is only half the number of clocks that triggered the channel.
    numPointsTriggerAxis /= 2;
  }

  // There must be another axis, because the Bricklet has two dimensions:
  const AxisDescriptor rootAxis = session->getAxisDescriptor(vernissageBricklet, triggerAxis.triggerAxisName);

  // Determine the length of one "line" of data
  int numPointsRootAxis = rootAxis.clocks;

  if(rootAxis.mirrored)
  {
    // The axis has the "mirrored" characteristic, thus it has a
    // "forward" and a "backward" section. Thus, the length of one line
    // is only half the number of clocks that triggered the channel.
    numPointsRootAxis /= 2;
  }

  DEBUGPRINT("numPointsRootAxis=%d", numPointsRootAxis);
  DEBUGPRINT("numPointsTriggerAxis=%d", numPointsTriggerAxis);

  dimensionSizes[ROWS]    = numPointsTriggerAxis;
  dimensionSizes[COLUMNS] = numPointsRootAxis;
  const CountInt waveSize = dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

  WaveVec waves(MAX_NUM_WAVES);
  Wave &traceUpData     = waves[TRACE_UP];
  Wave &reTraceUpData   = waves[RE_TRACE_UP];
  Wave &traceDownData   = waves[TRACE_DOWN];
  Wave &reTraceDownData = waves[RE_TRACE_DOWN];

  int triggerAxisBlockSize;

  // now we have to distinguish three cases on how many 2D waves we need
  // both axis are mirrored:  4
  // one mirrored, one not:  2
  // none mirrored:      1
  if(triggerAxis.mirrored && rootAxis.mirrored)
  {
    triggerAxisBlockSize = 2 * numPointsTriggerAxis;
    waves[TRACE_UP].setProperties(waveBaseName, TRACE_UP);
    waves[RE_TRACE_UP].setProperties(waveBaseName, RE_TRACE_UP);
    waves[TRACE_DOWN].setProperties(waveBaseName, TRACE_DOWN);
    waves[RE_TRACE_DOWN].setProperties(waveBaseName, RE_TRACE_DOWN);
  }
  else if(triggerAxis.mirrored)
  {
    triggerAxisBlockSize = 2 * numPointsTriggerAxis;
    waves[TRACE_UP].setProperties(waveBaseName, TRACE_UP);
    waves[RE_TRACE_UP].setProperties(waveBaseName, RE_TRACE_UP);
  }
  else if(rootAxis.mirrored)
  {
    triggerAxisBlockSize = numPointsTriggerAxis;
    waves[TRACE_UP].setProperties(waveBaseName, TRACE_UP);
    waves[TRACE_DOWN].setProperties(waveBaseName, TRACE_DOWN);
  }
  else
  {
    triggerAxisBlockSize = numPointsTriggerAxis;
    waves[TRACE_UP].setProperties(waveBaseName, TRACE_UP);
  }

  int ret = createEmptyWaves(waves, waveFolderHandle, dimensionSizes);
  if(ret != 0)
  {
    return ret;
  }

  const int firstBlockOffset = numPointsRootAxis * triggerAxisBlockSize;

  const TransData transData = CalculateTransformationParameter(bricklet);
  const double slope        = transData.first;
  const double yIntercept   = transData.second;

  // See also Vernissage manual page 22f
  // triggerAxisBlockSize: number of points in the raw data array which were acquired at the same root axis position
  // firstBlockOffset: offset position where the traceDown data starts

  // *RawBrickletIndex: source index into the raw data vernissage
  // *DataIndex: destination index into the igor wave (the funny index tricks are needed because of the organization of
  // the wave in the memory)

  // data layout of igor waves in memory (Igor XOP Manual p. 238)
  // - the wave is linear in the memory
  // - going along the arrray will first fill the first column from row 0 to end and then the second column and so on

  // TraceUp aka Forward/Up
  // ReTraceUp aka Backward/Up
  // TraceDown aka Forward/Down
  // ReTraceDown aka Backward/Down
  // horizontal axis aka X axis in Pascal's Scala Routines aka triggerAxis aka ROWS
  // vertical   axis aka Y axis in Pascal's Scala Routines aka rootAxis aka COLUMNS

  // Definitions:
  // raw data index (the array from wich the data is read): r(i,j)
  // wave data index (the array to which the data is written after conversion to physical values): w(i,j)
  // conversion routine: c(r,s,y) = rawData[r(i,j)] * s + y
  // writing data algorithm: waveData[w(i,j)] = c(r,s,y)
  // COLUMNS index: 0 <= i < numPointsRootAxis
  // ROW index: 0 <= j < numPointsTriggerAxis
  // t_{bs}: triggerAxisBlockSize
  // t_{np}: numPointsTriggerAxis
  // r_{np}: numPointsRootAxis
  // f_{bo}: offset position into the rawdata array denoting where the traceDown data starts
  // TRACE_UP
  // r(i,j) = i * t_{bs} + j
  // w(i,j) = i * t_{np} + j
  // TRACE_DOWN
  // r(i,j) = f_{bo} + i * t_{bs} + j
  // w(i,j) = ( r_{np} - ( i + 1 ) ) * t_{np} + j
  // RETRACE_UP
  // r(i,j) = ( i + 1 ) * t_{bs} - ( j + 1 )
  // w(i,j) = i * t_{np} + j
  // RETRACE_DOWN
  // r(i,j) = f_{bo} + ( i + 1 ) * t_{bs} - ( j + 1 )
  // w(i,j) = ( r_{np} - ( i + 1 ) ) * t_{np} + j

  double scaledValue;
  int rawValue;
  int dataIndex, rawIndex;

  // COLUMNS
  for(int i = 0; i < numPointsRootAxis; i++)
  {
    // ROWS
    for(int j = 0; j < numPointsTriggerAxis; j++)
    {
      // traceUp
      if(traceUpData.moreData)
      {
        rawIndex  = i * triggerAxisBlockSize + j;
        dataIndex = i * numPointsTriggerAxis + j;

        if(dataIndex >= 0 && dataIndex < waveSize && rawIndex < rawBrickletSize && rawIndex >= 0)
        {
          rawValue    = rawBrickletDataPtr[rawIndex];
          scaledValue = rawValue * slope + yIntercept;

          traceUpData.fillWave(dataIndex, rawValue, scaledValue);
        }
        else
        {
          DEBUGPRINT("Index out of range in traceUp");
          DEBUGPRINT("traceUpDataIndex=%d,waveSize=%d", dataIndex, waveSize);
          DEBUGPRINT("traceUpRawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
          traceUpData.moreData = false;
        }
      }

      // traceDown
      if(traceDownData.moreData)
      {
        rawIndex = firstBlockOffset + i * triggerAxisBlockSize + j;
        // compared to the traceUpData->dbl the index i is shifted
        // this takes into account that the data in the traceDown is aquired from the highest y value to the lowest y
        // value
        dataIndex = (numPointsRootAxis - (i + 1)) * numPointsTriggerAxis + j;

        if(dataIndex >= 0 && dataIndex < waveSize && rawIndex < rawBrickletSize && rawIndex >= 0)
        {
          rawValue    = rawBrickletDataPtr[rawIndex];
          scaledValue = rawValue * slope + yIntercept;

          traceDownData.fillWave(dataIndex, rawValue, scaledValue);
        }
        else
        {
          DEBUGPRINT("Index out of range in traceDown");
          DEBUGPRINT("traceDownDataIndex=%d,waveSize=%d", dataIndex, waveSize);
          DEBUGPRINT("traceDownRawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
          traceDownData.moreData = false;
        }
      }

      // reTraceUp
      if(reTraceUpData.moreData)
      {

        // here we shift the j index, because the data is now acquired from high column number to low column number
        rawIndex  = i * triggerAxisBlockSize + triggerAxisBlockSize - (j + 1);
        dataIndex = i * numPointsTriggerAxis + j;

        if(dataIndex >= 0 && dataIndex < waveSize && rawIndex < rawBrickletSize && rawIndex >= 0)
        {
          rawValue    = rawBrickletDataPtr[rawIndex];
          scaledValue = rawValue * slope + yIntercept;

          reTraceUpData.fillWave(dataIndex, rawValue, scaledValue);
        }
        else
        {
          DEBUGPRINT("Index out of range in reTraceUp");
          DEBUGPRINT("reTraceUpDataIndex=%d,waveSize=%d", dataIndex, waveSize);
          DEBUGPRINT("reTraceUpRawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
          reTraceUpData.moreData = false;
        }
      }

      // reTraceDown
      if(reTraceDownData.moreData)
      {

        rawIndex  = firstBlockOffset + i * triggerAxisBlockSize + triggerAxisBlockSize - (j + 1);
        dataIndex = (numPointsRootAxis - (i + 1)) * numPointsTriggerAxis + j;

        if(dataIndex >= 0 && dataIndex < waveSize && rawIndex < rawBrickletSize && rawIndex >= 0)
        {

          rawValue    = rawBrickletDataPtr[rawIndex];
          scaledValue = rawValue * slope + yIntercept;
          reTraceDownData.fillWave(dataIndex, rawValue, scaledValue);
        }
        else
        {
          DEBUGPRINT("Index out of range in reTraceDown");
          DEBUGPRINT("reTraceDownDataIndex=%d,waveSize=%d", dataIndex, waveSize);
          DEBUGPRINT("reTraceDownRawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
          reTraceDownData.moreData = false;
        }
      }
    }
  }

  for(WaveIt it = waves.begin(); it != waves.end(); it++)
  {
    if(it->isEmpty())
    {
      continue;
    }

    ret = HandleResamplingIfRequested(waveFolderHandle, *it, 2, resampleData, pixelSize);

    if(ret != 0)
    {
      continue;
    }

    double xAxisDelta, yAxisDelta;

    // also the wave scaling changes if we have resampled the data
    if(resampleData)
    {
      CountInt interpolatedDimSizes[MAX_DIMENSIONS + 1];
      MemClear(interpolatedDimSizes, sizeof(interpolatedDimSizes));

      int numDimensions;
      ret = MDGetWaveDimensions(it->getWaveHandle(), &numDimensions, interpolatedDimSizes);

      if(ret != 0)
      {
        return ret;
      }

      xAxisDelta = triggerAxis.physicalIncrement * double(dimensionSizes[ROWS]) / double(interpolatedDimSizes[ROWS]);
      yAxisDelta = rootAxis.physicalIncrement * double(dimensionSizes[COLUMNS]) / double(interpolatedDimSizes[COLUMNS]);
    }
    else // original image
    {
      xAxisDelta = triggerAxis.physicalIncrement;
      yAxisDelta = rootAxis.physicalIncrement;
    }

    // set wave note and add info about resampling to the wave note
    setDataWaveNote(brickletID, *it);

    const double zeroSetScaleOffset = 0.0;
    it->setWaveScaling(ROWS, &xAxisDelta, &zeroSetScaleOffset);
    it->setWaveScaling(COLUMNS, &yAxisDelta, &zeroSetScaleOffset);

    it->setWaveUnits(ROWS, triggerAxis.physicalUnit);
    it->setWaveUnits(COLUMNS, rootAxis.physicalUnit);
    it->setWaveUnits(DATA, bricklet.getMetaDataValue<std::string>(CHANNEL_UNIT_KEY));

    appendToWaveList(baseFolderHandle, it->getWaveHandle(), waveNameList);
  }

  return SUCCESS;
}

int createWaves3D(DataFolderHandle baseFolderHandle, DataFolderHandle waveFolderHandle, const char *waveBaseName,
                  int brickletID, std::string &waveNameList)
{
  CountInt dimensionSizes[MAX_DIMENSIONS + 1];
  MemClear(dimensionSizes, sizeof(dimensionSizes));

  Bricklet &bricklet             = GlobalData::Instance().getBricklet(brickletID);
  void *const vernissageBricklet = bricklet.getBrickletPointer();
  Vernissage::Session *session   = getVernissageSession();

  const int *rawBrickletDataPtr = bricklet.getRawData();
  if(rawBrickletDataPtr == NULL)
  {
    HISTPRINT("Could not load bricklet contents.");
    return UNKNOWN_ERROR;
  }
  const int rawBrickletSize = bricklet.getRawDataSize();

  // V triggerAxis -> V is triggered by X , X is triggered by Y and Y is the root axis

  const AxisDescriptor specAxis =
      session->getAxisDescriptor(vernissageBricklet, session->getTriggerAxisName(vernissageBricklet));
  const AxisDescriptor xAxis = session->getAxisDescriptor(vernissageBricklet, specAxis.triggerAxisName);
  const AxisDescriptor yAxis = session->getAxisDescriptor(vernissageBricklet, xAxis.triggerAxisName);
  const AxisTableSets sets =
      session->getAxisTableSets(vernissageBricklet, session->getTriggerAxisName(vernissageBricklet));

  const TableSet xSet = sets.find(specAxis.triggerAxisName)->second;
  const TableSet ySet = sets.find(xAxis.triggerAxisName)->second;

  // V Axis
  int numPointsVAxis = specAxis.clocks;

  if(specAxis.mirrored)
  {
    numPointsVAxis /= 2;
  }

  // X Axis
  int numPointsXAxis = xAxis.clocks;

  if(xAxis.mirrored)
  {
    numPointsXAxis /= 2;
  }

  // Y Axis
  int numPointsYAxis = yAxis.clocks;

  if(yAxis.mirrored)
  {
    numPointsYAxis /= 2;
  }

  // Determine how much space the data occupies in X and Y direction
  // For that we have to take into account the table sets
  // Then we also know how many 3D cubes we have, we can have 1,2 or 4. the same as in the 2D case

  TableSetCIt yIt, xIt;
  int tablePosX, tablePosY;

  // BEGIN Borrowed from SCALA exporter plugin

  // Determine the number of different x values

  // number of x axis points with taking the axis table sets into account
  int numPointsXAxisWithTableBoth = 0;
  // the part of numPointsXAxisWithTableBoth which is used in traceUp
  int numPointsXAxisWithTableFWD = 0;
  // the part of numPointsXAxisWithTableBoth which is used in reTraceUp
  int numPointsXAxisWithTableBWD = 0;

  bool forward;

  xIt       = xSet.begin();
  tablePosX = xIt->start;

  while(xIt != xSet.end())
  {
    numPointsXAxisWithTableBoth++;
    forward = (tablePosX <= numPointsXAxis);

    if(forward)
    {
      numPointsXAxisWithTableFWD++;
    }
    else
    {
      numPointsXAxisWithTableBWD++;
    }

    tablePosX += xIt->step;

    if(tablePosX > xIt->stop)
    {
      ++xIt;

      if(xIt != xSet.end())
      {
        tablePosX = xIt->start;
      }
    }
  }

  // Determine the number of different y values

  // number of y axis points with taking the axis table sets into account
  int numPointsYAxisWithTableBoth = 0;
  // the part of numPointsYAxisWithTableBoth which is used in traceUp
  int numPointsYAxisWithTableUp = 0;
  // the part of numPointsYAxisWithTableBoth which is used in traceDown
  int numPointsYAxisWithTableDown = 0;

  bool up;

  yIt       = ySet.begin();
  tablePosY = yIt->start;

  while(yIt != ySet.end())
  {
    numPointsYAxisWithTableBoth++;

    up = (tablePosY <= numPointsYAxis);

    if(up)
    {
      numPointsYAxisWithTableUp++;
    }
    else
    {
      numPointsYAxisWithTableDown++;
    }

    tablePosY += yIt->step;

    if(tablePosY > yIt->stop)
    {
      ++yIt;

      if(yIt != ySet.end())
      {
        tablePosY = yIt->start;
      }
    }
  }

  // END Borrowed from SCALA exporter plugin

  DEBUGPRINT("Axis mirroring: X=%s, Y=%s, Spec=%s", boolToCString(xAxis.mirrored), boolToCString(yAxis.mirrored),
             boolToCString(specAxis.mirrored));

  DEBUGPRINT("Spec Axis: points %d, clocks %d", numPointsVAxis, specAxis.clocks);

  DEBUGPRINT("X Axis # points with tableSet: Total=%d, Forward=%d, Backward=%d", numPointsXAxisWithTableBoth,
             numPointsXAxisWithTableFWD, numPointsXAxisWithTableBWD);

  DEBUGPRINT("Y Axis # points with tableSet: Total=%d, Up=%d, Down=%d", numPointsYAxisWithTableBoth,
             numPointsYAxisWithTableUp, numPointsYAxisWithTableDown);

  // Theoretical the sizes of the cubes could be different but we are igoring that for now
  if(numPointsXAxisWithTableBWD != 0 && numPointsXAxisWithTableFWD != 0 &&
     numPointsXAxisWithTableFWD != numPointsXAxisWithTableBWD)
  {
    HISTPRINT("BUG: Number of X axis points is different in forward and backward direction.");
    return INTERNAL_ERROR_CONVERTING_DATA;
  }
  else if(numPointsYAxisWithTableUp != 0 && numPointsYAxisWithTableDown != 0 &&
          numPointsYAxisWithTableUp != numPointsYAxisWithTableDown)
  {
    HISTPRINT("BUG: Number of Y axis points is different in up and down direction.");
    return INTERNAL_ERROR_CONVERTING_DATA;
  }

  double xAxisDelta, yAxisDelta;
  double xAxisOffset, yAxisOffset;

  // normally we have table sets with some step width >1, so the physicalIncrement has to be multiplied by this factor
  // Note: this approach assumes that the axis table sets of one axis all have the same physical step width,
  // this is at least the case for Matrix 2.2-1 and Matrix 3.0
  if(ySet.size() > 0)
  {
    yAxisDelta  = yAxis.physicalIncrement * ySet.begin()->step;
    yAxisOffset = yAxis.physicalIncrement * (ySet.begin()->start - 1);
  }
  else
  {
    yAxisDelta  = yAxis.physicalIncrement;
    yAxisOffset = 0.0;
  }

  if(xSet.size() > 0)
  {
    xAxisDelta = xAxis.physicalIncrement * xSet.begin()->step;

    if(numPointsXAxisWithTableFWD != 0)
    {
      // we also scanned in TraceUP direction, therefore we can use the same algorithm like for y-Axis
      xAxisOffset = xAxis.physicalIncrement * (xSet.begin()->start - 1);
    }
    else
    {
      // we didn't scan in TraceUp direction, therefore we assume that this table sets stop value is somewhere
      // near the start of the scan
      // Typically xSet.begin()->stop is equal to xAxis.clocks and therefore xAxisOffset being zero
      xAxisOffset = xAxis.physicalIncrement * (xSet.begin()->stop - xAxis.clocks);
    }
  }
  else
  {
    xAxisDelta  = xAxis.physicalIncrement;
    xAxisOffset = 0.0;
  }

  if(GlobalData::Instance().isDebuggingEnabled())
  {
    DEBUGPRINT("Number of axes we have table sets for: %d", sets.size());
    DEBUGPRINT("Tablesets: xAxis");

    for(TableSetCIt it = xSet.begin(); it != xSet.end(); it++)
    {
      DEBUGPRINT("start=%d, step=%d, stop=%d", it->start, it->step, it->stop);
    }

    DEBUGPRINT("Tablesets: yAxis");

    for(TableSetCIt it = ySet.begin(); it != ySet.end(); it++)
    {
      DEBUGPRINT("start=%d, step=%d, stop=%d", it->start, it->step, it->stop);
    }

    DEBUGPRINT("xAxisDelta=%g, yAxisDelta=%g", xAxisDelta, yAxisDelta);
    DEBUGPRINT("xAxisOffset=%g, yAxisOffset=%g", xAxisOffset, yAxisOffset);
  }

  // dimensions of the cube
  if(numPointsXAxisWithTableFWD != 0)
  {
    dimensionSizes[ROWS] = numPointsXAxisWithTableFWD;
  }
  else
  {
    // we only scanned in BWD direction
    dimensionSizes[ROWS] = numPointsXAxisWithTableBWD;
  }

  // we must always scan in Up direction
  dimensionSizes[COLUMNS] = numPointsYAxisWithTableUp;
  dimensionSizes[LAYERS]  = numPointsVAxis;

  const CountInt waveSize = dimensionSizes[ROWS] * dimensionSizes[COLUMNS] * dimensionSizes[LAYERS];

  DEBUGPRINT("dimensions of the cube: rows=%d,cols=%d,layers=%d", dimensionSizes[ROWS], dimensionSizes[COLUMNS],
             dimensionSizes[LAYERS]);

  WaveVec waves(MAX_NUM_WAVES);
  Wave &traceUpDataRampFwd     = waves[TRACE_UP_RAMP_FWD];
  Wave &traceUpDataRampBwd     = waves[TRACE_UP_RAMP_BWD];
  Wave &reTraceUpDataRampFwd   = waves[RE_TRACE_UP_RAMP_FWD];
  Wave &reTraceUpDataRampBwd   = waves[RE_TRACE_UP_RAMP_BWD];
  Wave &traceDownDataRampFwd   = waves[TRACE_DOWN_RAMP_FWD];
  Wave &traceDownDataRampBwd   = waves[TRACE_DOWN_RAMP_BWD];
  Wave &reTraceDownDataRampFwd = waves[RE_TRACE_DOWN_RAMP_FWD];
  Wave &reTraceDownDataRampBwd = waves[RE_TRACE_DOWN_RAMP_BWD];

  // 4 cubes, TraceUpRampFwd, TraceDownRampFwd, ReTraceUpRampFwd, ReTraceDownRampFwd
  // x2 if spec axis mirrored
  if(numPointsXAxisWithTableFWD != 0 && numPointsXAxisWithTableBWD != 0 && numPointsYAxisWithTableUp != 0 &&
     numPointsYAxisWithTableDown != 0)
  {
    waves[TRACE_UP_RAMP_FWD].setProperties(waveBaseName, TRACE_UP, suffixRampFwd);
    waves[RE_TRACE_UP_RAMP_FWD].setProperties(waveBaseName, RE_TRACE_UP, suffixRampFwd);
    waves[TRACE_DOWN_RAMP_FWD].setProperties(waveBaseName, TRACE_DOWN, suffixRampFwd);
    waves[RE_TRACE_DOWN_RAMP_FWD].setProperties(waveBaseName, RE_TRACE_DOWN, suffixRampFwd);

    if(specAxis.mirrored)
    {
      waves[TRACE_UP_RAMP_BWD].setProperties(waveBaseName, TRACE_UP, suffixRampBwd);
      waves[RE_TRACE_UP_RAMP_BWD].setProperties(waveBaseName, RE_TRACE_UP, suffixRampBwd);
      waves[TRACE_DOWN_RAMP_BWD].setProperties(waveBaseName, TRACE_DOWN, suffixRampBwd);
      waves[RE_TRACE_DOWN_RAMP_BWD].setProperties(waveBaseName, RE_TRACE_DOWN, suffixRampBwd);
    }
  }
  // 2 cubes, TraceUpRampFwd, TraceDownRampFwd
  // x2 if spec axis mirrored
  else if(numPointsXAxisWithTableBWD == 0 && numPointsYAxisWithTableDown != 0)
  {
    waves[TRACE_UP_RAMP_FWD].setProperties(waveBaseName, TRACE_UP, suffixRampFwd);
    waves[TRACE_DOWN_RAMP_FWD].setProperties(waveBaseName, TRACE_DOWN, suffixRampFwd);

    if(specAxis.mirrored)
    {
      waves[TRACE_UP_RAMP_BWD].setProperties(waveBaseName, TRACE_UP, suffixRampBwd);
      waves[TRACE_DOWN_RAMP_BWD].setProperties(waveBaseName, TRACE_DOWN, suffixRampBwd);
    }
  }
  // 2 cubes, TraceUpRampFwd, ReTraceUpRampFwd
  // x2 if spec axis mirrored
  else if(numPointsXAxisWithTableBWD != 0 && numPointsYAxisWithTableDown == 0)
  {
    waves[TRACE_UP_RAMP_FWD].setProperties(waveBaseName, TRACE_UP, suffixRampFwd);
    waves[RE_TRACE_UP_RAMP_FWD].setProperties(waveBaseName, RE_TRACE_UP, suffixRampFwd);

    if(specAxis.mirrored)
    {
      waves[TRACE_UP_RAMP_BWD].setProperties(waveBaseName, TRACE_UP, suffixRampBwd);
      waves[RE_TRACE_UP_RAMP_BWD].setProperties(waveBaseName, RE_TRACE_UP, suffixRampBwd);
    }
  }
  // 2 cubes, ReTraceUpRampFwd, ReTraceDownRampFwd
  // x2 if spec axis mirrored
  else if(numPointsXAxisWithTableFWD == 0 && numPointsYAxisWithTableDown != 0)
  {
    waves[RE_TRACE_UP_RAMP_FWD].setProperties(waveBaseName, RE_TRACE_UP, suffixRampFwd);
    waves[RE_TRACE_DOWN_RAMP_FWD].setProperties(waveBaseName, RE_TRACE_DOWN, suffixRampFwd);

    if(specAxis.mirrored)
    {
      waves[RE_TRACE_UP_RAMP_BWD].setProperties(waveBaseName, RE_TRACE_UP, suffixRampBwd);
      waves[RE_TRACE_DOWN_RAMP_BWD].setProperties(waveBaseName, RE_TRACE_DOWN, suffixRampBwd);
    }
  }
  // 1 cube, TraceUpRampFwd
  // x2 if spec axis mirrored
  else if(numPointsXAxisWithTableBWD == 0 && numPointsYAxisWithTableDown == 0)
  {
    waves[TRACE_UP_RAMP_FWD].setProperties(waveBaseName, TRACE_UP, suffixRampFwd);

    if(specAxis.mirrored)
    {
      waves[TRACE_UP_RAMP_BWD].setProperties(waveBaseName, TRACE_UP, suffixRampBwd);
    }
  }
  // 1 cube, ReTraceUpRampFwd
  // x2 if spec axis mirrored
  else if(numPointsXAxisWithTableFWD == 0 && numPointsYAxisWithTableDown == 0)
  {
    waves[RE_TRACE_UP_RAMP_FWD].setProperties(waveBaseName, RE_TRACE_UP, suffixRampFwd);

    if(specAxis.mirrored)
    {
      waves[RE_TRACE_UP_RAMP_BWD].setProperties(waveBaseName, RE_TRACE_UP, suffixRampBwd);
    }
  }
  // not possible
  else
  {
    HISTPRINT("BUG: Error in determining the number of cubes.");
    return INTERNAL_ERROR_CONVERTING_DATA;
  }

  const int specAxisBlockSize     = specAxis.clocks;
  const int xAxisBlockSize        = (numPointsXAxisWithTableBWD + numPointsXAxisWithTableFWD) * specAxisBlockSize;
  const int xAxisForwardBlockSize = numPointsXAxisWithTableFWD * specAxisBlockSize;

  // data index to the start of the TraceDown data (this is the same for all combinations as xAxisBlockSize is set
  // apropriately) in case the traceDown scan does not exist this is also no problem
  const int firstBlockOffset = numPointsYAxisWithTableUp * xAxisBlockSize;

  DEBUGPRINT("xAxisBlockSize=%d, xAxisForwardBlockSize=%d, firstBlockOffset=%d", xAxisBlockSize, xAxisForwardBlockSize,
             firstBlockOffset);

  int ret = createEmptyWaves(waves, waveFolderHandle, dimensionSizes);
  if(ret != 0)
  {
    return ret;
  }

  const TransData transData = CalculateTransformationParameter(bricklet);
  const double slope        = transData.first;
  const double yIntercept   = transData.second;

  CountInt rawIndex, dataIndex;
  int rawValue;
  double scaledValue;

  // COLUMNS
  for(CountInt i = 0; i < dimensionSizes[COLUMNS]; i++)
  {
    // ROWS
    for(CountInt j = 0; j < dimensionSizes[ROWS]; j++)
    {
      // LAYERS
      for(CountInt k = 0; k < dimensionSizes[LAYERS]; k++)
      {
        // traceUp, RampFwd
        if(traceUpDataRampFwd.moreData)
        {
          rawIndex  = i * xAxisBlockSize + j * specAxisBlockSize + k;
          dataIndex = i * dimensionSizes[ROWS] + j + k * dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

          if(dataIndex >= 0 && dataIndex < waveSize && rawIndex < rawBrickletSize && rawIndex >= 0)
          {

            rawValue    = rawBrickletDataPtr[rawIndex];
            scaledValue = rawValue * slope + yIntercept;

            traceUpDataRampFwd.fillWave(dataIndex, rawValue, scaledValue);
          }
          else
          {
            DEBUGPRINT("Index out of range in traceUpRampFwd");
            DEBUGPRINT("dataIndex=%d,waveSize=%d", dataIndex, waveSize);
            DEBUGPRINT("rawIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
            traceUpDataRampFwd.moreData = false;
          }
        } // if traceUpDataRampFwd.moreData

        // traceUp, RampBwd
        if(traceUpDataRampBwd.moreData)
        {
          rawIndex  = i * xAxisBlockSize + j * specAxisBlockSize + specAxisBlockSize - (k + 1);
          dataIndex = i * dimensionSizes[ROWS] + j + k * dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

          if(dataIndex >= 0 && dataIndex < waveSize && rawIndex < rawBrickletSize && rawIndex >= 0)
          {

            rawValue    = rawBrickletDataPtr[rawIndex];
            scaledValue = rawValue * slope + yIntercept;

            traceUpDataRampBwd.fillWave(dataIndex, rawValue, scaledValue);
          }
          else
          {
            DEBUGPRINT("Index out of range in traceUpRampBwd");
            DEBUGPRINT("dataIndex=%d,waveSize=%d", dataIndex, waveSize);
            DEBUGPRINT("rawIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
            traceUpDataRampBwd.moreData = false;
          }
        } // if traceUpDataRampBwd.moreData

        // traceDown, RampFwd
        if(traceDownDataRampFwd.moreData)
        {
          rawIndex  = firstBlockOffset + i * xAxisBlockSize + j * specAxisBlockSize + k;
          dataIndex = (dimensionSizes[COLUMNS] - (i + 1)) * dimensionSizes[ROWS] + j +
                      k * dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

          if(dataIndex >= 0 && dataIndex < waveSize && rawIndex < rawBrickletSize && rawIndex >= 0)
          {
            rawValue    = rawBrickletDataPtr[rawIndex];
            scaledValue = rawValue * slope + yIntercept;

            traceDownDataRampFwd.fillWave(dataIndex, rawValue, scaledValue);
          }
          else
          {
            DEBUGPRINT("Index out of range in traceDownRampFwd");
            DEBUGPRINT("dataIndex=%d,waveSize=%d", dataIndex, waveSize);
            DEBUGPRINT("rawIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
            traceDownDataRampFwd.moreData = false;
          }
        } // if traceDownDataRampFwd.moreData

        // traceDown, RampBwd
        if(traceDownDataRampBwd.moreData)
        {
          rawIndex  = firstBlockOffset + i * xAxisBlockSize + j * specAxisBlockSize + specAxisBlockSize - (k + 1);
          dataIndex = (dimensionSizes[COLUMNS] - (i + 1)) * dimensionSizes[ROWS] + j +
                      k * dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

          if(dataIndex >= 0 && dataIndex < waveSize && rawIndex < rawBrickletSize && rawIndex >= 0)
          {
            rawValue    = rawBrickletDataPtr[rawIndex];
            scaledValue = rawValue * slope + yIntercept;

            traceDownDataRampBwd.fillWave(dataIndex, rawValue, scaledValue);
          }
          else
          {
            DEBUGPRINT("Index out of range in traceDownRampBwd");
            DEBUGPRINT("dataIndex=%d,waveSize=%d", dataIndex, waveSize);
            DEBUGPRINT("rawIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
            traceDownDataRampBwd.moreData = false;
          }
        } // if traceDownDataRampBwd.moreData

        // reTraceUp, RampFwd
        if(reTraceUpDataRampFwd.moreData)
        {
          rawIndex =
              xAxisForwardBlockSize + i * xAxisBlockSize + (dimensionSizes[ROWS] - (j + 1)) * specAxisBlockSize + k;
          dataIndex = i * dimensionSizes[ROWS] + j + k * dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

          if(dataIndex >= 0 && dataIndex < waveSize && rawIndex < rawBrickletSize && rawIndex >= 0)
          {
            rawValue    = rawBrickletDataPtr[rawIndex];
            scaledValue = rawValue * slope + yIntercept;

            reTraceUpDataRampFwd.fillWave(dataIndex, rawValue, scaledValue);
          }
          else
          {
            DEBUGPRINT("Index out of range in reTraceUpRampFwd");
            DEBUGPRINT("dataIndex=%d,waveSize=%d", dataIndex, waveSize);
            DEBUGPRINT("rawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
            reTraceUpDataRampFwd.moreData = false;
          }
        } // if reTraceUpDataRampFwd.moreData

        // reTraceUp, RampBwd
        if(reTraceUpDataRampBwd.moreData)
        {
          rawIndex = xAxisForwardBlockSize + i * xAxisBlockSize + (dimensionSizes[ROWS] - (j + 1)) * specAxisBlockSize +
                     specAxisBlockSize - (k + 1);
          dataIndex = i * dimensionSizes[ROWS] + j + k * dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

          if(dataIndex >= 0 && dataIndex < waveSize && rawIndex < rawBrickletSize && rawIndex >= 0)
          {
            rawValue    = rawBrickletDataPtr[rawIndex];
            scaledValue = rawValue * slope + yIntercept;

            reTraceUpDataRampBwd.fillWave(dataIndex, rawValue, scaledValue);
          }
          else
          {
            DEBUGPRINT("Index out of range in reTraceUpRampBwd");
            DEBUGPRINT("dataIndex=%d,waveSize=%d", dataIndex, waveSize);
            DEBUGPRINT("rawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
            reTraceUpDataRampBwd.moreData = false;
          }
        } // if reTraceUpDataRampBwd.moreData

        // reTraceDown, RampFwd
        if(reTraceDownDataRampFwd.moreData)
        {
          rawIndex = firstBlockOffset + xAxisForwardBlockSize + i * xAxisBlockSize +
                     (dimensionSizes[ROWS] - (j + 1)) * specAxisBlockSize + k;
          dataIndex = (dimensionSizes[COLUMNS] - (i + 1)) * dimensionSizes[ROWS] + j +
                      k * dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

          if(dataIndex >= 0 && dataIndex < waveSize && rawIndex < rawBrickletSize && rawIndex >= 0)
          {
            rawValue    = rawBrickletDataPtr[rawIndex];
            scaledValue = rawValue * slope + yIntercept;

            reTraceDownDataRampFwd.fillWave(dataIndex, rawValue, scaledValue);
          }
          else
          {
            DEBUGPRINT("Index out of range in reTraceDownRampFwd");
            DEBUGPRINT("dataIndex=%d,waveSize=%d", dataIndex, waveSize);
            DEBUGPRINT("rawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
            reTraceDownDataRampFwd.moreData = false;
          }
        } // if reTraceDownDataRampFwd.moreData

        // reTraceDown, RampBwd
        if(reTraceDownDataRampBwd.moreData)
        {
          rawIndex = firstBlockOffset + xAxisForwardBlockSize + i * xAxisBlockSize +
                     (dimensionSizes[ROWS] - (j + 1)) * specAxisBlockSize + specAxisBlockSize - (k + 1);
          dataIndex = (dimensionSizes[COLUMNS] - (i + 1)) * dimensionSizes[ROWS] + j +
                      k * dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

          if(dataIndex >= 0 && dataIndex < waveSize && rawIndex < rawBrickletSize && rawIndex >= 0)
          {
            rawValue    = rawBrickletDataPtr[rawIndex];
            scaledValue = rawValue * slope + yIntercept;

            reTraceDownDataRampBwd.fillWave(dataIndex, rawValue, scaledValue);
          }
          else
          {
            DEBUGPRINT("Index out of range in reTraceDownRampBwd");
            DEBUGPRINT("dataIndex=%d,waveSize=%d", dataIndex, waveSize);
            DEBUGPRINT("rawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
            reTraceDownDataRampBwd.moreData = false;
          }
        } // if reTraceDownDataRampBwd.moreData
      }   // for LAYERS
    }     // for ROWS
  }       // for COLUMNS

  for(WaveIt it = waves.begin(); it != waves.end(); it++)
  {
    if(it->isEmpty())
    {
      continue;
    }

    setDataWaveNote(brickletID, *it);

    it->setWaveScaling(ROWS, &xAxisDelta, &xAxisOffset);
    it->setWaveScaling(COLUMNS, &yAxisDelta, &yAxisOffset);
    it->setWaveScaling(LAYERS, &specAxis.physicalIncrement, &specAxis.physicalStart);

    it->setWaveUnits(ROWS, xAxis.physicalUnit);
    it->setWaveUnits(COLUMNS, yAxis.physicalUnit);
    it->setWaveUnits(LAYERS, specAxis.physicalUnit);
    it->setWaveUnits(DATA, bricklet.getMetaDataValue<std::string>(CHANNEL_UNIT_KEY));

    appendToWaveList(baseFolderHandle, it->getWaveHandle(), waveNameList);
  }

  return SUCCESS;
}

} // anonymous namespace

/*
  Creates the real data waves, supports 1D-3D data
*/
int createWaves(DataFolderHandle baseFolderHandle, DataFolderHandle waveFolderHandle, const char *waveBaseName,
                int brickletID, bool resampleData, int pixelSize, std::string &waveNameList)
{
  Bricklet &bricklet  = GlobalData::Instance().getBricklet(brickletID);
  const int dimension = bricklet.getMetaDataValue<int>(DIMENSION_KEY);

  if(GlobalData::Instance().isDebuggingEnabled())
  {
    void *vernissageBricklet = bricklet.getBrickletPointer();

    DEBUGPRINT("### BrickletID %d ###", brickletID);
    DEBUGPRINT("dimension %d", dimension);

    Vernissage::Session *session           = getVernissageSession();
    const ViewTypeCodeVector viewTypeCodes = session->getViewTypes(vernissageBricklet);
    for(ViewTypeCodeVectorCIt it = viewTypeCodes.begin(); it != viewTypeCodes.end(); it++)
    {
      DEBUGPRINT("viewType %s", viewTypeCodeToString(*it).c_str());
    }
    const int brickletType               = session->getType(bricklet.getBrickletPointer());
    const std::string brickletTypeString = brickletTypeToString(brickletType);
    DEBUGPRINT("brickletType %s", brickletTypeString.c_str());

    DEBUGPRINT("Axis order is from triggerAxis to rootAxis");

    const StringVector allAxes = bricklet.getAxes<std::string>();

    for(StringVectorCIt it = allAxes.begin(); it != allAxes.end(); it++)
    {
      DEBUGPRINT("Axis %s", it->c_str());
    }
  }

  switch(dimension)
  {
  case 1:
    return createWaves1D(baseFolderHandle, waveFolderHandle, waveBaseName, brickletID, resampleData, pixelSize,
                         waveNameList);
    break;

  case 2:
    return createWaves2D(baseFolderHandle, waveFolderHandle, waveBaseName, brickletID, resampleData, pixelSize,
                         waveNameList);
    break;

  case 3:
    return createWaves3D(baseFolderHandle, waveFolderHandle, waveBaseName, brickletID, waveNameList);
    break;

  default:
    HISTPRINT("Dimension %d can not be handled. Please file a bug report at %s and attach the measured data.",
              dimension, PROJECTURL);
    return INTERNAL_ERROR_CONVERTING_DATA;
    break;
  }
  return SUCCESS;
}

/*
  create the raw data wave which just holds the raw data as 1D array
*/
int createRawDataWave(DataFolderHandle baseFolderHandle, DataFolderHandle dfHandle, const char *waveName,
                      int brickletID, std::string &waveNameList)
{
  CountInt dimensionSizes[MAX_DIMENSIONS + 1];
  MemClear(dimensionSizes, sizeof(dimensionSizes));

  Bricklet &bricklet = GlobalData::Instance().getBricklet(brickletID);

  Wave wave(bricklet.getExtrema());
  wave.setProperties(waveName, NO_TRACE);

  const int *rawBrickletDataPtr = bricklet.getRawData();
  if(rawBrickletDataPtr == NULL)
  {
    HISTPRINT("Could not load bricklet contents.");
    return UNKNOWN_ERROR;
  }
  const int rawBrickletSize = bricklet.getRawDataSize();

  // create 1D wave with count points
  dimensionSizes[ROWS] = rawBrickletSize;

  waveHndl waveHandle;
  int ret = MDMakeWave(&waveHandle, wave.getWaveName(), dfHandle, dimensionSizes, NT_I32, isOverwriteEnabled());

  if(ret == NAME_WAV_CONFLICT)
  {
    DEBUGPRINT("Wave %s already exists.", wave.getWaveName());
    return WAVE_EXIST;
  }
  else if(ret != 0)
  {
    return ret;
  }

  wave.setWaveHandle(waveHandle);
  waveHandle = NULL;

  int *dataPtr = getWaveDataPtr<int>(wave.getWaveHandle());

  memcpy(dataPtr, rawBrickletDataPtr, sizeof(int) * rawBrickletSize);

  setDataWaveNote(brickletID, wave);

  appendToWaveList(baseFolderHandle, wave.getWaveHandle(), waveNameList);
  return ret;
}
