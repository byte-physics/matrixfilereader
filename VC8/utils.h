#pragma once

#include "header.h"

#include <sstream>
#include <limits>

// formatStrings
const char outputFormat[]			= "%s\r";	//  outputToHistory
const char debugOutputFormat[]		= "DEBUG: %s\r"; // debugOutputToHistory

std::wstring StringToWString(const std::string& s);
std::string WStringToString(const std::wstring& s);

#define MYASSERT(A,B) { if(A == NULL){ XOPNotice("ASSERT: Pointer " #A " is NULL. You might want to drop the author a note :)\r"); return B; } }
#define ASSERT_RETURN_ZERO(A) { MYASSERT(A,0)}
#define ASSERT_RETURN_MINUSONE(A) { MYASSERT(A,-1)}
#define ASSERT_RETURN_VOID(A) { MYASSERT(A,)}

std::wstring CharPtrToWString(char* cStr);

void debugOutputToHistory(const char *str);
void outputToHistory(const char *str);

// taken from http://rhyous.com/2009/10/01/how-to-convert-an-int-to-a-string-in-c/

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

int stringVectorToTextWave(std::vector<std::string> &metaData,waveHndl &waveHandle);

int createAndFillTextWave(std::vector<std::string> &firstColumn, std::vector<std::string> &secondColumn, DataFolderHandle dataFolderHandle,const char *waveName, int brickletID, std::string &fullPathOfCreatedWaves);

std::vector<std::wstring> getAllAxesNames(void *pBricklet);
std::string viewTypeCodeToString(int idx);

void setDataWaveNote(int brickletID, int rawMin, int rawMax, double scaledMin, double scaledMax, waveHndl waveHandle);
void setOtherWaveNote(int brickletID,waveHndl waveHandle);
void mySetWaveNote(std::string waveNote, waveHndl waveHandle);

std::string getStandardWaveNote(int brickletID);

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