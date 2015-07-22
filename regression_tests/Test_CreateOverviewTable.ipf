#pragma rtGlobals=3    // Use modern global access method.
#pragma ModuleName=CreateOverviewTable

static Function complains_no_file_open()
	Struct errorCode err
	initStruct(err)

	MFR_CreateOverviewTable
	CHECK_EQUAL_VAR(err.NO_FILE_OPEN, V_flag)
	CHECK_EMPTY_FOLDER()
End

static Function complains_empty_file()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folderEmptyResultFile + fileEmptyResultFile
	MFR_CreateOverviewTable
	CHECK_EQUAL_VAR(err.EMPTY_RESULTFILE, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_empty_wavename()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_CreateOverviewTable/N=""
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_wavename_toolong()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_CreateOverviewTable/N="abcdefghijklmnopqrstuvwxyzabcdefghij"
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_baseName_ill_chars()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EMPTY_FOLDER()

	// invalid characters(') in wave name
	MFR_CreateOverviewTable/N="a 'b"
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
	MFR_CreateOverviewTable/DEST=df
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
	MFR_CreateOverviewTable/DEST=df
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
	MFR_CreateOverviewTable/DEST=df
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_empty_keys()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_CreateOverviewTable/KEYS=""
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_not_with_invalid_keys()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	variable numBricklets = V_count

	MFR_CreateOverviewTable/KEYS="non_existing_key"
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	Wave/Z wv = $StringFromList(0, S_waveNames)
	CHECK_WAVE(wv, TEXT_WAVE)
	CHECK_EQUAL_VAR(DimSize(wv, 0), numBricklets)
	CHECK_EQUAL_VAR(DimSize(wv, 1), 1)
End

static Function complains_not_with_toolong_key()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	variable numBricklets = V_count

	// toolong keys are currently silently truncated
	MFR_CreateOverviewTable/KEYS="non_existing_key_very_long_string_here_really"
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
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
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	variable numBricklets = V_count

	MFR_CreateOverviewTable
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($"overviewTable", TEXT_WAVE)

	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	Wave/Z wv = $StringFromList(0, S_waveNames)
	CHECK_WAVE(wv, TEXT_WAVE)
	CHECK_EQUAL_VAR(DimSize(wv, 0), numBricklets)
	CHECK_EQUAL_VAR(DimSize(wv, 1), 6) // we have 6 keys by default
End

static Function creates_one_wave_df_name()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	variable numBricklets = V_count

	NewDataFolder abcd
	dfref df = abcd
	MFR_CreateOverviewTable/DEST=df/N="myWave"
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($":abcd:myWave", TEXT_WAVE)

	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	Wave/Z/SDFR=df wv = $StringFromList(0, S_waveNames)
	CHECK_WAVE(wv, TEXT_WAVE)
	CHECK_EQUAL_VAR(DimSize(wv, 0), numBricklets)
	CHECK_EQUAL_VAR(DimSize(wv, 1), 6) // we have 6 keys by default
End

static Function creates_one_wave_free_df()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	variable numBricklets = V_count

	dfref df = NewFreeDataFolder()
	MFR_CreateOverviewTable/DEST=df
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	Wave/Z/SDFR=df wv = $"overviewTable"
	CHECK_WAVE(wv, TEXT_WAVE)

	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	Wave/Z/SDFR=df wv = $StringFromList(0, S_waveNames)
	CHECK_WAVE(wv, TEXT_WAVE)
End

static Function creates_one_wave_user_keys()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	variable numBricklets = V_count

	MFR_CreateOverviewTable/KEYS="brickletID;cycleCount"
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($"overviewTable", TEXT_WAVE)

	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	Wave/Z wv = $StringFromList(0, S_waveNames)
	CHECK_WAVE(wv, TEXT_WAVE)
	CHECK_EQUAL_VAR(DimSize(wv, 0), numBricklets)
	CHECK_EQUAL_VAR(DimSize(wv, 1), 2) // we have 6 keys by default
End

static Function does_not_overwrite_by_default()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	variable numBricklets = V_count

	MFR_CreateOverviewTable
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($"overviewTable", TEXT_WAVE)

	MFR_CreateOverviewTable
	CHECK_EQUAL_VAR(err.WAVE_EXIST, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
End

static Function setting_overwrite()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	variable numBricklets = V_count

	MFR_CreateOverviewTable
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	Wave/Z wv = $StringFromList(0, S_waveNames)
	CHECK_WAVE(wv, TEXT_WAVE)

	variable/G V_MatrixFileReaderOverwrite = 1
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	Wave/Z wv = $StringFromList(0, S_waveNames)
	CHECK_WAVE(wv, TEXT_WAVE)
End

static Function prefers_dfref_settings()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	variable/G V_MatrixFileReaderOverwrite = 0

	dfref dfr = NewFreeDataFolder()

	variable/G dfr:V_MatrixFileReaderOverwrite = 1

	MFR_CreateOverviewTable/DEST=dfr
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	Wave/SDFR=dfr wv1 = $"overviewTable"
	CHECK_WAVE(wv1, TEXT_WAVE)

	MFR_CreateOverviewTable/DEST=dfr
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
End

static Function liberal_wave_name_works()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EMPTY_FOLDER()

	MFR_CreateOverviewTable/N="a b"
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 1)
	CHECK_WAVE($StringFromList(0, S_waveNames), TEXT_WAVE)
End
