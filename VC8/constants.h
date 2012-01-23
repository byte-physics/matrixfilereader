/*
	The file constants.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

/*
	Global constants
	Most strings are part of the Igor interface
	The total number of bricklets is limited to 10^5, see brickletDataFormat and friends
*/

#pragma once

const int ARRAY_SIZE = 1024;

// format string for wavenames in igor

const char brickletDataDefault[]	 = "data";
const char brickletMetaDefault[]	 = "metaData";
const char resultMetaDefault[]		 = "resultFileMetaData";
const char overViewTableDefault[]	 = "overViewTable";
const char brickletRawDefault[]		 = "rawData";
const char brickletMetaDataFormat[]	 = "%s_%05d";
const char brickletDataFormat[]		 = "%s_%05d";
const char dataFolderFormat[]		 = "X_%05d";

// value types from element instance parameters
// from the vernissage header file
const char VT_SPECIAL_STRING[] = "vt_Special";
const char VT_INTEGER_STRING[] = "vt_Integer";
const char VT_DOUBLE_STRING[]  = "vt_Double";
const char VT_BOOLEAN_STRING[] = "vt_Boolean";
const char VT_ENUM_STRING[]	   = "vt_Enum";
const char VT_STRING_STRING[]  = "vt_String";

// viewtype codes as strings
// from the vernissage header file
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
const char TRACE_UP_STRING[]		= "_Up";
const char TRACE_DOWN_STRING[]		= "_Down";
const char RE_TRACE_UP_STRING[]		= "_ReUp";
const char RE_TRACE_DOWN_STRING[]	= "_ReDown";

// used for WaveClass
const int NO_TRACE					=-1;
const int TRACE_UP 					= 0;
const int RE_TRACE_UP 				= 1;
const int TRACE_DOWN				= 2;
const int RE_TRACE_DOWN				= 3;

// number of different trace types, TRACE_UP to RE_TRACE_DOWN
const int MAX_NUM_TRACES			= 4;

const char PROJECTURL[]				= "http://www.igorexchange.com/project/matrixFileReader";

// number of keys to reserve space for in the metadata std::vector
const int  METADATA_RESERVE_SIZE	= 1024;

// useful for MDSetWaveUnits, defined in accordance to ROWS, COLUMNS, etc.
const int DATA = -1;

// default values for the options
const bool debug_default		= false;
const bool double_default		= true;
const bool datafolder_default	= true;
const bool overwrite_default	= false;
const bool cache_default		= true;

const char debug_option_name[]		= "V_MatrixFileReaderDebug";
const char datafolder_option_name[] = "V_MatrixFileReaderFolder";
const char overwrite_option_name[]	= "V_MatrixFileReaderOverwrite";
const char double_option_name[]		= "V_MatrixFileReaderDouble";
const char cache_option_name[]		= "V_MatrixFileReaderCache";

// Due to historic problems with different vernissage version, we currently only support exactly one version
const char properVernissageVersion[] ="2.0";

// default keys for MFR_CreateOverViewTable
const char keyList_default[] = "brickletID;scanCycleCount;runCycleCount;sequenceID;dimension;channelName";

// runtime variables, returned by the operations
const char V_flag[]				= "V_flag";
const char V_startBrickletID[]	= "V_startBrickletID";
const char V_endBrickletID[]	= "V_endBrickletID";
const char V_count[]			= "V_count";
const char V_XOPversion[]		= "V_XOPversion";
const char V_DLLversion[]		= "V_DLLversion";

const char S_fileName[]			= "S_fileName";
const char S_dirPath[]			= "S_dirPath";
const char S_value[]			= "S_value";
const char S_waveNames[]		= "S_waveNames";
const char S_errorMessage[]		= "S_errorMessage";

// Openresultfile dialog
const char filterStr[] = "Matrix result files (*.mtrx)\0*.mtrx\0All Files (*.*)\0*.*\0\0";
const char dlgPrompt[] = "Looking for a MATRIX result file";

//maximum allowed pixelSize, although this is already ridiculously high
const int maximum_pixelSize = 10;

const char listSepChar[] = ";";