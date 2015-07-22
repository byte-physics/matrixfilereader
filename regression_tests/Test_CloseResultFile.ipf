#pragma rtGlobals=3    // Use modern global access method.
#pragma ModuleName=CloseResultFile

static Function complains_no_file_open()
	Struct errorCode err
	initStruct(err)

	MFR_CloseResultFile
	CHECK_EQUAL_VAR(err.NO_FILE_OPEN, V_flag)
	CHECK_EMPTY_FOLDER()
End

static Function works()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_CloseResultFile
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EMPTY_FOLDER()
End
