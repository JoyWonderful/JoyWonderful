@echo off
chcp 65001

setlocal

cd %~dp0

set file=".\student-system.c"
set out=".\student-system.exe"
set gcc="gcc.exe"
set args="-fexec-charset=UTF-8"

%gcc% %args% %file% -o %out%

endlocal

pause