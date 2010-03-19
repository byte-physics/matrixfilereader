
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
		IntVoidMap::const_iterator it;
		for(it = m_brickletIDBrickletPointerMap.begin(); it != m_brickletIDBrickletPointerMap.end(); it++){
			m_VernissageSession->unloadBrickletContents(it->second);
		}

		m_dllStuff->closeSession();
		m_VernissageSession=NULL;

		// erase filenames
		m_resultFileName.erase();
		m_resultFilePath.erase();

		// empty maps
		m_brickletIDBrickletPointerMap.clear();
		m_brickletIDRawBufferMap.clear();
		m_brickletIDRawBufferLengthMap.clear();
	}
}

std::string myData::getVernissageVersion(){

	std::string version= "Unknown version";

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

void myData::getBrickletContentsBuffer(int brickletID, const int** pBuffer, int &count){

	char buf[ARRAY_SIZE];
	count=0;

	ASSERT_RETURN_VOID(pBuffer);
	ASSERT_RETURN_VOID(m_VernissageSession);

	void* pBricklet= getBrickletPointerFromMap(brickletID);
	ASSERT_RETURN_VOID(pBricklet);

	// we are not called the first time
	if(m_brickletIDRawBufferMap.find(brickletID) != m_brickletIDRawBufferMap.end()){
		debugOutputToHistory(DEBUG_LEVEL,"myData::getBrickletContentsBuffer Using cached values");

		sprintf(buf,"before: pBuffer=%d,count=%d",*pBuffer,count);
		debugOutputToHistory(DEBUG_LEVEL,buf);

		*pBuffer = m_brickletIDRawBufferMap[brickletID];
		count   = m_brickletIDRawBufferLengthMap[brickletID];

		sprintf(buf,"after: pBuffer=%d,count=%d",*pBuffer,count);
		debugOutputToHistory(DEBUG_LEVEL,buf);

		return;
	}
	else{ // we are called the first time
		m_VernissageSession->loadBrickletContents(pBricklet,pBuffer,count);

		sprintf(buf,"pBuffer=%d,count=%d",*pBuffer,count);
		debugOutputToHistory(DEBUG_LEVEL,buf);

		m_brickletIDRawBufferMap[brickletID] = *pBuffer;
		m_brickletIDRawBufferLengthMap[brickletID] = count;
	}
}

