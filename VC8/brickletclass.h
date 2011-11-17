/*
	The file brickletclass.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/
#pragma once

#include <vector>
#include <string>
#include "ForwardDecl.h"

/*
	Internal representation of a bricklet
*/
class BrickletClass
{
public:
	BrickletClass(void* const pBricklet,int brickletID);
	~BrickletClass(void);

public:
	// resetting *pBricklet is only needed after the same result file is loaded again to check for new bricklets
	void setBrickletPointer(void* const pBricklet){ m_brickletPtr = pBricklet; };

	void clearCache(void);
	void getBrickletContentsBuffer(const int** pBuffer, int &count);

	const std::vector<std::string>& getBrickletMetaDataValues();
	const std::vector<std::string>& getBrickletMetaDataKeys();
	std::string getMetaDataValueAsString(const std::string &key);
	int			getMetaDataValueAsInt(const std::string &key);
	double		getMetaDataValueAsDouble(const std::string &key);

	const std::vector<std::wstring>& getAxes();
	const std::vector<std::string>& getAxesString();

// const methods
public:
	const std::vector<Vernissage::Session::ViewTypeCode>& getViewTypeCodes()const{ return m_viewTypeCodes; };
	void* getBrickletPointer()const{ return m_brickletPtr; };
	const ExtremaData& getExtrema()const{ return *m_extrema; };

private:
	void BrickletClass::loadBrickletMetaDataFromResultFile();
	void generateAllAxesVector();

private:
	void *m_brickletPtr;
	Vernissage::Session *m_VernissageSession;
	int m_brickletID;

	// storage for the raw data
	int *m_rawBufferContents;
	int m_rawBufferContentsSize;

	ExtremaData *m_extrema;

	// meta data
	std::vector<std::string> m_metaDataKeys;
	std::vector<std::string> m_metaDataValues;

	// special meta data
	std::vector<std::wstring> m_allAxesWString;
	std::vector<std::string>  m_allAxesString;
	std::vector<Vernissage::Session::ViewTypeCode> m_viewTypeCodes;
};
