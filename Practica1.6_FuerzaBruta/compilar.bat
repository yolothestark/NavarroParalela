@echo off
cd /d "%~dp0"
echo ========================================================
echo   Compilando Practica 1.6: Fuerza Bruta MPI + OpenMP
echo ========================================================
powershell -ExecutionPolicy Bypass -File "%~dp0build.ps1"
pause
