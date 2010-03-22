

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct checkForNewBrickletsParams{
	double  rememberCalls;
	double *endBrickletID;
	double *startBrickletID;
	double  result;	
};
typedef struct checkForNewBrickletsParams checkForNewBrickletsParams;
#pragma pack()

// variable checkForNewBricklets(variable *startBrickletID,variable *endBrickletID,variable rememberCalls)
static int checkForNewBricklets(checkForNewBrickletsParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct closeResultFileParams{
	double  result;	
};
typedef struct closeResultFileParams closeResultFileParams;
#pragma pack()

// variable closeResultFile()
static int closeResultFile(closeResultFileParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getAllBrickletDataParams{
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;	
};
typedef struct getAllBrickletDataParams getAllBrickletDataParams;
#pragma pack()

// variable getAllBrickletData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletData(getAllBrickletDataParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getAllBrickletMetaDataParams{
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;	
};
typedef struct getAllBrickletMetaDataParams getAllBrickletMetaDataParams;
#pragma pack()

// variable getAllBrickletMetaData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletMetaData(getAllBrickletMetaDataParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBrickletDataParams{
	double  brickletID;
	double  separateFolderForEachBricklet;
	Handle  baseName;
	double  result;	
};
typedef struct getBrickletDataParams getBrickletDataParams;
#pragma pack()

// variable getBrickletData(string baseName, variable separateFolderForEachBricklet, variable brickletID)
static int getBrickletData(getBrickletDataParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBrickletMetaDataParams{
	Handle  metaData;
	double  brickletID;
	double  result;	
};
typedef struct getBrickletMetaDataParams getBrickletMetaDataParams;
#pragma pack()

// variable getBrickletMetaData(variable brickletID, string metaData)
static int getBrickletMetaData(getBrickletMetaDataParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getBrickletRawDataParams{
	Handle  dataWave;
	double  brickletID;
	double  result;	
};
typedef struct getBrickletRawDataParams getBrickletRawDataParams;
#pragma pack()

// variable getBrickletRawData(variable brickletID, string dataWave)
static int getBrickletRawData(getBrickletRawDataParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getErrorMessageParams{
	double  errorCode;
	Handle  result;	
};
typedef struct getErrorMessageParams getErrorMessageParams;
#pragma pack()

// string getErrorMessage(variable errorCode)
static int getErrorMessage(getErrorMessageParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getNumberOfBrickletsParams{
	double *totalNumberOfBricklets;
	double  result;	
};
typedef struct getNumberOfBrickletsParams getNumberOfBrickletsParams;
#pragma pack()

// variable getNumberOfBricklets(variable *totalNumberOfBricklets)
static int getNumberOfBricklets(getNumberOfBrickletsParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




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

// variable getRangeBrickletData(string baseName, variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletData(getRangeBrickletDataParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




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

// variable getRangeBrickletMetaData(string baseName,variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletMetaData(getRangeBrickletMetaDataParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getResultFileMetaDataParams{
	Handle  waveName;
	double  result;	
};
typedef struct getResultFileMetaDataParams getResultFileMetaDataParams;
#pragma pack()

// variable getResultFileMetaData(string waveName)
static int getResultFileMetaData(getResultFileMetaDataParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getResultFileNameParams{
	Handle *filename;
	double  result;	
};
typedef struct getResultFileNameParams getResultFileNameParams;
#pragma pack()

// variable getResultFileName(string *filename)
static int getResultFileName(getResultFileNameParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getResultFilePathParams{
	Handle *absoluteFilePath;
	double  result;	
};
typedef struct getResultFilePathParams getResultFilePathParams;
#pragma pack()

// variable getResultFilePath(string *absoluteFilePath)
static int getResultFilePath(getResultFilePathParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getVernissageVersionParams{
	Handle *vernissageVersion;
	double  result;	
};
typedef struct getVernissageVersionParams getVernissageVersionParams;
#pragma pack()

// variable getVernissageVersion(string *vernissageVersion)
static int getVernissageVersion(getVernissageVersionParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getXOPVersionParams{
	Handle *xopVersion;
	double  result;	
};
typedef struct getXOPVersionParams getXOPVersionParams;
#pragma pack()

// variable getXOPVersion(string *xopVersion)
static int getXOPVersion(getXOPVersionParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}




#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct openResultFileParams{
	Handle  fileName;
	Handle  absoluteFilePath;
	double  result;	
};
typedef struct openResultFileParams openResultFileParams;
#pragma pack()

// variable openResultFile(string absoluteFilePath, string fileName)
static int openResultFile(openResultFileParams *p){

	p->result = UNKOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}






static long RegisterFunction()
{
	/*	NOTE:
		Some XOPs should return a result of NIL in response to the FUNCADDRS message.
		See XOP manual "Restrictions on Direct XFUNCs" section.
	*/

	int funcIndex = GetXOPItem(0);		/* which function invoked ? */
	long returnValue = NIL;

	switch (funcIndex) {
		case 0:						
			returnValue = (long) checkForNewBricklets;
			break;
		case 1:						
			returnValue = (long) closeResultFile;
			break;
		case 2:						
			returnValue = (long) getAllBrickletData;
			break;
		case 3:						
			returnValue = (long) getAllBrickletMetaData;
			break;
		case 4:						
			returnValue = (long) getBrickletData;
			break;
		case 5:						
			returnValue = (long) getBrickletMetaData;
			break;
		case 6:						
			returnValue = (long) getBrickletRawData;
			break;
		case 7:						
			returnValue = (long) getErrorMessage;
			break;
		case 8:						
			returnValue = (long) getNumberOfBricklets;
			break;
		case 9:						
			returnValue = (long) getRangeBrickletData;
			break;
		case 10:						
			returnValue = (long) getRangeBrickletMetaData;
			break;
		case 11:						
			returnValue = (long) getResultFileMetaData;
			break;
		case 12:						
			returnValue = (long) getResultFileName;
			break;
		case 13:						
			returnValue = (long) getResultFilePath;
			break;
		case 14:						
			returnValue = (long) getVernissageVersion;
			break;
		case 15:						
			returnValue = (long) getXOPVersion;
			break;
		case 16:						
			returnValue = (long) openResultFile;
			break;

	}
	return returnValue;
}
