/** @file api-layout.h
 * @brief API between XOP and Igor Pro Procedures. The XOP serves as a wrapper over the Vernissage DLLs because one can not talk to DLLs from Igor directly.
 *
 * @version 0.02
 * @date Jun/19/2010
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
 * - TODO add Igor code for errorCode struct
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
 * - The wave contents are always transformed physical data and not raw data.
 * - All waves hold wave notes of the form:
 *   	@code
 *   		xopVersion=0.1
 *   		vernissageVersion=1.0
 *   		resultFilePath=c:\data
 *   		resultFileName=10MicronLong-GNR_001.mtrx
 *   		brickletID=28
 *   		rawMin=someValue1 (integer)
 *   		rawMax=someValue2 (integer)
 *   		scaledMin=someValue3 (double)
 *   		scaledMax=someValue  (double)
 *   	@endcode
 *   	In case the brickletID does not make sense, the entry is "brickletID=".
 *		The rawMin and rawMax values hold the minimum and maximum value of the unstransformed rawData. scaledMin and scaledMax holds it for the scaled data.
 * - Textwaves with (nx2) dimensions must never be considered ordered. Always check in the first column for a key and then readout the value from same row and the second column. 
 * 	Textwave layout:
 *	@code
 *	| key4 	| value4|
 *	| key1 	| value1|
 *	| key2 	| value2|
 *	| key3 	| value3|  <- last entry
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
 *		- dateOfLastChange (nicely formatted string from timeStampOfLastChange)
 *	- Bricklet meta data textwave:
 *		- TODO
 *  - The dataType strings are:
 *		- 2D data: TraceUp, ReTraceUp, TraceDown, ReTraceDown
*/

/**
 *  Open a matrix result file for further treating. Usually you want to do that as first step. The path separator can be ":" or "\".
 *  @param[in] absoluteFilePath absolute  path for the result file
 *  @param[in] fileName fileName of the result file. If the filename is empty, it is assumed that the path includes the filename.
 *  @return SUCCESS | FILE_NOT_READABLE | ALREADY_FILE_OPEN
*/
variable openResultFile(string absoluteFilePath, string fileName);

/**
 *  Close the result file. This will free the memory used for the result file and all of its bricklets in the Vernissage DLLs.
 *  A call to openResultFile() is needed if you want to work with a result file again
 *  @return SUCCESS | NO_FILE_OPEN
*/
variable closeResultFile();

/**
 *  @param[out] filename of the open result file
 *  @return SUCCESS | NO_FILE_OPEN | WRONG_PARAMETER
*/ 
variable getResultFileName(string *filename);

/**
 *  @param[out] absoluteFilePath absolute path of the folder of the open result file
 *  @return SUCCESS | NO_FILE_OPEN | WRONG_PARAMETER
*/  
variable getResultFilePath(string *absoluteFilePath);

/**
 * Get the version of this XOP. The version number has the form: "<digit>.<digit><digit>"
 * @param[out] xopVersion
 * @return SUCCESS | WRONG_PARAMETER
*/ 
variable getXOPVersion(variable *xopVersion);

/**
 * Get the vernissage DLL version. It returns the version "x.y" from part of the registry key HKEY_LOCAL_MACHINE\Software\Omicron NanoTechnology\Vernissage\Vx.y\Main\InstallDirectory.
 * @param[out] vernissageVersion 
 * @return SUCCESS| WRONG_PARAMETER
*/
variable getVernissageVersion(variable *vernissageVersion);

/**
 *  @param[out] totalNumberOfBricklets total number of bricklets of the result file
 *  @return SUCCESS | NO_FILE_OPEN | WRONG_PARAMETER
*/
variable getNumberOfBricklets(variable *totalNumberOfBricklets);

/**
 *  Get the result file meta data. See general remarks about textwaves.
 *  @param[out] waveName  name for a textwave (nx2) dims holding all relevant information of the complete experiment/result file. In case it is empty, the waveName will be resultFileMetaData.
 *  @return SUCCESS | NO_FILE_OPEN | WAVE_EXIST | WRONG_PARAMETER
*/
variable getResultFileMetaData(string waveName);

/** NOT YET
 * Check if new bricklets were added after the openResultFile() call. In case we got no new bricklets NO_NEW_BRICKLETS is returned and both startBrickletID and endBrickletID are set to -1.
 * @param[out] 	startBrickletID first new brickletID
 * @param[out] 	endBrickletID last new brickletID
 * @param[in]  	rememberCalls can be 1 (true) or 0 (false). If true, the call to  checkForNewBricklets() is internally remembered and succesive calls to it only return sucessfully, if the result file changed again. Useful for checking for updates in an endless loop.
 * @return SUCCESS | NO_NEW_BRICKLETS | NO_FILE_OPEN | WRONG_PARAMETER
 */
variable checkForNewBricklets(variable *startBrickletID,variable *endBrickletID,variable rememberCalls);

