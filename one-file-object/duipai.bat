@echo off

:: name: problem name
:: compPath: for g++/gcc compiler path
:: compPar: for params of comiler
:: maxcase: for total cases number
set name=network
set compPath=g++
set compPar=-O2
set maxcase=4

if exist duipai.txt (
    del duipai.txt
)

for /L %%i in (1, 1, %maxcase%) do (
    copy %name%\%name%%%i.in %name%.in
    copy %name%\%name%%%i.ans %name%.ans
    %compPath% %name%.cpp -o %name%.exe %compPar%
    %name%.exe
    
    echo Case %%i: >> duipai.txt
    echo. >> duipai.txt
    fc /N %name%.ans %name%.out >> duipai.txt
    echo. >> duipai.txt
    echo. >> duipai.txt
)

pause
