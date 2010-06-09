
#ifndef DATACLASS_H
#define DATACLASS_H

#include <map>
#include <ctime>

#include "dll-stuff.h"
#include "myBricklet.h"
#include "xopstandardheaders.h"

typedef	std::map<int,	 MyBricklet*, std::less<int>>		IntMyBrickletPtrMap;
//typedef std::map<int, const int*, std::less<int>> IntIntPtrMap;
//typedef std::map<int, int, std::less<int>> IntIntMap;
//typedef std::map<int, std::vector<std::string>, std::less<int>> IntStdVecMap;

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
	MyBricklet* getBrickletClassFromMap(int brickletID);
	void setBrickletClassMap(int brickletID, void *pBricklet);
	void setWaveNote(int brickletID,waveHndl waveHandle);

public:


private:

	//variables
	std::string m_resultFileName, m_resultFilePath;
	Vernissage::Session *m_VernissageSession;
	DllStuff *m_dllStuff;
	int m_debugLevel;

	IntMyBrickletPtrMap		m_brickletIDBrickletClassMap;
	//IntIntPtrMap			m_brickletIDRawBufferMap;
	//IntIntMap				m_brickletIDRawBufferLengthMap;
	//IntStdVecMap			m_brickletIDMetaDataKeysMap;
	//IntStdVecMap			m_brickletIDMetaDataValuesMap;
};

#endif // DATACLASS_H