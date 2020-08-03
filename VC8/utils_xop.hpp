/*
  The file utils_xop.hpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/
#pragma once

#include <string>

#include "globaldata.hpp"
#include "ForwardDecl.hpp"

/*
  Utility functions which are xop specific
*/

// Accepts multipe arguments like printf and prints them to the history
// Custom prefixes can be also inserted, see DEBUGPRINT
// Copies only ARRAY_SIZE-2 characters in _snprintf, because we want to have space for the terminating \0 (1) and for
// the CR (1) Checking the return value of _snprintf is not done on purpose, we just always append a \0 to be safe
// @param A prints only if A evaluates to true
// @param B uses silent printing (does not mark the experiment as changed) if true
#define PRINT_TO_HISTORY(A, B, ...)                                                                                    \
  if(RunningInMainThread() && A)                                                                                       \
  {                                                                                                                    \
    char *buf = &GlobalData::Instance().outputBuffer[0];                                                               \
    _snprintf(buf, ARRAY_SIZE - 2, __VA_ARGS__);                                                                       \
    buf[ARRAY_SIZE - 2] = '\0';                                                                                        \
    strcat(buf, CR_STR);                                                                                               \
    if(!B)                                                                                                             \
    {                                                                                                                  \
      XOPNotice(buf);                                                                                                  \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
      XOPNotice2(buf, 0);                                                                                              \
    }                                                                                                                  \
  }

// Convenience wrapper using silent debug print
#define DEBUGPRINT_SILENT(...) PRINT_TO_HISTORY(GlobalData::Instance().isDebuggingEnabled(), true, "DEBUG: "__VA_ARGS__)

// Convenience wrapper using debug print
#define DEBUGPRINT(...) PRINT_TO_HISTORY(GlobalData::Instance().isDebuggingEnabled(), false, "DEBUG: "__VA_ARGS__)

// Convenience wrapper for printing
#define HISTPRINT(...) PRINT_TO_HISTORY(true, false, __VA_ARGS__)

// Convenience wrapper for silent printing
#define HISTPRINT_SILENT(...) PRINT_TO_HISTORY(true, true, __VA_ARGS__)

// be sure to check the return value for NULL
template <class T>
T *getWaveDataPtr(waveHndl waveH)
{
  THROW_IF_NULL(waveH);

  BCInt dataOffset;
  const int accessMode = kMDWaveAccessMode0;
  const int ret        = MDAccessNumericWaveData(waveH, accessMode, &dataOffset);

  if(ret != 0)
  {
    throw std::runtime_error("MDAccessNumericWaveData returned error: " + toString(ret));
  }

  return reinterpret_cast<T *>(reinterpret_cast<char *>(*waveH) + dataOffset);
}

int stringVectorToTextWave(const std::vector<std::string> &data, waveHndl waveHandle);

void waveClearNaN64(double *wavePtr, CountInt length);
void waveClearNaN32(float *wavePtr, CountInt length);

void appendToWaveList(DataFolderHandle df, waveHndl wv, std::string &waveList);

void convertHandleToString(Handle strHandle, std::string &str);
bool dataFolderExists(DataFolderHandle df);
std::string getRelativePath(DataFolderHandle df, waveHndl wv);
