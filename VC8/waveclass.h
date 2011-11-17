/*
	The file waveclass.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/
#pragma once

#include "stdafx.h"

#include <string>
#include "ForwardDecl.h"
#include "extremadata.h"

/*
	Internal representation of a igor wave
*/

class WaveClass{

public:
	WaveClass();
	~WaveClass();

	void clearWave();
	void setWaveHandle(const waveHndl &waveHandle);
	void setNameAndTraceDir(const std::string &basename, int traceDir);
	void printDebugInfo();
	void setWaveScaling(int dimension, const double* sfAPtr, const double* sfBPtr);
	void setWaveUnits(int dimension, const std::wstring& units);
	void setWaveUnits(int dimension, const std::string& units);
	void setExtrema(const ExtremaData& extremaData);

	// const getters
	const char* getWaveName()const{ return m_wavename.c_str(); }
	waveHndl getWaveHandle()const{ return m_waveHandle; }
	int getTraceDir()const{ return m_traceDir; }
	bool isEmpty()const{ return m_wavename.empty(); }
	const ExtremaData& getExtrema()const{ return *m_extrema; };

	// inlined, has to be very fast
	void fillWave(const int &index, const int &rawValue, const double &scaledValue);

public:
	bool moreData;
	int pixelSize;

private:
	ExtremaData *m_extrema;
	std::string m_wavename;
	int m_traceDir;
	waveHndl m_waveHandle;
	float* m_floatPtr;
	double* m_doublePtr;

};

/*
	Writes the data into the wave
	It will _not_ be checked if index is out-of-range
	Here we also determine the extrema values
*/
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
		if(rawValue < m_extrema->getRawMin()){
			m_extrema->setRawMin(rawValue);
			m_extrema->setPhysValRawMin(scaledValue);
		}

		//check if it is a new maximum
		if(rawValue > m_extrema->getRawMax()){
			m_extrema->setRawMax(rawValue);
			m_extrema->setPhysValRawMax(scaledValue);
		}
	}
