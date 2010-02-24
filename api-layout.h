/** @file api-layout.h
 * @brief API between XOP and Igor Pro Procedures. The XOP serves as a wrapper over the Vernissage DLLs because one can not talk to DLLs from Igor directly.
 *
 * @version 0.11
 * @date Feb/24/2010
*/

typedef char* HSTRING_TYPE;
typedef WAVE_TYPE TEXTWAVE;
typedef WAVE_TYPE FP64WAVE;

enum{ E_SUCCESS=0;  }; //FIXME

/* TODO - check if C++ namespaces work
 *      - make function names more distinct, maybe prefix with "matrixReader_"
 * 	- check if the error E_EMPTY_RESULTFILE really is possible 
*/

/** @mainpage General Remarks
 *
 * 
 * - All functions return an integer as error code. This error code is either zero (everything is okay) or non-zero in case something went wrong.
 *  This means the functions can be called in Igor like
 *  @code
 *  if( !openResultFile() )
 *  	// call succeeded
 *  else
 *	// do error handling
 *  endif
 *  @endcode
 * - For rare cases the error code E_UNKOWN_ERROR might be returned. Obviously in this case something went really badly wrong.
 * - An error message for displaying the user can be readout via a call to  getHumanReadableStringFromErrorCode()
 * - Textwaves with (nx2) dimensions must never be considered ordered in any sense. Always check in the first column for a key and then readout the value from same row and the second column.
 * 	Textwave layout:
 *	@code
 *	| key4 | value4|
 *	| key1 | value1|
 *	| key2 | value2|
 *	| key3 | value3|
 *	|      |       | <- last entry because the whole row is empty
 *	|      |       | <- empty entries (their number should be considered unknown)
 *	|      |       |
 *	@endcode
 * - waves are always passed as references (which means the called XOP function can and will change the waves contents).
 * - brickletIDs are 1-based and range from 1 to totalNumberOfBricklets. It is guaranteed (at least in theory) that these IDs do not change even after closing and opening the file again.
*/

/** Return a human readable error string
 *  @param[in] errorCode return value from the functions here
 *  @return string with error message, an empty string in case the error code is not found
*/ 
HSTRING_TYPE getHumanReadableStringFromErrorCode(int errorCode);

/** Open a matrix result file for further treating. Usually you want to do that as first step.
 *  @param[in] absoluteFilePath absolute path for an result file to open. This is @b no Igor symbolic path.
 *  @return E_SUCCESS | E_NOT_READABLE | E_FILE_OPEN
*/
int openResultFile(HSTRING_TYPE absoluteFilePath);

/** Close the result file. This will free the memory used for the result file and all of its bricklets in the Vernissage DLLs.
 *  A call to  openResultFile() is needed to work again with the last opened file or a new one
 *  @return E_SUCCESS | E_NO_FILE_OPEN
*/
int closeResultFile();

/**
 *  @param[out] filename of the opened result file
 *  @return E_SUCCESS | E_NO_FILE_OPEN
*/ 
int getResultFileName(HSTRING_TYPE *filename);

/**
 *  @param[out] absoluteFilePath absolute path of the opened result file
 *  @return E_SUCCESS | E_NO_FILE_OPEN
*/  
int getResultFilePath(HSTRING_TYPE *absoluteFilePath);

/**
 *  @param[out] totalNumberOfBricklets total number of bricklets of the result file
 *  @return E_SUCCESS | E_NO_FILE_OPEN | E_EMPTY_RESULTFILE
*/
int getNumberOfBricklets(double *totalNumberOfBricklets);

/**
 *  Get the result file meta data. See general remarks about textwaves
 *  @param[out] resultFileMetaData textwave (nx2) dims holding all relevant information of the complete experiment/result file
 *  @return E_SUCCESS | E_NO_FILE_OPEN
*/
int getResultFileMetaData(TEXTWAVE resultFileMetaData);

/**
 * Check if new bricklets were added after the openResultFile() call. In case we got no new bricklets E_NO_NEW_BRICKLETSis returned and both startBrickletID and endBrickletID are set to -1.
 * @param[out] 	startBrickletID first new brickletID
 * @param[out] 	endBrickletID last new brickletID.
 * @param[in]  	rememberCalls can be 1 (true) or 0 (false). If true, the call to  checkForNewBricklets() is internally remembered and
 * 		succesive calls to it only return sucessfully, if the result file changed again. Useful for checking for updates in an endless loop.
 * @return E_SUCCESS | E_NO_NEW_BRICKLETS | E_NO_FILE_OPEN
 */
int checkForNewBricklets(int *startBrickletID,int *endBrickletID,int rememberCalls);

/** Get the bricklet viewTypeCodes of one bricklet as wave. Usually this wave will consist only of one entry.
 * The numbers have the following correspondence with the names in the Vernissage DLLs (taken from Vernissage.h)
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
 * @param[in]  brickletID see General Remarks
 * @param[out] allViewTypeCodes wave with all viewTypeCodes of the given bricklet
 * @return E_SUCCESS | E_NON_EXISTENT_BRICKLET | E_NO_FILE_OPEN | E_EMPTY_RESULTFILE
*/
int getBrickletViewTypeCode(int brickletID, FP64WAVE allViewTypeCodes);

