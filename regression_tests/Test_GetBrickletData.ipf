#pragma rtGlobals=3    // Use modern global access method.
#pragma ModuleName=GetBrickletData

static Function complains_no_file_open()
	Struct errorCode err
	initStruct(err)

	MFR_GetBrickletData
	CHECK_EQUAL_VAR(err.NO_FILE_OPEN, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_empty_file()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folderEmptyResultFile + fileEmptyResultFile
	MFR_GetBrickletData
	CHECK_EQUAL_VAR(err.EMPTY_RESULTFILE, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_invalid_range()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file

	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	variable numBricklets = V_count
	CHECK(numBricklets > 0)

	variable first, last

	// negative
	first = -1
	last  = -1
	MFR_GetBrickletData/R=(first, last)
	CHECK_EQUAL_VAR(err.INVALID_RANGE, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()

	// wrong order
	first = numBricklets
	last  = 1
	MFR_GetBrickletData/R=(first, last)
	CHECK_EQUAL_VAR(err.INVALID_RANGE, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()

	// range is 1-based
	first = 0
	last  = numBricklets
	MFR_GetBrickletData/R=(first, last)
	CHECK_EQUAL_VAR(err.INVALID_RANGE, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()

	// no intersection
	first = numBricklets+1
	last  = numBricklets*2
	MFR_GetBrickletData/R=(first, last)
	CHECK_EQUAL_VAR(err.INVALID_RANGE, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()

	// single bricklet out of range
	variable index
	index = -1
	MFR_GetBrickletData/R=(index)
	CHECK_EQUAL_VAR(err.INVALID_RANGE, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()

	// 1-based
	index = 0
	MFR_GetBrickletData/R=(index)
	CHECK_EQUAL_VAR(err.INVALID_RANGE, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()

	// 1-based
	index = numBricklets + 1
	MFR_GetBrickletData/R=(index)
	CHECK_EQUAL_VAR(err.INVALID_RANGE, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()

End

static Function complains_empty_wavename()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetBrickletData/N=""
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_wavename_toolong()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetBrickletData/N=PadString("", 256, 0x61)
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
	MFR_GetBrickletData/N="a 'b"
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
	MFR_GetBrickletData/DEST=df
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
	MFR_GetBrickletData/DEST=df
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
	MFR_GetBrickletData/DEST=df
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function complains_invalid_ps()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	variable pixelSize
	pixelSize = 0
	MFR_GetBrickletData/S=(pixelSize)
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()

	pixelSize = -1
	MFR_GetBrickletData/S=(pixelSize)
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()

	pixelSize = 11
	MFR_GetBrickletData/S=(pixelSize)
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()

	pixelSize = 1
	MFR_GetBrickletData/S=(pixelSize)
	CHECK_EQUAL_VAR(err.WRONG_PARAMETER, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK_EMPTY_FOLDER()
End

static Function valid_pixelsizes()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	variable/G V_MatrixFileReaderOverwrite = 1

	variable i
	variable iMax = 10
	for(i = 2; i <= iMax; i+=1)
		MFR_GetBrickletData/S=(i)/R=(1)
		CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
		CHECK(ItemsInList(S_waveNames) == 4) // we have four trace directions
		variable j
		for(j = 0; j < ItemsInList(S_waveNames); j+=1)
			CHECK_WAVE($StringFromList(j, S_waveNames), NUMERIC_WAVE, minorType = DOUBLE_WAVE)
		endfor
	endfor
End

static Function valid_ps_with_liberal_names()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	variable/G V_MatrixFileReaderOverwrite = 1

	variable i
	variable iMax = 10
	for(i = 2; i <= iMax; i+=1)
		MFR_GetBrickletData/S=(i)/R=(1)/N="a b"
		CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
		CHECK(ItemsInList(S_waveNames) == 4) // we have four trace directions
		variable j
		for(j = 0; j < ItemsInList(S_waveNames); j+=1)
			CHECK_WAVE($StringFromList(j, S_waveNames), NUMERIC_WAVE, minorType = DOUBLE_WAVE)
		endfor
	endfor
End

// defaults:
// datafolders yes
// double    no
// overwrite  no
// cache    yes
// debug    no
static Function read_one_bricklet()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	MFR_GetBrickletData/R=(1)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($":X_00001:data_00001_Up", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":X_00001:data_00001_Down", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":X_00001:data_00001_ReUp", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":X_00001:data_00001_ReDown", NUMERIC_WAVE, minorType = DOUBLE_WAVE)

	CHECK(ItemsInList(S_waveNames) == 4)
	variable j
	for(j = 0; j < ItemsInList(S_waveNames); j+=1)
		CHECK_WAVE($StringFromList(j, S_waveNames), NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	endfor
End

static Function read_one_bricklet_df_name()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	NewDataFolder abcd
	dfref df = abcd
	MFR_GetBrickletData/DEST=df/N="myWave"/R=(1)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($":abcd:X_00001:myWave_00001_Up", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":abcd:X_00001:myWave_00001_Down", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":abcd:X_00001:myWave_00001_ReUp", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":abcd:X_00001:myWave_00001_ReDown", NUMERIC_WAVE, minorType = DOUBLE_WAVE)

	CHECK(ItemsInList(S_waveNames) == 4)
	variable j
	for(j = 0; j < ItemsInList(S_waveNames); j+=1)
		Wave/SDFR=df wv = $StringFromList(j, S_waveNames)
		CHECK_WAVE(wv, NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	endfor
End

static Function read_one_bricklet_free_df()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	dfref df = NewFreeDataFolder()
	MFR_GetBrickletData/DEST=df/R=(1)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	Wave/SDFR=df wv1 = $":X_00001:data_00001_Up"
	CHECK_WAVE(wv1, NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	Wave/SDFR=df wv2 = $":X_00001:data_00001_Down"
	CHECK_WAVE(wv2, NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	Wave/SDFR=df wv2 = $":X_00001:data_00001_ReDown"
	CHECK_WAVE(wv2, NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	Wave/SDFR=df wv2 = $":X_00001:data_00001_ReUp"
	CHECK_WAVE(wv2, NUMERIC_WAVE, minorType = DOUBLE_WAVE)

	CHECK(ItemsInList(S_waveNames) == 4)
	variable j
	for(j = 0; j < ItemsInList(S_waveNames); j+=1)
		Wave/SDFR=df wv = $StringFromList(j, S_waveNames)
		CHECK_WAVE(wv, NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	endfor
End

static Function creates_waves()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	MFR_GetBrickletCount
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	variable numBricklets = V_count
	CHECK(numBricklets > 0)

	MFR_GetBrickletData
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	variable i
	for(i = 0; i < ItemsInList(S_waveNames); i+=1)
		CHECK_WAVE($StringFromList(i, S_waveNames), NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	endfor
End

static Function does_not_overwrite_by_default()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	MFR_GetBrickletData/R=(1)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($":X_00001:data_00001_Up", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":X_00001:data_00001_Down", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":X_00001:data_00001_ReUp", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":X_00001:data_00001_ReDown", NUMERIC_WAVE, minorType = DOUBLE_WAVE)

	//aborts immediately if trying to overwrite a file
	MFR_GetBrickletData/R=(1, 2)
	CHECK_EQUAL_VAR(err.WAVE_EXIST, V_flag)
	CHECK_EMPTY_STR(S_waveNames)
	CHECK(!DataFolderExists(":X_00002"))
End

static Function setting_overwrite()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	MFR_GetBrickletData/R=(1)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($":X_00001:data_00001_Up", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":X_00001:data_00001_Down", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":X_00001:data_00001_ReUp", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":X_00001:data_00001_ReDown", NUMERIC_WAVE, minorType = DOUBLE_WAVE)

	variable/G V_MatrixFileReaderOverwrite = 1
	MFR_GetBrickletData/R=(1)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($":X_00001:data_00001_Up", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":X_00001:data_00001_Down", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":X_00001:data_00001_ReUp", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($":X_00001:data_00001_ReDown", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
End

static Function setting_datafolder()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	variable/G V_MatrixFileReaderFolder = 0
	MFR_GetBrickletData/R=(1)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($"data_00001_Up", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($"data_00001_Down", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($"data_00001_ReUp", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($"data_00001_ReDown", NUMERIC_WAVE, minorType = DOUBLE_WAVE)

	variable i
	for(i = 0; i < ItemsInList(S_waveNames); i+=1)
		CHECK_WAVE($StringFromList(i, S_waveNames), NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	endfor
End

static Function setting_double()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	variable/G V_MatrixFileReaderOverwrite = 1
	variable/G V_MatrixFileReaderFolder = 0

	MFR_GetBrickletData/R=(1)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($"data_00001_Up", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($"data_00001_Down", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($"data_00001_ReUp", NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	CHECK_WAVE($"data_00001_ReDown", NUMERIC_WAVE, minorType = DOUBLE_WAVE)

	variable/G V_MatrixFileReaderDouble = 0
	MFR_GetBrickletData/R=(1)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_WAVE($"data_00001_Up", NUMERIC_WAVE, minorType = FLOAT_WAVE)
	CHECK_WAVE($"data_00001_Down", NUMERIC_WAVE, minorType = FLOAT_WAVE)
	CHECK_WAVE($"data_00001_ReUp", NUMERIC_WAVE, minorType = FLOAT_WAVE)
	CHECK_WAVE($"data_00001_ReDown", NUMERIC_WAVE, minorType = FLOAT_WAVE)
End

static Function prefers_dfref_settings()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile/K folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)

	variable/G V_MatrixFileReaderOverwrite = 0
	variable/G V_MatrixFileReaderFolder = 0
	variable/G V_MatrixFileReaderDouble = 0

	dfref dfr = NewFreeDataFolder()

	variable/G dfr:V_MatrixFileReaderOverwrite = 1
	variable/G dfr:V_MatrixFileReaderFolder = 1
	variable/G dfr:V_MatrixFileReaderDouble = 1

	MFR_GetBrickletData/R=(1)/DEST=dfr
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	Wave/SDFR=dfr wv1 = $":X_00001:data_00001_Up"
	CHECK_WAVE(wv1, NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	Wave/SDFR=dfr wv2 = $":X_00001:data_00001_Down"
	CHECK_WAVE(wv2, NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	Wave/SDFR=dfr wv2 = $":X_00001:data_00001_ReDown"
	CHECK_WAVE(wv2, NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	Wave/SDFR=dfr wv2 = $":X_00001:data_00001_ReUp"
	CHECK_WAVE(wv2, NUMERIC_WAVE, minorType = DOUBLE_WAVE)

	MFR_GetBrickletData/R=(1)/DEST=dfr
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
End

static Function liberal_wave_name_works()
	Struct errorCode err
	initStruct(err)

	MFR_OpenResultFile folder + file
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EMPTY_FOLDER()

	MFR_GetBrickletData/N="a b"/R=1
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames), 4)
	variable i
	for(i = 0; i < ItemsInList(S_waveNames); i+=1)
		CHECK_WAVE($StringFromList(i, S_waveNames), NUMERIC_WAVE, minorType = DOUBLE_WAVE)
	endfor
End
