/*
	The file errorcodes.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#pragma once

// If this enum is changed, check GlobalData::setError(...)
enum errorCode{
	SUCCESS =0,
	UNKNOWN_ERROR=10001,
	ALREADY_FILE_OPEN=10002,
	EMPTY_RESULTFILE=10004,
	FILE_NOT_READABLE=10008,
	NO_NEW_BRICKLETS=10016,
	WRONG_PARAMETER=10032,
	INTERNAL_ERROR_CONVERTING_DATA=10064,
	NO_FILE_OPEN=10128,
	INVALID_RANGE=10256,  
	NON_EXISTENT_BRICKLET=10512,
	WAVE_EXIST=11024
};