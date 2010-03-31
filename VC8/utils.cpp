
// Taken from http://www.codeguru.com/forum/archive/index.php/t-193852.html

#include <string>
#include <algorithm>


#include "utils.h"

#include "xopstandardheaders.h"

#include "dataclass.h"

#include "globalvariables.h"

#define DEBUG_LEVEL 1


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

	sprintf(buf,"totalSize of strings %d",GetHandleSize(textHandle));
	debugOutputToHistory(DEBUG_LEVEL,buf);

	for(i=0; i < numEntriesPlusOne; i++){

		if(i == 0){// position of the first string
			offset = numEntriesPlusOne*sizeof(long);
		}
		else{ // and of all the others
			offset+=stringSizes[i-1];
		}
		sprintf(buf,"offset=%d, offsetPosition=%d*sizeof(long)",offset,i);
		debugOutputToHistory(DEBUG_LEVEL,buf);

		// write offsets
		memcpy(*textHandle+i*sizeof(long),&offset,sizeof(long));

		if(i < stringVector.size()){

			sprintf(buf,"string=%s, stringPosition=%d",stringVector[i].c_str(),offset);
			debugOutputToHistory(DEBUG_LEVEL,buf);

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
	
	sprintf(buf,"SetTextWaveData returned %d",ret);
	debugOutputToHistory(DEBUG_LEVEL,buf);

	// release waveHandle lock
	HSetState(waveHandle, lockStateWave);

	DisposeHandle(textHandle);

	return ret;
}

int createAndFillTextWave(std::vector<std::string> &firstColumn, std::vector<std::string> &secondColumn, const char *waveName){

	long dimensionSizes[MAX_DIMENSIONS+1];
	int overwrite=0;
	waveHndl waveHandle;
	int ret=-1;
	char buf[ARRAY_SIZE];

	MemClear(dimensionSizes, sizeof(dimensionSizes));

	// create 2D textwave with firstColumn.size() rows and 2 columns
	if(firstColumn.size() == secondColumn.size() ){
		dimensionSizes[ROWS] = firstColumn.size();
	}
	else if(firstColumn.size() == 0 ){
		outputToHistory("BUG: got empty list");
		return UNKNOWN_ERROR;
	}
	else{
		outputToHistory("BUG: size mismatch of first and second column data");
		return UNKNOWN_ERROR;
	}

	dimensionSizes[COLUMNS]=2;

	// NULL says that we want to create the wave in the current datafolder
	ret = MDMakeWave(&waveHandle,waveName,NULL,dimensionSizes,TEXT_WAVE_TYPE,overwrite);

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

	// append the strings from the second column at the end of the first column strings
	firstColumn.insert(firstColumn.end(),secondColumn.begin(),secondColumn.end());
	ret = stringVectorToTextWave(firstColumn,waveHandle);

	if(ret != 0){
		sprintf(buf,"stringVectorToTextWave returned %d",ret);
		outputToHistory(buf);
		return ret;
	}

	return 0;
}