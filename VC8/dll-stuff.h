
#ifndef DLL_STUFF_H
#define DLL_STUFF_H

#ifndef _WIN32_WINNT		
#define _WIN32_WINNT 0x0502	// needed for SetDLLDirectory
#endif						

#include <windows.h>

#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <ctime>
#include <map>

#include "Vernissage.h"

typedef Vernissage::Session * (*GetSessionFunc) ();
typedef void (*ReleaseSessionFunc) ();

class DllStuff{

	public:
		// de/-constructors
		DllStuff();
		~DllStuff();

		// functions
		Vernissage::Session* createSessionObject();
		std::string getVernissageVersion(){ return m_vernissageVersion;};

	private:
		// variables
		GetSessionFunc m_pGetSessionFunc;
		ReleaseSessionFunc  m_pReleaseSessionFunc;
		HMODULE m_foundationModule;
		std::string m_vernissageVersion;

};

#endif // DLL_STUFF_H