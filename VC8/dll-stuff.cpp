
#include "dll-stuff.h"

#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

#include "dataclass.h"

#include "globalvariables.h"

#include "utils.h"

#define DEBUG_LEVEL 1 // standard debug level for this file

using std::string;
using std::vector;

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
// - Currently only Foundation.dll must be loaded, otherwise it crashes seriously. Maybe this has something to do with SetDllDirectory()
Vernissage::Session* DllStuff::createSessionObject(){

	Vernissage::Session *pSession=NULL;

	HMODULE module;
	BOOL result;
	char data[16383];
	char buf[ARRAY_SIZE];
	DWORD dataLength = (DWORD) sizeof(data)/sizeof(char);
	HKEY hKey, hregBaseKey;
	string regKey;
	char subKeyName[255];
	DWORD subKeyLength = (DWORD) sizeof(subKeyName)/sizeof(WCHAR);
	int subKeyIndex=0;
	string regBaseKeyName = "SOFTWARE\\Omicron NanoTechnology\\Vernissage";
	string dllName;

	vector<string> dllNames;
	//dllNames.push_back("Ace.dll");
	//dllNames.push_back("Base.dll");
	//dllNames.push_back("Xerces.dll");
	//dllNames.push_back("Store_XML.dll");
	//dllNames.push_back("Store_ResultWriter.dll");
	//dllNames.push_back("Store_Vernissage.dll");
	dllNames.push_back("Foundation.dll");

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,regBaseKeyName.c_str(),0,KEY_READ,&hregBaseKey);

	if(result != ERROR_SUCCESS){
		debugOutputToHistory(DEBUG_LEVEL,"Opening the registry key failed. Is Vernissage installed?");
		return pSession;
	}

	result = RegEnumKeyEx(hregBaseKey,subKeyIndex, subKeyName, &subKeyLength,NULL,NULL,NULL,NULL);

	if(result != ERROR_SUCCESS){
		sprintf(buf,"Opening the registry key %s\\%s failed with error code %d. Please reinstall Vernissage.",regBaseKeyName.c_str(),subKeyName,result);
		debugOutputToHistory(DEBUG_LEVEL,buf);
		return pSession;
	}

	regKey  = regBaseKeyName;
	regKey += "\\";
	regKey += subKeyName;
	regKey += "\\Main";

	sprintf(buf,"Checking registry key %s",regKey.c_str());
	debugOutputToHistory(DEBUG_LEVEL,buf);
		
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,regKey.c_str(),0,KEY_READ,&hKey);

	if(result != ERROR_SUCCESS){
		sprintf(buf,"Opening the registry key failed strangely (error code %d). Please reinstall Vernissage.",result);
		debugOutputToHistory(DEBUG_LEVEL,buf);
		return pSession;
	}

	result = RegQueryValueEx(hKey,"InstallDirectory",NULL,NULL,(LPBYTE) data,&dataLength);

	RegCloseKey(hKey);
	RegCloseKey(hregBaseKey);

	if(result != ERROR_SUCCESS){
		sprintf(buf,"Reading the registry key failed very strangely (error code %d). Please reinstall Vernissage.",result);
		debugOutputToHistory(DEBUG_LEVEL,buf);
		return pSession;
	}

	string dllDirectory (data);
	dllDirectory.append("\\Bin");
	sprintf(buf, "The path to look for the vernissage DLLs is %s",dllDirectory.c_str());
	debugOutputToHistory(DEBUG_LEVEL,buf);
	result = SetDllDirectory((LPCSTR) dllDirectory.c_str());

	if(!result){
		debugOutputToHistory(DEBUG_LEVEL,"Error setting DLL load path");
		return pSession;
	}

	string version = subKeyName;
	m_vernissageVersion = version.substr(1,version.length()-1);

	if(m_vernissageVersion.compare("1.0") != 0 ){
		sprintf(buf,"Vernissage version %s can not be used to due a bug in this version. Please install version 1.0 and try again.",m_vernissageVersion.c_str());
		outputToHistory(buf);
		return pSession;
	}
	else{
		sprintf(buf,"Vernissage version %s",m_vernissageVersion.c_str());
		debugOutputToHistory(DEBUG_LEVEL,buf);	
	}

	for( vector<string>::iterator it = dllNames.begin(); it != dllNames.end(); it++){
		dllName = *it;
		module = LoadLibrary( (LPCSTR) dllName.c_str());

		if(module != NULL){
			sprintf(buf,"Successfully loaded DLL %s",dllName.c_str());
			debugOutputToHistory(DEBUG_LEVEL,buf);
		}
		else{
			sprintf(buf,"Something went wrong loading the DLL %s",dllName.c_str());
			debugOutputToHistory(DEBUG_LEVEL,buf);
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

	if(pSession == NULL){
		debugOutputToHistory(DEBUG_LEVEL,"pSession is NULL.\n");
		return pSession;
	}
	return pSession;
}
