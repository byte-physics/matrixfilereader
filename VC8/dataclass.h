
#ifndef DATACLASS_H
#define DATACLASS_H

#include <map>
#include <ctime>

#include "dll-stuff.h"
#include "myBricklet.h"
#include "xopstandardheaders.h"

typedef	std::map<int, MyBricklet*, std::less<int>>		IntMyBrickletPtrMap;

class myData{

public:
	// constructors/deconstructors
	myData();
	~myData();
	
public:
	// functions

	// return the filename and the dirPath of the currently loaded result set
	std::string getFileName();
	std::string getDirPath();
	void setResultFile(char *dirPath, char *fileName){ this->setResultFile(std::string(dirPath),std::string(fileName));}
	void setResultFile( std::string dirPath, std::string fileName){ m_resultFilePath = dirPath; m_resultFileName = fileName;}
	bool resultFileOpen();
	Vernissage::Session* getVernissageSession();
	std::string getVernissageVersion();
	void closeSession();
	MyBricklet* getMyBrickletObject(int brickletID);
	void createMyBrickletObject(int brickletID, void *pBricklet);
	void setLastError(int errorCode, std::string argument = std::string());
	int getLastError(){ return m_lastError; }
	std::string getLastErrorArgument(){ return m_lastErrorArgument; }
	std::string getLastErrorMessage();

private:
	std::string m_resultFileName, m_resultFilePath;
	Vernissage::Session *m_VernissageSession;
	DllStuff *m_dllStuff;
	int m_lastError;
	std::string m_lastErrorArgument;
	IntMyBrickletPtrMap		m_brickletIDBrickletClassMap;
};

#endif // DATACLASS_H