#pragma rtGlobals=3    // Use modern global access method.
#pragma ModuleName=CheckForNewBricklets

static Function complains_no_file_open()
	Struct errorCode err
	initStruct(err)

	MFR_CheckForNewBricklets
	CHECK_EQUAL_VAR(err.NO_FILE_OPEN, V_flag)
	CHECK_EMPTY_FOLDER()
End

static Function no_new_bricklets()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_CheckForNewBricklets
	CHECK_EQUAL_VAR(err.NO_NEW_BRICKLETS, V_flag)
	CHECK_EMPTY_FOLDER()
End

// TODO how to test for err.SUCCESS ?
