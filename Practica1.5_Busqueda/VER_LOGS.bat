@echo off
echo Abriendo los 5 archivos de log generados por los nodos MPI en el Bloc de Notas...
for %%f in (log_equipo_*_nodo_*.txt) do (
    start notepad.exe "%%f"
)
