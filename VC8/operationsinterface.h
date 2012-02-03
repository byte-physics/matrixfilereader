/*
	The file operationsinterface.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/
#pragma once

#include "operationstructs.h"

/*
	Takes care of xop initialization, igor message handling and registering all operations
*/

/* custom error codes */
#define REQUIRES_IGOR_620	FIRST_XOP_ERR + 1
#define BROKEN_XOP			FIRST_XOP_ERR + 2

/* Prototypes */
HOST_IMPORT int XOPMain(IORecHandle ioRecHandle);
extern "C" void XOPEntry();

// Igor aborts if it encounters an unhandled exception, therefore every operation
// needs enclosing BEGIN_OUTER_CATCH and END_OUTER_CATCH macros
#define BEGIN_OUTER_CATCH	try{
#define END_OUTER_CATCH		}\
							catch(...){\
								sprintf(GlobalData::Instance().outputBuffer,"Unexpected exception caught in line %d, function %s,  file %s\r", __LINE__, __FUNCTION__, __FILE__);\
								XOPNotice(GlobalData::Instance().outputBuffer);\
								GlobalData::Instance().setError(UNKNOWN_ERROR);\
								return 0;\
							}

// each execute function holds the implementation of one igor operation
// are each in a separate file named operationsinterface_*
extern "C" int ExecuteCheckForNewBricklets(CheckForNewBrickletsRuntimeParamsPtr p);
extern "C" int ExecuteGetResultFileMetaData(GetResultFileMetaDataRuntimeParamsPtr p);
extern "C" int ExecuteCreateOverviewTable(CreateOverviewTableRuntimeParamsPtr p);
extern "C" int ExecuteGetReportTemplate(GetReportTemplateRuntimeParamsPtr p);
extern "C" int ExecuteGetBrickletData(GetBrickletDataRuntimeParamsPtr p);
extern "C" int ExecuteGetBrickletMetaData(GetBrickletMetaDataRuntimeParamsPtr p);
extern "C" int ExecuteGetBrickletRawData(GetBrickletRawDataRuntimeParamsPtr p);
extern "C" int ExecuteGetXOPErrorMessage(GetXOPErrorMessageRuntimeParamsPtr p);
extern "C" int ExecuteGetResultFileName(GetResultFileNameRuntimeParamsPtr p);
extern "C" int ExecuteGetVernissageVersion(GetVernissageVersionRuntimeParamsPtr p);
extern "C" int ExecuteGetVersion(GetVersionRuntimeParamsPtr p);
extern "C" int ExecuteOpenResultFile(OpenResultFileRuntimeParamsPtr p);
extern "C" int ExecuteGetBrickletCount(GetBrickletCountRuntimeParamsPtr p);
extern "C" int ExecuteCloseResultFile(CloseResultFileRuntimeParamsPtr p);
