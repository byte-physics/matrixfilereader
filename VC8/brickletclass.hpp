/*
  The file brickletclass.hpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/
#pragma once

#include <vector>
#include <string>

#include "ForwardDecl.hpp"
#include "extremadata.hpp"
#include "utils_xop.hpp"
#include "globaldata.hpp"

/*
  Internal representation of a bricklet
*/
class BrickletClass
{
public:
  BrickletClass(void* const pBricklet, int brickletID);
  ~BrickletClass();

public:
  void setBrickletPointer(void* const pBricklet);

  void clearCache();
  void getBrickletContentsBuffer(const int** pBuffer, int& count);

  typedef std::pair<std::string,std::string> StringPair;
  const std::vector<StringPair>& getBrickletMetaData();

  template<typename T>
  T getMetaDataValue(const std::string& key)
  {
    return stringToAnyType<T>(this->getMetaDataValue<std::string>(key));
  }

  template<>
  std::string BrickletClass::getMetaDataValue( const std::string& key )
  {
    if (key.empty())
    {
      HISTPRINT("BUG: getMetaDataValueAsString called with empty parameter");
      return std::string();
    }

    if (m_metaData.empty())
    {
      loadBrickletMetaDataFromResultFile();
    }

    for (unsigned int i = 0; i < m_metaData.size(); i++)
    {
      if (m_metaData[i].first == key)
      {
        return m_metaData[i].second;
      }
    }
    return std::string();
  }

  template<typename T>
  const std::vector<T>& getAxes(); ///< Empty template definition

  template<>
  const std::vector<std::string>& getAxes();

  template<>
  const std::vector<std::wstring>& getAxes();

  const std::vector<Vernissage::Session::ViewTypeCode>& getViewTypeCodes()const;
  void* getBrickletPointer()const;
  const ExtremaData& getExtrema()const;

private:
  void loadBrickletMetaDataFromResultFile();
  void generateAllAxesVector();

  void* m_brickletPtr;
  Vernissage::Session* m_vernissageSession;
  int m_brickletID;

  // storage for the raw data
  int* m_rawBufferContents;
  int m_rawBufferContentsSize;

  ExtremaData m_extrema;

  // meta data
  std::vector<StringPair> m_metaData;

  // special meta data
  std::vector<std::wstring> m_allAxesWString;
  std::vector<std::string>  m_allAxesString;
  std::vector<Vernissage::Session::ViewTypeCode> m_viewTypeCodes;
};
