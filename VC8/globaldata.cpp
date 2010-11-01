#include "header.h"

#include "globaldata.h"
#include "dllhandler.h"
#include "utils.h"

GlobalData *globDataPtr;

GlobalData::GlobalData(): m_VernissageSession(NULL), m_DLLHandler(NULL), m_lastError(UNKNOWN_ERROR),m_debug(debug_default),m_doubleWave(double_default), m_overwrite(overwrite_default),m_datafolder(datafolder_default),m_errorToHistory(false){

	try{
		m_DLLHandler = new DLLHandler;
	}
	catch( CMemoryException* e ){
		XOPNotice("Out of memory in GlobalData constructor");
		e->Delete();
		return;
	}
}

GlobalData::~GlobalData(){

	delete m_DLLHandler;
	m_DLLHandler = NULL;
}

void GlobalData::setResultFile(std::wstring dirPath, std::wstring fileName){
	
	if(this->resultFileOpen()){
		outputToHistory("BUG: there is already a result file open, please close that first");
		return;
	}
	
	m_resultDirPath = dirPath;
	m_resultFileName = fileName;
}

Vernissage::Session* GlobalData::getVernissageSession(){

	if(m_VernissageSession != NULL){
		return m_VernissageSession;
	}
	else{
		m_VernissageSession = m_DLLHandler->createSessionObject();
		return m_VernissageSession;
	}
}

void GlobalData::closeResultFile(){

	//delete BrickletClass objects
	IntBrickletClassPtrMap::const_iterator it;
	for(it = m_brickletIDBrickletClassMap.begin(); it != m_brickletIDBrickletClassMap.end(); it++){
		delete it->second;
	}
	// empty bricklet map
	m_brickletIDBrickletClassMap.clear();

	// remove opened result set from internal database
	if(m_VernissageSession){
		m_VernissageSession->eraseResultSets();
	}

	// erase filenames
	m_resultFileName.erase();
	m_resultDirPath.erase();
}

void GlobalData::closeSession(){
	
	this->closeResultFile();
	m_DLLHandler->closeSession();
	m_VernissageSession = NULL;
}

std::string GlobalData::getVernissageVersion(){

	if(m_VernissageSession == NULL){
		this->getVernissageSession();
	}

	if(m_DLLHandler != NULL){
		return m_DLLHandler->getVernissageVersion();
	}
	else
		return std::string();
}

bool GlobalData::resultFileOpen(){

	return !m_resultFileName.empty();
}

std::string GlobalData::getDirPath(){

	return WStringToString(m_resultDirPath);
}
std::string GlobalData::getFileName(){

	return WStringToString(m_resultFileName);
}

std::wstring GlobalData::getDirPathWString(){

	return m_resultDirPath;
}
std::wstring GlobalData::getFileNameWString(){

	return m_resultFileName;
}


int GlobalData::getIgorWaveType(){

	int waveType;

	if(m_doubleWave){
		waveType = NT_FP64;
	}
	else{
		waveType = NT_FP32;
	}
	return waveType;
}


BrickletClass* GlobalData::getBrickletClassObject(int brickletID){

	IntBrickletClassPtrMap::iterator it = m_brickletIDBrickletClassMap.find(brickletID);

	if(it != m_brickletIDBrickletClassMap.end()){ // we found the element
		return it->second;
	}
	else{
		return NULL;
	}
}

void GlobalData::createBrickletClassObject(int brickletID, void *pBricklet){
	
	BrickletClass *bricklet = NULL;
	try{
		bricklet = new BrickletClass(pBricklet,brickletID);
	}
	catch(CMemoryException *e){
		XOPNotice("Out of memory in createBrickletClassObject\r");
		throw e;
	}

	m_brickletIDBrickletClassMap[brickletID] = bricklet;

	sprintf(globDataPtr->outputBuffer,"setBrickletPointerMap brickletID=%d,pBricklet=%p",brickletID,pBricklet);
	debugOutputToHistory(globDataPtr->outputBuffer);
}

void GlobalData::setInternalError(int errorValue){

	char errorMessage[256];
	int ret;

	sprintf(globDataPtr->outputBuffer,"BUG: xop internal error %d returned.",errorValue);
	debugOutputToHistory(globDataPtr->outputBuffer);

	ret = GetIgorErrorMessage(errorValue,errorMessage);
	if(ret == 0){
		outputToHistory(errorMessage);		
	}
}

void GlobalData::finalize(){
	this->setError(SUCCESS);

	if(dataCacheEnabled()){
		return;
	}

	BrickletClass *bricklet = NULL;
	for(int i=0; i <= m_VernissageSession->getBrickletCount(); i++){

		bricklet = globDataPtr->getBrickletClassObject(i);
		ASSERT_RETURN_VOID(bricklet);
		bricklet->clearCache();
	}
}

void GlobalData::initialize(int calledFromMacro,int calledFromFunction){

	this->readSettings();
	m_errorToHistory = false;	// otherwise the setError in the next line causes the error message to be printout
	this->setError(UNKNOWN_ERROR);
	m_errorToHistory = ( calledFromMacro == 0 && calledFromFunction == 0 );
}

