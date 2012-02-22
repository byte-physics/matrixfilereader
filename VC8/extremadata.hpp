/*
	The file extremadata.hpp is part of the "MatrixFileReader XOP".
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
		// we don't want the smallest positive number but the most negative one
		m_physValRawMax(-std::numeric_limits<double>::max())
		{}

public:
	void setMinimum(int rawMin, double physValRawMin){ m_rawMin = rawMin; m_physValRawMin = physValRawMin; };
	void setMaximum(int rawMax, double physValRawMax){ m_rawMax = rawMax; m_physValRawMax = physValRawMax; };

	// const getters
	int getRawMin() const{ return m_rawMin ; };
	int getRawMax() const{ return m_rawMax; };
	double getPhysValRawMin() const{ return m_physValRawMin; };
	double getPhysValRawMax() const{ return m_physValRawMax; };

private:
	int		m_rawMin;
	int		m_rawMax;
	double	m_physValRawMin; //converted value of rawMin
	double	m_physValRawMax; //converted value of rawMax
};
