This is a step-by-step manual to compile the MatrixFileReader XOP.

Requirements:
- Visual Studio 2005 (it is not advised to try a different version. If you have to switch you need a different Vernissage SDK/DLLs)
- Windows XP SP1 or later
- Vernissage 2.0 SDK (at least the T10 preview version, included)
- XOP Toolkit 6 (included)
- Igor 6.20 or later

Various Hints:
- Always compile the "Release" version. The "Debug" version can not be used with the Vernissage DLLs.
- Include and library paths are set in header.h
- If you want to add new operations checkout Operation-Template-Generator.pxp

Compilation:
- Close all Igor instances
- Open MatrixFileReader.sln
- Adjust the path where the XOP should be copied after the build (Project->Properties, Build-Events, Post-Build Event)
- Build it (Build->Rebuild Solution)
- After the build the XOP is automatically copied to the location given earlier

Description of the source files:

constants.h: Constants for all files, some runTime variables are also declared in operationsinterface.h
header.h: main header file for all source files
version.h: version string of the XOP, this is returned by the GetMtrxFileReaderVersion operation
brickletClass.*: Internal representation of a bricklet, holds copies of the metadata and the rawdata
dllhandler.*: Everything related to loading the Vernissage DLLs. Calling closeSession() before exiting the application is mandatory.
brickletconverter.*: The heart of the XOP. Here the 1D arrays are converted to multiple igor waves having the data in the correct form. Holds the implementations for 1D, 2D and 3D
error-codes.h: The error codes which are used for the operations
globaldata.*: Global and unique object which holds various data
operationsinterface.*: Interface to Igor; all operations are declared and implemented here
operationstructs.h: Operation structs are defined here (they are automatically generated using the ParseOperationTemplate Igor function, see Operation-Template-Generator.pxp)
resource.h: Some XOP internal stuff
uitls.*: Various utility functions
matrixFileReaderWinCustom.rc: The name of the Operations implemented by the XOP must be also defined here
