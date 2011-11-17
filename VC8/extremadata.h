/*
	The file extremadata.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#pragma once
#include <limits>

/*
	Holds the minima and maxima values for a given dataset. This dataset can either be a BrickletClass
	(which may hold data of multiple trace directions) or a WaveClass (data from exactly one trace direction)
*/

class ExtremaData{

public:
	ExtremaData()
		:
		m_rawMin(std::numeric_limits<int>::max()),
		m_rawMax(std::numeric_limits<int>::min()),
		m_physValRawMin(std::numeric_limits<double>::max()),
		m_physValRawMax(std::numeric_limits<double>::min())
		{}

public:
	void setRawMin(int rawMin){ m_rawMin = rawMin; };
	void setRawMax(int rawMax){ m_rawMax = rawMax; };
	void setPhysValRawMin(double physValRawMin){ m_physValRawMin = physValRawMin; };
	void setPhysValRawMax(double physValRawMax){ m_physValRawMax = physValRawMax; };

	// const getters
	int getRawMin ()const{ return m_rawMin ; };
	int getRawMax ()const{ return m_rawMax; };
	double getPhysValRawMin ()const{ return m_physValRawMin; };
	double getPhysValRawMax ()const{ return m_physValRawMax; };

private:
	int		m_rawMin;
	int		m_rawMax;
	double	m_physValRawMin; //converted value of rawMin
	double	m_physValRawMax; //converted value of rawMax
};
