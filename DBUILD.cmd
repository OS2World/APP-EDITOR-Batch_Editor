@echo off
set debug=1
mode co80,102

SET INCLUDE=F:\DEV\CVS\ECS-INSTALLER\ECSGUIDPROCSRC\HEADERS;%INCLUDE%
rem if the flag file doesn't exist create it and touch all the sources
if exist @debug@ goto build

echo this is a flag file: do not delete! > @debug@
for %%1 in (*.c) do touch %%1
goto build

:tryagain
echo.
echo Premere un tasto qualsiasi per rieseguire NMAKE
echo Premere Ctrl-C per terminare.
pause > nul

:build
cls
nmake -nologo
if errorlevel 1 goto tryagain

