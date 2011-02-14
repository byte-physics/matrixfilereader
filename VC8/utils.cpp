/*
	The file utils.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "header.h"

#include <string>

#include "globaldata.h"
#include "utils.h"

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

std::wstring CharPtrToWString(char* cStr){

	if(cStr == NULL){
		return std::wstring();
	}

	std::string str(cStr);
	return StringToWString(str);
}

void debugOutputToHistory(const char* str, bool silent /* = false */){

	if(!globDataPtr->debuggingEnabled()){
		return;
	}

	ASSERT_RETURN_VOID(str);
	char buf[ARRAY_SIZE];

	sprintf(buf,debugOutputFormat,str);

	if(silent){
		XOPNotice2(buf,0);
	}
	else{
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

	ASSERT_RETURN_ONE(stringVector.size());

	std::vector<long int> stringSizes;

	int ret;
	unsigned int i;

	long int offset;
	long int totalSize=0;

	// number of 32-bit integers (aka long int) is one more compared to the number of strings
	const unsigned long int numEntriesPlusOne = stringVector.size()+1;

	std::vector<std::string>::const_iterator it;
	for(it = stringVector.begin(); it != stringVector.end(); it++){
		try{
			stringSizes.push_back(it->size());
		}catch(...){
			return NOMEM;
		}
		totalSize += it->size();
	}

	totalSize += numEntriesPlusOne*sizeof(long);

	Handle textHandle = NewHandle(totalSize);
	if(MemError() || textHandle == NULL){
		return NOMEM;
	}

	//sprintf(globDataPtr->outputBuffer,"totalSize of strings %d",GetHandleSize(textHandle));
	//debugOutputToHistory(globDataPtr->outputBuffer);

	for(i=0; i < numEntriesPlusOne; i++){

		if(i == 0){// position of the first string
			offset = numEntriesPlusOne*sizeof(long);
		}
		else{ // and of all the others
			offset+=stringSizes[i-1];
		}
		//sprintf(globDataPtr->outputBuffer,"offset=%d, offsetPosition=%d*sizeof(long)",offset,i);
		//debugOutputToHistory(globDataPtr->outputBuffer);

		// write offsets
		memcpy(*textHandle+i*sizeof(long),&offset,sizeof(long));

		if(i < stringVector.size()){

			//sprintf(globDataPtr->outputBuffer,"string=%s, stringPosition=%d",stringVector[i].c_str(),offset);
			//debugOutputToHistory(globDataPtr->outputBuffer);

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
	
	//sprintf(globDataPtr->outputBuffer,"SetTextWaveData returned %d",ret);
	//debugOutputToHistory(globDataPtr->outputBuffer);
	DisposeHandle(textHandle);

	return ret;
}

int createAndFillTextWave(std::vector<std::string> &firstColumn, std::vector<std::string> &secondColumn, DataFolderHandle dataFolderHandle,const char *waveName, int brickletID, std::string &fullPathOfCreatedWaves){

	std::vector<std::string> allColumns;
	waveHndl waveHandle;
	int ret=-1;
	CountInt dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));

	// create 2D textwave with firstColumn.size() rows and 2 columns
	if(firstColumn.size() == 0 || secondColumn.size() == 0 ){
		sprintf(globDataPtr->outputBuffer,"BUG: list sizes may not be zero: first column size %d, second column size %d",firstColumn.size(), secondColumn.size());
		outputToHistory(globDataPtr->outputBuffer);
		return UNKNOWN_ERROR;
	}
	else if(firstColumn.size() == secondColumn.size() ){
		dimensionSizes[ROWS] = firstColumn.size();
	}
	else{
		sprintf(globDataPtr->outputBuffer,"BUG: size mismatch of first column %d and second column %d",firstColumn.size(), secondColumn.size());
		outputToHistory(globDataPtr->outputBuffer);
		return UNKNOWN_ERROR;
	}

	dimensionSizes[COLUMNS]=2;
	
	ret = MDMakeWave(&waveHandle,waveName,dataFolderHandle,dimensionSizes,TEXT_WAVE_TYPE,globDataPtr->overwriteEnabledAsInt());

	if(ret == NAME_WAV_CONFLICT){
		sprintf(globDataPtr->outputBuffer,"Wave %s already exists.",waveName);
		debugOutputToHistory(globDataPtr->outputBuffer);
		return WAVE_EXIST;
	}
	else if(ret != 0 ){
		sprintf(globDataPtr->outputBuffer,"Error %d in creating wave %s.",ret, waveName);
		outputToHistory(globDataPtr->outputBuffer);
		return UNKNOWN_ERROR;
	}
	ASSERT_RETURN_ONE(waveHandle);

	// copy the strings of both columns into a new vector
	// so that they are then 1D
	// first firstColumn, then secondColumn
	try{
		allColumns.reserve(firstColumn.size() + secondColumn.size());
	}
	catch(...){
		XOPNotice("Out of memory in createAndFillTextWave()\r");
		return UNKNOWN_ERROR;
	}
	allColumns.insert(allColumns.begin(),firstColumn.begin(),firstColumn.end());
	allColumns.insert(allColumns.end(),secondColumn.begin(),secondColumn.end());

	ret = stringVectorToTextWave(allColumns,waveHandle);

	if(ret != 0){
		sprintf(globDataPtr->outputBuffer,"stringVectorToTextWave returned %d",ret);
		outputToHistory(globDataPtr->outputBuffer);
		return ret;
	}
	setOtherWaveNote(brickletID,waveHandle);
	fullPathOfCreatedWaves.append(getFullWavePath(dataFolderHandle,waveHandle));
	fullPathOfCreatedWaves.append(";");

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
		sprintf(globDataPtr->outputBuffer,"BUG: viewTypeCodeToString got %d as parameter, but it should be between 0 and %d",idx,names.size()-1);
		debugOutputToHistory(globDataPtr->outputBuffer);
		return std::string();
	}
	else{
		return names.at(idx);
	}
}

