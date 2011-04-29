/*
	The file globaldata.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "header.h"

#include "globaldata.h"

#include "dllhandler.h"

GlobalData *globDataPtr;

GlobalData::GlobalData(): m_VernissageSession(NULL),
	m_DLLHandler(NULL), m_lastError(UNKNOWN_ERROR),
	m_debug(debug_default),m_doubleWave(double_default),
	m_overwrite(overwrite_default),m_datafolder(datafolder_default),
	m_datacache(cache_default),m_errorToHistory(false){

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

void GlobalData::setResultFile(const std::wstring &dirPath, const std::wstring &fileName){
	
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
	IntBrickletClassPtrMap::iterator it;
	for(it = m_brickletIDBrickletClassMap.begin(); it != m_brickletIDBrickletClassMap.end(); it++){
		delete it->second;
		it->second = NULL;
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

void GlobalData::finalize(bool clearCache /* = false */, int errorCode /* = SUCCESS */ ){
	this->setError(errorCode);

	if(!dataCacheEnabled() && clearCache){
		BrickletClass *bricklet = NULL;
		const int totalBrickletCount = m_VernissageSession->getBrickletCount();
		for(int i=1; i <= totalBrickletCount; i++){
			bricklet = globDataPtr->getBrickletClassObject(i);
			ASSERT_RETURN_VOID(bricklet);
			bricklet->clearCache();
		}
	}
}

void GlobalData::initializeWithoutReadSettings(int calledFromMacro,int calledFromFunction){

	m_errorToHistory = false;	// otherwise the setError in the next line causes the error message to be printed
	this->setError(UNKNOWN_ERROR);
	m_errorToHistory = ( calledFromMacro == 0 && calledFromFunction == 0 );
}

void GlobalData::initialize(int calledFromMacro,int calledFromFunction){

	this->readSettings();
	this->initializeWithoutReadSettings(calledFromMacro,calledFromFunction);
}

void GlobalData::setError(int errorCode, std::string argument){

	if(errorCode < SUCCESS || errorCode > WAVE_EXIST){
		outputToHistory("BUG: errorCode is out of range");
		m_lastError = UNKNOWN_ERROR;
		return;
	}

	m_lastError = errorCode;
	SetOperationNumVar(V_flag,static_cast<double>(errorCode));

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
		msg = "No error, everything went nice and smooth.";
		break;
	case UNKNOWN_ERROR:
		msg = "A strange and unknown error happened. It might be appropriate to file a bug report at " + std::string(PROJECTURL) + ".";
		break;
	case ALREADY_FILE_OPEN:
		msg = "A file is already file open and it can only be one file open at a time.";
		break;
	case EMPTY_RESULTFILE:
		msg = "The result file is empty, so there is little one can do here...";
		break;
	case FILE_NOT_READABLE:
		msg = "The file/folder " + this->getLastErrorArgument() + " is not readable.";
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
	case WAVE_EXIST:
		msg = "The wave " + this->getLastErrorArgument() + " already exists. Please move/delete it first.";
		break;
	default:
		msg = "BUG: unknown error code";
		break;
	}

	return msg;
}

// Reads settings, see *_option_name in the data folder dataFolderHndl. In case dataFolderHndl is
// null the current data folder is used.
void GlobalData::readSettings(DataFolderHandle dataFolderHndl /* = NULL */){

	int ret, objType;
	bool setting;

	char dataFolderPath[MAXCMDLEN+1], buf[ARRAY_SIZE];
	DataObjectValue objValue;

	// debug setting
	ret = GetDataFolderObject(dataFolderHndl,debug_option_name,&objType,&objValue);
	if(ret != 0 || objType != VAR_OBJECT){// variable does not exist or is of wrong type
		enableDebugging(debug_default);
		sprintf(globDataPtr->outputBuffer,"debug=%d (default)",debug_default);
	}
	else{
		setting = doubleToBool(objValue.nv.realValue);
		enableDebugging(setting);
		sprintf(globDataPtr->outputBuffer,"debug=%d",setting);
	}
	if(debuggingEnabled()){
		// flags=3 returns the full path to the datafolder and including quotes if needed
		ret = GetDataFolderNameOrPath(dataFolderHndl, 3,dataFolderPath);
		if(ret == 0){
			// the additional buf array is needed here because globDataPtr->outputBuffer is still occupied
			sprintf(buf,"V_MatrixFileReader* variables in the folder %s:",dataFolderPath);
			debugOutputToHistory(buf);
		}
		debugOutputToHistory(globDataPtr->outputBuffer);
	}

	//overwrite setting
	ret = GetDataFolderObject(dataFolderHndl,overwrite_option_name,&objType,&objValue);
	if(ret != 0 || objType != VAR_OBJECT){// variable does not exist or is of wrong type
		enableOverwrite(overwrite_default);
		sprintf(globDataPtr->outputBuffer,"overwrite=%d (default)",overwrite_default);
	}
	else{
		setting = doubleToBool(objValue.nv.realValue);
		enableOverwrite(setting);
		sprintf(globDataPtr->outputBuffer,"overwrite=%d",setting);
	}
	debugOutputToHistory(globDataPtr->outputBuffer);

	// double setting
	ret = GetDataFolderObject(dataFolderHndl,double_option_name,&objType,&objValue);
	if(ret != 0 || objType != VAR_OBJECT){// variable does not exist or is of wrong type
		enableDoubleWave(double_default);
		sprintf(globDataPtr->outputBuffer,"double=%d (default)",double_default);
	}
	else{
		setting = doubleToBool(objValue.nv.realValue);
		enableDoubleWave(setting);
		sprintf(globDataPtr->outputBuffer,"double=%d",setting);
	}
	debugOutputToHistory(globDataPtr->outputBuffer);

	// folder setting
	ret = GetDataFolderObject(dataFolderHndl,datafolder_option_name,&objType,&objValue);
	if(ret != 0 || objType != VAR_OBJECT){// variable does not exist or is of wrong type
		enableDatafolder(datafolder_default);
		sprintf(globDataPtr->outputBuffer,"datafolder=%d (default)",datafolder_default);
	}
	else{
		setting = doubleToBool(objValue.nv.realValue);
		enableDatafolder(setting);
		sprintf(globDataPtr->outputBuffer,"datafolder=%d",setting);
	}
	debugOutputToHistory(globDataPtr->outputBuffer);

	// chache setting
	ret = GetDataFolderObject(dataFolderHndl,cache_option_name,&objType,&objValue);
	if(ret != 0 || objType != VAR_OBJECT){// variable does not exist or is of wrong type
		enableDataCaching(cache_default);
		sprintf(globDataPtr->outputBuffer,"cache=%d (default)",cache_default);
	}
	else{
		setting = doubleToBool(objValue.nv.realValue);
		enableDataCaching(setting);
		sprintf(globDataPtr->outputBuffer,"cache=%d",setting);
	}
	debugOutputToHistory(globDataPtr->outputBuffer);

	return;
}
