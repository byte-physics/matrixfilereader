/*
	The file brickletconverter.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "stdafx.h"

#include "brickletconverter.h"
#include "utils_bricklet.h"
#include "globaldata.h"
#include "waveclass.h"
#include "brickletclass.h"
#include "extremadata.h"
#include "utils_generic.h"

/*
	create the raw data wave which just holds the raw data as 1D array
*/
int createRawDataWave(DataFolderHandle dfHandle,const char *waveName, int brickletID, std::string &fullPathOfCreatedWaves){

	const int *pBuffer;
	int* dataPtr = NULL;
	int count=0,ret;
	CountInt dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));
	WaveClass wave;
	waveHndl waveHandle;

	wave.setNameAndTraceDir(std::string(waveName),NO_TRACE);

	BrickletClass *bricklet = GlobalData::Instance().getBrickletClassObject(brickletID);
	ASSERT_RETURN_ONE(bricklet);

	bricklet ->getBrickletContentsBuffer(&pBuffer,count);

	if(count == 0 || pBuffer == NULL){
		outputToHistory("Could not load bricklet contents.");
		return 0;
	}
	// create 1D wave with count points
	dimensionSizes[ROWS]=count;

	ret = MDMakeWave(&waveHandle,wave.getWaveName(),dfHandle,dimensionSizes,NT_I32,GlobalData::Instance().overwriteEnabledAsInt());
	if(ret == NAME_WAV_CONFLICT){
		sprintf(GlobalData::Instance().outputBuffer,"Wave %s already exists.",wave.getWaveName());
		debugOutputToHistory(GlobalData::Instance().outputBuffer);
		return WAVE_EXIST;	
	}
	else if(ret != 0 ){
		return ret;
	}

	ASSERT_RETURN_ONE(waveHandle);
	wave.setWaveHandle(waveHandle);
	waveHandle = NULL;

	dataPtr = getWaveDataPtr<int>(wave.getWaveHandle());

	ASSERT_RETURN_ONE(dataPtr);
	ASSERT_RETURN_ONE(pBuffer);
	memcpy(dataPtr,pBuffer,count*sizeof(int));
	
	wave.setExtrema(bricklet->getExtrema());
	setDataWaveNote(brickletID,	wave);
	
	appendToWaveList(dfHandle,wave,fullPathOfCreatedWaves);
	return ret;
}

