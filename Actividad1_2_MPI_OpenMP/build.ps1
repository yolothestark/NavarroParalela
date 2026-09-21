# Script de compilacion para Windows PowerShell
Write-Host "Compilando proyectos MPI + OpenMP..." -ForegroundColor Cyan

$inc = $env:MSMPI_INC.TrimEnd('\')
$lib = $env:MSMPI_LIB64.TrimEnd('\')

Write-Host "1. Compilando Versión 1 (MPI_Send / MPI_Recv)..." -ForegroundColor Yellow
g++ -O3 -fopenmp -I "$inc" -L "$lib" src/OperacionesArreglos.cpp src/main_send_recv.cpp -lmsmpi -o main_send_recv.exe

Write-Host "2. Compilando Versión 2 (MPI_Scatter / MPI_Gather)..." -ForegroundColor Yellow
g++ -O3 -fopenmp -I "$inc" -L "$lib" src/OperacionesArreglos.cpp src/main_scatter_gather.cpp -lmsmpi -o main_scatter_gather.exe

Write-Host "3. Compilando Lanzador del Sistema..." -ForegroundColor Yellow
g++ -O3 src/lanzador.cpp -o Lanzador_Actividad1_2.exe

Write-Host "Compilación finalizada con éxito." -ForegroundColor Green
