#pragma rtGlobals=3		// Use modern global access method and strict wave access.
#pragma ModuleName=BrickletConversionDetails

static Function check_unique_values(rawData, convDataNames)
	WAVE rawData
	string convDataNames

	variable i, j, dims

	Make/O/N=0 result
	// check that all waves have unique entries
	for(i = 0; i < ItemsInList(convDataNames); i += 1)
		WAVE cube = $StringFromList(i, convDataNames)

		dims = 1
		for(j = 0; j < WaveDims(cube); j += 1)
			dims *= DimSize(cube, j)
		endfor

		Duplicate cube, $("oneDim" + num2str(i))/Wave=line
		Redimension/N=(dims) line
		Duplicate/FREE line, sortedLine
		Sort/A sortedLine, sortedLine
		for(j = 0; j < DimSize(sortedLine, 0) - 1; j += 2)
			REQUIRE_NEQ_VAR(sortedLine[j], sortedLine[j + 1])
		endfor

		Concatenate/NP {sortedLine}, result
	endfor

	Sort/A result, result
	REQUIRE_EQUAL_WAVES(result, rawData, mode=WAVE_DATA)
End

static Function check_image_All()
	Struct errorCode err
	initStruct(err)

	variable numPointsSpecAxis, blockOffset, brickletID, numCols, numRaw, numRows
	string convDataNames
	brickletID = 98

	MFR_OpenResultFile/K folderCube + fileCube
	CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
	CHECK_EMPTY_FOLDER()

	// don't scale the converted data to ease comparison
	// and use artifical data
	variable/G V_MatrixFileReaderMagic  = 1 + 2
	variable/G V_MatrixFileReaderFolder = 0
	variable/G V_MatrixFileReaderDebug  = 1

	MFR_GetBrickletMetaData/R=(brickletID)
	MFR_GetBrickletRawData/R=(brickletID)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
	WAVE rawData = $StringFromList(0, S_waveNames)

	MFR_GetBrickletData/R=(brickletID)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames),4)
	convDataNames = S_waveNames

	check_unique_values(rawData, convDataNames)
End

static Function check_cube_Up_RampFwd()
	Struct errorCode err
	initStruct(err)

	variable numPointsSpecAxis, blockOffset, brickletID, numCols, numRaw, numRows
	string convDataNames
	brickletID = 1

	MFR_OpenResultFile/K folderCubeUp + fileCubeUp
	CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
	CHECK_EMPTY_FOLDER()

	// don't scale the converted data to ease comparison
	// and use artifical data
	variable/G V_MatrixFileReaderMagic  = 1 + 2
	variable/G V_MatrixFileReaderFolder = 0
	variable/G V_MatrixFileReaderDebug  = 1

	MFR_GetBrickletMetaData/R=(brickletID)
	MFR_GetBrickletRawData/R=(brickletID)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
	WAVE rawData = $StringFromList(0, S_waveNames)

	MFR_GetBrickletData/R=(brickletID)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
	convDataNames = S_waveNames

	check_unique_values(rawData, convDataNames)

	WAVE wv = $StringFromList(0, convDataNames)
	numRows           = DimSize(wv, 0)
	numCols           = DimSize(wv, 1)
	numPointsSpecAxis = DimSize(wv, 2)
	numRaw            = DimSize(rawData, 0)

	// traceUP, RampFwd, first spectra
	WAVE cube = $StringFromList(0, convDataNames)

	Duplicate/O/R=[0][0][*] cube slice
	Redimension/N=(numPointsSpecAxis) slice

	Duplicate/O/R=[0, numPointsSpecAxis - 1] rawData sliceRaw
	REQUIRE_EQUAL_WAVES(slice, sliceRaw, mode=WAVE_DATA)
	WaveClear slice, sliceRaw, cube
End

