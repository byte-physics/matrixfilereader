/*
	The file xop_const_wrappers.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

/*
	Wrapper functions for the XOP Toolkit to allow passing a const char instead of a char
*/

#pragma once

#include <string.h>

#include "XOP_Toolkit_6/IgorXOPs6/XOPSupport/xopstandardheaders.h"

int MDSetDimensionLabel(waveHndl waveH, int dimension, IndexInt element, const char label[MAX_DIM_LABEL_CHARS+1]);
