/*
	The file dllhandler.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions, see License.txt in the source folder
	for details.
*/

#include "stdafx.h"

#include "dllhandler.h"
#include "globaldata.h"
#include "utils_generic.h"

DLLHandler::DLLHandler():
	m_foundationModule(NULL),
	m_pGetSessionFunc(NULL),
	m_pReleaseSessionFunc(NULL),
	m_vernissageVersion("0.00"){
}

DLLHandler::~DLLHandler(){}

void DLLHandler::closeSession(){

	if(m_pReleaseSessionFunc != NULL){
		(*m_pReleaseSessionFunc) ();
	}
	
	FreeLibrary(m_foundationModule);
	m_foundationModule = NULL;
	m_vernissageVersion = "0.00";
	m_pReleaseSessionFunc = NULL;
	m_pGetSessionFunc = NULL;
}

/*
Reads the registry, which tells us where the vernissage libraries are located
This path will then be added to the DLL search path
Only one vernissage version can be installed at a time, so we take the one which is referenced in the regsitry
The length of the arrays is taken from "Registry Element Size Limits"@MSDN
The registry key looks like "HKEY_LOCAL_MACHINE\SOFTWARE\Omicron NanoTechnology\Vernissage\V1.0\Main"
*/
std::string DLLHandler::getVernissagePath(){

	BOOL result;
	char data[16383];
	
	DWORD dataLength = (DWORD) sizeof(data)/sizeof(char);
	HKEY hKey, hregBaseKey;
	std::string regKey;
	char subKeyName[255];
	DWORD subKeyLength = (DWORD) sizeof(subKeyName)/sizeof(WCHAR);
	int subKeyIndex=0;
	std::string regBaseKeyName = "SOFTWARE\\Omicron NanoTechnology\\Vernissage";

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,regBaseKeyName.c_str(),0,KEY_READ,&hregBaseKey);

	if(result != ERROR_SUCCESS){
		outputToHistory("Opening a registry key failed. Is Vernissage installed?");
		return std::string();
	}

	result = RegEnumKeyEx(hregBaseKey,subKeyIndex, subKeyName, &subKeyLength,NULL,NULL,NULL,NULL);

	if(result != ERROR_SUCCESS){
		sprintf(GlobalData::Instance().outputBuffer,"Opening the registry key %s\\%s failed with error code %d. Please reinstall Vernissage.",regBaseKeyName.c_str(),subKeyName,result);
		debugOutputToHistory(GlobalData::Instance().outputBuffer);
		return std::string();
	}

	regKey  = regBaseKeyName;
	regKey += "\\";
	regKey += subKeyName;
	regKey += "\\Main";

	sprintf(GlobalData::Instance().outputBuffer,"Checking registry key %s",regKey.c_str());
	debugOutputToHistory(GlobalData::Instance().outputBuffer);
		
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,regKey.c_str(),0,KEY_READ,&hKey);

	if(result != ERROR_SUCCESS){
		sprintf(GlobalData::Instance().outputBuffer,"Opening the registry key failed strangely (error code %d). Please reinstall Vernissage.",result);
		debugOutputToHistory(GlobalData::Instance().outputBuffer);
		return std::string();
	}

	result = RegQueryValueEx(hKey,"InstallDirectory",NULL,NULL,(LPBYTE) data,&dataLength);

	RegCloseKey(hKey);
	RegCloseKey(hregBaseKey);

	if(result != ERROR_SUCCESS){
		sprintf(GlobalData::Instance().outputBuffer,"Reading the registry key failed very strangely (error code %d). Please reinstall Vernissage.",result);
		debugOutputToHistory(GlobalData::Instance().outputBuffer);
		return std::string();
	}

	std::string version = subKeyName;
	m_vernissageVersion = version.substr(1,version.length()-1);

	if(m_vernissageVersion.compare(properVernissageVersion) != 0 ){
		sprintf(GlobalData::Instance().outputBuffer,"Vernissage version %s can not be used. Please install version %s and try again.",m_vernissageVersion.c_str(),properVernissageVersion);
		outputToHistory(GlobalData::Instance().outputBuffer);
		return std::string();
	}
	else{
		sprintf(GlobalData::Instance().outputBuffer,"Vernissage version %s",m_vernissageVersion.c_str());
		debugOutputToHistory(GlobalData::Instance().outputBuffer);	
	}

	std::string dllDirectory(data);
	dllDirectory += "Bin";
	sprintf(GlobalData::Instance().outputBuffer, "The path to look for the vernissage DLLs is %s",dllDirectory.c_str());
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	return dllDirectory;
}

/*
Load the vernissage library, setLibraryPath() will be called before
In case something goes wrong, NULL is returned
*/
Vernissage::Session* DLLHandler::createSessionObject(){

	Vernissage::Session *pSession=NULL;
	std::vector<std::string> dllNames;

	dllNames.push_back("Ace.dll");
	dllNames.push_back("Platform.dll");
	dllNames.push_back("Base.dll");
	dllNames.push_back("Xerces.dll");
	dllNames.push_back("Store_XML.dll");
	dllNames.push_back("Store_ResultWriter.dll");
	dllNames.push_back("Store_Vernissage.dll");
	dllNames.push_back("Foundation.dll");

	std::string dllDirectory = getVernissagePath();

	HMODULE module;
	for( std::vector<std::string>::iterator it = dllNames.begin(); it != dllNames.end(); it++){
		std::string dllName = ( dllDirectory.empty() ?  *it : dllDirectory + "\\" + *it );
		module = LoadLibrary( (LPCSTR) dllName.c_str());

		if(module != NULL){
			sprintf(GlobalData::Instance().outputBuffer,"Successfully loaded DLL %s",dllName.c_str());
			debugOutputToHistory(GlobalData::Instance().outputBuffer);
		}
		else{
			sprintf(GlobalData::Instance().outputBuffer,"Something went wrong (windows error code %d) loading the DLL %s", GetLastError(), dllName.c_str());
			outputToHistory(GlobalData::Instance().outputBuffer);
			return pSession;
		}
	}
	// module is now pointing to Foundation.dll
	m_foundationModule		= module;
	ASSERT_RETURN_ZERO(m_foundationModule);

	m_pGetSessionFunc		= (GetSessionFunc)		GetProcAddress(m_foundationModule, "getSession");
	m_pReleaseSessionFunc	= (ReleaseSessionFunc)  GetProcAddress(m_foundationModule, "releaseSession");

	ASSERT_RETURN_ZERO(m_pGetSessionFunc);
	ASSERT_RETURN_ZERO(m_pReleaseSessionFunc);

	pSession = (*m_pGetSessionFunc) ();
	return pSession;
}
