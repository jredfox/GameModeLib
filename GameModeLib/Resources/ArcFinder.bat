@ECHO OFF
setlocal enableDelayedExpansion

set arc=%PROCESSOR_ARCHITEW6432%
IF "!arc!" EQU "" (set arc=%PROCESSOR_ARCHITECTURE%)
echo !arc!
