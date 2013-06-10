#pragma rtGlobals=3    // Use modern global access method.
#pragma ModuleName=RegressionTest
#pragma igorVersion=6.20

//  The file regression_testing.pxp is part of the "MatrixFileReader XOP".
//  It is licensed under the LGPLv3 with additional permissions,
//  see License.txt in the source folder for details.

StrConstant RESULT_FILE_SUFFIX = ".mtrx"

// Load all data from the result file and save it to the disc
Function createData(resultFileFull)
  string resultFileFull

  dfref cdf = GetDataFolderDFR()

  string dataFolder = UniqueName("test",11,0)
  NewDataFolder/S $dataFolder

  variable/G V_MatrixFileReaderOverwrite = 0
  variable/G V_MatrixFileReaderFolder    = 0
  variable/G V_MatrixFileReaderCache     = 1
  variable/G V_MatrixFileReaderDebug     = 0
  variable/G V_MatrixFileReaderDouble    = 0

  MFR_OpenResultFile/K resultFileFull

  MFR_GetBrickletCount
  variable numBricklets = V_count
  if(numBricklets == 0)
    print "No bricklets in the result file"
    return 0
  endif

  MFR_GetResultFileName
  string folder = CleanUpName(RemoveEnding(S_fileName,RESULT_FILE_SUFFIX),1)

  MFR_GetResultFileMetaData
  MFR_CreateOverviewTable

  SaveData/L=1/D=1/O/T=$folder/P=savePath ":"
  Killwaves/Z/A

  variable i
  for(i = 1; i <= numBricklets; i += 1)

    V_MatrixFileReaderDouble    = 0
    MFR_GetBrickletData/R=(i)/N="dataFP32"

    V_MatrixFileReaderDouble    = 1
    MFR_GetBrickletData/R=(i)/N="dataFP64"

    MFR_GetBrickletMetaData/R=(i)
    MFR_GetBrickletRawData/R=(i)
    MFR_GetBrickletDeployData/R=(i)

    SaveData/Q/L=1/D=1/O/T=$folder/P=savePath ":"
    Killwaves/Z/A
  endfor

  MFR_CloseResultFile

  SetdataFolder cdf
  KillDataFolder $dataFolder
End

// compares all waves in the folders on the disc
// every wave existing in refFolderOnDisc also exists in newFolderOnDisc
Function compareDiscFolders(refFolderOnDisc, newFolderOnDisc, [ignoreTextWaves])
  string refFolderOnDisc, newFolderOnDisc
  variable ignoreTextWaves

  if(ParamIsDefault(ignoreTextWaves))
    ignoreTextWaves = 0
  endif

  string refFileList, newFileList
  string refSymPath, newSymPath
  variable numWaves, i

  GetFileFolderInfo/Z/Q newFolderOnDisc
  if(V_flag != 0)
    printf "The path %s does not exist\r", newFolderOnDisc
    return INF
  endif

  GetFileFolderInfo/Z/Q refFolderOnDisc
  if(V_flag != 0)
    printf "The path %s does not exist\r", refFolderOnDisc
    return INF
  endif

  refSymPath="refSymPath"
  newSymPath="newSymPath"

  NewPath/O $refSymPath, refFolderOnDisc
  NewPath/O $newSymPath, newFolderOnDisc

  refFileList = getFilesRecursively(refSymPath, ".ibw")

  numWaves = ItemsInList(refFileList)

  if( numWaves == 0 )
    printf "Found no Igor Binary Waves in %s\r", refFolderOnDisc
    return INF
  endif

//  variable timerRefNum = startMSTimer
//
//  variable numThreads = ThreadProcessorCount
//  variable tgID = ThreadGroupCreate(numThreads)
//
//  variable wavesPerThread = numWaves/numThreads
//  for(i = 0; i < numThreads; i+=1)
//    variable first = i * wavesPerThread
//    variable last
//    // numWaves may not be a divisor of numThreads so we take the rest
//    if( i == numThreads - 1 )
//      last = numWaves
//    else
//      last = first + wavesPerThread - 1
//    endif
//    printf "Starting thread %d with the range [%d, %d]\r", i, first, last
//    ThreadStart tgID, i, CompareRange(refFolderOnDisc, newFolderOnDisc, refFileList, ignoreTextWaves, first, last)
//  endfor
//
//  variable result = ThreadGroupWait(tgID, INF)
//
//  variable numErrors = 0
//  for(i = 0; i < numThreads; i+=1)
//    numErrors += ThreadReturnValue(tgID, i)
//  endfor
//
//  Printf "Multithreaded seconds %d\r", stopMSTimer(timerRefNum)/1e6

//  variable timerRefNum = startMSTimer
  CompareRange(refFolderOnDisc, newFolderOnDisc, refFileList, ignoreTextWaves, 0, numWaves)
