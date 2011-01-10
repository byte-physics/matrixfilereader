/*
	The file header.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#pragma once

// allow the system to be eual or newer than Windows Server 2003 with SP1 or Windows XP with SP2
#ifndef _WIN32_WINNT		
	#define _WIN32_WINNT 0x0502
#endif					
#include <afx.h>

#include "XOP_Toolkit_6/IgorXOPs6/XOPSupport/xopstandardheaders.h"

#include "VernissageSDK_V2.0/src/Incl/vernissage.h"

#pragma comment(lib,"XOP_Toolkit_6/IgorXOPs6/XOPSupport/IGOR.lib")
#pragma comment(lib,"XOP_Toolkit_6/IgorXOPs6/XOPSupport/VC8/XOPSupport.lib")

#include "constants.h"
#include "errorcodes.h"
#include "version.h"
