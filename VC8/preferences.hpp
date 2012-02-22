/*
	The file preferences.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/
#pragma once

/*
	Saving and Loading of XOP Preferences is done here
*/

#pragma pack(2)

struct XOPprefStruct {
	short version;						/* structure version number */
	char openDlgInitialDir[MAX_PATH_LEN+1];
	int openDlgFileIndex;
};

typedef struct XOPprefStruct XOPprefStruct;
typedef struct XOPprefStruct *XOPprefStructPtr;
typedef struct XOPprefStruct **XOPprefStructHandle;

#define XOPprefStruct_VERSION 1

#pragma pack()		// Reset structure alignment to default.

void loadXOPPreferences();
void saveXOPPreferences();
