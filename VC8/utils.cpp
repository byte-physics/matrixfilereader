

#include <string>
#include <algorithm>
#include <sys/stat.h>
#include <limits.h>
#include <float.h>

#include "utils.h"

#include "xopstandardheaders.h"
#include "dataclass.h"

#include "globalvariables.h"

#define DEBUG_LEVEL 1

// Taken from http://www.codeguru.com/forum/archive/index.php/t-193852.html

std::wstring StringToWString(const std::string& s)
{
	std::wstring temp(s.length(),L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}


std::string WStringToString(const std::wstring& s)
{
	std::string temp(s.length(), ' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

// till here

std::wstring CharPtrToWString(char* cStr){

	if(cStr == NULL){
		return std::wstring();
	}

	std::string str(cStr);
	return StringToWString(str);
}

void debugOutputToHistory(int level,const char* str){
		
		const char* formatString = "DEBUG %d: %s\r";

		if(str == NULL){
			outputToHistory("BUG: null pointer in debugOutputToHistory");
			return;
		}

		if(level > MAX_DEBUG_LEVEL){
			outputToHistory("BUG: debug level must no exceed MAX_DEBUG_LEVEL.");
			return;
		}
		
		int length = strlen(str) + strlen(formatString) + 2;
		if(length >= ARRAY_SIZE){
			outputToHistory("BUG: string too long to output.");
			return;
		}

		if(pMyData == NULL){
			outputToHistory("BUG: pMyData is NULL.");
			return;
		}

		if(level >= pMyData->getDebugLevel()){
			char buf[ARRAY_SIZE];
			sprintf(buf,formatString,level,str);
			XOPNotice(buf);
		}
	}

void outputToHistory(const char *str){

		const char* formatString = "%s\r";	

		if( str == NULL){
			XOPNotice("BUG: null pointer in outputToHistory\r");
			return;		
		}

		int length = strlen(str) + strlen(formatString);

		if(length >= ARRAY_SIZE){
			XOPNotice("BUG: string too long to output\r");
			return;
		}

		char buf[ARRAY_SIZE];
		sprintf(buf,formatString,str);
		XOPNotice(buf);
}

int stringVectorToTextWave(std::vector<std::string> &stringVector, waveHndl &waveHandle){

	ASSERT_RETURN_MINUSONE(stringVector.size());

	std::vector<long int> stringSizes;
	char buf[ARRAY_SIZE];
	int lockStateWave, ret, i;

	long int offset;
	long int totalSize=0;

	// number of 32-bit integers (aka long int) is one more compared to the number of strings
	const long int numEntriesPlusOne = stringVector.size()+1;

	std::vector<std::string>::const_iterator it;
	for(it = stringVector.begin(); it != stringVector.end(); it++){
		stringSizes.push_back(it->size());
		totalSize += it->size();
	}

	totalSize += numEntriesPlusOne*sizeof(long);

	Handle textHandle = NewHandle(totalSize);
	if(MemError() || textHandle == NULL){
		return NOMEM;
	}

	//sprintf(buf,"totalSize of strings %d",GetHandleSize(textHandle));
	//debugOutputToHistory(DEBUG_LEVEL,buf);

	for(i=0; i < numEntriesPlusOne; i++){

		if(i == 0){// position of the first string
			offset = numEntriesPlusOne*sizeof(long);
		}
		else{ // and of all the others
			offset+=stringSizes[i-1];
		}
		//sprintf(buf,"offset=%d, offsetPosition=%d*sizeof(long)",offset,i);
		//debugOutputToHistory(DEBUG_LEVEL,buf);

		// write offsets
		memcpy(*textHandle+i*sizeof(long),&offset,sizeof(long));

		if(i < stringVector.size()){

			//sprintf(buf,"string=%s, stringPosition=%d",stringVector[i].c_str(),offset);
			//debugOutputToHistory(DEBUG_LEVEL,buf);

			// write strings
			memcpy(*textHandle+offset,stringVector[i].c_str(),stringSizes[i]);
		}
	}

	// mode = 2 defines the format of the handle contents to
	// offsetToFirstString
	// offsetToSecondString
	// ...
	// offsetToPositionAfterLastString
	// firstString
	//...
	int mode=2;

	// acquire waveHandle lock
	lockStateWave=MoveLockHandle(waveHandle);
	ret = SetTextWaveData(waveHandle,mode,textHandle);
	
	//sprintf(buf,"SetTextWaveData returned %d",ret);
	//debugOutputToHistory(DEBUG_LEVEL,buf);

	// release waveHandle lock
	HSetState(waveHandle, lockStateWave);

	DisposeHandle(textHandle);

	return ret;
}

int createAndFillTextWave(std::vector<std::string> &firstColumn, std::vector<std::string> &secondColumn, DataFolderHandle dataFolderHandle,const char *waveName, int brickletID){

	long dimensionSizes[MAX_DIMENSIONS+1];
	int overwrite=0;
	waveHndl waveHandle;
	int ret=-1;
	char buf[ARRAY_SIZE];
	std::vector<std::string> allColumns;

	MemClear(dimensionSizes, sizeof(dimensionSizes));

	// create 2D textwave with firstColumn.size() rows and 2 columns
	if(firstColumn.size() == secondColumn.size() ){
		dimensionSizes[ROWS] = firstColumn.size();
	}
	else if(firstColumn.size() == 0 || secondColumn.size() == 0 ){
		sprintf(buf,"BUG: list sizes may not be zero: first column size %d, second column size %d",firstColumn.size(), secondColumn.size());
		outputToHistory(buf);
		return UNKNOWN_ERROR;
	}
	else{
		sprintf(buf,"BUG: size mismatch of first column %d and second column %d",firstColumn.size(), secondColumn.size());
		outputToHistory(buf);
		
		std::vector<std::string>::const_iterator it;

		outputToHistory("keys are:");
		for(it= firstColumn.begin(); it != firstColumn.end(); it++){
			sprintf(buf,"_%s_",it->c_str());
			outputToHistory(buf);
		}

		outputToHistory("values are:");
		for(it= secondColumn.begin(); it != secondColumn.end(); it++){
			sprintf(buf,"_%s_",it->c_str());
			outputToHistory(buf);
		}
		return UNKNOWN_ERROR;
	}

	dimensionSizes[COLUMNS]=2;
	
	ret = MDMakeWave(&waveHandle,waveName,dataFolderHandle,dimensionSizes,TEXT_WAVE_TYPE,overwrite);

	if(ret == NAME_WAV_CONFLICT){
		sprintf(buf,"Wave %s already exists.",waveName);
		debugOutputToHistory(DEBUG_LEVEL,buf);
		return WAVE_EXIST;
	}

	if(ret != 0 ){
		sprintf(buf,"Error %d in creating wave %s.",ret, waveName);
		outputToHistory(buf);
		return UNKNOWN_ERROR;
	}

	ASSERT_RETURN_MINUSONE(waveHandle);

	// copy the strings of both columns into a new vector
	// so that they are then 1D
	// first firstColumn, then secondColumn
	allColumns.insert(allColumns.begin(),firstColumn.begin(),firstColumn.end());
	allColumns.insert(allColumns.end(),secondColumn.begin(),secondColumn.end());

	ret = stringVectorToTextWave(allColumns,waveHandle);

	if(ret != 0){
		sprintf(buf,"stringVectorToTextWave returned %d",ret);
		outputToHistory(buf);
		return ret;
	}

	ASSERT_RETURN_ZERO(pMyData);
	pMyData->setOtherWaveNote(brickletID,waveHandle);

	return 0;
}

std::string viewTypeCodeToString(int idx){

	char buf[ARRAY_SIZE];
	std::vector<std::string> names;

	names.push_back("vtc_Other");
	names.push_back("vtc_Simple2D");
	names.push_back("vtc_Simple1D");
	names.push_back("vtc_ForwardBackward2D");
	names.push_back("vtc_2Dof3D");
	names.push_back("vtc_Spectroscopy");
	names.push_back("vtc_ForceCurve");
	names.push_back("vtc_1DProfile");

	if(idx < 0 || idx >= names.size()){
		sprintf(buf,"BUG: viewTypeCodeToString got %d as parameter, but it should be between 0 and %d",idx,names.size()-1);
		outputToHistory(buf);
		return 0;
	}
	else{
		return names.at(idx);
	}
}

std::string valueTypeToString(int idx){

	char buf[ARRAY_SIZE];
	std::vector<std::string> names;

	names.push_back(VT_SPECIAL_STRING);
	names.push_back(VT_INTEGER_STRING);
	names.push_back(VT_DOUBLE_STRING);
	names.push_back(VT_BOOLEAN_STRING);
	names.push_back(VT_ENUM_STRING);
	names.push_back(VT_STRING_STRING);

	if(idx < 0 || idx >= names.size()){
		sprintf(buf,"BUG: valueTypeToString got %d as parameter, but it should be between 0 and %d",idx,names.size()-1);
		outputToHistory(buf);
		return 0;
	}
	else{
		return names.at(idx);
	}
}


bool fileExists(std::string filePath, std::string fileName){

	struct stat stFileInfo;
	int intStat;
	std::string fullPath = filePath + "\\" + fileName;

	// Attempt to get the file attributes
	intStat = stat(fullPath.c_str(),&stFileInfo);
	if(intStat == 0) {
		return true;
	} else{
		return false;
	}
}

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
	pSession = pMyData->getSession();

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

			pMyData->setDataWaveNote(brickletID,rawMinValue,rawMaxValue,scaledMinValue,scaledMaxValue,waveHandle);

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
				waveNameVector.push_back(waveBaseName + "_TraceUp");
				waveNameVector.push_back(waveBaseName + "_ReTraceUp");
				waveNameVector.push_back(waveBaseName + "_TraceDown");
				waveNameVector.push_back(waveBaseName + "_ReTraceDown");
			}
			else if( triggerAxis.mirrored ){
				waveNameVector.push_back(waveBaseName + "_TraceUp");
				waveNameVector.push_back(waveBaseName + "_ReTraceUp");
			}
			else if( rootAxis.mirrored ){
				waveNameVector.push_back(waveBaseName + "_TraceUp");
				waveNameVector.push_back(waveBaseName + "_TraceDown");
			}
			else{
				waveNameVector.push_back(waveBaseName + "_TraceUp");
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
				pMyData->setDataWaveNote(brickletID,rawMin[i],rawMax[i],scaledMin[i],scaledMax[i],waveHandleVector[i]);

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
			return INTERNAL_ERROR_CONVERTING_DATA;

			break;

		default:
			sprintf(buf,"BUG: Dimension %d can not be handled",dimension);
			outputToHistory(buf);
			break;	
	}


	return 0;
}