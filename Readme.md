## Installation

### Requirements
* Igor Pro 8.0 or later
* Scienta Omicron Vernissage 2.4.1 (32-bit or 64-bit)
* Windows 10

### Steps
1. Install Igor Pro
2. Start Igor, this will create a folder called WaveMetrics in "My Documents"
3. Install Vernissage
4. Install vcredist_x86.exe/vcredist_x64.exe (Microsoft Visual C++ 2013 Redistributable Package x86/x64) shipped with the XOP.
5.
  1. Copy "matrixfilereader.xop" to "My Documents\WaveMetrics\Igor Pro X User Files\Igor extensions"
  2. Copy "matrixfilereader Help.ihf" to "My Documents\WaveMetrics\Igor Pro X User Files\Igor Help files"
6. Restart Igor
7. Load the experiment matrixfilereader-basic-gui.pxp which includes a basic GUI for easy access to the XOP operations.

## Compilation

### Requirements
* Visual Studio 2013 Community/Professional with [1]
* Vernissage and the corresponding SDK
* XOP Toolkit 8.01
* Boost 1.55
* Igor Pro

### Steps
* Close all Igor instances
* Open MatrixFileReader.sln
* Adjust the path where the XOP should be copied by editing copyfiles-32.bat/copyfiles-64.bat
* Build it (`Build->Rebuild Solution`)
* The XOP and the help file is automatically copied to the location given earlier

### Various Hints
* Always compile the "Release" version. The "Debug" version can not be used with the Vernissage DLLs.
* If you want to add new operations have a look at
  Operation-Template-Generator.pxp which uses `ParseOperationTemplate`.
* Each header file contains a small abstract about its purpose.
* Boost can be compiled from a Visual Studio command prompt:
  * Apply the patch from [2]
  * Compile 32-bit libraries
    * `b2 -j6 toolset=msvc-12.0 variant=release threading=multi link=static runtime-link=shared stage`
    * `move stage stage-32bit`
  * Compile 64-bit libraries
    * `b2 -j6 toolset=msvc-12.0 address-model=64 variant=release threading=multi link=static runtime-link=shared stage`
    * `move stage stage-64bit`
* XOPSupport compilation itself can be done via `cmake -G "Visual Studio 12
  2013`/`cmake -G "Visual Studio 12 2013" -A x64` and `cmake --build . --config
  release`.

### Testing

* The unit and regression test suite requires our [unit testing framework](https://github.com/byte-physics/igor-unit-testing-framework).
* The test suite always finishes without errors.
* The reference data is available upon request only as it is roughly 5GB.

### Links

[1]: https://devblogs.microsoft.com/cppblog/mfc-support-for-mbcs-deprecated-in-visual-studio-2013/
[2]: https://svn.boost.org/trac10/attachment/ticket/8757/0005-Boost.S11n-include-missing-algorithm.patch
