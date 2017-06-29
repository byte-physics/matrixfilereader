/*
  The file operationsinterface_openresultfile.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "operationstructs.hpp"
#include "operationsinterface.hpp"
#include "globaldata.hpp"
#include "utils_generic.hpp"

extern "C" int ExecuteOpenResultFile(OpenResultFileRuntimeParamsPtr p)
{
  BEGIN_OUTER_CATCH
  GlobalData::Instance().initialize(p->calledFromMacro, p->calledFromFunction);

  // /K will close an possibly open result file before opening a new one
  if (p->KFlagEncountered)
  {
    GlobalData::Instance().closeResultFile();
  }

  if (GlobalData::Instance().resultFileOpen())
  {
    GlobalData::Instance().setError(ALREADY_FILE_OPEN, convertEncoding(GlobalData::Instance().getFileName()));
    return 0;
  }

  char fullPath[MAX_PATH_LEN + 1], fileName[MAX_FILENAME_LEN + 1], dirPath[MAX_PATH_LEN + 1], fileNameOrPath[MAX_PATH_LEN + 1];
  int ret = 0;
  if (p->fileNameOrPathEncountered && GetHandleSize(p->fileNameOrPath) != 0L)
  {
    ret = GetCStringFromHandle(p->fileNameOrPath, fileNameOrPath, MAX_PATH_LEN);

    if (ret != 0)
    {
      GlobalData::Instance().setInternalError(ret);
      return 0;
    }

    // check if we have a symbolic path
    if (p->PFlagEncountered && strlen(p->pathName) > 0L)
    {
      ret = GetFullPathFromSymbolicPathAndFilePath(p->pathName, fileNameOrPath, fullPath);

      if (ret != 0)
      {
        GlobalData::Instance().setInternalError(ret);
        return 0;
      }
    }
    else // if not, fileNameOrPath is an absolute path
    {
      // GetNativePath ensure that the path has only either backslashes or colons but not mixed, as
      // GetDirectoryAndFileNameFromFullPath() does not like that
      ret = GetNativePath(fileNameOrPath, fullPath);

      if (ret != 0)
      {
        GlobalData::Instance().setInternalError(ret);
        return 0;
      }
    }
  }
  // an empty or missing fileNameOrPath parameter results in an openfile dialog being displayed
  else
  {
    DEBUGPRINT("dir=%s,index=%d", GlobalData::Instance().openDlgInitialDir, GlobalData::Instance().openDlgFileIndex);

    // empty initial filename
    fullPath[0] = '\0';
    ret = XOPOpenFileDialog(dlgPrompt , filterStr, &(GlobalData::Instance().openDlgFileIndex), GlobalData::Instance().openDlgInitialDir, fullPath);

    if (ret == -1) //the user cancelled the dialog
    {
      GlobalData::Instance().setError(WRONG_PARAMETER, "fileNameOrPath");
      return 0;
    }
    else if (ret != 0)
    {
      GlobalData::Instance().setInternalError(ret);
      return 0;
    }
  }

  ret = GetDirectoryAndFileNameFromFullPath(fullPath, dirPath, fileName);

  if (ret == WM_BAD_FILE_NAME)
  {
    GlobalData::Instance().setError(FILE_NOT_READABLE, fullPath);
    return 0;
  }
  else if (ret != 0)
  {
    GlobalData::Instance().setInternalError(ret);
    return 0;
  }

  // from here on we have
  // filename : myName.test
  // dirPath c:\data
  // fullPath c:\data\myName.test

  DEBUGPRINT("fullPath %s", fullPath);
  DEBUGPRINT("filename %s", fileName);
  DEBUGPRINT("dirPath %s", dirPath);

  if (!FullPathPointsToFolder(dirPath))
  {
    GlobalData::Instance().setError(FILE_NOT_READABLE, dirPath);
    return 0;
  }

  if (!FullPathPointsToFile(fullPath))
  {
    GlobalData::Instance().setError(FILE_NOT_READABLE, fullPath);
    return 0;
  }

  RemoveAllBackslashesAtTheEnd(dirPath);

  // store the last used directory in GlobalData::Instance().openDlgInitialDir
  strncpy(GlobalData::Instance().openDlgInitialDir, dirPath, MAX_PATH_LEN + 1);
  GlobalData::Instance().openDlgInitialDir[MAX_PATH_LEN] = '\0';

  // first call to getVernissageSession() will result in the DLL loading
  // which must happen in the main thread
  Vernissage::Session* session = GlobalData::Instance().getVernissageSession();

  // now we convert to wide strings
  const std::wstring  dirPathWString = convertEncoding(dirPath);
  const std::wstring  fileNameWString = convertEncoding(fileName);

  // true -> result set will be added to the database
  // false -> replaces the current results sets in the internal databse
  const bool accumulative = false;

  // loading the result set is done in its own thread in order to not block the GUI
  boost::packaged_task<bool> task(boost::bind(&Vernissage::Session::loadResultSet, session, boost::cref(dirPathWString), boost::cref(fileNameWString), accumulative));
  boost::future<bool> loadResult = task.get_future();
  boost::thread thread(boost::move(task));

  while(!thread.try_join_for(boost::chrono::milliseconds(100)))
  {
    // we don't allow the user to abort here
    SpinProcess();
  }

  if (!loadResult.get())
  {
    HISTPRINT("Could not load the result file");
    return 0;
  }

  //starting from here the result file is valid
  GlobalData::Instance().setResultFile(dirPathWString, fileNameWString);

  const int totalNumBricklets = session->getBrickletCount();
  DEBUGPRINT("totalNumBricklets=%d", totalNumBricklets);

  // brickletIDs are 1-based
  void* pContext = NULL;
  for (int i = 1; i <= totalNumBricklets; i++)
  {
    void* vernissageBricklet = session->getNextBricklet(&pContext);

    try
    {
      GlobalData::Instance().createBricklet(i, vernissageBricklet);
    }
    catch (CMemoryException* e)
    {
      e->Delete();
      HISTPRINT("Could not reserve memory for brickletID %d, giving up", i);
      break;
    }
  }

  GlobalData::Instance().finalize();
  END_OUTER_CATCH
  return 0;
}
