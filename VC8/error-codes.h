/** @file error-codes.h
 *
 */
#pragma once

/** @var  errorCode
 * @brief The error codes returned from the functions. It might be handy to define a struct in Igor which hold these values.
 */
// If this enum is changed, change also myData::setLastError(...)
enum errorCode{
	SUCCESS =0,
	UNKNOWN_ERROR=1,
	ALREADY_FILE_OPEN=2,
	EMPTY_RESULTFILE=4,
	FILE_NOT_READABLE=8,
	NO_NEW_BRICKLETS=16,
	WRONG_PARAMETER=32,
	INTERNAL_ERROR_CONVERTING_DATA=64,
	NO_FILE_OPEN=128,
	INVALID_RANGE=256,  
	NON_EXISTENT_BRICKLET=512,
	WAVE_EXIST=1024
};