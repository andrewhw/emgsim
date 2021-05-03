# Microsoft Developer Studio Project File - Name="common" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=common - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "common-nomfc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "common-nomfc.mak" CFG="common - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "common - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "common - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "common - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "OS_STATIC" /D "TCL_MEM_DEBUG" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\common-nomfc.lib"

!ELSEIF  "$(CFG)" == "common - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "common___Win32_Debug"
# PROP BASE Intermediate_Dir "common___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "lib"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "include" /I "common/include" /I "c++/include" /I "emgtools/include" /I "Compatibility" /I "Decomposition_Classes" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "OS_STATIC" /D "TCL_MEM_DEBUG" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\common-nomfcd.lib"

!ENDIF 

# Begin Target

# Name "common - Win32 Release"
# Name "common - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\math\acceleration.c
# End Source File
# Begin Source File

SOURCE=.\math\adjust.c
# End Source File
# Begin Source File

SOURCE=.\file\attvalue.c
# End Source File
# Begin Source File

SOURCE=.\math\calcPeakToPeak.c
# End Source File
# Begin Source File

SOURCE=.\math\chords.c
# End Source File
# Begin Source File

SOURCE=.\file\cleanfile.c
# End Source File
# Begin Source File

SOURCE=.\commandline\commandline.c
# End Source File
# Begin Source File

SOURCE=.\path\convertPath.c
# End Source File
# Begin Source File

SOURCE=.\file\copyfile.c
# End Source File
# Begin Source File

SOURCE=.\file\csvtools.c
# End Source File
# Begin Source File

SOURCE=.\dbg\debug.c
# End Source File
# Begin Source File

SOURCE=.\error\Derror.c
# End Source File
# Begin Source File

SOURCE=.\path\dirlist.c
# End Source File
# Begin Source File

SOURCE=.\math\factorial.c
# End Source File
# Begin Source File

SOURCE=.\math\fft.c
# End Source File
# Begin Source File

SOURCE=.\math\filtfilt.c
# End Source File
# Begin Source File

SOURCE=.\path\fopenpath.c
# End Source File
# Begin Source File

SOURCE=.\string\formatParagraph.c
# End Source File
# Begin Source File

SOURCE=.\math\functions.c
# End Source File
# Begin Source File

SOURCE=.\os\getOsVersion.c
# End Source File
# Begin Source File

SOURCE=.\gnuplot\histogram.c
# End Source File
# Begin Source File

SOURCE=.\intrretry\i_close.c
# End Source File
# Begin Source File

SOURCE=.\intrretry\i_open.c
# End Source File
# Begin Source File

SOURCE=.\intrretry\i_rdwr.c
# End Source File
# Begin Source File

SOURCE=.\intrretry\i_stat.c
# End Source File
# Begin Source File

SOURCE=.\io\io_utils.c
# End Source File
# Begin Source File

SOURCE=.\alloc\isort.c
# End Source File
# Begin Source File

SOURCE=.\time\julian.c
# End Source File
# Begin Source File

SOURCE=.\log\log.c
# End Source File
# Begin Source File

SOURCE=.\alloc\lsList.c
# End Source File
# Begin Source File

SOURCE=.\alloc\memlist.c
# End Source File
# Begin Source File

SOURCE=.\string\niceDouble.c
# End Source File
# Begin Source File

SOURCE=.\string\niceFilename.c
# End Source File
# Begin Source File

SOURCE=.\path\openpath.c
# End Source File
# Begin Source File

SOURCE=.\alloc\panic.c
# End Source File
# Begin Source File

SOURCE=.\path\pathtools.c
# End Source File
# Begin Source File

SOURCE=.\file\plotBuffer.c
# End Source File
# Begin Source File

SOURCE=.\math\random.c
# End Source File
# Begin Source File

SOURCE=.\gnuplot\simpleplots.c
# End Source File
# Begin Source File

SOURCE=.\string\slnprintf.c
# End Source File
# Begin Source File

