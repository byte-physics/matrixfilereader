/*
	The file waveclass.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/

#pragma once

#include "header.h"
#include "extremadata.h"
#include <string>

class WaveClass{

public:
	WaveClass();
	~WaveClass();

	void clearWave();
	void setWaveDataPtr(const waveHndl &waveHandle);
	void setNameAndTraceDir(const std::string &basename, const int &traceDir);
	void printDebugInfo();
	void setWaveScaling(int dimension, const double* sfAPtr, const double* sfBPtr);
	void setWaveUnits(int dimension, const std::wstring& units){ setWaveUnits(dimension,WStringToString(units)); }
	void setWaveUnits(int dimension, const std::string& units);

	// const getters
	const char* getWaveName()const{ return m_wavename.c_str(); }
	waveHndl getWaveHandle()const{ return m_waveHandle; }
	int getTraceDir()const{ return m_traceDir; }
	bool isEmpty()const{ return m_wavename.empty(); }

	// inlined, has to be very fast
	void fillWave(const int &index, const int &rawValue, const double &scaledValue);


public:
	bool moreData;
	ExtremaData extrema;
	int pixelSize;

private:
	std::string m_wavename;
	int m_traceDir;
	waveHndl m_waveHandle;
	float* m_floatPtr;
	double* m_doublePtr;

};

__forceinline void WaveClass::fillWave(const int &index, const int &rawValue, const double &scaledValue){
		if(m_doublePtr){
			m_doublePtr[index] = scaledValue;
		}
		else if(m_floatPtr){
			m_floatPtr[index]  = static_cast<float>(scaledValue);
		}
		else{
			return;
		}

		//check if it is a new minimium
		if(rawValue < extrema.getRawMin()){
			extrema.setRawMin(rawValue);
			extrema.setPhysValRawMin(scaledValue);
		}

		//check if it is a new maximum
		if(rawValue > extrema.getRawMax()){
			extrema.setRawMax(rawValue);
			extrema.setPhysValRawMax(scaledValue);
		}
	}
