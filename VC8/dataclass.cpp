
#include "stdlib.h"

#include "vernissage.h"

#include "dataclass.h"

#include "globalvariables.h"

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
		m_dllStuff->closeSession();
		m_VernissageSession=NULL;
		m_resultFileName.erase();
		m_resultFilePath.erase();
	}
}

bool myData::resultFileOpen(){

	if(m_resultFilePath.empty()){
		return false;
	}
	else{
		return true;
	}
}

std::wstring myData::getResultFilePath(){

	return m_resultFilePath;

}

std::wstring myData::getResultFileName(){

	return m_resultFileName;
}

	//wchar_t drive[_MAX_DRIVE];
	//wchar_t dir[_MAX_DIR];
	//wchar_t fname[_MAX_FNAME];
	//wchar_t ext[_MAX_EXT];

	//int ret = _wsplitpath_s(m_resultFileAbsolutePath.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

	//if(ret != 0){ // something went wrong
	//	return std::wstring();
	//}

	//std::wstring filenameWithExtension(fname);
	//filenameWithExtension.append(ext);

	//return filenameWithExtension;
