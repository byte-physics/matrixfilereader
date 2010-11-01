#pragma once

#include "header.h"

#include "operationstructs.h"

/* custom error codes */

#define REQUIRES_IGOR_620	FIRST_XOP_ERR + 1
#define OUT_OF_MEMORY		FIRST_XOP_ERR + 2

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

int RegisterGetResultFileMetaData(void);
int RegisterGetMtrxFileReaderVersion(void);
int RegisterGetVernissageVersion(void);
int RegisterGetXOPErrorMessage(void);
int RegisterOpenResultFile(void);
int RegisterCloseResultFile(void);
int RegisterGetBrickletCount(void);
int RegisterGetResultFileName(void);
int RegisterGetBrickletData(void);
int RegisterGetBrickletMetaData(void);
int RegisterGetBrickletRawData(void);
int RegisterGetReportTemplate(void);
int RegisterCreateOverviewTable(void);
int RegisterCheckForNewBricklets(void);

/* Prototypes */
HOST_IMPORT int main(IORecHandle ioRecHandle);
static void XOPEntry();
