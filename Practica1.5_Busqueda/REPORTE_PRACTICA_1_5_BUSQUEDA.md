# UNIVERSIDAD DE GUADALAJARA
## Centro Universitario de Ciencias Exactas e Ingenierías (CUCEI)
### División de Tecnologías para la Ciber-Innovación
**Materia:** Computación Paralela  
**Práctica:** Actividad 1.5: Búsqueda Paralela Distribuida con MPI y OpenMP  
**Equipo:** Equipo Giallo  

---

### Integrantes del Equipo (Primer apellido en orden alfabético):
1. **Anguiano Garcia Angel Yahir Guadalupe** — Código: 221076554
2. **Figueroa Robles Axel Israel**

**Fecha de Entrega:** 11 de Octubre de 2026  

---

# 1. Introducción y Objetivo

El presente informe documenta el diseño, desarrollo, implementación y análisis de rendimiento de un sistema de **búsqueda paralela híbrida (MPI + OpenMP)** implementado sobre arquitecturas de computación distribuida y multinúcleo en lenguaje C++.

### Objetivo Principal:
Implementar un sistema de búsqueda distribuida por bloques sobre arreglos dinámicos en memoria que permita comparar la eficiencia computacional entre:
1. **Búsqueda Secuencial Distribuida por Bloques.**
2. **Búsqueda Binaria Distribuida por Bloques.**

Evaluando ambos algoritmos bajo dos modalidades de paralelismo:
- **Esquema Local Multihilo (OpenMP):** Procesamiento concurrente en memoria compartida dentro de un solo nodo físico.
- **Esquema Distribuido Multinivel (MPI + OpenMP):** Procesamiento híbrido donde el estándar **MPI** particiona y distribuye los bloques entre nodos interconectados (memoria distribuida), mientras que **OpenMP** explota los núcleos locales de cada equipo mediante hilos concurrentes (memoria compartida).

Adicionalmente, se garantiza la integridad del sistema implementando un ordenamiento paralelo previo (**Parallel Merge Sort** con OpenMP) y un mecanismo estricto de auditoría síncrona mediante archivos de registro (`log`) persistentes e independientes por cada nodo ejecutor.

---

# 2. Desarrollo y Propuesta Técnica

## 2.1. Arquitectura del Sistema Híbrido Multinivel

El diseño de la aplicación se divide en dos capas de concurrencia:
- **Capa Distribuida (Nivel MPI):** El nodo maestro (Rank 0) genera el arreglo dinámico completo de enteros de tamaño $N$, realiza el ordenamiento previo con OpenMP (imprescindible para la búsqueda binaria), difunde el elemento a buscar (*target*) mediante `MPI_Bcast`, y dispersa los bloques correspondientes a los procesos trabajadores mediante `MPI_Scatterv`.
- **Capa Local (Nivel OpenMP):** Cada proceso MPI recibe su bloque local contiguo de tamaño $N / p$. Al recibir la instrucción de búsqueda, crea una región paralela (`#pragma omp parallel` / `#pragma omp parallel for`) para evaluar internamente los datos asignados en múltiples sub-intervalos simultáneamente.
- **Consolidación de Resultados:** Mediante `MPI_Allreduce` con el operador `MPI_MIN`, los nodos reportan el índice global exacto del primer elemento coincidente ($idx_{global} = offset_{nodo} + idx_{local}$), garantizando sincronización con complejidad $O(\log p)$.

```
                            [Nodo 0: Proceso Maestro]
                                       │
            ┌──────────────────────────┴──────────────────────────┐
            ▼                                                     ▼
 [Generación Dinámica: new int[N]]                [Broadcast de Target: MPI_Bcast]
            │
            ▼
 [Ordenamiento OpenMP: Merge Sort Paralelo]
            │
            ▼
 [Distribución Colectiva: MPI_Scatterv]
    │                 │                 │                 │                 │
    ▼                 ▼                 ▼                 ▼                 ▼
[Nodo 0]          [Nodo 1]          [Nodo 2]          [Nodo 3]          [Nodo 4]
Bloque 0          Bloque 1          Bloque 2          Bloque 3          Bloque 4
[0..N/p-1]      [N/p..2N/p-1]    [2N/p..3N/p-1]    [3N/p..4N/p-1]    [4N/p..N-1]
    │                 │                 │                 │                 │
    ├─── OpenMP ──────┼─── OpenMP ──────┼─── OpenMP ──────┼─── OpenMP ──────┤
    ▼                 ▼                 ▼                 ▼                 ▼
(T Hilos)         (T Hilos)         (T Hilos)         (T Hilos)         (T Hilos)
    │                 │                 │                 │                 │
    └─────────────────┴────────┬────────┴─────────────────┴─────────────────┘
                               ▼
            [Consolidación Global: MPI_Allreduce(MPI_MIN)]
                               │
                               ▼
            [Persistencia Local en Logs Independientes]
```

