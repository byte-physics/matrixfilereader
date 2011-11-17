/*
	The file utils_generic.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/

/*
	Place for generic functions which are neither XOP nor vernissage specific
*/

#pragma once

#include <sstream>
#include <string>
#include <vector>

#define MYASSERT(A,B) { if(A == NULL){ XOPNotice("ASSERT: Pointer " #A " is NULL.\r"); return B; } }
#define ASSERT_RETURN_ZERO(A) { MYASSERT(A,0)}
#define ASSERT_RETURN_MINUSONE(A) { MYASSERT(A,-1)}
#define ASSERT_RETURN_ONE(A) { MYASSERT(A,1)}
#define ASSERT_RETURN_VOID(A) { MYASSERT(A,)}

template <class T>
T stringToAnyType(std::string str){
	std::stringstream ss(str);
	T t;
	ss >> t;
	return t;
}

std::wstring StringToWString(const std::string& s);
std::string WStringToString(const std::wstring& s);
std::wstring CharPtrToWString(char* cStr);

template <class T>
std::string anyTypeToString(const T& t){
	std::stringstream ss;
	ss.precision(DBL_DIG);
	ss << t;
	return ss.str();
}

void  splitString(const char* stringChar, const char* sepChar, std::vector<std::string> &list);
void  splitString(const std::string &str, const char* sepChar, std::vector<std::string> &list);

void joinString(const std::vector<std::string> &list,const char* sepChar, std::string &joinedList);

bool doubleToBool(double value);

