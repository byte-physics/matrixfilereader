#pragma rtGlobals=3    // Use modern global access method.
#pragma ModuleName=OpenResultFile

static Function openResultFile_empty_file_path()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile " "
  CHECK_EQUAL_VAR(err.FILE_NOT_READABLE,V_flag)
  CHECK_EMPTY_FOLDER()
End

static Function openResultFile_non_exist_file()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile "F:does_not_exist_or_what"
  CHECK_EQUAL_VAR(err.FILE_NOT_READABLE,V_flag)
  CHECK_EMPTY_FOLDER()
End

static Function openResultFile_empty_file_K()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K " "
  CHECK_EQUAL_VAR(err.FILE_NOT_READABLE,V_flag)
  CHECK_EMPTY_FOLDER()
End

static Function openResultFile_non_exist_file_K()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K "F:does_not_exist_or_what"
  CHECK_EQUAL_VAR(err.FILE_NOT_READABLE,V_flag)
  CHECK_EMPTY_FOLDER()
End

static Function openResultFile_non_exist_path()
  Struct errorCode err
  initStruct(err)

  // FIXME v_flag returning unknown error
  MFR_OpenResultFile/P=myPath " "
  CHECK_EQUAL_VAR(err.UNKNOWN_ERROR,V_flag)
  CHECK_EMPTY_FOLDER()
End

static Function openResultFile_non_exist_path_K()
  Struct errorCode err
  initStruct(err)

  // FIXME v_flag returning unknown error
  MFR_OpenResultFile/K/P=myPath " "
  CHECK_EQUAL_VAR(err.UNKNOWN_ERROR,V_flag)
  CHECK_EMPTY_FOLDER()
End

static Function openResultFile_undefined_path()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/P=myPath "F:does_not_exist_or_what"
  CHECK_EQUAL_VAR(err.FILE_NOT_READABLE,V_flag)
  CHECK_EMPTY_FOLDER()
End

static Function openResultFile_undefined_path_K()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K/P=myPath "F:does_not_exist_or_what"
  CHECK_EQUAL_VAR(err.FILE_NOT_READABLE,V_flag)
  CHECK_EMPTY_FOLDER()
End

static Function openResultFile_undef_path_file()
  Struct errorCode err
  initStruct(err)

  NewPath/Z/Q/O myPath "F:"
  MFR_OpenResultFile/K/P=myPath "does_not_exist_or_what"
  CHECK_EQUAL_VAR(err.UNKNOWN_ERROR,V_flag)
  CHECK_EMPTY_FOLDER()
End

static Function openResultFile_nonexist_path()
  Struct errorCode err
  initStruct(err)

  NewPath/Z/Q/O myPath "C:"
  MFR_OpenResultFile/K/P=myPath "does_not_exist_or_what"
  CHECK_EQUAL_VAR(err.FILE_NOT_READABLE,V_flag)
  CHECK_EMPTY_FOLDER()
End

static Function openResultFile_loading_works()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_EMPTY_FOLDER()

  MFR_GetBrickletCount
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_EQUAL_VAR(53,V_count)
End

static Function openResultFile_loading_works_K()
  Struct errorCode err
  initStruct(err)

  MFR_OpenResultFile/K folder + file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_EMPTY_FOLDER()

  MFR_GetBrickletCount
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_EQUAL_VAR(53,V_count)
End

static Function openResultFile_loading_path()
  Struct errorCode err
  initStruct(err)

  NewPath/Z/Q/O myPath folder
  MFR_OpenResultFile/P=myPath file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_EMPTY_FOLDER()

  MFR_GetBrickletCount
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_EQUAL_VAR(53,V_count)
End

static Function openResultFile_already_open()
  Struct errorCode err
  initStruct(err)

  NewPath/Z/Q/O myPath folder
  MFR_OpenResultFile/P=myPath file
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_EMPTY_FOLDER()

  MFR_GetBrickletCount
  CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
  CHECK_EQUAL_VAR(53,V_count)

  MFR_OpenResultFile
  CHECK_EQUAL_VAR(err.ALREADY_FILE_OPEN,V_flag)
End

//TODO how can we test for a canceled "open file" dialog
//this is the only place where WRONG_PARAMETER is returned
