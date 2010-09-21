
#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#define ARRAY_SIZE 1000

#include "error-codes.h"

#include "dataclass.h"
extern myData *pMyData;

// format string for wavenames in igor

const char brickletDataDefault[]	 = "brickletData";
const char brickletMetaDefault[]	 = "brickletMetaData";
const char resultMetaDefault[]		 = "resultFileMetaData";
const char overViewTableDefault[]	 = "overViewTable";
const char rawBrickletFormatString[] = "brickletRawData_%04d";
const char brickletMetaDataFormat[]	 = "%s_%04d";
const char brickletDataFormat[]		 = "%s_%04d";
const char dataFolderFormat[]		 = "X_%04d";

// value types from element instance parameters
const char VT_SPECIAL_STRING[] = "vt_Special";
const char VT_INTEGER_STRING[] = "vt_Integer";
const char VT_DOUBLE_STRING[]  = "vt_Double";
const char VT_BOOLEAN_STRING[] = "vt_Boolean";
const char VT_ENUM_STRING[]	   = "vt_Enum";
const char VT_STRING_STRING[]  = "vt_String";

// viewtype codes as strings
const char VTC_OTHER_STRING[]		= "vtc_Other";
const char VTC_SIMPLE2D_STRING[]	= "vtc_Simple2D";
const char VTC_SIMPLE1D_STRING[]	= "vtc_Simple1D";
const char VTC_FWDBWD2D_STRING[]	= "vtc_ForwardBackward2D";
const char VTC_2DOF3D_STRING[]		= "vtc_2Dof3D";
const char VTC_SPECTROSCOPY_STRING[]= "vtc_Spectroscopy";
const char VTC_FORCECURVE_STRING[]	= "vtc_ForceCurve";
const char VTC_1DPROFILE_STRING[]	= "vtc_1DProfile";
const char VTC_INTERFEROMETER[]		= "vtc_Interferometer";

// wave name suffixes
const char TRACE_UP_STRING[]		= "_TraceUp";
const char TRACE_DOWN_STRING[]		= "_TraceDown";
const char RE_TRACE_UP_STRING[]		= "_ReTraceUp";
const char RE_TRACE_DOWN_STRING[]	= "_ReTraceDown";

// as all parameters to XOP functions are of type double one has to define an epsilon value when one compares them to a given int value
const double doubleEpsilon = 1e-5;

// number of key to reserve space for in the metadata std::vector
const int  METADATA_RESERVE_SIZE	= 1000;

// default values for the options
const bool debug_default	= false;
const bool double_default = true;
const bool datafolder_default = true;
const bool overwrite_default	= false;

const char debug_option_name[] = "debug";
const char folder_option_name[] = "folder";
const char overwrite_option_name[] = "overwrite";
const char double_option_name[] = "double";

// only a few vernissage DLL version are bug-free enough to be used 
const char properVernissageVersion[] ="1.0";

#endif // GLOBALVARIABLES_H