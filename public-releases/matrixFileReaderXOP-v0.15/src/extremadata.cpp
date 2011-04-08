#include "extremadata.h"

#include <limits>

ExtremaData::ExtremaData(void){

	// properly initialize the values
	m_rawMin		 =  _I32_MAX;
	m_rawMax		 =  _I32_MIN;
	m_physValRawMin =  DBL_MAX;
	m_physValRawMax = -DBL_MAX;
}

ExtremaData::~ExtremaData(void){
}
