
#ifndef DATACLASS_H
#define DATACLASS_H

#include <map>
#include <ctime>

#include "dll-stuff.h"

typedef	std::map<int,	 void*, std::less<int>>		IntVoidMap;
typedef std::map<int, const int*,std::less<int>> IntIntPtrMap;
typedef std::map<int, int,std::less<int>> IntIntMap;

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
	void setDebugLevel(int level){ m_debugLevel= level; }
	void* getBrickletPointerFromMap(int brickletID){ return m_brickletIDBrickletPointerMap[brickletID]; }
	void setBrickletPointerMap(int brickletID, void *pBricklet){ m_brickletIDBrickletPointerMap[brickletID] = pBricklet; }
	void getBrickletContentsBuffer(int brickletID, const int** pBuffer, int &count);

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

};

#endif // DATACLASS_H