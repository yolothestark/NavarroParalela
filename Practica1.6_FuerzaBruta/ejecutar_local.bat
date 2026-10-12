@echo off
cd /d "%~dp0"
title Practica 1.6 - Ejecucion Local MPI (5 Procesos)
echo ========================================================================
echo   ACTIVIDAD 1.6: ALGORITMO DE FUERZA BRUTA DISTRIBUIDO (MPI + OPENMP)
echo   EJECUCION LOCAL: 5 PROCESOS MPI (1 MAESTRO + 4 ESCLAVOS)
echo ========================================================================
echo.
if not exist "Practica1_6.exe" (
    echo [ADVERTENCIA] Practica1_6.exe no existe. Compilando automaticamente...
    call "%~dp0compilar.bat"
)

mpiexec -n 5 Practica1_6.exe
pause
