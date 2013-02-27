#pragma rtGlobals=3		// Use modern global access method.
#pragma ModuleName=GetBrickletCount

static Function complains_no_file_open()
	Struct errorCode err
	initStruct(err)

	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.NO_FILE_OPEN,V_flag)
	CHECK_EMPTY_FOLDER()
End

static Function works_empty_file()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folderEmptyResultFile + fileEmptyResultFile
	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
	CHECK_EQUAL_VAR(0,V_count)
	CHECK_EMPTY_FOLDER()
End

static Function check_with_filled_file()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
	CHECK_EQUAL_VAR(53,V_count)
	CHECK_EMPTY_FOLDER()
End