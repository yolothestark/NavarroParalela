@echo off
echo ========================================================
echo   Ejecutando Version 1: MPI_Send / MPI_Recv (5 Procesos)
echo ========================================================
mpiexec -n 5 .\main_send_recv.exe
pause