SOURCE=.\interpolate\spline.c
# End Source File
# Begin Source File

SOURCE=.\string\strbasename.c
# End Source File
# Begin Source File

SOURCE=.\string\strcase.c
# End Source File
# Begin Source File

SOURCE=.\string\strconcat.c
# End Source File
# Begin Source File

SOURCE=.\string\strdup.c
# End Source File
# Begin Source File

SOURCE=.\error\strerror.c
# End Source File
# Begin Source File

SOURCE=.\string\strisblank.c
# End Source File
# Begin Source File

SOURCE=.\string\strndup.c
# End Source File
# Begin Source File

SOURCE=.\string\strqconcat.c
# End Source File
# Begin Source File

SOURCE=.\string\strsplit.c
# End Source File
# Begin Source File

SOURCE=.\string\strTimeDelta.c
# End Source File
# Begin Source File

SOURCE=.\string\strTimeString.c
# End Source File
# Begin Source File

SOURCE=.\string\strtruncblank.c
# End Source File
# Begin Source File

SOURCE=.\string\strunctrl.c
# End Source File
# Begin Source File

SOURCE=.\string\strUnique.c
# End Source File
# Begin Source File

SOURCE=.\alloc\tclCkalloc.c
# End Source File
# Begin Source File

SOURCE=.\file\tempfile.c
# End Source File
# Begin Source File

SOURCE=.\timing\timer.c
# End Source File
# Begin Source File

SOURCE=.\file\tokenizer.c
# End Source File
# Begin Source File

SOURCE=.\gnuplot\tools.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\attvalfile.h
# End Source File
# Begin Source File

SOURCE=.\include\bitstring.h
# End Source File
# Begin Source File

SOURCE=.\include\commandline.h
# End Source File
# Begin Source File

SOURCE=.\include\dir_defaults.h
# End Source File
# Begin Source File

SOURCE=.\include\error.h
# End Source File
# Begin Source File

SOURCE=.\include\filetools.h
# End Source File
# Begin Source File

SOURCE=.\include\filter.h
# End Source File
# Begin Source File

SOURCE=.\include\io_utils.h
# End Source File
# Begin Source File

SOURCE=.\include\julian.h
# End Source File
# Begin Source File

SOURCE=.\include\listalloc.h
# End Source File
# Begin Source File

SOURCE=.\include\localstrings.h
# End Source File
# Begin Source File

SOURCE=.\include\log.h
# End Source File
# Begin Source File

SOURCE=.\include\massert.h
# End Source File
# Begin Source File

SOURCE=.\include\mathtools.h
# End Source File
# Begin Source File

SOURCE=.\include\msgdbids.h
# End Source File
# Begin Source File

SOURCE=.\include\msgir.h
# End Source File
# Begin Source File

SOURCE=.\include\msgisort.h
# End Source File
# Begin Source File

SOURCE=.\include\msgstrerror.h
# End Source File
# Begin Source File

SOURCE=.\include\NRinterpolate.h
# End Source File
# Begin Source File

SOURCE=.\include\os_defs.h
# End Source File
# Begin Source File

SOURCE=.\include\os_names.h
# End Source File
# Begin Source File

SOURCE=.\include\os_types.h
# End Source File
# Begin Source File

SOURCE=.\include\pathtools.h
# End Source File
# Begin Source File

SOURCE=.\include\plottools.h
# End Source File
# Begin Source File

SOURCE=.\include\protodefns.h
# End Source File
# Begin Source File

SOURCE=.\include\random.h
# End Source File
# Begin Source File

SOURCE=.\include\reporttimer.h
# End Source File
# Begin Source File

SOURCE=.\include\stringtools.h
# End Source File
# Begin Source File

SOURCE=.\include\tclCkalloc.h
# End Source File
# Begin Source File

SOURCE=.\include\tokens.h
# End Source File
# End Group
# End Target
# End Project
