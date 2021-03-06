/*
  The file globaldata.h is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

/*
  Global and unique object which keeps track of the internal state
*/
#pragma once

#include <string>
#include <vector>

#include "errorcodes.hpp"
#include "constants.hpp"
#include "ForwardDecl.hpp"
#include "dllhandler.hpp"
#include "utils_generic.hpp"

typedef std::vector<BrickletPtr> BrickletPtrVector;
typedef std::map<void *, int> RawBrickletToIDMap;

class GlobalData
{
public:
  void setResultFile(const std::wstring &dirPath, const std::wstring &fileName);
  void closeSession();
  void closeResultFile();

  void createBricklet(int brickletID, void *const vernissageBricklet);
  void updateBricklet(int brickletID, void *const vernissageBricklet);

  void setError(int errorCode, const std::string &msgArgument = std::string());
  void setInternalError(int errorCode);

  Vernissage::Session *getVernissageSession();
  const std::string &getVernissageVersion();

  void initializeWithoutReadSettings(int calledFromMacro, int calledFromFunction);
  void initialize(int calledFromMacro, int calledFromFunction);

  void finalize();
  void finalizeWithFilledCache();

  const std::wstring &getFileName() const;
  const std::wstring &getDirPath() const;

  bool resultFileOpen() const;

  std::string getLastErrorMessage() const;
  std::string getErrorMessage(int errorCode) const;

  Bricklet &getBricklet(int brickletID);
  int convertBrickletPtr(void *rawBrickletPtr) const;

  ///@name Settings handling
  ///@{
  bool isDebuggingEnabled() const;
  bool isDoubleWaveEnabled() const;
  bool isDatafolderEnabled() const;
  bool isDataCacheEnabled() const;
  bool isOverwriteEnabled() const;
  int magicSetting() const;

  void readSettings(DataFolderHandle dataFolderHndl = NULL);
  ///@}

  /// Access to singleton-type global object
  static GlobalData &GlobalData::Instance()
  {
    static GlobalData globData;
    return globData;
  }

  std::size_t getUsedMemory() const;

  // Public member variables
  char outputBuffer[ARRAY_SIZE];
  int openDlgFileIndex;
  char openDlgInitialDir[MAX_PATH_LEN + 1];

private:
  GlobalData();                              // hide ctor
  ~GlobalData();                             // hide dtor
  GlobalData(const GlobalData &);            // hide copy ctor
  GlobalData &operator=(const GlobalData &); // hide assignment operator
  bool m_debug, m_doubleWave, m_datafolder, m_overwrite, m_datacache;
  int m_magic;
  std::wstring m_resultFileName, m_resultDirPath;
  Vernissage::Session *m_VernissageSession;
  DLLHandler m_DLLHandler;
  bool m_errorToHistory;
  int m_lastError;
  std::string m_lastErrorArgument;
  BrickletPtrVector m_bricklets;
  RawBrickletToIDMap m_rawToBrickletID;
};