---

## 2.2. Algoritmos de Búsqueda Implementados

### A. Búsqueda Secuencial Distribuida por Bloques
- **Mecánica:** Cada nodo recibe su fragmento de datos. Internamente, una directiva `#pragma omp parallel for schedule(static)` divide los elementos del bloque entre los hilos de OpenMP disponibles. Cada hilo recorre secuencialmente su partición comparando con el valor buscado.
- **Detección y Mapeo:** Si el elemento coincide, el hilo calcula la posición absoluta en el arreglo general mediante:
  $$\text{Índice Global} = \text{Offset del Nodo} + \text{Índice Local}$$
- **Complejidad Algorítmica:**
  - Secuencial puro: $O(N)$
  - Paralelo Híbrido: $O\left(\frac{N}{p \cdot T}\right) + O(\text{latencia de red})$
  donde $p$ es el número de procesos MPI y $T$ es el número de hilos OpenMP por nodo.

### B. Búsqueda Binaria Distribuida por Bloques
- **Requisito Previsto:** El arreglo global debe encontrarse ordenado ascendentemente.
- **Fase 1: Evaluación y Descarte de Límites ($O(1)$ por nodo):**
  Cada proceso MPI inspecciona exclusivamente el primer y el último elemento de su bloque local:
  $$\text{Rango Local} = [\text{bloque}[0], \;\text{bloque}[\text{size\_local} - 1]]$$
  Si el valor `target` está fuera de este rango, el nodo descarta instantáneamente su bloque sin inspeccionar sus millones de elementos internos, reportando `-1`.
- **Fase 2: Búsqueda Binaria Local Multihilo:**
  El único nodo (o nodos) cuyo rango contiene el valor divide su bloque en $T$ sub-intervalos entre los hilos de OpenMP. El hilo asignado al sub-intervalo correspondiente ejecuta la búsqueda binaria mediante divisiones sucesivas a la mitad.
- **Complejidad Algorítmica:**
  - Secuencial puro: $O(\log_2 N)$
  - Distribuida por Bloques: $O(1) \text{ (descarte en } p-1 \text{ nodos)} + O\left(\log_2\left(\frac{N}{p \cdot T}\right)\right) + O(\log p)$

---

## 2.3. Ordenamiento Paralelo con OpenMP (Parallel Merge Sort)

Para posibilitar la búsqueda binaria sin violar las restricciones de memoria, se implementó en el Nodo Maestro un algoritmo de **Merge Sort Paralelo** basado en tareas dinámicas de OpenMP (`#pragma omp task` y `#pragma omp taskwait`).

Para evitar la saturación del planificador (*thread overhead*) en sub-arreglos minúsculos, se incorporó un **umbral condicional de corte secuencial**: cuando la longitud del sub-arreglo es inferior a 10,000 elementos, el algoritmo deja de emitir tareas y resuelve secuencialmente.

---

## 2.4. Gestión Estricta de Memoria Dinámica

En estricto cumplimiento con las especificaciones de la práctica:
- Se prohíbe el uso de `std::vector` y arreglos estáticos de tamaño fijo.
- Todos los búferes se gestionan dinámicamente con punteros nativos de C++:
  - En Nodo 0: `int* arr_global = new int[n];`
  - En todos los Nodos: `int* bloque_local = new int[size_local];`
  - Búfer temporal de fusión: `int* temp = new int[n];`
- Toda la memoria asignada es liberada formalmente mediante `delete[]` al culminar cada operación o al destruir la instancia de la clase `BusquedaParalela`.

---

## 2.5. Política de Auditoría y Persistencia de Logs

Cada nodo mantiene un flujo síncrono e independiente hacia su propio archivo local de registro:
$$\text{Formato del archivo: } \texttt{log\_equipo\_[NOMBRE\_PC]\_nodo\_[RANK].txt}$$

Estructura estandarizada del registro:
1. **Encabezado:** Hostname, rango MPI, total de hilos detectados y estampa de tiempo.
2. **Trazabilidad de Bloque y Hilo:** Cada mensaje incluye el identificador de nodo, hilo OpenMP, límites del bloque y resultado de la búsqueda:
   $$\texttt{[Equipo: Giallo] [Nodo MPI: 2] [Hilo OpenMP: 10] [Bloque: 50 - 50] [Algoritmo: Búsqueda Binaria] [Estado: Elemento Encontrado en Índice Global: 50]}$$

---

# 3. Resultados Experimentales y Comparativa de Rendimiento

Las pruebas fueron ejecutadas en un entorno de **5 procesos MPI concurrentes** sobre un procesador con **12 hilos lógicos OpenMP**.

## 3.1. Prueba de Control (Arreglo Pequeño: $N = 100$ elementos)

