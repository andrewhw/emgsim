echo off
REM
REM DOS build file to build the DQEMG Tool
REM
REM $Id: build-mfc.bat 2 2002-03-11 15:48:35Z andrew $
REM
echo msdev DQEmgData-mfc.dsw /MAKE "DQEmgData-mfc - Win32 Debug" /REBUILD /OUT build.log
msdev DQEmgData-mfc.dsw /MAKE "DQEmgData-mfc - Win32 Debug" /REBUILD /OUT build.log
type build.log
echo y | del build.log
echo on