// FIXME no dataset available
//static Function check_cube_UpReUp_RampFwd()
//	Struct errorCode err
//	initStruct(err)
//
//	variable numPointsSpecAxis, blockOffset, brickletID, numCols, numRaw, numRows
//	string convDataNames
//	brickletID = 7
//
//	MFR_OpenResultFile/K folderCubeUp + fileCubeUp
//	CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
//	CHECK_EMPTY_FOLDER()
//
//	// don't scale the converted data to ease comparison
//	// and use artifical data
//	variable/G V_MatrixFileReaderMagic  = 1 + 2
//	variable/G V_MatrixFileReaderFolder = 0
//	variable/G V_MatrixFileReaderDebug  = 1
//
//	MFR_GetBrickletMetaData/R=(brickletID)
//	MFR_GetBrickletRawData/R=(brickletID)
//	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
//	CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
//	WAVE rawData = $StringFromList(0, S_waveNames)
//
//	MFR_GetBrickletData/R=(brickletID)
//	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
//	CHECK_EQUAL_VAR(ItemsInList(S_waveNames),2)
//	convDataNames = S_waveNames
//
//	check_unique_values(rawData, convDataNames)
//
//	WAVE wv = $StringFromList(0, convDataNames)
// 	numRows           = DimSize(wv, 0)
// 	numCols           = DimSize(wv, 1)
// 	numPointsSpecAxis = DimSize(wv, 2)
// 	numRaw            = DimSize(rawData, 0)
//
//	// traceUP, RampFwd, first spectra
//	WAVE cube = $StringFromList(0, convDataNames)
//
//	Duplicate/O/R=[0][0][*] cube slice
//	Redimension/N=(numPointsSpecAxis) slice
//
//	Duplicate/O/R=[0, numPointsSpecAxis - 1] rawData sliceRaw
//	REQUIRE_EQUAL_WAVES(slice, sliceRaw, mode=WAVE_DATA)
//	WaveClear slice, sliceRaw, cube
//	FAIL()
//End

static Function check_cube_Up_Down_RampFwd()
	Struct errorCode err
	initStruct(err)

	variable numPointsSpecAxis, blockOffset, brickletID, numCols, numRaw
	string convDataNames
	brickletID = 88

	MFR_OpenResultFile/K folderCube + fileCube
	CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
	CHECK_EMPTY_FOLDER()

	// don't scale the converted data to ease comparison
	// and use artifical data
	variable/G V_MatrixFileReaderMagic  = 1 + 2
	variable/G V_MatrixFileReaderFolder = 0
	variable/G V_MatrixFileReaderDebug  = 1

	MFR_GetBrickletMetaData/R=(brickletID)
	MFR_GetBrickletRawData/R=(brickletID)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
	WAVE rawData = $StringFromList(0, S_waveNames)

	MFR_GetBrickletData/R=(brickletID)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames),2)
	convDataNames = S_waveNames

	check_unique_values(rawData, convDataNames)

	WAVE wv = $StringFromList(0, convDataNames)
	numPointsSpecAxis = DimSize(wv, 2)
	numCols           = DimSize(wv, 1)
	numRaw            = DimSize(rawData, 0)

	// traceUP, RampFwd, first spectra
	WAVE cube = $StringFromList(0, convDataNames)

	Duplicate/O/R=[0][0][*] cube slice
	Redimension/N=(numPointsSpecAxis) slice

	Duplicate/O/R=[0, numPointsSpecAxis - 1] rawData sliceRaw
	REQUIRE_EQUAL_WAVES(slice, sliceRaw, mode=WAVE_DATA)
	WaveClear slice, sliceRaw, cube

	// traceDown, RampFwd, first spectra
	WAVE cube = $StringFromList(1, convDataNames)

	Duplicate/O/R=[0][numCols - 1][*] cube slice
	Redimension/N=(numPointsSpecAxis) slice

	Duplicate/O/R=[numRaw/2, numRaw/2 + numPointsSpecAxis - 1] rawData sliceRaw
	REQUIRE_EQUAL_WAVES(slice, sliceRaw, mode=WAVE_DATA)
	WaveClear slice, sliceRaw, cube
End

static Function check_cube_ReUp_ReDown_RampFwd()
	Struct errorCode err
	initStruct(err)

	variable numPointsSpecAxis, blockOffset, brickletID, numCols, numRaw, numRows
	string convDataNames
	brickletID = 91

	MFR_OpenResultFile/K folderCube + fileCube
	CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
	CHECK_EMPTY_FOLDER()

	// don't scale the converted data to ease comparison
	// and use artifical data
	variable/G V_MatrixFileReaderMagic  = 1 + 2
	variable/G V_MatrixFileReaderFolder = 0
	variable/G V_MatrixFileReaderDebug  = 1

	MFR_GetBrickletMetaData/R=(brickletID)
	MFR_GetBrickletRawData/R=(brickletID)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
	WAVE rawData = $StringFromList(0, S_waveNames)

	MFR_GetBrickletData/R=(brickletID)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames),2)
	convDataNames = S_waveNames

	check_unique_values(rawData, convDataNames)

	WAVE wv = $StringFromList(0, convDataNames)
	numRows           = DimSize(wv, 0)
	numCols           = DimSize(wv, 1)
	numPointsSpecAxis = DimSize(wv, 2)
	numRaw            = DimSize(rawData, 0)

	// traceReUp, RampFwd, first spectra
	WAVE cube = $StringFromList(0, convDataNames)

	Duplicate/O/R=[numRows - 1][0][*] cube slice
	Redimension/N=(numPointsSpecAxis) slice

	Duplicate/O/R=[0, numPointsSpecAxis - 1] rawData sliceRaw
	REQUIRE_EQUAL_WAVES(slice, sliceRaw, mode=WAVE_DATA)
	WaveClear slice, sliceRaw, cube

	// traceReDown, RampFwd, first spectra
	WAVE cube = $StringFromList(1, convDataNames)

	Duplicate/O/R=[numRows - 1][numCols - 1][*] cube slice
	Redimension/N=(numPointsSpecAxis) slice

	Duplicate/O/R=[numRaw/2, numRaw/2 + numPointsSpecAxis - 1] rawData sliceRaw
	REQUIRE_EQUAL_WAVES(slice, sliceRaw, mode=WAVE_DATA)
	WaveClear slice, sliceRaw, cube
