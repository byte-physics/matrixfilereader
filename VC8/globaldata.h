/*
	The file globaldata.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

/*
	Global and unique object which keeps track of the internal state
*/
#pragma once

#include "ForwardDecl.h"
#include "dllhandler.h"

class GlobalData{

	typedef	std::map<int, BrickletClass*, std::less<int>>		IntBrickletClassPtrMap;

public:
	// functions

	// return the filename and the dirPath of the currently loaded result set
	void setResultFile(const std::wstring& dirPath, const std::wstring& fileName);
	void closeSession();
	void closeResultFile();
	void createBrickletClassObject(int brickletID, void* const pBricklet);
	void setError(int errorCode, std::string msgArgument = std::string());
	void setInternalError(int errorCode);

	Vernissage::Session* getVernissageSession();
	std::string getVernissageVersion();

	void initializeWithoutReadSettings(int calledFromMacro,int calledFromFunction);
	void initialize(int calledFromMacro,int calledFromFunction);

	void finalize(bool filledCache = false, int errorCode = SUCCESS);

	void readSettings(DataFolderHandle dataFolderHndl = NULL);

	// variables
	char outputBuffer[ARRAY_SIZE];
	int  openDlgFileIndex;
	char openDlgInitialDir[MAX_PATH_LEN+1];

	// const functions
	std::wstring getFileNameWString() const;
	std::wstring getDirPathWString() const;
	std::string getFileName() const;
	std::string getDirPath() const;

	bool resultFileOpen() const;

	std::string getLastErrorMessage() const;
	std::string getErrorMessage (int errorCode) const;
	int getLastError() const{ return m_lastError; }

	BrickletClass* getBrickletClassObject(int brickletID) const;

	// debug
	bool debuggingEnabled() const{ return m_debug; };
	// double
	bool doubleWaveEnabled() const{ return m_doubleWave; };
	int getIgorWaveType() const;

	// datafolder
	bool datafolderEnabled() const{ return m_datafolder; };

	// overwrite
	bool overwriteEnabled() const{ return m_overwrite; };
	int overwriteEnabledAsInt() const{ return int(m_overwrite); };

	// cache
	bool dataCacheEnabled() const{ return m_datacache; };

	// Access to singleton-type global object
	static GlobalData& GlobalData::Instance(){
		static GlobalData globData;
		return globData;
	}

private:
	GlobalData(); // hide ctor
	~GlobalData(); // hide dtor
	GlobalData(const GlobalData&); // hide copy ctor
	GlobalData& operator=(const GlobalData&); // hide assignment operator

	bool m_debug, m_doubleWave, m_datafolder, m_overwrite, m_datacache;
	std::wstring m_resultFileName, m_resultDirPath;
	Vernissage::Session *m_VernissageSession;
	DLLHandler m_DLLHandler;
	bool m_errorToHistory;
	int m_lastError;
	std::string m_lastErrorArgument;
	IntBrickletClassPtrMap m_brickletIDBrickletClassMap;

	void setLastError(int errorCode, std::string argument = std::string());

	void enableDatafolder(bool var){ m_datafolder = var; };
	void enableDoubleWave(bool var){ m_doubleWave = var; };
	void enableDebugging(bool var){ m_debug=var; };
	void enableOverwrite(bool var){ m_overwrite = var; };
	void enableDataCaching(bool var){m_datacache = var; };
};
