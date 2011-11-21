/*
	The file operationsinterface_checkfornewbricklets.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.h"
#include "operationsinterface.h"
#include "globaldata.h"
#include "brickletclass.h"
#include "utils_generic.h"

extern "C" int ExecuteCheckForNewBricklets(CheckForNewBrickletsRuntimeParamsPtr p){
	BEGIN_OUTER_CATCH
	GlobalData::Instance().initialize(p->calledFromMacro,p->calledFromFunction);

	void* pContext  = NULL, *pBricklet = NULL;
	BrickletClass *bricklet = NULL;
	std::wstring fileName, dirPath;
	bool loadSuccess;
	int i,ret;

	// save defaults
	SetOperationNumVar(V_startBrickletID,-1.0);
	SetOperationNumVar(V_endBrickletID,-1.0);

	if(!GlobalData::Instance().resultFileOpen()){
		GlobalData::Instance().setError(NO_FILE_OPEN);
		return 0;
	}

	Vernissage::Session *pSession = GlobalData::Instance().getVernissageSession();
	ASSERT_RETURN_ZERO(pSession);

	const int oldNumberOfBricklets = pSession->getBrickletCount();

	fileName = GlobalData::Instance().getFileNameWString();
	dirPath = GlobalData::Instance().getDirPathWString();

	// true -> result set will be added to the database
	// false -> replaces the current results sets in the internal databse 
	loadSuccess = pSession->loadResultSet(dirPath,fileName,false);

	if(!loadSuccess){
		outputToHistory("Could not check for updates of the result file. Maybe it was moved?");
		outputToHistory("Try closing and opening the result file again.");
		return 0;
	}

	// starting from here we have to
	// - update the pBricklet pointers in the BrickletClass objects
	// - compare old to new totalNumberOfBricklets
	const int numberOfBricklets = pSession->getBrickletCount();

	for(i=1; i <= numberOfBricklets; i++ ){
		pBricklet = pSession->getNextBricklet(&pContext);
		ASSERT_RETURN_ZERO(pBricklet);

		bricklet = GlobalData::Instance().getBrickletClassObject(i);

		if(bricklet == NULL){// this is a new bricklet
			GlobalData::Instance().createBrickletClassObject(i,pBricklet);
		}
		else{	// the bricklet is old and we only have to update *pBricklet
			bricklet->setBrickletPointer(pBricklet);
		}
	}

	// should not happen
	if(numberOfBricklets < oldNumberOfBricklets){
		outputToHistory("Error in udating the result file. Please close and reopen it.");
		return 0;
	}

	// from here on we know that numberOfBricklets >= oldNumberOfBricklets 
	if(oldNumberOfBricklets == numberOfBricklets){
		GlobalData::Instance().setError(NO_NEW_BRICKLETS);
		return 0;
	}

	ret = SetOperationNumVar(V_startBrickletID,oldNumberOfBricklets+1);
	if(ret != 0){
		GlobalData::Instance().setInternalError(ret);
		return 0;
	}

	ret = SetOperationNumVar(V_endBrickletID,numberOfBricklets);
	if(ret != 0){
		GlobalData::Instance().setInternalError(ret);
		return 0;
	}

	GlobalData::Instance().finalize();
	END_OUTER_CATCH
	return 0;
}
