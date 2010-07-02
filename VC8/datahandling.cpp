
#include "datahandling.h"

#include <string>
#include <sstream>
#include <vector>
#include "float.h"

#include "xopstandardheaders.h"

#include "globals.h"
#include "utils.h"

#define DEBUG_LEVEL 1

// TODO
// support axis table sets
// support 3D data (gridded spectroscopy)

int createAndFillDataWave(DataFolderHandle dataFolderHandle, const char *waveBaseNameChar, int brickletID){

	char buf[ARRAY_SIZE];

	const int noOverwrite=0;

	int dimension;
	std::vector<Vernissage::Session::ViewTypeCode> viewTypeCodes;
	Vernissage::Session *pSession;
	std::vector<std::string> allAxes;
	Vernissage::Session::AxisDescriptor triggerAxis, rootAxis;
	int numPointsTriggerAxis=-1, numPointsRootAxis=-1, ret=-1, i, j;
	std::vector<waveHndl> waveHandleVector;
	std::vector<int> hStateVector;
	waveHndl waveHandle;
	std::vector<std::string> waveNameVector;

	int *rawBrickletDataPtr = NULL;
	int hState, rawBrickletSize=0, waveSize=0, firstBlockOffset=0, triggerAxisBlockSize=0, rootAxisBlockSize=0; 
	int traceUpRawBrickletIndex, traceUpDataIndex,reTraceUpDataIndex,reTraceUpRawBrickletIndex, traceDownRawBrickletIndex,traceDownDataIndex, reTraceDownRawBrickletIndex,reTraceDownDataIndex;

	double *traceUpDataPtr = NULL;
	double *reTraceUpDataPtr = NULL;
	double *traceDownDataPtr = NULL;
	double *reTraceDownDataPtr = NULL;
	double *waveData = NULL;

	double setScaleOffset=0.0;

	std::vector<int>	rawMax(4),	rawMin(4);
	std::vector<double> scaledMax(4), scaledMin(4);

	int rawValue, rawMaxValue, rawMinValue;
	double scaledMinValue, scaledMaxValue, scaledValue;

	std::vector<std::string>::const_iterator itWaveNames;

	long dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));

	std::string waveBaseName(waveBaseNameChar);

	ASSERT_RETURN_MINUSONE(pMyData);
	MyBricklet *myBricklet = pMyData->getBrickletClassFromMap(brickletID);

	ASSERT_RETURN_MINUSONE(myBricklet);
	void *pBricklet = myBricklet->getBrickletPointer();

	ASSERT_RETURN_MINUSONE(pBricklet);
	pSession = pMyData->getVernissageSession();

	ASSERT_RETURN_MINUSONE(pSession);

	dimension = myBricklet->getMetaDataValueAsInt("dimension");
	myBricklet->getAxes(allAxes);
	myBricklet->getViewTypeCodes(viewTypeCodes);

	sprintf(buf,"### BrickletID %d ###",brickletID);
	debugOutputToHistory(DEBUG_LEVEL,buf);

	sprintf(buf,"dimension %d",dimension);
	debugOutputToHistory(DEBUG_LEVEL,buf);

	std::vector<Vernissage::Session::ViewTypeCode>::const_iterator itViewTypeCodes;
	for(itViewTypeCodes = viewTypeCodes.begin(); itViewTypeCodes != viewTypeCodes.end(); itViewTypeCodes++){
		sprintf(buf,"viewType %s",viewTypeCodeToString(*itViewTypeCodes).c_str());
		debugOutputToHistory(DEBUG_LEVEL,buf);
	}
	
	debugOutputToHistory(DEBUG_LEVEL,"Axis order is from triggerAxis to rootAxis");

	std::vector<std::string>::const_iterator itAllAxes;	
	for(itAllAxes = allAxes.begin(); itAllAxes != allAxes.end(); itAllAxes++){
		sprintf(buf,"Axis %s",itAllAxes->c_str());
		debugOutputToHistory(DEBUG_LEVEL,buf);
	}

	// set min and max values to safe values
	for(i=0; i < rawMin.size(); i++){
		rawMin[i]	= _I32_MAX;
		rawMax[i]	= _I32_MIN;
		scaledMin[i]= DBL_MAX;
		scaledMax[i]= -DBL_MAX;
	}
	rawMinValue = _I32_MAX;
	rawMaxValue = _I32_MIN;
	scaledMinValue = DBL_MAX;
	scaledMaxValue = -DBL_MAX;

	// get pointer to raw data
	const int* pBuffer;
	myBricklet->getBrickletContentsBuffer(&pBuffer,rawBrickletSize);
	rawBrickletDataPtr = (int *) pBuffer;

	switch(dimension){
	
		case 1:

			triggerAxis = pSession->getAxisDescriptor(pBricklet,pSession->getTriggerAxisName(pBricklet));
			
			numPointsTriggerAxis = triggerAxis.clocks;
			
			if (triggerAxis.mirrored)
			{
				numPointsTriggerAxis /= 2;
			}
			
			dimensionSizes[ROWS] = numPointsTriggerAxis;
			ret = MDMakeWave(&waveHandle, waveBaseName.c_str(),dataFolderHandle,dimensionSizes,NT_FP64,noOverwrite);

			if(ret == NAME_WAV_CONFLICT){
				sprintf(buf,"Wave %s already exists.",waveBaseName.c_str());
				debugOutputToHistory(DEBUG_LEVEL,buf);
				return WAVE_EXIST;
			}

			if(ret != 0 ){
				sprintf(buf,"Error %d in creating wave %s.",ret, waveBaseName.c_str());
				outputToHistory(buf);
				return UNKNOWN_ERROR;
			}

			ASSERT_RETURN_MINUSONE(waveHandle);

			hState = MoveLockHandle(waveHandle);

			waveData = (double*) WaveData(waveHandle);

			for(i=0; i < numPointsTriggerAxis; i++){
				
				rawValue	= rawBrickletDataPtr[i];
				scaledValue = pSession->toPhysical(rawValue,pBricklet);			
				waveData[i]	= scaledValue;

				if(rawValue < rawMinValue){
					rawMinValue = rawValue;
				}
				if(rawValue > rawMaxValue){
					rawMaxValue = rawValue;
				}
				if(scaledValue < scaledMinValue){
					scaledMinValue = scaledValue;
				}
				if(scaledValue > scaledMaxValue){
					scaledMaxValue = scaledValue;
				}
			
			}

			setDataWaveNote(brickletID,rawMinValue,rawMaxValue,scaledMinValue,scaledMaxValue,waveHandle);

			MDSetWaveScaling(waveHandle,ROWS,&triggerAxis.physicalIncrement,&triggerAxis.physicalStart);
			
			// FIXME casting should not be necessary
			MDSetWaveUnits(waveHandle,ROWS,(char *)WStringToString(triggerAxis.physicalUnit).c_str());
			MDSetWaveUnits(waveHandle,-1,(char *)myBricklet->getMetaDataValueAsString("channelUnit").c_str());			
	
			HSetState(waveHandle,hState);
		
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

			sprintf(buf,"numPointsRootAxis=%d",numPointsRootAxis);
			debugOutputToHistory(DEBUG_LEVEL,buf);

			sprintf(buf,"numPointsTriggerAxis=%d",numPointsTriggerAxis);
			debugOutputToHistory(DEBUG_LEVEL,buf);

			dimensionSizes[ROWS] = numPointsTriggerAxis;
			dimensionSizes[COLUMNS] = numPointsRootAxis;
			waveSize = dimensionSizes[ROWS]*dimensionSizes[COLUMNS];

			// now we have to disinguish three cases on how many 2D waves we need
			// both are mirrored:		4
			// one mirrored, one not:	2
			// none mirrored:			1
			if( triggerAxis.mirrored && rootAxis.mirrored ){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				waveNameVector.push_back(waveBaseName + RE_TRACE_UP_STRING);
				waveNameVector.push_back(waveBaseName + TRACE_DOWN_STRING);
				waveNameVector.push_back(waveBaseName + RE_TRACE_DOWN_STRING);
			}
			else if( triggerAxis.mirrored ){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				waveNameVector.push_back(waveBaseName + RE_TRACE_UP_STRING);
			}
			else if( rootAxis.mirrored ){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				waveNameVector.push_back(waveBaseName + TRACE_DOWN_STRING);
			}
			else{
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
			}

			for(itWaveNames = waveNameVector.begin(); itWaveNames != waveNameVector.end(); itWaveNames++){

				ret = MDMakeWave(&waveHandle, itWaveNames->c_str(),dataFolderHandle,dimensionSizes,NT_FP64,noOverwrite);

				if(ret == NAME_WAV_CONFLICT){
					sprintf(buf,"Wave %s already exists.",itWaveNames->c_str());
					debugOutputToHistory(DEBUG_LEVEL,buf);
					return WAVE_EXIST;
				}

				if(ret != 0 ){
					sprintf(buf,"Error %d in creating wave %s.",ret, itWaveNames->c_str());
					outputToHistory(buf);
					return UNKNOWN_ERROR;
				}

				ASSERT_RETURN_MINUSONE(waveHandle);

				waveHandleVector.push_back(waveHandle);

				// lock wave and store state
				hStateVector.push_back(MoveLockHandle(waveHandle));
				// clear wave
				MemClear((double*) WaveData(waveHandle), numPointsTriggerAxis*numPointsRootAxis*sizeof(double));
			}

			//// TraceUp aka Forward/Up
			//// ReTraceUp aka Backward/Up
			//// TraceDown aka Forward/Down
			//// ReTraceDown aka Backward/Down
			//// horizontal axis aka X axis in Pascal's Scala Routines aka triggerAxis 		aka 	ROWS
			//// vertical   axis aka Y axis in Pascal's Scala Routines aka rootAxis 		aka		COLUMNS

			// data layout of igor waves in memory (Igor XOP Manual p. 238)
			// - the wave is linear in the memory
			// - going along the arrray will first fill the first column from row 0 to end and then the second column and so on


			// both axes are mirrored
			if(waveHandleVector.size() == 4){

				rootAxisBlockSize	 = 2*numPointsRootAxis;
				triggerAxisBlockSize = 2*numPointsTriggerAxis;
				firstBlockOffset	 = numPointsRootAxis * triggerAxisBlockSize;

				traceUpDataPtr    = (double*) WaveData(waveHandleVector[0]);
				reTraceUpDataPtr  = (double*) WaveData(waveHandleVector[1]);
				traceDownDataPtr  = (double*) WaveData(waveHandleVector[2]);
				reTraceDownDataPtr  = (double*) WaveData(waveHandleVector[3]);
			}
			// only triggerAxis (X) is mirrored
			else if(waveHandleVector.size() == 2 && triggerAxis.mirrored){

				rootAxisBlockSize	 = numPointsRootAxis;
				triggerAxisBlockSize = 2*numPointsTriggerAxis;
				firstBlockOffset	 = numPointsRootAxis * triggerAxisBlockSize;

				traceUpDataPtr    = (double*) WaveData(waveHandleVector[0]);
				reTraceUpDataPtr  = (double*) WaveData(waveHandleVector[1]);			
			}
			// only rootAxis (Y) is mirrored
			else if(waveHandleVector.size() == 2 && rootAxis.mirrored){

				rootAxisBlockSize	 = 2*numPointsRootAxis;
				triggerAxisBlockSize = numPointsTriggerAxis;
				firstBlockOffset	 = numPointsRootAxis * triggerAxisBlockSize;

				traceUpDataPtr    = (double*) WaveData(waveHandleVector[0]);
				traceDownDataPtr  = (double*) WaveData(waveHandleVector[1]);			
			}
			// no mirroring
			else if(waveHandleVector.size() == 1 && rootAxis.mirrored == false && triggerAxis.mirrored == false){

				rootAxisBlockSize	 = numPointsRootAxis;
				triggerAxisBlockSize = numPointsTriggerAxis;
				firstBlockOffset	 = numPointsRootAxis * triggerAxisBlockSize;

				traceUpDataPtr    = (double*) WaveData(waveHandleVector[0]);			
			}
			else{
				outputToHistory("BUG: createAndFillDataWave()...");
				return 1;
			}

			// TODO explain the messy indizes here and above

			// COLUMNS
			for(i = 0; i < numPointsRootAxis; i++){

				// ROWS
				for(j=0; j < numPointsTriggerAxis; j++){

					// traceUp
					if(traceUpDataPtr){
						traceUpRawBrickletIndex			= i*triggerAxisBlockSize+ j;
						traceUpDataIndex				= i*numPointsTriggerAxis   + j;

						if(	traceUpDataIndex >= 0 &&
							traceUpDataIndex < waveSize &&
							traceUpRawBrickletIndex < rawBrickletSize &&
							traceUpRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[traceUpRawBrickletIndex];
								scaledValue = pSession->toPhysical(rawValue,pBricklet);
								traceUpDataPtr[traceUpDataIndex] =  scaledValue;

								if(rawValue < rawMin[0]){
									rawMin[0] = rawValue;
								}
								if(rawValue > rawMax[0]){
									rawMax[0] = rawValue;
								}
								if(scaledValue < scaledMin[0]){
									scaledMin[0] = scaledValue;
								}
								if(scaledValue > scaledMax[0]){
									scaledMax[0] = scaledValue;
								}
						}
						else{
							debugOutputToHistory(DEBUG_LEVEL,"Index out of range in traceUp");

							sprintf(buf,"traceUpDataIndex=%d,waveSize=%d",traceUpDataIndex,waveSize);
							debugOutputToHistory(DEBUG_LEVEL,buf);

							sprintf(buf,"traceUpRawBrickletIndex=%d,rawBrickletSize=%d",traceUpRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(DEBUG_LEVEL,buf);

							traceUpDataPtr=NULL;
						}
					}

					// traceDown
					if(traceDownDataPtr){

						traceDownRawBrickletIndex	= firstBlockOffset + i*triggerAxisBlockSize + j;
						traceDownDataIndex			= ( (numPointsRootAxis-1) - i) * numPointsTriggerAxis   + j;

						if(	traceDownDataIndex >= 0 &&
							traceDownDataIndex < waveSize &&
							traceDownRawBrickletIndex < rawBrickletSize &&
							traceDownRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[traceDownRawBrickletIndex];
								scaledValue = pSession->toPhysical(rawValue,pBricklet);
								traceDownDataPtr[traceDownDataIndex] =  scaledValue;

								if(rawValue < rawMin[1]){
									rawMin[1] = rawValue;
								}
								if(rawValue > rawMax[1]){
									rawMax[1] = rawValue;
								}
								if(scaledValue < scaledMin[1]){
									scaledMin[1] = scaledValue;
								}
								if(scaledValue > scaledMax[1]){
									scaledMax[1] = scaledValue;
								}
						}
						else{
							outputToHistory("Index out of range in traceDown");

							sprintf(buf,"traceDownDataIndex=%d,waveSize=%d",traceDownDataIndex,waveSize);
							debugOutputToHistory(DEBUG_LEVEL,buf);

							sprintf(buf,"traceDownRawBrickletIndex=%d,rawBrickletSize=%d",traceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(DEBUG_LEVEL,buf);

							traceDownDataPtr=NULL;
						}
					}

					// reTraceUp
					if(reTraceUpDataPtr){

						reTraceUpRawBrickletIndex	= i*triggerAxisBlockSize + triggerAxisBlockSize - (j+1);
						reTraceUpDataIndex			= i *  numPointsTriggerAxis + j;

						if(	reTraceUpDataIndex >= 0 &&
							reTraceUpDataIndex < waveSize &&
							reTraceUpRawBrickletIndex < rawBrickletSize &&
							reTraceUpRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[reTraceUpRawBrickletIndex];
								scaledValue = pSession->toPhysical(rawValue,pBricklet);
								reTraceUpDataPtr[reTraceUpDataIndex] =  scaledValue;

								if(rawValue < rawMin[2]){
									rawMin[2] = rawValue;
								}
								if(rawValue > rawMax[2]){
									rawMax[2] = rawValue;
								}
								if(scaledValue < scaledMin[2]){
									scaledMin[2] = scaledValue;
								}
								if(scaledValue > scaledMax[2]){
									scaledMax[2] = scaledValue;
								}
						}
						else{
							outputToHistory("Index out of range in reTraceUp");

							sprintf(buf,"reTraceUpDataIndex=%d,waveSize=%d",reTraceUpDataIndex,waveSize);
							debugOutputToHistory(DEBUG_LEVEL,buf);

							sprintf(buf,"reTraceUpRawBrickletIndex=%d,rawBrickletSize=%d",reTraceUpRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(DEBUG_LEVEL,buf);

							reTraceUpDataPtr=NULL;
						}
					}

					// reTraceDown
					if(reTraceDownDataPtr){

						reTraceDownRawBrickletIndex		= firstBlockOffset + i*triggerAxisBlockSize + triggerAxisBlockSize - (j+1);
						reTraceDownDataIndex			= ( (numPointsRootAxis-1) - i) * numPointsTriggerAxis   + j;

						if(	reTraceDownDataIndex >= 0 &&
							reTraceDownDataIndex < waveSize &&
							reTraceDownRawBrickletIndex < rawBrickletSize &&
							reTraceDownRawBrickletIndex >= 0
							){

								rawValue	= rawBrickletDataPtr[reTraceDownRawBrickletIndex];
								scaledValue = pSession->toPhysical(rawValue,pBricklet);
								reTraceDownDataPtr[reTraceDownDataIndex] =  scaledValue;

								if(rawValue < rawMin[3]){
									rawMin[3] = rawValue;
								}
								if(rawValue > rawMax[3]){
									rawMax[3] = rawValue;
								}
								if(scaledValue < scaledMin[3]){
									scaledMin[3] = scaledValue;
								}
								if(scaledValue > scaledMax[3]){
									scaledMax[3] = scaledValue;
								}
						}
						else{
							outputToHistory("Index out of range in reTraceDown");

							sprintf(buf,"reTraceDownDataIndex=%d,waveSize=%d",reTraceDownDataIndex,waveSize);
							debugOutputToHistory(DEBUG_LEVEL,buf);

							sprintf(buf,"reTraceDownRawBrickletIndex=%d,rawBrickletSize=%d",reTraceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(DEBUG_LEVEL,buf);

							reTraceDownDataPtr=NULL;
						}
					}
				}
			}

			// unlock waves and set wave note
			for(i=0; i < hStateVector.size(); i++){
				HSetState(waveHandleVector[i],hStateVector[i]);
				setDataWaveNote(brickletID,rawMin[i],rawMax[i],scaledMin[i],scaledMax[i],waveHandleVector[i]);

				MDSetWaveScaling(waveHandleVector[i],ROWS,&triggerAxis.physicalIncrement,&setScaleOffset);
				MDSetWaveScaling(waveHandleVector[i],COLUMNS,&rootAxis.physicalIncrement,&setScaleOffset);

				// FIXME casting should not be necessary
				MDSetWaveUnits(waveHandleVector[i],ROWS,(char *)WStringToString(triggerAxis.physicalUnit).c_str());
				MDSetWaveUnits(waveHandleVector[i],COLUMNS,(char *)WStringToString(rootAxis.physicalUnit).c_str());
				MDSetWaveUnits(waveHandleVector[i],-1,(char *)myBricklet->getMetaDataValueAsString("channelUnit").c_str());
			}

			break;

		case 3:
			outputToHistory("TODO and FIXME: Not yet implemented, feel free to help :)");

			break;

		default:
			sprintf(buf,"BUG: Dimension %d can not be handled",dimension);
			outputToHistory(buf);
			break;	
	}


	return 0;
}
