# Práctica 1.5: Búsqueda Paralela Distribuida con MPI y OpenMP

**Universidad de Guadalajara (UDG)**  
**Centro Universitario de Tonalá (CUTonalá)**  
**Materia:** Computación Paralela  
**Equipo:** Equipo Giallo  

### Integrantes (Orden Alfabético por Primer Apellido):
1. **Anguiano Garcia Angel Yahir Guadalupe**
2. **Figueroa Robles Axel Israel**

---

## 📌 Descripción del Proyecto
Este proyecto implementa un sistema híbrido de **Búsqueda Paralela Distribuida por Bloques** combinando:
- **MS-MPI (Message Passing Interface):** Para la partición, distribución por bloques y recolección de resultados en entornos de memoria distribuida (red / clúster).
- **OpenMP (Open Multi-Processing):** Para el procesamiento multihilo en memoria compartida dentro de cada nodo, acelerando tanto el ordenamiento previo como la inspección interna de los bloques asignados.

Se comparan dos algoritmos fundamentales de búsqueda:
1. **Búsqueda Secuencial Distribuida por Bloques:** Complejidad $O(N)$ paralelizada a nivel de nodo e hilo.
2. **Búsqueda Binaria Distribuida por Bloques:** Requiere ordenamiento previo mediante **Parallel Merge Sort** con OpenMP. Evalúa límites de bloques ($O(1)$) y realiza búsqueda binaria multihilo en sub-intervalos ($O(\log(N/(p \cdot T)))$).

---

## 🏗️ Arquitectura y Flujo del Sistema

```text
[Nodo 0: Maestro]
       │
       ├─► Generación Dinámica de Datos (int* arr = new int[N])
       ├─► Ordenamiento Paralelo con OpenMP (Parallel Merge Sort con #pragma omp task)
       ├─► Broadcast de Parámetros y Target (MPI_Bcast)
       └─► Distribución Equitativa de Bloques (MPI_Scatterv)
              │
              ├───► [Nodo 0: Bloque 0] ──► OpenMP Threads (Búsqueda interna)
              ├───► [Nodo 1: Bloque 1] ──► OpenMP Threads (Búsqueda interna)
              ├───► [Nodo 2: Bloque 2] ──► OpenMP Threads (Búsqueda interna)
              ├───► [Nodo 3: Bloque 3] ──► OpenMP Threads (Búsqueda interna)
              └───► [Nodo 4: Bloque 4] ──► OpenMP Threads (Búsqueda interna)
                            │
                            ▼
              [Consolidación Global: MPI_Allreduce con MPI_MIN / MPI_MAX]
                            │
                            ▼
              [Persistencia en Logs Locales: log_equipo_[PC]_nodo_[RANK].txt]
```

---

## 💻 Requisitos y Configuración
- **Sistema Operativo:** Windows 10 / 11
- **Compilador:** MinGW-w64 GCC (con soporte para OpenMP `-fopenmp`)
- **MPI:** Microsoft MPI (MS-MPI v10.1+ runtime y SDK)
- **Variables de Entorno requeridas:**
  - `MSMPI_INC` = `C:\Program Files (x86)\Microsoft SDKs\MPI\Include`
  - `MSMPI_LIB64` = `C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64`
  - `PATH` con `C:\Program Files\Microsoft MPI\Bin\`

---

## 🚀 Compilación y Ejecución

### 1. Compilación
Ejecute el archivo por lotes:
```cmd
compilar.bat
```
O directamente vía PowerShell:
```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

### 2. Ejecución Local (5 Procesos MPI)
```cmd
ejecutar_local.bat
```
O desde la línea de comandos:
```cmd
mpiexec -n 5 Practica1_5.exe
```

### 3. Ejecución Distribuida en Red
Configure las direcciones IP de las máquinas esclavas en `hosts.txt` y ejecute:
```cmd
ejecutar_red.bat
```

---

## 📂 Archivos Generados y Logs de Auditoría
Cada nodo genera de manera síncrona e independiente su propio archivo de registro:
- `log_equipo_[NOMBRE_PC]_nodo_0.txt`
- `log_equipo_[NOMBRE_PC]_nodo_1.txt`
- `log_equipo_[NOMBRE_PC]_nodo_2.txt`
- `log_equipo_[NOMBRE_PC]_nodo_3.txt`
- `log_equipo_[NOMBRE_PC]_nodo_4.txt`

Cada archivo contiene el encabezado de especificaciones del procesador, hilos detectados, tiempos de ordenamiento y el rastreo exacto de bloques y búsquedas con el formato reglamentario.
