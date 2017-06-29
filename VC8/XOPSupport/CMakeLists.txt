CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

PROJECT(XOPSupport)

INCLUDE_DIRECTORIES(.)

SET(HEADERS
IgorErrors.h
IgorXOP.h
NamedFIFO.h
WMTypes.h
XOP.h
XOPContainers.h
XOPResources.h
XOPStandardHeaders.h
XOPSupport.h
XOPWinMacSupport.h
XOPWindows.h
)

SET(SOURCES
XOPContainers.c
XOPDataFolderAccess.c
XOPFiles.c
XOPMemory.c
XOPMenus.c
XOPNumericConversion.c
XOPSupport.c
XOPSupportWin.c
XOPTextUtilityWindows.c
XOPWaveAccess.c
XOPWindows.c)

IF(APPLE)
  SET(SOURCES ${SOURCES}
    XOPDialogsMac.c
    XOPSupportMac.c
    XOPFilesMac.c)

  SET(HEADERS ${HEADERS}
    XOPSupportMac.h)

ELSEIF(WIN32)
  SET(SOURCES ${SOURCES}
    XOPDialogsWin.c
    XOPSupportWin.c
    XOPFilesWin.c)

  SET(HEADERS ${HEADERS}
    XOPSupportWin.h)

ENDIF()

SOURCE_GROUP("Headers" FILES ${HEADERS})

ADD_LIBRARY(XOPSupport ${SOURCES} ${HEADERS})

ADD_DEFINITIONS(-D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D_CRT_SECURE_NO_DEPRECATE /MP)
TARGET_LINK_LIBRARIES(XOPSupport Igor.lib)
