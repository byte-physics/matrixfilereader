/*
  The file dllhandler.cpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions, see License.txt in the source folder
  for details.
*/

#include "stdafx.h"

#include "dllhandler.hpp"
#include "globaldata.hpp"
#include "utils_generic.hpp"

DLLHandler::DLLHandler()
  :
  m_foundationModule(NULL),
  m_getSessionFunc(NULL),
  m_releaseSessionFunc(NULL),
  m_vernissageVersion("0.00")
{}

DLLHandler::~DLLHandler()
{}

void DLLHandler::closeSession()
{
  if (m_releaseSessionFunc != NULL)
  {
    (*m_releaseSessionFunc)();
  }

  FreeLibrary(m_foundationModule);
  m_foundationModule = NULL;
  m_vernissageVersion = "0.00";
  m_releaseSessionFunc = NULL;
  m_getSessionFunc = NULL;
}

/*
Reads the registry, which tells us where the vernissage libraries are located
This path will then be added to the DLL search path
The length of the arrays is taken from "Registry Element Size Limits"@MSDN
The registry key looks like "HKEY_LOCAL_MACHINE\SOFTWARE\Omicron NanoTechnology\Vernissage\V1.0\Main"
*/
std::string DLLHandler::getVernissagePath()
{
  BOOL result;
  char data[16383];

  const DWORD dataLength = (DWORD) sizeof(data) / sizeof(char);
  DWORD dataLengthActual;
  HKEY hKey, hregBaseKey;
  std::string regKey, foundRegBaseKey;
  std::vector<std::string> regBaseKeyNames = boost::assign::list_of("SOFTWARE\\Scienta Omicron\\Vernissage");

  for(size_t i = 0; i < regBaseKeyNames.size(); i++)
  {
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regBaseKeyNames[i].c_str(), 0, KEY_READ, &hregBaseKey);

    if(result != ERROR_SUCCESS)
    {
      DEBUGPRINT("Opening a registry key %s failed (error code %d).", regBaseKeyNames[i].c_str(), result);
    }
    else
    {
      foundRegBaseKey = regBaseKeyNames[i];
      DEBUGPRINT("Opening registry key %s suceeded.", foundRegBaseKey.c_str());
      break;
    }
  }

  if(foundRegBaseKey.empty())
  {
	  // check if the wrong bitness of vernissage is installed
	  for (size_t i = 0; i < regBaseKeyNames.size(); i++)
	  {
#ifdef WINIGOR32
		  const int expectedBitness = 32;
		  const REGSAM samDesired = KEY_READ | KEY_WOW64_64KEY;
#else
		  const int expectedBitness = 64;
		  const REGSAM samDesired = KEY_READ | KEY_WOW64_32KEY;
#endif
		  result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regBaseKeyNames[i].c_str(), 0, samDesired, &hregBaseKey);

		  if (result == ERROR_SUCCESS)
		  {
			  RegCloseKey(hregBaseKey);
			  HISTPRINT("Please install the %d-bit version of Vernissage for this XOP.", expectedBitness);
			  return std::string();
		  }
	  }

    HISTPRINT("Could not find a Vernissage installation.");
    return std::string();
  }

  dataLengthActual = dataLength;
  result = RegQueryValueEx(hregBaseKey, "RecentVersion", NULL, NULL, (LPBYTE) data, &dataLengthActual);

  if (result != ERROR_SUCCESS)
  {
    DEBUGPRINT("Reading the registry key %s:%s failed (error code %d).", foundRegBaseKey.c_str(), "RecentVersion", result);
    return std::string();
  }

  RegCloseKey(hregBaseKey);

  // RecentVersion is "V2.4.1"
  m_vernissageVersion = std::string(data).substr(1);
  std::string majorVersion = m_vernissageVersion.substr(0, 3);

  typedef std::vector<std::string>::const_iterator VecCIt;
  VecCIt it = std::find(supportedMajorVernissageVersions.begin(), supportedMajorVernissageVersions.end(), majorVersion);
  if (it == supportedMajorVernissageVersions.end())
  {
    HISTPRINT("Vernissage version %s is not supported. Please install one of the supported versions and try again.", m_vernissageVersion.c_str());
    return std::string();
  }
  else
  {
    DEBUGPRINT("Vernissage version %s", m_vernissageVersion.c_str());
  }

  regKey  = foundRegBaseKey;
  regKey += "\\";
  regKey += data;
  regKey += "\\Main";

  DEBUGPRINT("Checking registry key %s", regKey.c_str());

  result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regKey.c_str(), 0, KEY_READ, &hKey);

  if (result != ERROR_SUCCESS)
  {
    DEBUGPRINT("Opening the registry key failed (error code %d). Please reinstall Vernissage.", result);
    return std::string();
  }

  dataLengthActual = dataLength;
  result = RegQueryValueEx(hKey, "InstallDirectory", NULL, NULL, (LPBYTE) data, &dataLengthActual);

  RegCloseKey(hKey);

  if (result != ERROR_SUCCESS)
  {
    DEBUGPRINT("Reading the registry value %s:%s failed (error code %d). Please reinstall Vernissage.", regKey.c_str(), "InstallDirectory", result);
    return std::string();
  }

  std::string dllDirectory(data);
  dllDirectory += "Bin";
  DEBUGPRINT("The path to look for the vernissage DLLs is %s", dllDirectory.c_str());

  return dllDirectory;
}

/*
Load the vernissage library, setLibraryPath() will be called before
In case something goes wrong, NULL is returned
*/
Vernissage::Session* DLLHandler::createSessionObject()
{
  Vernissage::Session* session = NULL;
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

  for (std::vector<std::string>::iterator it = dllNames.begin(); it != dllNames.end(); it++)
  {
    std::string dllName = (dllDirectory.empty() ?  *it : dllDirectory + "\\" + *it);
    module = LoadLibrary((LPCSTR) dllName.c_str());

    if (module != NULL)
    {
      DEBUGPRINT("Successfully loaded DLL %s", dllName.c_str());
    }
    else
    {
      HISTPRINT("Something went wrong (windows error code %d) loading the DLL %s", GetLastError(), dllName.c_str());
      return session;
    }
  }

  // module is now pointing to Foundation.dll
  m_foundationModule    = module;
  ASSERT_RETURN_ZERO(m_foundationModule);

  m_getSessionFunc     = (GetSessionFunc)     GetProcAddress(m_foundationModule, "getSession");
  m_releaseSessionFunc = (ReleaseSessionFunc) GetProcAddress(m_foundationModule, "releaseSession");

  ASSERT_RETURN_ZERO(m_getSessionFunc);
  ASSERT_RETURN_ZERO(m_releaseSessionFunc);

  session = (*m_getSessionFunc)();
  return session;
}

const std::string& DLLHandler::getVernissageVersion() const
{
  return m_vernissageVersion;
}
