# Microsoft Developer Studio Project File - Name="GeocoderD" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GeocoderD - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GeocoderD.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GeocoderD.mak" CFG="GeocoderD - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GeocoderD - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GeocoderD - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GeocoderD - Win32 ReleaseDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GeocoderD - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDynamic"
# PROP Intermediate_Dir "ReleaseDynamic"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\geocommon" /I "..\..\global" /I "..\..\libraries\xerces\src" /I "..\..\libraries\ACE_Wrappers" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "GEOCODER_EXPORTS" /D "GLOBAL_STATIC" /Yu"Geocoder_Headers.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 xerces-c_2.lib ace.lib /nologo /dll /machine:I386 /libpath:"..\..\libraries\xerces\Build\Win32\VC6\Release" /libpath:"..\..\libraries\ACE_Wrappers\ace"
# Begin Special Build Tool
OutDir=.\ReleaseDynamic
SOURCE="$(InputPath)"
PostBuild_Desc=Copying Files
PostBuild_Cmds=copy $(OutDir)\GeocoderD.dll \test\program
# End Special Build Tool

!ELSEIF  "$(CFG)" == "GeocoderD - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugDynamic"
# PROP Intermediate_Dir "DebugDynamic"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\geocommon" /I "..\..\global" /I "..\..\libraries\xerces\src" /I "..\..\libraries\ACE_Wrappers" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "GEOCODER_EXPORTS" /D "GLOBAL_STATIC" /Yu"Geocoder_Headers.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 xerces-c_2D.lib aced.lib /nologo /dll /debug /machine:I386 /out:"DebugDynamic/GeocoderD_d.dll" /pdbtype:sept /libpath:"..\..\libraries\xerces\Build\Win32\VC6\Debug" /libpath:"..\..\libraries\ACE_Wrappers\ace"
# Begin Special Build Tool
OutDir=.\DebugDynamic
SOURCE="$(InputPath)"
PostBuild_Desc=Copying Files
PostBuild_Cmds=copy $(OutDir)\GeocoderD_d.dll \test\program	copy $(OutDir)\GeocoderD_d.pdb \test\program
# End Special Build Tool

!ELSEIF  "$(CFG)" == "GeocoderD - Win32 ReleaseDebug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GeocoderD___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "GeocoderD___Win32_ReleaseDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebugDynamic"
# PROP Intermediate_Dir "ReleaseDebugDynamic"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\geocommon" /I "..\..\global" /I "..\..\libraries\xerces\src" /I "..\..\libraries\ACE_Wrappers" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "GEOCODER_EXPORTS" /D "GLOBAL_STATIC" /Yu"Geocoder_Headers.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "..\geocommon" /I "..\..\global" /I "..\..\libraries\xerces\src" /I "..\..\libraries\ACE_Wrappers" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "GEOCODER_EXPORTS" /D "GLOBAL_STATIC" /Yu"Geocoder_Headers.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xerces-c_2.lib ace.lib /nologo /dll /machine:I386 /libpath:"..\..\libraries\xerces\Build\Win32\VC6\Release" /libpath:"..\..\libraries\ACE_Wrappers\ace"
# ADD LINK32 xerces-c_2.lib ace.lib /nologo /dll /pdb:none /debug /debugtype:both /machine:I386 /libpath:"..\..\libraries\xerces\Build\Win32\VC6\Release" /libpath:"..\..\libraries\ACE_Wrappers\ace"
# Begin Special Build Tool
OutDir=.\ReleaseDebugDynamic
SOURCE="$(InputPath)"
PostBuild_Desc=Copying Files
PostBuild_Cmds=copy $(OutDir)\GeocoderD.dll \test\program
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "GeocoderD - Win32 Release"
# Name "GeocoderD - Win32 Debug"
# Name "GeocoderD - Win32 ReleaseDebug"
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
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