//  printf "single threaded seconds %d\r", stopMSTimer(timerRefNum)/1e6
End

Function CompareRange(refFolderOnDisc, newFolderOnDisc, refFileList, ignoreTextWaves, first, last)
  variable first, last, ignoreTextWaves
  string refFolderOnDisc, refFileList, newFolderOnDisc

  string cdf = GetDataFolder(1)

  variable numErrors = 0
  variable i
  for(i=first; i < last; i+=1)

    string fileName = ReplaceString(refFolderOnDisc,StringFromList(i,refFileList),"")

    SetDataFolder cdf
    NewDataFolder/S/O refFolder
    string path = refFolderOnDisc + fileName
    LoadWave/Q/C path
    Wave/Z refWave = $StringFromList(0,S_waveNames)

    SetDataFolder cdf
    NewDataFolder/S/O newFolder
    path = newFolderOnDisc + fileName
    LoadWave/Q/C path
    Wave/Z newWave = $StringFromList(0,S_waveNames)

    numErrors += compareTwoWaves(refWave, newWave, ignoreTextWaves)

    SetDataFolder cdf
    KillDataFolder refFolder
    KillDataFolder newFolder

    if(numErrors == INF)
      break
    endif
  endfor

  SetDataFolder cdf
End

// Compares two waves, removes the variable parts first and then calls CHECK_EQUAL_WAVE
Function compareTwoWaves(refWave,newWave, ignoreTextWaves)
  Wave/Z/T refWave, newWave
  variable ignoreTextWaves

  variable i, j
  variable refWaveCRC, newWaveCRC
  variable waveNoteisEqual, miscStuffIsEqual, dimScalingIsEqual
  string refWaveInfoString, newWaveInfoString
  variable numErrors

  if(!WaveExists(newWave) && !WaveExists(refWave))
    print "Both waves are missing"
    return INF
  elseif(!WaveExists(newWave))
    print "newWave is missing"
    return INF
  elseif(!WaveExists(refWave))
    print "refWave is missing"
    return INF
  elseif(WaveRefsEqual(newWave,refWave))
    print "Don't pass identical waves"
    return INF
  endif

  if( WaveType(newWave) == 0) // non-numeric wave (aka text wave)
    if(ignoreTextWaves)
      return 0
    endif

    // we have to set the resultDirPath to the correct one as the current run might from a different path
    if( cmpstr(refWave[4][0],"resultDirPath") == 0 ) // bricklet metadata
      newWave[4][1] = refWave[4][1]
    endif

    if( cmpstr(refWave[0][0],"resultDirPath") == 0 ) // resultfile metadata
      newWave[0][1] = refWave[0][1]
    endif

    // Starting with Vernissage T2.2-2 the values spatialInfo.physicalX and
    // spatialInfo.physicalY are not written anymore if spatialInfo.originatorKnown is false
    // Therefore we delete these entries from the refWave
    FindValue/TEXT="spatialInfo.physicalX.No1"/TXOP=4 refWave
    variable pos1 = V_value

    FindValue/TEXT="spatialInfo.originatorKnown"/TXOP=4 refWave
    variable pos2 = V_value

    if( pos2 > 0 && pos1 > 0 && cmpstr(refWave[pos2][1],"false") == 0 && cmpstr(refWave[pos1][1],"0") == 0)
	    FindValue/TEXT="spatialInfo.physicalX.No1"/TXOP=4 refWave
	    DeletePoints V_value, 1, refWave

	    FindValue/TEXT="spatialInfo.physicalY.No1"/TXOP=4 refWave
	    DeletePoints V_value, 1, refWave
    endif

  endif

  // remove variable parts of the wave's note
  string refWaveNote  = note(refWave)
  string newWaveNote  = note(newWave)

  refWaveNote = RemoveByKey("vernissageVersion",refWaveNote,"=","\r")
  newWaveNote = RemoveByKey("vernissageVersion",newWaveNote,"=","\r")

  refWaveNote = RemoveByKey("xopVersion",refWaveNote,"=","\r")
  newWaveNote = RemoveByKey("xopVersion",newWaveNote,"=","\r")

  refWaveNote = RemoveByKey("resultDirPath",refWaveNote,"=","\r")
  newWaveNote = RemoveByKey("resultDirPath",newWaveNote,"=","\r")

  Note/K refWave, refWaveNote
  Note/K newWave, newWaveNote

  REQUIRE_EQUAL_WAVES(refWave,newWave)
End

