/*
	The file operationsinterface_getversion.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.h"
#include "operationsinterface.h"
#include "globaldata.h"
#include "utils_generic.h"

extern "C" int ExecuteGetVersion(GetVersionRuntimeParamsPtr p){

	SetOperationNumVar(V_XOPversion,stringToAnyType<double>(MatrixFileReader_XOP_VERSION_STR));
	return 0;
}