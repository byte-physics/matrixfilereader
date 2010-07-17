--- C:/Documents and Settings/brt127/My Documents/gitRepo/Archiv-so-erhalten/VernissageSDK_T2.0-3/src/Incl/Vernissage.h	Wed Jun 23 16:13:02 2010
+++ C:/Documents and Settings/brt127/My Documents/gitRepo/Archiv-so-erhalten/VernissageSDK_V1.2/src/Incl/Vernissage.h	Mon Nov 09 16:00:44 2009
@@ -2,10 +2,10 @@
  ** $RCSfile$ $Revision$ $Date$
  **
  ** This file has been generated from file 'Vernissage.h'
- ** of module 'Foundation vYYWWNN'
+ ** of module 'Foundation v094301b091101'
  **
- ** Created: Wed Jun 23 14:13:01 2010 [UTC]
- **      by: ModuleInclGenerator.py 1.5
+ ** Created: Thu Oct 22 09:03:01 2009 [UTC]
+ **      by: ModuleInclGenerator.py 1.4
  **
  ** WARNING! All changes made to this file will be lost
  **          when recompiling module 'Foundation' !
@@ -17,10 +17,10 @@
  * Declaration of callable Vernissage entities; these entities are required
  * to extend or access the core functionality of the Vernissage subsystems.
  *
- * Copyright (c) 2008, 2009, 2010 Omicron NanoTechnology GmbH. All rights reserved.
+ * Copyright (c) 2008 Omicron NanoTechnology GmbH. All rights reserved.
  * 
- * Vernissage V2.0
- * $'Id: Vernissage.h,v 1.16 2010/06/08 17:56:56 '$
+ * Vernissage V1.1
+ * $'Id: Vernissage.h,v 1.9 2009/03/13 09:43:28 '$
  */
 
 #ifndef __VERNISSAGE_H__
@@ -260,9 +260,6 @@
       // Return information on all plug-ins loaded
       virtual std::vector<Session::PlugInInfo> getPlugInInfo () const;
 
-      // Return the path to the directory storing the plug-ins 
-      virtual std::wstring getPlugInPath () const;
-
       /**
        * Message management
        */
@@ -298,24 +295,7 @@
       // Return next Bricklet in the database. 'pContext' is a pointer to an
       // opaque context variable that must initially be assigned a null
       // pointer. (as in 'void *pContext = NULL;')
-      // 'pFilterSet' is an optional pointer to a Bricklet filter set.
-      virtual void * getNextBricklet (void **pContext, void *pFilterSet = 0);
-
-      // 
-      typedef enum
-      {
-        btc_Unknown,              // not known
-        btc_SPMSpectroscopy,      // curve (1D)
-        btc_AtomManipulation,     // curve (1D)
-        btc_1DCurve,              // arbitrary curve (1D)
-        btc_SPMImage,             // image (2D)
-        btc_PathSpectroscopy,     // set of curves, acquired along some path
-        btc_ESpRegion,            // sets of counts (2D)
-        btc_DetectorSnapshot,      
-        btc_VolumeCITS,           // grid spectroscopy (3D)
-        btc_DiscreteEnergyMap     // sets of counts (3D)
-      } BrickletTypeCode;
-      virtual BrickletTypeCode getType (void *pBricklet);
+      virtual void * getNextBricklet (void **pContext);
 
       // Terminate a Bricklet set iteration sequence (started by calling
       // 'getNextBricklet').
@@ -483,8 +463,7 @@
         vtc_2Dof3D,
         vtc_Spectroscopy,
         vtc_ForceCurve,
-        vtc_1DProfile,
-        vtc_Interferometer
+        vtc_1DProfile
       } ViewTypeCode;
       virtual std::vector<ViewTypeCode> getViewTypes (void *pBricklet);


	   // TILL HERE all needed changes are done
 
@@ -496,18 +475,6 @@
       // experiment execution.
       virtual std::wstring getCreationComment (void *pBricklet);
 
-      // Return the Bricklet image comment set specified by the user during
-      // experiment execution.
-      virtual std::vector<std::wstring> getDataComments (void *pBricklet);
-
-      // Return the sample name that the user has specified during experiment
-      // execution.
-      virtual std::wstring getSampleName (void *pBricklet);
-
-      // Return the data set name that the user has specified during experiment
-      // execution.
-      virtual std::wstring getDataSetName (void *pBricklet);
-
       // Return the names of all Experiment Element instances that match the
       // specified type.
       virtual std::vector<std::wstring> getExperimentElementInstanceNames (
@@ -569,11 +536,8 @@
       // 'PlugInInfo' struct).
       // 'outputDirectoryPath' is a full directory path specification to
       // the directory to be used for storing the conversion results.
-      // 'pFilterSet' points to a Bricklet filter set that must be obeyed
-      // when exporting data.
      virtual bool initiateExport (std::wstring plugIn,                        
-                                  std::wstring outputDirectoryPath,
-                                  void *pFilterSet);
+                                  std::wstring outputDirectoryPath);
 
       // Cancel a progressing export operation. 
       // 'plugIn' must be the name of the plug-in to be used for conversion
@@ -663,8 +627,6 @@
                                                           std::wstring instanceName);        
       int getBrickletDataItemCapacity (void *pBricklet);
       __time64_t getUTCCreationTimestamp (void *pBricklet);
-      void getChunkMarkerMultiPointSupplementaryData (void* pBricklet,
-                                                      std::wstring& txt) const;
 
     protected:
 
