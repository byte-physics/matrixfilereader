#pragma once

#include "header.h"
#include "operationsinterface.h"

int RegisterGetResultFileMetaData(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the GetResultFileMetaDataRuntimeParams structure as well.
	cmdTemplate = "MFR_GetResultFileMetaData /N=string:waveName";
	runtimeNumVarList = V_flag;
	runtimeStrVarList = S_waveNames;
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(GetResultFileMetaDataRuntimeParams), (void*)ExecuteGetResultFileMetaData, 0);
}

int RegisterGetMtrxFileReaderVersion(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the GetMtrxFileReaderVersionRuntimeParams structure as well.
	cmdTemplate = "MFR_GetVersion";
	runtimeNumVarList = V_XOPversion;
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(GetVersionRuntimeParams), (void*)ExecuteGetVersion, 0);
}

int RegisterGetVernissageVersion(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the GetVernissageVersionRuntimeParams structure as well.
	cmdTemplate = "MFR_GetVernissageVersion";
	runtimeNumVarList = V_DLLversion;
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(GetVernissageVersionRuntimeParams), (void*)ExecuteGetVernissageVersion, 0);
}

int RegisterGetXOPErrorMessage(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the GetXOPErrorMessageRuntimeParams structure as well.
	cmdTemplate = "MFR_GetXOPErrorMessage [number:errorCode]";
	runtimeNumVarList = "";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(GetXOPErrorMessageRuntimeParams), (void*)ExecuteGetXOPErrorMessage, 0);
}

int RegisterOpenResultFile(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the OpenResultFileRuntimeParams structure as well.
	cmdTemplate = "MFR_OpenResultFile /K /P=name:pathName [string:fileNameOrPath]";
	runtimeNumVarList = V_flag;
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(OpenResultFileRuntimeParams), (void*)ExecuteOpenResultFile, 0);
}

int RegisterCloseResultFile(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the CloseResultFileRuntimeParams structure as well.
	cmdTemplate = "MFR_CloseResultFile";
	runtimeNumVarList = V_flag;
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(CloseResultFileRuntimeParams), (void*)ExecuteCloseResultFile, 0);
}

int RegisterGetBrickletCount(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the GetBrickletCountRuntimeParams structure as well.
	cmdTemplate = "MFR_GetBrickletCount";
	runtimeNumVarList = "V_flag;V_count";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(GetBrickletCountRuntimeParams), (void*)ExecuteGetBrickletCount, 0);
}

int RegisterGetResultFileName(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the GetResultFileNameRuntimeParams structure as well.
	cmdTemplate = "MFR_GetResultFileName";
	runtimeNumVarList = V_flag;
	runtimeStrVarList = "S_fileName;S_dirPath";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(GetResultFileNameRuntimeParams), (void*)ExecuteGetResultFileName, 0);
}

int RegisterGetBrickletData(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the GetBrickletDataRuntimeParams structure as well.
	cmdTemplate = "MFR_GetBrickletData /R=(number:startBrickletID[,number:endBrickletID]) /S=number:size /N=string:baseName";
	runtimeNumVarList = V_flag;
	runtimeStrVarList = S_waveNames;
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(GetBrickletDataRuntimeParams), (void*)ExecuteGetBrickletData, 0);
}

int RegisterGetBrickletMetaData(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the GetBrickletMetaDataRuntimeParams structure as well.
	cmdTemplate = "MFR_GetBrickletMetaData /R=(number:startBrickletID[,number:endBrickletID]) /N=string:baseName";
	runtimeNumVarList = V_flag;
	runtimeStrVarList = S_waveNames;
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(GetBrickletMetaDataRuntimeParams), (void*)ExecuteGetBrickletMetaData, 0);
}

int RegisterGetBrickletRawData(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the GetBrickletRawDataRuntimeParams structure as well.
	cmdTemplate = "MFR_GetBrickletRawData /R=(number:startBrickletID[,number:endBrickletID]) /N=string:baseName";
	runtimeNumVarList = V_flag;
	runtimeStrVarList = S_waveNames;
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(GetBrickletRawDataRuntimeParams), (void*)ExecuteGetBrickletRawData, 0);
}

int RegisterGetReportTemplate(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the GetReportTemplateRuntimeParams structure as well.
	cmdTemplate = "MFR_GetReportTemplate";
	runtimeNumVarList = "";
	runtimeStrVarList = S_value;
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(GetReportTemplateRuntimeParams), (void*)ExecuteGetReportTemplate, 0);
}

int RegisterCreateOverviewTable(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the CreateOverviewTableRuntimeParams structure as well.
	cmdTemplate = "MFR_CreateOverviewTable /N=string:waveName /KEYS=string:keyList";
	runtimeNumVarList = V_flag;
	runtimeStrVarList = S_waveNames;
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(CreateOverviewTableRuntimeParams), (void*)ExecuteCreateOverviewTable, 0);
}

int RegisterCheckForNewBricklets(void){
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the CheckForNewBrickletsRuntimeParams structure as well.
	cmdTemplate = "MFR_CheckForNewBricklets";
	runtimeNumVarList = "V_flag;V_startBrickletID;V_endBrickletID";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(CheckForNewBrickletsRuntimeParams), (void*)ExecuteCheckForNewBricklets, 0);
}