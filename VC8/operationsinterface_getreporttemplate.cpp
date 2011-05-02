/*
	The file operationsinterface_getreporttemplate.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "header.h"

#include "operationstructs.h"
#include "operationsinterface.h"

#include "globaldata.h"

extern "C" int ExecuteGetReportTemplate(GetReportTemplateRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
		SetOperationStrVar(S_value,"");

	std::string str;

	// get windows version
	// see http://msdn.microsoft.com/en-us/library/ms724451%28VS.85%29.aspx
	OSVERSIONINFO osvi;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	str.append("####\r");

	BOOL retValue = GetVersionEx(&osvi);
	// non zero is success here
	if(retValue != 0){
		str.append("Windows version: " + anyTypeToString<int>(osvi.dwMajorVersion) + "." + anyTypeToString<int>(osvi.dwMinorVersion) + " (Build " + anyTypeToString<int>(osvi.dwBuildNumber) + ")\r" );
	}
	else{
		str.append("Windows version: unknown\r");
	}

#if defined(_MSC_VER)
	str.append("Visual Studio version: " + anyTypeToString<int>(_MSC_VER) + "\r");
#elif defined(__GNUC__)
	str.append("GCC version: " __VERSION__ "\r");
#else
	str.append("Unknown compiler version\r");
#endif

	str.append("Igor Pro Version: " + anyTypeToString<int>(igorVersion) + "\r");
	str.append("Vernissage version: " + globDataPtr->getVernissageVersion() + "\r");
	str.append("XOP version: " + std::string(MatrixFileReader_XOP_VERSION_STR) + "\r");
	str.append("Compilation date and time: " __DATE__ " " __TIME__ "\r");
	str.append("\r");
	str.append("Your Name:\r");
	str.append("Bug description:\r");
	str.append("####\r");

	SetOperationStrVar(S_value,str.c_str());

	outputToHistory(str.c_str());
	END_OUTER_CATCH
		return 0;
}
