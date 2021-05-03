echo off
REM
REM DOS build file to build the common lib
REM
REM $Id: build.bat 620 2003-08-09 14:32:48Z andrew $
REM
echo msdev common-nomfc.dsw /MAKE "common - Win32 Debug" /REBUILD /OUT build.log
msdev common-nomfc.dsw /MAKE "common - Win32 Debug" /REBUILD /OUT build.log
type build.log
echo y | del build.log
echo on
