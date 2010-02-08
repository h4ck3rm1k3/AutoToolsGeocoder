# Microsoft Developer Studio Project File - Name="GeocoderS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GeocoderS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GeocoderS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GeocoderS.mak" CFG="GeocoderS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GeocoderS - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GeocoderS - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "GeocoderS - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GeocoderS - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseStatic"
# PROP Intermediate_Dir "ReleaseStatic"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\geocommon" /I "..\..\global" /I "..\..\libraries\xerces\src" /I "..\..\libraries\ACE_Wrappers" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_AFXEXT" /D "_AFXDLL" /D "GEOCODER_STATIC" /D "GLOBAL_STATIC" /Yu"Geocoder_Headers.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GeocoderS - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugStatic"
# PROP Intermediate_Dir "DebugStatic"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\geocommon" /I "..\..\global" /I "..\..\libraries\xerces\src" /I "..\..\libraries\ACE_Wrappers" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_AFXEXT" /D "_AFXDLL" /D "GEOCODER_STATIC" /D "GLOBAL_STATIC" /Yu"Geocoder_Headers.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"DebugStatic\GeocoderS_d.lib"

!ELSEIF  "$(CFG)" == "GeocoderS - Win32 ReleaseDebug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GeocoderS___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "GeocoderS___Win32_ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebugStatic"
# PROP Intermediate_Dir "ReleaseDebugStatic"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\geocommon" /I "..\..\global" /I "..\..\libraries\xerces\src" /I "..\..\libraries\ACE_Wrappers" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_AFXEXT" /D "_AFXDLL" /D "GEOCODER_STATIC" /D "GLOBAL_STATIC" /Yu"Geocoder_Headers.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "..\geocommon" /I "..\..\global" /I "..\..\libraries\xerces\src" /I "..\..\libraries\ACE_Wrappers" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_AFXEXT" /D "_AFXDLL" /D "GEOCODER_STATIC" /D "GLOBAL_STATIC" /Yu"Geocoder_Headers.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "GeocoderS - Win32 Release"
# Name "GeocoderS - Win32 Debug"
# Name "GeocoderS - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\GeoAddressTemplate.cpp
# End Source File
# Begin Source File

SOURCE=..\geocommon\GeoBitPtr.cpp
# End Source File
# Begin Source File

SOURCE=..\geocommon\GeoBitStream.cpp
# End Source File
# Begin Source File

SOURCE=.\Geocoder.cpp
# End Source File
# Begin Source File

SOURCE=.\Geocoder.rc
# End Source File
# Begin Source File

SOURCE=.\Geocoder_Headers.cpp
# ADD CPP /Yc"Geocoder_Headers.h"
# End Source File
# Begin Source File

SOURCE=.\GeocoderD.cpp
# End Source File
# Begin Source File

SOURCE=.\GeocoderImp.cpp
# End Source File
# Begin Source File

SOURCE=.\GeoQuery.cpp
# End Source File
# Begin Source File

SOURCE=.\GeoQueryImp.cpp
# End Source File
# Begin Source File

SOURCE=..\geocommon\GeoUtil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\geocommon\GeoAbstractByteIO.h
# End Source File
# Begin Source File

SOURCE=.\GeoAddressTemplate.h
# End Source File
# Begin Source File

SOURCE=..\geocommon\GeoBitPtr.h
# End Source File
# Begin Source File

SOURCE=..\geocommon\GeoBitStream.h
# End Source File
# Begin Source File

SOURCE=.\Geocoder.h
# End Source File
# Begin Source File

SOURCE=.\Geocoder_DllExport.h
# End Source File
# Begin Source File

SOURCE=..\geocommon\Geocoder_DllExport.h
# End Source File
# Begin Source File

SOURCE=.\Geocoder_Headers.h
# End Source File
# Begin Source File

SOURCE=.\GeocoderImp.h
# End Source File
# Begin Source File

SOURCE=..\geocommon\GeoDataInput.h
# End Source File
# Begin Source File

SOURCE=.\GeoDataVersion.h
# End Source File
# Begin Source File

SOURCE=..\geocommon\GeoFreqTable.h
# End Source File
# Begin Source File

SOURCE=..\geocommon\GeoHuffman.h
# End Source File
# Begin Source File

SOURCE=.\GeoQuery.h
# End Source File
# Begin Source File

SOURCE=.\GeoQueryImp.h
# End Source File
# Begin Source File

SOURCE=.\GeoQueryItf.h
# End Source File
# Begin Source File

SOURCE=.\GeoResultsInternal.h
# End Source File
# Begin Source File

SOURCE=..\geocommon\GeoUtil.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# End Target
# End Project
