/*
	TODO
*/

/* custom error codes */

#define REQUIRES_IGOR_504	FIRST_XOP_ERR + 1
#define OUT_OF_MEMORY		FIRST_XOP_ERR + 2

/* Prototypes */
HOST_IMPORT void main(IORecHandle ioRecHandle);

static void XOPEntry();
static long RegisterFunction();
void doCleanup();




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct checkForNewBrickletsParams{
	double  rememberCalls;
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
struct getAllBrickletDataParams{
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;
};
typedef struct getAllBrickletDataParams getAllBrickletDataParams;
#pragma pack()

static int getAllBrickletData(getAllBrickletDataParams *p);


#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getAllBrickletMetaDataParams{
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;
};
typedef struct getAllBrickletMetaDataParams getAllBrickletMetaDataParams;
#pragma pack()

static int getAllBrickletMetaData(getAllBrickletMetaDataParams *p);


#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBrickletDataParams{
	double  brickletID;
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;
};
typedef struct getBrickletDataParams getBrickletDataParams;
#pragma pack()

static int getBrickletData(getBrickletDataParams *p);


#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBrickletMetaDataParams{
	double  brickletID;
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;
};
typedef struct getBrickletMetaDataParams getBrickletMetaDataParams;
#pragma pack()

static int getBrickletMetaData(getBrickletMetaDataParams *p);


#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBrickletRawDataParams{
	double  brickletID;
	Handle  baseName;
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
struct getErrorMessageParams{
	double  errorCode;
	Handle  result;
};
typedef struct getErrorMessageParams getErrorMessageParams;
#pragma pack()

static int getErrorMessage(getErrorMessageParams *p);


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
	double  separateFolderForEachBricklet;
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
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;
};
typedef struct getRangeBrickletMetaDataParams getRangeBrickletMetaDataParams;
#pragma pack()

static int getRangeBrickletMetaData(getRangeBrickletMetaDataParams *p);


#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getResultFileMetaDataParams{
	Handle  waveName;
	double  result;
};
typedef struct getResultFileMetaDataParams getResultFileMetaDataParams;
#pragma pack()

static int getResultFileMetaData(getResultFileMetaDataParams *p);


#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getResultFileNameParams{
	Handle *filename;
	double  result;
};
typedef struct getResultFileNameParams getResultFileNameParams;
#pragma pack()

static int getResultFileName(getResultFileNameParams *p);


#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getResultFilePathParams{
	Handle *absoluteFilePath;
	double  result;
};
typedef struct getResultFilePathParams getResultFilePathParams;
#pragma pack()

static int getResultFilePath(getResultFilePathParams *p);


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
