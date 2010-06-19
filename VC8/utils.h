

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <vector>

#include "xopstandardheaders.h"

std::wstring StringToWString(const std::string& s);
std::string WStringToString(const std::wstring& s);

#define ASSERT(A,B) { if(A == NULL){ XOPNotice("ASSERT: Pointer " #A " is NULL. You might want to drop the author a note :)\r"); return B; } }
#define ASSERT_RETURN_ZERO(A) { ASSERT(A,0)}
#define ASSERT_RETURN_MINUSONE(A) { ASSERT(A,-1)}
#define ASSERT_RETURN_VOID(A) { ASSERT(A,)}

std::wstring CharPtrToWString(char* cStr);

void debugOutputToHistory(int level,const char *str);
void outputToHistory(const char *str);

// taken from http://rhyous.com/2009/10/01/how-to-convert-an-int-to-a-string-in-c/

template <class T>
std::string anyTypeToString(const T& t){
	std::stringstream ss;
	// TODO is it enough or too much, make it a define
	ss.precision(30);
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

int createAndFillTextWave(std::vector<std::string> &firstColumn, std::vector<std::string> &secondColumn, DataFolderHandle dataFolderHandle,const char *waveName, int brickletID);
int createAndFillDataWave(DataFolderHandle dataFolderHandle, const char *waveName, int brickletID);

std::vector<std::wstring> getAllAxesNames(void *pBricklet);
std::string viewTypeCodeToString(int idx);
std::string valueTypeToString(int idx);
bool fileExists(std::string filePath, std::string fileName);
#endif // UTILS_H