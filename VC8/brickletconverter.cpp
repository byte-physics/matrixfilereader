/*
  The file brickletconverter.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt  in the source folder for details.
*/

#include "stdafx.h"

#include "brickletconverter.hpp"
#include "utils_bricklet.hpp"
#include "globaldata.hpp"
#include "waveclass.hpp"
#include "brickletclass.hpp"
#include "extremadata.hpp"
#include "utils_generic.hpp"

namespace
{
  // Create data for the raw->scaled transformation
  void CalculateTransformationParameter(BrickletClass* const bricklet, double& slope, double& yIntercept)
  {
    // the min and max values here are for the complete bricklet data and not only for one wave
    const int xOne = bricklet->getExtrema().getRawMin();
    const int xTwo = bricklet->getExtrema().getRawMax();
    const double yOne = bricklet->getExtrema().getPhysValRawMin();
    const double yTwo = bricklet->getExtrema().getPhysValRawMax();

    // usually xOne is not equal to xTwo
    if (xOne != xTwo)
    {
      slope = (yOne - yTwo) / (xOne * 1.0 - xTwo * 1.0);
      yIntercept = yOne - slope * xOne;
    }
    else
    {
      // but if it is we have to do something different
      // xOne == xTwo means that the minimum is equal to the maximum, so the data is everywhere yOne == yTwo aka constant
      slope = 0.0;
      yIntercept = yOne;
    }

    DEBUGPRINT("raw->scaled transformation: xOne=%d,xTwo=%d,yOne=%g,yTwo=%g", xOne, xTwo, yOne, yTwo);
    DEBUGPRINT("raw->scaled transformation: slope=%g,yIntercept=%g", slope, yIntercept);
  }

  int createWaves1D(DataFolderHandle dfHandle, const char* waveBaseName, int brickletID, std::string& fullPathOfCreatedWave)
  {
    CountInt dimensionSizes[MAX_DIMENSIONS + 1];
    MemClear(dimensionSizes, sizeof(dimensionSizes));

    BrickletClass* const bricklet = GlobalData::Instance().getBrickletClassObject(brickletID);
    ASSERT_RETURN_ONE(bricklet);

    void* pBricklet = bricklet->getBrickletPointer();
    ASSERT_RETURN_ONE(pBricklet);

    Vernissage::Session* pSession = GlobalData::Instance().getVernissageSession();
    ASSERT_RETURN_ONE(pSession);

    // pointer to raw data
    int rawBrickletSize = 0;
    const int* rawBrickletDataPtr;
    bricklet->getBrickletContentsBuffer(&rawBrickletDataPtr , rawBrickletSize);

    if (rawBrickletSize == 0 || &rawBrickletDataPtr == NULL)
    {
      HISTPRINT("Could not load bricklet contents.");
      return UNKNOWN_ERROR;
    }

    const std::wstring triggerAxisName = pSession->getTriggerAxisName(pBricklet);
    const Vernissage::Session::AxisDescriptor triggerAxis = pSession->getAxisDescriptor(pBricklet, triggerAxisName);
    int numPointsTriggerAxis = triggerAxis.clocks;

    if (triggerAxis.mirrored)
    {
      numPointsTriggerAxis /= 2;
    }

    dimensionSizes[ROWS] = numPointsTriggerAxis;

    WaveClass wave1D;
    wave1D.setNameAndTraceDir(waveBaseName, NO_TRACE);

    waveHndl waveHandle;
    int ret = MDMakeWave(&waveHandle, wave1D.getWaveName(), dfHandle, dimensionSizes,
                         GlobalData::Instance().getIgorWaveType(), GlobalData::Instance().isOverwriteEnabled<int>());

    if (ret == NAME_WAV_CONFLICT)
    {
      DEBUGPRINT("Wave %s already exists.", wave1D.getWaveName());
      return WAVE_EXIST;
    }
    else if (ret != 0)
    {
      HISTPRINT("Error %d in creating wave %s.", ret, wave1D.getWaveName());
      return UNKNOWN_ERROR;
    }

    wave1D.setWaveHandle(waveHandle);
    wave1D.clearWave();
    waveHandle = NULL;

    if (GlobalData::Instance().isDebuggingEnabled())
    {
      wave1D.printDebugInfo();
    }

    double slope, yIntercept;
    CalculateTransformationParameter(bricklet, slope, yIntercept);

    for (int i = 0; i < rawBrickletSize ; i++)
    {
      const int rawValue = rawBrickletDataPtr[i];
      wave1D.fillWave(i, rawValue, rawValue * slope + yIntercept);
    }

    setDataWaveNote(brickletID, wave1D);

    wave1D.setWaveScaling(ROWS, &triggerAxis.physicalIncrement, &triggerAxis.physicalStart);
    wave1D.setWaveUnits(ROWS, triggerAxis.physicalUnit);
    wave1D.setWaveUnits(DATA, bricklet->getMetaDataValue<std::string>(CHANNEL_UNIT_KEY));

    appendToWaveList(dfHandle, wave1D.getWaveHandle(), fullPathOfCreatedWave);
    return SUCCESS;
  }

