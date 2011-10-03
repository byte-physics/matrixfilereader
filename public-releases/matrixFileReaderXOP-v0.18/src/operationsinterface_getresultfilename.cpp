/*
	The file operationsinterface_getresultfilename.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "header.h"

#include "operationstructs.h"
#include "operationsinterface.h"

#include "globaldata.h"

extern "C" int ExecuteGetResultFileName(GetResultFileNameRuntimeParamsPtr p){

	globDataPtr->initialize(p->calledFromMacro,p->calledFromFunction);
	SetOperationStrVar(S_fileName,"");
	SetOperationStrVar(S_dirPath,"");

	if(!globDataPtr->resultFileOpen()){
		globDataPtr->setError(NO_FILE_OPEN);
		return 0;
	}

	SetOperationStrVar(S_fileName,globDataPtr->getFileName().c_str());
	SetOperationStrVar(S_dirPath,globDataPtr->getDirPath().c_str());

	globDataPtr->finalize();
	return 0;
}
