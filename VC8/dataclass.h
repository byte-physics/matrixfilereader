#pragma once

#include <map>

#include "dll-stuff.h"
#include "globals.h"
#include "myBricklet.h"

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
	int getIgorWaveType();

	// settings
	void setOption(std::string key, int value);
	// debug
	bool debuggingEnabled(){ return m_debug; };
	void enableDebugging(bool var){ m_debug=var; };
	
	// double
	bool doubleWaveEnabled(){ return m_doubleWave; };
	void enableDoubleWave(bool var){ m_doubleWave = var; };

	// datafolder
	bool datafolderEnabled(){ return m_datafolder; };
	void enableDatafolder(bool var){ m_datafolder = var; };

	// overwrite
	bool overwriteEnabled(){ return m_overwrite; };
	int overwriteEnabledAsInt(){ return int(m_overwrite); };
	void enableOverwrite(bool var){ m_overwrite = var; };

	// variables
	char outputBuffer[ARRAY_SIZE];

private:

	bool m_debug,m_doubleWave, m_datafolder, m_overwrite;
	std::string m_resultFileName, m_resultFilePath;
	Vernissage::Session *m_VernissageSession;
	DllStuff *m_dllStuff;
	int m_lastError;
	std::string m_lastErrorArgument;
	IntMyBrickletPtrMap		m_brickletIDBrickletClassMap;
};

// declare global object pMyData
extern myData *pMyData;