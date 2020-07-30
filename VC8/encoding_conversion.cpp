/*
  The file encoding_conversion.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/
#include "stdafx.h"

#include "encoding_conversion.hpp"

namespace
{

// Convert a UTF-16 encoded wstring to a UTF8 encoded string
//
// inspired by: https://stackoverflow.com/a/3082975
std::string convertUTF16ToUTF8(const std::wstring &utf16String)
{
  if(utf16String.empty())
    return std::string();

  const int utf16len = boost::numeric_cast<int>(utf16String.size());

  // get length of utf8 string
  const int utf8len = WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), utf16len, NULL, 0, NULL, NULL);

  std::vector<char> utf8Vector(utf8len);
  const int numBytesWritten =
      WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), utf16len, &utf8Vector[0], utf8len, NULL, NULL);

  return std::string(&utf8Vector[0], numBytesWritten);
}

// And back
std::wstring convertUTF8ToUTF16(const std::string &utf8String)
{
  if(utf8String.empty())
    return std::wstring();

  const int utf8len = boost::numeric_cast<int>(utf8String.size());

  // get length of utf16 string
  const int utf16len = MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), utf8len, NULL, 0);

  std::vector<wchar_t> utf16Vector(utf16len);
  const int numBytesWritten = MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), utf8len, &utf16Vector[0], utf16len);

  return std::wstring(&utf16Vector[0], numBytesWritten);
}

std::wstring ansiToUnicode(const std::string &s)
{
  return GlobalData::Instance().getVernissageSession()->ansiToUnicode(s);
}

std::string unicodeToAnsi(const std::wstring &s)
{
  return GlobalData::Instance().getVernissageSession()->unicodeToAnsi(s);
}

} // anonymous namespace

EncodingConversion::EncodingConversion()
{
  if(igorVersion < 700)
  {
    m_stringToWString = ansiToUnicode;
    m_wstringToString = unicodeToAnsi;
  }
  else
  {
    m_stringToWString = convertUTF8ToUTF16;
    m_wstringToString = convertUTF16ToUTF8;
  }
}

EncodingConversion::~EncodingConversion()
{
}

std::wstring EncodingConversion::convertEncoding(const std::string &str)
{
  return m_stringToWString(str);
}

std::string EncodingConversion::convertEncoding(const std::wstring &str)
{
  return m_wstringToString(str);
}
