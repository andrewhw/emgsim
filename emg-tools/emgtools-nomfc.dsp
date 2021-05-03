# Microsoft Developer Studio Project File - Name="emgtools" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=emgtools - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "emgtools-nomfc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "emgtools-nomfc.mak" CFG="emgtools - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "emgtools - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "emgtools - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "emgtools - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../common/include" /I "./include" /I "include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "OS_STATIC" /D "TCL_MEM_DEBUG" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\emgtools-nomfc.lib"

!ELSEIF  "$(CFG)" == "emgtools - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "emgtools___Win32_Debug"
# PROP BASE Intermediate_Dir "emgtools___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "lib"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../common/include" /I "include" /I "common/include" /I "c++/include" /I "emgtools/include" /I "Compatibility" /I "Decomposition_Classes" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "OS_STATIC" /D "TCL_MEM_DEBUG" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\emgtools-nomfcd.lib"

!ENDIF 

# Begin Target

# Name "emgtools - Win32 Release"
# Name "emgtools - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\buffertools.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dco_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\src\emg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\emgdat.cpp
# End Source File
# Begin Source File

SOURCE=.\src\prm.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\buffertools.h
# End Source File
# Begin Source File

SOURCE=.\include\dco.h
# End Source File
# Begin Source File

SOURCE=.\include\emg.h
# End Source File
# Begin Source File

SOURCE=.\include\emgdat.h
# End Source File
# Begin Source File

SOURCE=.\include\prm.h
# End Source File
# End Group
# End Target
# End Project
