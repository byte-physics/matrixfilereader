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

#define THROW_IF_NULL(A) {if(A == NULL){ throw std::runtime_error("THROW_IF_NULL: Pointer " #A " is NULL.\r"); } }
#define MYASSERT(A,B) { if(A == NULL){ XOPNotice("ASSERT: Pointer " #A " is NULL.\r"); return B; } }
#define ASSERT_RETURN_ZERO(A) { MYASSERT(A,0)}
#define ASSERT_RETURN_MINUSONE(A) { MYASSERT(A,-1)}
#define ASSERT_RETURN_ONE(A) { MYASSERT(A,1)}
#define ASSERT_RETURN_VOID(A) { MYASSERT(A,)}

//#define START_TIMER(A) \
//  HISTPRINT("Starting timer %d: line %d, function %s,  file %s", A, __LINE__, __FUNCTION__, __FILE__);\
//  CStopWatch sw##A; sw##A.startTimer();
//
//
//#define STOP_TIMER(A)  { sw##A.stopTimer();\
//  HISTPRINT("Elapsed time of timer %d [s]: %g, line %d, function %s,  file %s", A, sw##A.getElapsedTime(), __LINE__, __FUNCTION__, __FILE__);\
//  }

//#define DEBUGCODE \
//  HISTPRINT("line %d, function %s,  file %s", __LINE__, __FUNCTION__, __FILE__);\

std::wstring ansiToUnicode(const std::string& s);
std::string unicodeToAnsi(const std::wstring& s);

template <class T>
T stringToAnyType(std::string str)
{
  std::stringstream ss(str);
  T t;
  ss >> t;
  return t;
}

/*
  Converts any type to a std::string
*/
template <typename T>
std::string toString(const T& t)
{
  std::stringstream ss;
  ss.precision(DBL_DIG);
  ss << t;
  return ss.str();
}

/*
  Specialization
*/
template<>
inline std::string toString<std::wstring>(const std::wstring& s)
{
  return unicodeToAnsi(s);
}

/*
  Specialization (which does nothing)
*/
template<>
inline std::string toString<std::string>( const std::string& s )
{
  return s;
}

void  splitString(const char* stringChar, const char* sepChar, std::vector<std::string> &list);
void  splitString(const std::string& str, const char* sepChar, std::vector<std::string> &list);

/*
  Undos a former split string, therefore concatenating each element plus sepchar of list into one string
*/
template <class T>
void joinString(const std::vector<T> &list, const char* sepChar, std::string& joinedList)
{
  joinedList.clear();

  for (std::vector<T>::const_iterator it = list.begin(); it != list.end(); it++)
  {
    joinedList.append(toString<T>(*it));
    joinedList.append(sepChar);
  }
}

bool doubleToBool(double value);
void RemoveAllBackslashesAtTheEnd(char* str);
