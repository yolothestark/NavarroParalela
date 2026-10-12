@echo off
echo ========================================================
echo   Limpiando archivos de log previos para nueva prueba...
echo ========================================================
del /f /q log_equipo_*_nodo_*.txt 2>nul
echo Archivos de log eliminados. El directorio esta limpio para grabar.
pause
