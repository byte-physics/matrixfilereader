/*
  The file globaldata.h is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

/*
  Global and unique object which keeps track of the internal state
*/
#pragma once

#include "ForwardDecl.hpp"
#include "dllhandler.hpp"

class GlobalData
{
  typedef  std::map<int, BrickletClass*, std::less<int>>    IntBrickletClassPtrMap;

public:
  void setResultFile(const std::wstring& dirPath, const std::wstring& fileName);
  void closeSession();
  void closeResultFile();
  void createBrickletClassObject(int brickletID, void* const pBricklet);
  void setError(int errorCode, const std::string& msgArgument = std::string());
  void setInternalError(int errorCode);

  Vernissage::Session* getVernissageSession();
  const std::string& getVernissageVersion();

  void initializeWithoutReadSettings(int calledFromMacro, int calledFromFunction);
  void initialize(int calledFromMacro, int calledFromFunction);

  void finalize(bool filledCache = false, int errorCode = SUCCESS);

  template<typename T>
  T getFileName() const;

  template<typename T>
  T getDirPath() const;

  bool resultFileOpen() const;

  std::string getLastErrorMessage() const;
  std::string getErrorMessage(int errorCode) const;

  BrickletClass* getBrickletClassObject(int brickletID) const;

  std::vector<int> convertBrickletPtr(const std::vector<void*>&);
  int convertBrickletPtr(void*);

  std::vector<void*> getBrickletSeries(void* rawBrickletPtr);

  int getIgorWaveType() const;

  ///@name Settings handling
  ///@{
  bool isDebuggingEnabled() const;
  bool isDoubleWaveEnabled() const;
  bool isDatafolderEnabled() const;
  bool isDataCacheEnabled() const;

  template<typename T>
  T isOverwriteEnabled() const
  {
    return static_cast<T>(m_overwrite);
  }

  void readSettings(DataFolderHandle dataFolderHndl = NULL);
  ///@}

  /// Access to singleton-type global object
  static GlobalData& GlobalData::Instance()
  {
    static GlobalData globData;
    return globData;
  }

  // Public member variables
  char outputBuffer[ARRAY_SIZE];
  int  openDlgFileIndex;
  char openDlgInitialDir[MAX_PATH_LEN + 1];

private:
  GlobalData(); // hide ctor
  ~GlobalData(); // hide dtor
  GlobalData(const GlobalData&); // hide copy ctor
  GlobalData& operator=(const GlobalData&); // hide assignment operator

  bool m_debug, m_doubleWave, m_datafolder, m_overwrite, m_datacache;
  std::wstring m_resultFileName, m_resultDirPath;
  Vernissage::Session* m_VernissageSession;
  DLLHandler m_DLLHandler;
  bool m_errorToHistory;
  int m_lastError;
  std::string m_lastErrorArgument;
  IntBrickletClassPtrMap m_brickletIDBrickletClassMap;
};
