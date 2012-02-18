/*
	The file utils_xop.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/
#pragma once

#include <string>

#include "ForwardDecl.h"

/*
	Utility functions which are xop specific
*/

// Accepts multipe arguments like printf and prints them to the history
// Copies only ARRAY_SIZE-2 characters in _snprintf, because we want to have space for the terminating \0 (1) and for the CR (1)
// @param A prints only if A evaluates to true
// @param B use silent printing (does not mark the experiment as changed) if true
#define PRINT_TO_HISTORY(A,B,...)								\
	if (A)														\
	{															\
		char* buf = &GlobalData::Instance().outputBuffer[0];	\
		_snprintf(buf,ARRAY_SIZE-2, __VA_ARGS__);				\
		buf[ARRAY_SIZE-2] = '\0';								\
		strcat(buf,CR_STR);										\
		if (!B)													\
		{														\
			XOPNotice(buf);										\
		}														\
		else													\
		{														\
			XOPNotice2(buf,0);									\
		}														\
	}

// Convenience wrapper using silent debug print
#define DEBUGPRINT_SILENT(...) \
	PRINT_TO_HISTORY(GlobalData::Instance().debuggingEnabled(),true,"DEBUG: "__VA_ARGS__)

// Convenience wrapper using debug print
#define DEBUGPRINT(...) \
	PRINT_TO_HISTORY(GlobalData::Instance().debuggingEnabled(),false,"DEBUG: "__VA_ARGS__)

// Convenience wrapper which always prints
#define HISTPRINT(...) \
	PRINT_TO_HISTORY(true,false,__VA_ARGS__)

// be sure to check the return value for NULL
template <class T> T* getWaveDataPtr(waveHndl waveH){
	int accessMode = kMDWaveAccessMode0;
	int ret=-1;
	BCInt dataOffset;
	T *dataPtr;

	if(waveH == NULL){
		return NULL;
	}

	ret = MDAccessNumericWaveData(waveH, accessMode, &dataOffset);
	if(ret != 0 ){
		// throw here someting if you want to have it more C++-ish
		return NULL;
	}

	dataPtr = reinterpret_cast<T*>( reinterpret_cast<char*>(*waveH) + dataOffset );
	return dataPtr;
}

void setWaveNoteAsString(const std::string& waveNote, waveHndl waveHandle);

int stringVectorToTextWave(const std::vector<std::string>& metaData,waveHndl waveHandle);

void waveClearNaN64(double *wavePtr, CountInt length);
void waveClearNaN32(float *wavePtr, CountInt length);

std::string getFullWavePath(const DataFolderHandle& df, const waveHndl& wv);
void appendToWaveList(const DataFolderHandle& df, const WaveClass& wave, std::string& waveList);
void appendToWaveList(const DataFolderHandle& df, const waveHndl& wv, std::string& waveList);

void convertHandleToString(Handle strHandle,std::string &str);
