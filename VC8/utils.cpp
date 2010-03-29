
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

void debugOutputToHistory(int level, char* str){
		
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

void outputToHistory(char *str){

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

	ASSERT_RETURN_ZERO(stringVector.size());

	std::vector<long int> stringSizes;
	long int totalSize=0;
	char buf[ARRAY_SIZE];

	const long int numEntriesPlusOne = stringVector.size()+1;

	std::vector<std::string>::const_iterator it;
	for(it = stringVector.begin(); it != stringVector.end(); it++){
		stringSizes.push_back(it->size());
		totalSize += it->size();
	}

	totalSize += numEntriesPlusOne*sizeof(long);

	char *textPtr = (char*) NewPtr(totalSize);
	if(MemError() || textPtr == NULL){
		return -1;
	}
	MemClear(textPtr,totalSize);

	int i;
	long int offset;

	sprintf(buf,"totalSize of strings %d",GetPtrSize(textPtr));
	debugOutputToHistory(DEBUG_LEVEL,buf);

	for(i=0; i < numEntriesPlusOne; i++){

		if(i == 0){
			offset = numEntriesPlusOne*sizeof(long);
		}
		else{
			offset+=stringSizes[i-1];
		}

		sprintf(buf,"offset=%d, offsetPosition=%d*sizeof(long)",offset,i);
		debugOutputToHistory(DEBUG_LEVEL,buf);
		
		//debugOutputToHistory(DEBUG_LEVEL,"before offset memcpy");

		// write offsets into textDataHandle
		memcpy(textPtr+i*sizeof(long),&offset,sizeof(long));
		//debugOutputToHistory(DEBUG_LEVEL,"after offset memcpy");

		if(i < stringVector.size()){

			sprintf(buf,"string=%s, stringPosition=%d",stringVector[i].c_str(),offset);
			debugOutputToHistory(DEBUG_LEVEL,buf);

			// put string into textDataHandle
			memcpy(textPtr+offset,stringVector[i].c_str(),stringSizes[i]);

			//debugOutputToHistory(DEBUG_LEVEL,"after string memcpy");
		}
	}
	//debugOutputToHistory(DEBUG_LEVEL,"after memcpy loop");

	// acquire waveHandle lock
	int hState=MoveLockHandle(waveHandle);
	int ret = SetTextWaveData(waveHandle,2,&textPtr);
	
	sprintf(buf,"SetTextWaveData returned %d",ret);
	debugOutputToHistory(DEBUG_LEVEL,buf);

	// release waveHandle lock
	HSetState((Handle) waveHandle, hState);

	DisposePtr((Ptr) textPtr);

	return ret;
}