  int createWaves2D(DataFolderHandle dfHandle, const char* waveBaseName, int brickletID, bool resampleData, int pixelSize, std::string& fullPathOfCreatedWave)
  {
    CountInt dimensionSizes[MAX_DIMENSIONS + 1];
    MemClear(dimensionSizes, sizeof(dimensionSizes));

    BrickletClass* const bricklet = GlobalData::Instance().getBrickletClassObject(brickletID);
    ASSERT_RETURN_ONE(bricklet);

    void* pBricklet = bricklet->getBrickletPointer();
    ASSERT_RETURN_ONE(pBricklet);

    Vernissage::Session* pSession = GlobalData::Instance().getVernissageSession();
    ASSERT_RETURN_ONE(pSession);

    // pointer to raw data
    int rawBrickletSize = 0;
    const int* rawBrickletDataPtr;
    bricklet->getBrickletContentsBuffer(&rawBrickletDataPtr , rawBrickletSize);

    if (rawBrickletSize == 0 || &rawBrickletDataPtr == NULL)
    {
      HISTPRINT("Could not load bricklet contents.");
      return UNKNOWN_ERROR;
    }

    const Vernissage::Session::AxisDescriptor triggerAxis = pSession->getAxisDescriptor(pBricklet, pSession->getTriggerAxisName(pBricklet));

    // Determine the length of one "line" of data
    int numPointsTriggerAxis = triggerAxis.clocks;

    if (triggerAxis.mirrored)
    {
      // The axis has the "mirrored" characteristic, thus it has a
      // "forward" and a "backward" section. Thus, the length of one line
      // is only half the number of clocks that triggered the channel.
      numPointsTriggerAxis /= 2;
    }

    // There must be another axis, because the Bricklet has two dimensions:
    const Vernissage::Session::AxisDescriptor rootAxis = pSession->getAxisDescriptor(pBricklet, triggerAxis.triggerAxisName);

    // Determine the length of one "line" of data
    int numPointsRootAxis = rootAxis.clocks;

    if (rootAxis.mirrored)
    {
      // The axis has the "mirrored" characteristic, thus it has a
      // "forward" and a "backward" section. Thus, the length of one line
      // is only half the number of clocks that triggered the channel.
      numPointsRootAxis /= 2;
    }

    DEBUGPRINT("numPointsRootAxis=%d", numPointsRootAxis);

    DEBUGPRINT("numPointsTriggerAxis=%d", numPointsTriggerAxis);

    dimensionSizes[ROWS] = numPointsTriggerAxis;
    dimensionSizes[COLUMNS] = numPointsRootAxis;
    const int waveSize = dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

    WaveClass wave[MAX_NUM_TRACES];
    WaveClass* traceUpData = &wave[TRACE_UP];
    WaveClass* reTraceUpData = &wave[RE_TRACE_UP];
    WaveClass* traceDownData = &wave[TRACE_DOWN];
    WaveClass* reTraceDownData = &wave[RE_TRACE_DOWN];

    int triggerAxisBlockSize;

    // now we have to distinguish three cases on how many 2D waves we need
    // both axis are mirrored:  4
    // one mirrored, one not:  2
    // none mirrored:      1
    if (triggerAxis.mirrored && rootAxis.mirrored)
    {
      triggerAxisBlockSize = 2 * numPointsTriggerAxis;
      wave[TRACE_UP].setNameAndTraceDir(waveBaseName, TRACE_UP);
      wave[RE_TRACE_UP].setNameAndTraceDir(waveBaseName, RE_TRACE_UP);
      wave[TRACE_DOWN].setNameAndTraceDir(waveBaseName, TRACE_DOWN);
      wave[RE_TRACE_DOWN].setNameAndTraceDir(waveBaseName, RE_TRACE_DOWN);
    }
    else if (triggerAxis.mirrored)
    {
      triggerAxisBlockSize = 2 * numPointsTriggerAxis;
      wave[TRACE_UP].setNameAndTraceDir(waveBaseName, TRACE_UP);
      wave[RE_TRACE_UP].setNameAndTraceDir(waveBaseName, RE_TRACE_UP);
    }
    else if (rootAxis.mirrored)
    {
      triggerAxisBlockSize = numPointsTriggerAxis;
      wave[TRACE_UP].setNameAndTraceDir(waveBaseName, TRACE_UP);
      wave[TRACE_DOWN].setNameAndTraceDir(waveBaseName, TRACE_DOWN);
    }
    else
    {
      triggerAxisBlockSize = numPointsTriggerAxis;
      wave[TRACE_UP].setNameAndTraceDir(waveBaseName, TRACE_UP);
    }

    for (int i = 0; i < MAX_NUM_TRACES; i++)
    {
      // skip empty entries
      if (wave[i].isEmpty())
      {
        continue;
      }

      waveHndl waveHandle;
      int ret = MDMakeWave(&waveHandle, wave[i].getWaveName(), dfHandle, dimensionSizes,
                           GlobalData::Instance().getIgorWaveType(), GlobalData::Instance().isOverwriteEnabled<int>());

      if (ret == NAME_WAV_CONFLICT)
      {
        DEBUGPRINT("Wave %s already exists.", wave[i].getWaveName());
        return WAVE_EXIST;
      }
      else if (ret != 0)
      {
        HISTPRINT("Error %d in creating wave %s.", ret, wave[i].getWaveName());
        return UNKNOWN_ERROR;
      }

      ASSERT_RETURN_ONE(waveHandle);
      wave[i].setWaveHandle(waveHandle);
      wave[i].clearWave();
    }

    const int firstBlockOffset = numPointsRootAxis * triggerAxisBlockSize;

    for (int i = 0; GlobalData::Instance().isDebuggingEnabled() && i < MAX_NUM_TRACES; i++)
    {
      wave[i].printDebugInfo();
    }

    double slope, yIntercept;
    CalculateTransformationParameter(bricklet, slope, yIntercept);

    // See also Vernissage manual page 22f
    // triggerAxisBlockSize: number of points in the raw data array which were acquired at the same root axis position
    // firstBlockOffset: offset position where the traceDown data starts

    // *RawBrickletIndex: source index into the raw data vernissage
    // *DataIndex: destination index into the igor wave (the funny index tricks are needed because of the organization of the wave in the memory)

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
    for (int i = 0; i < numPointsRootAxis; i++)
    {
      // ROWS
      for (int j = 0; j < numPointsTriggerAxis; j++)
      {
        // traceUp
        if (traceUpData->moreData)
        {
          rawIndex  = i * triggerAxisBlockSize + j;
          dataIndex = i * numPointsTriggerAxis + j;

          if (dataIndex >= 0 &&
              dataIndex < waveSize &&
              rawIndex < rawBrickletSize &&
              rawIndex >= 0
             )
          {
            rawValue  = rawBrickletDataPtr[rawIndex];
            scaledValue = rawValue * slope + yIntercept;

            traceUpData->fillWave(dataIndex, rawValue, scaledValue);
          }
          else
          {
            DEBUGPRINT("Index out of range in traceUp");
            DEBUGPRINT("traceUpDataIndex=%d,waveSize=%d", dataIndex, waveSize);
            DEBUGPRINT("traceUpRawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
            traceUpData->moreData = false;
          }
        }

        // traceDown
        if (traceDownData->moreData)
        {
          rawIndex = firstBlockOffset + i * triggerAxisBlockSize + j;
          // compared to the traceUpData->dbl the index i is shifted
          // this takes into account that the data in the traceDown is aquired from the highest y value to the lowest y value
          dataIndex = (numPointsRootAxis - (i + 1)) * numPointsTriggerAxis + j;

          if (dataIndex >= 0 &&
              dataIndex < waveSize &&
              rawIndex < rawBrickletSize &&
              rawIndex >= 0
             )
          {
            rawValue  = rawBrickletDataPtr[rawIndex];
            scaledValue = rawValue * slope + yIntercept;

            traceDownData->fillWave(dataIndex, rawValue, scaledValue);
          }
          else
          {
            DEBUGPRINT("Index out of range in traceDown");
            DEBUGPRINT("traceDownDataIndex=%d,waveSize=%d", dataIndex, waveSize);
            DEBUGPRINT("traceDownRawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
            traceDownData->moreData = false;
          }
        }

        // reTraceUp
        if (reTraceUpData->moreData)
        {

          // here we shift the j index, because the data is now acquired from high column number to low column number
          rawIndex  = i * triggerAxisBlockSize + triggerAxisBlockSize - (j + 1);
          dataIndex = i *  numPointsTriggerAxis + j;

          if (dataIndex >= 0 &&
              dataIndex < waveSize &&
              rawIndex < rawBrickletSize &&
              rawIndex >= 0
             )
          {
            rawValue  = rawBrickletDataPtr[rawIndex];
            scaledValue  = rawValue * slope + yIntercept;

            reTraceUpData->fillWave(dataIndex, rawValue, scaledValue);
          }
          else
          {
            DEBUGPRINT("Index out of range in reTraceUp");
            DEBUGPRINT("reTraceUpDataIndex=%d,waveSize=%d", dataIndex, waveSize);
            DEBUGPRINT("reTraceUpRawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
            reTraceUpData->moreData = false;
          }
        }

        // reTraceDown
        if (reTraceDownData->moreData)
        {

          rawIndex  = firstBlockOffset + i * triggerAxisBlockSize + triggerAxisBlockSize - (j + 1);
          dataIndex = (numPointsRootAxis - (i + 1)) * numPointsTriggerAxis   + j;

          if (dataIndex >= 0 &&
              dataIndex < waveSize &&
              rawIndex < rawBrickletSize &&
              rawIndex >= 0
             )
          {

            rawValue = rawBrickletDataPtr[rawIndex];
            scaledValue = rawValue * slope + yIntercept;
            reTraceDownData->fillWave(dataIndex, rawValue, scaledValue);
          }
          else
          {
            DEBUGPRINT("Index out of range in reTraceDown");
            DEBUGPRINT("reTraceDownDataIndex=%d,waveSize=%d", dataIndex, waveSize);
            DEBUGPRINT("reTraceDownRawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
            reTraceDownData->moreData = false;
          }
        }
      }
    }

    for (int i = 0; i < MAX_NUM_TRACES; i++)
    {
      if (wave[i].isEmpty())
      {
        continue;
      }

      CountInt interpolatedDimSizes[MAX_DIMENSIONS + 1];
      MemClear(interpolatedDimSizes, sizeof(interpolatedDimSizes));

      if (resampleData)
      {
        wave[i].pixelSize = pixelSize;

        DEBUGPRINT("Resampling wave %s with pixelSize=%d", wave[i].getWaveName(), pixelSize);

        char dataFolderPath[MAXCMDLEN + 1];
        // flag=3 results in the full path being returned including a trailing colon
        int ret = GetDataFolderNameOrPath(dfHandle, 3, dataFolderPath);

        if (ret != 0)
        {
          return ret;
        }

        char cmd[ARRAY_SIZE];
        // The "ImageInterpolate [...] Pixelate" command is used here
        sprintf(cmd, "ImageInterpolate/PXSZ={%d,%d}/DEST=%sM_PixelatedImage Pixelate %s",
                pixelSize, pixelSize, dataFolderPath, dataFolderPath);
        // quote waveName properly, it might be a liberal name
        CatPossiblyQuotedName(cmd, wave[i].getWaveName());

        ret = XOPSilentCommand(cmd);

        if (ret != 0)
        {
          HISTPRINT("The command _%s_ failed to execute. So the XOP has to be fixed...", cmd);
          continue;
        }

        // kill the un-interpolated wave and invalidate waveHandeVector[i]
        ret = KillWave(wave[i].getWaveHandle());

        if (ret != 0)
        {
          return ret;
        }

        // rename wave from M_PixelatedImage to waveNameVector[i] both sitting in dfHandle
        ret = RenameDataFolderObject(dfHandle, WAVE_OBJECT, "M_PixelatedImage", wave[i].getWaveName());

        if (ret != 0)
        {
          return ret;
        }

        wave[i].setWaveHandle(FetchWaveFromDataFolder(dfHandle, wave[i].getWaveName()));
        ASSERT_RETURN_ONE(wave[i].getWaveHandle());
        // get wave dimensions; needed for setScale below
        int numDimensions;
        ret = MDGetWaveDimensions(wave[i].getWaveHandle(), &numDimensions, interpolatedDimSizes);

        if (ret != 0)
        {
          return ret;
        }
      }

      // set wave note and add info about resampling to the wave note
      setDataWaveNote(brickletID, wave[i]);

      double xAxisDelta, yAxisDelta;

      //  also the wave scaling changes if we have resampled the data
      if (resampleData)
      {
        xAxisDelta = triggerAxis.physicalIncrement * double(dimensionSizes[ROWS]) / double(interpolatedDimSizes[ROWS]);
        yAxisDelta = rootAxis.physicalIncrement * double(dimensionSizes[COLUMNS]) / double(interpolatedDimSizes[COLUMNS]);
      }
      else // original image
      {
        xAxisDelta = triggerAxis.physicalIncrement;
        yAxisDelta = rootAxis.physicalIncrement;
      }

      const double zeroSetScaleOffset = 0.0;
      wave[i].setWaveScaling(ROWS, &xAxisDelta, &zeroSetScaleOffset);
      wave[i].setWaveScaling(COLUMNS, &yAxisDelta, &zeroSetScaleOffset);

      wave[i].setWaveUnits(ROWS, triggerAxis.physicalUnit);
      wave[i].setWaveUnits(COLUMNS, rootAxis.physicalUnit);
      wave[i].setWaveUnits(DATA, bricklet->getMetaDataValue<std::string>(CHANNEL_UNIT_KEY));

      appendToWaveList(dfHandle, wave[i].getWaveHandle(), fullPathOfCreatedWave);
    }

    return SUCCESS;
  }

  int createWaves3D(DataFolderHandle dfHandle, const char* waveBaseName, int brickletID, std::string& fullPathOfCreatedWave)
  {
    CountInt dimensionSizes[MAX_DIMENSIONS + 1];
    MemClear(dimensionSizes, sizeof(dimensionSizes));

    BrickletClass* const bricklet = GlobalData::Instance().getBrickletClassObject(brickletID);
    ASSERT_RETURN_ONE(bricklet);

    void* pBricklet = bricklet->getBrickletPointer();
    ASSERT_RETURN_ONE(pBricklet);

    Vernissage::Session* pSession = GlobalData::Instance().getVernissageSession();
    ASSERT_RETURN_ONE(pSession);

    // pointer to raw data
    int rawBrickletSize = 0;
    const int* rawBrickletDataPtr;
    bricklet->getBrickletContentsBuffer(&rawBrickletDataPtr , rawBrickletSize);

    if (rawBrickletSize == 0 || &rawBrickletDataPtr == NULL)
    {
      HISTPRINT("Could not load bricklet contents.");
      return UNKNOWN_ERROR;
    }

    // V triggerAxis -> V is triggered by X , X is triggered by Y and Y is the root axis

    // check for correct view type codes
    int found = 0;
    std::vector<Vernissage::Session::ViewTypeCode> viewTypeCodes = bricklet->getViewTypeCodes();

    for (std::vector<Vernissage::Session::ViewTypeCode>::const_iterator itViewTypeCodes = viewTypeCodes.begin(); itViewTypeCodes != viewTypeCodes.end(); itViewTypeCodes++)
    {
      if (*itViewTypeCodes == Vernissage::Session::vtc_Spectroscopy)
      {
        found += 1;
      }

      if (*itViewTypeCodes == Vernissage::Session::vtc_2Dof3D)
      {
        found += 2;
      }
    }

    if (found != 3)
    {
      DEBUGPRINT("The 3D data is not of the type vtc_2Dof3D and vtc_Spectroscopy.");
    }

    const Vernissage::Session::AxisDescriptor specAxis = pSession->getAxisDescriptor(pBricklet, pSession->getTriggerAxisName(pBricklet));
    const Vernissage::Session::AxisDescriptor xAxis = pSession->getAxisDescriptor(pBricklet, specAxis.triggerAxisName);
    const Vernissage::Session::AxisDescriptor yAxis = pSession->getAxisDescriptor(pBricklet, xAxis.triggerAxisName);
    const Vernissage::Session::AxisTableSets sets = pSession->getAxisTableSets(pBricklet, pSession->getTriggerAxisName(pBricklet));

    const Vernissage::Session::TableSet xSet = sets.find(specAxis.triggerAxisName)->second;
    const Vernissage::Session::TableSet ySet = sets.find(xAxis.triggerAxisName)->second;

    // V Axis
    int numPointsVAxis = specAxis.clocks;

    if (specAxis.mirrored)
    {
      numPointsVAxis /= 2;
    }

    // X Axis
    int numPointsXAxis = xAxis.clocks;

    if (xAxis.mirrored)
    {
      numPointsXAxis /= 2;
    }

    // Y Axis
    int numPointsYAxis = yAxis.clocks;

    if (yAxis.mirrored)
    {
      numPointsYAxis /= 2;
    }

    // Determine how much space the data occupies in X and Y direction
    // For that we have to take into account the table sets
    // Then we also know how many 3D cubes we have, we can have 1,2 or 4. the same as in the 2D case

    Vernissage::Session::TableSet::const_iterator yIt, xIt;
    int tablePosX, tablePosY;

    // BEGIN Borrowed from SCALA exporter plugin

    // Determine the number of different x values

    // number of y axis points with taking the axis table sets into account
    int numPointsXAxisWithTableBoth = 0;
    // the part of numPointsXAxisWithTableBoth which is used in traceUp
    int numPointsXAxisWithTableFWD = 0;
    // the part of numPointsXAxisWithTableBoth which is used in reTraceUp
    int numPointsXAxisWithTableBWD = 0;

    bool forward;

    xIt = xSet.begin();
    tablePosX = xIt->start;

    while (xIt != xSet.end())
    {
      numPointsXAxisWithTableBoth++;
      forward = (tablePosX <= numPointsXAxis);

      if (forward)
      {
        numPointsXAxisWithTableFWD++;
      }
      else
      {
        numPointsXAxisWithTableBWD++;
      }

      tablePosX += xIt->step;

      if (tablePosX > xIt->stop)
      {
        ++xIt;

        if (xIt != xSet.end())
        {
          tablePosX = xIt->start;
        }
      }
    }

    // Determine the number of different y values

    // number of y axis points with taking the axis table sets into account
    int numPointsYAxisWithTableBoth = 0;
    // the part of numPointsYAxisWithTableBoth which is used in traceUp
    int numPointsYAxisWithTableUp   = 0;
    // the part of numPointsYAxisWithTableBoth which is used in traceDown
    int numPointsYAxisWithTableDown = 0;

    bool up;

    yIt = ySet.begin();
    tablePosY = yIt->start;

    while (yIt != ySet.end())
    {
      numPointsYAxisWithTableBoth++;

      up = (tablePosY <= numPointsYAxis);

      if (up)
      {
        numPointsYAxisWithTableUp++;
      }
      else
      {
        numPointsYAxisWithTableDown++;
      }

      tablePosY += yIt->step;

      if (tablePosY > yIt->stop)
      {
        ++yIt;

        if (yIt != ySet.end())
        {
          tablePosY = yIt->start;
        }
      }
    }

    // END Borrowed from SCALA exporter plugin

    DEBUGPRINT("V Axis # points: %d", numPointsVAxis);

    DEBUGPRINT("X Axis # points with tableSet: Total=%d, Forward=%d, Backward=%d",
               numPointsXAxisWithTableBoth, numPointsXAxisWithTableFWD, numPointsXAxisWithTableBWD);

    DEBUGPRINT("Y Axis # points with tableSet: Total=%d, Up=%d, Down=%d",
               numPointsYAxisWithTableBoth, numPointsYAxisWithTableUp, numPointsYAxisWithTableDown);

    // Theoretical the sizes of the cubes could be different but we are igoring that for now
    if (numPointsXAxisWithTableBWD != 0 && numPointsXAxisWithTableFWD != 0 && numPointsXAxisWithTableFWD != numPointsXAxisWithTableBWD)
    {
      HISTPRINT("BUG: Number of X axis points is different in forward and backward direction. Keep fingers crossed.");
    }

    if (numPointsYAxisWithTableUp != 0 && numPointsYAxisWithTableDown != 0 && numPointsYAxisWithTableUp != numPointsYAxisWithTableDown)
    {
      HISTPRINT("BUG: Number of Y axis points is different in up and down direction. Keep fingers crossed.");
    }

    double xAxisDelta, yAxisDelta;
    double xAxisOffset, yAxisOffset;

    // normally we have table sets with some step width >1, so the physicalIncrement has to be multiplied by this factor
    // Note: this approach assumes that the axis table sets of one axis all have the same physical step width,
    // this is at least the case for Matrix 2.2-1 and Matrix 3.0
    if (ySet.size() > 0)
    {
      yAxisDelta  = yAxis.physicalIncrement * ySet.begin()->step;
      yAxisOffset  = yAxis.physicalIncrement * (ySet.begin()->start - 1);
    }
    else
    {
      yAxisDelta  = yAxis.physicalIncrement;
      yAxisOffset  = 0.0;
    }

    if (xSet.size() > 0)
    {
      xAxisDelta  = xAxis.physicalIncrement * xSet.begin()->step;

      if (numPointsXAxisWithTableFWD != 0)
      {
        // we also scanned in TraceUP direction, therefore we can use the same algorithm like for y-Axis
        xAxisOffset  = xAxis.physicalIncrement * (xSet.begin()->start - 1);
      }
      else
      {
        // we didn't scan in TraceUp direction, therefore we assume that this table sets stop value is somewhere
        // near the start of the scan
        // Typically xSet.begin()->stop is equal to xAxis.clocks and therefore xAxisOffset being zero
        xAxisOffset  = xAxis.physicalIncrement * (xSet.begin()->stop - xAxis.clocks);
      }
    }
    else
    {
      xAxisDelta  = xAxis.physicalIncrement;
      xAxisOffset  = 0.0;
    }

    if (GlobalData::Instance().isDebuggingEnabled())
    {
      DEBUGPRINT("Number of axes we have table sets for: %d", sets.size());
      DEBUGPRINT("Tablesets: xAxis");

      for (Vernissage::Session::TableSet::const_iterator  it = xSet.begin(); it != xSet.end(); it++)
      {
        DEBUGPRINT("start=%d, step=%d, stop=%d", it->start, it->step, it->stop);
      }

      DEBUGPRINT("Tablesets: yAxis");

      for (Vernissage::Session::TableSet::const_iterator it = ySet.begin(); it != ySet.end(); it++)
      {
        DEBUGPRINT("start=%d, step=%d, stop=%d", it->start, it->step, it->stop);
      }

      DEBUGPRINT("xAxisDelta=%g, yAxisDelta=%g", xAxisDelta, yAxisDelta);
      DEBUGPRINT("xAxisOffset=%g, yAxisOffset=%g", xAxisOffset, yAxisOffset);
    }

    // dimensions of the cube
    if (numPointsXAxisWithTableFWD != 0)
    {
      dimensionSizes[ROWS]  = numPointsXAxisWithTableFWD;
    }
    else
    {
      // we only scanned in BWD direction
      dimensionSizes[ROWS]  = numPointsXAxisWithTableBWD;
    }

    //we must always scan in Up direction
    dimensionSizes[COLUMNS] = numPointsYAxisWithTableUp;
    dimensionSizes[LAYERS]  = numPointsVAxis;

    const int waveSize = dimensionSizes[ROWS] * dimensionSizes[COLUMNS] * dimensionSizes[LAYERS];

    DEBUGPRINT("dimensions of the cube: rows=%d,cols=%d,layers=%d",
               dimensionSizes[ROWS], dimensionSizes[COLUMNS], dimensionSizes[LAYERS]);

    WaveClass wave[MAX_NUM_TRACES];
    WaveClass* traceUpData = &wave[TRACE_UP];
    WaveClass* reTraceUpData = &wave[RE_TRACE_UP];
    WaveClass* traceDownData = &wave[TRACE_DOWN];
    WaveClass* reTraceDownData = &wave[RE_TRACE_DOWN];

    // 4 cubes, TraceUp, TraceDown, ReTraceUp, ReTraceDown
    if (numPointsXAxisWithTableFWD != 0 && numPointsXAxisWithTableBWD != 0 &&
        numPointsYAxisWithTableUp != 0 && numPointsYAxisWithTableUp != 0)
    {
      wave[TRACE_UP].setNameAndTraceDir(waveBaseName, TRACE_UP);
      wave[RE_TRACE_UP].setNameAndTraceDir(waveBaseName, RE_TRACE_UP);
      wave[TRACE_DOWN].setNameAndTraceDir(waveBaseName, TRACE_DOWN);
      wave[RE_TRACE_DOWN].setNameAndTraceDir(waveBaseName, RE_TRACE_DOWN);
    }
    // 2 cubes, TraceUp, TraceDown
    else if (numPointsXAxisWithTableBWD == 0 && numPointsYAxisWithTableDown != 0)
    {
      wave[TRACE_UP].setNameAndTraceDir(waveBaseName, TRACE_UP);
      wave[TRACE_DOWN].setNameAndTraceDir(waveBaseName, TRACE_DOWN);
    }
    // 2 cubes, TraceUp, ReTraceUp
    else if (numPointsXAxisWithTableBWD != 0 && numPointsYAxisWithTableDown == 0)
    {
      wave[TRACE_UP].setNameAndTraceDir(waveBaseName, TRACE_UP);
      wave[RE_TRACE_UP].setNameAndTraceDir(waveBaseName, RE_TRACE_UP);
    }
    // 2 cubes, ReTraceUp, ReTraceDown
    else if (numPointsXAxisWithTableFWD == 0 && numPointsYAxisWithTableDown != 0)
    {
      wave[RE_TRACE_UP].setNameAndTraceDir(waveBaseName, RE_TRACE_UP);
      wave[RE_TRACE_DOWN].setNameAndTraceDir(waveBaseName, RE_TRACE_DOWN);
    }
    // 1 cube, TraceUp
    else if (numPointsXAxisWithTableBWD == 0 && numPointsYAxisWithTableDown == 0)
    {
      wave[TRACE_UP].setNameAndTraceDir(waveBaseName, TRACE_UP);
    }
    // 1 cube, ReTraceUp
    else if (numPointsXAxisWithTableFWD == 0 && numPointsYAxisWithTableDown == 0)
    {
      wave[RE_TRACE_UP].setNameAndTraceDir(waveBaseName, RE_TRACE_UP);
    }
    // not possible
    else
    {
      HISTPRINT("BUG: Error in determining the number of cubes.");
      return INTERNAL_ERROR_CONVERTING_DATA;
    }

    // numPointsXAxisWithTableBWD or numPointsXAxisWithTableFWD being zero makes it correct
    const int xAxisBlockSize   = (numPointsXAxisWithTableBWD + numPointsXAxisWithTableFWD) * numPointsVAxis;

    // data index to the start of the TraceDown data (this is the same for all combinations as xAxisBlockSize is set apropriately)
    // in case the traceDown scan does not exist this is also no problem
    const int firstBlockOffset = numPointsYAxisWithTableUp * xAxisBlockSize;

    DEBUGPRINT("xAxisBlockSize=%d,firstBlockOffset=%d", xAxisBlockSize, firstBlockOffset);

    for (int i = 0; i < MAX_NUM_TRACES; i++)
    {
      // skip empty entries
      if (wave[i].isEmpty())
      {
        continue;
      }

      waveHndl waveHandle;
      int ret = MDMakeWave(&waveHandle, wave[i].getWaveName(), dfHandle, dimensionSizes, GlobalData::Instance().getIgorWaveType(), GlobalData::Instance().isOverwriteEnabled<int>());

      if (ret == NAME_WAV_CONFLICT)
      {
        DEBUGPRINT("Wave %s already exists.", wave[i].getWaveName());
        return WAVE_EXIST;
      }
      else if (ret != 0)
      {
        HISTPRINT("Error %d in creating wave %s.", ret, wave[i].getWaveName());
        return UNKNOWN_ERROR;
      }

      ASSERT_RETURN_ONE(waveHandle);
      wave[i].setWaveHandle(waveHandle);
      wave[i].clearWave();
    }

    for (int i = 0; GlobalData::Instance().isDebuggingEnabled() && i < MAX_NUM_TRACES; i++)
    {
      wave[i].printDebugInfo();
    }

    double slope, yIntercept;
    CalculateTransformationParameter(bricklet, slope, yIntercept);

    int rawIndex, dataIndex;
    int rawValue;
    double scaledValue;

    // COLUMNS
    for (int i = 0; i < dimensionSizes[COLUMNS]; i++)
    {
      // ROWS
      for (int j = 0; j < dimensionSizes[ROWS]; j++)
      {
        // LAYERS
        for (int k = 0; k < dimensionSizes[LAYERS]; k++)
        {
          // traceUp
          if (traceUpData->moreData)
          {
            rawIndex  = i * xAxisBlockSize + j * dimensionSizes[LAYERS] + k;
            dataIndex = i * dimensionSizes[ROWS] + j + k * dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

            if (dataIndex >= 0 &&
                dataIndex < waveSize &&
                rawIndex < rawBrickletSize &&
                rawIndex >= 0)
            {

              rawValue  = rawBrickletDataPtr[rawIndex];
              scaledValue = rawValue * slope + yIntercept;

              traceUpData->fillWave(dataIndex, rawValue, scaledValue);
            }
            else
            {
              DEBUGPRINT("Index out of range in traceUp");
              DEBUGPRINT("traceUpDataIndex=%d,waveSize=%d", dataIndex, waveSize);
              DEBUGPRINT("traceUpRawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
              traceUpData->moreData = false;
            }
          }// if traceUpDataPtr

          if (traceDownData->moreData)
          {

            rawIndex  = firstBlockOffset + i * xAxisBlockSize + j * dimensionSizes[LAYERS] + k;
            dataIndex = (dimensionSizes[COLUMNS] - (i + 1)) * dimensionSizes[ROWS] + j + k * dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

            if (dataIndex >= 0 &&
                dataIndex < waveSize &&
                rawIndex < rawBrickletSize &&
                rawIndex >= 0
               )
            {
              rawValue  = rawBrickletDataPtr[rawIndex];
              scaledValue = rawValue * slope + yIntercept;

              traceDownData->fillWave(dataIndex, rawValue, scaledValue);
            }
            else
            {
              DEBUGPRINT("Index out of range in traceDown");
              DEBUGPRINT("traceDownDataIndex=%d,waveSize=%d", dataIndex, waveSize);
              DEBUGPRINT("traceDownRawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
              traceDownData->moreData = false;
            }
          }// if traceDownDataPtr

          if (reTraceUpData->moreData)
          {

            rawIndex  = i * xAxisBlockSize + (dimensionSizes[ROWS] - (j + 1)) * dimensionSizes[LAYERS] + k;
            dataIndex = i * dimensionSizes[ROWS] + j + k * dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

            if (dataIndex >= 0 &&
                dataIndex < waveSize &&
                rawIndex < rawBrickletSize &&
                rawIndex >= 0
               )
            {
              rawValue  = rawBrickletDataPtr[rawIndex];
              scaledValue = rawValue * slope + yIntercept;

              reTraceUpData->fillWave(dataIndex, rawValue, scaledValue);
            }
            else
            {
              DEBUGPRINT("Index out of range in reTraceUp");
              DEBUGPRINT("reTraceUpDataIndex=%d,waveSize=%d", dataIndex, waveSize);
              DEBUGPRINT("reTraceUpRawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
              reTraceUpData->moreData = false;
            }
          }// if reTraceUpDataPtr

          if (reTraceDownData->moreData)
          {

            rawIndex  = firstBlockOffset + i * xAxisBlockSize + (dimensionSizes[ROWS] - (j + 1)) * dimensionSizes[LAYERS] + k;
            dataIndex = (dimensionSizes[COLUMNS] - (i + 1)) * dimensionSizes[ROWS] + j + k * dimensionSizes[ROWS] * dimensionSizes[COLUMNS];

            if (dataIndex >= 0 &&
                dataIndex < waveSize &&
                rawIndex < rawBrickletSize &&
                rawIndex >= 0
               )
            {

              rawValue  = rawBrickletDataPtr[rawIndex];
              scaledValue = rawValue * slope + yIntercept;

              reTraceDownData->fillWave(dataIndex, rawValue, scaledValue);
            }
            else
            {
              DEBUGPRINT("Index out of range in reTraceDown");
              DEBUGPRINT("reTraceDownDataIndex=%d,waveSize=%d", dataIndex, waveSize);
              DEBUGPRINT("reTraceDownRawBrickletIndex=%d,rawBrickletSize=%d", rawIndex, rawBrickletSize);
              reTraceDownData->moreData = false;
            }
          }// if reTraceDownDataPtr
        } // for LAYERS
      } // for ROWS
    } // for COLUMNS

    for (int i = 0; i < MAX_NUM_TRACES; i++)
    {
      if (wave[i].isEmpty())
      {
        continue;
      }

      setDataWaveNote(brickletID, wave[i]);

      wave[i].setWaveScaling(ROWS, &xAxisDelta, &xAxisOffset);
      wave[i].setWaveScaling(COLUMNS, &yAxisDelta, &yAxisOffset);
      wave[i].setWaveScaling(LAYERS, &specAxis.physicalIncrement, &specAxis.physicalStart);

      wave[i].setWaveUnits(ROWS, xAxis.physicalUnit);
      wave[i].setWaveUnits(COLUMNS, yAxis.physicalUnit);
      wave[i].setWaveUnits(LAYERS, specAxis.physicalUnit);
      wave[i].setWaveUnits(DATA, bricklet->getMetaDataValue<std::string>(CHANNEL_UNIT_KEY));

      appendToWaveList(dfHandle, wave[i].getWaveHandle(), fullPathOfCreatedWave);
    }

    return SUCCESS;
  }

} // anonymous namespace

/*
  Creates the real data waves, supports 1D-3D data
*/
int createWaves(DataFolderHandle dfHandle, const char* waveBaseName, int brickletID, bool resampleData,
                int pixelSize, std::string& fullPathOfCreatedWave)
{
  if (GlobalData::Instance().isDebuggingEnabled())
  {
    BrickletClass* const bricklet = GlobalData::Instance().getBrickletClassObject(brickletID);
    ASSERT_RETURN_ONE(bricklet);

    void* pBricklet = bricklet->getBrickletPointer();
    ASSERT_RETURN_ONE(pBricklet);

    DEBUGPRINT("### BrickletID %d ###", brickletID);
    DEBUGPRINT("dimension %d", bricklet->getMetaDataValue<int>("dimension"));

    std::vector<Vernissage::Session::ViewTypeCode> viewTypeCodes = bricklet->getViewTypeCodes();
    std::vector<Vernissage::Session::ViewTypeCode>::const_iterator itViewTypeCodes;

    for (itViewTypeCodes = viewTypeCodes.begin(); itViewTypeCodes != viewTypeCodes.end(); itViewTypeCodes++)
    {
      DEBUGPRINT("viewType %s", viewTypeCodeToString(*itViewTypeCodes).c_str());
    }

    DEBUGPRINT("Axis order is from triggerAxis to rootAxis");

    std::vector<std::string> allAxes = bricklet->getAxes<std::string>();

    for (std::vector<std::string>::const_iterator itAllAxes = allAxes.begin(); itAllAxes != allAxes.end(); itAllAxes++)
    {
      DEBUGPRINT("Axis %s", itAllAxes->c_str());
    }
  }

  BrickletClass* const bricklet = GlobalData::Instance().getBrickletClassObject(brickletID);
  ASSERT_RETURN_ONE(bricklet);
  const int dimension = bricklet->getMetaDataValue<int>("dimension");

  switch (dimension)
  {
  case 1:
    return createWaves1D(dfHandle, waveBaseName, brickletID, fullPathOfCreatedWave);
    break;

  case 2:
    return createWaves2D(dfHandle, waveBaseName, brickletID, resampleData, pixelSize, fullPathOfCreatedWave);
    break;

  case 3:
    return createWaves3D(dfHandle, waveBaseName, brickletID, fullPathOfCreatedWave);
    break;

  default:
    HISTPRINT("Dimension %d can not be handled. Please file a bug report at %s and attach the measured data.", dimension, PROJECTURL);
    return INTERNAL_ERROR_CONVERTING_DATA;
    break;
  }
  return SUCCESS;
}

/*
  create the raw data wave which just holds the raw data as 1D array
*/
int createRawDataWave(DataFolderHandle dfHandle, const char* waveName, int brickletID, std::string& fullPathOfCreatedWaves)
{
  const int* pBuffer;
  int* dataPtr = NULL;
  int count = 0, ret;
  CountInt dimensionSizes[MAX_DIMENSIONS + 1];
  MemClear(dimensionSizes, sizeof(dimensionSizes));

  BrickletClass* bricklet = GlobalData::Instance().getBrickletClassObject(brickletID);
  ASSERT_RETURN_ONE(bricklet);

  WaveClass wave(bricklet->getExtrema());
  wave.setNameAndTraceDir(std::string(waveName), NO_TRACE);

  bricklet ->getBrickletContentsBuffer(&pBuffer, count);

  if (count == 0 || pBuffer == NULL)
  {
    HISTPRINT("Could not load bricklet contents.");
    return 0;
  }

  // create 1D wave with count points
  dimensionSizes[ROWS] = count;

  waveHndl waveHandle;
  ret = MDMakeWave(&waveHandle, wave.getWaveName(), dfHandle, dimensionSizes, NT_I32, GlobalData::Instance().isOverwriteEnabled<int>());

  if (ret == NAME_WAV_CONFLICT)
  {
    DEBUGPRINT("Wave %s already exists.", wave.getWaveName());
    return WAVE_EXIST;
  }
  else if (ret != 0)
  {
    return ret;
  }

  ASSERT_RETURN_ONE(waveHandle);
  wave.setWaveHandle(waveHandle);
  waveHandle = NULL;

  dataPtr = getWaveDataPtr<int>(wave.getWaveHandle());

  ASSERT_RETURN_ONE(dataPtr);
  ASSERT_RETURN_ONE(pBuffer);
  memcpy(dataPtr, pBuffer, count * sizeof(int));

  setDataWaveNote(brickletID,  wave);

  appendToWaveList(dfHandle, wave.getWaveHandle(), fullPathOfCreatedWaves);
  return ret;
}

