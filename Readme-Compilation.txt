This is a step-by-step manual to compile the MatrixFileReader XOP.

Requirements:
- Visual Studio 2005 Professional, the Express Version does not work. It is not advised to try a different version. If you have to switch, you need a different Vernissage SDK also)
- Windows XP SP2/Windows Vista/Windows 7
- Vernissage 2.0 and the corresponding SDK
- XOP Toolkit 6
- Igor 6.2.1 or later

Various Hints:
- Always compile the "Release" version. The "Debug" version can not be used with the Vernissage DLLs.
- Include and library paths are set in header.h
- If you want to add new operations checkout Operation-Template-Generator.pxp which uses ParseOperationTemplate

Compilation:
- Close all Igor instances
- Open MatrixFileReader.sln
- Adjust the path where the XOP should be copied by editing copyfiles.bat
- Build it (Build->Rebuild Solution)
- After the build the XOP and the help file is automatically copied to the location given earlier

Description of the source files:

constants.h: Constants for all files, some run time variables are also declared in operationsinterface.h
header.h: main header file which is included by all source files
version.h: version string of the XOP, this is returned by the GetMtrxFileReaderVersion operation and also used for the resource file
brickletclass.*: Internal representation of a bricklet, holds copies of the metadata and the rawdata
dllhandler.*: Everything related to loading the Vernissage DLLs. Calling closeSession() before exiting the application is mandatory.
brickletconverter.*: The heart of the XOP. Here the 1D arrays are converted to multiple igor waves having the data in the correct form. Holds the implementations for 1D, 2D and 3D. Check carefully before doing any changes
error-codes.h: The error codes which are used for the operations
globaldata.*: Global and unique object which holds various data
operationsinterface.*: Interface to Igor; all operations are declared and implemented here
operationstructs.h: Operation structs are defined here (they are automatically generated using the ParseOperationTemplate Igor function, see Operation-Template-Generator.pxp)
operationsregister.*: All RegisterOperation functions are implemented here
preferences.*: Saving and Loading of XOP Preferences is done here
resource.h: Some XOP internal stuff
uitls.*: Various utility functions
matrixFileReaderWinCustom.rc: The name of the operations implemented by the XOP must be defined here
