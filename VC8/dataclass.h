
#ifndef DATACLASS_H
#define DATACLASS_H

#include <map>
#include <ctime>

#include "dll-stuff.h"

typedef	std::map<time_t,   int, std::less<time_t>>  TimeStampIntMap;
typedef	std::map<int,	 void*, std::less<int>>		IntVoidMap;
typedef	std::map<time_t, void*, std::less<time_t>>  TimeStampVoidMap;

class myData{

public:
	// constructors/deconstructors
	myData();
	~myData();
	
public:
	// API functions
	std::wstring getResultFileName();
	std::wstring getResultFilePath();
	void setResultFile( std::wstring filePath, std::wstring fileName){ m_resultFilePath = filePath; m_resultFileName = fileName;};

	// functions
	bool resultFileOpen();
	Vernissage::Session* getSession();
	void closeSession();
	int getDebugLevel(){ return  m_debugLevel; };
	void setDebugLevel(int level){ m_debugLevel= level; }


public:
	// variables
	std::wstring xopVersion;
	std::wstring vernissageVersion;

	TimeStampIntMap  brickletTimeStampIDMap;
	TimeStampVoidMap brickletTimeStampBrickletPointerMap;
	IntVoidMap		 brickletIDBrickletPointerMap;

	int m_debugLevel;

private:

	//variables
	std::wstring m_resultFileName, m_resultFilePath;
	Vernissage::Session *m_VernissageSession;
	DllStuff *m_dllStuff;

};

#endif // DATACLASS_H