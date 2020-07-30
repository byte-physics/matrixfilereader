/*
  The file operationsinterface_createoverviewtable.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.hpp"
#include "operationsinterface.hpp"
#include "globaldata.hpp"
#include "utils_bricklet.hpp"
#include "bricklet.hpp"
#include "utils_generic.hpp"

extern "C" int ExecuteCreateOverviewTable(CreateOverviewTableRuntimeParamsPtr p)
{
  BEGIN_OUTER_CATCH
  GlobalData::Instance().initializeWithoutReadSettings(p->calledFromMacro, p->calledFromFunction);
  int ret = SetOperationStrVar(S_waveNames, "");

  if(ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  std::string keyList, waveName;
  waveHndl waveHandle;
  DataFolderHandle destDataFolderHndl = NULL;

  CountInt dimensionSizes[MAX_DIMENSIONS + 1];
  MemClear(dimensionSizes, sizeof(dimensionSizes));

  // check of DEST flag which tells us that we should place all output in the supplied datafolder
  // and also read the variable settings from this folder
  if(p->DESTFlagEncountered)
  {
    if(!dataFolderExists(p->dfref))
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

    if(ret != 0)
    {
      GlobalData::Instance().setInternalError(ret);
      return 0;
    }
  }

  GlobalData::Instance().readSettings(destDataFolderHndl);

  if(!GlobalData::Instance().resultFileOpen())
  {
    GlobalData::Instance().setError(NO_FILE_OPEN);
    return 0;
  }

  Vernissage::Session *session = getVernissageSession();
  const int numberOfBricklets  = session->getBrickletCount();

  if(numberOfBricklets == 0)
  {
    GlobalData::Instance().setError(EMPTY_RESULTFILE);
    return 0;
  }

  // check keyList parameter
  if(p->KEYSFlagEncountered)
  {
    if(GetHandleSize(p->keyList) == 0L)
    {
      GlobalData::Instance().setError(WRONG_PARAMETER, "keyList");
      return 0;
    }
    else
    {
      convertHandleToString(p->keyList, keyList);
    }
  }
  else
  {
    keyList = keyList_default;
  }

  // check waveName parameter
  if(p->NFlagEncountered)
  {
    if(GetHandleSize(p->waveName) == 0L)
    {
      GlobalData::Instance().setError(WRONG_PARAMETER, "waveName");
      return 0;
    }
    else
    {
      convertHandleToString(p->waveName, waveName);
    }
  }
  else
  {
    waveName = overViewTableDefault;
  }

  ret = CheckName(NULL, WAVE_OBJECT, waveName.c_str());
  if(ret == WAVE_NAME_TOO_LONG || ret == BAD_CHAR_IN_WAVE_NAME)
  {
    GlobalData::Instance().setError(WRONG_PARAMETER, "waveName");
    return 0;
  }

  std::vector<std::string> keys;
  splitString(keyList, listSepChar, keys);

  if(keys.empty())
  {
    GlobalData::Instance().setError(WRONG_PARAMETER, "keyList");
    return 0;
  }

  dimensionSizes[ROWS]    = numberOfBricklets;
  dimensionSizes[COLUMNS] = keys.size();

  ret = MDMakeWave(&waveHandle, waveName.c_str(), destDataFolderHndl, dimensionSizes, TEXT_WAVE_TYPE,
                   isOverwriteEnabled());

  if(ret == NAME_WAV_CONFLICT)
  {
    DEBUGPRINT("Wave %s already exists.", waveName.c_str());
    GlobalData::Instance().setError(WAVE_EXIST, waveName);
    return 0;
  }
  else if(ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  ASSERT_RETURN_ZERO(waveHandle);

  loadAllBrickletMetaData();

  std::vector<std::string> content;
  content.reserve(keys.size() * numberOfBricklets);

  for(unsigned int j = 0; j < keys.size(); j++)
  {
    const std::string key = keys[j];
    // DEBUGPRINT("key=%s", key.c_str());
    int ret = MDSetDimensionLabel(waveHandle, COLUMNS, j, key.c_str());
    if(ret != 0)
    {
      DEBUGPRINT("MDSetDimensionLabel returned %d", ret);
      return ret;
    }

    for(int i = 1; i <= numberOfBricklets; i++)
    {
      Bricklet &bricklet      = GlobalData::Instance().getBricklet(i);
      const std::string value = bricklet.getMetaDataValue<std::string>(key);
      content.push_back(value);
      // DEBUGPRINT("   value=%s", value.c_str());
    }
  }

  // needs data in column-major format
  ret = stringVectorToTextWave(content, waveHandle);

  if(ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  setOtherWaveNote(waveHandle);

  std::string waveList;
  appendToWaveList(destDataFolderHndl, waveHandle, waveList);

  ret = SetOperationStrVar(S_waveNames, waveList.c_str());

  if(ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  GlobalData::Instance().finalizeWithFilledCache();
  END_OUTER_CATCH
  return 0;
}
