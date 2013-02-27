#pragma rtGlobals=3		// Use modern global access method.
#pragma ModuleName=GetVernissageVersion

static Function vernissage_version_format()
	Struct errorCode err
	initStruct(err)

	MFR_GetVernissageVersion
	
	string version
	sprintf version, "%f", V_DLLVersion
	
	CHECK(GrepString(version,"[[:digit:]]\\.[[:digit:]]"))
End