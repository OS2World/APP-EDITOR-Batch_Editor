@echo off
mode co80,102

SET FRENCH=TRUE
SET INCLUDE=F:\DEV\CVS\ECS-INSTALLER\ECSGUIDPROCSRC\HEADERS;%INCLUDE%
rem - check if the flag file exists
if exist @debug@ del @debug@ > nul
rem - touch all sources so new object files are generated
for %%1 in (*.c) do touch %%1

goto build

:tryagain
echo.
echo Prememere un tasto qualsiasi per rieseguire NMAKE
echo Premerere Ctrl-C per terminare.
pause > nul

:build
cls
nmake -nologo -a
if errorlevel 1 goto tryagain
CALL LXLITE BATCHED.EXE
COPY BATCHED.EXE batchedFr.exe
COPY BATCHED.EXE DISTRIBUZIONE\FRA
