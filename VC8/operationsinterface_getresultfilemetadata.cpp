/*
	The file operationsinterface_getresultfilemetadata.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "header.h"

#include "operationstructs.h"
#include "operationsinterface.h"

#include "utils_bricklet.h"

#include "globaldata.h"

extern "C" int ExecuteGetResultFileMetaData(GetResultFileMetaDataRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	GlobalData::Instance().initializeWithoutReadSettings(p->calledFromMacro,p->calledFromFunction);

	SetOperationStrVar(S_waveNames,"");

	std::string waveName, fullPathOfCreatedWaves;
	DataFolderHandle destDataFolderHndl = NULL;
	std::vector<std::string> keys,values;
	char buf[ARRAY_SIZE];
	int ret;
	void *pBricklet = NULL;
	BrickletClass *bricklet = NULL;
	tm ctime;
	Vernissage::Session::BrickletMetaData brickletMetaData;

	// check of DEST flag which tells us that we should place all output in the supplied datafolder
	// and also read the variable settings from this folder
	if (p->DESTFlagEncountered){
		if(p->dfref == NULL){
			GlobalData::Instance().setError(WRONG_PARAMETER,"dfref");
			return 0;
		}
		destDataFolderHndl = p->dfref;
		// Here we check again for the config variables, this time in the destDataFolderHndl
	}
	else{// no DEST flag given, so we take the current data folder as destination folder
		ret = GetCurrentDataFolder(&destDataFolderHndl);
		if(ret != 0){
			GlobalData::Instance().setInternalError(ret);
			return 0;
		}
	}
	GlobalData::Instance().readSettings(destDataFolderHndl);

	if(!GlobalData::Instance().resultFileOpen()){
		GlobalData::Instance().setError(NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = GlobalData::Instance().getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int numberOfBricklets = pSession->getBrickletCount();

	// check waveName parameter
	if (p->NFlagEncountered){
		if( GetHandleSize(p->waveName) == 0L ){
			GlobalData::Instance().setError(WRONG_PARAMETER,"waveName");
			return 0;
		}
		else{
			convertHandleToString(p->waveName,waveName);
		}
	}
	else{
		waveName = resultMetaDefault;
	}

	keys.push_back(RESULT_DIR_PATH_KEY);
	values.push_back(GlobalData::Instance().getDirPath());

	keys.push_back(RESULT_FILE_NAME_KEY);
	values.push_back(GlobalData::Instance().getFileName());

	keys.push_back("totalNumberOfBricklets");
	values.push_back(anyTypeToString<int>(numberOfBricklets));

	if(numberOfBricklets > 0){
		bricklet = GlobalData::Instance().getBrickletClassObject(numberOfBricklets);
		ASSERT_RETURN_ZERO(bricklet);	
		pBricklet  = bricklet->getBrickletPointer();
		ASSERT_RETURN_ZERO(pBricklet);

		// use the timestamp of the last bricklet as dateOfLastChange
		ctime = pSession->getCreationTimestamp(pBricklet);

		brickletMetaData = pSession->getMetaData(pBricklet);

		keys.push_back("dateOfLastChange");
		sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d",ctime.tm_mon+1,ctime.tm_mday,ctime.tm_year+1900, ctime.tm_hour,ctime.tm_min,ctime.tm_sec);
		values.push_back(buf);

		keys.push_back("timeStampOfLastChange");
		values.push_back(anyTypeToString<time_t>(mktime(&ctime)));

		keys.push_back("BrickletMetaData.fileCreatorName");
		values.push_back(WStringToString(brickletMetaData.fileCreatorName));

		keys.push_back("BrickletMetaData.fileCreatorVersion");
		values.push_back(WStringToString(brickletMetaData.fileCreatorVersion));

		keys.push_back("BrickletMetaData.userName");
		values.push_back(WStringToString(brickletMetaData.userName));

		keys.push_back("BrickletMetaData.accountName");
		values.push_back(WStringToString(brickletMetaData.accountName));
	}
	else{
		keys.push_back("dateOfLastChange");
		values.push_back("");

		keys.push_back("timeStampOfLastChange");
		values.push_back("");

		keys.push_back("BrickletMetaData.fileCreatorName");
		values.push_back("");

		keys.push_back("BrickletMetaData.fileCreatorVersion");
		values.push_back("");

		keys.push_back("BrickletMetaData.userName");
		values.push_back("");

		keys.push_back("BrickletMetaData.accountName");
		values.push_back("");
	}

	// brickletID=0 because we are handling resultfile metadata
	ret = createAndFillTextWave(keys,values,destDataFolderHndl,waveName.c_str(),0,fullPathOfCreatedWaves);

	if(ret == WAVE_EXIST){
		GlobalData::Instance().setError(ret,waveName);
		return 0;
	}
	else if(ret != 0){
		GlobalData::Instance().setInternalError(ret);
		return 0;
	}

	SetOperationStrVar(S_waveNames,fullPathOfCreatedWaves.c_str());
	bool clearCache=true;
	GlobalData::Instance().finalize(clearCache);
	END_OUTER_CATCH
	return 0;
}
