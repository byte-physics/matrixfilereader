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
class Bricklet
{
public:
  Bricklet(int brickletID, void *const vernissageBricklet);
  ~Bricklet();

public:
  void setBrickletPointer(void *const vernissageBricklet);

  void clearCache();
  int *getRawData();
  int getRawDataSize();

  const std::vector<StringPair> &getMetaData();
  const std::vector<StringPair> &getDeploymentParameter();

  template <typename T>
  T getMetaDataValue(const std::string &key)
  {
    return stringToAnyType<T>(getMetaDataValue<std::string>(key));
  }

  template <>
  std::string getMetaDataValue(const std::string &key)
  {
    if(key.empty())
    {
      HISTPRINT("BUG: getMetaDataValueAsString called with empty parameter");
      return std::string();
    }

    getMetaData();

    for(unsigned int i = 0; i < m_metaData.size(); i++)
    {
      if(m_metaData[i].first == key.c_str())
      {
        return std::string(m_metaData[i].second);
      }
    }
    return std::string();
  }

  template <typename T>
  const std::vector<T> &getAxes(); ///< Empty template definition

  template <>
  const std::vector<std::string> &getAxes();

  template <>
  const std::vector<std::wstring> &getAxes();

  void *getBrickletPointer() const;
  const ExtremaData &getExtrema() const;

  std::size_t getUsedMemory() const;

private:
  Bricklet(const Bricklet &);
  Bricklet &operator=(const Bricklet &);

  void loadMetaData();
  void loadDeploymentParameters();
  void generateAllAxesVector();

  void *m_brickletPtr;
  const int m_brickletID;

  // storage for the raw data
  typedef boost::scoped_array<int> ScopedIntArray;
  ScopedIntArray m_rawBufferContents;
  int m_rawBufferContentsSize;

  ExtremaData m_extrema;

  // meta data
  std::vector<StringPair> m_metaData, m_deployParams;

  // special meta data
  std::vector<std::wstring> m_allAxesWString;
  std::vector<std::string> m_allAxesString;

  boost::recursive_mutex m_mutex;
};
