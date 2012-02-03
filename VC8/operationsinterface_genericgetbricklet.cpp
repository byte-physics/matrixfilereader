/*
	The file operationsinterface_genericgetbricklet.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.h"
#include "operationsinterface.h"
#include "utils_bricklet.h"
#include "brickletconverter.h"
#include "globaldata.h"
#include "brickletclass.h"
#include "utils_generic.h"

namespace{
	enum TYPE{ RAW_DATA=1, CONVERTED_DATA=2, META_DATA=4 };
}

extern "C" int ExecuteGetBrickletData(GetBrickletDataRuntimeParamsPtr p){

	GenericGetBrickletParams params;

	params.calledFromFunction	= p->calledFromFunction;
	params.calledFromMacro		= p->calledFromMacro;

	params.NFlagEncountered		= p->NFlagEncountered;
	params.NFlagParamsSet[0]	= p->NFlagParamsSet[0];
	params.baseName				= p->baseName;

	params.RFlagEncountered		= p->RFlagEncountered;
	params.RFlagParamsSet[0]	= p->RFlagParamsSet[0];
	params.RFlagParamsSet[1]	= p->RFlagParamsSet[1];
	params.startBrickletID		= p->startBrickletID;
	params.endBrickletID		= p->endBrickletID;

	params.SFlagEncountered		= p->SFlagEncountered;
	params.SFlagParamsSet[0]	= p->SFlagParamsSet[0];
	params.pixelSize			= p->pixelSize;

	params.DESTFlagEncountered  = p->DESTFlagEncountered;
	params.DESTFlagParamsSet[0] = p->DESTFlagParamsSet[0];
	params.dfref				= p->dfref;

	return GenericGetBricklet(&params,CONVERTED_DATA);
}

int ExecuteGetBrickletMetaData(GetBrickletMetaDataRuntimeParamsPtr p){

	GenericGetBrickletParams params;

	params.calledFromFunction	= p->calledFromFunction;
	params.calledFromMacro		= p->calledFromMacro;

	params.NFlagEncountered		= p->NFlagEncountered;
	params.NFlagParamsSet[0]	= p->NFlagParamsSet[0];
	params.baseName				= p->baseName;

	params.RFlagEncountered		= p->RFlagEncountered;
	params.RFlagParamsSet[0]	= p->RFlagParamsSet[0];
	params.RFlagParamsSet[1]	= p->RFlagParamsSet[1];
	params.startBrickletID		= p->startBrickletID;
	params.endBrickletID		= p->endBrickletID;

	params.SFlagEncountered		= 0;
	params.SFlagParamsSet[0]	= 0;
	params.pixelSize			= 0.0;

	params.DESTFlagEncountered  = p->DESTFlagEncountered;
	params.DESTFlagParamsSet[0] = p->DESTFlagParamsSet[0];
	params.dfref				= p->dfref;

	return GenericGetBricklet(&params,META_DATA);
}

extern "C" int ExecuteGetBrickletRawData(GetBrickletRawDataRuntimeParamsPtr p){

	GenericGetBrickletParams params;

	params.calledFromFunction	= p->calledFromFunction;
	params.calledFromMacro		= p->calledFromMacro;

	params.NFlagEncountered		= p->NFlagEncountered;
	params.NFlagParamsSet[0]	= p->NFlagParamsSet[0];
	params.baseName				= p->baseName;

	params.RFlagEncountered		= p->RFlagEncountered;
	params.RFlagParamsSet[0]	= p->RFlagParamsSet[0];
	params.RFlagParamsSet[1]	= p->RFlagParamsSet[1];
	params.startBrickletID		= p->startBrickletID;
	params.endBrickletID		= p->endBrickletID;

	params.SFlagEncountered		= 0;
	params.SFlagParamsSet[0]	= 0;
	params.pixelSize			= 0.0;

	params.DESTFlagEncountered  = p->DESTFlagEncountered;
	params.DESTFlagParamsSet[0] = p->DESTFlagParamsSet[0];
	params.dfref				= p->dfref;

	return GenericGetBricklet(&params,RAW_DATA);
}

int GenericGetBricklet(GenericGetBrickletParamsPtr p,int typeOfData){
	BEGIN_OUTER_CATCH

	// due to our special handling of the /DEST flag we read the settigs later
	GlobalData::Instance().initializeWithoutReadSettings(p->calledFromMacro,p->calledFromFunction);
	SetOperationStrVar(S_waveNames,"");

	std::string fullPathOfCreatedWaves, baseName;
	BrickletClass *bricklet = NULL;
	char waveName[ARRAY_SIZE], dataFolderName[MAX_OBJ_NAME+1];
	DataFolderHandle destDataFolderHndl = NULL, brickletDataFolderHndl = NULL;
	int brickletID=-1, ret=-1;
	int startBrickletID=-1, endBrickletID=-1;
	int pixelSize;
	bool resampleData;

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
	if(numberOfBricklets == 0){
		GlobalData::Instance().setError(EMPTY_RESULTFILE);
		return 0;
	}

	// By default the range is 1 to totalNumberOfBricklets (aka all bricklets)
	startBrickletID = 1;
	endBrickletID = numberOfBricklets;	

	// the /R flag specifies a single brickletID or a brickletID range
	// Parameter: p->startBrickletID
	// Parameter: p->endBrickletID [optional]
	if(p->RFlagEncountered){
		startBrickletID = int(p->startBrickletID);
		if (p->RFlagParamsSet[1]){// endBrickletID is set
			endBrickletID   = int(p->endBrickletID);
		}
		else{
			endBrickletID  = int(p->startBrickletID); // the range is restricted to one bricklet given by startBrickletID
		}
	}

	sprintf(GlobalData::Instance().outputBuffer,"startBrickletID=%d, endBrickletID=%d",startBrickletID,endBrickletID);
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	if(!isValidBrickletRange(startBrickletID,endBrickletID,numberOfBricklets)){
		GlobalData::Instance().setError(INVALID_RANGE);
		return 0;
	}

	// from here on we have a none empty result set open and a valid bricklet range
	if( p->NFlagEncountered ){
		if( GetHandleSize(p->baseName) == 0L ){
			GlobalData::Instance().setError(WRONG_PARAMETER,"baseName");
			return 0;
		}
		else{
			convertHandleToString(p->baseName,baseName);
		}
	}
	else{
		// use the default name for the waves
		switch(typeOfData){
			case RAW_DATA:
				baseName = brickletRawDefault;
				break;
			case CONVERTED_DATA:
				baseName = brickletDataDefault;
				break;
			case META_DATA:
				baseName = brickletMetaDefault;
				break;
			default:
				outputToHistory("BUG: Error in GenericGetBricklet");
				return 0;
				break;
		}
	}

	// check for possible resample flag (only in case we are dealing with converted data)
	pixelSize=1;
	if( p->SFlagEncountered ){
		resampleData = true;
		pixelSize = int( floor(p->pixelSize) );
		if(pixelSize < 2 || pixelSize > maximum_pixelSize){
			GlobalData::Instance().setError(WRONG_PARAMETER,"pixelSize");
			return 0;
		}
	}
	else{
		resampleData = false;
	}
	sprintf(GlobalData::Instance().outputBuffer,"pixelSize=%d",pixelSize);
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	for(brickletID=startBrickletID; brickletID <= endBrickletID; brickletID++){

		bricklet = GlobalData::Instance().getBrickletClassObject(brickletID);
		ASSERT_RETURN_ZERO(bricklet);

		// check only the length of the wave "baseName"
		// if we don't do it here we can not know an upper limit for char waveName[]
		// if it is too long we abort
		ret = CheckName(NULL,WAVE_OBJECT,baseName.c_str());
		if(ret == NAME_TOO_LONG){
			GlobalData::Instance().setInternalError(ret);
			return ret;
		}

		sprintf(waveName,brickletDataFormat,baseName.c_str(),brickletID);

		// datafolder handling
		if( GlobalData::Instance().datafolderEnabled() ){

			sprintf(dataFolderName,dataFolderFormat,brickletID);
			ret = NewDataFolder(destDataFolderHndl, dataFolderName, &brickletDataFolderHndl);

			// continue if the datafolder alrady exists, abort on all other errors
			if( ret != 0 && ret != FOLDER_NAME_EXISTS ){
				GlobalData::Instance().setInternalError(ret);
				return 0;
			}
		}
		else{
			// we don't use extra datafolders for the bricklets, so they should be created 
			// in destDataFolderHndl, see DEST flag handling above
			brickletDataFolderHndl = destDataFolderHndl;
		}

		switch(typeOfData){
			case RAW_DATA:
				ret = createRawDataWave(brickletDataFolderHndl,waveName,brickletID,fullPathOfCreatedWaves);
				break;
			case CONVERTED_DATA:
				ret = createWaves(brickletDataFolderHndl,waveName,brickletID,resampleData,pixelSize,fullPathOfCreatedWaves);		
				break;
			case META_DATA:
				ret = createAndFillTextWave(bricklet->getBrickletMetaDataKeys(),bricklet->getBrickletMetaDataValues()
											,brickletDataFolderHndl,waveName,brickletID,fullPathOfCreatedWaves);
				break;
			default:
				outputToHistory("Error in GenericGetBricklet");
				return 0;
				break;
		}
		if(!GlobalData::Instance().dataCacheEnabled()){
			bricklet->clearCache();
		}

		if(ret == WAVE_EXIST){
			GlobalData::Instance().setError(ret,waveName);
			return 0;
		}
		else if(ret == INTERNAL_ERROR_CONVERTING_DATA || ret == UNKNOWN_ERROR){
			GlobalData::Instance().setError(ret);
			return 0;
		}
		else if(ret != SUCCESS){
			GlobalData::Instance().setInternalError(ret);
			return 0;
		}

		//check for user abort
		if( SpinProcess() != 0 ){
			break;
		}
	}

	SetOperationStrVar(S_waveNames,fullPathOfCreatedWaves.c_str());

	bool filledCache=true;
	GlobalData::Instance().finalize(filledCache,ret);
	END_OUTER_CATCH
	return 0;
}