/*
	Creates the real data waves, supports 1D-3D data
*/
int createWaves(DataFolderHandle dfHandle, const char *waveBaseNameChar, int brickletID, bool resampleData,\
				int pixelSize, std::string &fullPathOfCreatedWave){

	int dimension;
	std::vector<Vernissage::Session::ViewTypeCode> viewTypeCodes;
	Vernissage::Session *pSession;
	std::vector<std::string> allAxes;
	Vernissage::Session::AxisDescriptor triggerAxis, rootAxis;
	int numPointsTriggerAxis=-1, numPointsRootAxis=-1, ret=-1, i, j,k;
	waveHndl waveHandle;

	const int *rawBrickletDataPtr;
	int rawBrickletSize=0, waveSize=0, firstBlockOffset=0, triggerAxisBlockSize=0;

	int traceUpRawBrickletIndex, traceUpDataIndex,reTraceUpDataIndex,reTraceUpRawBrickletIndex;
	int traceDownRawBrickletIndex,traceDownDataIndex, reTraceDownRawBrickletIndex,reTraceDownDataIndex;

	const double zeroSetScaleOffset=0.0;

	int rawValue;
	double scaledValue;

	CountInt dimensionSizes[MAX_DIMENSIONS+1], interpolatedDimSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));
	MemClear(interpolatedDimSizes, sizeof(interpolatedDimSizes));

	char cmd[ARRAY_SIZE];
	char dataFolderPath[MAXCMDLEN+1];
	int numDimensions;
	double xAxisDelta, yAxisDelta;
	double xAxisOffset, yAxisOffset;

	std::string waveBaseName(waveBaseNameChar);

	BrickletClass* const bricklet = GlobalData::Instance().getBrickletClassObject(brickletID);

	ASSERT_RETURN_ONE(bricklet);
	void *pBricklet = bricklet->getBrickletPointer();

	ASSERT_RETURN_ONE(pBricklet);
	pSession = GlobalData::Instance().getVernissageSession();

	ASSERT_RETURN_ONE(pSession);

	dimension = bricklet->getMetaDataValueAsInt("dimension");
	allAxes = bricklet->getAxesString();
	viewTypeCodes = bricklet->getViewTypeCodes();

	sprintf(GlobalData::Instance().outputBuffer,"### BrickletID %d ###",brickletID);
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	sprintf(GlobalData::Instance().outputBuffer,"dimension %d",dimension);
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	std::vector<Vernissage::Session::ViewTypeCode>::const_iterator itViewTypeCodes;
	for(itViewTypeCodes = viewTypeCodes.begin(); itViewTypeCodes != viewTypeCodes.end(); itViewTypeCodes++){
		sprintf(GlobalData::Instance().outputBuffer,"viewType %s",viewTypeCodeToString(*itViewTypeCodes).c_str());
		debugOutputToHistory(GlobalData::Instance().outputBuffer);
	}
	
	debugOutputToHistory("Axis order is from triggerAxis to rootAxis");

	std::vector<std::string>::const_iterator itAllAxes;	
	for(itAllAxes = allAxes.begin(); itAllAxes != allAxes.end(); itAllAxes++){
		sprintf(GlobalData::Instance().outputBuffer,"Axis %s",itAllAxes->c_str());
		debugOutputToHistory(GlobalData::Instance().outputBuffer);
	}

	WaveClass wave[MAX_NUM_TRACES];
	WaveClass *traceUpData = &wave[TRACE_UP];
	WaveClass *reTraceUpData = &wave[RE_TRACE_UP];
	WaveClass *traceDownData = &wave[TRACE_DOWN];
	WaveClass *reTraceDownData = &wave[RE_TRACE_DOWN];

	WaveClass wave1D;

	// pointer to raw data
	bricklet->getBrickletContentsBuffer(&rawBrickletDataPtr ,rawBrickletSize);
	if(rawBrickletSize == 0 || &rawBrickletDataPtr == NULL){
		outputToHistory("Could not load bricklet contents.");
		return UNKNOWN_ERROR;
	}

	sprintf(GlobalData::Instance().outputBuffer,"rawBrickletDataPtr =%p,count=%d",&rawBrickletDataPtr,rawBrickletSize);
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	ASSERT_RETURN_ONE(rawBrickletDataPtr);

	// create data for raw->scaled transformation
	// the min and max values here are for the complete bricklet data and not only for one wave
	int xOne, xTwo;
	double yOne, yTwo, slope, yIntercept;
	
	xOne = bricklet->getExtrema().getRawMin();
	xTwo = bricklet->getExtrema().getRawMax();
	yOne = bricklet->getExtrema().getPhysValRawMin();
	yTwo = bricklet->getExtrema().getPhysValRawMax();

	// usually xOne is not equal to xTwo
	if(xOne != xTwo){
		slope = (yOne - yTwo) / (xOne*1.0 - xTwo*1.0);
		yIntercept = yOne - slope*xOne;
	}
	else{
		// but if it is we have to do something different
		// xOne == xTwo means that the minimum is equal to the maximum, so the data is everywhere yOne == yTwo aka constant
		slope = 0;
		yIntercept = yOne;
	}

	sprintf(GlobalData::Instance().outputBuffer,"raw->scaled transformation: xOne=%d,xTwo=%d,yOne=%g,yTwo=%g",xOne,xTwo,yOne,yTwo);
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	sprintf(GlobalData::Instance().outputBuffer,"raw->scaled transformation: slope=%g,yIntercept=%g",slope,yIntercept);
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	if( dimension < 1 || dimension > 3 ){
		sprintf(GlobalData::Instance().outputBuffer,"Dimension %d can not be handled. Please file a bug report and attach the measured data.",dimension);
		outputToHistory(GlobalData::Instance().outputBuffer);
		return INTERNAL_ERROR_CONVERTING_DATA;
	}

	switch(dimension){
	
		case 1:

			triggerAxis = pSession->getAxisDescriptor(pBricklet,pSession->getTriggerAxisName(pBricklet));
			numPointsTriggerAxis = triggerAxis.clocks;
			
			if (triggerAxis.mirrored){
				numPointsTriggerAxis /= 2;
			}
			dimensionSizes[ROWS] = numPointsTriggerAxis;

			wave1D.setNameAndTraceDir(waveBaseName,NO_TRACE);

			ret = MDMakeWave(&waveHandle, wave1D.getWaveName(),dfHandle,dimensionSizes,\
				GlobalData::Instance().getIgorWaveType(),GlobalData::Instance().overwriteEnabledAsInt());
			if(ret == NAME_WAV_CONFLICT){
				sprintf(GlobalData::Instance().outputBuffer,"Wave %s already exists.",wave1D.getWaveName());
				debugOutputToHistory(GlobalData::Instance().outputBuffer);
				return WAVE_EXIST;
			}
			else if(ret != 0 ){
				sprintf(GlobalData::Instance().outputBuffer,"Error %d in creating wave %s.",ret, wave1D.getWaveName());
				outputToHistory(GlobalData::Instance().outputBuffer);
				return UNKNOWN_ERROR;
			}

			ASSERT_RETURN_ONE(waveHandle);

			wave1D.setWaveHandle(waveHandle);
			wave1D.clearWave();
			waveHandle = NULL;

			for(i=0; i < MAX_NUM_TRACES; i++){
				wave[i].printDebugInfo();
			}

			if(!wave1D.moreData){
				return UNKNOWN_ERROR;
			}

			for(i=0; i < numPointsTriggerAxis ; i++){
				rawValue	= rawBrickletDataPtr[i];
				scaledValue = rawValue*slope + yIntercept;
				wave1D.fillWave(i,rawValue,scaledValue);
			}
			setDataWaveNote(brickletID,wave1D);

			wave1D.setWaveScaling(ROWS,&triggerAxis.physicalIncrement,&triggerAxis.physicalStart);
			
			wave1D.setWaveUnits(ROWS,triggerAxis.physicalUnit);
			wave1D.setWaveUnits(DATA,bricklet->getMetaDataValueAsString(CHANNEL_UNIT_KEY));

			appendToWaveList(dfHandle,wave1D,fullPathOfCreatedWave);

			break;

		case 2:

			// Two dimensions, probably an image
			triggerAxis = pSession->getAxisDescriptor(pBricklet,pSession->getTriggerAxisName(pBricklet));
			
			// Determine the length of one "line" of data
			numPointsTriggerAxis = triggerAxis.clocks;
			
			if (triggerAxis.mirrored)
			{
				// The axis has the "mirrored" characteristic, thus it has a
				// "forward" and a "backward" section. Thus, the length of one line
				// is only half the number of clocks that triggered the channel.
				numPointsTriggerAxis /= 2;
			}

			// There must be another axis, because the Bricklet has two dimensions:
			rootAxis = pSession->getAxisDescriptor(pBricklet,triggerAxis.triggerAxisName);

			// Determine the length of one "line" of data
			numPointsRootAxis = rootAxis.clocks;

			if (rootAxis.mirrored)
			{
				// The axis has the "mirrored" characteristic, thus it has a
				// "forward" and a "backward" section. Thus, the length of one line
				// is only half the number of clocks that triggered the channel.
				numPointsRootAxis/= 2;
			}

			sprintf(GlobalData::Instance().outputBuffer,"numPointsRootAxis=%d",numPointsRootAxis);
			debugOutputToHistory(GlobalData::Instance().outputBuffer);

			sprintf(GlobalData::Instance().outputBuffer,"numPointsTriggerAxis=%d",numPointsTriggerAxis);
			debugOutputToHistory(GlobalData::Instance().outputBuffer);

			dimensionSizes[ROWS] = numPointsTriggerAxis;
			dimensionSizes[COLUMNS] = numPointsRootAxis;
			waveSize = dimensionSizes[ROWS]*dimensionSizes[COLUMNS];

			// now we have to disinguish three cases on how many 2D waves we need
			// both are mirrored:		4
			// one mirrored, one not:	2
			// none mirrored:			1
			if( triggerAxis.mirrored && rootAxis.mirrored ){
				
				triggerAxisBlockSize = 2*numPointsTriggerAxis;

				wave[TRACE_UP].setNameAndTraceDir(waveBaseName,TRACE_UP);
				wave[RE_TRACE_UP].setNameAndTraceDir(waveBaseName,RE_TRACE_UP);
				wave[TRACE_DOWN].setNameAndTraceDir(waveBaseName,TRACE_DOWN);
				wave[RE_TRACE_DOWN].setNameAndTraceDir(waveBaseName,RE_TRACE_DOWN);
			}
			else if( triggerAxis.mirrored ){

				triggerAxisBlockSize = 2*numPointsTriggerAxis;

				wave[TRACE_UP].setNameAndTraceDir(waveBaseName,TRACE_UP);
				wave[RE_TRACE_UP].setNameAndTraceDir(waveBaseName,RE_TRACE_UP);
			}
			else if( rootAxis.mirrored ){

				triggerAxisBlockSize = numPointsTriggerAxis;

				wave[TRACE_UP].setNameAndTraceDir(waveBaseName,TRACE_UP);
				wave[TRACE_DOWN].setNameAndTraceDir(waveBaseName,TRACE_DOWN);
			}
			else{
				triggerAxisBlockSize = numPointsTriggerAxis;

				wave[TRACE_UP].setNameAndTraceDir(waveBaseName,TRACE_UP);
			}

			for(i=0; i < MAX_NUM_TRACES; i++){
				// skip empty entries
				if(wave[i].isEmpty()){
					continue;
				}
				ret = MDMakeWave(&waveHandle, wave[i].getWaveName(),dfHandle,dimensionSizes,\
					GlobalData::Instance().getIgorWaveType(),GlobalData::Instance().overwriteEnabledAsInt());

				if(ret == NAME_WAV_CONFLICT){
					sprintf(GlobalData::Instance().outputBuffer,"Wave %s already exists.",wave[i].getWaveName());
					debugOutputToHistory(GlobalData::Instance().outputBuffer);
					return WAVE_EXIST;
				}
				else if(ret != 0 ){
					sprintf(GlobalData::Instance().outputBuffer,"Error %d in creating wave %s.",ret, wave[i].getWaveName());
					outputToHistory(GlobalData::Instance().outputBuffer);
					return UNKNOWN_ERROR;
				}

				ASSERT_RETURN_ONE(waveHandle);
				wave[i].setWaveHandle(waveHandle);
				wave[i].clearWave();
				waveHandle=NULL;
			}

			firstBlockOffset = numPointsRootAxis * triggerAxisBlockSize;

			for(i=0; i < MAX_NUM_TRACES; i++){
				wave[i].printDebugInfo();
			}

			// See also Vernissage manual page 22f
			// triggerAxisBlockSize: number of points in the raw data array which were acquired at the same root axis position
			// firstBlockOffset: offset position where the traceDown data starts

			// *RawBrickletIndex: source index into the raw data vernissage
			// *DataIndex: destination index into the igor wave (the funny index tricks are needed because of the organization of the wave in the memory)

			// data layout of igor waves in memory (Igor XOP Manual p. 238)
			// - the wave is linear in the memory
			// - going along the arrray will first fill the first column from row 0 to end and then the second column and so on

			//// TraceUp aka Forward/Up
			//// ReTraceUp aka Backward/Up
			//// TraceDown aka Forward/Down
			//// ReTraceDown aka Backward/Down
			//// horizontal axis aka X axis in Pascal's Scala Routines aka triggerAxis 		aka 	ROWS
			//// vertical   axis aka Y axis in Pascal's Scala Routines aka rootAxis 		aka		COLUMNS

			// COLUMNS
			for(i = 0; i < numPointsRootAxis; i++){
				// ROWS
				for(j=0; j < numPointsTriggerAxis; j++){

					// traceUp
					if(traceUpData->moreData){
						traceUpRawBrickletIndex			= i*triggerAxisBlockSize+ j;
						traceUpDataIndex				= i*numPointsTriggerAxis   + j;

						if(	traceUpDataIndex >= 0 &&
							traceUpDataIndex < waveSize &&
							traceUpRawBrickletIndex < rawBrickletSize &&
							traceUpRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[traceUpRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

								traceUpData->fillWave(traceUpDataIndex,rawValue,scaledValue);
						}
						else{
							debugOutputToHistory("Index out of range in traceUp");

							sprintf(GlobalData::Instance().outputBuffer,"traceUpDataIndex=%d,waveSize=%d",traceUpDataIndex,waveSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							sprintf(GlobalData::Instance().outputBuffer,"traceUpRawBrickletIndex=%d,rawBrickletSize=%d",traceUpRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							traceUpData->moreData = false;
						}
					}

					// traceDown
					if(traceDownData->moreData){

						traceDownRawBrickletIndex	= firstBlockOffset + i*triggerAxisBlockSize + j;
						// compared to the traceUpData->dbl the index i is shifted
						// this takes into account that the data in the traceDown is aquired from the highest y value to the lowest y value
						traceDownDataIndex			= ( numPointsRootAxis -( i+1) ) * numPointsTriggerAxis   + j;

						if(	traceDownDataIndex >= 0 &&
							traceDownDataIndex < waveSize &&
							traceDownRawBrickletIndex < rawBrickletSize &&
							traceDownRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[traceDownRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

								traceDownData->fillWave(traceDownDataIndex,rawValue,scaledValue);
						}
						else{
							debugOutputToHistory("Index out of range in traceDown");

							sprintf(GlobalData::Instance().outputBuffer,"traceDownDataIndex=%d,waveSize=%d",traceDownDataIndex,waveSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							sprintf(GlobalData::Instance().outputBuffer,"traceDownRawBrickletIndex=%d,rawBrickletSize=%d",traceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							traceDownData->moreData = false;
						}
					}

					// reTraceUp
					if(reTraceUpData->moreData){

						// here we shift the j index, because the data is now acquired from high column number to low column number
						reTraceUpRawBrickletIndex	= i*triggerAxisBlockSize + triggerAxisBlockSize - (j+1);
						reTraceUpDataIndex			= i *  numPointsTriggerAxis + j;

						if(	reTraceUpDataIndex >= 0 &&
							reTraceUpDataIndex < waveSize &&
							reTraceUpRawBrickletIndex < rawBrickletSize &&
							reTraceUpRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[reTraceUpRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

								reTraceUpData->fillWave(reTraceUpDataIndex,rawValue,scaledValue);
						}
						else{
							debugOutputToHistory("Index out of range in reTraceUp");

							sprintf(GlobalData::Instance().outputBuffer,"reTraceUpDataIndex=%d,waveSize=%d",reTraceUpDataIndex,waveSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							sprintf(GlobalData::Instance().outputBuffer,"reTraceUpRawBrickletIndex=%d,rawBrickletSize=%d",reTraceUpRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							reTraceUpData->moreData = false;
						}
					}

					// reTraceDown
					if(reTraceDownData->moreData){

						reTraceDownRawBrickletIndex		= firstBlockOffset + i*triggerAxisBlockSize + triggerAxisBlockSize - (j+1);
						reTraceDownDataIndex			= ( numPointsRootAxis -( i+1) ) * numPointsTriggerAxis   + j;

						if(	reTraceDownDataIndex >= 0 &&
							reTraceDownDataIndex < waveSize &&
							reTraceDownRawBrickletIndex < rawBrickletSize &&
							reTraceDownRawBrickletIndex >= 0
							){

								rawValue	= rawBrickletDataPtr[reTraceDownRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;
								reTraceDownData->fillWave(reTraceDownDataIndex,rawValue,scaledValue);
						}
						else{
							debugOutputToHistory("Index out of range in reTraceDown");

							sprintf(GlobalData::Instance().outputBuffer,"reTraceDownDataIndex=%d,waveSize=%d",reTraceDownDataIndex,waveSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							sprintf(GlobalData::Instance().outputBuffer,"reTraceDownRawBrickletIndex=%d,rawBrickletSize=%d",reTraceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							reTraceDownData->moreData = false;
						}
					}
				}
			}

			for(i=0; i < MAX_NUM_TRACES; i++){
				if(wave[i].isEmpty()){
					continue;
				}

				if( resampleData ){
					wave[i].pixelSize = pixelSize;

					sprintf(GlobalData::Instance().outputBuffer,"Resampling wave %s with pixelSize=%d",wave[i].getWaveName(),pixelSize);
					debugOutputToHistory(GlobalData::Instance().outputBuffer);

					// flag=3 results in the full path being returned including a trailing colon
					ret = GetDataFolderNameOrPath(dfHandle, 3, dataFolderPath);
					if(ret != 0){
						return ret;
					}
					// The "ImageInterpolate [...] Pixelate" command is used here
					sprintf(cmd,"ImageInterpolate/PXSZ={%d,%d}/DEST=%sM_PixelatedImage Pixelate %s",\
						pixelSize,pixelSize,dataFolderPath,dataFolderPath);
					// quote waveName properly, it might be a liberal name
					CatPossiblyQuotedName(cmd,wave[i].getWaveName());
					if(GlobalData::Instance().debuggingEnabled()){
						debugOutputToHistory(cmd);
					}
					ret = XOPSilentCommand(cmd);
					if(ret != 0){
						sprintf(GlobalData::Instance().outputBuffer,"The command _%s_ failed to execute. So the XOP has to be fixed...",cmd);
						outputToHistory(GlobalData::Instance().outputBuffer);
						continue;
					}

					// kill the un-interpolated wave and invalidate waveHandeVector[i]
					ret = KillWave(wave[i].getWaveHandle());
					if(ret != 0){
						return ret;
					}
					// rename wave from M_PixelatedImage to waveNameVector[i] both sitting in dfHandle
					ret = RenameDataFolderObject(dfHandle,WAVE_OBJECT,"M_PixelatedImage",wave[i].getWaveName());
					if(ret != 0){
						return ret;
					}
					wave[i].setWaveHandle(FetchWaveFromDataFolder(dfHandle,wave[i].getWaveName()));
					ASSERT_RETURN_ONE(wave[i].getWaveHandle());
					// get wave dimensions; needed for setScale below
					ret = MDGetWaveDimensions(wave[i].getWaveHandle(),&numDimensions,interpolatedDimSizes);
					if(ret != 0){
						return ret;
					}
				}
		
				// set wave note and add info about resampling to the wave note
				setDataWaveNote(brickletID,wave[i]);

				//  also the wave scaling changes if we have resampled the data
				if( resampleData ){
					xAxisDelta = triggerAxis.physicalIncrement*double(dimensionSizes[ROWS])/double(interpolatedDimSizes[ROWS]);
					yAxisDelta = rootAxis.physicalIncrement*double(dimensionSizes[COLUMNS])/double(interpolatedDimSizes[COLUMNS]);
				}
				else{// original image
					xAxisDelta = triggerAxis.physicalIncrement;
					yAxisDelta = rootAxis.physicalIncrement;
				}
				wave[i].setWaveScaling(ROWS,&xAxisDelta,&zeroSetScaleOffset);
				wave[i].setWaveScaling(COLUMNS,&yAxisDelta,&zeroSetScaleOffset);

				wave[i].setWaveUnits(ROWS,triggerAxis.physicalUnit);
				wave[i].setWaveUnits(COLUMNS,rootAxis.physicalUnit);
				wave[i].setWaveUnits(DATA,bricklet->getMetaDataValueAsString(CHANNEL_UNIT_KEY));

				appendToWaveList(dfHandle,wave[i],fullPathOfCreatedWave);
			}
			break;

		case 3:
			// V triggerAxis -> V is triggered by X , X is triggered by Y and Y is the root axis

			// check for correct view type codes
			int found=0;
			for(itViewTypeCodes = viewTypeCodes.begin(); itViewTypeCodes != viewTypeCodes.end(); itViewTypeCodes++){
				if(*itViewTypeCodes == Vernissage::Session::vtc_Spectroscopy){
					found +=1;
				}
				if(*itViewTypeCodes == Vernissage::Session::vtc_2Dof3D){
					found +=2;
				}
			}

			if(found != 3){
				debugOutputToHistory("The 3D data is not of the type vtc_2Dof3D and vtc_Spectroscopy.");
			}

			Vernissage::Session::AxisDescriptor specAxis = pSession->getAxisDescriptor(pBricklet,pSession->getTriggerAxisName(pBricklet));
			Vernissage::Session::AxisDescriptor xAxis = pSession->getAxisDescriptor(pBricklet, specAxis.triggerAxisName);
			Vernissage::Session::AxisDescriptor yAxis = pSession->getAxisDescriptor(pBricklet, xAxis.triggerAxisName);
			Vernissage::Session::AxisTableSets sets = pSession->getAxisTableSets(pBricklet, pSession->getTriggerAxisName(pBricklet));

			Vernissage::Session::TableSet xSet = sets[specAxis.triggerAxisName];
			Vernissage::Session::TableSet ySet = sets[xAxis.triggerAxisName];

			int xAxisBlockSize=0,yAxisBlockSize=0;

			// V Axis
			int numPointsVAxis = specAxis.clocks;
			
			if (specAxis.mirrored){
				numPointsVAxis /= 2;
			}

			// X Axis
			int numPointsXAxis = xAxis.clocks;

			if(xAxis.mirrored){
				numPointsXAxis /= 2;
			}

			// Y Axis
			int numPointsYAxis = yAxis.clocks;
			
			if(yAxis.mirrored){
				numPointsYAxis /= 2;
			}

			// Determine how much space the data occupies in X and Y direction
			// For that we have to take into account the table sets
			// Then we also know how many 3D cubes we have, we can have 1,2 or 4. the same as in the 2D case

			Vernissage::Session::TableSet::const_iterator yIt,xIt;
			int tablePosX,tablePosY;

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

				if (forward){
					numPointsXAxisWithTableFWD++;
				}
				else{
					numPointsXAxisWithTableBWD++;
				}

				tablePosX += xIt->step;

				if (tablePosX > xIt->stop){
					++xIt;

					if(xIt != xSet.end()){
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

				if (up){
					numPointsYAxisWithTableUp++;
				}
				else{
					numPointsYAxisWithTableDown++;
				}

				tablePosY += yIt->step;

				if (tablePosY > yIt->stop){
					++yIt;

					if(yIt != ySet.end()){
						tablePosY = yIt->start;
					}
				}
			}

			// END Borrowed from SCALA exporter plugin

			sprintf(GlobalData::Instance().outputBuffer,"V Axis # points: %d",numPointsVAxis);
			debugOutputToHistory(GlobalData::Instance().outputBuffer);

			sprintf(GlobalData::Instance().outputBuffer,"X Axis # points with tableSet: Total=%d, Forward=%d, Backward=%d",
				numPointsXAxisWithTableBoth,numPointsXAxisWithTableFWD,numPointsXAxisWithTableBWD);
			debugOutputToHistory(GlobalData::Instance().outputBuffer);

			sprintf(GlobalData::Instance().outputBuffer,"Y Axis # points with tableSet: Total=%d, Up=%d, Down=%d",
				numPointsYAxisWithTableBoth,numPointsYAxisWithTableUp,numPointsYAxisWithTableDown);
			debugOutputToHistory(GlobalData::Instance().outputBuffer);

			// Theoretical the sizes of the cubes could be different but we are igoring that for now
			if(numPointsXAxisWithTableBWD != 0 && numPointsXAxisWithTableFWD != 0 && numPointsXAxisWithTableFWD != numPointsXAxisWithTableBWD){
				sprintf(GlobalData::Instance().outputBuffer,"BUG: Number of X axis points is different in forward and backward direction. Keep fingers crossed.");
				outputToHistory(GlobalData::Instance().outputBuffer);
			}
			if(numPointsYAxisWithTableUp != 0 && numPointsYAxisWithTableDown != 0 && numPointsYAxisWithTableUp != numPointsYAxisWithTableDown){
				sprintf(GlobalData::Instance().outputBuffer,"BUG: Number of Y axis points is different in up and down direction. Keep fingers crossed.");
				outputToHistory(GlobalData::Instance().outputBuffer);
			}

			// normally we have table sets with some step width >1, so the physicalIncrement has to be multiplied by this factor
			// Note: this approach assumes that the axis table sets of one axis all have the same physical step width,
			// this is at least the case for Matrix 2.2-1 and Matrix 3.0
			if( ySet.size() > 0){
				yAxisDelta	= yAxis.physicalIncrement * ySet.begin()->step;
				yAxisOffset	= yAxis.physicalIncrement *( ySet.begin()->start - 1 );
			}
			else{
				yAxisDelta	= yAxis.physicalIncrement;
				yAxisOffset	= 0.0;
			}

			if( xSet.size() > 0 ){
				xAxisDelta	= xAxis.physicalIncrement * xSet.begin()->step;
				if( numPointsXAxisWithTableFWD != 0 ){
					// we also scanned in TraceUP direction, therefore we can use the same algorithm like for y-Axis
					xAxisOffset	= xAxis.physicalIncrement *( xSet.begin()->start - 1 );
				}
				else{
					// we didn't scan in TraceUp direction, therefore we assume that this table sets stop value is somewhere
					// near the start of the scan
					// Typically xSet.begin()->stop is equal to xAxis.clocks and therefore xAxisOffset being zero
					xAxisOffset	= xAxis.physicalIncrement *( xSet.begin()->stop - xAxis.clocks );
				}
			}
			else{
				xAxisDelta	= xAxis.physicalIncrement;
				xAxisOffset	= 0.0;
			}

			if(GlobalData::Instance().debuggingEnabled()){
				Vernissage::Session::TableSet::const_iterator it;

				sprintf(GlobalData::Instance().outputBuffer,"Number of axes we have table sets for: %d",sets.size());
				debugOutputToHistory(GlobalData::Instance().outputBuffer);

				debugOutputToHistory("Tablesets: xAxis");
				for( it= xSet.begin(); it != xSet.end(); it++){
					sprintf(GlobalData::Instance().outputBuffer,"start=%d, step=%d, stop=%d",it->start,it->step,it->stop);
					debugOutputToHistory(GlobalData::Instance().outputBuffer);
				}

				debugOutputToHistory("Tablesets: yAxis");
				for( it= ySet.begin(); it != ySet.end(); it++){
					sprintf(GlobalData::Instance().outputBuffer,"start=%d, step=%d, stop=%d",it->start,it->step,it->stop);
					debugOutputToHistory(GlobalData::Instance().outputBuffer);
				}

				sprintf(GlobalData::Instance().outputBuffer,"xAxisDelta=%g, yAxisDelta=%g",xAxisDelta,yAxisDelta);
				debugOutputToHistory(GlobalData::Instance().outputBuffer);
				sprintf(GlobalData::Instance().outputBuffer,"xAxisOffset=%g, yAxisOffset=%g",xAxisOffset,yAxisOffset);
				debugOutputToHistory(GlobalData::Instance().outputBuffer);
			}

			// dimensions of the cube
			if(numPointsXAxisWithTableFWD != 0)
				dimensionSizes[ROWS]	= numPointsXAxisWithTableFWD;
			else{
				// we only scanned in BWD direction
				dimensionSizes[ROWS]	= numPointsXAxisWithTableBWD;
			}

			//we must always scan in Up direction
			dimensionSizes[COLUMNS] = numPointsYAxisWithTableUp;
			dimensionSizes[LAYERS]  = numPointsVAxis;

			waveSize = dimensionSizes[ROWS]*dimensionSizes[COLUMNS]*dimensionSizes[LAYERS];

			sprintf(GlobalData::Instance().outputBuffer,"dimensions of the cube: rows=%d,cols=%d,layers=%d",
				dimensionSizes[ROWS],dimensionSizes[COLUMNS],dimensionSizes[LAYERS]);
			debugOutputToHistory(GlobalData::Instance().outputBuffer);

			// 4 cubes, TraceUp, TraceDown, ReTraceUp, ReTraceDown
			if(	numPointsXAxisWithTableFWD != 0 && numPointsXAxisWithTableBWD != 0 &&
				numPointsYAxisWithTableUp != 0 && numPointsYAxisWithTableUp != 0){
				wave[TRACE_UP].setNameAndTraceDir(waveBaseName,TRACE_UP);
				wave[RE_TRACE_UP].setNameAndTraceDir(waveBaseName,RE_TRACE_UP);
				wave[TRACE_DOWN].setNameAndTraceDir(waveBaseName,TRACE_DOWN);
				wave[RE_TRACE_DOWN].setNameAndTraceDir(waveBaseName,RE_TRACE_DOWN);
			}
			// 2 cubes, TraceUp, TraceDown
			else if(numPointsXAxisWithTableBWD == 0 && numPointsYAxisWithTableDown != 0){
				wave[TRACE_UP].setNameAndTraceDir(waveBaseName,TRACE_UP);
				wave[TRACE_DOWN].setNameAndTraceDir(waveBaseName,TRACE_DOWN);
			}
			// 2 cubes, TraceUp, ReTraceUp
			else if(numPointsXAxisWithTableBWD != 0 && numPointsYAxisWithTableDown == 0){
				wave[TRACE_UP].setNameAndTraceDir(waveBaseName,TRACE_UP);
				wave[RE_TRACE_UP].setNameAndTraceDir(waveBaseName,RE_TRACE_UP);
			}
			// 2 cubes, ReTraceUp, ReTraceDown
			else if(numPointsXAxisWithTableFWD == 0 && numPointsYAxisWithTableDown != 0){
				wave[RE_TRACE_UP].setNameAndTraceDir(waveBaseName,RE_TRACE_UP);
				wave[RE_TRACE_DOWN].setNameAndTraceDir(waveBaseName,RE_TRACE_DOWN);
			}
			// 1 cube, TraceUp
			else if(numPointsXAxisWithTableBWD == 0 && numPointsYAxisWithTableDown == 0){
				wave[TRACE_UP].setNameAndTraceDir(waveBaseName,TRACE_UP);
			}
			// 1 cube, ReTraceUp
			else if(numPointsXAxisWithTableFWD == 0 && numPointsYAxisWithTableDown == 0){
				wave[RE_TRACE_UP].setNameAndTraceDir(waveBaseName,RE_TRACE_UP);
			}
			// not possible
			else{
				outputToHistory("BUG: Error in determining the number of cubes.");
				return INTERNAL_ERROR_CONVERTING_DATA;
			}
			// numPointsXAxisWithTableBWD or numPointsXAxisWithTableFWD being zero makes it correct
			xAxisBlockSize	   = ( numPointsXAxisWithTableBWD + numPointsXAxisWithTableFWD ) * numPointsVAxis;

			// data index to the start of the TraceDown data (this is the same for all combinations as xAxisBlockSize is set apropriately)
			// in case the traceDown scan does not exist this is also no problem
			firstBlockOffset = numPointsYAxisWithTableUp*xAxisBlockSize;

			sprintf(GlobalData::Instance().outputBuffer,"xAxisBlockSize=%d,firstBlockOffset=%d",xAxisBlockSize,firstBlockOffset);
			debugOutputToHistory(GlobalData::Instance().outputBuffer);

			for(i=0; i < MAX_NUM_TRACES; i++){
				// skip empty entries
				if(wave[i].isEmpty()){
					continue;
				}

				ret = MDMakeWave(&waveHandle, wave[i].getWaveName(),dfHandle,dimensionSizes,GlobalData::Instance().getIgorWaveType(),GlobalData::Instance().overwriteEnabledAsInt());

				if(ret == NAME_WAV_CONFLICT){
					sprintf(GlobalData::Instance().outputBuffer,"Wave %s already exists.",wave[i].getWaveName());
					debugOutputToHistory(GlobalData::Instance().outputBuffer);
					return WAVE_EXIST;
				}
				else if(ret != 0 ){
					sprintf(GlobalData::Instance().outputBuffer,"Error %d in creating wave %s.",ret, wave[i].getWaveName());
					outputToHistory(GlobalData::Instance().outputBuffer);
					return UNKNOWN_ERROR;
				}
				ASSERT_RETURN_ONE(waveHandle);
				wave[i].setWaveHandle(waveHandle);
				wave[i].clearWave();
				waveHandle = NULL;
			}

			for(i=0; i < MAX_NUM_TRACES; i++){
				wave[i].printDebugInfo();
			}

			// COLUMNS
			for(i = 0; i < dimensionSizes[COLUMNS]; i++){
				// ROWS
				for(j=0; j < dimensionSizes[ROWS]; j++){
					// LAYERS
					for(k=0; k < dimensionSizes[LAYERS]; k++){

						// traceUp
						if(traceUpData->moreData){
							traceUpRawBrickletIndex	= i*xAxisBlockSize + j*dimensionSizes[LAYERS] + k;
							traceUpDataIndex		= i*dimensionSizes[ROWS] + j + k*dimensionSizes[ROWS]*dimensionSizes[COLUMNS];

							if(	traceUpDataIndex >= 0 &&
								traceUpDataIndex < waveSize &&
								traceUpRawBrickletIndex < rawBrickletSize &&
								traceUpRawBrickletIndex >= 0 ){

									rawValue	= rawBrickletDataPtr[traceUpRawBrickletIndex];
									scaledValue = rawValue*slope + yIntercept;

									traceUpData->fillWave(traceUpDataIndex,rawValue,scaledValue);
							}
							else{
								debugOutputToHistory("Index out of range in traceUp");

								sprintf(GlobalData::Instance().outputBuffer,"traceUpDataIndex=%d,waveSize=%d",traceUpDataIndex,waveSize);
								debugOutputToHistory(GlobalData::Instance().outputBuffer);

								sprintf(GlobalData::Instance().outputBuffer,"traceUpRawBrickletIndex=%d,rawBrickletSize=%d",traceUpRawBrickletIndex,rawBrickletSize);
								debugOutputToHistory(GlobalData::Instance().outputBuffer);

								traceUpData->moreData = false;
							}
					}// if traceUpDataPtr

					if(traceDownData->moreData){

							traceDownRawBrickletIndex = firstBlockOffset + i*xAxisBlockSize + j*dimensionSizes[LAYERS] + k;
							traceDownDataIndex		  = (dimensionSizes[COLUMNS] -(i+1))*dimensionSizes[ROWS] + j + k*dimensionSizes[ROWS]*dimensionSizes[COLUMNS];

						if(	traceDownDataIndex >= 0 &&
							traceDownDataIndex < waveSize &&
							traceDownRawBrickletIndex < rawBrickletSize &&
							traceDownRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[traceDownRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

								traceDownData->fillWave(traceDownDataIndex,rawValue,scaledValue);
						}
						else{
							debugOutputToHistory("Index out of range in traceDown");

							sprintf(GlobalData::Instance().outputBuffer,"traceDownDataIndex=%d,waveSize=%d",traceDownDataIndex,waveSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							sprintf(GlobalData::Instance().outputBuffer,"traceDownRawBrickletIndex=%d,rawBrickletSize=%d",traceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							traceDownData->moreData = false;
						}
					}// if traceDownDataPtr

					if(reTraceUpData->moreData){

						reTraceUpRawBrickletIndex = i*xAxisBlockSize + (dimensionSizes[ROWS] - (j+1))*dimensionSizes[LAYERS] + k;
						reTraceUpDataIndex		  = i*dimensionSizes[ROWS] + j + k*dimensionSizes[ROWS]*dimensionSizes[COLUMNS];

						if(	reTraceUpDataIndex >= 0 &&
							reTraceUpDataIndex < waveSize &&
							reTraceUpRawBrickletIndex < rawBrickletSize &&
							reTraceUpRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[reTraceUpRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

								reTraceUpData->fillWave(reTraceUpDataIndex,rawValue,scaledValue);
						}
						else{
							debugOutputToHistory("Index out of range in reTraceUp");

							sprintf(GlobalData::Instance().outputBuffer,"reTraceUpDataIndex=%d,waveSize=%d",reTraceUpDataIndex,waveSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							sprintf(GlobalData::Instance().outputBuffer,"reTraceUpRawBrickletIndex=%d,rawBrickletSize=%d",reTraceUpRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							reTraceUpData->moreData = false;
						}
					}// if reTraceUpDataPtr

					if(reTraceDownData->moreData){

						reTraceDownRawBrickletIndex	= firstBlockOffset + i*xAxisBlockSize + (dimensionSizes[ROWS] - (j+1))*dimensionSizes[LAYERS] + k;
						reTraceDownDataIndex		= ( dimensionSizes[COLUMNS] - (i+1) )*dimensionSizes[ROWS] + j + k*dimensionSizes[ROWS]*dimensionSizes[COLUMNS];

						if(	reTraceDownDataIndex >= 0 &&
							reTraceDownDataIndex < waveSize &&
							reTraceDownRawBrickletIndex < rawBrickletSize &&
							reTraceDownRawBrickletIndex >= 0
							){

								rawValue	= rawBrickletDataPtr[reTraceDownRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;
								reTraceDownData->fillWave(reTraceDownDataIndex,rawValue,scaledValue);

								//if(k < 10 && i < 2 && j < 2){
								//	sprintf(GlobalData::Instance().outputBuffer,"j(rows)=%d,i(cols)=%d,k(layers)=%d,reTraceDownRawBrickletIndex=%d,reTraceDownDataIndex=%d,rawValue=%d,scaledValue=%g",
								//		j,i,k,reTraceDownRawBrickletIndex,reTraceDownDataIndex,rawValue,scaledValue);
								//	debugOutputToHistory(GlobalData::Instance().outputBuffer);
								//}
						}
						else{
							debugOutputToHistory("Index out of range in reTraceDown");

							sprintf(GlobalData::Instance().outputBuffer,"reTraceDownDataIndex=%d,waveSize=%d",reTraceDownDataIndex,waveSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							sprintf(GlobalData::Instance().outputBuffer,"reTraceDownRawBrickletIndex=%d,rawBrickletSize=%d",reTraceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(GlobalData::Instance().outputBuffer);

							reTraceDownData->moreData = false;
						}
					}// if reTraceDownDataPtr
				} // for LAYERS
			} // for ROWS
		} // for COLUMNS

			for(i=0; i < MAX_NUM_TRACES; i++){
				if(wave[i].isEmpty()){
					continue;
				}

				setDataWaveNote(brickletID,wave[i]);

				wave[i].setWaveScaling(ROWS,&xAxisDelta,&xAxisOffset);
				wave[i].setWaveScaling(COLUMNS,&yAxisDelta,&yAxisOffset);
				wave[i].setWaveScaling(LAYERS,&specAxis.physicalIncrement,&specAxis.physicalStart);
	
				wave[i].setWaveUnits(ROWS,xAxis.physicalUnit);
				wave[i].setWaveUnits(COLUMNS,yAxis.physicalUnit);
				wave[i].setWaveUnits(LAYERS,specAxis.physicalUnit);
				wave[i].setWaveUnits(DATA,bricklet->getMetaDataValueAsString(CHANNEL_UNIT_KEY));

				appendToWaveList(dfHandle,wave[i],fullPathOfCreatedWave);
			}

			break;
	}
	return SUCCESS;
}
