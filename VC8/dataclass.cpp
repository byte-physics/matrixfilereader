
#include "xopstandardheaders.h"

#include "stdlib.h"

#include "vernissage.h"

#include "dataclass.h"
#include "utils.h"

#include "globalvariables.h"

#define DEBUG_LEVEL 1

myData::myData():
	m_VernissageSession(NULL),
	m_debugLevel(0){

	m_dllStuff = new DllStuff;

}

myData::~myData(){

	delete m_dllStuff;
	m_dllStuff = NULL;
}

Vernissage::Session* myData::getSession(){

	if(m_VernissageSession != NULL){
		return m_VernissageSession;
	}
	else{
		m_VernissageSession = m_dllStuff->createSessionObject();
		return m_VernissageSession;
	}
}

void myData::closeSession(){

	if(m_VernissageSession != NULL){

		//unload bricklets
		IntMyBrickletPtrMap::const_iterator it;
		for(it = m_brickletIDBrickletClassMap.begin(); it != m_brickletIDBrickletClassMap.end(); it++){
			delete it->second;
		}

		m_dllStuff->closeSession();
		m_VernissageSession=NULL;

		// erase filenames
		m_resultFileName.erase();
		m_resultFilePath.erase();

		// empty maps
		m_brickletIDBrickletClassMap.clear();
	}
}

std::string myData::getVernissageVersion(){

	std::string version= "unknown";

	if(m_VernissageSession != NULL){
	
		version = m_dllStuff->getVernissageVersion();
	}

	return version;
}

bool myData::resultFileOpen(){

	if(m_resultFilePath.empty()){
		return false;
	}
	else{
		return true;
	}
}

std::string myData::getResultFilePath(){

	return m_resultFilePath;

}

std::string myData::getResultFileName(){

	return m_resultFileName;
}

MyBricklet* myData::getBrickletClassFromMap(int brickletID){

	return m_brickletIDBrickletClassMap[brickletID];
}

void myData::setBrickletClassMap(int brickletID, void *pBricklet){
	
/*	char buf[ARRAY_SIZE];
	sprintf(buf,"setBrickletPointerMap brickletID=%d,pBricklet=%p\n",brickletID,pBricklet);
	debugOutputToHistory(DEBUG_LEVEL,buf);
*/
	MyBricklet *bricklet = new MyBricklet(pBricklet,m_VernissageSession);
	m_brickletIDBrickletClassMap[brickletID] = bricklet;
}

void myData::setWaveNote(int brickletID,waveHndl waveHandle){

	std::string waveNote;
	char buf[ARRAY_SIZE];

	waveNote.append("resultFileName=" + getResultFileName() + "\r");
	waveNote.append("resultFilePath=" + getResultFilePath() + "\r");

	// we pass brickletID=0 for waveNotes concerning the resultFileMetaData wave
	if(brickletID > 0){
		waveNote.append("brickletID=" + anyTypeToString<int>(brickletID) + "\r");
	}
	else{
		waveNote.append("brickletID=\r");
	}

	waveNote.append("xopVersion=" + std::string(myXopVersion) + "\r");
	waveNote.append("vernissageVersion=" + this->getVernissageVersion());

	Handle noteHandle  = NewHandle(waveNote.size()) ;
	PutCStringInHandle(waveNote.c_str(),noteHandle);

	SetWaveNote(waveHandle, noteHandle);
}