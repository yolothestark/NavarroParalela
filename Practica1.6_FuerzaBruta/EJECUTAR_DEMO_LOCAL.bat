@echo off
set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
cd /d "%SCRIPT_DIR%"

color 0B
title Practica 1.6: Fuerza Bruta Distribuida (MPI + OpenMP) - DEMO LOCAL
cls
echo ===============================================================================
echo      CENTRO UNIVERSITARIO DE TONALA (CUTONALA) - UDG
echo                          MATERIA: COMPUTACION PARALELA
echo       PRACTICA 1.6: ALGORITMO DE FUERZA BRUTA DISTRIBUIDO (MPI + OPENMP)
echo ===============================================================================
echo   INTEGRANTES DEL EQUIPO:
echo   1. Anguiano Garcia Angel Yahir Guadalupe
echo   2. Figueroa Robles Axel Israel
echo ===============================================================================
echo.
echo   [MODO DE EJECUCION LOCAL: CLUSTER DE 5 PROCESOS MPI EN ESTA COMPUTADORA]
echo   - 1 Proceso Maestro (Rank 0 - Coordinador y receptor de Early Termination)
echo   - 4 Procesos Esclavos (Ranks 1, 2, 3, 4 - Trabajadores de Fuerza Bruta)
echo   - Cada esclavo paraleliza con OpenMP usando schedule(dynamic) en sus hilos
echo.
echo   Sugerencia para tu demostracion / video:
echo   1. Opcion 4: Ejecuta la secuencia automatizada de pruebas y escalabilidad.
echo   2. Opcion 5: Muestra la tabla consolidada de tiempos y Speedup.
echo   3. Opcion 6: Finaliza la sesion.
echo.
echo ===============================================================================

if not exist "Practica1_6.exe" (
    echo [AVISO] Compilando Practica1_6.exe por primera vez...
    call compilar.bat
)

echo Presione cualquier tecla para iniciar el cluster MPI...
pause >nul
echo.
echo ===============================================================================
echo Iniciando cluster local de 5 procesos con MS-MPI...
echo ===============================================================================
echo.

mpiexec -n 5 Practica1_6.exe
set "RET_CODE=%errorlevel%"

echo.
if "%RET_CODE%" neq "0" (
    echo ===============================================================================
    echo [ERROR] Hubo un problema al ejecutar el programa (Codigo de error: %RET_CODE%).
    echo Verifique que MS-MPI este instalado y que Practica1_6.exe no este bloqueado.
    echo ===============================================================================
    pause
    exit /b %RET_CODE%
)

echo ===============================================================================
echo Ejecucion local finalizada con exito.
echo Se han generado 5 archivos de auditoria independientes:
echo  - log_equipo_Giallo_nodo_0.txt (Maestro)
echo  - log_equipo_Giallo_nodo_1.txt (Esclavo 1)
echo  - log_equipo_Giallo_nodo_2.txt (Esclavo 2)
echo  - log_equipo_Giallo_nodo_3.txt (Esclavo 3)
echo  - log_equipo_Giallo_nodo_4.txt (Esclavo 4)
echo ===============================================================================
echo.
echo Presione cualquier tecla para abrir los 5 archivos de log en el Bloc de Notas...
pause >nul
call VER_LOGS.bat
pause
