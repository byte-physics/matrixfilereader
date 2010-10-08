#include "header.h"

// Runtime param structure for GetResultFileMetaData operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetResultFileMetaDataRuntimeParams {
	// Flag parameters.

	// Parameters for /N flag group.
	int NFlagEncountered;
	Handle waveName;
	int NFlagParamsSet[1];

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetResultFileMetaDataRuntimeParams GetResultFileMetaDataRuntimeParams;
typedef struct GetResultFileMetaDataRuntimeParams* GetResultFileMetaDataRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.


// Operation template: CreateOverviewTable /N=string:waveName /KEYS=string:keyList

// Runtime param structure for CreateOverviewTable operation.
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

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct CreateOverviewTableRuntimeParams CreateOverviewTableRuntimeParams;
typedef struct CreateOverviewTableRuntimeParams* CreateOverviewTableRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: OpenResultFile /C /P=name:pathName string:fileNameOrPath

// Runtime param structure for OpenResultFile operation.
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
	Handle fileNameOrPath;
	int fileNameOrPathParamsSet[1];

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct OpenResultFileRuntimeParams OpenResultFileRuntimeParams;
typedef struct OpenResultFileRuntimeParams* OpenResultFileRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.


// Operation template: CloseResultFile

// Runtime param structure for CloseResultFile operation.
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

// Operation template: GetBrickletCount

// Runtime param structure for GetBrickletCount operation.
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

// Operation template: GetResultFileName

// Runtime param structure for GetResultFileName operation.
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

// Operation template: GetMtrxFileReaderVersion

// Runtime param structure for GetMtrxFileReaderVersion operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetMtrxFileReaderVersionRuntimeParams {
	// Flag parameters.

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetMtrxFileReaderVersionRuntimeParams GetMtrxFileReaderVersionRuntimeParams;
typedef struct GetMtrxFileReaderVersionRuntimeParams* GetMtrxFileReaderVersionRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: GetVernissageVersion

// Runtime param structure for GetVernissageVersion operation.
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

// Operation template: GetBrickletData /R=(number:startBrickletID,number:endBrickletID) /N=string:baseName

// Operation template: GetBrickletData /R=(number:startBrickletID[,number:endBrickletID]) /N=string:baseName

// Runtime param structure for GetBrickletData operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GetBrickletDataRuntimeParams {
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

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetBrickletDataRuntimeParams GetBrickletDataRuntimeParams;
typedef struct GetBrickletDataRuntimeParams* GetBrickletDataRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: GetBrickletMetaData /R=(number:startBrickletID[,number:endBrickletID]) /N=string:baseName

// Runtime param structure for GetBrickletMetaData operation.
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

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetBrickletMetaDataRuntimeParams GetBrickletMetaDataRuntimeParams;
typedef struct GetBrickletMetaDataRuntimeParams* GetBrickletMetaDataRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Runtime param structure for GetBrickletMetaData operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct GenericGetBrickletParams {
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

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GenericGetBrickletParams GenericGetBrickletParams;
typedef struct GenericGetBrickletParams* GenericGetBrickletParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: GetBrickletRawData /R=(number:startBrickletID[,number:endBrickletID]) /N=string:baseName

// Runtime param structure for GetBrickletRawData operation.
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

	// Main parameters.

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct GetBrickletRawDataRuntimeParams GetBrickletRawDataRuntimeParams;
typedef struct GetBrickletRawDataRuntimeParams* GetBrickletRawDataRuntimeParamsPtr;
#pragma pack()	// All structures passed to Igor are two-byte aligned.

// Operation template: GetReportTemplate

// Runtime param structure for GetReportTemplate operation.
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

int GenericGetBricklet(GenericGetBrickletParamsPtr p, int typeOfData);

// Operation template: CheckForNewBricklets

// Runtime param structure for CheckForNewBricklets operation.
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
