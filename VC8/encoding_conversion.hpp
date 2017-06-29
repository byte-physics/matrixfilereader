/*
  The file encoding_conversion.hpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/
#pragma once

#include "ForwardDecl.hpp"

/// Helper class for converting between std::wstring and std::string
///
/// The encoding of std::string depends on the igor version
/// IP7: UTF-8
/// IP6: ANSI code page of the installed windows (Windows-1252 for example)
class EncodingConversion
{
public:
  /// Access to singleton-type object
  static EncodingConversion& EncodingConversion::Instance()
  {
    static EncodingConversion encConv;
    return encConv;
  }

  std::wstring convertEncoding(const std::string& str);
  std::string  convertEncoding(const std::wstring& str);

private:
  EncodingConversion(); // hide ctor
  ~EncodingConversion(); // hide dtor
  EncodingConversion(const EncodingConversion&); // hide copy ctor
  EncodingConversion& operator=(const EncodingConversion&); // hide assignment operator

  boost::function<std::wstring (const std::string&)> m_stringToWString;
  boost::function<std::string (const std::wstring&)> m_wstringToString;
};
