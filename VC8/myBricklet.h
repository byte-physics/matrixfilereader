#pragma once

#include <vector>
#include <string>

#include "header.h"

class MyBricklet
{
public:
	MyBricklet(void* pBricklet,int brickletID);
	~MyBricklet(void);

public:
	void* getBrickletPointer(){ return m_brickletPtr;};
	// resetting *pBricklet is only needed after the same result file is loaded again to check for new bricklets
	void setBrickletPointer(void* pBricklet){ m_brickletPtr = pBricklet;};

	void  getBrickletContentsBuffer(const int** pBuffer, int &count);
	void  getBrickletMetaData(std::vector<std::string> &keys, std::vector<std::string> &values);
	std::vector<std::wstring> generateAllAxesVector();
	void getAxes(std::vector<std::string> &allAxes){ allAxes = m_allAxes; };
	void getViewTypeCodes(std::vector<Vernissage::Session::ViewTypeCode> &viewTypeCodes){ viewTypeCodes = m_viewTypeCodes; };
	std::string getMetaDataValueAsString(std::string key);
	int			getMetaDataValueAsInt(std::string key);
	double		getMetaDataValueAsDouble(std::string key);
	int getRawMax(){ return m_maxRawValue; };
	int getRawMin(){ return m_minRawValue; };
	double getPhysValRawMax(){ return m_maxScaledValue; };
	double getPhysValRawMin(){ return m_minScaledValue; };

private:
	void MyBricklet::loadBrickletMetaDataFromResultFile();

private:
	void *m_brickletPtr;
	Vernissage::Session *m_VernissageSession;
	int m_brickletID;

	// storage for the raw data
	int *m_rawBufferContents;
	int m_rawBufferContentsSize;

	// min and max raw values
	int m_minRawValue, m_maxRawValue;

	// min and max scaled values
	double m_minScaledValue, m_maxScaledValue;

	// meta data
	std::vector<std::string> m_metaDataKeys;
	std::vector<std::string> m_metaDataValues;

	// special meta data
	std::vector<std::string> m_allAxes;
	std::vector<Vernissage::Session::ViewTypeCode> m_viewTypeCodes;
};
