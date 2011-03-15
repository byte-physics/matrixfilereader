/*
	The file utils.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/

#pragma once

#include "header.h"

#include <sstream>
#include <limits>

// formatStrings
const char outputFormat[]			= "%s\r";	//  outputToHistory
const char debugOutputFormat[]		= "DEBUG: %s\r"; // debugOutputToHistory

std::wstring StringToWString(const std::string& s);
std::string WStringToString(const std::wstring& s);

#define MYASSERT(A,B) { if(A == NULL){ XOPNotice("ASSERT: Pointer " #A " is NULL.\r"); return B; } }
#define ASSERT_RETURN_ZERO(A) { MYASSERT(A,0)}
#define ASSERT_RETURN_MINUSONE(A) { MYASSERT(A,-1)}
#define ASSERT_RETURN_ONE(A) { MYASSERT(A,1)}
#define ASSERT_RETURN_VOID(A) { MYASSERT(A,)}

std::wstring CharPtrToWString(char* cStr);

void debugOutputToHistory(const char *str, bool silent = false);
void outputToHistory(const char *str);

template <class T>
std::string anyTypeToString(const T& t){
	std::stringstream ss;
	ss.precision(DBL_DIG);
	ss << t;
	return ss.str();
}

template <class T>
T stringToAnyType(std::string str){
	std::stringstream ss(str);
	T t;
	ss >> t;
	return t;
}

// be sure to check the return value for NULL
template <class T> T* getWaveDataPtr(waveHndl waveH){
	int accessMode = kMDWaveAccessMode0;
	int ret=-1;
	BCInt dataOffset;
	T *dataPtr;

	ret = MDAccessNumericWaveData(waveH, accessMode, &dataOffset);
	if(ret != 0 ){
		// throw here someting if you want to have it more C++-ish
		return NULL;
	}

	dataPtr = reinterpret_cast<T*>( reinterpret_cast<char*>(*waveH) + dataOffset );
	return dataPtr;
}

int stringVectorToTextWave(std::vector<std::string> &metaData,waveHndl &waveHandle);

int createAndFillTextWave(std::vector<std::string> &firstColumn, std::vector<std::string> &secondColumn, DataFolderHandle dataFolderHandle,const char *waveName, int brickletID, std::string &fullPathOfCreatedWaves);

std::vector<std::wstring> getAllAxesNames(void *pBricklet);
std::string viewTypeCodeToString(int idx);

void setDataWaveNote(int brickletID, int traceDir, const ExtremaData &extrema,waveHndl waveHandle, int pixelSize = 1);

void setOtherWaveNote(waveHndl waveHandle,int brickletID  = -1,int traceDir  = -1);
void appendToWaveNote(std::string waveNote, waveHndl waveHandle);

std::string getStandardWaveNote(int brickletID = -1, int traceDir = -1 );

void waveClearNaN64(double *wavePtr, long length);
void waveClearNaN32(float  *wavePtr, long length);

void  splitString(char* stringChar, char *sepChar, std::vector<std::string> &list);
void  splitString(std::string &str, char *sepChar, std::vector<std::string> &list);

void joinString(std::vector<std::string> &list,const char *sepChar, std::string &joinedList);

bool doubleToBool(double value);

bool isValidBrickletRange(double startID, double endID, int numberOfBricklets);
bool isValidBrickletID(int brickletID, int numberOfBricklets);

std::string getFullWavePath(DataFolderHandle df, waveHndl wv);
int createRawDataWave(DataFolderHandle dataFolderHandle,char *waveName, int brickletID, std::string &fullPathOfCreatedWaves);

void convertHandleToString(Handle strHandle,std::string &str);