/** Return the bricklet dimension
 * @param[in]  brickletID
 * @param[out] brickletDimension
 * @sa getBrickletMetaData()
 * @return E_SUCCESS | E_NON_EXISTENT_BRICKLET | E_NO_FILE_OPEN | E_EMPTY_RESULTFILE
*/ 
int getBrickletDimension(int brickletID, double *brickletDimension);

/** Return the dimension of all bricklets
 * @param[out] brickletDimension wave holding the dimension data
 * @return E_SUCCESS | E_NO_FILE_OPEN | E_EMPTY_RESULTFILE
*/ 
int getAllBrickletDimension(FP64WAVE brickletDimension);


/** Get the processed data (means it is put in the correct form, @b no other processing is done) for the bricklet. In case you don't get what you expect here (or you get the return value E_INTERNAL_ERROR_CONVERTING_DATA), try getBrickletRawData() and do the low-level data twiddling yourself. For this you should definitely consult the Vernissage Manual.
 * @param[in]  	brickletID
 * @param[out] 	brickletData data of the bricklet, already processed and ready to display. Its dimension is usually something between one and three, but is actually not limited.
 * @return E_SUCCESS | E_NON_EXISTENT_BRICKLET | E_NO_FILE_OPEN | E_EMPTY_RESULTFILE | E_INTERNAL_ERROR_CONVERTING_DATA
*/
int getBrickletData(int brickletID, FP64WAVE brickletData);

/** Get processed data for a few bricklets, might be handy together with checkForNewBricklets()
 * @param[in] startBrickletID brickletID of the first bricklet to load, must be greater or equal than 1
 * @param[in] endBrickletID brickletID of the last bricklet to load, must be smaller or equal to the totalNumberOfBricklets
 * @return E_SUCCESS | E_DATAFOLDER_DOES_NOT_EXIST | E_INVALID_RANGE | E_WAVE_EXIST | E_NO_FILE_OPEN | E_EMPTY_RESULTFILE
 * @sa getAllBrickletData()
*/ 
int getRangeBrickletData(HSTRING_TYPE dataFolderPath, HSTRING_TYPE baseName,int separateFolderForEachBricklet, int startBrickletID, int endBrickletID);

/** Get processed data for all bricklets of the result file.  Be sure to call checkForNewBricklets() before if you want to be sure that you catch all. The waves for each bricklet will be double precision with the appropriate dimension. This function never overwrites wave, in case a wave exists, the function returns E_WAVE_EXISTS
 *  Note: This might take a while
 * @param[in] dataFolderPath path to store the bricklets, must exist before calling the function
 * @param[in] basename first part of the wavename which will be created, the wavenames are baseName0, baseName1, ... baseNameN, where the number is the brickletID
 * @param[in] separateFolderForEachBricklet must be 0 or 1, create a subfolder in dataFolderPath named baseName for each bricklet
 * @return E_SUCCESS | E_DATAFOLDER_DOES_NOT_EXIST | E_WAVE_EXISTS | E_NO_FILE_OPEN | E_EMPTY_RESULTFILE
 * @sa getRangeBrickletData()
 * @sa getBrickletData()
*/ 
int getAllBrickletData(HSTRING_TYPE dataFolderPath,HSTRING_TYPE baseName,int separateFolderForEachBricklet);

/** Get the raw data with using axis information. Useful if getBrickletData() returns rubbish. You should have a deep look into the vernissage manual before using this function.
 * @param[in]  	brickletID
 * @param[out] 	brickletData data of the bricklet, this wave will be always be one dimensional
 * @return E_SUCCESS | E_NON_EXISTENT_BRICKLET | E_NO_FILE_OPEN | E_EMPTY_RESULTFILE
*/
int getBrickletRawData(int brickletID, FP64WAVE data);

/** Return all meta data of the bricklet.
 * @param[in]  brickletID
 * @param[out] metaData textwave having @b all meta data of the specified bricklet
 * @return E_SUCCESS | E_NON_EXISTENT_BRICKLET | E_NO_FILE_OPEN | E_EMPTY_RESULTFILE
*/ 
int getBrickletMetaData(int brickletID, TEXTWAVE metaData);

/** Return the meta data of all bricklets
 * @return E_SUCCESS | E_DATAFOLDER_DOES_NOT_EXIST | E_WAVE_EXISTS | E_NO_FILE_OPEN | E_EMPTY_RESULTFILE
 * @sa getAllBrickletData()
*/ 
int getAllBrickletMetaData(HSTRING_TYPE dataFolderPath,HSTRING_TYPE baseName,int separateFolderForEachBricklet);

/** Return the meta data for the given range of bricklets
 * @sa getRangeBrickletData()
 * @return E_SUCCESS | E_DATAFOLDER_DOES_NOT_EXIST | E_INVALID_RANGE | E_WAVE_EXIST | E_NO_FILE_OPEN | E_EMPTY_RESULTFILE
*/ 
int getRangeBrickletMetaData(HSTRING_TYPE dataFolderPath,HSTRING_TYPE baseName,int separateFolderForEachBricklet, int startBrickletID, int endBrickletID);

