#pragma rtGlobals=3		// Use modern global access method and strict wave access.

#include "unit-testing"

StrConstant folder = "h:projekte:matrixfilereader-data:TestData-Matrix-V3.0-3.1:2011-01-11:"
StrConstant file = "20110111-164555_STM_0001.mtrx"

StrConstant folderEmptyResultFile = "h:projekte:matrixfilereader-data:TestData-Matrix-V2.2:"
StrConstant fileEmptyResultFile   = "default_2010Oct19-190255_STM-STM_Spectroscopy-VT-STM_0001.mtrx"

StrConstant folderCube = "h:projekte:matrixfilereader-data:TestData-Matrix-V3.0-3.1:2011-01-12:"
StrConstant fileCube = "20110112-144308_STM_0001.mtrx"

StrConstant folderCubeUp = "h:projekte:matrixfilereader-data:TestData-Matrix-V3.2:2015-03-10:"
StrConstant fileCubeUp   = "2015Mar10-112049_STM-RT_0001.mtrx"

StrConstant folderCubeSpecial = "h:projekte:matrixfilereader-data:TestData-Matrix-V3.2:2015-09-16:"
StrConstant	 fileCubeSpecial   = "default_2015Sep16-134954_STM-STM_AtomManipulation_0001.mtrx"

Structure errorCode
	int32 SUCCESS
	int32	 UNKNOWN_ERROR
	int32	 ALREADY_FILE_OPEN
	int32	 EMPTY_RESULTFILE
	int32	 FILE_NOT_READABLE
	int32	 NO_NEW_BRICKLETS
	int32	 WRONG_PARAMETER
	int32	 INTERNAL_ERROR_CONVERTING_DATA
	int32	 NO_FILE_OPEN
	int32	 INVALID_RANGE
	int32	 WAVE_EXIST
EndStructure

Function initStruct(errorCode)
	Struct errorCode &errorCode

	errorCode.SUCCESS =0
	errorCode.UNKNOWN_ERROR=10001
	errorCode.ALREADY_FILE_OPEN=10002
	errorCode.EMPTY_RESULTFILE=10004
	errorCode.FILE_NOT_READABLE=10008
	errorCode.NO_NEW_BRICKLETS=10016
	errorCode.WRONG_PARAMETER=10032
	errorCode.INTERNAL_ERROR_CONVERTING_DATA=10064
	errorCode.NO_FILE_OPEN=10128
	errorCode.INVALID_RANGE=10256
	errorCode.WAVE_EXIST=10512
end

Function TEST_CASE_BEGIN_OVERRIDE(testCase)
	string testCase

	TEST_CASE_BEGIN(testCase)

	MFR_CloseResultFile
End

Function TEST_CASE_END_OVERRIDE(testCase)
	string testCase

	dfref dfr = GetPackageFolder()
	NVAR/Z/SDFR=dfr error_count

	if(NVAR_Exists(error_count) && error_count > 0)
	//	abortNow()
	endif

	TEST_CASE_END(testCase)
End

Function CreateFancyOverViewTable()

	string keys  = "brickletID;scanCycleCount;runCycleCount;sequenceID;dimension;channelName;"
			keys += "X.mirrored;Y.mirrored;V.mirrored;;X.clocks;Y.clocks;V.clocks;"
			keys += "V.AxisTableSet.count;V.AxisTableSetNo1.axis;V.AxisTableSetNo1.axis.start;V.AxisTableSetNo1.axis.stop;V.AxisTableSetNo2.axis;V.AxisTableSetNo2.axis.start;V.AxisTableSetNo2.axis.stop;"
			keys += "V.AxisTableSetNo3.axis;V.AxisTableSetNo3.axis.start;V.AxisTableSetNo3.axis.stop"

	MFR_OpenResultFile/K
	MFR_createOverviewTable/keys=keys
End

Function run()

	SetDataFolder root:
//	EnableDebugOutput()
	DisableDebugOutput()
	string procList=""
	procList += "RegressionTest.ipf;"
	procList += "Test_BrickletConversionDetails.ipf;"
	procList += "Test_CheckForNewBricklets.ipf;Test_CloseResultFile.ipf;Test_CreateOverviewTable.ipf;Test_GetBrickletCount.ipf;"
	procList += "Test_GetBrickletData.ipf;Test_GetBrickletMetaData.ipf;Test_GetBrickletRawData.ipf;Test_GetReportTemplate.ipf;Test_GetResultFileMetaData.ipf;Test_GetBrickletDeployData.ipf;"
	procList += "Test_GetResultFileName.ipf;Test_GetVernissageVersion.ipf;Test_GetVersion.ipf;Test_GetXOPErrorMessage.ipf;Test_OpenResultFile.ipf;"
	RunTest(procList,name="MatrixFileReader XOP")
End
