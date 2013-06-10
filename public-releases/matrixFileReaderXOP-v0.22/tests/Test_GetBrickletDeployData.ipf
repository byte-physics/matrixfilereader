#pragma rtGlobals=3    // Use modern global access method.
#pragma ModuleName=GetBrickletDeployData

static Function complains_no_file_open()
  Struct errorCode err
  initStruct(err)

  MFR_GetBrickletDeployData
  CHECK_EQUAL_VAR(err.NO_FILE_OPEN,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()
End

static Function complains_empty_file()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folderEmptyResultFile + fileEmptyResultFile
  MFR_GetBrickletDeployData
  CHECK_EQUAL_VAR(err.EMPTY_RESULTFILE,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()
End

static Function complains_invalid_range()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file

  MFR_GetBrickletCount
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  variable numBricklets = V_count
  CHECK(numBricklets > 0)

  variable first, last

  // negative
  first = -1
  last  = -1
  MFR_GetBrickletDeployData/R=(first,last)
  CHECK_EQUAL_VAR(err.INVALID_RANGE,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()

  // wrong order
  first = numBricklets
  last  = 1
  MFR_GetBrickletDeployData/R=(first,last)
  CHECK_EQUAL_VAR(err.INVALID_RANGE,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()

  // range is 1-based
  first = 0
  last  = numBricklets
  MFR_GetBrickletDeployData/R=(first,last)
  CHECK_EQUAL_VAR(err.INVALID_RANGE,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()

  // no intersection
  first = numBricklets+1
  last  = numBricklets*2
  MFR_GetBrickletDeployData/R=(first,last)
  CHECK_EQUAL_VAR(err.INVALID_RANGE,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()

  // single bricklet out of range
  variable index
  index = -1
  MFR_GetBrickletDeployData/R=(index)
  CHECK_EQUAL_VAR(err.INVALID_RANGE,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()

  // 1-based
  index = 0
  MFR_GetBrickletDeployData/R=(index)
  CHECK_EQUAL_VAR(err.INVALID_RANGE,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()

  // 1-based
  index = numBricklets + 1
  MFR_GetBrickletDeployData/R=(index)
  CHECK_EQUAL_VAR(err.INVALID_RANGE,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()

End

static Function complains_empty_wavename()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)

  MFR_GetBrickletDeployData/N=""
  CHECK_EQUAL_VAR(err.WRONG_PARAMETER,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()
End

static Function complains_wavename_toolong()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)

  MFR_GetBrickletDeployData/N="abcdefghijklmnopqrstuvwxyz"
  CHECK_EQUAL_VAR(err.WRONG_PARAMETER,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()
End

static Function complains_baseName_ill_chars()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_EMPTY_FOLDER()

  // invalid characters(') in wave name
  MFR_GetBrickletDeployData/N="a 'b"
  CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()
End

static Function complains_empty_df()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)

  dfref df
  MFR_GetBrickletDeployData/DEST=df
  CHECK_EQUAL_VAR(err.WRONG_PARAMETER,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()
End

static Function complains_invalid_df()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)

  dfref df = does_not_exist
  MFR_GetBrickletDeployData/DEST=df
  CHECK_EQUAL_VAR(err.WRONG_PARAMETER,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()
End

static Function complains_deleted_df()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)

  NewDataFolder abcd
  dfref df = abcd
  KillDataFolder abcd
  MFR_GetBrickletDeployData/DEST=df
  CHECK_EQUAL_VAR(err.WRONG_PARAMETER,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK_EMPTY_FOLDER()
End

// defaults:
// datafolders yes
// double    no
// overwrite  no
// cache    yes
// debug    no
static Function creates_one_wave()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  MFR_GetBrickletDeployData/R=(1)
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_WAVE($":X_00001:deployData_00001", TEXT_WAVE)

  CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
  CHECK_WAVE($StringFromList(0,S_waveNames), TEXT_WAVE)

End

static Function creates_one_wave_df_name()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)

  NewDataFolder abcd
  dfref df = abcd
  MFR_GetBrickletDeployData/DEST=df/N="myWave"/R=(1)
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_WAVE($":abcd:X_00001:myWave_00001", TEXT_WAVE)

  CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
  Wave/Z/SDFR=df wv = $StringFromList(0,S_waveNames)
  CHECK_WAVE(wv, TEXT_WAVE)
End

static Function creates_one_wave_free_df()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)

  dfref df = NewFreeDataFolder()
  MFR_GetBrickletDeployData/DEST=df/R=(1)
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  Wave/SDFR=df wv = $":X_00001:deployData_00001"
  CHECK_WAVE(wv, TEXT_WAVE)

  CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
  Wave/Z/SDFR=df wv = $StringFromList(0,S_waveNames)
  CHECK_WAVE(wv, TEXT_WAVE)
End

static Function creates_waves()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)

  MFR_GetBrickletCount
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  variable numBricklets = V_count
  CHECK(numBricklets > 0)

  variable/G V_MatrixFileReaderFolder = 0
  MFR_GetBrickletDeployData
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)

  variable i
  for(i = 1; i <= numBricklets; i+=1)
    string wvName
    sprintf wvName, "deployData_%05d", i
    CHECK_WAVE($wvName, TEXT_WAVE)
  endfor

  CHECK(ItemsInList(S_waveNames) == numBricklets)

  for(i = 0; i < ItemsInList(S_waveNames); i+=1)
    CHECK_WAVE($StringFromList(i,S_waveNames), TEXT_WAVE)
  endfor
End

static Function does_not_overwrite_by_default()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  MFR_GetBrickletDeployData/R=(1)
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_WAVE($":X_00001:deployData_00001", TEXT_WAVE)

  //aborts immediately if trying to overwrite a file
  MFR_GetBrickletDeployData/R=(1,2)
  CHECK_EQUAL_VAR(err.WAVE_EXIST,V_flag)
  CHECK_EMPTY_STR(S_waveNames)
  CHECK(!WaveExists($":X_00002:deployData_00002"))
End

static Function setting_overwrite()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  MFR_GetBrickletDeployData/R=(1)
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_WAVE($":X_00001:deployData_00001", TEXT_WAVE)

  CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
  CHECK_WAVE($StringFromList(0,S_waveNames), TEXT_WAVE)

  variable/G V_MatrixFileReaderOverwrite = 1
  MFR_GetBrickletDeployData/R=(1)
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_WAVE($":X_00001:deployData_00001", TEXT_WAVE)

  CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
  CHECK_WAVE($StringFromList(0,S_waveNames), TEXT_WAVE)
End

static Function setting_datafolder()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)

  variable/G V_MatrixFileReaderFolder = 0
  MFR_GetBrickletDeployData/R=(1)
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_WAVE($"deployData_00001", TEXT_WAVE)

  CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
  CHECK_WAVE($StringFromList(0,S_waveNames), TEXT_WAVE)
End

static Function prefers_dfref_settings()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)

  variable/G V_MatrixFileReaderOverwrite = 0
  variable/G V_MatrixFileReaderFolder = 0
  variable/G V_MatrixFileReaderDouble = 0

  dfref dfr = NewFreeDataFolder()

  variable/G dfr:V_MatrixFileReaderOverwrite = 1
  variable/G dfr:V_MatrixFileReaderFolder = 1
  variable/G dfr:V_MatrixFileReaderDouble = 1

  MFR_GetBrickletDeployData/R=(1)/DEST=dfr
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  Wave/SDFR=dfr wv1 = $":X_00001:deployData_00001"
  CHECK_WAVE(wv1, TEXT_WAVE)

  MFR_GetBrickletDeployData/R=(1)/DEST=dfr
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
End

static Function liberal_wave_name_works()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_EMPTY_FOLDER()

  MFR_GetBrickletDeployData/N="a b"/R=1
  CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
  CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
  CHECK_WAVE($StringFromList(0,S_waveNames),TEXT_WAVE)
End