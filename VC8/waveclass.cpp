/*
	The file wavedata.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/
#include "header.h"

#include "waveclass.h"
#include "globaldata.h"

WaveClass::WaveClass(){
	// set save defaults
	m_traceDir = -1;
	moreData=false;
	m_doublePtr=NULL;
	m_floatPtr=NULL;
	pixelSize=1;
	m_waveHandle=NULL;
}

WaveClass::~WaveClass(){
}

void WaveClass::setWaveDataPtr(const waveHndl &waveHandle){

	if(WaveType(waveHandle) & NT_FP64){
		m_doublePtr = getWaveDataPtr<double>(waveHandle);
		m_floatPtr = NULL;

		if(m_doublePtr == NULL){
			outputToHistory("BUG: setWaveDataPtr(...) m_doublePtr is NULL, this should not happen...");
		}
		else{
			moreData = true;
			m_waveHandle = waveHandle;
		}
	}
	else if(WaveType(waveHandle) & NT_FP32){
		m_doublePtr = NULL;
		m_floatPtr = getWaveDataPtr<float>(waveHandle);

		if(m_floatPtr == NULL){
			outputToHistory("BUG: setWaveDataPtr(...) m_floatPtr is NULL, this should not happen...");
		}
		else{
			moreData = true;
			m_waveHandle = waveHandle;
		}
	}
	else if(WaveType(waveHandle) & NT_I32){
		m_doublePtr = NULL;
		m_floatPtr = NULL;

		m_waveHandle = waveHandle;
	}
	else{
		outputToHistory("wrong datatype in setWaveDataPtr");
	}
}

void WaveClass::setNameAndTraceDir(const std::string &basename, const int &traceDir){

	m_traceDir = traceDir;

	switch(traceDir){
		case NO_TRACE:
			m_wavename = basename;
			break;
		case TRACE_UP:
			m_wavename = basename + TRACE_UP_STRING;
			break;
		case RE_TRACE_UP:
			m_wavename = basename + RE_TRACE_UP_STRING;
			break;
		case TRACE_DOWN:
			m_wavename = basename + TRACE_DOWN_STRING;
			break;
		case RE_TRACE_DOWN:
			m_wavename = basename + RE_TRACE_DOWN_STRING;
			break;
	default:	
		outputToHistory("tracedir is out of range");
		break;
	}
}

void WaveClass::printDebugInfo(){
	sprintf(globDataPtr->outputBuffer,"%s: waveHandle=%p, float=%p, double=%p, moreData=%s",\
		m_wavename.empty() ? "empty" : m_wavename.c_str(), m_waveHandle, m_floatPtr, m_doublePtr, moreData ? "true" : "false");
	debugOutputToHistory(globDataPtr->outputBuffer);
}

void WaveClass::clearWave(){

	if(m_doublePtr){
		waveClearNaN64(m_doublePtr,WavePoints(m_waveHandle));
	}
	else if(m_floatPtr){
		waveClearNaN32(m_floatPtr,WavePoints(m_waveHandle));
	}
}
