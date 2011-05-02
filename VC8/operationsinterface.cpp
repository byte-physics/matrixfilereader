/*
	The file operationsinterface.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "header.h"

#include "operationstructs.h"
#include "operationsinterface.h"

#include <algorithm>
#include <string>
#include <math.h>

#include "utils_bricklet.h"
#include "globaldata.h"
#include "preferences.h"

/*	XOPEntry()

This is the entry point from the host application to the XOP for all messages after the
INIT message.
*/
extern "C" void XOPEntry(void){
	XOPIORecResult result = 0;

	try{
		switch (GetXOPMessage()) {
			case CLEANUP:
				saveXOPPreferences();
				// in case the user has forgotten to close the result file
				if(globDataPtr->resultFileOpen()){
					globDataPtr->closeResultFile();
				}
				// close the session and unload the DLL
				globDataPtr->closeSession();
				delete globDataPtr;
				globDataPtr = NULL;
			break;
		}
	}
	catch(...){
		XOPNotice("Unexpected exception in XOPEntry");
	}
	SetXOPResult(result);
}

/*	XOPMain(ioRecHandle)

This is the initial entry point at which the host application calls XOP.
The message sent by the host must be INIT.

*/

HOST_IMPORT int XOPMain(IORecHandle ioRecHandle){	
	XOPInit(ioRecHandle);							/* do standard XOP initialization */
	SetXOPEntry(XOPEntry);							/* set entry point for future calls */
	int errorCode;

	if (igorVersion < 620){
		SetXOPResult(REQUIRES_IGOR_620);
		return EXIT_FAILURE;
	}

	try{
		globDataPtr = new GlobalData();
	}
	catch(...){
		SetXOPResult(OUT_OF_MEMORY);
		return EXIT_FAILURE;
	}

	// load preferences from file
	loadXOPPreferences();

	if (errorCode = RegisterOpenResultFile()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterCloseResultFile()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetBrickletCount()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetResultFileName()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetVernissageVersion()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetMtrxFileReaderVersion()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetBrickletData()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetBrickletMetaData()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetBrickletRawData()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetReportTemplate()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterCreateOverviewTable()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetResultFileMetaData()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterCheckForNewBricklets()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}
	if (errorCode = RegisterGetXOPErrorMessage()) {
		SetXOPResult(errorCode);
		return EXIT_FAILURE;
	}

	SetXOPResult(0L);
	return EXIT_SUCCESS;
}
