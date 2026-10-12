@echo off
cd /d "%~dp0"
echo ========================================================
echo   Compilando Practica 1.5: Busqueda Paralela MPI + OpenMP
echo ========================================================
powershell -ExecutionPolicy Bypass -File "%~dp0build.ps1"
pause
