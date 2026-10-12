@echo off
cd /d "%~dp0"
title Practica 1.6 - Ejecucion Distribuida en Red (Cluster MS-MPI)
echo ========================================================================
echo   ACTIVIDAD 1.6: ALGORITMO DE FUERZA BRUTA DISTRIBUIDO (MPI + OPENMP)
echo   EJECUCION DISTRIBUIDA EN RED (MINIMO 3 EQUIPOS / NODOS)
echo ========================================================================
echo.
echo Recuerde que en las computadoras remotas debe:
echo 1. Estar instalado MS-MPI con el servicio smpd iniciado (smpd -d).
echo 2. Estar disponible el ejecutable Practica1_6.exe en la misma ruta compartida.
echo 3. Haber configurado los hosts en el archivo hosts.txt.
echo.
mpiexec -machinefile hosts.txt Practica1_6.exe
pause
