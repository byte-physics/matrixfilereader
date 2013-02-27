#pragma rtGlobals=3		// Use modern global access method.
#pragma ModuleName=GetResultFileName

static Function file_names_are_empty()
	Struct errorCode err
	initStruct(err)

	MFR_GetResultFileName
	CHECK_EQUAL_VAR(err.NO_FILE_OPEN,V_flag)
	CHECK_EMPTY_STR(S_fileName)
	CHECK_EMPTY_STR(S_dirPath)
End

static Function file_names_are_filled()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EMPTY_FOLDER()

	MFR_GetResultFileName
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK(strlen(S_fileName) > 0)
	CHECK(strlen(S_dirPath) > 0)
End

static Function file_names_with_corr_bs()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EMPTY_FOLDER()

	MFR_GetResultFileName
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	// filename has no backslashes
	CHECK(!GrepString(S_fileName,"\\\\"))

	// dirpath is an absolute windows path
	CHECK(GrepString(S_dirPath,"^[[:alpha:]]:\\\\"))
	
	// dirpath has backslashes
	CHECK(GrepString(S_dirPath,"\\\\"))

	// dirpath has no trailing backslash
	CHECK(!GrepString(S_dirPath,"\\\\$"))
End