void setDataWaveNote(int brickletID, int rawMin, int rawMax, double physicalValueOfRawMin, double physicalValueOfRawMax, waveHndl waveHandle, int pixelSize /* = 1 */){

	std::string	waveNote = getStandardWaveNote(brickletID);

	waveNote.append("rawMin="    + anyTypeToString<int>(rawMin)    + "\r");
	waveNote.append("rawMax="	 + anyTypeToString<int>(rawMax)	 + "\r");
	
	waveNote.append("physicalValueOfRawMin=" + anyTypeToString<double>(physicalValueOfRawMin) + "\r");
	waveNote.append("physicalValueOfRawMax=" + anyTypeToString<double>(physicalValueOfRawMax) + "\r");
	waveNote.append("pixelSize=" + anyTypeToString<int>(pixelSize) + "\r");

	//clear the wave note
	SetWaveNote(waveHandle,NULL);
	appendToWaveNote(waveNote,waveHandle);
}

void setDataWaveNote(int brickletID, extremaData &extremaData, waveHndl waveHandle, int pixelSize /* = 1 */){
	setDataWaveNote(brickletID,extremaData.rawMin,extremaData.rawMax,extremaData.physValRawMin,extremaData.physValRawMax,waveHandle,pixelSize);
}
void setOtherWaveNote(int brickletID, waveHndl waveHandle){

	std::string waveNote = getStandardWaveNote(brickletID);

	//clear the wave note
	SetWaveNote(waveHandle,NULL);
	appendToWaveNote(waveNote,waveHandle);
}

std::string getStandardWaveNote(int brickletID){

	std::string waveNote;

	waveNote.append(RESULT_FILE_NAME_KEY + "=" + globDataPtr->getFileName() + "\r");
	waveNote.append(RESULT_DIR_PATH_KEY +"=" + globDataPtr->getDirPath() + "\r");

	// we pass brickletID=0 for waveNotes concerning the resultFileMetaData wave
	if(brickletID > 0){
		waveNote.append(BRICKLET_ID_KEY + "=" + anyTypeToString<int>(brickletID) + "\r");
	}
	else{
		waveNote.append(BRICKLET_ID_KEY + "=\r");
	}

	waveNote.append("xopVersion=" + std::string(MatrixFileReader_XOP_VERSION_STR) + "\r");
	waveNote.append("vernissageVersion=" + globDataPtr->getVernissageVersion() + "\r");

	return waveNote;
}

void appendToWaveNote(std::string waveNote, waveHndl waveHandle){

	if(waveNote.empty()){
		outputToHistory("BUG: got empty waveNote in GlobalData::appendToWaveNote.");
		return;
	}

	int ret;
	Handle newNoteHandle, existingNoteHandle;
	std::string existingNote;

	existingNoteHandle = WaveNote(waveHandle);
	convertHandleToString(existingNoteHandle,existingNote);
	existingNote += waveNote;

	newNoteHandle = NewHandle(existingNote.size()) ;
	if(MemError() || newNoteHandle   == NULL){
		return;
	}

	ret = PutCStringInHandle(existingNote.c_str(),newNoteHandle);
	if(ret != 0){
		sprintf(globDataPtr->outputBuffer,"internal error %d, aborting",ret);
		outputToHistory(globDataPtr->outputBuffer);
		return;
	}
	SetWaveNote(waveHandle, newNoteHandle);
}

void waveClearNaN64(double *data, long size){

	long i;

	ASSERT_RETURN_VOID(data);

	for (i = 0; i < size; i++){
		*data++ = DOUBLE_NAN;
	}
}

void waveClearNaN32(float *data, long size){

	long i;

	ASSERT_RETURN_VOID(data);
	
	for (i = 0; i < size; i++){
		*data++ = SINGLE_NAN;
	}
}

