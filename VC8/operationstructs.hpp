/*
	The file operationstructs.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/
#pragma once

/*
	Operation structs are defined here (they are automatically generated using the
	ParseOperationTemplate Igor function, see Operation-Template-Generator.pxp)
*/

// Operation template: MFR_GetXOPErrorMessage [number:errorCode]

// Runtime param structure for MFR_GetXOPErrorMessage operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetXOPErrorMessageRuntimeParams {
	// Flag parameters.

	// Main parameters.

	// Parameters for simple main group #0.
	int errorCodeEncountered;
	double errorCode;						// Optional parameter.
	int errorCodeParamsSet[1];

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetXOPErrorMessageRuntimeParams GetXOPErrorMessageRuntimeParams;
typedef struct GetXOPErrorMessageRuntimeParams* GetXOPErrorMessageRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Runtime param structure for MFR_GetResultFileMetaData operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetResultFileMetaDataRuntimeParams {
	// Flag parameters.

	// Parameters for /N flag group.
	int NFlagEncountered;
	Handle waveName;
	int NFlagParamsSet[1];

	// Parameters for /DEST flag group.
	int DESTFlagEncountered;
	DataFolderHandle dfref;
	int DESTFlagParamsSet[1];

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetResultFileMetaDataRuntimeParams GetResultFileMetaDataRuntimeParams;
typedef struct GetResultFileMetaDataRuntimeParams* GetResultFileMetaDataRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.


// Operation template: MFR_CreateOverviewTable /N=string:waveName /KEYS=string:keyList

// Runtime param structure for MFR_CreateOverviewTable operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct CreateOverviewTableRuntimeParams {
	// Flag parameters.

	// Parameters for /N flag group.
	int NFlagEncountered;
	Handle waveName;
	int NFlagParamsSet[1];

	// Parameters for /KEYS flag group.
	int KEYSFlagEncountered;
	Handle keyList;
	int KEYSFlagParamsSet[1];

	// Parameters for /DEST flag group.
	int DESTFlagEncountered;
	DataFolderHandle dfref;
	int DESTFlagParamsSet[1];

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct CreateOverviewTableRuntimeParams CreateOverviewTableRuntimeParams;
typedef struct CreateOverviewTableRuntimeParams* CreateOverviewTableRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: MFR_OpenResultFile /C /P=name:pathName [string:fileNameOrPath]

// Runtime param structure for MFR_OpenResultFile operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct OpenResultFileRuntimeParams {
	// Flag parameters.

	// Parameters for /K flag group.
	int KFlagEncountered;
	// There are no fields for this group because it has no parameters.

	// Parameters for /P flag group.
	int PFlagEncountered;
	char pathName[MAX_OBJ_NAME+1];
	int PFlagParamsSet[1];

	// Main parameters.

	// Parameters for simple main group #0.
	int fileNameOrPathEncountered;
	Handle fileNameOrPath;					// Optional parameter.
	int fileNameOrPathParamsSet[1];

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct OpenResultFileRuntimeParams OpenResultFileRuntimeParams;
typedef struct OpenResultFileRuntimeParams* OpenResultFileRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.


// Operation template: MFR_CloseResultFile

// Runtime param structure for MFR_CloseResultFile operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct CloseResultFileRuntimeParams {
	// Flag parameters.

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct CloseResultFileRuntimeParams CloseResultFileRuntimeParams;
typedef struct CloseResultFileRuntimeParams* CloseResultFileRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: MFR_GetBrickletCount

// Runtime param structure for MFR_GetBrickletCount operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetBrickletCountRuntimeParams {
	// Flag parameters.

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetBrickletCountRuntimeParams GetBrickletCountRuntimeParams;
typedef struct GetBrickletCountRuntimeParams* GetBrickletCountRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: MFR_GetResultFileName

// Runtime param structure for MFR_GetResultFileName operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetResultFileNameRuntimeParams {
	// Flag parameters.

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetResultFileNameRuntimeParams GetResultFileNameRuntimeParams;
typedef struct GetResultFileNameRuntimeParams* GetResultFileNameRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: MFR_GetVersion

// Runtime param structure for MFR_GetVersion operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetVersionRuntimeParams {
	// Flag parameters.

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetVersionRuntimeParams GetVersionRuntimeParams;
typedef struct GetVersionRuntimeParams* GetVersionRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: MFR_GetVernissageVersion

// Runtime param structure for MFR_GetVernissageVersion operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetVernissageVersionRuntimeParams {
	// Flag parameters.

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetVernissageVersionRuntimeParams GetVernissageVersionRuntimeParams;
typedef struct GetVernissageVersionRuntimeParams* GetVernissageVersionRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: MFR_GetBrickletData /R=(number:startBrickletID[,number:endBrickletID]) /S=number:pixelSize /N=string:baseName /DEST=dataFolderRef:dfref

// Runtime param structure for MFR_GetBrickletData operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetBrickletDataRuntimeParams {
	// Flag parameters.

	// Parameters for /R flag group.
	int RFlagEncountered;
	double startBrickletID;
	double endBrickletID;					// Optional parameter.
	int RFlagParamsSet[2];

	// Parameters for /S flag group.
	int SFlagEncountered;
	double pixelSize;
	int SFlagParamsSet[1];

	// Parameters for /N flag group.
	int NFlagEncountered;
	Handle baseName;
	int NFlagParamsSet[1];

	// Parameters for /DEST flag group.
	int DESTFlagEncountered;
	DataFolderHandle dfref;
	int DESTFlagParamsSet[1];

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetBrickletDataRuntimeParams GetBrickletDataRuntimeParams;
typedef struct GetBrickletDataRuntimeParams* GetBrickletDataRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: MFR_GetBrickletMetaData /R=(number:startBrickletID[,number:endBrickletID]) /N=string:baseName

// Runtime param structure for MFR_GetBrickletMetaData operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetBrickletMetaDataRuntimeParams {
	// Flag parameters.

	// Parameters for /R flag group.
	int RFlagEncountered;
	double startBrickletID;
	double endBrickletID;					// Optional parameter.
	int RFlagParamsSet[2];

	// Parameters for /N flag group.
	int NFlagEncountered;
	Handle baseName;
	int NFlagParamsSet[1];

	// Parameters for /DEST flag group.
	int DESTFlagEncountered;
	DataFolderHandle dfref;
	int DESTFlagParamsSet[1];

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetBrickletMetaDataRuntimeParams GetBrickletMetaDataRuntimeParams;
typedef struct GetBrickletMetaDataRuntimeParams* GetBrickletMetaDataRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Runtime param structure for GenericGetBricklet function
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GenericGetBrickletParams {
	// Flag parameters.

	// Parameters for /R flag group.
	int RFlagEncountered;
	double startBrickletID;
	double endBrickletID;					// Optional parameter.
	int RFlagParamsSet[2];

	// Parameters for /S flag group.
	int SFlagEncountered;
	double pixelSize;
	int SFlagParamsSet[1];

	// Parameters for /N flag group.
	int NFlagEncountered;
	Handle baseName;
	int NFlagParamsSet[1];

	// Parameters for /DEST flag group.
	int DESTFlagEncountered;
	DataFolderHandle dfref;
	int DESTFlagParamsSet[1];

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GenericGetBrickletParams GenericGetBrickletParams;
typedef struct GenericGetBrickletParams* GenericGetBrickletParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: MFR_GetBrickletRawData /R=(number:startBrickletID[,number:endBrickletID]) /N=string:baseName

// Runtime param structure for MFR_GetBrickletRawData operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetBrickletRawDataRuntimeParams {
	// Flag parameters.

	// Parameters for /R flag group.
	int RFlagEncountered;
	double startBrickletID;
	double endBrickletID;					// Optional parameter.
	int RFlagParamsSet[2];

	// Parameters for /N flag group.
	int NFlagEncountered;
	Handle baseName;
	int NFlagParamsSet[1];

	// Parameters for /DEST flag group.
	int DESTFlagEncountered;
	DataFolderHandle dfref;
	int DESTFlagParamsSet[1];

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetBrickletRawDataRuntimeParams GetBrickletRawDataRuntimeParams;
typedef struct GetBrickletRawDataRuntimeParams* GetBrickletRawDataRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

int GenericGetBricklet(GenericGetBrickletParamsPtr p, int typeOfData);

// Operation template: MFR_GetReportTemplate

// Runtime param structure for MFR_GetReportTemplate operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetReportTemplateRuntimeParams {
	// Flag parameters.

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetReportTemplateRuntimeParams GetReportTemplateRuntimeParams;
typedef struct GetReportTemplateRuntimeParams* GetReportTemplateRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: MFR_CheckForNewBricklets

// Runtime param structure for MFR_CheckForNewBricklets operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct CheckForNewBrickletsRuntimeParams {
	// Flag parameters.

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct CheckForNewBrickletsRuntimeParams CheckForNewBrickletsRuntimeParams;
typedef struct CheckForNewBrickletsRuntimeParams* CheckForNewBrickletsRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.