**Propósito:** Validar la exactitud de los índices encontrados, la correcta ordenación y la auditoría detallada de cada hilo.

### Salida de Auditoría Registrada en Consola y Logs:
```text
>>> EJECUTANDO PRUEBA CON N = 100 ELEMENTOS <<<
[Nodo 0] Arreglo dinamico de 100 elementos generado en 0.002000 s (Rango: 0 a 200)
[AUDITORIA NODO 0] Ordenamiento previo con OpenMP (Parallel Merge Sort) completado en 0.000000 s para 100 elementos.
Target seleccionado para la prueba: 96 (en indice ~50)

[Equipo: Giallo] [Nodo MPI: 0] [Hilo OpenMP: 5] [Bloque: 0 - 100] [Algoritmo: Búsqueda Secuencial Local] [Estado: Elemento Encontrado en Índice Global: 50]
[Equipo: Giallo] [Nodo MPI: 0] [Hilos: 12] [Bloque: 0 - 20] [Algoritmo: Búsqueda Secuencial Distribuida] [Estado: Elemento NO encontrado en este bloque local]
[Equipo: Giallo] [Nodo MPI: 2] [Hilo OpenMP: 5] [Bloque: 40 - 60] [Algoritmo: Búsqueda Secuencial Distribuida] [Estado: Elemento Encontrado en Índice Global: 50]
[Equipo: Giallo] [Nodo MPI: 4] [Hilos: 12] [Bloque: 80 - 100] [Algoritmo: Búsqueda Secuencial Distribuida] [Estado: Elemento NO encontrado en este bloque local]
[Equipo: Giallo] [Nodo MPI: 3] [Hilos: 12] [Bloque: 60 - 80] [Algoritmo: Búsqueda Secuencial Distribuida] [Estado: Elemento NO encontrado en este bloque local]
[Equipo: Giallo] [Nodo MPI: 1] [Hilos: 12] [Bloque: 20 - 40] [Algoritmo: Búsqueda Secuencial Distribuida] [Estado: Elemento NO encontrado en este bloque local]

[Equipo: Giallo] [Nodo MPI: 0] [Hilo OpenMP: 6] [Sub-intervalo: 48 - 55] [Algoritmo: Búsqueda Binaria Local] [Estado: Elemento Encontrado en Índice Global: 50]
[Equipo: Giallo] [Nodo MPI: 0] [Bloque: 0 - 20] [Algoritmo: Búsqueda Binaria Distribuida] [Estado: Descartado por evaluación de límites (Rango: 2 a 47)]
[Equipo: Giallo] [Nodo MPI: 1] [Bloque: 20 - 40] [Algoritmo: Búsqueda Binaria Distribuida] [Estado: Descartado por evaluación de límites (Rango: 49 a 78)]
[Equipo: Giallo] [Nodo MPI: 3] [Bloque: 60 - 80] [Algoritmo: Búsqueda Binaria Distribuida] [Estado: Descartado por evaluación de límites (Rango: 127 a 174)]
[Equipo: Giallo] [Nodo MPI: 4] [Bloque: 80 - 100] [Algoritmo: Búsqueda Binaria Distribuida] [Estado: Descartado por evaluación de límites (Rango: 175 a 196)]
[Equipo: Giallo] [Nodo MPI: 2] [Hilo OpenMP: 10] [Bloque: 50 - 50] [Algoritmo: Búsqueda Binaria Distribuida] [Estado: Elemento Encontrado en Índice Global: 50]
```

---

## 3.2. Prueba Masiva de Rendimiento ($N = 10,000,000$ elementos)

**Propósito:** Evaluar la aceleración real (*Speedup*) y la eficiencia de la paralelización al procesar grandes volúmenes de datos.

### Tabla Comparativa de Tiempos y Speedup:

| Operación / Algoritmo | Esquema de Ejecución | Procesos / Hilos | Tiempo Total (s) | Aceleración (Speedup) |
| :--- | :--- | :--- | :--- | :--- |
| **Ordenamiento Previo (Merge Sort)** | Secuencial (1 Hilo) | 1 proceso, 1 hilo | 1.964000 s | 1.00x (Base) |
| **Ordenamiento Previo (Merge Sort)** | Paralelo (OpenMP) | 1 proceso, 12 hilos | **0.450000 s** | **4.36x** |
| **Búsqueda Secuencial** | Secuencial (1 Hilo) | 1 proceso, 1 hilo | 0.004993 s | 1.00x (Base) |
| **Búsqueda Secuencial** | Local Multihilo (OpenMP) | 1 proceso, 12 hilos | **0.003000 s** | **1.66x** |
| **Búsqueda Secuencial** | Distribuido (MPI + OpenMP)| 5 procesos, 12 hilos c/u| **0.003165 s** | **1.58x** |
| **Búsqueda Binaria** | Secuencial (1 Hilo) | 1 proceso, 1 hilo | 0.000002 s | 1.00x (Base) |
| **Búsqueda Binaria** | Local Multihilo (OpenMP) | 1 proceso, 12 hilos | < 0.000001 s | Instantáneo |
| **Búsqueda Binaria** | Distribuido (MPI + OpenMP)| 5 procesos, 12 hilos c/u| 0.000184 s | Ver análisis de latencia |

