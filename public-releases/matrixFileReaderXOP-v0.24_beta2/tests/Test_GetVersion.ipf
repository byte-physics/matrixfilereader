#pragma rtGlobals=3    // Use modern global access method.
#pragma ModuleName=GetVersion

Function version_format_is_correct()
  Struct errorCode err
  initStruct(err)

  MFR_GetVersion

  string version
  sprintf version, "%f", V_XOPVersion

  CHECK(GrepString(version,"[[:digit:]]\\.[[:digit:]]{2}"))
End