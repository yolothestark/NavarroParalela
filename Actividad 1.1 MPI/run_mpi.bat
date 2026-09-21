@echo off
title Ejecucion MPI (4 Procesos)
set PATH=%PATH%;C:\Program Files\CodeBlocks\MinGW\bin
cd bin\Debug
mpiexec -n 4 Actividad_1.exe
echo.
pause
