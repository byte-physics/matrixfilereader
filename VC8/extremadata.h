#pragma once

class ExtremaData{

public:
	ExtremaData(void);
	~ExtremaData(void);

public:
	void setRawMin(int rawMin){ m_rawMin = rawMin; };
	void setRawMax(int rawMax){ m_rawMax = rawMax; };
	void setPhysValRawMin(double physValRawMin){ m_physValRawMin = physValRawMin; };
	void setPhysValRawMax(double physValRawMax){ m_physValRawMax = physValRawMax; };

	int getRawMin ()const{return m_rawMin ;};
	int getRawMax ()const{ return m_rawMax; };
	double getPhysValRawMin ()const{ return m_physValRawMin; };
	double getPhysValRawMax ()const{ return m_physValRawMax; };

private:
	int		m_rawMin;
	int		m_rawMax;
	double	m_physValRawMin; //converted value of rawMin
	double	m_physValRawMax; //converted value of rawMax
};

