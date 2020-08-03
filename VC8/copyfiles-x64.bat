REM
REM  The file copyfiles.bat is part of the "MatrixFileReader XOP".
REM  It is licensed under the LGPLv3 with additional permissions,
REM   see License.txt in the source folder for details.
REM

copy Release-x64\matrixfilereader-64.xop   "c:\Dokumente und Einstellungen\thomas\Eigene Dateien\WaveMetrics\Igor Pro 8 User Files\Igor Extensions (64-bit)"
if errorlevel 1 goto BuildEventFailed

copy "..\matrixfilereader Help.ihf" "c:\Dokumente und Einstellungen\thomas\Eigene Dateien\WaveMetrics\Igor Pro 8 User Files\Igor Help Files"
if errorlevel 1 goto BuildEventFailed

"C:\Program Files\WaveMetrics\Igor Pro 8 Folder\IgorBinaries_x64\Igor64.exe" E:\projekte\matrixfilereader\matrixfilereader-basic-gui.pxp
if errorlevel 1 goto BuildEventFailed

goto BuildEventOK

:BuildEventFailed
exit 1

:BuildEventOK
