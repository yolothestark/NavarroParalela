@echo off
chcp 65001 > nul
echo =======================================================
echo    Iniciando Simulacion Bancaria (MPI + OpenMP)
echo    Se levantaran 5 procesos (1 Servidor, 4 Clientes)
echo =======================================================
echo.

mpiexec -n 5 .\principal.exe

echo.
pause
