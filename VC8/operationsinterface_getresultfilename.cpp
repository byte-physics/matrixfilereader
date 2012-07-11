/*
  The file operationsinterface_getresultfilename.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt  in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.hpp"
#include "operationsinterface.hpp"
#include "globaldata.hpp"

extern "C" int ExecuteGetResultFileName(GetResultFileNameRuntimeParamsPtr p)
{
  GlobalData::Instance().initialize(p->calledFromMacro, p->calledFromFunction);
  SetOperationStrVar(S_fileName, "");
  SetOperationStrVar(S_dirPath, "");

  if (!GlobalData::Instance().resultFileOpen())
  {
    GlobalData::Instance().setError(NO_FILE_OPEN);
    return 0;
  }

  SetOperationStrVar(S_fileName, GlobalData::Instance().getFileName<std::string>().c_str());
  SetOperationStrVar(S_dirPath, GlobalData::Instance().getDirPath<std::string>().c_str());

  GlobalData::Instance().finalize();
  return 0;
}
