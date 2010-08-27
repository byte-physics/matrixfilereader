
#include <string>
#include <sys/stat.h>
#include <limits.h>
#include <float.h>

#include "utils.h"

#include "xopstandardheaders.h"
#include "dataclass.h"

#include "globals.h"



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

void debugOutputToHistory(const char* str){

	if(pMyData->debuggingEnabled()){

		ASSERT_RETURN_VOID(str);

		char buf[ARRAY_SIZE];
		sprintf(buf,debugOutputFormat,str);
		XOPNotice(buf);
	}
}

void outputToHistory(const char *str){

	ASSERT_RETURN_VOID(str);

	char buf[ARRAY_SIZE];
	sprintf(buf,outputFormat,str);
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
	//debugOutputToHistory(buf);

	for(i=0; i < numEntriesPlusOne; i++){

		if(i == 0){// position of the first string
			offset = numEntriesPlusOne*sizeof(long);
		}
		else{ // and of all the others
			offset+=stringSizes[i-1];
		}
		//sprintf(buf,"offset=%d, offsetPosition=%d*sizeof(long)",offset,i);
		//debugOutputToHistory(buf);

		// write offsets
		memcpy(*textHandle+i*sizeof(long),&offset,sizeof(long));

		if(i < stringVector.size()){

			//sprintf(buf,"string=%s, stringPosition=%d",stringVector[i].c_str(),offset);
			//debugOutputToHistory(buf);

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
	//debugOutputToHistory(buf);

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
		debugOutputToHistory(buf);
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
	setOtherWaveNote(brickletID,waveHandle);

	return 0;
}

std::string viewTypeCodeToString(int idx){

	char buf[ARRAY_SIZE];
	std::vector<std::string> names;

	names.push_back(VTC_OTHER_STRING);
	names.push_back(VTC_SIMPLE2D_STRING);
	names.push_back(VTC_SIMPLE1D_STRING);
	names.push_back(VTC_FWDBWD2D_STRING);
	names.push_back(VTC_2DOF3D_STRING);
	names.push_back(VTC_SPECTROSCOPY_STRING);
	names.push_back(VTC_FORCECURVE_STRING);
	names.push_back(VTC_1DPROFILE_STRING);
	names.push_back(VTC_INTERFEROMETER);

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


void setDataWaveNote(int brickletID, int rawMin, int rawMax, double scaledMin, double scaledMax, waveHndl waveHandle){

	std::string	waveNote = getStandardWaveNote(brickletID);

	waveNote.append("rawMin="    + anyTypeToString<int>(rawMin)    + "\r");
	waveNote.append("rawMax="	 + anyTypeToString<int>(rawMax)	 + "\r");
	
	waveNote.append("scaledMin=" + anyTypeToString<double>(scaledMin) + "\r");
	waveNote.append("scaledMax=" + anyTypeToString<double>(scaledMax) + "\r");

	mySetWaveNote(waveNote,waveHandle);
}


void setOtherWaveNote(int brickletID, waveHndl waveHandle){

	std::string waveNote = getStandardWaveNote(brickletID);

	mySetWaveNote(waveNote,waveHandle);
}



std::string getStandardWaveNote(int brickletID){

	std::string waveNote;

	waveNote.append("resultFileName=" + pMyData->getFileName() + "\r");
	waveNote.append("resultFilePath=" + pMyData->getDirPath() + "\r");

	// we pass brickletID=0 for waveNotes concerning the resultFileMetaData wave
	if(brickletID > 0){
		waveNote.append("brickletID=" + anyTypeToString<int>(brickletID) + "\r");
	}
	else{
		waveNote.append("brickletID=\r");
	}

	waveNote.append("xopVersion=" + std::string(myXopVersion) + "\r");
	waveNote.append("vernissageVersion=" + pMyData->getVernissageVersion() + "\r");

	return waveNote;
}

void mySetWaveNote(std::string waveNote, waveHndl waveHandle){

	if(waveNote.empty()){
		outputToHistory("BUG: got empty waveNote in myData::setWaveNote.");
		return;
	}

	Handle noteHandle  = NewHandle(waveNote.size()) ;
	PutCStringInHandle(waveNote.c_str(),noteHandle);

	SetWaveNote(waveHandle, noteHandle);
}