
#include "dataclass.h"

#include "xopstandardheaders.h"


#include "vernissage.h"

#include "globals.h"
#include "utils.h"



myData::myData():
	m_VernissageSession(NULL),
	m_dllStuff(NULL),
	m_lastError(UNKNOWN_ERROR){

	m_dllStuff = new DllStuff;
}

myData::~myData(){

	delete m_dllStuff;
	m_dllStuff = NULL;
}

Vernissage::Session* myData::getVernissageSession(){

	if(m_VernissageSession != NULL){
		return m_VernissageSession;
	}
	else{
		m_VernissageSession = m_dllStuff->createSessionObject();
		return m_VernissageSession;
	}
}

void myData::closeSession(){

	//unload bricklets
	IntMyBrickletPtrMap::const_iterator it;
	for(it = m_brickletIDBrickletClassMap.begin(); it != m_brickletIDBrickletClassMap.end(); it++){
		delete it->second;
	}

	// remove opened result set from internal database
	if(m_VernissageSession){
		m_VernissageSession->eraseResultSets();
	}

	// empty bricklet map
	m_brickletIDBrickletClassMap.clear();

	// erase filenames
	m_resultFileName.erase();
	m_resultFilePath.erase();

}

std::string myData::getVernissageVersion(){

	if(m_VernissageSession == NULL){
		this->getVernissageSession();
	}

	return m_dllStuff->getVernissageVersion();
}

bool myData::resultFileOpen(){

	if(m_resultFileName.empty()){
		return false;
	}
	else{
		return true;
	}
}

std::string myData::getDirPath(){

	return m_resultFilePath;

}

std::string myData::getFileName(){

	return m_resultFileName;
}

MyBricklet* myData::getMyBrickletObject(int brickletID){

	IntMyBrickletPtrMap::iterator it = m_brickletIDBrickletClassMap.find(brickletID);

	if(it != m_brickletIDBrickletClassMap.end()){ // we found the element
		return it->second;
	}
	else{
		return NULL;
	}
}

void myData::createMyBrickletObject(int brickletID, void *pBricklet){
	
/*	char buf[ARRAY_SIZE];
	sprintf(buf,"setBrickletPointerMap brickletID=%d,pBricklet=%p\n",brickletID,pBricklet);
	debugOutputToHistory(buf);
*/
	MyBricklet *bricklet = new MyBricklet(pBricklet,brickletID);
	m_brickletIDBrickletClassMap[brickletID] = bricklet;
}

void myData::setLastError(int errorCode, std::string argument){
	
	char buf[ARRAY_SIZE];

	if(errorCode < SUCCESS || errorCode > WAVE_EXIST){
		outputToHistory("BUG: errorCode is out of range");
		m_lastError = UNKNOWN_ERROR;
		return;
	}

	m_lastError = errorCode;
	if(argument.size() == 0){
		m_lastErrorArgument = "(missing argument)";
	}
	else{
		m_lastErrorArgument = argument;
	}
	sprintf(buf,"lastErrorCode %d, argument %s", errorCode, argument.c_str());
	debugOutputToHistory(buf);
}

std::string myData::getLastErrorMessage(){

	std::string msg;

	switch(m_lastError){

	case SUCCESS:
		msg = "No error, everything went nice and smoothly.";
		break;
	case UNKNOWN_ERROR:
		msg = "A strange and unknown error happened.";
		break;
	case ALREADY_FILE_OPEN:
		msg = "It is already a file open and it can only be one file open at a time.";
		break;
	case EMPTY_RESULTFILE:
		msg = "The result file is empty, so there is little one can do here...";
		break;
	case FILE_NOT_READABLE:
		msg = "The file " + this->getLastErrorArgument() + " can not be opened for reading.";
		break;
	case NO_NEW_BRICKLETS:
		msg = "There are no new bricklets in the result file.";
		break;
	case WRONG_PARAMETER:
		msg = "The paramter " + this->getLastErrorArgument() + " is wrong. Please consult the documentation.";
		break;
	case INTERNAL_ERROR_CONVERTING_DATA:
		msg = "The rawdata could not be interpreted. You can try using getRawBrickleData() instead. Please file also a bug report.";
		break;
	case NO_FILE_OPEN:
		msg = "There is no result file open at the moment.";
		break;
	case INVALID_RANGE:
		msg = "The brickletID range was wrong. brickletIDs have to lie between 1 and numberOfBricklets, and startBrickletID must be smaller than endBrickletID.";
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