// main entry point for creating data on the disc
// Loads all result files in the selected path
// Calls createData for each result file
Function createDataSet(targetPath, rawDataPath)
  string targetPath, rawDataPath

  string startFolder="MFR_VerifyXOP", resultFile, fileList
  variable i

  NewPath/Q/O/M="Select a folder with many result files" $startFolder, rawDataPath
  if(V_flag != 0)
    return 1
  endif

  fileList = getFilesRecursively(startFolder, RESULT_FILE_SUFFIX)
  KillPath/Z $startFolder

  GetFileFolderInfo/Z/Q targetPath
  if(V_flag == 0)
    printf "The path %s does already exists, pleace rename it manually\r", targetPath
    REQUIRE(0)
    return 1
  endif

  NewPath/O/C/Z savePath, targetPath

  for(i=0; i < ItemsInList(fileList); i+=1)
    resultFile = StringFromList(i,fileList)
    printf "%d, %s\r", i, resultFile
    createData(resultFile)
  endfor

  return 0
End

// Returns a list of all files with the extension given in the symbolic path pathName
// Warning! This function uses recursion, so it might take some time
Function/S getFilesRecursively(pathName, extension, [level])
  String pathName      // Name of symbolic path in which to look for folders and files.
  String extension      // File name extension (e.g., ".txt") or "????" for all files.
  variable level        // indicate level of recursion, do not use for calling the function

  Variable fileIndex, folderIndex, levelValue
  String foundFilesList="", path, fileName, subFolderPathName, subFolderPath, recursFoundFilesList=""

  if(ParamIsDefault(level))
    levelValue = 0
  else
    levelValue = level
  endif

  levelValue+=1

  // get folder name from symbolic path
  PathInfo $pathName
  path = S_path

  if(V_flag == 0)
    printf "path %s does not exist, aborting\r", path
    return foundFilesList
  endif

  string fileNames = IndexedFile($pathName, -1, extension)
  fileIndex  = 0
  // get all files in the folder pathName
  do
    filename = StringFromList(fileIndex,fileNames)
    if (strlen(fileName) == 0)
      break          // No more files
    endif
    foundFilesList = AddListItem(path + fileName, foundFilesList,";", inf)
    fileIndex += 1
  while(1)

  // traverse into the first subfolder and call this function recursively
  string paths = IndexedDir($pathName, -1, 1)
  folderIndex = 0
  do
    path = StringFromList(folderIndex,paths)

    if (strlen(path) == 0)
      break            // No more folders
    endif

    // name of the new symbolic path
    subFolderPathName =  UniqueName("tempPrintFoldersPath_",12,levelValue)
    // Now we get the path to the new parent folder
    subFolderPath = path

    NewPath/Q/O $subFolderPathName, subFolderPath
    recursFoundFilesList = getFilesRecursively(subFolderPathName, extension, level=levelValue)
    KillPath/Z $subFolderPathName

    if(cmpstr(recursFoundFilesList,"") != 0)
      foundFilesList += recursFoundFilesList
    endif

    folderIndex += 1
  while(1)

  return foundFilesList
End

Function diff(wvName)
  string wvName

  variable i, j

  if(WaveType(root:newFolder:$wvName) == 0) // text waves
    wave/T newWaveT = :newFolder:$wvName
    wave/T refWaveT = :refFolder:$wvName

    for(i=0; i < DimSize(newWaveT,0); i+=1)
      if ( cmpstr(newWaveT[i][0],refWaveT[i][0]) != 0 )
        printf "mismatched key: row %d\r", i
        printf "    new _%s_ vs old _%s_\r", newWaveT[i][1], refWaveT[i][1]
      endif

      if ( cmpstr(newWaveT[i][1],refWaveT[i][1]) != 0 )
        printf "mismatched value: row %d\r", i
        printf "    new _%s_ vs old _%s_\r", newWaveT[i][1], refWaveT[i][1]
      endif
    endfor
  else // numeric waves
    // todo check for matching dimensions
    wave newWave = root:newFolder:$wvName
    wave refWave = root:refFolder:$wvName

    for(i=0; i < DimSize(newWave,0); i+=1)
      for(j=0; j < DimSize(newWave,1); j+=1)
        if(newWave[i][j] != refWave[i][j])
          printf "mismatched value: row %d, col %d\r", i, j
          printf "    new _%.15g_ vs old _%.15g_\r", newWave[i][j], refWave[i][j]
        endif
      endfor
    endfor
  endif
End

Function regressionTest()

  string refDataPath = "h:projekte:gitRepo:Coding:matrix-file-reader:igor-xop:regression_tests:referenceData_0.22"
  string rawDataPath = "h:projekte:gitRepo:Coding:matrix-file-reader:TestData-Matrix-V3.0"

  string PATH = "r:newVersion_0.22"
  DeleteFolder/Z=1 PATH
  variable ret
  ret = createDataSet(PATH, rawDataPath)

  if(ret == 1)
      return ret
  endif

  compareDiscFolders(refDataPath, PATH)
End

