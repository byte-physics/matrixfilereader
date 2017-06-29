## Installation

### Requirements
* Igor Pro 6.2.0 or later (Igor Pro 7 works as well)
* Scienta Omicron Vernissage 2.2 or 2.3.x
* Windows XP SP2/Vista/7/10

### Steps
1. Install Igor Pro
2. Start Igor, this will create a folder called WaveMetrics in "My Documents"
3. Install Vernissage
4. Install vcredist_x86.exe (Microsoft Visual C++ 2005 Redistributable Package x86) shipped with the XOP.
5.
  1. Copy "matrixfilereader.xop" to "My Documents\WaveMetrics\Igor Pro X User Files\Igor extensions"
  2. Copy "matrixfilereader Help.ihf" to "My Documents\WaveMetrics\Igor Pro X User Files\Igor Help files"
6. Restart Igor
7. Load the experiment matrixfilereader-basic-gui.pxp which includes a basic GUI for easy access to the XOP operations.

## Compilation

### Requirements
* Visual Studio 2005 Professional, the Express Version does not work.
* Windows XP SP2/Vista/7/10
* Vernissage and the corresponding SDK
* XOP Toolkit 6.40
* Boost 1.55
* Igor Pro

### Steps
* Close all Igor instances
* Open MatrixFileReader.sln
* Adjust the path where the XOP should be copied by editing copyfiles.bat
* Build it (`Build->Rebuild Solution`)
* The XOP and the help file is automatically copied to the location given earlier

### Various Hints
* Always compile the "Release" version. The "Debug" version can not be used with the Vernissage DLLs.
* If you want to add new operations have a look at Operation-Template-Generator.pxp which uses `ParseOperationTemplate`.
* Each header file contains a small abstract about its purpose.
* Boost can be compiled from a Visual Studio command prompt with
  `b2 -j6 toolset=msvc-8.0 variant=release threading=multi link=static runtime-link=shared stage`

### Testing

* The unit and regression test suite requires the [unit testing framework](https://github.com/t-b/igor-unit-testing-framework).
* The test suite always finishes without errors.
* The reference data is available upon request only as it is roughly 5GB.
