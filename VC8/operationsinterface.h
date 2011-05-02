/*
	The file operationsinterface.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#pragma once

#include "header.h"

#include "operationstructs.h"

/* custom error codes */
#define REQUIRES_IGOR_620	FIRST_XOP_ERR + 1
#define OUT_OF_MEMORY		FIRST_XOP_ERR + 2

/* Prototypes */
HOST_IMPORT int XOPMain(IORecHandle ioRecHandle);

#define BEGIN_OUTER_CATCH	try{
#define END_OUTER_CATCH		}\
							catch(...){\
								sprintf(globDataPtr->outputBuffer,"Unexpected exception caught in line %d, function %s,  file %s\r", __LINE__, __FUNCTION__, __FILE__);\
								XOPNotice(globDataPtr->outputBuffer);\
								globDataPtr->setError(UNKNOWN_ERROR);\
								return 0;\
							}

extern "C" {
	int ExecuteCheckForNewBricklets(CheckForNewBrickletsRuntimeParamsPtr p);
	int ExecuteGetResultFileMetaData(GetResultFileMetaDataRuntimeParamsPtr p);
	int ExecuteCreateOverviewTable(CreateOverviewTableRuntimeParamsPtr p);
	int ExecuteGetReportTemplate(GetReportTemplateRuntimeParamsPtr p);
	int ExecuteGetBrickletData(GetBrickletDataRuntimeParamsPtr p);
	int ExecuteGetBrickletMetaData(GetBrickletMetaDataRuntimeParamsPtr p);
	int ExecuteGetBrickletRawData(GetBrickletRawDataRuntimeParamsPtr p);
	int ExecuteGetXOPErrorMessage(GetXOPErrorMessageRuntimeParamsPtr p);
	int ExecuteGetResultFileName(GetResultFileNameRuntimeParamsPtr p);
	int ExecuteGetVernissageVersion(GetVernissageVersionRuntimeParamsPtr p);
	int ExecuteGetVersion(GetVersionRuntimeParamsPtr p);
	int ExecuteOpenResultFile(OpenResultFileRuntimeParamsPtr p);
	int ExecuteGetBrickletCount(GetBrickletCountRuntimeParamsPtr p);
	int ExecuteCloseResultFile(CloseResultFileRuntimeParamsPtr p);
	void XOPEntry();
}

int RegisterGetResultFileMetaData();
int RegisterGetMtrxFileReaderVersion();
int RegisterGetVernissageVersion();
int RegisterGetXOPErrorMessage();
int RegisterOpenResultFile();
int RegisterCloseResultFile();
int RegisterGetBrickletCount();
int RegisterGetResultFileName();
int RegisterGetBrickletData();
int RegisterGetBrickletMetaData();
int RegisterGetBrickletRawData();
int RegisterGetReportTemplate();
int RegisterCreateOverviewTable();
int RegisterCheckForNewBricklets();
