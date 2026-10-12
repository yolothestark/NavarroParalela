# Script de compilacion para Practica 1.5 en Windows PowerShell
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host " Compilando Practica 1.5: Busqueda Paralela MPI + OpenMP" -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan

$gpp = "C:\Program Files\CodeBlocks\MinGW\bin\g++.exe"
if (-not (Test-Path $gpp)) {
    $gpp = (Get-Command g++.exe -ErrorAction SilentlyContinue).Source
}

if (-not $gpp) {
    Write-Host "[ERROR] No se encontro el compilador g++.exe." -ForegroundColor Red
    exit 1
}

$inc = $env:MSMPI_INC.TrimEnd('\')
if (-not $inc) { $inc = "C:\Program Files (x86)\Microsoft SDKs\MPI\Include" }

$lib = $env:MSMPI_LIB64.TrimEnd('\')
if (-not $lib) { $lib = "C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64" }

Write-Host "Compilador: $gpp" -ForegroundColor Yellow
Write-Host "MPI Include: $inc" -ForegroundColor Yellow
Write-Host "MPI Lib64:   $lib" -ForegroundColor Yellow

& "$gpp" -O3 -fopenmp -static -static-libgcc -static-libstdc++ -I "$inc" -L "$lib" BusquedaParalela.cpp main.cpp -lmsmpi -o Practica1_5.exe

# Copiar DLLs por si alguna dependencia dinamica persiste
$mingwBin = Split-Path -Parent $gpp
$dlls = @("libgomp-1.dll", "libgcc_s_seh-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll")
foreach ($d in $dlls) {
    $src = Join-Path $mingwBin $d
    if (Test-Path $src) {
        Copy-Item $src .\ -Force -ErrorAction SilentlyContinue
    }
}

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n[EXITO] Compilacion completada con exito -> Practica1_5.exe" -ForegroundColor Green
} else {
    Write-Host "`n[ERROR] Ocurrio un error durante la compilacion." -ForegroundColor Red
}
