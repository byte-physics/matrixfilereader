The data here serves as a reference for future MatrixFileReader XOP versions.
It was created by version 0.15 with a one fix (see below).
The MATRIX data was acquired with version 3.0 of MATRIX.

From 5b8ad167291ae88173099e06ce28efe8ec452256 Mon Sep 17 00:00:00 2001
From: Thomas Braun <thomas.braun@virtuell-zuhause.de>
Date: Fri, 29 Apr 2011 10:10:40 +0200
Subject: [PATCH] fix a off by one error, now the min/max data should always be correct

---
 .../igor-xop/VC8/brickletconverter.cpp             |    2 +-
 1 files changed, 1 insertions(+), 1 deletions(-)

diff --git a/Coding/matrix-file-reader/igor-xop/VC8/brickletconverter.cpp b/Coding/matrix-file-reader/igor-xop/VC8/brickletconverter.cpp
index 4775980..88d9156 100644
--- a/Coding/matrix-file-reader/igor-xop/VC8/brickletconverter.cpp
+++ b/Coding/matrix-file-reader/igor-xop/VC8/brickletconverter.cpp
@@ -168,7 +168,7 @@ int createWaves(DataFolderHandle dfHandle, const char *waveBaseNameChar, int bri
 		debugOutputToHistory(globDataPtr->outputBuffer);
 	}
 
-	ExtremaData extremaData[3];
+	ExtremaData extremaData[4];
 
 	ExtremaData *traceUpExtrema=NULL,*traceDownExtrema=NULL,*reTraceUpExtrema=NULL,*reTraceDownExtrema=NULL;
 
-- 
1.7.4.msysgit.0

