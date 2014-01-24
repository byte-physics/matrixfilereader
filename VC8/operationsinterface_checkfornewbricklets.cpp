/*
  The file operationsinterface_checkfornewbricklets.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.hpp"
#include "operationsinterface.hpp"
#include "globaldata.hpp"
#include "brickletclass.hpp"
#include "utils_generic.hpp"

extern "C" int ExecuteCheckForNewBricklets(CheckForNewBrickletsRuntimeParamsPtr p)
{
  BEGIN_OUTER_CATCH
  GlobalData::Instance().initialize(p->calledFromMacro, p->calledFromFunction);

  // save defaults
  int ret = SetOperationNumVar(V_startBrickletID, -1.0);

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  ret = SetOperationNumVar(V_endBrickletID, -1.0);

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  if (!GlobalData::Instance().resultFileOpen())
  {
    GlobalData::Instance().setError(NO_FILE_OPEN);
    return 0;
  }

  Vernissage::Session* session = GlobalData::Instance().getVernissageSession();
  ASSERT_RETURN_ZERO(session);

  const int oldNumberOfBricklets = session->getBrickletCount();

  std::wstring fileName = GlobalData::Instance().getFileName<std::wstring>();
  std::wstring dirPath = GlobalData::Instance().getDirPath<std::wstring>();

  // true -> result set will be added to the database
  // false -> replaces the current results sets in the internal databse
  const bool loadSuccess = session->loadResultSet(dirPath, fileName, false);

  if (!loadSuccess)
  {
    HISTPRINT("Could not check for updates of the result file. Maybe it was moved?");
    HISTPRINT("Try closing and opening the result file again.");
    return 0;
  }

  // starting from here we have to
  // - update the vernissageBricklet pointers in the BrickletClass objects
  // - compare old to new totalNumberOfBricklets
  const int numberOfBricklets = session->getBrickletCount();

  void *pContext = NULL;
  for (int i = 1; i <= numberOfBricklets; i++)
  {
    void* vernissageBricklet = session->getNextBricklet(&pContext);
    ASSERT_RETURN_ZERO(vernissageBricklet);

    BrickletClass *bricklet = GlobalData::Instance().getBrickletClassObject(i);

    if (bricklet == NULL) // this is a new bricklet
    {
      GlobalData::Instance().createBrickletClassObject(i, vernissageBricklet);
    }
    else   // the bricklet is old and we only have to update *vernissageBricklet
    {
      bricklet->setBrickletPointer(vernissageBricklet);
    }
  }

  // should not happen
  if (numberOfBricklets < oldNumberOfBricklets)
  {
    HISTPRINT("Error in updating the result file. Please close and reopen it.");
    return 0;
  }

  // from here on we know that numberOfBricklets >= oldNumberOfBricklets
  if (oldNumberOfBricklets == numberOfBricklets)
  {
    GlobalData::Instance().setError(NO_NEW_BRICKLETS);
    return 0;
  }

  ret = SetOperationNumVar(V_startBrickletID, oldNumberOfBricklets + 1);

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  ret = SetOperationNumVar(V_endBrickletID, numberOfBricklets);

  if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  GlobalData::Instance().finalize();
  END_OUTER_CATCH
  return 0;
}
