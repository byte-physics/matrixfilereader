

#include <string>
#include <algorithm>
#include <sys/stat.h>

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
	pMyData->setWaveNote(brickletID,waveHandle);

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

// TODO guess what ???
int createAndFillDataWave(DataFolderHandle dataFolderHandle, const char *waveBaseNameChar, int brickletID){

	char buf[ARRAY_SIZE];

	const int noOverwrite=0;

	int dimension;
	std::vector<Vernissage::Session::ViewTypeCode> viewTypeCodes;
	Vernissage::Session *pSession;
	std::vector<std::string> allAxes;
	Vernissage::Session::AxisDescriptor triggerAxis, rootAxis; 
	int numPointsTriggerAxis, numPointsRootAxis, numWavesToCreate, ret=-1,i;
	double physicalLengthTriggerAxis, physicalLengthRootAxis;
	std::vector<waveHndl> waveHandleVector;
	waveHndl waveHandle;
	std::vector<std::string> waveNameVector;

	std::vector<std::string>::const_iterator itWaveNames;

	long dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));

	std::string waveBaseName(waveBaseNameChar);

	ASSERT_RETURN_MINUSONE(pMyData);
	MyBricklet *myBricklet = pMyData->getBrickletClassFromMap(brickletID);
	void *pBricklet = myBricklet->getBrickletPointer();
	pSession = pMyData->getSession();
	ASSERT_RETURN_MINUSONE(myBricklet);
	ASSERT_RETURN_MINUSONE(pSession);

	myBricklet->getDimension(dimension);
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


	switch(dimension){
	
		case 1:

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

			// Now we can determine the Bricklet "width". If the axis has the unit
			// "meter", then it would be really a width (probably the scan area
			// width.)
			physicalLengthTriggerAxis = triggerAxis.physicalStart + triggerAxis.physicalIncrement * ( numPointsTriggerAxis -1 );

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

			// Now we can determine the Bricklet "width". If the axis has the unit
			// "meter", then it would be really a width (probably the scan area
			// width.)
			physicalLengthRootAxis = rootAxis.physicalStart + rootAxis.physicalIncrement * (numPointsRootAxis - 1);

			sprintf(buf,"physicalLengthRootAxis=%g",physicalLengthRootAxis);
			debugOutputToHistory(DEBUG_LEVEL,buf);

			sprintf(buf,"physicalLengthTriggerAxis=%g",physicalLengthTriggerAxis);
			debugOutputToHistory(DEBUG_LEVEL,buf);

			// now we have to disinguish three cases on how many 2D waves we need
			// both are mirrored:		4
			// one mirrored, one not:	2
			// none mirrored:			1
			if( triggerAxis.mirrored && rootAxis.mirrored ){
				waveNameVector.push_back(waveBaseName + "_TraceUp");
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

			// check order of ROWS and COLUMNS
			dimensionSizes[ROWS] = numPointsTriggerAxis;
			dimensionSizes[COLUMNS] = numPointsRootAxis;

			// get pointer to the raw data
			const int **rawData;
			int count;
			myBricklet->getBrickletContentsBuffer(rawData,count);

			for(itWaveNames = waveNameVector.begin(); itWaveNames != waveNameVector.end(); itWaveNames++){

				ret = MDMakeWave(&waveHandle, itWaveNames->c_str(),dataFolderHandle,dimensionSizes,NT_I32,noOverwrite);

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
			}

			// FIXME table sets are currently ignored!!
			// TODO copy data to waves, make a solution which works for all cases of mirroring!
			for(i = 0; i <numPointsTriggerAxis; i++){
			
				// copy the data linewise with memcpy at the place we got by WaveData
			}

			break;

		case 3:

			break;

		default:
			sprintf(buf,"BUG: Dimension %d can not be handled");
			outputToHistory(buf);
			break;
	
	
	}


	return 0;
}