void GlobalData::setError(int errorCode, std::string argument){

	if(errorCode < SUCCESS || errorCode > WAVE_EXIST){
		outputToHistory("BUG: errorCode is out of range");
		m_lastError = UNKNOWN_ERROR;
		return;
	}

	m_lastError = errorCode;
	SetOperationNumVar(V_flag,double(errorCode));

	if(argument.size() == 0){
		m_lastErrorArgument = "(missing argument)";
	}
	else{
		m_lastErrorArgument = argument;
	}
	sprintf(globDataPtr->outputBuffer,"lastErrorCode %d, argument %s", errorCode, argument.c_str());
	debugOutputToHistory(globDataPtr->outputBuffer);
	
	if(m_errorToHistory && errorCode != SUCCESS){
		outputToHistory(getLastErrorMessage().c_str());
	}
}

std::string GlobalData::getLastErrorMessage(){
	return getErrorMessage(getLastError());
}

std::string GlobalData::getErrorMessage(int errorCode){

	std::string msg;

	switch(errorCode){

	case SUCCESS:
		msg = "No error, everything went nice and smoothly.";
		break;
	case UNKNOWN_ERROR:
		msg = "A strange and unknown error happened. It might be appropriate to file a bug report.";
		break;
	case ALREADY_FILE_OPEN:
		msg = "A file is already file open and it can only be one file open at a time.";
		break;
	case EMPTY_RESULTFILE:
		msg = "The result file is empty, so there is little one can do here...";
		break;
	case FILE_NOT_READABLE:
		msg = "The file " + this->getLastErrorArgument() + " can not be opened.";
		break;
	case NO_NEW_BRICKLETS:
		msg = "There are no new bricklets in the result file.";
		break;
	case WRONG_PARAMETER:
		msg = "The paramter " + this->getLastErrorArgument() + " is missing or wrong. Please consult the documentation.";
		break;
	case INTERNAL_ERROR_CONVERTING_DATA:
		msg = "The rawdata could not be interpreted. You can try using getRawBrickleData() instead and consulting the vernissage documentation. Please file also a bug report and attach your data.";
		break;
	case NO_FILE_OPEN:
		msg = "There is no result file open.";
		break;
	case INVALID_RANGE:
		msg = "The brickletID range was wrong. brickletIDs have to lie between 1 and numberOfBricklets, and startBrickletID may not be bigger than endBrickletID.";
		break;
	case NON_EXISTENT_BRICKLET:
		msg = "The bricklet number " + this->getLastErrorArgument() + " does not exist.";
		break;
	case WAVE_EXIST:
		msg = "The wave " + this->getLastErrorArgument() + " already exists. Please move/delete it first.";
		break;
	default:
		msg = "BUG: unknown error code";
		break;
	}

	return msg;
}

void GlobalData::readSettings(){

	double realPart, complexPart;
	int ret;
	bool setting,debugEnabled=false;

	// we need to check that here to get consistent output, changing debugging with these settings otherwise interferes with the output
	if(debuggingEnabled()){
		debugEnabled = true;
		outputToHistory("DEBUG: Various Settings");
	}

	// overwrite setting
	ret = FetchNumVar(overwrite_option_name,&realPart,&complexPart);
	if(ret == -1){// variable does not exist
		enableOverwrite(overwrite_default);
		sprintf(globDataPtr->outputBuffer,"DEBUG: overwrite=%d (default)",overwrite_default);
	}
	else{
		setting = doubleToBool(realPart);
		enableOverwrite(setting);
		sprintf(globDataPtr->outputBuffer,"DEBUG: overwrite=%d",setting);
	}
	if(debugEnabled){
		outputToHistory(globDataPtr->outputBuffer);
	}

	// debug setting
	ret = FetchNumVar(debug_option_name,&realPart,&complexPart);
	if(ret == -1){// variable does not exist
		enableDebugging(debug_default);
		sprintf(globDataPtr->outputBuffer,"DEBUG: debug=%d (default)",debug_default);
	}
	else{
		setting = doubleToBool(realPart);
		enableDebugging(setting);
		sprintf(globDataPtr->outputBuffer,"DEBUG: debug=%d",setting);
	}
	if(debugEnabled){
		outputToHistory(globDataPtr->outputBuffer);
	}

	// double setting
	ret = FetchNumVar(double_option_name,&realPart,&complexPart);
	if(ret == -1){// variable does not exist
		enableDoubleWave(double_default);
		sprintf(globDataPtr->outputBuffer,"DEBUG: double=%d (default)",double_default);
	}
	else{
		setting = doubleToBool(realPart);
		enableDoubleWave(setting);
		sprintf(globDataPtr->outputBuffer,"DEBUG: double=%d",setting);
	}
	if(debugEnabled){
		outputToHistory(globDataPtr->outputBuffer);
	}

	// folder setting
	ret = FetchNumVar(datafolder_option_name,&realPart,&complexPart);
	if(ret == -1){// variable does not exist
		enableDatafolder(datafolder_default);
		sprintf(globDataPtr->outputBuffer,"DEBUG: datafolder=%d (default)",datafolder_default);
	}
	else{
		setting = doubleToBool(realPart);
		enableDatafolder(setting);
		sprintf(globDataPtr->outputBuffer,"DEBUG: datafolder=%d",setting);
	}
	if(debugEnabled){
		outputToHistory(globDataPtr->outputBuffer);
	}

	// cache setting
	ret = FetchNumVar(cache_option_name,&realPart,&complexPart);
	if(ret == -1){// variable does not exist
		enableDataCaching(cache_default);
		sprintf(globDataPtr->outputBuffer,"DEBUG: cache=%d (default)",cache_default);
	}
	else{
		setting = doubleToBool(realPart);
		enableDataCaching(setting);
		sprintf(globDataPtr->outputBuffer,"DEBUG: cache=%d",setting);
	}
	if(debugEnabled){
		outputToHistory(globDataPtr->outputBuffer);
	}
}