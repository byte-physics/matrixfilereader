/*
  The file constants.h is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

/*
  Global constants
  Most strings are part of the Igor interface
*/
#pragma once

const int ARRAY_SIZE = 1024;

// format string for wavenames in igor
// The total number of bricklets is limited to 10^5 - 1
const char brickletWaveFormat[] = "%s_%05d";
const char dataFolderFormat[]   = "X_%05d";

// default wave and datafolder names
const char brickletDataDefault[]   = "data";
const char brickletMetaDefault[]   = "metaData";
const char brickletDeployDefault[] = "deployData";
const char resultMetaDefault[]     = "resultFileMetaData";
const char overViewTableDefault[]  = "overViewTable";
const char brickletRawDefault[]    = "rawData";

// bricklet types as string
// from the vernissage header file
const char BTC_UNKNOWN[]           = "btc_Unknown";
const char BTC_SPMSPECTROSCOPY[]   = "btc_SPMSpectroscopy";
const char BTC_ATOMMANIPULATION[]  = "btc_AtomManipulation";
const char BTC_1DCURVE[]           = "btc_1DCurve";
const char BTC_SPMIMAGE[]          = "btc_SPMImage";
const char BTC_PATHSPECTROSCOPY[]  = "btc_PathSpectroscopy";
const char BTC_ESPREGION[]         = "btc_ESpRegion";
const char BTC_VOLUMECITS[]        = "btc_VolumeCITS";
const char BTC_DISCRETEENERGYMAP[] = "btc_DiscreteEnergyMap";
const char BTC_FORCECURVE[]        = "btc_ForceCurve";
const char BTC_PHASEAMPLITUDECUR[] = "btc_PhaseAmplitudeCurve";
const char BTC_SIGNALOVERTIME[]    = "btc_SignalOverTime";
const char BTC_RAWPATHSPEC[]       = "btc_RawPathSpectroscopy";
const char BTC_ESPSNAPSHOTSEQ[]    = "btc_ESpSnapshotSequence";
const char BTC_ESPIMAGEMAP[]       = "btc_ESpImageMap";
const char BTC_INTERFEROMETERCUR[] = "btc_InterferometerCurve";
const char BTC_ESPIMAGE[]          = "btc_ESpImage";

// viewtype codes as strings
// from the vernissage header file
const char VTC_OTHER[]             = "vtc_Other";
const char VTC_SIMPLE2D[]          = "vtc_Simple2D";
const char VTC_SIMPLE1D[]          = "vtc_Simple1D";
const char VTC_FWDBWD2D[]          = "vtc_ForwardBackward2D";
const char VTC_2DOF3D[]            = "vtc_2Dof3D";
const char VTC_SPECTROSCOPY[]      = "vtc_Spectroscopy";
const char VTC_FORCECURVE[]        = "vtc_ForceCurve";
const char VTC_1DPROFILE[]         = "vtc_1DProfile";
const char VTC_INTERFEROMETER[]    = "vtc_Interferometer";
const char VTC_CONTINUOUSCURVE[]   = "vtc_ContinuousCurve";
const char VTC_PHASEAMPLITUDECUR[] = "vtc_PhaseAmplitudeCurve";
const char VTC_CURVESET[]          = "vtc_CurveSet";
const char VTC_PARAMETERISEDCUR[]  = "vtc_ParameterisedCurveSet";
const char VTC_DISCRETEENERGYMAP[] = "vtc_DiscreteEnergyMap";
const char VTC_ESPIMAGEMAP[]       = "vtc_ESpImageMap";
const char VTC_DOWNWARD2D[]        = "vtc_Downward2D";

// wave name suffixes
const char TRACE_UP_STRING[]      = "_Up";
const char TRACE_DOWN_STRING[]    = "_Down";
const char RE_TRACE_UP_STRING[]   = "_ReUp";
const char RE_TRACE_DOWN_STRING[] = "_ReDown";

// used for WaveClass
const int NO_TRACE      = -1;
const int TRACE_UP      = 0;
const int RE_TRACE_UP   = 1;
const int TRACE_DOWN    = 2;
const int RE_TRACE_DOWN = 3;

const int TRACE_UP_RAMP_FWD      = 0;
const int RE_TRACE_UP_RAMP_FWD   = 1;
const int TRACE_DOWN_RAMP_FWD    = 2;
const int RE_TRACE_DOWN_RAMP_FWD = 3;
const int TRACE_UP_RAMP_BWD      = 4;
const int RE_TRACE_UP_RAMP_BWD   = 5;
const int TRACE_DOWN_RAMP_BWD    = 6;
const int RE_TRACE_DOWN_RAMP_BWD = 7;

// number of different trace types, TRACE_UP to RE_TRACE_DOWN x 2 for Ramp Reversal
const int MAX_NUM_WAVES = 8;

const char PROJECTURL[] = "http://www.igorexchange.com/project/matrixFileReader";

// number of entries to reserve in some vectors
const int RESERVE_SIZE = 1024;

// useful for MDSetWaveUnits, defined in accordance to ROWS, COLUMNS, etc.
const int DATA = -1;

// functions returning a brickletID should return that on failure
const int INVALID_BRICKLETID = -1;

// default values for the options
const bool debug_default      = false;
const bool double_default     = true;
const bool datafolder_default = true;
const bool overwrite_default  = false;
const bool cache_default      = true;
const bool magic_default      = false;

// All possible values for magic_option
// don't scale the converted data
const int identity_transformation = 0x1;
const int artifical_data          = 0x2;

const char debug_option[]      = "V_MatrixFileReaderDebug";
const char datafolder_option[] = "V_MatrixFileReaderFolder";
const char overwrite_option[]  = "V_MatrixFileReaderOverwrite";
const char double_option[]     = "V_MatrixFileReaderDouble";
const char cache_option[]      = "V_MatrixFileReaderCache";
const char magic_option[]      = "V_MatrixFileReaderMagic";

// Supported major vernissage versions
const std::vector<std::string> supportedMajorVernissageVersions = boost::assign::list_of("2.4");

// default keys for MFR_CreateOverViewTable
const char keyList_default[] = "brickletID;scanCycleCount;runCycleCount;sequenceID;dimension;channelName";

// runtime variables, returned by the operations
const char V_flag[]            = "V_flag";
const char V_startBrickletID[] = "V_startBrickletID";
const char V_endBrickletID[]   = "V_endBrickletID";
const char V_count[]           = "V_count";
const char V_XOPversion[]      = "V_XOPversion";
const char V_DLLversion[]      = "V_DLLversion";

const char S_fileName[]     = "S_fileName";
const char S_dirPath[]      = "S_dirPath";
const char S_value[]        = "S_value";
const char S_waveNames[]    = "S_waveNames";
const char S_errorMessage[] = "S_errorMessage";

// Openresultfile dialog
const char filterStr[] = "Matrix result files (*.mtrx)\0*.mtrx\0All Files (*.*)\0*.*\0\0";
const char dlgPrompt[] = "Looking for a MATRIX result file";

const int default_pixelsize = 1;
// maximum allowed pixelSize, although this is already ridiculously high
const int maximum_pixelSize = 10;

const char listSepChar[]          = ";";
const char relativeFolderPrefix[] = ":";

const char suffixName[]    = "suffix";
const char suffixRampFwd[] = "_RampFwd";
const char suffixRampBwd[] = "_RampBwd";
