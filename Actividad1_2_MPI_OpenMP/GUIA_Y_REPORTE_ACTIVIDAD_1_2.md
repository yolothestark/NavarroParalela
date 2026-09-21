# REPORTE Y GUÍA COMPLETA DE LA ACTIVIDAD 1.2: CARGA DE MEMORIA DISTRIBUIDA EN MPI Y OPENMP

## 1. Portada y Datos del Equipo
- **Institución:** Universidad / Facultad de Ingeniería
- **Materia:** Programación Paralela y Distribuida
- **Actividad:** Actividad 1.2: Carga de memoria distribuida en MPI. Práctica
- **Integrantes (Iniciando por Primer Apellido en Orden Alfabético):**
  1. **ANGUIANO GARCIA, ANGEL YAHIR GUADALUPE**
  2. **FIGUEROA ROBLES, AXEL ISRAEL** `[APELLIDO_MATERNO_3]`, `[NOMBRE_3]`
- **Fecha:** Septiembre 2026

---

## 2. Objetivo de la Práctica
Implementar el modelo híbrido de paralelismo combinando **Memoria Distribuida (MPI)** y **Memoria Compartida (OpenMP)**. La distribución de carga se realiza entre procesos mediante un esquema Maestro-Trabajador (1 nodo maestro y 4 nodos trabajadores, 5 procesos en total), mientras que dentro de cada proceso se emplea OpenMP para paralelizar las operaciones aritméticas a nivel de hilos utilizando los núcleos disponibles en la CPU.

---

## 3. Arquitectura del Sistema y Explicación Técnica

### 3.1. Clase `OperacionesArreglos`
La clase OperacionesArreglos encapsula las operaciones aritméticas elementales sobre arreglos dinámicos mediante punteros tradicionales (`double*`):
- `crearArregloMPI`: Llena las secciones de los arreglos. En modo detallado ($N=40$), genera valores consecutivos e imprime el detalle por hilo y nodo; en modo masivo ($N=4,000,000$), genera números aleatorios entre 1 y 1,000,000 sin sobrecarga de E/S.
- `sumar`: Calcula $C[i] = A[i] + B[i]$.
- `restar`: Calcula $C[i] = A[i] - B[i]$.
- `multiplicar`: Calcula $C[i] = A[i] \times B[i]$.
- `cuadrado`: Calcula $C[i] = (A[i])^2$.

Cada una de estas funciones utiliza la directiva `#pragma omp parallel for schedule(static)` para repartir las iteraciones del bucle equitativamente entre los hilos del procesador. Durante la ejecución detallada, se utiliza una sección crítica (`#pragma omp critical`) para sincronizar la salida estándar e imprimir con el formato solicitado:
`[Equipo: <nombre>] [Proceso MPI: <rank>] [Hilo OpenMP: <tid>] [Posición: <pos>] [Operación: <detalle>]`

### 3.2. Implementación 1: `MPI_Send` y `MPI_Recv` (Punto a Punto)
- **Mecanismo:** 
  1. El nodo Maestro (Rank 0) transmite la opción seleccionada por menú a todos los nodos con `MPI_Bcast`.
  2. El Maestro envía mediante un bucle explícito con `MPI_Send` el bloque o sección del arreglo a cada uno de los 4 trabajadores (Ranks 1 al 4).
  3. Cada trabajador recibe su sección con `MPI_Recv`, la procesa con OpenMP, y devuelve el resultado parcial al maestro con otro `MPI_Send`.
  4. El maestro recibe cada bloque con `MPI_Recv` y reconstruye el arreglo final `C`.

### 3.3. Implementación 2: `MPI_Scatter` y `MPI_Gather` (Comunicaciones Colectivas)
- **Mecanismo:**
  1. El Maestro transmite la opción del menú con `MPI_Bcast`.
  2. Con una sola llamada colectiva a `MPI_Scatter`, el búfer del maestro es dividido en trozos de tamaño equitativo y distribuido simultáneamente a todos los procesos del comunicador.
  3. Los procesos trabajadores calculan la operación asignada mediante OpenMP.
  4. Con una sola llamada colectiva a `MPI_Gather`, los resultados parciales de cada trabajador son recopilados de forma sincronizada en el búfer maestro.

### 3.4. Comparación entre Send/Recv y Scatter/Gather
| Criterio | Versión Punto a Punto (`Send`/`Recv`) | Versión Colectiva (`Scatter`/`Gather`) |
| :--- | :--- | :--- |
| **Tipo de Comunicación** | Bloqueante individualizada (1 a 1). | Colectiva optimizada (1 a Todos / Todos a 1). |
| **Complejidad del Código** | Requiere bucles `for` para iterar sobre cada trabajador. | Una sola línea de llamada colectiva por operación. |
| **Sincronización** | Asíncrona o bloqueante por mensaje según el búfer. | Implica sincronización implícita de los participantes. |
| **Rendimiento en 4,000,000 elementos** | Mayor sobrecarga por mensajes individuales. | Altamente optimizado por la pila de red MPI. |

---

## 4. Guía para la Grabación del Video (Máx 5 Minutos, 720p+)
> **Importante:** Asegúrate de que la grabación capture la **pantalla completa** (mostrando fecha y hora de tu sistema en la barra de tareas).

1. **Introducción (0:00 - 0:45):**
   - Presentar al equipo y los nombres de los integrantes.
   - Explicar el objetivo: Implementación de memoria distribuida con MPI (5 procesos locales) y memoria compartida con OpenMP sobre arreglos dinámicos.
2. **Explicación del Código (0:45 - 1:45):**
   - Mostrar `OperacionesArreglos.h` y `OperacionesArreglos.cpp` (resaltar el uso de punteros `double*` y `#pragma omp parallel for`).
   - Mostrar la diferencia entre `main_send_recv.cpp` (`MPI_Send`/`MPI_Recv`) y `main_scatter_gather.cpp` (`MPI_Scatter`/`MPI_Gather`).
   - Mostrar la impresión de los integrantes al inicio y fin del `main`.
3. **Demostración de Ejecución con 40 elementos (1:45 - 3:00):**
   - Ejecutar `ejecutar_send_recv.bat`.
   - Seleccionar opción 1 (Crear arreglos) y opción 2 (Suma).
   - Resaltar en pantalla cómo cada línea muestra claramente:
     `[Equipo: ...] [Proceso MPI: ...] [Hilo OpenMP: ...] [Posición: ...] [Operación: ...]`
   - Mostrar cómo los 4 procesos trabajadores (Rank 1 a 4) procesan 10 elementos cada uno y los hilos de OpenMP participan en paralelo.
4. **Demostración con 4,000,000 de elementos (3:00 - 4:00):**
   - En el menú, seleccionar la **Opción 6 (Cambiar tamaño)**.
   - Ejecutar las operaciones (Suma, Multiplicación, Cuadrado) y mostrar los tiempos reportados por `MPI_Wtime`.
5. **Comparación con la versión Scatter / Gather (4:00 - 4:45):**
   - Ejecutar `ejecutar_scatter_gather.bat` y mostrar su funcionamiento tanto en 40 elementos como en 4M de elementos.
6. **Conclusión y Cierre (4:45 - 5:00):**
   - Resumir los resultados obtenidos y despedida del equipo.

