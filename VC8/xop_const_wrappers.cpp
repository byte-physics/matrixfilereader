#include "xop_const_wrappers.h"

int MDSetDimensionLabel(waveHndl waveH, int dimension, IndexInt element, const char constLabel[MAX_DIM_LABEL_CHARS+1]){

	char nonConstLabel[MAX_DIM_LABEL_CHARS+1];

	strncpy(nonConstLabel,constLabel,MAX_DIM_LABEL_CHARS+1);
	nonConstLabel[MAX_DIM_LABEL_CHARS] = '\0';

	return MDSetDimensionLabel(waveH, dimension, element,nonConstLabel);
};
