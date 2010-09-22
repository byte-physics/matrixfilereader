
#include "dll-stuff.h"

#include <vector>
#include <string>

#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

#include "globals.h"
#include "utils.h"

DllStuff::DllStuff():
	m_foundationModule(NULL),
	m_pGetSessionFunc(NULL),
	m_pReleaseSessionFunc(NULL),
	m_vernissageVersion("0.00"){

}

DllStuff::~DllStuff(){

	ASSERT_RETURN_VOID(m_pReleaseSessionFunc);
	(*m_pReleaseSessionFunc) ();
	
	// FIXME this does not return
	// FreeLibrary(foundationModule);
}

// check the registry for the path to the Vernissage DLLs and return (as pointer in the argument) a pointer to the loaded Foundation.dll
// Remarks:
// - Only one vernissage version can be installed at a time, so we take the one which is referenced in the regsitry
// - The length of the arrays is taken from "Registry Element Size Limits"@MSDN
// - The registry key looks like "HKEY_LOCAL_MACHINE\SOFTWARE\Omicron NanoTechnology\Vernissage\V1.0\Main"
Vernissage::Session* DllStuff::createSessionObject(){

	Vernissage::Session *pSession=NULL;
	
	HMODULE module;
	BOOL result;
	char data[16383];
	
	DWORD dataLength = (DWORD) sizeof(data)/sizeof(char);
	HKEY hKey, hregBaseKey;
	std::string regKey;
	char subKeyName[255];
	DWORD subKeyLength = (DWORD) sizeof(subKeyName)/sizeof(WCHAR);
	int subKeyIndex=0;
	std::string regBaseKeyName = "SOFTWARE\\Omicron NanoTechnology\\Vernissage";
	std::string dllName;

	std::vector<std::string> dllNames;
	dllNames.push_back("Foundation.dll");

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,regBaseKeyName.c_str(),0,KEY_READ,&hregBaseKey);

	if(result != ERROR_SUCCESS){
		debugOutputToHistory("Opening the registry key failed. Is Vernissage installed?");
		return pSession;
	}

	result = RegEnumKeyEx(hregBaseKey,subKeyIndex, subKeyName, &subKeyLength,NULL,NULL,NULL,NULL);

	if(result != ERROR_SUCCESS){
		sprintf(pMyData->outputBuffer,"Opening the registry key %s\\%s failed with error code %d. Please reinstall Vernissage.",regBaseKeyName.c_str(),subKeyName,result);
		debugOutputToHistory(pMyData->outputBuffer);
		return pSession;
	}

	regKey  = regBaseKeyName;
	regKey += "\\";
	regKey += subKeyName;
	regKey += "\\Main";

	sprintf(pMyData->outputBuffer,"Checking registry key %s",regKey.c_str());
	debugOutputToHistory(pMyData->outputBuffer);
		
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,regKey.c_str(),0,KEY_READ,&hKey);

	if(result != ERROR_SUCCESS){
		sprintf(pMyData->outputBuffer,"Opening the registry key failed strangely (error code %d). Please reinstall Vernissage.",result);
		debugOutputToHistory(pMyData->outputBuffer);
		return pSession;
	}

	result = RegQueryValueEx(hKey,"InstallDirectory",NULL,NULL,(LPBYTE) data,&dataLength);

	RegCloseKey(hKey);
	RegCloseKey(hregBaseKey);

	if(result != ERROR_SUCCESS){
		sprintf(pMyData->outputBuffer,"Reading the registry key failed very strangely (error code %d). Please reinstall Vernissage.",result);
		debugOutputToHistory(pMyData->outputBuffer);
		return pSession;
	}

	std::string dllDirectory (data);
	dllDirectory.append("\\Bin");
	sprintf(pMyData->outputBuffer, "The path to look for the vernissage DLLs is %s",dllDirectory.c_str());
	debugOutputToHistory(pMyData->outputBuffer);
	result = SetDllDirectory((LPCSTR) dllDirectory.c_str());

	if(!result){
		debugOutputToHistory("Error setting DLL load path");
		return pSession;
	}

	std::string version = subKeyName;
	m_vernissageVersion = version.substr(1,version.length()-1);

	if(m_vernissageVersion.compare(properVernissageVersion) != 0 ){
		sprintf(pMyData->outputBuffer,"Vernissage version %s can not be used to due a bug in this version. Please install version 1.0 and try again.",m_vernissageVersion.c_str());
		outputToHistory(pMyData->outputBuffer);
		return pSession;
	}
	else{
		sprintf(pMyData->outputBuffer,"Vernissage version %s",m_vernissageVersion.c_str());
		debugOutputToHistory(pMyData->outputBuffer);	
	}

	for( std::vector<std::string>::iterator it = dllNames.begin(); it != dllNames.end(); it++){
		dllName = *it;
		module = LoadLibrary( (LPCSTR) dllName.c_str());

		if(module != NULL){
			sprintf(pMyData->outputBuffer,"Successfully loaded DLL %s",dllName.c_str());
			debugOutputToHistory(pMyData->outputBuffer);
		}
		else{
			sprintf(pMyData->outputBuffer,"Something went wrong loading the DLL %s",dllName.c_str());
			debugOutputToHistory(pMyData->outputBuffer);
			return pSession;
		}
	}

	// module is now pointing to Foundation.dll
	m_foundationModule		= module;
	m_pGetSessionFunc		= (GetSessionFunc)		GetProcAddress(m_foundationModule, "getSession");
	m_pReleaseSessionFunc	= (ReleaseSessionFunc)  GetProcAddress(m_foundationModule, "releaseSession");

	if(m_pGetSessionFunc != NULL){
		pSession = (*m_pGetSessionFunc) ();
	}

	return pSession;
}
