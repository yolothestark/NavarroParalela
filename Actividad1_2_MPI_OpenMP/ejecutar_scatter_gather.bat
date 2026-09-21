@echo off
echo ========================================================
echo   Ejecutando Version 2: MPI_Scatter / MPI_Gather (5 Procesos)
echo ========================================================
mpiexec -n 5 .\main_scatter_gather.exe
pause
