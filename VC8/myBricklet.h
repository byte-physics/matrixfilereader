#pragma once

#include <vector>
#include <string>

#include "vernissage.h"

class MyBricklet
{
public:
	MyBricklet(void* pBricklet,Vernissage::Session *pSession);
	~MyBricklet(void);

public:
	void* getBrickletPointer(){ return m_brickletPtr;};
	void  getBrickletContentsBuffer(const int** pBuffer, int &count);
	void  getBrickletMetaData(std::vector<std::string> &keys, std::vector<std::string> &values);
	std::vector<std::wstring> getAllAxesNames();
	//bool  gotCachedBrickletMetaData(int brickletID);
	//void  storeBrickletMetaData(int brickletID, std::vector<std::string> &keys, std::vector<std::string> &values);
	//void  loadCachedBrickletMetaData(int brickletID, std::vector<std::string> &keys, std::vector<std::string> &values);

private:
	void MyBricklet::loadBrickletMetaDataFromResultFile();


private:
	void *m_brickletPtr;
	Vernissage::Session *m_VernissageSession;


	// storage for the raw data
	const int *m_rawBufferContents;
	int m_rawBufferContentsSize;

	// meta data
	std::vector<std::string> m_metaDataKeys;
	std::vector<std::string> m_metaDataValues;

	// special meta data
	std::vector<std::wstring> m_allAxes;
};
