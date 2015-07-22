#pragma rtGlobals=3    // Use modern global access method.
#pragma ModuleName=GetReportTemplate

Function returns_non_empty_string()
	MFR_GetReportTemplate
	CHECK(strlen(S_value) > 0)
End
