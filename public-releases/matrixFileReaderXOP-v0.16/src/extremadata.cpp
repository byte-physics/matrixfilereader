/*
	The file extremadata.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#include "extremadata.h"

#include <limits>

ExtremaData::ExtremaData(void){

	// properly initialize the values
	m_rawMin		=  _I32_MAX;
	m_rawMax		=  _I32_MIN;
	m_physValRawMin =  DBL_MAX;
	m_physValRawMax = -DBL_MAX;
}

ExtremaData::~ExtremaData(void){
}
