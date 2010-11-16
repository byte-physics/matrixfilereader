/*
	The file dllhandler.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#pragma once

#include "header.h"

typedef Vernissage::Session * (*GetSessionFunc) ();
typedef void (*ReleaseSessionFunc) ();

class DLLHandler{

	public:
		// de/-constructors
		DLLHandler();
		~DLLHandler();

		// functions
		Vernissage::Session* createSessionObject();
		void setLibraryPath();
		void closeSession();
		std::string getVernissageVersion(){ return m_vernissageVersion;};

	private:
		// variables
		GetSessionFunc m_pGetSessionFunc;
		ReleaseSessionFunc  m_pReleaseSessionFunc;
		HMODULE m_foundationModule;
		std::string m_vernissageVersion;
};
