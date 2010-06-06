
#ifndef DATACLASS_H
#define DATACLASS_H

#include <map>
#include <ctime>

#include "dll-stuff.h"

typedef	std::map<int,	 void*, std::less<int>>		IntVoidMap;
typedef std::map<int, const int*, std::less<int>> IntIntPtrMap;
typedef std::map<int, int, std::less<int>> IntIntMap;
typedef std::map<int, std::vector<std::string>, std::less<int>> IntStdVecMap;

class myData{

public:
	// constructors/deconstructors
	myData();
	~myData();
	
public:
	// API functions
	std::string getResultFileName();
	std::string getResultFilePath();
	void setResultFile( std::string filePath, std::string fileName){ m_resultFilePath = filePath; m_resultFileName = fileName;};

	// functions
	bool resultFileOpen();
	Vernissage::Session* getSession();
	std::string getVernissageVersion();
	void closeSession();
	int getDebugLevel(){ return  m_debugLevel; };
	void  setDebugLevel(int level){ m_debugLevel= level; }
	void* getBrickletPointerFromMap(int brickletID);
	void  setBrickletPointerMap(int brickletID, void *pBricklet);
	void  getBrickletContentsBuffer(int brickletID, const int** pBuffer, int &count);
	bool  gotCachedBrickletMetaData(int brickletID);
	void  storeBrickletMetaData(int brickletID, std::vector<std::string> &keys, std::vector<std::string> &values);
	void  loadCachedBrickletMetaData(int brickletID, std::vector<std::string> &keys, std::vector<std::string> &values);

public:

	//TimeStampIntMap  brickletTimeStampIDMap;
	//TimeStampVoidMap brickletTimeStampBrickletPointerMap;
	int m_debugLevel;

private:

	//variables
	std::string m_resultFileName, m_resultFilePath;
	Vernissage::Session *m_VernissageSession;
	DllStuff *m_dllStuff;

	IntVoidMap		m_brickletIDBrickletPointerMap;
	IntIntPtrMap	m_brickletIDRawBufferMap;
	IntIntMap		m_brickletIDRawBufferLengthMap;
	IntStdVecMap    m_brickletIDMetaDataKeysMap;
	IntStdVecMap    m_brickletIDMetaDataValuesMap;
};

#endif // DATACLASS_H