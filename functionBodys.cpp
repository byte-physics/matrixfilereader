


// variable checkForNewBricklets(variable *startBrickletID,variable *endBrickletID,variable rememberCalls)
static int checkForNewBricklets(checkForNewBrickletsParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable closeResultFile()
static int closeResultFile(closeResultFileParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable createOverViewTable(string waveName, string keyList)
static int createOverViewTable(createOverViewTableParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getAllBrickletData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletData(getAllBrickletDataParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getAllBrickletMetaData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletMetaData(getAllBrickletMetaDataParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getBrickletData(string baseName, variable separateFolderForEachBricklet, variable brickletID)
static int getBrickletData(getBrickletDataParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getBrickletMetaData(string baseName, variable separateFolderForEachBricklet, variable brickletID)
static int getBrickletMetaData(getBrickletMetaDataParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getBrickletRawData(string baseName, variable brickletID)
static int getBrickletRawData(getBrickletRawDataParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// string getBugReportTemplate()
static int getBugReportTemplate(getBugReportTemplateParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getLastError()
static int getLastError(getLastErrorParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// string getLastErrorMessage()
static int getLastErrorMessage(getLastErrorMessageParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getNumberOfBricklets(variable *totalNumberOfBricklets)
static int getNumberOfBricklets(getNumberOfBrickletsParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getRangeBrickletData(string baseName, variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletData(getRangeBrickletDataParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getRangeBrickletMetaData(string baseName,variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletMetaData(getRangeBrickletMetaDataParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getResultFileMetaData(string waveName)
static int getResultFileMetaData(getResultFileMetaDataParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getResultFileName(string *filename)
static int getResultFileName(getResultFileNameParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getResultFilePath(string *absoluteFilePath)
static int getResultFilePath(getResultFilePathParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getVernissageVersion(variable *vernissageVersion)
static int getVernissageVersion(getVernissageVersionParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable getXOPVersion(variable *xopVersion)
static int getXOPVersion(getXOPVersionParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





// variable openResultFile(string absoluteFilePath, string fileName)
static int openResultFile(openResultFileParams *p){

	p->result = UNKNOWN_ERROR;

	ASSERT_RETURN_ZERO(pMyData);
	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);



	p->result = SUCCESS;
	return 0;
}





