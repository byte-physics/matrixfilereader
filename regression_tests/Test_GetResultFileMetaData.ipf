#pragma rtGlobals=3    // Use modern global access method.
#pragma ModuleName=GetResultFileMetaData

static Function complains_no_file_open()
	Struct errorCode err
	initStruct(err)

	MFR_GetResultFileMetaData
	CHECK_EQUAL_VAR(err.NO_FILE_OPEN, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_empty_wavename()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetResultFileMetaData/N=""
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_wavename_toolong()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetResultFileMetaData/N=PadString("", 256, 0x61)
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_empty_df()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	dfref df
	MFR_GetResultFileMetaData/DEST=df
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_invalid_df()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	dfref df = does_not_exist
	MFR_GetResultFileMetaData/DEST=df
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_waveName_ill_chars()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EMPTY_FOLDER()

	// invalid characters(') in wave name
	MFR_GetResultFileMetaData/N="a 'b"
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_deleted_df()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	NewDataFolder abcd
	dfref df = abcd
	KillDataFolder abcd
	MFR_GetResultFileMetaData/DEST=df
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function works_with_empty_file()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folderEmptyResultFile + fileEmptyResultFile
	MFR_GetResultFileMetaData
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($"resultFileMetaData", TEXT_WAVE)

	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	CHECK_WAVE($StringFromList(0, S_waveNames), TEXT_WAVE)
End

static Function creates_one_wave()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	MFR_GetResultFileMetaData
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($"resultFileMetaData", TEXT_WAVE)

	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	CHECK_WAVE($StringFromList(0, S_waveNames), TEXT_WAVE)
End

static Function creates_one_wave_df_name()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	NewDataFolder abcd
	dfref df = abcd
	MFR_GetResultFileMetaData/DEST=df/N="myWave"
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($":abcd:myWave", TEXT_WAVE)

	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	Wave/SDFR=df wv = $StringFromList(0, S_waveNames)
	CHECK_WAVE(wv, TEXT_WAVE)
End

static Function creates_one_wave_free_df()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	dfref df = NewFreeDataFolder()
	MFR_GetResultFileMetaData/DEST=df
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	Wave/SDFR=df wv = $"resultFileMetaData"
	CHECK_WAVE(wv, TEXT_WAVE)

	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	Wave/SDFR=df wv = $StringFromList(0, S_waveNames)
	CHECK_WAVE(wv, TEXT_WAVE)
End

static Function does_not_overwrite_by_default()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	MFR_GetResultFileMetaData
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($"resultFileMetaData", TEXT_WAVE)

	MFR_GetResultFileMetaData
	CHECK_EQUAL_VAR(err.WAVE_EXIST, V_flag)
End

static Function overwrites_if_told()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	MFR_GetResultFileMetaData
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($"resultFileMetaData", TEXT_WAVE)

	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	CHECK_WAVE($StringFromList(0, S_waveNames), TEXT_WAVE)

	variable/G V_MatrixFileReaderOverwrite = 1
	MFR_GetResultFileMetaData
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($"resultFileMetaData", TEXT_WAVE)

	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	CHECK_WAVE($StringFromList(0, S_waveNames), TEXT_WAVE)
End

static Function liberal_wave_name_works()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EMPTY_FOLDER()

	MFR_GetResultFileMetaData/N="a b"
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	CHECK_WAVE($StringFromList(0, S_waveNames), TEXT_WAVE)
End
