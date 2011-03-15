/*
	The file brickletclass.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#pragma once

#include <vector>
#include <string>

#include "header.h"

class BrickletClass
{
public:
	BrickletClass(void* pBricklet,int brickletID);
	~BrickletClass(void);

public:
	void* getBrickletPointer(){ return m_brickletPtr;};
	// resetting *pBricklet is only needed after the same result file is loaded again to check for new bricklets
	void setBrickletPointer(void* pBricklet){ m_brickletPtr = pBricklet;};

	void  clearCache(void);
	void  getBrickletContentsBuffer(const int** pBuffer, int &count);
	void  getBrickletMetaData(std::vector<std::string> &keys, std::vector<std::string> &values);
	std::vector<std::wstring> generateAllAxesVector();
	void getAxes(std::vector<std::string> &allAxes){ allAxes = m_allAxes; };
	void getViewTypeCodes(std::vector<Vernissage::Session::ViewTypeCode> &viewTypeCodes){ viewTypeCodes = m_viewTypeCodes; };
	std::string getMetaDataValueAsString(std::string key);
	int			getMetaDataValueAsInt(std::string key);
	double		getMetaDataValueAsDouble(std::string key);
	const ExtremaData& getExtrema()const{ return m_extrema; };

private:
	void BrickletClass::loadBrickletMetaDataFromResultFile();

private:
	void *m_brickletPtr;
	Vernissage::Session *m_VernissageSession;
	int m_brickletID;

	// storage for the raw data
	int *m_rawBufferContents;
	int m_rawBufferContentsSize;

	ExtremaData m_extrema;

	// meta data
	std::vector<std::string> m_metaDataKeys;
	std::vector<std::string> m_metaDataValues;

	// special meta data
	std::vector<std::string> m_allAxes;
	std::vector<Vernissage::Session::ViewTypeCode> m_viewTypeCodes;
};
