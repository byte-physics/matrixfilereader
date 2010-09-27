
#include <string>
#include <limits>

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

	if(!pMyData->debuggingEnabled()){
		return;
	}

	ASSERT_RETURN_VOID(str);
	char buf[ARRAY_SIZE];

	sprintf(buf,debugOutputFormat,str);
	XOPNotice(buf);
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
	
	int ret;
	unsigned int i;

	long int offset;
	long int totalSize=0;

	// number of 32-bit integers (aka long int) is one more compared to the number of strings
	const unsigned long int numEntriesPlusOne = stringVector.size()+1;

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

	//sprintf(pMyData->outputBuffer,"totalSize of strings %d",GetHandleSize(textHandle));
	//debugOutputToHistory(pMyData->outputBuffer);

	for(i=0; i < numEntriesPlusOne; i++){

		if(i == 0){// position of the first string
			offset = numEntriesPlusOne*sizeof(long);
		}
		else{ // and of all the others
			offset+=stringSizes[i-1];
		}
		//sprintf(pMyData->outputBuffer,"offset=%d, offsetPosition=%d*sizeof(long)",offset,i);
		//debugOutputToHistory(pMyData->outputBuffer);

		// write offsets
		memcpy(*textHandle+i*sizeof(long),&offset,sizeof(long));

		if(i < stringVector.size()){

			//sprintf(pMyData->outputBuffer,"string=%s, stringPosition=%d",stringVector[i].c_str(),offset);
			//debugOutputToHistory(pMyData->outputBuffer);

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

	ret = SetTextWaveData(waveHandle,mode,textHandle);
	
	//sprintf(pMyData->outputBuffer,"SetTextWaveData returned %d",ret);
	//debugOutputToHistory(pMyData->outputBuffer);
	DisposeHandle(textHandle);

	return ret;
}

int createAndFillTextWave(std::vector<std::string> &firstColumn, std::vector<std::string> &secondColumn, DataFolderHandle dataFolderHandle,const char *waveName, int brickletID){

	long dimensionSizes[MAX_DIMENSIONS+1];
	waveHndl waveHandle;
	int ret=-1;
	
	
	std::vector<std::string> allColumns;

	MemClear(dimensionSizes, sizeof(dimensionSizes));

	// create 2D textwave with firstColumn.size() rows and 2 columns
	if(firstColumn.size() == secondColumn.size() ){
		dimensionSizes[ROWS] = firstColumn.size();
	}
	else if(firstColumn.size() == 0 || secondColumn.size() == 0 ){
		sprintf(pMyData->outputBuffer,"BUG: list sizes may not be zero: first column size %d, second column size %d",firstColumn.size(), secondColumn.size());
		outputToHistory(pMyData->outputBuffer);
		return UNKNOWN_ERROR;
	}
	else{
		sprintf(pMyData->outputBuffer,"BUG: size mismatch of first column %d and second column %d",firstColumn.size(), secondColumn.size());
		outputToHistory(pMyData->outputBuffer);
		
		std::vector<std::string>::const_iterator it;

		outputToHistory("keys are:");
		for(it= firstColumn.begin(); it != firstColumn.end(); it++){
			sprintf(pMyData->outputBuffer,"_%s_",it->c_str());
			outputToHistory(pMyData->outputBuffer);
		}

		outputToHistory("values are:");
		for(it= secondColumn.begin(); it != secondColumn.end(); it++){
			sprintf(pMyData->outputBuffer,"_%s_",it->c_str());
			outputToHistory(pMyData->outputBuffer);
		}
		return UNKNOWN_ERROR;
	}

	dimensionSizes[COLUMNS]=2;
	
	ret = MDMakeWave(&waveHandle,waveName,dataFolderHandle,dimensionSizes,TEXT_WAVE_TYPE,pMyData->overwriteEnabledAsInt());

	if(ret == NAME_WAV_CONFLICT){
		sprintf(pMyData->outputBuffer,"Wave %s already exists.",waveName);
		debugOutputToHistory(pMyData->outputBuffer);
		return WAVE_EXIST;
	}

	if(ret != 0 ){
		sprintf(pMyData->outputBuffer,"Error %d in creating wave %s.",ret, waveName);
		outputToHistory(pMyData->outputBuffer);
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
		sprintf(pMyData->outputBuffer,"stringVectorToTextWave returned %d",ret);
		outputToHistory(pMyData->outputBuffer);
		return ret;
	}
	setOtherWaveNote(brickletID,waveHandle);

	return 0;
}

std::string viewTypeCodeToString(int idx){

	
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

	if(idx < 0 || idx >= int(names.size())){
		sprintf(pMyData->outputBuffer,"BUG: viewTypeCodeToString got %d as parameter, but it should be between 0 and %d",idx,names.size()-1);
		debugOutputToHistory(pMyData->outputBuffer);
		return std::string();
	}
	else{
		return names.at(idx);
	}
}

void setDataWaveNote(int brickletID, int rawMin, int rawMax, double physicalValueOfRawMin, double physicalValueOfRawMax, waveHndl waveHandle){

	std::string	waveNote = getStandardWaveNote(brickletID);

	waveNote.append("rawMin="    + anyTypeToString<int>(rawMin)    + "\r");
	waveNote.append("rawMax="	 + anyTypeToString<int>(rawMax)	 + "\r");
	
	waveNote.append("physicalValueOfRawMin=" + anyTypeToString<double>(physicalValueOfRawMin) + "\r");
	waveNote.append("physicalValueOfRawMax=" + anyTypeToString<double>(physicalValueOfRawMax) + "\r");

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
	if(MemError() || noteHandle == NULL){
		return;
	}
	int ret = PutCStringInHandle(waveNote.c_str(),noteHandle);
	if(ret != 0){
		sprintf(pMyData->outputBuffer,"internal error %d, aborting",ret);
		outputToHistory(pMyData->outputBuffer);
		return;
	}

	SetWaveNote(waveHandle, noteHandle);
}

void waveClearNaN64(double *data, long size){

	long i;
	
	for (i = 0; i < size; i++){
		*data++ = DOUBLE_NAN;
	}
}

void waveClearNaN32(float *data, long size){

	long i;
	
	for (i = 0; i < size; i++){
		*data++ = SINGLE_NAN;
	}
}


void splitString(char* stringChar, char *sepChar, std::vector<std::string> &list){

	if(stringChar == NULL || sepChar == NULL){
		return;
	}

	list.clear();

	
	int pos=-1;
	int offset=0;

	std::string string;
	string = stringChar;

	string.append(sepChar); // add ; at the end to make the list complete, double ;; are no problem

	sprintf(pMyData->outputBuffer,"keyList=%s",stringChar);
	debugOutputToHistory(pMyData->outputBuffer);


	while( ( pos = string.find(sepChar,offset) ) != std::string::npos ){

		if(pos == offset){// skip empty element
			offset++;
			continue;
		}

		list.push_back(string.substr(offset,pos-offset));
		sprintf(pMyData->outputBuffer,"key=%s,pos=%d,offset=%d",list.back().c_str(),pos,offset);
		debugOutputToHistory(pMyData->outputBuffer);

		offset = pos+1;
	}
}