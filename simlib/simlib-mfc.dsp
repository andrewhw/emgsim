# Microsoft Developer Studio Project File - Name="simlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=simlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "simlib-mfc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "simlib-mfc.mak" CFG="simlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "simlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "simlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "simlib - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../common/include" /I "../DQEmgData/include" /I "../emg-tools/include" /I "../r-tree/include" /I "../glpk/include" /I "./include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "OS_STATIC" /D "TCL_MEM_DEBUG" /D "DQD_DEBUG_DUMP" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\simulator-mfc.lib"

!ELSEIF  "$(CFG)" == "simlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "simlib___Win32_Debug"
# PROP BASE Intermediate_Dir "simlib___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "lib"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../common/include" /I "../DQEmgData/include" /I "../emg-tools/include" /I "../r-tree/include" /I "../glpk/include" /I "include" /D "_LIB" /D "TCL_MEM_DEBUG" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "OS_STATIC" /D "DQD_DEBUG_DUMP" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\simulator-mfcd.lib"

!ENDIF 

# Begin Target

# Name "simlib - Win32 Release"
# Name "simlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\3Circle.cpp
# End Source File
# Begin Source File

SOURCE=.\src\emgutil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fileutil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\firing.cpp
# End Source File
# Begin Source File

SOURCE=.\src\globalHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\src\globals.cpp
# End Source File
# Begin Source File

SOURCE=.\src\JitterDB.cpp
# End Source File
# Begin Source File

SOURCE=.\src\logwrite.cpp
# End Source File
# Begin Source File

SOURCE=.\src\make16bit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\makeMUP.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MUP.cpp
# End Source File
# Begin Source File

SOURCE=.\src\muscle.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MuscleData.cpp
# End Source File
# Begin Source File

SOURCE=.\src\muscleNeuropathy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\NeedleInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\noiseFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\NoiseGenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SimulationResult.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Simulator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SineWave.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SMUP.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sMUP_util.cpp
# End Source File
# Begin Source File

SOURCE=.\src\statistics.cpp
# End Source File
# Begin Source File

SOURCE=.\src\userinput.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\3Circle.h
# End Source File
# Begin Source File

SOURCE=.\include\globalHandler.h
# End Source File
# Begin Source File

SOURCE=.\include\JitterDB.h
# End Source File
# Begin Source File

SOURCE=.\include\logwrite.h
# End Source File
# Begin Source File

SOURCE=.\include\make16bit.h
# End Source File
# Begin Source File

SOURCE=.\include\MUP.h
# End Source File
# Begin Source File

SOURCE=.\include\MUP_utils.h
# End Source File
# Begin Source File

SOURCE=.\include\muscle.h
# End Source File
# Begin Source File

SOURCE=.\include\MuscleData.h
# End Source File
# Begin Source File

SOURCE=.\include\NeedleInfo.h
# End Source File
# Begin Source File

SOURCE=.\include\noise.h
# End Source File
# Begin Source File

SOURCE=.\include\NoiseGenerator.h
# End Source File
# Begin Source File

SOURCE=.\include\Simulator.h
# End Source File
# Begin Source File

SOURCE=.\include\SimulatorControl.h
# End Source File
# Begin Source File

SOURCE=.\include\SineWave.h
# End Source File
# Begin Source File

SOURCE=.\include\SMUP.h
# End Source File
# Begin Source File

SOURCE=.\include\sMUP_util.h
# End Source File
# Begin Source File

SOURCE=.\include\statistics.h
# End Source File
# Begin Source File

SOURCE=.\include\SurfaceEmg.h
# End Source File
# Begin Source File

SOURCE=.\include\userinput.h
# End Source File
# End Group
# End Target
# End Project
