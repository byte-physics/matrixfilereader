
#ifndef DATACLASS_H
#define DATACLASS_H

#include <map>
#include <ctime>

#include "dll-stuff.h"
#include "myBricklet.h"
#include "xopstandardheaders.h"

typedef	std::map<int,	 MyBricklet*, std::less<int>>		IntMyBrickletPtrMap;

class myData{

public:
	// constructors/deconstructors
	myData();
	~myData();
	
public:
	// API functions
	std::string getResultFileName();
	std::string getResultFilePath();
	void setResultFile( std::string filePath, std::string fileName){ m_resultFilePath = filePath; m_resultFileName = fileName;}

	// functions
	bool resultFileOpen();
	Vernissage::Session* getVernissageSession();
	std::string getVernissageVersion();
	void closeSession();
	int getDebugLevel(){ return  m_debugLevel; };
	void  setDebugLevel(int level){ m_debugLevel= level; }
	MyBricklet* getBrickletClassFromMap(int brickletID);
	void setBrickletClassMap(int brickletID, void *pBricklet);
	void setDataWaveNote(int brickletID, int rawMin, int rawMax, double scaledMin, double scaledMax, waveHndl waveHandle);
	void setOtherWaveNote(int brickletID,waveHndl waveHandle);

private:
	
	void setWaveNote(std::string waveNote, waveHndl waveHandle);
	std::string getStandardWaveNote(int brickletID);


//variables

public:


private:
	std::string m_resultFileName, m_resultFilePath;
	Vernissage::Session *m_VernissageSession;
	DllStuff *m_dllStuff;
	int m_debugLevel;

	IntMyBrickletPtrMap		m_brickletIDBrickletClassMap;
};

#endif // DATACLASS_H