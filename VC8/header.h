#pragma once

#ifndef _WIN32_WINNT		
	#define _WIN32_WINNT 0x0502	// needed for SetDLLDirectory
#endif					
#include <afx.h>

#include "IgorXOPs6-Beta-100819/XOPSupport/xopstandardheaders.h"

#include "VernissageSDK_T2.0_8/src/Incl/vernissage.h"

#pragma comment(lib,"IgorXOPs6-Beta-100819/XOPSupport/IGOR.lib")
#pragma comment(lib,"IgorXOPs6-Beta-100819/XOPSupport/VC8/XOPSupport.lib")

#include "constants.h"
#include "error-codes.h"
#include "version.h"
