/*
	The file errorcodes.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#pragma once

/*
	error codes which are returned in V_flag by the operations
*/

// If this enum is changed, check GlobalData::setError(...) to see if it needs adaption
// the values start at 10000 because these are reserved for custom XOP error codes according to the XOP Toolkit manual
enum errorCode{
	SUCCESS							=0,
	UNKNOWN_ERROR					=10001,
	ALREADY_FILE_OPEN				=10002,
	EMPTY_RESULTFILE				=10004,
	FILE_NOT_READABLE				=10008,
	NO_NEW_BRICKLETS				=10016,
	WRONG_PARAMETER					=10032,
	INTERNAL_ERROR_CONVERTING_DATA	=10064,
	NO_FILE_OPEN					=10128,
	INVALID_RANGE					=10256,
	WAVE_EXIST						=10512
};