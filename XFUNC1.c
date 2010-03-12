/*	XFUNC1.c -- illustrates Igor external functions.

*/

#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "XFUNC1.h"

/* Global Variables (none) */

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
struct XFUNC1AddParams  {
	double p2;
	double p1;
	double result;
};
typedef struct XFUNC1AddParams XFUNC1AddParams;
#include "XOPStructureAlignmentReset.h"

static int
XFUNC1Add(XFUNC1AddParams* p)
{
	p->result = p->p1 + p->p2;
	
	return(0);					/* XFunc error code */
}


#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
struct XFUNC1DivParams  {
	double p2;
	double p1;
	double result;
};
typedef struct XFUNC1DivParams XFUNC1DivParams;
#include "XOPStructureAlignmentReset.h"

static int
XFUNC1Div(XFUNC1DivParams* p)
{
	p->result = p->p1 / p->p2;
	
	return(0);					/* XFunc error code */
}


struct DPComplexNum {
	double real;
	double imag;
};

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
struct XFUNC1ComplexConjugateParams  {
	struct DPComplexNum p1;					// Complex parameter
	struct DPComplexNum result;				// Complex result
};
typedef struct XFUNC1ComplexConjugateParams XFUNC1ComplexConjugateParams;
#include "XOPStructureAlignmentReset.h"

static int
XFUNC1ComplexConjugate(XFUNC1ComplexConjugateParams* p)
{
	p->result.real = p->p1.real;
	p->result.imag = -p->p1.imag;

	return 0;
}


static long
RegisterFunction()
{
	int funcIndex;

	/*	NOTE:
		Some XOPs should return a result of NIL in response to the FUNCADDRS message.
		See XOP manual "Restrictions on Direct XFUNCs" section.
	*/

	funcIndex = GetXOPItem(0);		/* which function invoked ? */
	switch (funcIndex) {
		case 0:						/* XFUNC1Add(p1, p2) */
			return((long)XFUNC1Add);
			break;
		case 1:						/* XFUNC1Div(p1, p2) */
			return((long)XFUNC1Div);
			break;
		case 2:						/* XFUNC1ComplexConjugate(p1) */
			return((long)XFUNC1ComplexConjugate);
			break;
	}
	return(NIL);
}

/*	XOPEntry()

	This is the entry point from the host application to the XOP for all messages after the
	INIT message.
*/

static void
XOPEntry(void)
{	
	long result = 0;

	switch (GetXOPMessage()) {
		//case FUNCTION:								/* our external function being invoked ? */
		//	result = DoFunction();
		//	break;

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

HOST_IMPORT void
main(IORecHandle ioRecHandle)
{	
	XOPInit(ioRecHandle);							/* do standard XOP initialization */
	SetXOPEntry(XOPEntry);							/* set entry point for future calls */
	
	if (igorVersion < 200)
		SetXOPResult(REQUIRES_IGOR_200);
	else
		SetXOPResult(0L);
}
