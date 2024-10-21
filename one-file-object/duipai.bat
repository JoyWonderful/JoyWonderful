@echo off
chcp 437

set name=network
set compPath=g++
set compPar=-O2
set maxcase=4

if exist duipai.txt (
    del duipai.txt
)

%compPath% %name%.cpp -o %name%.exe %compPar%
for /l %%i in (1, 1, %maxcase%) do (
    copy %name%\%name%%%i.in %name%.in
    copy %name%\%name%%%i.ans %name%.ans
    %name%.exe
    
    echo Case %%i: >> duipai.txt
    echo. >> duipai.txt
    fc /n %name%.ans %name%.out >> duipai.txt
    echo. >> duipai.txt
    echo. >> duipai.txt
)

pause
