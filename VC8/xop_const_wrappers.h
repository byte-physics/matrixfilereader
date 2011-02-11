#pragma once

#include <string.h>

#include "XOP_Toolkit_6/IgorXOPs6/XOPSupport/xopstandardheaders.h"

int MDSetDimensionLabel(waveHndl waveH, int dimension, IndexInt element, const char constLabel[MAX_DIM_LABEL_CHARS+1]);