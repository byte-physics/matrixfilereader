/*
  The file operationsinterface_getreporttemplate.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.hpp"
#include "operationsinterface.hpp"
#include "globaldata.hpp"
#include "utils_generic.hpp"

extern "C" int ExecuteGetReportTemplate(GetReportTemplateRuntimeParamsPtr p)
{
  BEGIN_OUTER_CATCH
  int ret = SetOperationStrVar(S_value, "");

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  std::string str;

  // get windows version
  // see http://msdn.microsoft.com/en-us/library/ms724451%28VS.85%29.aspx
  OSVERSIONINFO osvi;

  ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

  str.append("####\r");

  BOOL retValue = GetVersionEx(&osvi);

  // non zero is success here
  if (retValue != 0)
  {
    str.append("Windows version: " + toString(osvi.dwMajorVersion) + "." + toString(osvi.dwMinorVersion) + " (Build " + toString(osvi.dwBuildNumber) + ")\r");
  }
  else
  {
    str.append("Windows version: unknown\r");
  }

#if defined(_MSC_VER)
  str.append("Visual Studio version: " + toString(_MSC_VER) + "\r");
#elif defined(__GNUC__)
  str.append("GCC version: " __VERSION__ "\r");
#else
  str.append("Unknown compiler version\r");
#endif

  str.append("Igor Pro Version: " + toString(igorVersion) + "\r");
  str.append("Vernissage version: " + GlobalData::Instance().getVernissageVersion() + "\r");
  str.append("XOP version: " + std::string(MatrixFileReader_XOP_VERSION_STR) + "\r");
  str.append("Compilation date and time: " __DATE__ " " __TIME__ "\r");
  str.append("\r");
  str.append("Your Name:\r");
  str.append("Bug description:\r");
  str.append("####\r");

  ret = SetOperationStrVar(S_value, str.c_str());

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  HISTPRINT(str.c_str());

  GlobalData::Instance().finalize();
  END_OUTER_CATCH
  return 0;
}
