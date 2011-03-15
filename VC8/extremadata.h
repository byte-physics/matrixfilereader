#pragma once

class ExtremaData{

public:
	ExtremaData(void);
	~ExtremaData(void);

public:
	int getRawMin ()const{return m_rawMin ;};
	void setRawMin(int rawMin){ m_rawMin = rawMin; };
	int getRawMax ()const{ return m_rawMax; };
	void setRawMax(int rawMax){ m_rawMax = rawMax; };
	double getPhysValRawMin ()const{ return m_physValRawMin; };
	void setPhysValRawMin(double physValRawMin){ m_physValRawMin = physValRawMin; };
	double getPhysValRawMax ()const{ return m_physValRawMax; };
	void setPhysValRawMax(double physValRawMax){ m_physValRawMax = physValRawMax; };

private:
	int		m_rawMin;
	int		m_rawMax;
	double	m_physValRawMin; //converted value of rawMin
	double	m_physValRawMax; //converted value of rawMax
};

