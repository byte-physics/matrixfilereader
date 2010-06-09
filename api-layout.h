/** @file api-layout.h
 * @brief API between XOP and Igor Pro Procedures. The XOP serves as a wrapper over the Vernissage DLLs because one can not talk to DLLs from Igor directly.
 *
 * @version 0.12
 * @date Mar/5/2010
*/

/** @cond
 *  make the C compiler happy
*/
typedef char* HSTRING_TYPE;
typedef double NT_FP64;
typedef HSTRING_TYPE string;
typedef NT_FP64 variable;
typedef void* WAVE_TYPE;
typedef WAVE_TYPE TEXTWAVE;
typedef WAVE_TYPE FP64WAVE;
// @endcond

/** @defgroup todoItems TODO List
 * - TODO Where should we put the experiment deployment parameters, also in the metadata wave for each bricklet?
 * - TODO make function names more distinct, maybe prefix with "matrixReader_"
 * - TODO document dataTypes in getBrickletData() properly
 * - TODO add Igor code for errorCode struct
 * - TODO do we really need UNKNOWN_ERROR
 * - Add wave notes stuff
*/

 /** @mainpage General Remarks
 * - The data types "string" and "variable" denote the Igor Pro data types 
 * - All functions return a floating point value as error code. This error code is either zero (everything is okay) or non-zero in case something went wrong. In this manual these error codes can be translated to human readable strings. See the errorCode struct in error-codes.h for the numeric value. The reason it is a floating point is due to Igor Pro XOP limitations.
 *  These API functions can be called in Igor like
 *  @code
 *  if( openResultFile() == errorCode.SUCCESS )
 *  	// call succeeded
 *  else
 *	// do error handling
 *  endif
 *  @endcode
 * - For rare cases the error code UNKNOWN_ERROR might be returned. Obviously in this case something went really badly wrong and the XOP does not know why.
 * - All waves hold wave notes of the form:
 *   	@code
 *   		xopVersion=0.1
 *   		vernissageVersion=1.0
 *   		resultFilePath=c:\data
 *			resultFileName=10MicronLong-GNR_001.mtrx
 *   		brickletID=28
 *   	@endcode
 *   	In case the brickletID does not make any sense, the entry is "brickletID=".
 * - Textwaves with (nx2) dimensions must never be considered ordered in any sense. Always check in the first column for a key and then readout the value from same row and the second column. 
 * 	Textwave layout:
 *	@code
 *	| key4 	| value4|
 *	| key1 	| value1|
 *	| key2 	| value2|
 *	| key3 	| value3|  <- last entry because
 *	@endcode
 * - BrickletIDs are 1-based and range from 1 to totalNumberOfBricklets. It is guaranteed that these IDs do not change even after closing and opening the result file again.
 * - Result file keys for metadata textwave:
 *		- filenName
 *		- filePath
 *		- BrickletMetaData.fileCreatorName
 *		- BrickletMetaData.fileCreatorVersion
 *		- BrickletMetaData.userName
 *		- BrickletMetaData.accountName
 *		- totalNumberOfBricklets
 *		- timeStampOfLastChange (this will have the timestamp of the newest bricklet)
*/

/** 
 *  Return a human readable error string, applications should show the user this string instead of the error code
 *  @param[in] errorCode return value from the functions here
 *  @return string with error message, an empty string in case the error code is not found
*/ 
string getErrorMessage(variable errorCode);

/** IMPLEMENTED
 *  Open a matrix result file for further treating. Usually you want to do that as first step.
 *  @param[in] absoluteFilePath absolute path for an result file to open. This is @b not a Igor symbolic path.
 *  @param[in] fileName fileName of the result file
 *  @return SUCCESS | FILE_NOT_READABLE | ALREADY_FILE_OPEN
*/
variable openResultFile(string absoluteFilePath, string fileName);

/** IMPLEMENTED
 *  Close the result file. This will free the memory used for the result file and all of its bricklets in the Vernissage DLLs.
 *  A call to openResultFile() is needed if you want to work with a result file again
 *  @return SUCCESS | NO_FILE_OPEN
*/
variable closeResultFile();

/** IMPLEMENTED
 *  @param[out] filename of the opened result file
 *  @return SUCCESS | NO_FILE_OPEN | WRONG_PARAMETER
*/ 
variable getResultFileName(string *filename);

/** IMPLEMENTED
 *  @param[out] absoluteFilePath absolute path of the opened result file
 *  @return SUCCESS | NO_FILE_OPEN | WRONG_PARAMETER
*/  
variable getResultFilePath(string *absoluteFilePath);

