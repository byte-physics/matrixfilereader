/*
  The file operationsinterface_getvernissageversion.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.hpp"
#include "operationsinterface.hpp"
#include "globaldata.hpp"
#include "utils_generic.hpp"

extern "C" int ExecuteGetVernissageVersion(GetVernissageVersionRuntimeParamsPtr p)
{
  BEGIN_OUTER_CATCH
  int ret = SetOperationNumVar(V_DLLversion, 0);

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  ret = SetOperationNumVar(V_DLLversion, stringToAnyType<double>(GlobalData::Instance().getVernissageVersion()));

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  END_OUTER_CATCH
  return 0;
}
