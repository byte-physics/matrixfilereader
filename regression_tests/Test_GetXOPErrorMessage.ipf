#pragma rtGlobals=3		// Use modern global access method.
#pragma ModuleName=GetXOPErrorMessage

static Function check_that_messages_are_set()
	Struct errorCode err
	initStruct(err)

	variable refNum = CaptureHistoryStart()
	MFR_GetXOPErrorMessage err.SUCCESS
	string str = CaptureHistory(refnum,1)
	CHECK(strlen(str) > 0)

	refNum = CaptureHistoryStart()
	MFR_GetXOPErrorMessage err.ALREADY_FILE_OPEN
	str = CaptureHistory(refnum,1)
	CHECK(strlen(str) > 0)

	refNum = CaptureHistoryStart()
	MFR_GetXOPErrorMessage err.EMPTY_RESULTFILE
	str = CaptureHistory(refnum,1)
	CHECK(strlen(str) > 0)

	refNum = CaptureHistoryStart()
	MFR_GetXOPErrorMessage err.FILE_NOT_READABLE
	str = CaptureHistory(refnum,1)
	CHECK(strlen(str) > 0)

	refNum = CaptureHistoryStart()
	MFR_GetXOPErrorMessage err.NO_NEW_BRICKLETS
	str = CaptureHistory(refnum,1)
	CHECK(strlen(str) > 0)

	refNum = CaptureHistoryStart()
	MFR_GetXOPErrorMessage err.WRONG_PARAMETER
	str = CaptureHistory(refnum,1)
	CHECK(strlen(str) > 0)

	refNum = CaptureHistoryStart()
	MFR_GetXOPErrorMessage err.INTERNAL_ERROR_CONVERTING_DATA
	str = CaptureHistory(refnum,1)
	CHECK(strlen(str) > 0)
	
	refNum = CaptureHistoryStart()
	MFR_GetXOPErrorMessage err.NO_FILE_OPEN
	str = CaptureHistory(refnum,1)
	CHECK(strlen(str) > 0)
	
	refNum = CaptureHistoryStart()
	MFR_GetXOPErrorMessage err.INVALID_RANGE
	str = CaptureHistory(refnum,1)
	CHECK(strlen(str) > 0)

	refNum = CaptureHistoryStart()
	MFR_GetXOPErrorMessage err.WAVE_EXIST
	str = CaptureHistory(refnum,1)
	CHECK(strlen(str) > 0)
End
