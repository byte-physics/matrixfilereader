/*	XFUNC1.c -- illustrates Igor external functions.

*/

//variable ret = openResultFile("C:\Documents and Settings\brt127\My Documents\gitRepo\Coding\matrix-file-reader\example-data\2009-05-15","default_2009May15-115529_STM-STM_Spectroscopy_0001.mtrx")

#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "XFUNC1.h"

#include "dataclass.h"
#include "error-codes.h"
#include "utils.h"

#include "Vernissage.h"

#define ASSERT(A) { if(A == NULL){ XOPNotice("ASSERT: Pointer " #A " is NULL\r"); return 0; } }

/* Global Variables */

static myData *pMyData = NULL;

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct openResultFileParams{
	Handle resultFileName;
	Handle filePath;
	double result;
};
typedef struct openResultFileParams openResultFileParams;
#pragma pack()

static int openResultFile(openResultFileParams* p){
	
	p->result = UNKOWN_ERROR;

	char filePath[MAX_PATH_LEN+1];
	char fileName[MAX_PATH_LEN+1];
	std::wstring wstringFilePath,wstringFileName;

	int returnValueInt = 0;
	
	returnValueInt = GetCStringFromHandle(p->filePath,filePath,MAX_PATH_LEN);
	if( returnValueInt != 0 ){
		return returnValueInt;
	}

	returnValueInt = GetCStringFromHandle(p->resultFileName,fileName,MAX_PATH_LEN);
	if( returnValueInt != 0 ){
		return returnValueInt;
	}
	
	// check for first time start
	if(pMyData == NULL){
		pMyData = new myData();
	}

	if(pMyData->resultFileOpen()){
		p->result = ALREADY_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();

	if(pSession == NULL){
		XOPNotice("Could not create session object.\r");
		p->result = UNKOWN_ERROR;
		return 0;
	}
	
	XOPNotice("test1\r");
	
	ASSERT(pSession);

	//wstringFilePath.erase();
	//wstringFilePath.append(L"c:\\data");

	//wstringFileName.erase();
	//wstringFileName.append(L"default_2008Sep22-154040_STM-STM_Basic_0001.mtrx");

	wstringFilePath = CharPtrToWString(filePath);
	wstringFileName = CharPtrToWString(fileName);

	char buf[1000];
	sprintf(buf,"filename: %s\r",WStringToString(wstringFileName).c_str());
	XOPNotice(buf);

	sprintf(buf,"filepath: %s\r",WStringToString(wstringFilePath).c_str());
	XOPNotice(buf);

	bool retValue = pSession->loadResultSet(wstringFilePath,wstringFileName,true);

	XOPNotice("test2\r");

	if(!retValue){
		XOPNotice("File is not readable.\r");
		p->result = FILE_NOT_READABLE;
		return 0;
	}

	XOPNotice("test3\r");

	pMyData->setResultFile(wstringFilePath,wstringFileName);
	p->result = SUCCESS;

	XOPNotice("worked\r");

	return 0;					/* XOP error code */
}

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct closeResultFileParams{
	double result;
};
typedef struct closeResultFileParams closeResultFileParams;
#pragma pack()

static int closeResultFile(closeResultFileParams *p)
{
	p->result = UNKOWN_ERROR;

	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return(0);
	}

	Vernissage::Session *pSession = pMyData->getSession();

	ASSERT(pSession);

	pSession->eraseResultSets();
	pMyData->closeSession();
	
	p->result = SUCCESS;

	return(0);					/* XFunc error code */
}

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct getNumberOfBrickletsParams{
	double *totalNumberOfBricklets;	
	double result;
};
typedef struct getNumberOfBrickletsParams getNumberOfBrickletsParams;
#pragma pack()

static int getNumberOfBricklets(getNumberOfBrickletsParams *p){

	p->result = UNKOWN_ERROR;
	*p->totalNumberOfBricklets = -1;

	if(!pMyData->resultFileOpen()){
		p->result = NO_FILE_OPEN;
		return 0;
	}

	Vernissage::Session *pSession = pMyData->getSession();

	ASSERT(pSession);

	*p->totalNumberOfBricklets = pSession->getBrickletCount();

	if(*p->totalNumberOfBricklets == 0){
		p->result = EMPTY_RESULTFILE;
	}
	else{
		p->result = SUCCESS;
	}
	return 0;
}


// FIXME use a std::vector here
static long RegisterFunction()
{
	int funcIndex;

	/*	NOTE:
		Some XOPs should return a result of NIL in response to the FUNCADDRS message.
		See XOP manual "Restrictions on Direct XFUNCs" section.
	*/

	funcIndex = GetXOPItem(0);		/* which function invoked ? */
	switch (funcIndex) {
		case 0:						
			return((long)openResultFile);
			break;
		case 1:						
			return((long)closeResultFile);
			break;
		case 2:						
			return((long)getNumberOfBricklets);
			break;
	}
	return(NIL);
}

/*	XOPEntry()

	This is the entry point from the host application to the XOP for all messages after the
	INIT message.
*/

// FIXME add cleanup message support

static void XOPEntry(void)
{	
	long result = 0;

	switch (GetXOPMessage()) {
		case FUNCADDRS:
			result = RegisterFunction();
			break;
	}
	SetXOPResult(result);
}

/*	main(ioRecHandle)

	This is the initial entry point at which the host application calls XOP.
	The message sent by the host must be INIT.
	main() does any necessary initialization and then sets the XOPEntry field of the
	ioRecHandle to the address to be called for future messages.
*/


// FIXME set needed igor version to 6.1
HOST_IMPORT void
main(IORecHandle ioRecHandle)
{	
	XOPInit(ioRecHandle);							/* do standard XOP initialization */
	SetXOPEntry(XOPEntry);							/* set entry point for future calls */
	
	if (igorVersion < 600)
		SetXOPResult(REQUIRES_IGOR_600);
	else
		SetXOPResult(0L);
}
