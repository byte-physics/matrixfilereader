#include "header.h"

#include "operationstructs.h"

/* custom error codes */

#define REQUIRES_IGOR_620	FIRST_XOP_ERR + 1
#define OUT_OF_MEMORY		FIRST_XOP_ERR + 2

/* Prototypes */
HOST_IMPORT int main(IORecHandle ioRecHandle);
static void XOPEntry();
