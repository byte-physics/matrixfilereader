


// variable checkForNewBricklets(variable *startBrickletID,variable *endBrickletID,variable rememberCalls)
static int checkForNewBricklets(checkForNewBrickletsParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable closeResultFile()
static int closeResultFile(closeResultFileParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getAllBrickletData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletData(getAllBrickletDataParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getAllBrickletMetaData(string baseName, variable separateFolderForEachBricklet)
static int getAllBrickletMetaData(getAllBrickletMetaDataParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getBrickletData(string baseName, variable separateFolderForEachBricklet, variable brickletID)
static int getBrickletData(getBrickletDataParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getBrickletMetaData(string baseName, variable separateFolderForEachBricklet, variable brickletID)
static int getBrickletMetaData(getBrickletMetaDataParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getBrickletRawData(string baseName, variable brickletID)
static int getBrickletRawData(getBrickletRawDataParams *p){

	p->result = UNKNOWN_ERROR;

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





// string getBugReportTemplate()
static int getBugReportTemplate(getBugReportTemplateParams *p){

	p->result = UNKNOWN_ERROR;

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





// string getErrorMessage(variable errorCode)
static int getErrorMessage(getErrorMessageParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getNumberOfBricklets(variable *totalNumberOfBricklets)
static int getNumberOfBricklets(getNumberOfBrickletsParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getRangeBrickletData(string baseName, variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletData(getRangeBrickletDataParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getRangeBrickletMetaData(string baseName,variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID)
static int getRangeBrickletMetaData(getRangeBrickletMetaDataParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getResultFileMetaData(string waveName)
static int getResultFileMetaData(getResultFileMetaDataParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getResultFileName(string *filename)
static int getResultFileName(getResultFileNameParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getResultFilePath(string *absoluteFilePath)
static int getResultFilePath(getResultFilePathParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getVernissageVersion(variable *vernissageVersion)
static int getVernissageVersion(getVernissageVersionParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable getXOPVersion(variable *xopVersion)
static int getXOPVersion(getXOPVersionParams *p){

	p->result = UNKNOWN_ERROR;

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





// variable openResultFile(string absoluteFilePath, string fileName)
static int openResultFile(openResultFileParams *p){

	p->result = UNKNOWN_ERROR;

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
			returnValue = (long) getBugReportTemplate;
			break;
		case 8:						
			returnValue = (long) getErrorMessage;
			break;
		case 9:						
			returnValue = (long) getNumberOfBricklets;
			break;
		case 10:						
			returnValue = (long) getRangeBrickletData;
			break;
		case 11:						
			returnValue = (long) getRangeBrickletMetaData;
			break;
		case 12:						
			returnValue = (long) getResultFileMetaData;
			break;
		case 13:						
			returnValue = (long) getResultFileName;
			break;
		case 14:						
			returnValue = (long) getResultFilePath;
			break;
		case 15:						
			returnValue = (long) getVernissageVersion;
			break;
		case 16:						
			returnValue = (long) getXOPVersion;
			break;
		case 17:						
			returnValue = (long) openResultFile;
			break;

	}
	return returnValue;
}
