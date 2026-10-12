@echo off
cd /d "%~dp0"
echo ===============================================================================
echo   Abriendo archivos de log generados por los nodos MPI en el Bloc de Notas...
echo ===============================================================================
for %%f in (log_equipo_*_nodo_*.txt) do (
    echo  - Abriendo: %%~nxf
    start notepad.exe "%%f"
)
echo ===============================================================================
echo [OK] Archivos de log abiertos en Bloc de Notas para tu video.
echo ===============================================================================
