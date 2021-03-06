/*
  The file operationsinterface_genericgetbricklet.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.hpp"
#include "operationsinterface.hpp"
#include "utils_bricklet.hpp"
#include "brickletconverter.hpp"
#include "globaldata.hpp"
#include "brickletclass.hpp"
#include "utils_generic.hpp"

namespace  {

  enum TYPE
  {
    RAW_DATA       = 1,
    CONVERTED_DATA = 2,
    META_DATA      = 4
  };

  /*
    Constructs a valid wave name given a buffer of length MAX_OBJ_NAME + 1, a baseName and a brickletID
    Returns 0 on success
  */
  int formatWaveName( char* const buf, const std::string& baseName, int brickletID ) 
  {
    const int size = MAX_OBJ_NAME + 1;
    const int len = _snprintf(buf, size, brickletWaveFormat, baseName.c_str(), brickletID);
    buf[size - 1] = '\0';
    if ( len == size // no space for trailing \0
         || len < 0  // formatted string too long
       )
      return NAME_TOO_LONG;

    return 0;
  }
} // anonymous namespace 

extern "C" int ExecuteGetBrickletData(GetBrickletDataRuntimeParamsPtr p)
{
  GenericGetBrickletParams params;

  params.calledFromFunction   = p->calledFromFunction;
  params.calledFromMacro      = p->calledFromMacro;

  params.NFlagEncountered     = p->NFlagEncountered;
  params.NFlagParamsSet[0]    = p->NFlagParamsSet[0];
  params.baseName             = p->baseName;

  params.RFlagEncountered     = p->RFlagEncountered;
  params.RFlagParamsSet[0]    = p->RFlagParamsSet[0];
  params.RFlagParamsSet[1]    = p->RFlagParamsSet[1];
  params.startBrickletID      = p->startBrickletID;
  params.endBrickletID        = p->endBrickletID;

  params.SFlagEncountered     = p->SFlagEncountered;
  params.SFlagParamsSet[0]    = p->SFlagParamsSet[0];
  params.pixelSize            = p->pixelSize;

  params.DESTFlagEncountered  = p->DESTFlagEncountered;
  params.DESTFlagParamsSet[0] = p->DESTFlagParamsSet[0];
  params.dfref                = p->dfref;

  return GenericGetBricklet(&params, CONVERTED_DATA);
}

int ExecuteGetBrickletMetaData(GetBrickletMetaDataRuntimeParamsPtr p)
{
  GenericGetBrickletParams params;

  params.calledFromFunction   = p->calledFromFunction;
  params.calledFromMacro      = p->calledFromMacro;

  params.NFlagEncountered     = p->NFlagEncountered;
  params.NFlagParamsSet[0]    = p->NFlagParamsSet[0];
  params.baseName             = p->baseName;

  params.RFlagEncountered     = p->RFlagEncountered;
  params.RFlagParamsSet[0]    = p->RFlagParamsSet[0];
  params.RFlagParamsSet[1]    = p->RFlagParamsSet[1];
  params.startBrickletID      = p->startBrickletID;
  params.endBrickletID        = p->endBrickletID;

  params.SFlagEncountered     = 0;
  params.SFlagParamsSet[0]    = 0;
  params.pixelSize            = 0.0;

  params.DESTFlagEncountered  = p->DESTFlagEncountered;
  params.DESTFlagParamsSet[0] = p->DESTFlagParamsSet[0];
  params.dfref                = p->dfref;

  return GenericGetBricklet(&params, META_DATA);
}

extern "C" int ExecuteGetBrickletRawData(GetBrickletRawDataRuntimeParamsPtr p)
{
  GenericGetBrickletParams params;

  params.calledFromFunction   = p->calledFromFunction;
  params.calledFromMacro      = p->calledFromMacro;

  params.NFlagEncountered     = p->NFlagEncountered;
  params.NFlagParamsSet[0]    = p->NFlagParamsSet[0];
  params.baseName             = p->baseName;

  params.RFlagEncountered     = p->RFlagEncountered;
  params.RFlagParamsSet[0]    = p->RFlagParamsSet[0];
  params.RFlagParamsSet[1]    = p->RFlagParamsSet[1];
  params.startBrickletID      = p->startBrickletID;
  params.endBrickletID        = p->endBrickletID;

  params.SFlagEncountered     = 0;
  params.SFlagParamsSet[0]    = 0;
  params.pixelSize            = 0.0;

  params.DESTFlagEncountered  = p->DESTFlagEncountered;
  params.DESTFlagParamsSet[0] = p->DESTFlagParamsSet[0];
  params.dfref                = p->dfref;

  return GenericGetBricklet(&params, RAW_DATA);
}

