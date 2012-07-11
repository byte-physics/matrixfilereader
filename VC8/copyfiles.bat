REM
REM  The file copyfiles.bat is part of the "MatrixFileReader XOP".
REM  It is licensed under the LGPLv3 with additional permissions,
REM   see License.txt  in the source folder for details.
REM

copy Release\matrixfilereader.xop   "c:\Dokumente und Einstellungen\thomas\Eigene Dateien\WaveMetrics\Igor Pro 6 User Files\Igor Extensions"
if errorlevel 1 goto BuildEventFailed

copy "..\matrixfilereader Help.ihf" "c:\Dokumente und Einstellungen\thomas\Eigene Dateien\WaveMetrics\Igor Pro 6 User Files\Igor Help Files"
if errorlevel 1 goto BuildEventFailed

REM start H:\gitRepo\Coding\matrix-file-reader\igor-xop\matrixfilereader-basic-gui.pxp
REM if errorlevel 1 goto BuildEventFailed

goto BuildEventOK

:BuildEventFailed
exit 1

:BuildEventOK
