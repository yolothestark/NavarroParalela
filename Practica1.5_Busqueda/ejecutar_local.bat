@echo off
title Practica 1.5 - Ejecucion Local MPI (5 Procesos)
echo ========================================================================
echo   ACTIVIDAD 1.5: BUSQUEDA PARALELA DISTRIBUIDA (MPI + OPENMP)
echo   EJECUCION LOCAL: 5 PROCESOS MPI (1 PROCESO MAESTRO + 4 TRABAJADORES)
echo ========================================================================
echo.
if not exist "Practica1_5.exe" (
    echo [ADVERTENCIA] Practica1_5.exe no existe. Compilando automaticamente...
    call compilar.bat
)

mpiexec -n 5 Practica1_5.exe
pause
