/*
  The file operationsinterface_closeresultfile.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt  in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.hpp"
#include "operationsinterface.hpp"
#include "globaldata.hpp"

extern "C" int ExecuteCloseResultFile(CloseResultFileRuntimeParamsPtr p)
{
  BEGIN_OUTER_CATCH
  GlobalData::Instance().initialize(p->calledFromMacro, p->calledFromFunction);

  if (!GlobalData::Instance().resultFileOpen())
  {
    GlobalData::Instance().setError(NO_FILE_OPEN);
    return 0;
  }
  GlobalData::Instance().closeResultFile();

  GlobalData::Instance().finalize();
  END_OUTER_CATCH
  return 0;
}