void splitString(char *strChar, char *sepChar, std::vector<std::string> &list){
	
	list.clear();

	if(strChar == NULL){
		return;
	}

	std::string str = strChar;
	splitString(str, sepChar, list);
}

void splitString(std::string &string, char *sepChar, std::vector<std::string> &list){

	list.clear();

	if(sepChar == NULL){
		return;
	}

	int pos=-1;
	int offset=0;

	string.append(sepChar); // add ; at the end to make the list complete, double ;; are no problem

	sprintf(globDataPtr->outputBuffer,"keyList=%s",string.c_str());
	debugOutputToHistory(globDataPtr->outputBuffer);

	while( ( pos = string.find(sepChar,offset) ) != std::string::npos ){

		if(pos == offset){// skip empty element
			offset++;
			continue;
		}

		list.push_back(string.substr(offset,pos-offset));
		sprintf(globDataPtr->outputBuffer,"key=%s,pos=%d,offset=%d",list.back().c_str(),pos,offset);
		debugOutputToHistory(globDataPtr->outputBuffer);

		offset = pos+1;
	}
}

void joinString(std::vector<std::string> &list,const char *sepChar, std::string &joinedList){

	std::vector<std::string>::const_iterator it;
	for(it = list.begin(); it != list.end(); it++){
		joinedList.append(*it);
		joinedList.append(sepChar);	
	}
	debugOutputToHistory(joinedList.c_str());

	return;
}

bool doubleToBool(double value){
	
	bool result;

	if( value < doubleEpsilon){
		result = false;
	}
	else{
		result = true;
	}

	return result;
}

bool isValidBrickletRange(double startID, double endID,int numberOfBricklets){

	// brickletIDs are 1-based
	return ( startID <=  endID
		&& startID >=  1
		&& endID   >=  1
		&& startID <= numberOfBricklets
		&& endID   <= numberOfBricklets );
}

bool isValidBrickletID(int brickletID, int numberOfBricklets){

	return brickletID >= 1 && brickletID <= numberOfBricklets;
}

std::string getFullWavePath(DataFolderHandle df, waveHndl wv){

	char waveName[MAX_OBJ_NAME+1];
	char dataFolderPath[MAXCMDLEN+1];
	std::string fullPath;
	int ret;

	// flags=3 returns the full path to the datafolder and including quotes if needed
	ret = GetDataFolderNameOrPath(df, 3,dataFolderPath);
	if(ret != 0){
		debugOutputToHistory("BUG: Could not query the datafolder for its name");
		return fullPath;
	}
	
	WaveName(wv,waveName);
	fullPath.append(dataFolderPath);
	fullPath.append(waveName);
	return fullPath;
}

int createRawDataWave(DataFolderHandle dfHandle,char *waveName, int brickletID, std::string &fullPathOfCreatedWaves){

	const int *pBuffer;
	int* dataPtr = NULL;
	int count=0,ret;
	CountInt dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));
	waveHndl waveHandle;

	BrickletClass *bricklet = globDataPtr->getBrickletClassObject(brickletID);
	ASSERT_RETURN_MINUSONE(bricklet);

	bricklet ->getBrickletContentsBuffer(&pBuffer,count);

	if(count == 0 || pBuffer == NULL){
		outputToHistory("Could not load bricklet contents.");
		return 0;
	}
	// create 1D wave with count points
	dimensionSizes[ROWS]=count;

	ret = MDMakeWave(&waveHandle,waveName,dfHandle,dimensionSizes,NT_I32,globDataPtr->overwriteEnabledAsInt());
	if(ret == NAME_WAV_CONFLICT){
		sprintf(globDataPtr->outputBuffer,"Wave %s already exists.",waveName);
		debugOutputToHistory(globDataPtr->outputBuffer);
		return WAVE_EXIST;	
	}
	else if(ret != 0 ){
		return ret;
	}

	ASSERT_RETURN_ONE(waveHandle);
	dataPtr = getWaveDataPtr<int>(waveHandle);
	ASSERT_RETURN_ONE(dataPtr);
	ASSERT_RETURN_ONE(pBuffer);

	memcpy(dataPtr,pBuffer,count*sizeof(int));
	
	setDataWaveNote(brickletID,bricklet->getRawMin(),bricklet->getRawMax(),bricklet->getPhysValRawMin(),bricklet->getPhysValRawMax(),waveHandle);
	
	fullPathOfCreatedWaves.append(getFullWavePath(dfHandle, waveHandle));
	fullPathOfCreatedWaves.append(";");
	return ret;
}

void convertHandleToString(Handle strHandle,std::string &str){

	str.clear();
	int handleSize;

	// for both cases we return an empty string
	if(strHandle == NULL || GetHandleSize(strHandle) == 0L){
		return;
	}
	handleSize = GetHandleSize(strHandle);

	try{
		str = std::string(*strHandle,handleSize);
	}
	catch(...){
		XOPNotice("Out of memory in convertHandleToString");
		str.clear();
	}
}