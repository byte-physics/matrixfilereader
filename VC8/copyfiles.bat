REM
REM  The file copyfiles.bat is part of the "MatrixFileReader XOP".
REM  It is licensed under the LGPLv3 with additional permissions,
REM   see License.txt in the source folder for details.
REM

copy Release\matrixfilereader.xop   "c:\Dokumente und Einstellungen\thomas\Eigene Dateien\WaveMetrics\Igor Pro 8 User Files\Igor Extensions"
if errorlevel 1 goto BuildEventFailed

copy "..\matrixfilereader Help.ihf" "c:\Dokumente und Einstellungen\thomas\Eigene Dateien\WaveMetrics\Igor Pro 8 User Files\Igor Help Files"
if errorlevel 1 goto BuildEventFailed

"C:\Program Files\WaveMetrics\Igor Pro 8 Folder\IgorBinaries_Win32\Igor.exe" E:\projekte\matrixfilereader\matrixfilereader-basic-gui.pxp
if errorlevel 1 goto BuildEventFailed

goto BuildEventOK

:BuildEventFailed
exit 1

:BuildEventOK