/** IMPLEMENTED
 * Get the version of this XOP. The version number has the form: "<digit>.<digit><digit>"
 * @param[out] xopVersion
 * @return SUCCESS | WRONG_PARAMETER
*/ 
variable getXOPVersion(variable *xopVersion);

/** IMPLEMENTED
 * Get the vernissage DLL version. It returns the version "x.y" from part of the registry key HKEY_LOCAL_MACHINE\Software\Omicron NanoTechnology\Vernissage\Vx.y\Main\InstallDirectory.
 * @param[out] vernissageVersion 
 * @return SUCCESS| WRONG_PARAMETER
*/
variable getVernissageVersion(variable *vernissageVersion);

/** IMPLEMENTED
 *  @param[out] totalNumberOfBricklets total number of bricklets of the result file
 *  @return SUCCESS | NO_FILE_OPEN | EMPTY_RESULTFILE| WRONG_PARAMETER
*/
variable getNumberOfBricklets(variable *totalNumberOfBricklets);

/** IMPLEMENTED
 *  Get the result file meta data. See general remarks about textwaves.
 *  @param[out] waveName  name for a textwave (nx2) dims holding all relevant information of the complete experiment/result file.
 *  @return SUCCESS | NO_FILE_OPEN | WAVE_EXIST | WRONG_PARAMETER
*/
variable getResultFileMetaData(string waveName);

/**
 * Check if new bricklets were added after the openResultFile() call. In case we got no new bricklets NO_NEW_BRICKLETS is returned and both startBrickletID and endBrickletID are set to -1.
 * @param[out] 	startBrickletID first new brickletID
 * @param[out] 	endBrickletID last new brickletID
 * @param[in]  	rememberCalls can be 1 (true) or 0 (false). If true, the call to  checkForNewBricklets() is internally remembered and succesive calls to it only return sucessfully, if the result file changed again. Useful for checking for updates in an endless loop.
 * @return SUCCESS | NO_NEW_BRICKLETS | NO_FILE_OPEN | WRONG_PARAMETER
 */
variable checkForNewBricklets(variable *startBrickletID,variable *endBrickletID,variable rememberCalls);

/** Get the bricklet viewTypeCodes of one bricklet as wave. In nearly all cases this wave will consist only of one entry. The numbers have the following correspondence with the names in the Vernissage DLLs (taken from Vernissage.h)
 *	@code
 *      vtc_Other =0
 *      vtc_Simple2D = 1
 *      vtc_Simple1D = 2
 *      vtc_ForwardBackward2D = 3
 *      vtc_2Dof3D = 4
 *      vtc_Spectroscopy=5
 *      vtc_ForceCurve=6
 *      vtc_1DProfile=7
 *      @endcode
 * @sa getBrickletMetaData()
 * @param[in]  brickletID
 * @param[out] allViewTypeCodes wave with all viewTypeCodes of the given bricklet
 * @return SUCCESS | NON_EXISTENT_BRICKLET | NO_FILE_OPEN | EMPTY_RESULTFILE
*/
// variable getBrickletViewTypeCode(variable brickletID, FP64WAVE allViewTypeCodes);

/** Return the bricklets dimension, this together with the viewtypeCode is important for evaluating the rawData from getBrickletRawData().
 * @param[in]  brickletID
 * @param[out] brickletDimension, a integer number greater than zero
 * @sa getBrickletMetaData()
 * @return SUCCESS | NON_EXISTENT_BRICKLET | NO_FILE_OPEN | EMPTY_RESULTFILE
*/ 
// variable getBrickletDimension(variable brickletID, variable *brickletDimension);

// removed for now
/* Return the dimension of all bricklets
 * @param[out] brickletDimension wave holding the dimension data
 * @return SUCCESS | NO_FILE_OPEN | EMPTY_RESULTFILE
*/ 
// variable getAllBrickletDimension(FP64WAVE brickletDimension);

