# Microsoft Developer Studio Project File - Name="c++toollib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=c++toollib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "c++toollib-nomfc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "c++toollib-nomfc.mak" CFG="c++toollib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "c++toollib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "c++toollib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "c++toollib - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "../common/include" /I "./include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "OS_STATIC" /D "TCL_MEM_DEBUG" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\cpptool-nomfc.lib"

!ELSEIF  "$(CFG)" == "c++toollib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "lib"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../common/include" /I "./include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "OS_STATIC" /D "TCL_MEM_DEBUG" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\cpptool-nomfcd.lib"

!ENDIF 

# Begin Target

# Name "c++toollib - Win32 Release"
# Name "c++toollib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\tlBin.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlBinTable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlColumn.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlDataChooser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlErrorManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlExpression.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlHashTable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlMatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlRef.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlRefManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlSParser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlSrMatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlSrString.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlSrValue.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlSTable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlStringAllocationTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlTable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlTrie.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlTuple.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlUnicodeConvert.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tlVector.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\tlBin.h
# End Source File
# Begin Source File

SOURCE=.\include\tlBinTable.h
# End Source File
# Begin Source File

SOURCE=.\include\tlColumn.h
# End Source File
# Begin Source File

SOURCE=.\include\tlDataChooser.h
# End Source File
# Begin Source File

SOURCE=.\include\tlDocs.h
# End Source File
# Begin Source File

SOURCE=.\include\tlErrorManager.h
# End Source File
# Begin Source File

SOURCE=.\include\tlExpression.h
# End Source File
# Begin Source File

SOURCE=.\include\tlGslMatrix.h
# End Source File
# Begin Source File

SOURCE=.\include\tlHashTable.h
# End Source File
# Begin Source File

SOURCE=.\include\tlMatrix.h
# End Source File
# Begin Source File

SOURCE=.\include\tlRef.h
# End Source File
# Begin Source File

SOURCE=.\include\tlRefManager.h
# End Source File
# Begin Source File

SOURCE=.\include\tlSParser.h
# End Source File
# Begin Source File

SOURCE=.\include\tlSrMatrix.h
# End Source File
# Begin Source File

SOURCE=.\include\tlSrString.h
# End Source File
# Begin Source File

SOURCE=.\include\tlSrValue.h
# End Source File
# Begin Source File

SOURCE=.\include\tlSTable.h
# End Source File
# Begin Source File

SOURCE=.\include\tlStringAllocationTool.h
# End Source File
# Begin Source File

SOURCE=.\include\tlTable.h
# End Source File
# Begin Source File

SOURCE=.\include\tlTDoubleLinkedList.h
# End Source File
# Begin Source File

SOURCE=.\include\tlTRefList.h
# End Source File
# Begin Source File

SOURCE=.\include\tlTrie.h
# End Source File
# Begin Source File

SOURCE=.\include\tlTuple.h
# End Source File
# Begin Source File

SOURCE=.\include\tlTVector.h
# End Source File
# Begin Source File

SOURCE=.\include\tlUnicodeConvert.h
# End Source File
# Begin Source File

SOURCE=.\include\tlVector.h
# End Source File
# End Group
# End Target
# End Project