int GenericGetBricklet(GenericGetBrickletParamsPtr p, int typeOfData)
{
  BEGIN_OUTER_CATCH

  // due to our special handling of the /DEST flag we read the settings later
  GlobalData::Instance().initializeWithoutReadSettings(p->calledFromMacro, p->calledFromFunction);
  int ret = SetOperationStrVar(S_waveNames, "");

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  std::string waveNameList;

  // check of DEST flag which tells us that we should place all output in the supplied datafolder
  // and also read the variable settings from this folder
  DataFolderHandle destDataFolderHndl = NULL;
  if (p->DESTFlagEncountered)
  {
    if (!dataFolderExists(p->dfref))
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

  Vernissage::Session* session = GlobalData::Instance().getVernissageSession();
  ASSERT_RETURN_ZERO(session);

  const int numberOfBricklets = session->getBrickletCount();

  if (numberOfBricklets == 0)
  {
    GlobalData::Instance().setError(EMPTY_RESULTFILE);
    return 0;
  }

  // By default the range is over all bricklets
  int startBrickletID = 1;
  int endBrickletID = numberOfBricklets;

  // the /R flag specifies a single brickletID or a brickletID range
  // Parameter: p->startBrickletID
  // Parameter: p->endBrickletID [optional]
  if (p->RFlagEncountered)
  {
    startBrickletID = int(p->startBrickletID);

    if (p->RFlagParamsSet[1]) // endBrickletID is set
    {
      endBrickletID   = int(p->endBrickletID);
    }
    else
    {
      endBrickletID  = int(p->startBrickletID); // the range is restricted to one bricklet given by startBrickletID
    }
  }

  DEBUGPRINT("startBrickletID=%d, endBrickletID=%d", startBrickletID, endBrickletID);

  if (!isValidBrickletRange(startBrickletID, endBrickletID, numberOfBricklets))
  {
    GlobalData::Instance().setError(INVALID_RANGE);
    return 0;
  }

  // from here on we have a none empty result set open and a valid bricklet range
  std::string baseName;
  if (p->NFlagEncountered)
  {
    if (GetHandleSize(p->baseName) == 0L)
    {
      GlobalData::Instance().setError(WRONG_PARAMETER, "baseName");
      return 0;
    }
    else
    {
      convertHandleToString(p->baseName, baseName);
    }
  }
  else
  {
    // use the default name for the waves
    switch (typeOfData)
    {
    case RAW_DATA:
      baseName = brickletRawDefault;
      break;

    case CONVERTED_DATA:
      baseName = brickletDataDefault;
      break;

    case META_DATA:
      baseName = brickletMetaDefault;
      break;

    default:
      HISTPRINT("BUG: Error in GenericGetBricklet");
      return 0;
      break;
    }
  }

  // check for possible resample flag (only in case we are dealing with converted data)
  int pixelSize = 1;
  bool resampleData = false;
  if (p->SFlagEncountered)
  {
    resampleData = true;
    pixelSize = int(floor(p->pixelSize));

    if (pixelSize < 2 || pixelSize > maximum_pixelSize)
    {
      GlobalData::Instance().setError(WRONG_PARAMETER, "pixelSize");
      return 0;
    }
    DEBUGPRINT("pixelSize=%d", pixelSize);
  }

  for (int brickletID = startBrickletID; brickletID <= endBrickletID; brickletID++)
  {
    BrickletClass* bricklet = GlobalData::Instance().getBrickletClassObject(brickletID);
    ASSERT_RETURN_ZERO(bricklet);

    char waveName[MAX_OBJ_NAME + 1];
    ret = formatWaveName(waveName, baseName, brickletID);
    if (ret != 0)
    {
      GlobalData::Instance().setError(WRONG_PARAMETER, "baseName");
      return 0;
    }

    ret = CheckName(NULL, WAVE_OBJECT, waveName);
    if (ret == NAME_TOO_LONG || ret == BAD_CHAR_IN_WAVE_NAME)
    {
      GlobalData::Instance().setError(WRONG_PARAMETER, "waveName");
      return 0;
    }

    // datafolder handling
    DataFolderHandle brickletDataFolderHndl = NULL;
    if (GlobalData::Instance().isDatafolderEnabled())
    {
      char dataFolderName[MAX_OBJ_NAME + 1];
      sprintf(dataFolderName, dataFolderFormat, brickletID);
      ret = NewDataFolder(destDataFolderHndl, dataFolderName, &brickletDataFolderHndl);

      // continue if the datafolder alrady exists, abort on all other errors
      if (ret != 0 && ret != FOLDER_NAME_EXISTS)
      {
        GlobalData::Instance().setInternalError(ret);
        return 0;
      }
    }
    else
    {
      // we don't use extra datafolders for the bricklets, so they should be created
      // in destDataFolderHndl, see DEST flag handling above
      brickletDataFolderHndl = destDataFolderHndl;
    }

    switch (typeOfData)
    {
    case RAW_DATA:
      ret = createRawDataWave(destDataFolderHndl, brickletDataFolderHndl, waveName, brickletID, waveNameList);
      break;

    case CONVERTED_DATA:
      ret = createWaves(destDataFolderHndl, brickletDataFolderHndl, waveName, brickletID, resampleData, pixelSize, waveNameList);
      break;

    case META_DATA:
      ret = createAndFillTextWave(destDataFolderHndl, bricklet->getMetaData(), brickletDataFolderHndl, waveName, brickletID, waveNameList);
      break;

    default:
      HISTPRINT("Error in GenericGetBricklet");
      return 0;
      break;
    }

    if (!GlobalData::Instance().isDataCacheEnabled())
    {
      bricklet->clearCache();
    }

    if (ret == WAVE_EXIST)
    {
      GlobalData::Instance().setError(ret, waveName);
      return 0;
    }
    else if (ret == INTERNAL_ERROR_CONVERTING_DATA || ret == UNKNOWN_ERROR)
    {
      GlobalData::Instance().setError(ret);
      return 0;
    }
    else if (ret != SUCCESS)
    {
      GlobalData::Instance().setInternalError(ret);
      return 0;
    }

    //check for user abort
    if (SpinProcess() != 0)
    {
      break;
    }
  }

  ret = SetOperationStrVar(S_waveNames, waveNameList.c_str());

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  GlobalData::Instance().finalizeWithFilledCache();
  END_OUTER_CATCH
  return 0;
}
