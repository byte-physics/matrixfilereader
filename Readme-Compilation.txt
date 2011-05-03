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

Each header file contains a small abstract about is purpose.

