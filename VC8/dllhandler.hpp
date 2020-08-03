/*
  The file dllhandler.h is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/
#pragma once
#include "ForwardDecl.hpp"

/*
  Everything related to loading the Vernissage DLLs.
  Calling closeSession() before exiting the application is mandatory.
*/
class DLLHandler
{
  typedef Vernissage::Session *(*GetSessionFunc)();
  typedef void (*ReleaseSessionFunc)();

public:
  DLLHandler();
  ~DLLHandler();

  Vernissage::Session *createSessionObject();
  void closeSession();
  const std::string &getVernissageVersion() const;

private:
  std::string getVernissagePath();

private:
  GetSessionFunc m_getSessionFunc;
  ReleaseSessionFunc m_releaseSessionFunc;
  HMODULE m_foundationModule;
  std::string m_vernissageVersion;
};