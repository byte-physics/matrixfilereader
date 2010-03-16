/** @file error-codes.h
 *
 */

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

/** @var  errorCode
 * @brief The error codes returned from the functions. It might be handy to define a struct in Igor which hold these values.
 */ 
enum errorCode{
	SUCCESS =0,
	UNKOWN_ERROR=1,
	ALREADY_FILE_OPEN=2,
	EMPTY_RESULTFILE=4,
	FILE_NOT_READABLE=8,
	NO_NEW_BRICKLETS=16,
	WAVE_EXIST=32,
	INTERNAL_ERROR_CONVERTING_DATA=64,
	NON_EXISTENT_DATAFOLDER=128,
	INVALID_RANGE=256,  
	NON_EXISTENT_BRICKLET=512,
	NO_FILE_OPEN=1024,
};


#endif // ERROR_CODES_H