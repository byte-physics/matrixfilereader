
#include "datahandling.h"

#include <string>
#include <sstream>
#include <vector>
#include "float.h"

#include "xopstandardheaders.h"

#include "globals.h"
#include "utils.h"

// TODO
// check support for 3D data (gridded spectroscopy)

int createAndFillDataWave(DataFolderHandle dataFolderHandle, const char *waveBaseNameChar, int brickletID){

	char buf[ARRAY_SIZE];

	const int noOverwrite=0;

	int dimension;
	std::vector<Vernissage::Session::ViewTypeCode> viewTypeCodes;
	Vernissage::Session *pSession;
	std::vector<std::string> allAxes;
	Vernissage::Session::AxisDescriptor triggerAxis, rootAxis;
	int numPointsTriggerAxis=-1, numPointsRootAxis=-1, ret=-1, i, j,k;
	std::vector<waveHndl> waveHandleVector;
	std::vector<int> hStateVector;
	waveHndl waveHandle;
	std::vector<std::string> waveNameVector;

	int *rawBrickletDataPtr = NULL;
	int hState, rawBrickletSize=0, waveSize=0, firstBlockOffset=0, triggerAxisBlockSize=0;

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

	MyBricklet *myBricklet = pMyData->getMyBrickletObject(brickletID);

	ASSERT_RETURN_MINUSONE(myBricklet);
	void *pBricklet = myBricklet->getBrickletPointer();

	ASSERT_RETURN_MINUSONE(pBricklet);
	pSession = pMyData->getVernissageSession();

	ASSERT_RETURN_MINUSONE(pSession);

	dimension = myBricklet->getMetaDataValueAsInt("dimension");
	myBricklet->getAxes(allAxes);
	myBricklet->getViewTypeCodes(viewTypeCodes);

	sprintf(buf,"### BrickletID %d ###",brickletID);
	debugOutputToHistory(buf);

	sprintf(buf,"dimension %d",dimension);
	debugOutputToHistory(buf);

	std::vector<Vernissage::Session::ViewTypeCode>::const_iterator itViewTypeCodes;
	for(itViewTypeCodes = viewTypeCodes.begin(); itViewTypeCodes != viewTypeCodes.end(); itViewTypeCodes++){
		sprintf(buf,"viewType %s",viewTypeCodeToString(*itViewTypeCodes).c_str());
		debugOutputToHistory(buf);
	}
	
	debugOutputToHistory("Axis order is from triggerAxis to rootAxis");

	std::vector<std::string>::const_iterator itAllAxes;	
	for(itAllAxes = allAxes.begin(); itAllAxes != allAxes.end(); itAllAxes++){
		sprintf(buf,"Axis %s",itAllAxes->c_str());
		debugOutputToHistory(buf);
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

	// create data for raw->scaled transformation
	// the min and max values here are for the complete bricklet data and not only for one wave
	int xOne, xTwo;
	double yOne, yTwo, slope, yIntercept;
	
	xOne = myBricklet->getRawMin();
	xTwo = myBricklet->getRawMax();
	yOne = myBricklet->getScaledMin();
	yTwo = myBricklet->getScaledMax();

	slope = (yOne - yTwo) / (xOne*1.0 - xTwo*1.0);
	yIntercept = yOne - slope*xOne;

	sprintf(buf,"raw->scaled transformation: xOne=%d,xTwo=%d,yOne=%g,yTwo=%g",xOne,xTwo,yOne,yTwo);
	debugOutputToHistory(buf);

	sprintf(buf,"raw->scaled transformation: slope=%g,yIntercept=%g",slope,yIntercept);
	debugOutputToHistory(buf);

	if( dimension < 1 || dimension > 3 ){
		sprintf(buf,"Dimension %d can not be handled. Please file a bug report and attach the measured data.",dimension);
		outputToHistory(buf);
		return INTERNAL_ERROR_CONVERTING_DATA;
	}

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
				debugOutputToHistory(buf);
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
				scaledValue = rawValue*slope + yIntercept;

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
			
			MDSetWaveUnits(waveHandle,ROWS,const_cast<char *>(WStringToString(triggerAxis.physicalUnit).c_str()));
			MDSetWaveUnits(waveHandle,-1,const_cast<char *>(myBricklet->getMetaDataValueAsString("channelUnit").c_str()));			
	
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
			debugOutputToHistory(buf);

			sprintf(buf,"numPointsTriggerAxis=%d",numPointsTriggerAxis);
			debugOutputToHistory(buf);

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
					debugOutputToHistory(buf);
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
				MemClear((double*) WaveData(waveHandle), waveSize*sizeof(double));
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

				traceUpDataPtr    = (double*) WaveData(waveHandleVector[0]);
				reTraceUpDataPtr  = (double*) WaveData(waveHandleVector[1]);
				traceDownDataPtr  = (double*) WaveData(waveHandleVector[2]);
				reTraceDownDataPtr  = (double*) WaveData(waveHandleVector[3]);
			}
			// only triggerAxis (X) is mirrored
			else if(waveHandleVector.size() == 2 && triggerAxis.mirrored){

				triggerAxisBlockSize = 2*numPointsTriggerAxis;

				traceUpDataPtr    = (double*) WaveData(waveHandleVector[0]);
				reTraceUpDataPtr  = (double*) WaveData(waveHandleVector[1]);			
			}
			// only rootAxis (Y) is mirrored
			else if(waveHandleVector.size() == 2 && rootAxis.mirrored){

				triggerAxisBlockSize = numPointsTriggerAxis;
				
				traceUpDataPtr    = (double*) WaveData(waveHandleVector[0]);
				traceDownDataPtr  = (double*) WaveData(waveHandleVector[1]);			
			}
			// no mirroring
			else if(waveHandleVector.size() == 1 && rootAxis.mirrored == false && triggerAxis.mirrored == false){

				triggerAxisBlockSize = numPointsTriggerAxis;

				traceUpDataPtr    = (double*) WaveData(waveHandleVector[0]);			
			}
			else{
				outputToHistory("BUG: createAndFillDataWave()...");
				return 1;
			}

			firstBlockOffset	 = numPointsRootAxis * triggerAxisBlockSize;

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
					if(traceUpDataPtr){
						traceUpRawBrickletIndex			= i*triggerAxisBlockSize+ j;
						traceUpDataIndex				= i*numPointsTriggerAxis   + j;

						if(	traceUpDataIndex >= 0 &&
							traceUpDataIndex < waveSize &&
							traceUpRawBrickletIndex < rawBrickletSize &&
							traceUpRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[traceUpRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

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
							debugOutputToHistory("Index out of range in traceUp");

							sprintf(buf,"traceUpDataIndex=%d,waveSize=%d",traceUpDataIndex,waveSize);
							debugOutputToHistory(buf);

							sprintf(buf,"traceUpRawBrickletIndex=%d,rawBrickletSize=%d",traceUpRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(buf);

							traceUpDataPtr=NULL;
						}
					}

					// traceDown
					if(traceDownDataPtr){

						traceDownRawBrickletIndex	= firstBlockOffset + i*triggerAxisBlockSize + j;
						// compared to the traceUpDataPtr the index i is shifted
						// this takes into account that the data in the traceDown is aquired from the highest y value to the lowest y value
						traceDownDataIndex			= ( numPointsRootAxis -( i+1) ) * numPointsTriggerAxis   + j;

						if(	traceDownDataIndex >= 0 &&
							traceDownDataIndex < waveSize &&
							traceDownRawBrickletIndex < rawBrickletSize &&
							traceDownRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[traceDownRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

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
							debugOutputToHistory("Index out of range in traceDown");

							sprintf(buf,"traceDownDataIndex=%d,waveSize=%d",traceDownDataIndex,waveSize);
							debugOutputToHistory(buf);

							sprintf(buf,"traceDownRawBrickletIndex=%d,rawBrickletSize=%d",traceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(buf);

							traceDownDataPtr=NULL;
						}
					}

					// reTraceUp
					if(reTraceUpDataPtr){

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
							debugOutputToHistory("Index out of range in reTraceUp");

							sprintf(buf,"reTraceUpDataIndex=%d,waveSize=%d",reTraceUpDataIndex,waveSize);
							debugOutputToHistory(buf);

							sprintf(buf,"reTraceUpRawBrickletIndex=%d,rawBrickletSize=%d",reTraceUpRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(buf);

							reTraceUpDataPtr=NULL;
						}
					}

					// reTraceDown
					if(reTraceDownDataPtr){

						reTraceDownRawBrickletIndex		= firstBlockOffset + i*triggerAxisBlockSize + triggerAxisBlockSize - (j+1);
						reTraceDownDataIndex			= ( numPointsRootAxis -( i+1) ) * numPointsTriggerAxis   + j;

						if(	reTraceDownDataIndex >= 0 &&
							reTraceDownDataIndex < waveSize &&
							reTraceDownRawBrickletIndex < rawBrickletSize &&
							reTraceDownRawBrickletIndex >= 0
							){

								rawValue	= rawBrickletDataPtr[reTraceDownRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

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
							debugOutputToHistory("Index out of range in reTraceDown");

							sprintf(buf,"reTraceDownDataIndex=%d,waveSize=%d",reTraceDownDataIndex,waveSize);
							debugOutputToHistory(buf);

							sprintf(buf,"reTraceDownRawBrickletIndex=%d,rawBrickletSize=%d",reTraceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(buf);

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

				MDSetWaveUnits(waveHandleVector[i],ROWS,const_cast<char *>((WStringToString(triggerAxis.physicalUnit).c_str())));
				MDSetWaveUnits(waveHandleVector[i],COLUMNS,const_cast<char *>(WStringToString(rootAxis.physicalUnit).c_str()));
				MDSetWaveUnits(waveHandleVector[i],-1,const_cast<char *>(myBricklet->getMetaDataValueAsString("channelUnit").c_str()));
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
				outputToHistory("The 3D data is not of the type vtc_2Dof3D and vtc_Spectroscopy. So I don't know how to handle it.");
				return INTERNAL_ERROR_CONVERTING_DATA;
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
			int numPointsYAxis = xAxis.clocks;
			
			if(yAxis.mirrored){
				numPointsYAxis /= 2;
			}

			// Determine how much space the data occupies in X and Y direction
			// For that we have to take into account the table sets
			// Then we also now how many 3D cubes we have, we can have 1,2 or 4. the same as in the 2D case

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

			bool forward = true;

			xIt = xSet.begin();
			tablePosX = xIt->start;
	
			while (xIt != xSet.end())
			{
				numPointsXAxisWithTableBoth++;
				tablePosX += xIt->step;

				if (forward){
					numPointsXAxisWithTableFWD++;
				}
				else{
					numPointsXAxisWithTableBWD++;
				}
				if (tablePosX > xIt->stop){
					++xIt;

					if(xIt != xSet.end()){
						tablePosX = xIt->start;
					}
				}
				forward = (tablePosX <= numPointsXAxis);
			}

			// Determine the number of different y values
			
			// number of y axis points with taking the axis table sets into account
			int numPointsYAxisWithTableBoth     = 0;
			// the part of numPointsYAxisWithTableBoth which is used in traceUp
			int numPointsYAxisWithTableUp   = 0;
			// the part of numPointsYAxisWithTableBoth which is used in traceDown
			int numPointsYAxisWithTableDown = 0;
			bool up = true;

			yIt = ySet.begin();
			tablePosY = yIt->start;
	
			while (yIt != ySet.end())
			{
				numPointsYAxisWithTableBoth++;
				tablePosY += yIt->step;

				if (up){
					numPointsYAxisWithTableUp++;
				}
				else{
					numPointsYAxisWithTableDown++;
				}

				if (tablePosY > yIt->stop){
					++yIt;

					if(yIt != ySet.end()){
						tablePosY = yIt->start;
					}
				}
				up = (tablePosY <= numPointsYAxis);
			}

			// END Borrowed from SCALA exporter plugin

			sprintf(buf,"V Axis: total=%d",numPointsVAxis);
			debugOutputToHistory(buf);

			sprintf(buf,"X Axis # points with tableSet: Total=%d, Forward=%d, Backward=%d",
				numPointsXAxisWithTableBoth,numPointsXAxisWithTableFWD,numPointsXAxisWithTableBWD);
			debugOutputToHistory(buf);

			sprintf(buf,"Y Axis # points with tableSet: Total=%d, Up=%d, Down=%d",
				numPointsYAxisWithTableBoth,numPointsYAxisWithTableUp,numPointsYAxisWithTableDown);
			debugOutputToHistory(buf);

			// FIXME Theoretical the sizes of the cubes could be different but we are igoring that for now
			if(numPointsXAxisWithTableBWD != 0 && numPointsXAxisWithTableFWD != numPointsXAxisWithTableBWD){
				sprintf(buf,"BUG: Number of x axis points is different in forward and backward direction. Keep fingers crossed.");
				outputToHistory(buf);
			}
			if(numPointsYAxisWithTableUp != 0 && numPointsYAxisWithTableUp != numPointsYAxisWithTableDown){
				sprintf(buf,"BUG: Number of y axis points is different in up and down direction. Keep fingers crossed.");
				outputToHistory(buf);
			}

			// dimensions of the cube
			dimensionSizes[ROWS]	= numPointsXAxisWithTableFWD;
			dimensionSizes[COLUMNS] = numPointsYAxisWithTableUp;
			dimensionSizes[LAYERS]  = numPointsVAxis;

			waveSize = dimensionSizes[ROWS]*dimensionSizes[COLUMNS]*dimensionSizes[LAYERS];

			sprintf(buf,"dimensions of the cube: rows=%d,cols=%d,layers=%d",
				dimensionSizes[ROWS],dimensionSizes[COLUMNS],dimensionSizes[LAYERS]);
			debugOutputToHistory(buf);

			// 4 cubes, TraceUp, TraceDown, ReTraceUp, ReTraceDown
			if(	numPointsXAxisWithTableFWD != 0 && numPointsXAxisWithTableBWD != 0 &&
				numPointsYAxisWithTableUp != 0 && numPointsYAxisWithTableUp != 0){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				waveNameVector.push_back(waveBaseName + RE_TRACE_UP_STRING);
				waveNameVector.push_back(waveBaseName + TRACE_DOWN_STRING);
				waveNameVector.push_back(waveBaseName + RE_TRACE_DOWN_STRING);
			}
			// 2 cubes, TraceUp, TraceDown
			else if(numPointsXAxisWithTableBWD == 0 && numPointsYAxisWithTableDown != 0){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				waveNameVector.push_back(waveBaseName + TRACE_DOWN_STRING);			
			}
			// 2 cubes, TraceUp, ReTraceUp
			else if(numPointsXAxisWithTableBWD != 0 && numPointsYAxisWithTableDown == 0){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);
				waveNameVector.push_back(waveBaseName + RE_TRACE_UP_STRING);		
			}
			// 1 cube, TraceUp
			else if(numPointsXAxisWithTableBWD == 0 && numPointsYAxisWithTableDown == 0){
				waveNameVector.push_back(waveBaseName + TRACE_UP_STRING);			
			}
			// not possible
			else{
				outputToHistory("BUG: Error in determining the number of cubes.");
				return INTERNAL_ERROR_CONVERTING_DATA;
			}

			// create waves
			for(itWaveNames = waveNameVector.begin(); itWaveNames != waveNameVector.end(); itWaveNames++){

				ret = MDMakeWave(&waveHandle, itWaveNames->c_str(),dataFolderHandle,dimensionSizes,NT_FP64,noOverwrite);

				if(ret == NAME_WAV_CONFLICT){
					sprintf(buf,"Wave %s already exists.",itWaveNames->c_str());
					debugOutputToHistory(buf);
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
				MemClear((double*) WaveData(waveHandle), waveSize*sizeof(double));
			}

			// set wave data pointers
			if(waveHandleVector.size() == 4){

				xAxisBlockSize	   = numPointsXAxisWithTableBWD*numPointsXAxisWithTableFWD*numPointsVAxis;

				traceUpDataPtr     = (double*) WaveData(waveHandleVector[0]);
				reTraceUpDataPtr   = (double*) WaveData(waveHandleVector[1]);
				traceDownDataPtr   = (double*) WaveData(waveHandleVector[2]);
				reTraceDownDataPtr = (double*) WaveData(waveHandleVector[3]);

			}
			else if(waveHandleVector.size() == 2 && numPointsXAxisWithTableBWD == 0){

				xAxisBlockSize	   = numPointsXAxisWithTableFWD*numPointsVAxis;

				traceUpDataPtr     = (double*) WaveData(waveHandleVector[0]);
				traceDownDataPtr   = (double*) WaveData(waveHandleVector[1]);			
			}
			else if(waveHandleVector.size() == 2 && numPointsYAxisWithTableDown == 0){

				xAxisBlockSize	   = numPointsXAxisWithTableBWD*numPointsXAxisWithTableFWD*numPointsVAxis;

				traceUpDataPtr     = (double*) WaveData(waveHandleVector[0]);
				reTraceUpDataPtr   = (double*) WaveData(waveHandleVector[1]);			
			}
			// no mirroring
			else if(waveHandleVector.size() == 1 && numPointsXAxisWithTableBWD == 0 && numPointsYAxisWithTableDown == 0){

				traceUpDataPtr    = (double*) WaveData(waveHandleVector[0]);			
			}
			else{
				outputToHistory("BUG: createAndFillDataWave()...");
				return INTERNAL_ERROR_CONVERTING_DATA;
			}

			// data index to the start of the TraceDown data
			firstBlockOffset = numPointsYAxisWithTableUp*xAxisBlockSize;

			sprintf(buf,"xAxisBlockSize=%d,firstBlockOffset=%d",xAxisBlockSize,firstBlockOffset);
			debugOutputToHistory(buf);

			// COLUMNS
			for(i = 0; i < numPointsYAxisWithTableUp; i++){
				// ROWS
				for(j=0; j < numPointsXAxisWithTableFWD; j++){
					// LAYERS
					for(k=0; k < numPointsVAxis; k++){

						// traceUp
						if(traceUpDataPtr){
							traceUpRawBrickletIndex	= i*xAxisBlockSize + j*numPointsVAxis + k;
							traceUpDataIndex		= i*numPointsXAxisWithTableFWD + j + k*numPointsXAxisWithTableFWD*numPointsYAxisWithTableUp;

							if(	traceUpDataIndex >= 0 &&
								traceUpDataIndex < waveSize &&
								traceUpRawBrickletIndex < rawBrickletSize &&
								traceUpRawBrickletIndex >= 0 ){

									rawValue	= rawBrickletDataPtr[traceUpRawBrickletIndex];
									scaledValue = rawValue*slope + yIntercept;

									traceUpDataPtr[traceUpDataIndex] = scaledValue;

//									long int indices[MAX_DIMENSIONS];		
//									MemClear(indices, sizeof(indices));
//
//									// use this for debugging purposes
//									indices[ROWS] = j; 
//									indices[COLUMNS] = i; 
//									indices[LAYERS] = k;
//
//									double altScaledValue = pSession->toPhysical(rawValue,pBricklet);
//									MDSetNumericWavePointValue(waveHandleVector[1],indices,&altScaledValue);

									//if(k < 10 && i < 2 && j < 2){
									//	sprintf(buf,"i=%d,j=%d,k=%d,traceUpRawBrickletIndex=%d,traceUpDataIndex=%d,rawValue=%d,scaledValue=%g",
									//		i,j,k,traceUpRawBrickletIndex,traceUpDataIndex,rawValue,scaledValue);
									//	debugOutputToHistory(buf);
									//	sprintf(buf,"altScaledValue=%g,diff=%g",altScaledValue,scaledValue-altScaledValue);
									//	debugOutputToHistory(buf);
									//}

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
								debugOutputToHistory("Index out of range in traceUp");

								sprintf(buf,"traceUpDataIndex=%d,waveSize=%d",traceUpDataIndex,waveSize);
								debugOutputToHistory(buf);

								sprintf(buf,"traceUpRawBrickletIndex=%d,rawBrickletSize=%d",traceUpRawBrickletIndex,rawBrickletSize);
								debugOutputToHistory(buf);

								traceUpDataPtr=NULL;
							}
					}// if traceUpDataPtr

					// traceDown
					if(traceDownDataPtr){

							traceDownRawBrickletIndex = firstBlockOffset + i*xAxisBlockSize + j*numPointsVAxis + k;
							traceDownDataIndex		  = ( numPointsYAxisWithTableUp -(i+1) )*numPointsXAxisWithTableFWD + j + k*numPointsXAxisWithTableFWD*numPointsYAxisWithTableUp;

						if(	traceDownDataIndex >= 0 &&
							traceDownDataIndex < waveSize &&
							traceDownRawBrickletIndex < rawBrickletSize &&
							traceDownRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[traceDownRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

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
							debugOutputToHistory("Index out of range in traceDown");

							sprintf(buf,"traceDownDataIndex=%d,waveSize=%d",traceDownDataIndex,waveSize);
							debugOutputToHistory(buf);

							sprintf(buf,"traceDownRawBrickletIndex=%d,rawBrickletSize=%d",traceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(buf);

							traceDownDataPtr=NULL;
						}
					}// if traceDownDataPtr

					// reTraceUp
					if(reTraceUpDataPtr){

						reTraceUpRawBrickletIndex = i*xAxisBlockSize + (numPointsXAxisWithTableFWD - (j+1))*numPointsVAxis + k;
						reTraceUpDataIndex		  = i*numPointsXAxisWithTableFWD + j + k*numPointsXAxisWithTableFWD*numPointsYAxisWithTableUp;

						if(	reTraceUpDataIndex >= 0 &&
							reTraceUpDataIndex < waveSize &&
							reTraceUpRawBrickletIndex < rawBrickletSize &&
							reTraceUpRawBrickletIndex >= 0
							){
								rawValue	= rawBrickletDataPtr[reTraceUpRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

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
							debugOutputToHistory("Index out of range in reTraceUp");

							sprintf(buf,"reTraceUpDataIndex=%d,waveSize=%d",reTraceUpDataIndex,waveSize);
							debugOutputToHistory(buf);

							sprintf(buf,"reTraceUpRawBrickletIndex=%d,rawBrickletSize=%d",reTraceUpRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(buf);

							reTraceUpDataPtr=NULL;
						}
					}// if reTraceUpDataPtr

					// reTraceDown
					if(reTraceDownDataPtr){

						reTraceDownRawBrickletIndex	= firstBlockOffset + i*xAxisBlockSize + (numPointsXAxisWithTableFWD - (j+1))*numPointsVAxis + k;
						reTraceDownDataIndex		= ( numPointsYAxisWithTableUp - (i+1) )*numPointsXAxisWithTableFWD + j + k*numPointsXAxisWithTableFWD*numPointsYAxisWithTableUp;

						if(	reTraceDownDataIndex >= 0 &&
							reTraceDownDataIndex < waveSize &&
							reTraceDownRawBrickletIndex < rawBrickletSize &&
							reTraceDownRawBrickletIndex >= 0
							){

								rawValue	= rawBrickletDataPtr[reTraceDownRawBrickletIndex];
								scaledValue = rawValue*slope + yIntercept;

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
							debugOutputToHistory("Index out of range in reTraceDown");

							sprintf(buf,"reTraceDownDataIndex=%d,waveSize=%d",reTraceDownDataIndex,waveSize);
							debugOutputToHistory(buf);

							sprintf(buf,"reTraceDownRawBrickletIndex=%d,rawBrickletSize=%d",reTraceDownRawBrickletIndex,rawBrickletSize);
							debugOutputToHistory(buf);

							reTraceDownDataPtr=NULL;
						}
					}// if reTraceDownDataPtr
				} // for LAYERS
			} // for ROWS
		} // for COLUMNS

			// unlock waves and set wave note
			for(i=0; i < hStateVector.size(); i++){
				HSetState(waveHandleVector[i],hStateVector[i]);
				setDataWaveNote(brickletID,rawMin[i],rawMax[i],scaledMin[i],scaledMax[i],waveHandleVector[i]);

				MDSetWaveScaling(waveHandleVector[i],ROWS,&xAxis.physicalIncrement,&setScaleOffset);
				MDSetWaveScaling(waveHandleVector[i],COLUMNS,&yAxis.physicalIncrement,&setScaleOffset);
				MDSetWaveScaling(waveHandleVector[i],LAYERS,&specAxis.physicalIncrement,&setScaleOffset);

				MDSetWaveUnits(waveHandleVector[i],ROWS,const_cast<char *>((WStringToString(xAxis.physicalUnit).c_str())));
				MDSetWaveUnits(waveHandleVector[i],COLUMNS,const_cast<char *>(WStringToString(yAxis.physicalUnit).c_str()));
				MDSetWaveUnits(waveHandleVector[i],LAYERS,const_cast<char *>(WStringToString(specAxis.physicalUnit).c_str()));
				MDSetWaveUnits(waveHandleVector[i],-1,const_cast<char *>(myBricklet->getMetaDataValueAsString("channelUnit").c_str()));
			}

			break;
	}

	return 0;
}
