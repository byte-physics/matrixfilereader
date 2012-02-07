/*
	The file operationsinterface_createoverviewtable.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.h"
#include "operationsinterface.h"
#include "globaldata.h"
#include "utils_bricklet.h"
#include "brickletclass.h"
#include "utils_generic.h"

extern "C" int ExecuteCreateOverviewTable(CreateOverviewTableRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	GlobalData::Instance().initializeWithoutReadSettings(p->calledFromMacro,p->calledFromFunction);
	SetOperationStrVar(S_waveNames,"");

	int ret=-1;
	std::string keyList, key, value, waveName;
	waveHndl waveHandle;
	DataFolderHandle destDataFolderHndl = NULL;
	BrickletClass *bricklet=NULL;
	int i;
	unsigned int j;

	CountInt dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));
	std::vector<std::string> keys, textWaveContents;

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

	// check keyList parameter
	if (p->KEYSFlagEncountered) {
		if( GetHandleSize(p->keyList) == 0L ){
			GlobalData::Instance().setError(WRONG_PARAMETER,"keyList");
			return 0;
		}
		else{
			convertHandleToString(p->keyList,keyList);
		}
	}
	else{
		keyList = keyList_default;
	}

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
		waveName = overViewTableDefault;
	}

	splitString(keyList,listSepChar,keys);

	if( keys.empty() ){
		GlobalData::Instance().setError(WRONG_PARAMETER,"keyList");
		return 0;
	}

	dimensionSizes[ROWS] = numberOfBricklets;
	dimensionSizes[COLUMNS] = keys.size();

	ret = MDMakeWave(&waveHandle,waveName.c_str(),destDataFolderHndl,dimensionSizes,TEXT_WAVE_TYPE,GlobalData::Instance().overwriteEnabledAsInt());
	if(ret == NAME_WAV_CONFLICT){
		sprintf(GlobalData::Instance().outputBuffer,"Wave %s already exists.",waveName.c_str());
		debugOutputToHistory(GlobalData::Instance().outputBuffer);
		GlobalData::Instance().setError(WAVE_EXIST,waveName);
		return 0;
	}
	else if(ret != 0 ){
		GlobalData::Instance().setInternalError(ret);
		return 0;
	}

	ASSERT_RETURN_ZERO(waveHandle);

	for(j=0; j < keys.size(); j++){

		key = keys.at(j);
		MDSetDimensionLabel(waveHandle,COLUMNS,j,key.c_str());
		sprintf(GlobalData::Instance().outputBuffer,"key=%s",key.c_str());
		debugOutputToHistory(GlobalData::Instance().outputBuffer);

		for(i=1; i <= numberOfBricklets; i++){
			bricklet = GlobalData::Instance().getBrickletClassObject(i);
			ASSERT_RETURN_ZERO(bricklet);
			value = bricklet->getMetaDataValueAsString(key);
			textWaveContents.push_back(value);

			sprintf(GlobalData::Instance().outputBuffer,"   value=%s",value.c_str());
			debugOutputToHistory(GlobalData::Instance().outputBuffer);
		}
	}

	ret = stringVectorToTextWave(textWaveContents,waveHandle);
	if(ret != 0){
		GlobalData::Instance().setInternalError(ret);
		return 0;
	}

	setOtherWaveNote(waveHandle);

	SetOperationStrVar(S_waveNames,getFullWavePath(destDataFolderHndl,waveHandle).c_str());
	bool clearCache=true;
	GlobalData::Instance().finalize(clearCache);
	END_OUTER_CATCH
	return 0;
}