/** IMPLEMENTED as wrapper to getRangeBrickletData
 * Get the processed data for the bricklet. Processed here means it is put in the correct form as described in the Vernissage manual "Accessing raw data" p. 40. @b No other processing is done. The waves for each bricklet will 32bit integer waves with the appropriate dimension put into the current datafolder. In case a wave exists, the function returns WAVE_EXIST. In case you don't get what you expect here (or you get the return value INTERNAL_ERROR_CONVERTING_DATA), try getBrickletRawData() and do the low-level data twiddling yourself. For this you definitely have to consult the Vernissage Manual. In case of INTERNAL_ERROR_CONVERTING_DATA please submit a bugreport including a testcase which shows the error.
 * @param[in] basename first part of the wavename which will be created.
 * The wavenames are in general of the form <baseName>_<brickletID>_<dataType>, in case the basename is empty, the wavename will be X_<brickletID>_<dataType>. The string dataType is needed in case the number of datawaves is greater than one (usually the case for topographic data). If the number of data waves is one, the wavenames will be <baseName>_<brickletID>. 
 * @param[in]  separateFolderForEachBricklet must be zero or non-zero, create a subfolder named as the X_brickletID for each bricklet
 * @param[in]  brickletID
 * @return SUCCESS | NON_EXISTENT_BRICKLET | WAVE_EXIST | NO_FILE_OPEN | EMPTY_RESULTFILE | INTERNAL_ERROR_CONVERTING_DATA | DATAFOLDER_EXISTS
*/
variable getBrickletData(string baseName, variable separateFolderForEachBricklet, variable brickletID);

/**
 * Get processed data for a few bricklets, might be handy together with checkForNewBricklets()
 * @param[in] startBrickletID brickletID of the first bricklet to load, must be greater or equal than 1
 * @param[in] endBrickletID brickletID of the last bricklet to load, must be smaller or equal to the totalNumberOfBricklets
 * @return SUCCESS | INVALID_RANGE | WAVE_EXIST | NO_FILE_OPEN | EMPTY_RESULTFILE | INTERNAL_ERROR_CONVERTING_DATA
 * @sa getBrickletData()
*/ 
variable getRangeBrickletData(string baseName, variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID);

/** IMPLEMENTED as wrapper to getRangeBrickletData 
 * Get processed data for all bricklets of the result file.  Be sure to call checkForNewBricklets() before if you want to be sure that you catch all bricklets created until now.
 * @return SUCCESS  | WAVE_EXIST | NO_FILE_OPEN | EMPTY_RESULTFILE | INTERNAL_ERROR_CONVERTING_DATA
 * @sa getRangeBrickletData()
 * @sa getBrickletData()
*/ 
variable getAllBrickletData(string baseName, variable separateFolderForEachBricklet);

/** IMPLEMENTED
 * Get the raw data without doing any data interpretation. Useful if getBrickletData() returns rubbish or if it returns INTERNAL_ERROR_CONVERTING_DATA. You should have a deep look into the vernissage manual before using this function.
 * @param[in] 	baseName name of a wave to put the raw data into. The wave may not exist before calling and it will be created in the current datafolder. The format is described in ???
 * @param[in]  	brickletID
  * @return SUCCESS | NON_EXISTENT_BRICKLET | NO_FILE_OPEN | EMPTY_RESULTFILE | WAVE_EXIST
 * */
variable getBrickletRawData(string baseName, variable brickletID);

/** IMPLEMENTED as wrapper to getRangeBrickletMetaData
 * Return all meta data of the bricklet.
 * @param[in]  brickletID
 * @param[int] baseName name of a wave to put the bricklet meta data into. The wave may not exist before calling and it will be created in the current datafolder. The format is described in ???
 * TODO if baseName is empty the name "brickletMetaDataNo" is used
 * @return SUCCESS | NON_EXISTENT_BRICKLET | NO_FILE_OPEN | EMPTY_RESULTFILE | WAVE_EXIST | WRONG_PARAMETER
*/ 
variable getBrickletMetaData(string baseName, variable brickletID);

/** IMPLEMENTED as wrapper to getRangeBrickletMetaData
 * Return the meta data for all bricklets
 * @return SUCCESS | NO_FILE_OPEN | EMPTY_RESULTFILE | WAVE_EXIST | WRONG_PARAMETER
 * @sa getAllBrickletData()
 * @sa getBrickletMetaData()
*/ 
variable getAllBrickletMetaData(string baseName, variable separateFolderForEachBricklet);

/**
 * Return the meta data for the given range of bricklets
 * @sa getRangeBrickletData()
 * @sa getBrickletMetaData()
 * @return SUCCESS | INVALID_RANGE | WAVE_EXIST | NO_FILE_OPEN | EMPTY_RESULTFILE | WRONG_PARAMETER
*/ 
variable getRangeBrickletMetaData(string baseName,variable separateFolderForEachBricklet, variable startBrickletID, variable endBrickletID);

/** IMPLEMENTED
 *  Get some useful information for bug reporting. Prints the information to the Igor Pro History and into the returned string
 *  @return string with information to be included into all bug reports
*/ 
string getBugReportTemplate();
