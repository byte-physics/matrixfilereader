/*
  The file operationsinterface_getresultfilename.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.hpp"
#include "operationsinterface.hpp"
#include "globaldata.hpp"

extern "C" int ExecuteGetResultFileName(GetResultFileNameRuntimeParamsPtr p)
{
  BEGIN_OUTER_CATCH
  GlobalData::Instance().initialize(p->calledFromMacro, p->calledFromFunction);

  int ret = SetOperationStrVar(S_fileName, "");

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  ret = SetOperationStrVar(S_dirPath, "");

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  if (!GlobalData::Instance().resultFileOpen())
  {
    GlobalData::Instance().setError(NO_FILE_OPEN);
    return 0;
  }

  ret = SetOperationStrVar(S_fileName, convertEncoding(GlobalData::Instance().getFileName()).c_str());

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  ret = SetOperationStrVar(S_dirPath, convertEncoding(GlobalData::Instance().getDirPath()).c_str());

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  GlobalData::Instance().finalize();
  END_OUTER_CATCH
  return 0;
}