End

static Function check_cube_All_RampFwd()
	Struct errorCode err
	initStruct(err)

	variable numPointsSpecAxis, blockOffset, brickletID, numCols, numRaw, numRows
	string convDataNames
	brickletID = 100

	MFR_OpenResultFile/K folderCube + fileCube
	CHECK_EQUAL_VAR(err.SUCCESS,V_flag)
	CHECK_EMPTY_FOLDER()

	// don't scale the converted data to ease comparison
	// and use artifical data
	variable/G V_MatrixFileReaderMagic  = 1 + 2
	variable/G V_MatrixFileReaderFolder = 0
	variable/G V_MatrixFileReaderDebug  = 1

	MFR_GetBrickletMetaData/R=(brickletID)
	MFR_GetBrickletRawData/R=(brickletID)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames),1)
	WAVE rawData = $StringFromList(0, S_waveNames)

	MFR_GetBrickletData/R=(brickletID)
	CHECK_EQUAL_VAR(err.SUCCESS, V_flag)
	CHECK_EQUAL_VAR(ItemsInList(S_waveNames),4)
	convDataNames = S_waveNames

	check_unique_values(rawData, convDataNames)

	WAVE wv = $StringFromList(0, convDataNames)
	numRows           = DimSize(wv, 0)
	numCols           = DimSize(wv, 1)
	numPointsSpecAxis = DimSize(wv, 2)
	numRaw            = DimSize(rawData, 0)

	// traceUP, RampFwd, first spectra
	WAVE cube = $StringFromList(0, convDataNames)

	Duplicate/O/R=[0][0][*] cube slice
	Redimension/N=(numPointsSpecAxis) slice

	Duplicate/O/R=[0, numPointsSpecAxis - 1] rawData sliceRaw
	REQUIRE_EQUAL_WAVES(slice, sliceRaw, mode=WAVE_DATA) // 1
	WaveClear slice, sliceRaw, cube

	// traceReUp, RampFwd, first spectra
	WAVE cube = $StringFromList(1, convDataNames)

	Duplicate/O/R=[numRows - 1][0][*] cube slice
	Redimension/N=(numPointsSpecAxis) slice

	Duplicate/O/R=[numRows * numPointsSpecAxis, numRows * numPointsSpecAxis + numPointsSpecAxis - 1] rawData sliceRaw
	REQUIRE_EQUAL_WAVES(slice, sliceRaw, mode=WAVE_DATA) // 2
	WaveClear slice, sliceRaw, cube

	// traceDown, RampFwd, first spectra
	WAVE cube = $StringFromList(2, convDataNames)

	Duplicate/O/R=[0][numCols - 1][*] cube slice
	Redimension/N=(numPointsSpecAxis) slice

	Duplicate/O/R=[numRaw/2, numRaw/2 + numPointsSpecAxis - 1] rawData sliceRaw
	REQUIRE_EQUAL_WAVES(slice, sliceRaw, mode=WAVE_DATA) // 3
	WaveClear slice, sliceRaw, cube

	// traceReDown, RampFwd, first spectra
	WAVE cube = $StringFromList(3, convDataNames)

	Duplicate/O/R=[numRows - 1][numCols - 1][*] cube slice
	Redimension/N=(numPointsSpecAxis) slice

	Duplicate/O/R=[numRaw/2 + numRows * numPointsSpecAxis, numRaw/2 + numRows * numPointsSpecAxis + numPointsSpecAxis - 1] rawData sliceRaw
	REQUIRE_EQUAL_WAVES(slice, sliceRaw, mode=WAVE_DATA) // 4
	WaveClear slice, sliceRaw, cube
End
