/*
  The file operationsinterface_getresultfilemetadata.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.hpp"
#include "operationsinterface.hpp"
#include "utils_bricklet.hpp"
#include "globaldata.hpp"
#include "brickletclass.hpp"
#include "utils_generic.hpp"

extern "C" int ExecuteGetResultFileMetaData(GetResultFileMetaDataRuntimeParamsPtr p)
{
  BEGIN_OUTER_CATCH
  GlobalData::Instance().initializeWithoutReadSettings(p->calledFromMacro, p->calledFromFunction);

  SetOperationStrVar(S_waveNames, "");
  int ret;

  // check of DEST flag which tells us that we should place all output in the supplied datafolder
  // and also read the variable settings from this folder
  DataFolderHandle destDataFolderHndl = NULL;
  if (p->DESTFlagEncountered)
  {
    if (p->dfref == NULL)
    {
      GlobalData::Instance().setError(WRONG_PARAMETER, "dfref");
      return 0;
    }

    destDataFolderHndl = p->dfref;
    // Here we check again for the config variables, this time in the destDataFolderHndl
  }
  else // no DEST flag given, so we take the current data folder as destination folder
  {
    ret = GetCurrentDataFolder(&destDataFolderHndl);

    if (ret != 0)
    {
      GlobalData::Instance().setInternalError(ret);
      return 0;
    }
  }

  GlobalData::Instance().readSettings(destDataFolderHndl);

  if (!GlobalData::Instance().resultFileOpen())
  {
    GlobalData::Instance().setError(NO_FILE_OPEN);
    return 0;
  }

  Vernissage::Session* pSession = GlobalData::Instance().getVernissageSession();
  ASSERT_RETURN_ZERO(pSession);

  const int numberOfBricklets = pSession->getBrickletCount();

  // check waveName parameter
  std::string waveName = resultMetaDefault;
  if (p->NFlagEncountered)
  {
    if (GetHandleSize(p->waveName) == 0L)
    {
      GlobalData::Instance().setError(WRONG_PARAMETER, "waveName");
      return 0;
    }
    else
    {
      convertHandleToString(p->waveName, waveName);
    }
  }

  std::vector<std::pair<std::string,std::string> > data;
  data.push_back(std::make_pair(RESULT_DIR_PATH_KEY,GlobalData::Instance().getDirPath<std::string>()));
  data.push_back(std::make_pair(RESULT_FILE_NAME_KEY,GlobalData::Instance().getFileName<std::string>()));
  data.push_back(std::make_pair("totalNumberOfBricklets",toString(numberOfBricklets)));

  if (numberOfBricklets > 0)
  {
    const BrickletClass* bricklet = GlobalData::Instance().getBrickletClassObject(numberOfBricklets);
    ASSERT_RETURN_ZERO(bricklet);
    void* pBricklet = bricklet->getBrickletPointer();
    ASSERT_RETURN_ZERO(pBricklet);

    // use the timestamp of the last bricklet as dateOfLastChange
    tm ctime = pSession->getCreationTimestamp(pBricklet);
    const Vernissage::Session::BrickletMetaData brickletMetaData = pSession->getMetaData(pBricklet);

    char buf[ARRAY_SIZE];
    sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d", ctime.tm_mon + 1, ctime.tm_mday, ctime.tm_year + 1900, ctime.tm_hour, ctime.tm_min, ctime.tm_sec);

    data.push_back(std::make_pair("dateOfLastChange",buf));
    data.push_back(std::make_pair("timeStampOfLastChange",toString<time_t>(mktime(&ctime))));
    data.push_back(std::make_pair("BrickletMetaData.fileCreatorName",toString(brickletMetaData.fileCreatorName)));
    data.push_back(std::make_pair("BrickletMetaData.fileCreatorVersion",toString(brickletMetaData.fileCreatorVersion)));
    data.push_back(std::make_pair("BrickletMetaData.userName",toString(brickletMetaData.userName)));
    data.push_back(std::make_pair("BrickletMetaData.accountName",toString(brickletMetaData.accountName)));
  }
  else
  {
    data.push_back(std::make_pair("dateOfLastChange",""));
    data.push_back(std::make_pair("timeStampOfLastChange",""));
    data.push_back(std::make_pair("BrickletMetaData.fileCreatorName",""));
    data.push_back(std::make_pair("BrickletMetaData.fileCreatorVersion",""));
    data.push_back(std::make_pair("BrickletMetaData.userName",""));
    data.push_back(std::make_pair("BrickletMetaData.accountName",""));
  }

  // brickletID=0 because we are handling resultfile metadata
  std::string fullPathOfCreatedWaves;
  ret = createAndFillTextWave(data, destDataFolderHndl, waveName.c_str(), 0, fullPathOfCreatedWaves);

  if (ret == WAVE_EXIST)
  {
    GlobalData::Instance().setError(ret, waveName);
    return 0;
  }
  else if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  SetOperationStrVar(S_waveNames, fullPathOfCreatedWaves.c_str());
  const bool filledCache = true;
  GlobalData::Instance().finalize(filledCache);
  END_OUTER_CATCH
  return 0;
}