---

# 4. Análisis y Discusión de Resultados

### 1. Desempeño del Ordenamiento Paralelo previo:
El ordenamiento mediante **Parallel Merge Sort con OpenMP** redujo el tiempo de ordenación de **1.964 s** a **0.450 s**, alcanzando un **Speedup de 4.36x**. La estrategia de corte condicional a 10,000 elementos evitó la creación excesiva de tareas (`task overhead`), maximizando la utilización de los núcleos físicos de la CPU.

### 2. Búsqueda Secuencial (Local vs Distribuida):
En la búsqueda secuencial sobre 10 millones de enteros, el esquema multihilo de OpenMP y el esquema híbrido distribuido demostraron una aceleración real evidente (1.66x y 1.58x respectivamente) respecto a la ejecución en un solo hilo. La ligera diferencia a favor del esquema local se debe a que la búsqueda secuencial realiza lecturas directas en la memoria RAM compartida del nodo maestro, mientras que la versión distribuida requiere la dispersión previa del arreglo (`MPI_Scatterv`) y la reducción colectiva final (`MPI_Allreduce`).

### 3. Búsqueda Binaria y el Costo de Comunicación:
La búsqueda binaria sobre un arreglo ordenado posee una complejidad asintótica extraordinariamente baja ($O(\log_2 10^7) \approx 24$ comparaciones en el peor caso). 
- En el esquema local secuencial y OpenMP, el tiempo de ejecución es prácticamente imperceptible (menos de 2 microsegundos).
- En el esquema distribuido, los 4 nodos que no contienen el valor descartan su bloque en $O(1)$ tras comprobar sus extremos (`bloque[0]` y `bloque[size-1]`), mientras el nodo restante encuentra el dato casi instantáneamente.
- El tiempo registrado en el esquema distribuido (0.000184 s) refleja principalmente la **latencia de sincronización y paso de mensajes de MPI** (`MPI_Bcast` y `MPI_Allreduce`). Esto ilustra un principio fundamental de la computación distribuida: **para algoritmos de complejidad sub-lineal ultra rápidos ($O(\log N)$ o $O(1)$), la sobrecarga de comunicación inter-nodo supera el tiempo neto de cómputo.** Sin embargo, el esquema distribuido por bloques resulta indispensable cuando el volumen de datos excede la capacidad de memoria RAM de una sola computadora.

---

# 5. Conclusiones

1. **Cumplimiento Integral de Objetivos:** Se implementó exitosamente el sistema de búsqueda paralela distribuida híbrido combinando **MS-MPI** y **OpenMP** en C++, satisfaciendo todos los requerimientos de memoria dinámica, menús interactivos, pruebas de control y rendimiento, y persistencia de logs.
2. **Eficiencia en el Ordenamiento:** El uso de directivas OpenMP basadas en tareas (`#pragma omp task`) y puntos de espera (`#pragma omp taskwait`) logró un speedup de más de 4x en el ordenamiento del arreglo masivo de 10 millones de datos, demostrando la alta idoneidad del paradigma "Divide y Vencerás" para arquitecturas multihilo.
3. **Poder del Descarte en Bloques:** La estrategia de evaluación de límites en la búsqueda binaria distribuida permitió a los nodos trabajadores descartar bloques completos de 2 millones de elementos con una sola comparación de límites ($O(1)$), minimizando el consumo de CPU en el clúster.
4. **Trazabilidad y Auditoría:** Los registros generados de forma síncrona en cada nodo (`log_equipo_[NOMBRE_PC]_nodo_[RANK].txt`) demostraron de manera irrefutable qué nodo y qué hilo de OpenMP procesó y localizó cada elemento en el arreglo global.

---

# 6. Referencias Bibliográficas

- Chandra, R., Dagum, L., Kohr, D., Maydan, D., McDonald, J., & Menon, R. (2001). *Parallel programming in OpenMP*. Morgan Kaufmann.
- Gropp, W., Lusk, E., & Skjellum, A. (2014). *Using MPI: Portable parallel programming with the Message-Passing Interface* (3.ª ed.). MIT Press.
- Pacheco, P. S. (2011). *An introduction to parallel programming*. Morgan Kaufmann.
- Quinn, M. J. (2004). *Parallel programming in C with MPI and OpenMP*. McGraw-Hill Education.
