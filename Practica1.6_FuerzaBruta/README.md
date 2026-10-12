# Práctica 1.6: Algoritmo de Fuerza Bruta Distribuido con MPI y OpenMP

**Universidad de Guadalajara (UDG)**  
**Centro Universitario de Tonalá (CUTonalá)**  
**Materia:** Computación Paralela  
**Equipo:** Equipo Giallo  

### Integrantes (Orden Alfabético por Primer Apellido):
1. **Anguiano Garcia Angel Yahir Guadalupe** — Código: 221076554
2. **Figueroa Robles Axel Israel**

---

## 📌 Descripción del Proyecto
Este proyecto implementa una aplicación paralela y distribuida para descifrar contraseñas alfanuméricas mediante **Fuerza Bruta** bajo una arquitectura **Maestro-Esclavo**:
- **Proceso Maestro (Nodo 0):** Gestiona la clave ingresada, segmenta equitativamente el espacio de búsqueda ($36^N$) entre los procesos esclavos, mantiene escucha no bloqueante mediante `MPI_Iprobe` y propaga señales de **Cancelación / Parada Temprana (Early Termination)** al hallar la clave.
- **Procesos Esclavos (Nodos 1 a $P-1$):** Reciben su rango numérico $[Inicio, Fin]$, dividen el trabajo entre sus núcleos multihilo locales con OpenMP (`#pragma omp parallel for schedule(dynamic)`) y emiten la notificación de hallazgo al Maestro tan pronto como localizan la combinación exacta.

---

## 🔤 Espacio de Búsqueda y Mapeo Numérico Base 36
- **Alfabeto ($\Sigma$):** 36 caracteres en mayúsculas (`'A'..'Z'` y `'0'..'9'`).
- **Mapeo:** Cada combinación de longitud $N$ corresponde unívocamente a un entero de 64 bits en base 36 en el rango $[0, 36^N - 1]$.
  - $N = 3$: $36^3 = 46,656$ combinaciones.
  - $N = 7$: $36^7 = 78,364,164,096$ combinaciones (~78.3 mil millones).
  - $N = 10$: $36^{10} = 3,656,158,440,062,976$ combinaciones (~3.65 cuadrillones).

---

## 🏗️ Arquitectura de la Solución

```text
[Nodo 0: Maestro (Coordinador)]
       │
       ├─► Entrada y Validación de la Clave (A-Z, 0-9)
       ├─► Segmentación de Rangos: [Inicio_i, Fin_i]
       └─► Envío de Rangos y Target (MPI_Send / MPI_Bcast)
              │
              ├───► [Nodo 1: Esclavo] ──► OpenMP (schedule dynamic)
              ├───► [Nodo 2: Esclavo] ──► OpenMP (schedule dynamic)
              ├───► [Nodo 3: Esclavo] ──► OpenMP (schedule dynamic) ──► ¡HALLAZGO!
              └───► [Nodo 4: Esclavo] ──► OpenMP (schedule dynamic)         │
                            │                                               ▼
                            │◄─── [TAG_HALLAZGO: rank, tid, idx] ───────────┘
                            ▼
              [Maestro: Orden de Parada Temprana a Nodos 1, 2 y 4]
                            │
                            ▼
              [Persistencia en Logs: log_equipo_[PC]_nodo_[RANK].txt]
```

---

## 💻 Requisitos y Compilación
- **Compilador:** MinGW-w64 GCC con soporte OpenMP (`-fopenmp`)
- **MPI:** Microsoft MPI (MS-MPI v10.1+ runtime y SDK)
- **Compilar:** Doble clic en `compilar.bat` o vía PowerShell con `build.ps1`.
- **Ejecutar Localmente (5 procesos MPI):** Doble clic en `EJECUTAR_DEMO_LOCAL.bat`.
- **Ejecutar en Red (Clúster Multiequipo):** Doble clic en `ejecutar_red.bat` con configuración en `hosts.txt`.
