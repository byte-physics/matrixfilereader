#pragma once

#include <vector>
#include <string>

#include "vernissage.h"

class MyBricklet
{
public:
	MyBricklet(void* pBricklet,int brickletID);
	~MyBricklet(void);

public:
	void* getBrickletPointer(){ return m_brickletPtr;};
	void  getBrickletContentsBuffer(const int** pBuffer, int &count);
	void  getBrickletMetaData(std::vector<std::string> &keys, std::vector<std::string> &values);
	std::vector<std::wstring> generateAllAxesVector();
	void getAxes(std::vector<std::string> &allAxes){ allAxes = m_allAxes; };
	void getViewTypeCodes(std::vector<Vernissage::Session::ViewTypeCode> &viewTypeCodes){ viewTypeCodes = m_viewTypeCodes; };
	std::string getMetaDataValueAsString(std::string key);
	int			getMetaDataValueAsInt(std::string key);
	double		getMetaDataValueAsDouble(std::string key);

private:
	void MyBricklet::loadBrickletMetaDataFromResultFile();

private:
	void *m_brickletPtr;
	Vernissage::Session *m_VernissageSession;
	int m_brickletID;

	// storage for the raw data
	const int *m_rawBufferContents;
	int m_rawBufferContentsSize;

	// meta data
	std::vector<std::string> m_metaDataKeys;
	std::vector<std::string> m_metaDataValues;

	// special meta data
	std::vector<std::string> m_allAxes;
	std::vector<Vernissage::Session::ViewTypeCode> m_viewTypeCodes;
};