/**
 * Get the processed data for the bricklet. Processed here means it is put in the correct form as described in the Vernissage manual "Accessing raw data" p. 40. @b and transformed to physical values. The waves for each bricklet will be double (64bit) waves with the appropriate dimension.. In case a wave exists, the function returns WAVE_EXIST. In case you don't get what you expect here (or you get the return value INTERNAL_ERROR_CONVERTING_DATA), try getBrickletRawData() and do the low-level data twiddling yourself. For this you definitely have to consult the Vernissage Manual. In case of INTERNAL_ERROR_CONVERTING_DATA please submit a bugreport including a testcase which shows the error.
 * @param[in] basename first part of the wavename which will be created.
 * The wavenames are in general of the form <baseName>_<brickletID>_<dataType>, in case the basename is empty, the wavename will be brickletData_<brickletID>_<dataType>. The string dataType is needed in case the number of datawaves is greater than one (usually the case for topographic data). If the number of data waves is one, the wavename will be <baseName>_<brickletID>. 
 * @param[in]  separateFolderForEachBricklet must be zero or non-zero, create a subfolder named as the X_brickletID for each bricklet
 * @param[in]  brickletID
 * @return SUCCESS | NON_EXISTENT_BRICKLET | WAVE_EXIST | NO_FILE_OPEN | EMPTY_RESULTFILE | INTERNAL_ERROR_CONVERTING_DATA
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

/**
 * Get processed data for all bricklets of the result file.  Be sure to call checkForNewBricklets() before if you want to be sure that you catch all bricklets created until now.
 * @return SUCCESS  | NON_EXISTENT_BRICKLET | WAVE_EXIST | NO_FILE_OPEN | EMPTY_RESULTFILE | INTERNAL_ERROR_CONVERTING_DATA | WRONG_PARAMETER
 * @sa getBrickletData()
*/ 
variable getAllBrickletData(string baseName, variable separateFolderForEachBricklet);

/**
 * Get the raw data without doing any data interpretation. Useful if getBrickletData() returns rubbish or if it returns INTERNAL_ERROR_CONVERTING_DATA. You should have a deep look into the vernissage manual before using this function.
 * @param[in] 	baseName name of a wave to put the raw data into. The wave may not exist before calling and it will be created in the current  datafolder.
 * @param[in]  	brickletID
 * @return SUCCESS | NON_EXISTENT_BRICKLET | NO_FILE_OPEN | EMPTY_RESULTFILE | WAVE_EXIST | WRONG_PARAMETER
 * */
variable getBrickletRawData(string baseName, variable brickletID);

/**
 * Return all meta data of the bricklet.
 * @param[in]  brickletID
 * @param[in] baseName prefix for the meta data wave, which will be of the form <baseName>_<brickletID>. If baseName is empty the name "brickletMetaData" is used.
 * @return SUCCESS | NON_EXISTENT_BRICKLET | NO_FILE_OPEN | EMPTY_RESULTFILE | WAVE_EXIST | WRONG_PARAMETER
*/ 
variable getBrickletMetaData(string baseName, variable separateFolderForEachBricklet, variable brickletID);

/**
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

/**
 *  Get some useful information for bug reporting. Prints the information to the Igor Pro History and into the returned string
 *  @return string with information to be included into all bug reports
*/ 
string getBugReportTemplate();

/** 
 * Return a table with the given keys for getting an overview of the result file
 * @param[in] waveName name of the textwave to create, in case it is empty the name is "overViewTable". The textwave has in the first row labels which are keys.
 * Example for the keyList "brickletID;sequenceID;dimension;channelName"
 *		@code
 *		| brickletID | sequenceID | dimension | channelName |
 *		|	1	|	15	|	2	|	Z	|
 *		|	2	|	15	|	2	|	A	|
 *		@endcode
 * @param[in] keyList Semicolon separated list of bricklet metadata keys. One example would be "brickletID;sequenceID;dimension;channelName".
 * @return SUCCESS | NO_FILE_OPEN | EMPTY_RESULTFILE | WAVE_EXIST | WRONG_PARAMETER
*/ 
variable createOverViewTable(string waveName, string keyList);

/*
 * Return the error code of the last function call. If this can not be determined (usually if it is called as before all other functions) it returns UNKNOWN_ERROR.
 * @return 	SUCCESS  | UNKNOWN_ERROR | ALREADY_FILE_OPEN | EMPTY_RESULTFILE | FILE_NOT_READABLE | NO_NEW_BRICKLETS | WRONG_PARAMETER | INTERNAL_ERROR_CONVERTING_DATA | NO_FILE_OPEN | INVALID_RANGE |NON_EXISTENT_BRICKLET |WAVE_EXIST
*/
variable getLastError();

/* 
 * Return a human readable error message of the error code of the last error message.
 *  @return string with error message
 */
string getLastErrorMessage();
