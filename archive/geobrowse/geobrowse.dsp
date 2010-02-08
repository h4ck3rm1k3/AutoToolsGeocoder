# Microsoft Developer Studio Project File - Name="geobrowse" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=geobrowse - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "geobrowse.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "geobrowse.mak" CFG="geobrowse - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "geobrowse - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "geobrowse - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "geobrowse - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\geocommon" /I "..\geocoder" /I "..\..\global" /I "..\..\libraries\xerces\src" /I "..\..\libraries\STINGRAY\COMMON\COMMON 6.0\INCLUDE" /I "..\..\libraries\STINGRAY\OG70\INCLUDE" /I "..\..\libraries\STINGRAY\OT60\INCLUDE" /I "..\..\libraries\STINGRAY\OV60\INCLUDE" /I "..\..\libraries\STINGRAY\COMMON\MVC 6.0\INCLUDE" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_GXEXT" /D "_AFXEXT" /D "GEOCODER_STATIC" /D "COMPILE_GEOBROWSE" /D "_SECDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 xerces-c_2.lib /nologo /subsystem:windows /profile /machine:I386 /libpath:"..\..\libraries\xerces\Build\Win32\VC6\Release" /libpath:"..\..\libraries\STINGRAY\COMMON\COMMON 6.0\LIB" /libpath:"..\..\libraries\STINGRAY\COMMON\MVC 6.0\LIB" /libpath:"..\..\libraries\STINGRAY\OG70\LIB" /libpath:"..\..\libraries\STINGRAY\OT60\LIB" /libpath:"..\..\libraries\STINGRAY\OV60\LIB"

!ELSEIF  "$(CFG)" == "geobrowse - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\geocommon" /I "..\geocoder" /I "..\..\global" /I "..\..\libraries\xerces\src" /I "..\..\libraries\STINGRAY\COMMON\COMMON 6.0\INCLUDE" /I "..\..\libraries\STINGRAY\OG70\INCLUDE" /I "..\..\libraries\STINGRAY\OT60\INCLUDE" /I "..\..\libraries\STINGRAY\OV60\INCLUDE" /I "..\..\libraries\STINGRAY\COMMON\MVC 6.0\INCLUDE" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_GXEXT" /D "_AFXEXT" /D "GEOCODER_STATIC" /D "COMPILE_GEOBROWSE" /D "_SECDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 xerces-c_2D.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\libraries\xerces\Build\Win32\VC6\Debug" /libpath:"..\..\libraries\STINGRAY\COMMON\COMMON 6.0\LIB" /libpath:"..\..\libraries\STINGRAY\COMMON\MVC 6.0\LIB" /libpath:"..\..\libraries\STINGRAY\OG70\LIB" /libpath:"..\..\libraries\STINGRAY\OT60\LIB" /libpath:"..\..\libraries\STINGRAY\OV60\LIB"
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "geobrowse - Win32 Release"
# Name "geobrowse - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ChildView.cpp
# End Source File
# Begin Source File

SOURCE=.\CityStatePostcodeBrowseGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeAddressBrowse.cpp
# End Source File
# Begin Source File

SOURCE=.\CoordinateBrowseGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\geobrowse.cpp
# End Source File
# Begin Source File

SOURCE=.\geobrowse.rc
# End Source File
# Begin Source File

SOURCE=.\HierarchicalBrowse.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ParseAddressBrowse.cpp
# End Source File
# Begin Source File

SOURCE=.\PostcodeAliasBrowseGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\PostcodeCityBrowse.cpp
# End Source File
# Begin Source File

SOURCE=.\PostcodeStreetBrowse.cpp
# End Source File
# Begin Source File

SOURCE=.\StateCityBrowse.cpp
# End Source File
# Begin Source File

SOURCE=.\StateCitySoundexBrowse.cpp
# End Source File
# Begin Source File

SOURCE=.\StateCitySoundexBrowseGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StreetIntersectionBrowse.cpp
# End Source File
# Begin Source File

SOURCE=.\StreetIntersectionSoundexBrowse.cpp
# End Source File
# Begin Source File

SOURCE=.\StreetIntersectionSoundexBrowseGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\StreetNameBrowseGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\StreetNameSoundexBrowseGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\StreetSegmentBrowseGrid.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ChildView.h
# End Source File
# Begin Source File

SOURCE=.\CityStatePostcodeBrowseGrid.h
# End Source File
# Begin Source File

SOURCE=.\CodeAddressBrowse.h
# End Source File
# Begin Source File

SOURCE=.\CoordinateBrowseGrid.h
# End Source File
# Begin Source File

SOURCE=.\geobrowse.h
# End Source File
# Begin Source File

SOURCE=.\HierarchicalBrowse.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\ParseAddressBrowse.h
# End Source File
# Begin Source File

SOURCE=.\PostcodeAliasBrowseGrid.h
# End Source File
# Begin Source File

SOURCE=.\PostcodeCityBrowse.h
# End Source File
# Begin Source File

SOURCE=.\PostcodeStreetBrowse.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StateCityBrowse.h
# End Source File
# Begin Source File

SOURCE=.\StateCitySoundexBrowse.h
# End Source File
# Begin Source File

SOURCE=.\StateCitySoundexBrowseGrid.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StreetIntersectionBrowse.h
# End Source File
# Begin Source File

SOURCE=.\StreetIntersectionSoundexBrowse.h
# End Source File
# Begin Source File

SOURCE=.\StreetIntersectionSoundexBrowseGrid.h
# End Source File
# Begin Source File

SOURCE=.\StreetNameBrowseGrid.h
# End Source File
# Begin Source File

SOURCE=.\StreetNameSoundexBrowseGrid.h
# End Source File
# Begin Source File

SOURCE=.\StreetSegmentBrowseGrid.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\geobrowse.ico
# End Source File
# Begin Source File

SOURCE=.\res\geobrowse.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
