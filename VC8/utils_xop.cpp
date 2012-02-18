/*
	The file utils_xop.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "utils_xop.h"
#include "globaldata.h"
#include "waveclass.h"
#include "utils_generic.h"

/*
	Write stringVector to the textwave waveHandle, using memcpy this is quite fast
*/
int stringVectorToTextWave(const std::vector<std::string>& stringVector, waveHndl waveHandle){

	ASSERT_RETURN_ONE(stringVector.size());

	std::vector<long int> stringSizes;

	int ret;
	unsigned int i;

	long int offset;
	long int totalSize=0;

	// number of 32-bit integers (aka long int) is one more compared to the number of strings
	const unsigned long int numEntriesPlusOne = stringVector.size()+1;

	std::vector<std::string>::const_iterator it;
	for(it = stringVector.begin(); it != stringVector.end(); it++){
		try{
			stringSizes.push_back(it->size());
		}catch(...){
			return NOMEM;
		}
		totalSize += it->size();
	}

	totalSize += numEntriesPlusOne*sizeof(long);

	Handle textHandle = NewHandle(totalSize);
	if(MemError() || textHandle == NULL){
		return NOMEM;
	}

	//DEBUGPRINT("totalSize of strings %d",GetHandleSize(textHandle));
	//
	for(i=0; i < numEntriesPlusOne; i++){

		if(i == 0){// position of the first string
			offset = numEntriesPlusOne*sizeof(long);
		}
		else{ // and of all the others
			offset+=stringSizes[i-1];
		}
		//DEBUGPRINT("offset=%d, offsetPosition=%d*sizeof(long)",offset,i);
		//
		// write offsets
		memcpy(*textHandle+i*sizeof(long),&offset,sizeof(long));

		if(i < stringVector.size()){

			//DEBUGPRINT("string=%s, stringPosition=%d",stringVector[i].c_str(),offset);
			//
			// write strings
			memcpy(*textHandle+offset,stringVector[i].c_str(),stringSizes[i]);
		}
	}

	// mode = 2 defines the format of the handle contents to
	// offsetToFirstString
	// offsetToSecondString
	// ...
	// offsetToPositionAfterLastString
	// firstString
	//...
	int mode=2;

	ret = SetTextWaveData(waveHandle,mode,textHandle);
	
	//DEBUGPRINT("SetTextWaveData returned %d",ret);
	//	DisposeHandle(textHandle);

	return ret;
}

/*
	Write the string waveNote as wave note into waveHandle
*/
void setWaveNoteAsString(const std::string& waveNote, waveHndl waveHandle){

	int ret;
	Handle noteHandle;

	if(waveNote.empty()){
		HISTPRINT("BUG: got empty waveNote in setWaveNoteAsString.");
		return;
	}

	noteHandle = NewHandle(waveNote.size()) ;
	if(noteHandle == NULL){
		return;
	}

	ret = PutCStringInHandle(waveNote.c_str(),noteHandle);
	if(ret != 0){
		HISTPRINT("internal error %d, aborting",ret);
		return;
	}

	ASSERT_RETURN_VOID(waveHandle);
	SetWaveNote(waveHandle, noteHandle);
}

/*
	Sets the complete wave data to which data points to NaN (double precision version)
*/
void waveClearNaN64( double* data, CountInt length )
{
	ASSERT_RETURN_VOID(data);

	for (long i = 0; i < length; i++){
		*data++ = DOUBLE_NAN;
	}
}

/*
	Sets the complete wave data to which data points to NaN (single precision version)
*/
void waveClearNaN32( float* data, CountInt length )
{
	ASSERT_RETURN_VOID(data);
	
	for (long i = 0; i < length; i++){
		*data++ = SINGLE_NAN;
	}
}

/*
	Append the full path of wv to waveList
*/
void appendToWaveList(const DataFolderHandle& df, const waveHndl& wv, std::string &waveList){
	waveList.append(getFullWavePath(df,wv));
	waveList.append(listSepChar);
}

/*
	Append the full path of wave.getWaveHandle() to waveList
*/
void appendToWaveList(const DataFolderHandle& df, const WaveClass& wave, std::string &waveList){
	appendToWaveList(df,wave.getWaveHandle(), waveList);
}

/*
	return the full path of the wave
*/
std::string getFullWavePath(const DataFolderHandle& df, const waveHndl& wv){

	char waveName[MAX_OBJ_NAME+1];
	char dataFolderPath[MAXCMDLEN+1];
	std::string fullPath;
	int ret;

	// flags=3 returns the full path to the datafolder and including quotes if needed
	ret = GetDataFolderNameOrPath(df, 3,dataFolderPath);
	if(ret != 0){
		HISTPRINT("BUG: Could not query the datafolder for its name");
		return fullPath;
	}
	
	WaveName(wv,waveName);
	fullPath.append(dataFolderPath);
	fullPath.append(waveName);
	return fullPath;
}

/*
	Convert a XOP string handle to a std::string
*/
void convertHandleToString(Handle strHandle,std::string &str){

	str.clear();
	int handleSize;

	// for both cases we return an empty string
	if(strHandle == NULL || GetHandleSize(strHandle) == 0L){
		return;
	}
	handleSize = GetHandleSize(strHandle);

	try{
		str = std::string(*strHandle,handleSize);
	}
	catch(CMemoryException* e){
		e->Delete();
		HISTPRINT("Out of memory in convertHandleToString()");
	}
}
