@echo off
title Simulacion Red MPI (3 Nodos)
set PATH=%PATH%;C:\Program Files\CodeBlocks\MinGW\bin
cd bin\Debug
mpiexec -machinefile ..\..\hosts.txt -n 6 Actividad_1.exe
echo.
pause
