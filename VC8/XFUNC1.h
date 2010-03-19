/*
	XFUNC1.h -- equates for XFUNC1 XOP
*/

/* XFUNC1 custom error codes */

#define REQUIRES_IGOR_504	FIRST_XOP_ERR + 1
#define OUT_OF_MEMORY		FIRST_XOP_ERR + 2

const char myVersion[] = "0.01";

/* Prototypes */
HOST_IMPORT void main(IORecHandle ioRecHandle);

static void XOPEntry();
static long RegisterFunction();
void doCleanup();
