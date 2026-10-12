@echo off
color 0B
title Practica 1.5: Busqueda Paralela Distribuida (MPI + OpenMP) - DEMO LOCAL
cls
echo ===============================================================================
echo      CENTRO UNIVERSITARIO DE CIENCIAS EXACTAS E INGENIERIAS (CUCEI) - UDG
echo                          MATERIA: COMPUTACION PARALELA
echo            ACTIVIDAD 1.5: BUSQUEDA PARALELA DISTRIBUIDA (MPI + OPENMP)
echo ===============================================================================
echo   INTEGRANTES DEL EQUIPO:
echo   1. Anguiano Garcia Angel Yahir Guadalupe
echo   2. Figueroa Robles Axel Israel
echo ===============================================================================
echo.
echo   [MODO DE EJECUCION LOCAL: CLUSTER DE 5 PROCESOS MPI EN ESTA COMPUTADORA]
echo   - 1 Proceso Maestro (Rank 0)
echo   - 4 Procesos Trabajadores (Ranks 1, 2, 3, 4)
echo   - Cada proceso ejecuta paralelismo multihilo con OpenMP (12 hilos concurrentes)
echo.
echo   Sugerencia para tu demostracion / video:
echo   1. Opcion 5: Ejecuta la prueba automatizada (100 elementos de control y 10M masivos).
echo   2. Opcion 6: Muestra la tabla consolidada de Speedup y tiempos.
echo   3. Opcion 7: Finaliza la sesion.
echo.
echo ===============================================================================
pause
echo.
echo Iniciando cluster local con MS-MPI...
echo.

mpiexec -n 5 Practica1_5.exe

echo.
echo ===============================================================================
echo Ejecucion local finalizada con exito.
echo Se han generado 5 archivos de auditoria independientes:
echo  - log_equipo_Giallo_nodo_0.txt
echo  - log_equipo_Giallo_nodo_1.txt
echo  - log_equipo_Giallo_nodo_2.txt
echo  - log_equipo_Giallo_nodo_3.txt
echo  - log_equipo_Giallo_nodo_4.txt
echo ===============================================================================
echo.
set /p abrir="Deseas abrir los 5 archivos de log en el Bloc de Notas para tu video? (S/N): "
if /i "%abrir%"=="S" (
    call VER_LOGS.bat
)
pause
