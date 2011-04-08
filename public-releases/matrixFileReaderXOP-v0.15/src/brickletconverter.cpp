/*
	The file brickletconverter.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "header.h"

#include "brickletconverter.h"

#include <string>
#include <sstream>
#include <vector>

#include "utils.h"
#include "globaldata.h"

namespace {

	struct waveDataPtr{
		float* flt;
		double* dbl;
		bool moreData;
	};

	//Macro version of fillWave(const waveDataPtr &waveData,const int index, const double value,const bool isDoubleWaveType){
	//#define fillWave(A,B,C,D){	if(D){ A.dbl[B] = C;} else{	A.flt[B]	= (float) C; } }

	void clearWave(waveHndl waveHandle,long waveSize){

		waveDataPtr  waveData;

		if(globDataPtr->doubleWaveEnabled()){
			waveData.dbl = getWaveDataPtr<double>(waveHandle);
			waveClearNaN64(waveData.dbl, waveSize);
		}
		else{
			waveData.flt = getWaveDataPtr<float>(waveHandle);
			waveClearNaN32(waveData.flt, waveSize);
		}
	}

	void __forceinline fillWave(const waveDataPtr &waveData,const int index, const double value,const bool isDoubleWaveType){

		if(isDoubleWaveType){
			waveData.dbl[index] = value;
		}
		else{
			waveData.flt[index]	= static_cast<float>(value);
		}
	}

	void __forceinline setExtremaValue(ExtremaData * const extrema,const int rawValue,const double scaledValue){

		if(rawValue < extrema->getRawMin()){
			extrema->setRawMin(rawValue);
			extrema->setPhysValRawMin(scaledValue);
		}
		if(rawValue > extrema->getRawMax()){
			extrema->setRawMax(rawValue);
			extrema->setPhysValRawMax(scaledValue);
		}
	}

	// the NULL check prevents crashes in case we get NULL from waveData
	void setWaveDataPtr(waveDataPtr &waveData,const waveHndl &waveH){

		if(globDataPtr->doubleWaveEnabled()){
			waveData.dbl = getWaveDataPtr<double>(waveH);
			if(waveData.dbl == NULL){
				XOPNotice("BUG: setWaveDataPtr(...) waveData.dbl is NULL, this should not happen...");
				waveData.moreData = false;		
			}
			else{
				waveData.moreData = true;
			}
		}
		else{
			waveData.flt = getWaveDataPtr<float>(waveH);
			if(waveData.flt == NULL){
				XOPNotice("BUG: setWaveDataPtr(...) waveData.flt is NULL, this should not happen...");
				waveData.moreData = false;		
			}
			else{
				waveData.moreData = true;
			}
		}
	}
}

int createWaves(DataFolderHandle dfHandle, const char *waveBaseNameChar, int brickletID, bool resampleData, int pixelSize, std::string &fullPathOfCreatedWave){

	bool isDoubleWaveType = globDataPtr->doubleWaveEnabled();

	int dimension;
	std::vector<Vernissage::Session::ViewTypeCode> viewTypeCodes;
	Vernissage::Session *pSession;
	std::vector<std::string> allAxes;
	Vernissage::Session::AxisDescriptor triggerAxis, rootAxis;
	int numPointsTriggerAxis=-1, numPointsRootAxis=-1, ret=-1, i, j,k;
	unsigned int ii;
	std::vector<waveHndl> waveHandleVector;
	waveHndl waveHandle;
	std::vector<std::string> waveNameVector;
	std::vector<int> traceDirVector;
	std::string waveNote;

	const int *rawBrickletDataPtr = NULL;
	int rawBrickletSize=0, waveSize=0, firstBlockOffset=0, triggerAxisBlockSize=0;

	int traceUpRawBrickletIndex, traceUpDataIndex,reTraceUpDataIndex,reTraceUpRawBrickletIndex, traceDownRawBrickletIndex,traceDownDataIndex, reTraceDownRawBrickletIndex,reTraceDownDataIndex;

	struct waveDataPtr traceUpDataPtr     = {NULL,NULL,false};
	struct waveDataPtr reTraceUpDataPtr   = {NULL,NULL,false};
	struct waveDataPtr traceDownDataPtr   = {NULL,NULL,false};
	struct waveDataPtr reTraceDownDataPtr = {NULL,NULL,false};
	struct waveDataPtr waveData           = {NULL,NULL,false};

	double setScaleOffset=0.0;

	int rawValue;
	double scaledValue;

	std::vector<std::string>::const_iterator itWaveNames;

	CountInt dimensionSizes[MAX_DIMENSIONS+1], interpolatedDimSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));
	MemClear(interpolatedDimSizes, sizeof(interpolatedDimSizes));

	char cmd[ARRAY_SIZE];
	char dataFolderPath[MAXCMDLEN+1];
	int numDimensions;
	double physIncrement;

	std::string waveBaseName(waveBaseNameChar);

	BrickletClass *bricklet = globDataPtr->getBrickletClassObject(brickletID);

	ASSERT_RETURN_ONE(bricklet);
	void *pBricklet = bricklet->getBrickletPointer();

	ASSERT_RETURN_ONE(pBricklet);
	pSession = globDataPtr->getVernissageSession();

	ASSERT_RETURN_ONE(pSession);

	dimension = bricklet->getMetaDataValueAsInt("dimension");
	bricklet->getAxes(allAxes);
	bricklet->getViewTypeCodes(viewTypeCodes);

	sprintf(globDataPtr->outputBuffer,"### BrickletID %d ###",brickletID);
	debugOutputToHistory(globDataPtr->outputBuffer);

	sprintf(globDataPtr->outputBuffer,"dimension %d",dimension);
	debugOutputToHistory(globDataPtr->outputBuffer);

	std::vector<Vernissage::Session::ViewTypeCode>::const_iterator itViewTypeCodes;
	for(itViewTypeCodes = viewTypeCodes.begin(); itViewTypeCodes != viewTypeCodes.end(); itViewTypeCodes++){
		sprintf(globDataPtr->outputBuffer,"viewType %s",viewTypeCodeToString(*itViewTypeCodes).c_str());
		debugOutputToHistory(globDataPtr->outputBuffer);
	}
	
	debugOutputToHistory("Axis order is from triggerAxis to rootAxis");

	std::vector<std::string>::const_iterator itAllAxes;	
	for(itAllAxes = allAxes.begin(); itAllAxes != allAxes.end(); itAllAxes++){
		sprintf(globDataPtr->outputBuffer,"Axis %s",itAllAxes->c_str());
		debugOutputToHistory(globDataPtr->outputBuffer);
	}

	ExtremaData extremaData[3];

	ExtremaData *traceUpExtrema=NULL,*traceDownExtrema=NULL,*reTraceUpExtrema=NULL,*reTraceDownExtrema=NULL;

	// pointer to raw data
	const int* pBuffer;
	bricklet->getBrickletContentsBuffer(&pBuffer,rawBrickletSize);
	if(rawBrickletSize == 0 || &pBuffer == NULL){
		outputToHistory("Could not load bricklet contents.");
		return UNKNOWN_ERROR;
	}

	sprintf(globDataPtr->outputBuffer,"pBuffer=%p,count=%d",&pBuffer,rawBrickletSize);
	debugOutputToHistory(globDataPtr->outputBuffer);

	rawBrickletDataPtr = pBuffer;
	ASSERT_RETURN_ONE(rawBrickletDataPtr);

	// create data for raw->scaled transformation
	// the min and max values here are for the complete bricklet data and not only for one wave
	int xOne, xTwo;
	double yOne, yTwo, slope, yIntercept;
	
	xOne = bricklet->getExtrema().getRawMin();
	xTwo = bricklet->getExtrema().getRawMax();
	yOne = bricklet->getExtrema().getPhysValRawMin();
	yTwo = bricklet->getExtrema().getPhysValRawMax();

	// usually xOne is not euqal to xTwo
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

	sprintf(globDataPtr->outputBuffer,"raw->scaled transformation: xOne=%d,xTwo=%d,yOne=%g,yTwo=%g",xOne,xTwo,yOne,yTwo);
	debugOutputToHistory(globDataPtr->outputBuffer);

	sprintf(globDataPtr->outputBuffer,"raw->scaled transformation: slope=%g,yIntercept=%g",slope,yIntercept);
	debugOutputToHistory(globDataPtr->outputBuffer);

	if( dimension < 1 || dimension > 3 ){
		sprintf(globDataPtr->outputBuffer,"Dimension %d can not be handled. Please file a bug report and attach the measured data.",dimension);
		outputToHistory(globDataPtr->outputBuffer);
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

			ret = MDMakeWave(&waveHandle, waveBaseName.c_str(),dfHandle,dimensionSizes,globDataPtr->getIgorWaveType(),globDataPtr->overwriteEnabledAsInt());
			if(ret == NAME_WAV_CONFLICT){
				sprintf(globDataPtr->outputBuffer,"Wave %s already exists.",waveBaseName.c_str());
				debugOutputToHistory(globDataPtr->outputBuffer);
				return WAVE_EXIST;
			}
			else if(ret != 0 ){
				sprintf(globDataPtr->outputBuffer,"Error %d in creating wave %s.",ret, waveBaseName.c_str());
				outputToHistory(globDataPtr->outputBuffer);
				return UNKNOWN_ERROR;
			}

			ASSERT_RETURN_ONE(waveHandle);
			clearWave(waveHandle,waveSize);
			setWaveDataPtr(waveData,waveHandle);

			// this is false if we got a NULL pointer from setWaveDataPtr
			if(!waveData.moreData){
				return UNKNOWN_ERROR;
			}

			for(i=0; i < numPointsTriggerAxis ; i++){
				
				rawValue	= rawBrickletDataPtr[i];
				scaledValue = rawValue*slope + yIntercept;
				fillWave(waveData,i,scaledValue,isDoubleWaveType);

				setExtremaValue(&extremaData[0],rawValue,scaledValue);
			}
			// traceDir is -1 as this does not make sense here
			setDataWaveNote(brickletID,-1,extremaData[0],waveHandle);

			MDSetWaveScaling(waveHandle,ROWS,&triggerAxis.physicalIncrement,&triggerAxis.physicalStart);
			
			MDSetWaveUnits(waveHandle,ROWS,WStringToString(triggerAxis.physicalUnit).c_str());
			MDSetWaveUnits(waveHandle,-1,bricklet->getMetaDataValueAsString("channelUnit").c_str());
			fullPathOfCreatedWave.append(getFullWavePath(dfHandle,waveHandle));
			fullPathOfCreatedWave.append(";");

			waveHandle = NULL;

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

			sprintf(globDataPtr->outputBuffer,"numPointsRootAxis=%d",numPointsRootAxis);
			debugOutputToHistory(globDataPtr->outputBuffer);

			sprintf(globDataPtr->outputBuffer,"numPointsTriggerAxis=%d",numPointsTriggerAxis);
			debugOutputToHistory(globDataPtr->outputBuffer);

			dimensionSizes[ROWS] = numPointsTriggerAxis;
			dimensionSizes[COLUMNS] = numPointsRootAxis;
			waveSize = dimensionSizes[ROWS]*dimensionSizes[COLUMNS];

			// now we have to disinguish three cases on how many 2D waves we need
			// both are mirrored:		4
			// one mirrored, one not:	2
			// none mirrored:			1
			if( triggerAxis.mirrored && rootAxis.mirrored ){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				traceDirVector.push_back(TRACE_UP);
				waveNameVector.push_back(waveBaseName + RE_TRACE_UP_STRING);
				traceDirVector.push_back(RE_TRACE_UP);
				waveNameVector.push_back(waveBaseName + TRACE_DOWN_STRING);
				traceDirVector.push_back(TRACE_DOWN);
				waveNameVector.push_back(waveBaseName + RE_TRACE_DOWN_STRING);
				traceDirVector.push_back(RE_TRACE_DOWN);
			}
			else if( triggerAxis.mirrored ){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				traceDirVector.push_back(TRACE_UP);
				waveNameVector.push_back(waveBaseName + RE_TRACE_UP_STRING);
				traceDirVector.push_back(RE_TRACE_UP);
			}
			else if( rootAxis.mirrored ){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				traceDirVector.push_back(TRACE_UP);
				waveNameVector.push_back(waveBaseName + TRACE_DOWN_STRING);
				traceDirVector.push_back(TRACE_DOWN);
			}
			else{
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				traceDirVector.push_back(TRACE_UP);
			}

			for(itWaveNames = waveNameVector.begin(); itWaveNames != waveNameVector.end(); itWaveNames++){

				ret = MDMakeWave(&waveHandle, itWaveNames->c_str(),dfHandle,dimensionSizes,globDataPtr->getIgorWaveType(),globDataPtr->overwriteEnabledAsInt());

				if(ret == NAME_WAV_CONFLICT){
					sprintf(globDataPtr->outputBuffer,"Wave %s already exists.",itWaveNames->c_str());
					debugOutputToHistory(globDataPtr->outputBuffer);
					return WAVE_EXIST;
				}
				else if(ret != 0 ){
					sprintf(globDataPtr->outputBuffer,"Error %d in creating wave %s.",ret, itWaveNames->c_str());
					outputToHistory(globDataPtr->outputBuffer);
					return UNKNOWN_ERROR;
				}

				ASSERT_RETURN_ONE(waveHandle);
				waveHandleVector.push_back(waveHandle);
				clearWave(waveHandle,waveSize);
			}

			//// TraceUp aka Forward/Up
			//// ReTraceUp aka Backward/Up
			//// TraceDown aka Forward/Down
			//// ReTraceDown aka Backward/Down
			//// horizontal axis aka X axis in Pascal's Scala Routines aka triggerAxis 		aka 	ROWS
			//// vertical   axis aka Y axis in Pascal's Scala Routines aka rootAxis 		aka		COLUMNS

			// both axes are mirrored
			if(waveHandleVector.size() == 4){

				triggerAxisBlockSize = 2*numPointsTriggerAxis;

				setWaveDataPtr(traceUpDataPtr,waveHandleVector[0]);
				traceUpExtrema	   = &extremaData[0];

				setWaveDataPtr(reTraceUpDataPtr,waveHandleVector[1]);
				reTraceUpExtrema   = &extremaData[1];

				setWaveDataPtr(traceDownDataPtr,waveHandleVector[2]);
				traceDownExtrema   = &extremaData[2];

				setWaveDataPtr(reTraceDownDataPtr,waveHandleVector[3]);
				reTraceDownExtrema = &extremaData[3];
			}
			// only triggerAxis (X) is mirrored
			else if(waveHandleVector.size() == 2 && triggerAxis.mirrored){

				triggerAxisBlockSize = 2*numPointsTriggerAxis;

				setWaveDataPtr(traceUpDataPtr,waveHandleVector[0]);
				traceUpExtrema	   = &extremaData[0];

				setWaveDataPtr(reTraceUpDataPtr,waveHandleVector[1]);
				reTraceUpExtrema = &extremaData[1];
			}
			// only rootAxis (Y) is mirrored
			else if(waveHandleVector.size() == 2 && rootAxis.mirrored){

				triggerAxisBlockSize = numPointsTriggerAxis;
				
				setWaveDataPtr(traceUpDataPtr,waveHandleVector[0]);
				traceUpExtrema	   = &extremaData[0];

				setWaveDataPtr(traceDownDataPtr,waveHandleVector[1]);
				traceDownExtrema   = &extremaData[1];
			}
			// no mirroring
			else if(waveHandleVector.size() == 1 && rootAxis.mirrored == false && triggerAxis.mirrored == false){

				triggerAxisBlockSize = numPointsTriggerAxis;

				setWaveDataPtr(traceUpDataPtr,waveHandleVector[0]);
				traceUpExtrema	   = &extremaData[0];
			}
			else{
				outputToHistory("BUG: createWaves()...");
				return UNKNOWN_ERROR;
			}

			firstBlockOffset	 = numPointsRootAxis * triggerAxisBlockSize;

			sprintf(globDataPtr->outputBuffer,"traceUp: flt=%p, dbl=%p, moreData=%s",traceUpDataPtr.flt,traceUpDataPtr.dbl,traceUpDataPtr.moreData ? "true" : "false");
			debugOutputToHistory(globDataPtr->outputBuffer);
			sprintf(globDataPtr->outputBuffer,"reTraceUp: flt=%p, dbl=%p, moreData=%s",reTraceUpDataPtr.flt,reTraceUpDataPtr.dbl,reTraceUpDataPtr.moreData ? "true" : "false");
			debugOutputToHistory(globDataPtr->outputBuffer);
			sprintf(globDataPtr->outputBuffer,"traceDown: flt=%p, dbl=%p, moreData=%s",traceDownDataPtr.flt,traceDownDataPtr.dbl,traceDownDataPtr.moreData ? "true" : "false");
			debugOutputToHistory(globDataPtr->outputBuffer);
			sprintf(globDataPtr->outputBuffer,"reTraceDown: flt=%p, dbl=%p, moreData=%s",reTraceDownDataPtr.flt,reTraceDownDataPtr.dbl,reTraceDownDataPtr.moreData ? "true" : "false");
			debugOutputToHistory(globDataPtr->outputBuffer);

			// See also Vernissage manual page 22f
			// triggerAxisBlockSize: number of points in the raw data array which were acquired at the same root axis position
			// firstBlockOffset: offset position where the traceDown data starts

			// *RawBrickletIndex: source index into the raw data vernissage
			// *DataIndex: destination index into the igor wave (the funny index tricks are needed because of the organization of the wave in the memory)

			// data layout of igor waves in memory (Igor XOP Manual p. 238)
			// - the wave is linear in the memory
			// - going along the arrray will first fill the first column from row 0 to end and then the second column and so on
	
			// COLUMNS
			for(i = 0; i < numPointsRootAxis; i++){
				// ROWS
				for(j=0; j < numPointsTriggerAxis; j++){

					// traceUp
					if(traceUpDataPtr.moreData){
						traceUpRawBrickletIndex			= i*triggerAxisBlockSize+ j;
						traceUpDataIndex				= i*numPointsTriggerAxis   + j;

						if(	traceUpDataIndex >= 0 &&
							traceUpDataIndex < waveSize &&
							traceUpRawBrickletIndex < rawBrickletSize &&
							traceUpRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[traceUpRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

								fillWave(traceUpDataPtr,traceUpDataIndex,scaledValue,isDoubleWaveType);
								setExtremaValue(traceUpExtrema,rawValue,scaledValue);
						}
						else{
							debugOutputToHistory("Index out of range in traceUp");

							sprintf(globDataPtr->outputBuffer,"traceUpDataIndex=%d,waveSize=%d",traceUpDataIndex,waveSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							sprintf(globDataPtr->outputBuffer,"traceUpRawBrickletIndex=%d,rawBrickletSize=%d",traceUpRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							traceUpDataPtr.moreData = false;
						}
					}

					// traceDown
					if(traceDownDataPtr.moreData){

						traceDownRawBrickletIndex	= firstBlockOffset + i*triggerAxisBlockSize + j;
						// compared to the traceUpDataPtr.dbl the index i is shifted
						// this takes into account that the data in the traceDown is aquired from the highest y value to the lowest y value
						traceDownDataIndex			= ( numPointsRootAxis -( i+1) ) * numPointsTriggerAxis   + j;

						if(	traceDownDataIndex >= 0 &&
							traceDownDataIndex < waveSize &&
							traceDownRawBrickletIndex < rawBrickletSize &&
							traceDownRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[traceDownRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

								fillWave(traceDownDataPtr,traceDownDataIndex,scaledValue,isDoubleWaveType);
								setExtremaValue(traceDownExtrema,rawValue,scaledValue);
						}
						else{
							debugOutputToHistory("Index out of range in traceDown");

							sprintf(globDataPtr->outputBuffer,"traceDownDataIndex=%d,waveSize=%d",traceDownDataIndex,waveSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							sprintf(globDataPtr->outputBuffer,"traceDownRawBrickletIndex=%d,rawBrickletSize=%d",traceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							traceDownDataPtr.moreData = false;
						}
					}

					// reTraceUp
					if(reTraceUpDataPtr.moreData){

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

								fillWave(reTraceUpDataPtr,reTraceUpDataIndex,scaledValue,isDoubleWaveType);
								setExtremaValue(reTraceUpExtrema,rawValue,scaledValue);
						}
						else{
							debugOutputToHistory("Index out of range in reTraceUp");

							sprintf(globDataPtr->outputBuffer,"reTraceUpDataIndex=%d,waveSize=%d",reTraceUpDataIndex,waveSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							sprintf(globDataPtr->outputBuffer,"reTraceUpRawBrickletIndex=%d,rawBrickletSize=%d",reTraceUpRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							reTraceUpDataPtr.moreData = false;
						}
					}

					// reTraceDown
					if(reTraceDownDataPtr.moreData){

						reTraceDownRawBrickletIndex		= firstBlockOffset + i*triggerAxisBlockSize + triggerAxisBlockSize - (j+1);
						reTraceDownDataIndex			= ( numPointsRootAxis -( i+1) ) * numPointsTriggerAxis   + j;

						if(	reTraceDownDataIndex >= 0 &&
							reTraceDownDataIndex < waveSize &&
							reTraceDownRawBrickletIndex < rawBrickletSize &&
							reTraceDownRawBrickletIndex >= 0
							){

								rawValue	= rawBrickletDataPtr[reTraceDownRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

								fillWave(reTraceDownDataPtr,reTraceDownDataIndex,scaledValue,isDoubleWaveType);
								setExtremaValue(reTraceDownExtrema,rawValue,scaledValue);	
						}
						else{
							debugOutputToHistory("Index out of range in reTraceDown");

							sprintf(globDataPtr->outputBuffer,"reTraceDownDataIndex=%d,waveSize=%d",reTraceDownDataIndex,waveSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							sprintf(globDataPtr->outputBuffer,"reTraceDownRawBrickletIndex=%d,rawBrickletSize=%d",reTraceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							reTraceDownDataPtr.moreData = false;
						}
					}
				}
			}

			for(ii=0; ii < waveHandleVector.size() && ii < traceDirVector.size(); ii++){
				if( resampleData ){
					sprintf(globDataPtr->outputBuffer,"Resampling wave %s with pixelSize=%d",waveNameVector[ii].c_str(),pixelSize);
					debugOutputToHistory(globDataPtr->outputBuffer);

					// flag=3 results in the full path being returned including a trailing colon
					ret = GetDataFolderNameOrPath(dfHandle, 3, dataFolderPath);
					if(ret != 0){
						return ret;
					}
					// The "ImageInterpolate [...] Pixelate" command is used here
					sprintf(cmd,"ImageInterpolate/PXSZ={%d,%d}/DEST=%sM_PixelatedImage Pixelate %s",\
						pixelSize,pixelSize,dataFolderPath,dataFolderPath);
					// quote waveName properly, it might be a liberal name
					CatPossiblyQuotedName(cmd,waveNameVector[ii].c_str());
					if(globDataPtr->debuggingEnabled()){
						debugOutputToHistory(cmd);
					}
					ret = XOPSilentCommand(cmd);
					if(ret != 0){
						sprintf(globDataPtr->outputBuffer,"The command _%s_ failed to execute. So the XOP has to be fixed...",cmd);
						outputToHistory(globDataPtr->outputBuffer);
						continue;
					}

					// kill the un-interpolated wave and invalidate waveHandeVector[ii]
					ret = KillWave(waveHandleVector[ii]);
					waveHandleVector[ii] = NULL;
					if(ret != 0){
						return ret;
					}
					// rename wave from M_PixelatedImage to waveNameVector[ii] both sitting in dfHandle
					ret = RenameDataFolderObject(dfHandle,WAVE_OBJECT,"M_PixelatedImage",waveNameVector[ii].c_str());
					if(ret != 0){
						return ret;
					}
					waveHandleVector[ii] = FetchWaveFromDataFolder(dfHandle,waveNameVector[ii].c_str());
					ASSERT_RETURN_ONE(waveHandleVector[ii]);
					// get wave dimensions; needed for setScale below
					MDGetWaveDimensions(waveHandleVector[ii],&numDimensions,interpolatedDimSizes);
				}
		
				// set wave note and add info about resampling to the wave note
				setDataWaveNote(brickletID,traceDirVector[ii],extremaData[ii],waveHandleVector[ii],pixelSize);

				//  also the wave scaling changes if we have resampled the data
				if( resampleData ){
					physIncrement = triggerAxis.physicalIncrement*double(dimensionSizes[ROWS])/double(interpolatedDimSizes[ROWS]);
					MDSetWaveScaling(waveHandleVector[ii],ROWS,&physIncrement,&setScaleOffset);

					physIncrement = rootAxis.physicalIncrement*double(dimensionSizes[COLUMNS])/double(interpolatedDimSizes[COLUMNS]);
					MDSetWaveScaling(waveHandleVector[ii],COLUMNS,&physIncrement,&setScaleOffset);
				}
				else{// original image
					MDSetWaveScaling(waveHandleVector[ii],ROWS,&triggerAxis.physicalIncrement,&setScaleOffset);
					MDSetWaveScaling(waveHandleVector[ii],COLUMNS,&rootAxis.physicalIncrement,&setScaleOffset);
				}

				MDSetWaveUnits(waveHandleVector[ii],ROWS,WStringToString(triggerAxis.physicalUnit).c_str());
				MDSetWaveUnits(waveHandleVector[ii],COLUMNS,WStringToString(rootAxis.physicalUnit).c_str());
				MDSetWaveUnits(waveHandleVector[ii],-1,bricklet->getMetaDataValueAsString("channelUnit").c_str());
				fullPathOfCreatedWave.append(getFullWavePath(dfHandle,waveHandleVector[ii]));
				fullPathOfCreatedWave.append(";");
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

			double xAxisIncrement = xAxis.physicalIncrement;
			double yAxisIncrement = yAxis.physicalIncrement;
			
			// normally we have table sets with some step width >1, so the physicalIncrement has to be multiplied by this factor
			// Note: this approach assumes that all axis table sets have the same physical step width, this is at least the case for Matrix 2.2-1
			if( ySet.size() > 0 && xSet.size() > 0 ){
				xAxisIncrement *= xSet.begin()->step;
				yAxisIncrement *= ySet.begin()->step;
			}

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

			sprintf(globDataPtr->outputBuffer,"V Axis: total=%d",numPointsVAxis);
			debugOutputToHistory(globDataPtr->outputBuffer);

			sprintf(globDataPtr->outputBuffer,"X Axis # points with tableSet: Total=%d, Forward=%d, Backward=%d",
				numPointsXAxisWithTableBoth,numPointsXAxisWithTableFWD,numPointsXAxisWithTableBWD);
			debugOutputToHistory(globDataPtr->outputBuffer);

			sprintf(globDataPtr->outputBuffer,"Y Axis # points with tableSet: Total=%d, Up=%d, Down=%d",
				numPointsYAxisWithTableBoth,numPointsYAxisWithTableUp,numPointsYAxisWithTableDown);
			debugOutputToHistory(globDataPtr->outputBuffer);

			// FIXME Theoretical the sizes of the cubes could be different but we are igoring that for now
			if(numPointsXAxisWithTableBWD != 0 && numPointsXAxisWithTableFWD != 0 && numPointsXAxisWithTableFWD != numPointsXAxisWithTableBWD){
				sprintf(globDataPtr->outputBuffer,"BUG: Number of X axis points is different in forward and backward direction. Keep fingers crossed.");
				outputToHistory(globDataPtr->outputBuffer);
			}
			if(numPointsYAxisWithTableUp != 0 && numPointsYAxisWithTableDown != 0 && numPointsYAxisWithTableUp != numPointsYAxisWithTableDown){
				sprintf(globDataPtr->outputBuffer,"BUG: Number of Y axis points is different in up and down direction. Keep fingers crossed.");
				outputToHistory(globDataPtr->outputBuffer);
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

			sprintf(globDataPtr->outputBuffer,"dimensions of the cube: rows=%d,cols=%d,layers=%d",
				dimensionSizes[ROWS],dimensionSizes[COLUMNS],dimensionSizes[LAYERS]);
			debugOutputToHistory(globDataPtr->outputBuffer);

			// 4 cubes, TraceUp, TraceDown, ReTraceUp, ReTraceDown
			if(	numPointsXAxisWithTableFWD != 0 && numPointsXAxisWithTableBWD != 0 &&
				numPointsYAxisWithTableUp != 0 && numPointsYAxisWithTableUp != 0){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				traceDirVector.push_back(TRACE_UP);
				waveNameVector.push_back(waveBaseName + RE_TRACE_UP_STRING);
				traceDirVector.push_back(RE_TRACE_UP);
				waveNameVector.push_back(waveBaseName + TRACE_DOWN_STRING);
				traceDirVector.push_back(TRACE_DOWN);
				waveNameVector.push_back(waveBaseName + RE_TRACE_DOWN_STRING);
				traceDirVector.push_back(RE_TRACE_DOWN);
			}
			// 2 cubes, TraceUp, TraceDown
			else if(numPointsXAxisWithTableBWD == 0 && numPointsYAxisWithTableDown != 0){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				traceDirVector.push_back(TRACE_UP);
				waveNameVector.push_back(waveBaseName + TRACE_DOWN_STRING);			
				traceDirVector.push_back(TRACE_DOWN);
			}
			// 2 cubes, TraceUp, ReTraceUp
			else if(numPointsXAxisWithTableBWD != 0 && numPointsYAxisWithTableDown == 0){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				traceDirVector.push_back(TRACE_UP);
				waveNameVector.push_back(waveBaseName + RE_TRACE_UP_STRING);		
				traceDirVector.push_back(RE_TRACE_UP);
			}
			// 2 cubes, ReTraceUp, ReTraceDown
			else if(numPointsXAxisWithTableFWD == 0 && numPointsYAxisWithTableDown != 0){
				waveNameVector.push_back(waveBaseName + RE_TRACE_UP_STRING);
				traceDirVector.push_back(RE_TRACE_UP);
				waveNameVector.push_back(waveBaseName + RE_TRACE_DOWN_STRING);	
				traceDirVector.push_back(RE_TRACE_DOWN);
			}
			// 1 cube, TraceUp
			else if(numPointsXAxisWithTableBWD == 0 && numPointsYAxisWithTableDown == 0){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);			
				traceDirVector.push_back(TRACE_UP);
			}
			// 1 cube, ReTraceUp
			else if(numPointsXAxisWithTableFWD == 0 && numPointsYAxisWithTableDown == 0){
				waveNameVector.push_back(waveBaseName + RE_TRACE_UP_STRING);			
				traceDirVector.push_back(RE_TRACE_UP);
			}
			// not possible
			else{
				outputToHistory("BUG: Error in determining the number of cubes.");
				return INTERNAL_ERROR_CONVERTING_DATA;
			}

			// create waves
			for(itWaveNames = waveNameVector.begin(); itWaveNames != waveNameVector.end(); itWaveNames++){

				ret = MDMakeWave(&waveHandle, itWaveNames->c_str(),dfHandle,dimensionSizes,globDataPtr->getIgorWaveType(),globDataPtr->overwriteEnabledAsInt());

				if(ret == NAME_WAV_CONFLICT){
					sprintf(globDataPtr->outputBuffer,"Wave %s already exists.",itWaveNames->c_str());
					debugOutputToHistory(globDataPtr->outputBuffer);
					return WAVE_EXIST;
				}
				else if(ret != 0 ){
					sprintf(globDataPtr->outputBuffer,"Error %d in creating wave %s.",ret, itWaveNames->c_str());
					outputToHistory(globDataPtr->outputBuffer);
					return UNKNOWN_ERROR;
				}
				ASSERT_RETURN_ONE(waveHandle);
				waveHandleVector.push_back(waveHandle);
				clearWave(waveHandle,waveSize);
			}

			// set wave data pointers
			if(waveHandleVector.size() == 4){

				xAxisBlockSize	   = ( numPointsXAxisWithTableBWD + numPointsXAxisWithTableFWD ) * numPointsVAxis;

				setWaveDataPtr(traceUpDataPtr,waveHandleVector[0]);
				traceUpExtrema	   = &extremaData[0];
				setWaveDataPtr(reTraceUpDataPtr,waveHandleVector[1]);
				reTraceUpExtrema   = &extremaData[1];
				setWaveDataPtr(traceDownDataPtr,waveHandleVector[2]);
				traceDownExtrema   = &extremaData[2];
				setWaveDataPtr(reTraceDownDataPtr,waveHandleVector[3]);
				reTraceDownExtrema = &extremaData[3];
			}
			else if(waveHandleVector.size() == 2 && numPointsXAxisWithTableBWD == 0){

				xAxisBlockSize	   = numPointsXAxisWithTableFWD*numPointsVAxis;

				setWaveDataPtr(traceUpDataPtr,waveHandleVector[0]);
				traceUpExtrema	   = &extremaData[0];
				setWaveDataPtr(traceDownDataPtr,waveHandleVector[1]);		
				traceDownExtrema   = &extremaData[1];
			}
			else if(waveHandleVector.size() == 2 && numPointsXAxisWithTableFWD == 0){

				xAxisBlockSize	   = numPointsXAxisWithTableBWD*numPointsVAxis;

				setWaveDataPtr(reTraceUpDataPtr,waveHandleVector[0]);
				reTraceUpExtrema   = &extremaData[0];
				setWaveDataPtr(reTraceDownDataPtr,waveHandleVector[1]);		
				reTraceDownExtrema = &extremaData[1];
			}
			else if(waveHandleVector.size() == 2 && numPointsYAxisWithTableDown == 0){

				xAxisBlockSize	   = ( numPointsXAxisWithTableBWD + numPointsXAxisWithTableFWD ) * numPointsVAxis;

				setWaveDataPtr(traceUpDataPtr,waveHandleVector[0]);
				traceUpExtrema	   = &extremaData[0];
				setWaveDataPtr(reTraceUpDataPtr,waveHandleVector[1]);			
				reTraceUpExtrema   = &extremaData[1];
			}
			// no mirroring and FWD
			else if(waveHandleVector.size() == 1 && numPointsXAxisWithTableBWD == 0 && numPointsYAxisWithTableDown == 0){

				xAxisBlockSize	   = numPointsXAxisWithTableFWD*numPointsVAxis;

				setWaveDataPtr(traceUpDataPtr,waveHandleVector[0]);			
				traceUpExtrema	   = &extremaData[0];
			}
			// no mirroring and BWD
			else if(waveHandleVector.size() == 1 && numPointsXAxisWithTableFWD == 0 && numPointsYAxisWithTableDown == 0){

				xAxisBlockSize	   = numPointsXAxisWithTableBWD*numPointsVAxis;

				setWaveDataPtr(reTraceUpDataPtr,waveHandleVector[0]);		
				reTraceUpExtrema   = &extremaData[0];
			}
			else{
				outputToHistory("BUG: could not set wave pointers in createAndFillDataWave()...");
				return INTERNAL_ERROR_CONVERTING_DATA;
			}

			// data index to the start of the TraceDown data (this is the same for all combinations as xAxisBlockSize is set apropriately), in case traceDown does not exist this is no problem
			firstBlockOffset = numPointsYAxisWithTableUp*xAxisBlockSize;

			sprintf(globDataPtr->outputBuffer,"xAxisBlockSize=%d,firstBlockOffset=%d",xAxisBlockSize,firstBlockOffset);
			debugOutputToHistory(globDataPtr->outputBuffer);

			sprintf(globDataPtr->outputBuffer,"traceUp: flt=%p, dbl=%p, moreData=%s",traceUpDataPtr.flt,traceUpDataPtr.dbl,traceUpDataPtr.moreData ? "true" : "false");
			debugOutputToHistory(globDataPtr->outputBuffer);
			sprintf(globDataPtr->outputBuffer,"reTraceUp: flt=%p, dbl=%p, moreData=%s",reTraceUpDataPtr.flt,reTraceUpDataPtr.dbl,reTraceUpDataPtr.moreData ? "true" : "false");
			debugOutputToHistory(globDataPtr->outputBuffer);
			sprintf(globDataPtr->outputBuffer,"traceDown: flt=%p, dbl=%p, moreData=%s",traceDownDataPtr.flt,traceDownDataPtr.dbl,traceDownDataPtr.moreData ? "true" : "false");
			debugOutputToHistory(globDataPtr->outputBuffer);
			sprintf(globDataPtr->outputBuffer,"reTraceDown: flt=%p, dbl=%p, moreData=%s",reTraceDownDataPtr.flt,reTraceDownDataPtr.dbl,reTraceDownDataPtr.moreData ? "true" : "false");
			debugOutputToHistory(globDataPtr->outputBuffer);

			// COLUMNS
			for(i = 0; i < dimensionSizes[COLUMNS]; i++){
				// ROWS
				for(j=0; j < dimensionSizes[ROWS]; j++){
					// LAYERS
					for(k=0; k < dimensionSizes[LAYERS]; k++){

						// traceUp
						if(traceUpDataPtr.moreData){
							traceUpRawBrickletIndex	= i*xAxisBlockSize + j*dimensionSizes[LAYERS] + k;
							traceUpDataIndex		= i*dimensionSizes[ROWS] + j + k*dimensionSizes[ROWS]*dimensionSizes[COLUMNS];

							if(	traceUpDataIndex >= 0 &&
								traceUpDataIndex < waveSize &&
								traceUpRawBrickletIndex < rawBrickletSize &&
								traceUpRawBrickletIndex >= 0 ){

									rawValue	= rawBrickletDataPtr[traceUpRawBrickletIndex];
									scaledValue = rawValue*slope + yIntercept;

									fillWave(traceUpDataPtr,traceUpDataIndex,scaledValue,isDoubleWaveType);
									setExtremaValue(traceUpExtrema,rawValue,scaledValue);
							}
							else{
								debugOutputToHistory("Index out of range in traceUp");

								sprintf(globDataPtr->outputBuffer,"traceUpDataIndex=%d,waveSize=%d",traceUpDataIndex,waveSize);
								debugOutputToHistory(globDataPtr->outputBuffer);

								sprintf(globDataPtr->outputBuffer,"traceUpRawBrickletIndex=%d,rawBrickletSize=%d",traceUpRawBrickletIndex,rawBrickletSize);
								debugOutputToHistory(globDataPtr->outputBuffer);

								traceUpDataPtr.moreData = false;
							}
					}// if traceUpDataPtr

					// traceDown
					if(traceDownDataPtr.moreData){

							traceDownRawBrickletIndex = firstBlockOffset + i*xAxisBlockSize + j*dimensionSizes[LAYERS] + k;
							traceDownDataIndex		  = (dimensionSizes[COLUMNS] -(i+1))*dimensionSizes[ROWS] + j + k*dimensionSizes[ROWS]*dimensionSizes[COLUMNS];

						if(	traceDownDataIndex >= 0 &&
							traceDownDataIndex < waveSize &&
							traceDownRawBrickletIndex < rawBrickletSize &&
							traceDownRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[traceDownRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

								fillWave(traceDownDataPtr,traceDownDataIndex,scaledValue,isDoubleWaveType);
								setExtremaValue(traceDownExtrema,rawValue,scaledValue);
						}
						else{
							debugOutputToHistory("Index out of range in traceDown");

							sprintf(globDataPtr->outputBuffer,"traceDownDataIndex=%d,waveSize=%d",traceDownDataIndex,waveSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							sprintf(globDataPtr->outputBuffer,"traceDownRawBrickletIndex=%d,rawBrickletSize=%d",traceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							traceDownDataPtr.moreData = false;
						}
					}// if traceDownDataPtr

					// reTraceUp
					if(reTraceUpDataPtr.moreData){

						reTraceUpRawBrickletIndex = i*xAxisBlockSize + (dimensionSizes[ROWS] - (j+1))*dimensionSizes[LAYERS] + k;
						reTraceUpDataIndex		  = i*dimensionSizes[ROWS] + j + k*dimensionSizes[ROWS]*dimensionSizes[COLUMNS];

						if(	reTraceUpDataIndex >= 0 &&
							reTraceUpDataIndex < waveSize &&
							reTraceUpRawBrickletIndex < rawBrickletSize &&
							reTraceUpRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[reTraceUpRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

								fillWave(reTraceUpDataPtr,reTraceUpDataIndex,scaledValue,isDoubleWaveType);
								setExtremaValue(reTraceUpExtrema,rawValue,scaledValue);
						}
						else{
							debugOutputToHistory("Index out of range in reTraceUp");

							sprintf(globDataPtr->outputBuffer,"reTraceUpDataIndex=%d,waveSize=%d",reTraceUpDataIndex,waveSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							sprintf(globDataPtr->outputBuffer,"reTraceUpRawBrickletIndex=%d,rawBrickletSize=%d",reTraceUpRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							reTraceUpDataPtr.moreData = false;
						}
					}// if reTraceUpDataPtr

					// reTraceDown
					if(reTraceDownDataPtr.moreData){

						reTraceDownRawBrickletIndex	= firstBlockOffset + i*xAxisBlockSize + (dimensionSizes[ROWS] - (j+1))*dimensionSizes[LAYERS] + k;
						reTraceDownDataIndex		= ( dimensionSizes[COLUMNS] - (i+1) )*dimensionSizes[ROWS] + j + k*dimensionSizes[ROWS]*dimensionSizes[COLUMNS];

						if(	reTraceDownDataIndex >= 0 &&
							reTraceDownDataIndex < waveSize &&
							reTraceDownRawBrickletIndex < rawBrickletSize &&
							reTraceDownRawBrickletIndex >= 0
							){

								rawValue	= rawBrickletDataPtr[reTraceDownRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

								fillWave(reTraceDownDataPtr,reTraceDownDataIndex,scaledValue,isDoubleWaveType);
								setExtremaValue(reTraceDownExtrema,rawValue,scaledValue);

								//if(k < 10 && i < 2 && j < 2){
								//	sprintf(globDataPtr->outputBuffer,"j(rows)=%d,i(cols)=%d,k(layers)=%d,reTraceDownRawBrickletIndex=%d,reTraceDownDataIndex=%d,rawValue=%d,scaledValue=%g",
								//		j,i,k,reTraceDownRawBrickletIndex,reTraceDownDataIndex,rawValue,scaledValue);
								//	debugOutputToHistory(globDataPtr->outputBuffer);
								//}
						}
						else{
							debugOutputToHistory("Index out of range in reTraceDown");

							sprintf(globDataPtr->outputBuffer,"reTraceDownDataIndex=%d,waveSize=%d",reTraceDownDataIndex,waveSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							sprintf(globDataPtr->outputBuffer,"reTraceDownRawBrickletIndex=%d,rawBrickletSize=%d",reTraceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(globDataPtr->outputBuffer);

							reTraceDownDataPtr.moreData = false;
						}
					}// if reTraceDownDataPtr
				} // for LAYERS
			} // for ROWS
		} // for COLUMNS

			// set wave note and dimension units
			for(ii=0; ii < waveHandleVector.size() && ii < traceDirVector.size(); ii++){
				setDataWaveNote(brickletID,traceDirVector[ii],extremaData[ii],waveHandleVector[ii]);

				MDSetWaveScaling(waveHandleVector[ii],ROWS,&xAxisIncrement,&setScaleOffset);
				MDSetWaveScaling(waveHandleVector[ii],COLUMNS,&yAxisIncrement,&setScaleOffset);
				// here we don't use setScaleOffset=0 here
				MDSetWaveScaling(waveHandleVector[ii],LAYERS,&specAxis.physicalIncrement,&specAxis.physicalStart);

				MDSetWaveUnits(waveHandleVector[ii],ROWS,WStringToString(xAxis.physicalUnit).c_str());
				MDSetWaveUnits(waveHandleVector[ii],COLUMNS,WStringToString(yAxis.physicalUnit).c_str());
				MDSetWaveUnits(waveHandleVector[ii],LAYERS,WStringToString(specAxis.physicalUnit).c_str());
				MDSetWaveUnits(waveHandleVector[ii],-1,bricklet->getMetaDataValueAsString("channelUnit").c_str());

				fullPathOfCreatedWave.append(getFullWavePath(dfHandle,waveHandleVector[ii]));
				fullPathOfCreatedWave.append(";");
			}

			break;
	}
	return SUCCESS;
}