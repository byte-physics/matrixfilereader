/*
	The file header.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

/*
	Main header file which is included by all source files
*/
#pragma once

// allow the system to be equal or newer than Windows Server 2003 with SP1 or Windows XP with SP2
#ifndef _WIN32_WINNT		
	#define _WIN32_WINNT 0x0502
#endif					

#pragma  warning(push)
#pragma  warning(disable: 4244) // conversion from 'const wchar_t' to 'char', possible loss of data
#include <string>
#pragma  warning(pop)

#include <algorithm>
#include <sstream>
#include <vector>
#include <map>

#include <afx.h>

#undef max
#undef min

#include <limits>

#include "XOPStandardHeaders.h"

#include "utils_xop.h"

#include "constants.h"
#include "keynames.h"
#include "errorcodes.h"
#include "version.h"

#include "xop_const_wrappers.h"

//#include "VernissageSDK_V2.0-1/src/Incl/Vernissage.h"
#include "VernissageSDK_T2.1_7/src/Incl/Vernissage.h"
