// API between XOP and Procedures
// Purpose: The XOP serves as a wrapper over the Vernissage DLLs because
// one can not talk to DLLs from Igor directly
//
// Version: 0.1
// Date: Feb/17/2010

// BEGIN IGNORE_ME
typedef *char HSTRING_TYPE;
typedef WAVE_TYPE TEXTWAVE;
typedef WAVE_TYPE INTWAVE;
typedef WAVE_TYPE FP64WAVE;

enum{ E_SUCCESS=0; //FIXME };

// END IGNORE_ME

// TODO:
// - check if C++ namespaces work  
// - make function names more distinct, maybe prefix with "matrixReader_"

//General Remarks
// - All functions return an int value as error code. This error code is either zero (everything is okay) or non-zero in case something went wrong.
// - For rare cases the error code E_UNKOWN_ERROR might be returned. This is usually not stated in the comment
// - The exact error message can be readout via a call to getHumanReadableStringFromErrorCode(...)
// - The textwave with (nx2) dimensions must never be considered ordered and their length is fixed to some value. Always check in the first column for a key and then readout the same row from the second column to get its value.
// 	Example textwave contents:
//	| key4 | value4|
//	| key1 | value1|
//	|      |       | <- empty string
//	| key2 | value2|
//	| key3 | value3|

// @param[in] errorCode for which an human readable string should be returned
// @return string with error message,  empty string in case the error code is not found
HSTRING_TYPE getHumanReadableStringFromErrorCode(int errorCode);

// Open a matrix result file for further treating
// @param[in] absoluteFilePath absolute path for an result file to open
// @return E_SUCCESS or E_NOT_READABLE or E_ALREADY_OPEN
int openResultFile(HSTRING_TYPE absoluteFilePath);

// Closes the result file. This will free the memory used for the result file in the Vernissage DLLs. A call to openResultFile(...) is needed to work again with last opened file or a new one
// @return E_SUCCESS or E_UNKOWN_ERROR
int closeResultFile();

// @param[out] filename of the opened result file
// @return E_SUCCESS or E_NO_FILE_OPEN
int getResultFileName(HSTRING_TYPE *filename);

// @param[out] filepath absolute path of the opened result file
// @return E_SUCCESS or E_NO_FILE_OPEN
int getResultFilePath(HSTRING_TYPE *filepath);

// @param[out] numberOfBricklets the number of bricklets of the opened result file
// @return E_SUCCESS or E_NO_FILE_OPEN
int getNumberOfBricklets(int *numberOfBricklets);

// Get the result file meta data. See general remarks about textwaves
// @param[out] resultFileMetaData textwave (nx2) dims holding all relevant information of the complete experiment/result file
// @return E_SUCCESS or E_NO_FILE_OPEN
int getResultFileMetaData(TEXTWAVE *resultFileMetaData);

