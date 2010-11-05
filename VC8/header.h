#pragma once

#ifndef _WIN32_WINNT		
	#define _WIN32_WINNT 0x0502	// needed for SetDLLDirectory
#endif					
#include <afx.h>

#include "XOP_Toolkit_6/IgorXOPs6/XOPSupport/xopstandardheaders.h"

#include "VernissageSDK_T2.0_8/src/Incl/vernissage.h"

#pragma comment(lib,"XOP_Toolkit_6/IgorXOPs6/XOPSupport/IGOR.lib")
#pragma comment(lib,"XOP_Toolkit_6/IgorXOPs6/XOPSupport/VC8/XOPSupport.lib")

#include "constants.h"
#include "error-codes.h"
#include "version.h"
