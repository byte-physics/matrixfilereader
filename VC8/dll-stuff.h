
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
		~DllStuff(){};

		// functions
		void closeSession();
		Vernissage::Session* createSessionObject();

	private:
		// variables
		GetSessionFunc m_pGetSessionFunc;
		ReleaseSessionFunc  m_pReleaseSessionFunc;
		HMODULE m_foundationModule;
		std::string m_vernissageVersion;

		int m_debuglevel;
		// functions


		void debugOutputToHistory(int level, char *str);
		void outputToHistory(char *str);
};



#endif // DLL_STUFF_H