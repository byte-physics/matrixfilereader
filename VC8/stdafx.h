/*
  The file header.h is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

/*
  Main header file which is included by all source files
*/
#pragma once

// allow the system to be equal or newer than Windows Server 2003 with SP1 or Windows XP with SP2
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <afx.h>

#undef max
#undef min

#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <map>
#include <limits>

#include <boost/smart_ptr.hpp>
#include <boost/numeric/conversion/cast.hpp>

#define BOOST_THREAD_VERSION 3
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "XOPStandardHeaders.h"

#include "utils_xop.hpp"

#include "constants.hpp"
#include "keynames.hpp"
#include "errorcodes.hpp"
#include "version.hpp"

#include "xop_const_wrappers.hpp"

#include "VernissageSDK_T2.2_4/src/Incl/Vernissage.h"
