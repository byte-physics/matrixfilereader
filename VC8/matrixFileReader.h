
#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include <string>

/* custom error codes */

#define REQUIRES_IGOR_620	FIRST_XOP_ERR + 1
#define OUT_OF_MEMORY		FIRST_XOP_ERR + 2

/* Prototypes */
HOST_IMPORT int main(IORecHandle ioRecHandle);

static void XOPEntry();
static long RegisterFunction();
void doCleanup();

bool isValidBrickletRange(double startID, double endID, int numberOfBricklets);
bool isValidBrickletID(int brickletID, int numberOfBricklets);

void setError(double *result, int errorValue, std::string msgArgument = std::string());
void setInternalError(double *result, int errorValue);

// autogenerated
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct checkForNewBrickletsParams{
	double *endBrickletID;
	double *startBrickletID;
	double  result;
};
typedef struct checkForNewBrickletsParams checkForNewBrickletsParams;
#pragma pack()

static int checkForNewBricklets(checkForNewBrickletsParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct closeResultFileParams{
	double  result;
};
typedef struct closeResultFileParams closeResultFileParams;
#pragma pack()

static int closeResultFile(closeResultFileParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct createOverViewTableParams{
	Handle  keyList;
	Handle  waveName;
	double  result;
};
typedef struct createOverViewTableParams createOverViewTableParams;
#pragma pack()

static int createOverViewTable(createOverViewTableParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getAllBrickletDataParams{
	Handle  baseName;
	double  result;
};
typedef struct getAllBrickletDataParams getAllBrickletDataParams;
#pragma pack()

static int getAllBrickletData(getAllBrickletDataParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getAllBrickletMetaDataParams{
	Handle  baseName;
	double  result;
};
typedef struct getAllBrickletMetaDataParams getAllBrickletMetaDataParams;
#pragma pack()

static int getAllBrickletMetaData(getAllBrickletMetaDataParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBrickletDataParams{
	double  brickletID;
	Handle  baseName;
	double  result;
};
typedef struct getBrickletDataParams getBrickletDataParams;
#pragma pack()

static int getBrickletData(getBrickletDataParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBrickletMetaDataParams{
	double  brickletID;
	Handle  baseName;
	double  result;
};
typedef struct getBrickletMetaDataParams getBrickletMetaDataParams;
#pragma pack()

static int getBrickletMetaData(getBrickletMetaDataParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBrickletRawDataParams{
	double  brickletID;
	Handle  waveName;
	double  result;
};
typedef struct getBrickletRawDataParams getBrickletRawDataParams;
#pragma pack()

static int getBrickletRawData(getBrickletRawDataParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBugReportTemplateParams{
	Handle  result;
};
typedef struct getBugReportTemplateParams getBugReportTemplateParams;
#pragma pack()

static int getBugReportTemplate(getBugReportTemplateParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getLastErrorParams{
	double  result;
};
typedef struct getLastErrorParams getLastErrorParams;
#pragma pack()

static int getLastError(getLastErrorParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getLastErrorMessageParams{
	Handle  result;
};
typedef struct getLastErrorMessageParams getLastErrorMessageParams;
#pragma pack()

static int getLastErrorMessage(getLastErrorMessageParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getNumberOfBrickletsParams{
	double *totalNumberOfBricklets;
	double  result;
};
typedef struct getNumberOfBrickletsParams getNumberOfBrickletsParams;
#pragma pack()

static int getNumberOfBricklets(getNumberOfBrickletsParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getRangeBrickletDataParams{
	double  endBrickletID;
	double  startBrickletID;
	Handle  baseName;
	double  result;
};
typedef struct getRangeBrickletDataParams getRangeBrickletDataParams;
#pragma pack()

static int getRangeBrickletData(getRangeBrickletDataParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getRangeBrickletMetaDataParams{
	double  endBrickletID;
	double  startBrickletID;
	Handle  baseName;
	double  result;
};
typedef struct getRangeBrickletMetaDataParams getRangeBrickletMetaDataParams;
#pragma pack()

static int getRangeBrickletMetaData(getRangeBrickletMetaDataParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getResultFileParams{
	Handle *fileName;
	Handle *dirPath;
	double  result;
};
typedef struct getResultFileParams getResultFileParams;
#pragma pack()

static int getResultFile(getResultFileParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getResultFileMetaDataParams{
	Handle  waveName;
	double  result;
};
typedef struct getResultFileMetaDataParams getResultFileMetaDataParams;
#pragma pack()

static int getResultFileMetaData(getResultFileMetaDataParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getVernissageVersionParams{
	double *vernissageVersion;
	double  result;
};
typedef struct getVernissageVersionParams getVernissageVersionParams;
#pragma pack()

static int getVernissageVersion(getVernissageVersionParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getXOPVersionParams{
	double *xopVersion;
	double  result;
};
typedef struct getXOPVersionParams getXOPVersionParams;
#pragma pack()

static int getXOPVersion(getXOPVersionParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct openResultFileParams{
	Handle  fileName;
	Handle  absoluteFilePath;
	double  result;
};
typedef struct openResultFileParams openResultFileParams;
#pragma pack()

static int openResultFile(openResultFileParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct readXOPSettingsParams{
	Handle  result;
};
typedef struct readXOPSettingsParams readXOPSettingsParams;
#pragma pack()

static int readXOPSettings(readXOPSettingsParams *p);
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct writeXOPSettingsParams{
	Handle  config;
	double  result;
};
typedef struct writeXOPSettingsParams writeXOPSettingsParams;
#pragma pack()

static int writeXOPSettings(writeXOPSettingsParams *p);


