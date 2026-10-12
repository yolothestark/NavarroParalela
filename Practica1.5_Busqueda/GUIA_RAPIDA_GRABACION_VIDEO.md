# 🎬 Guía Rápida para Grabar el Video de la Actividad 1.5 (Máximo 5 Minutos)

**Universidad de Guadalajara (UDG) - Centro Universitario de Tonalá (CUTonalá)**  
**Materia:** Computación Paralela  
**Equipo:** Anguiano Garcia Angel Yahir Guadalupe & Figueroa Robles Axel Israel  

---

## ⏱️ Estructura del Video Minuto a Minuto

### Minuto 0:00 - 0:45 | Introducción y Portada
1. **Qué mostrar en pantalla:** La carpeta del proyecto con los archivos `main.cpp`, `BusquedaParalela.h`, `compilar.bat`, etc.
2. **Qué decir:**
   > *"Buenas tardes, profesor y compañeros. Presentamos la Actividad 1.5: Búsqueda Paralela Distribuida con MPI y OpenMP. Mi nombre es Angel Yahir Guadalupe Anguiano Garcia junto con mi compañero Axel Israel Figueroa Robles. En esta práctica implementamos un sistema híbrido que combina la memoria distribuida de MPI con la memoria compartida multihilo de OpenMP, utilizando estrictamente memoria dinámica con punteros en C++."*

---

### Minuto 0:45 - 1:30 | Explicación del Código y Arquitectura
1. **Qué mostrar en pantalla:** Abre `BusquedaParalela.h` y `BusquedaParalela.cpp` en Code::Blocks o VS Code.
2. **Puntos clave a señalar:**
   - **Gestión de Memoria Dinámica:** Muestra que no hay `vector` ni arreglos estáticos; todo usa `new int[...]` y `delete[]`.
   - **Ordenamiento con OpenMP:** Muestra la función `mergeSortPar` con directivas `#pragma omp task` y `#pragma omp taskwait`, explicando que es un requisito previo indispensable para la búsqueda binaria.
   - **Búsqueda Secuencial Distribuida:** Muestra `MPI_Scatterv`, el ciclo `#pragma omp parallel for` local y la consolidación con `MPI_Allreduce`.
   - **Búsqueda Binaria Distribuida:** Muestra cómo cada nodo evalúa primero los límites `[bloque[0], bloque[size-1]]` en $O(1)$, descartando bloques que no contienen el valor y activando los hilos OpenMP únicamente en el nodo correspondiente.

---

### Minuto 1:30 - 3:00 | Ejecución de la Demo Local (5 Procesos MPI)
1. **Qué mostrar en pantalla:** Ejecuta dando doble clic a **`EJECUTAR_DEMO_LOCAL.bat`**.
2. **Paso 1: Nombres del equipo:**
   - Señala en la consola que la primera línea de ejecución muestra los nombres completos en orden alfabético por primer apellido.
3. **Paso 2: Presiona la opción `5` (Pruebas Automatizadas):**
   - **Prueba 1 (Control $N=100$):**
     - Explica que para 100 elementos el modo debug está activo.
     - Muestra cómo el Nodo 0 ordenó el arreglo, el Nodo 2 encontró el elemento en el índice global 50, y los Nodos 0, 1, 3 y 4 descartaron sus bloques por evaluación de límites.
   - **Prueba 2 (Rendimiento $N=10,000,000$):**
     - Muestra el tiempo de ordenamiento previo: **Parallel Merge Sort con OpenMP tardó 0.45 segundos** contra **1.96 segundos secuencial** (**Speedup de más de 4.3x**).
     - Muestra los tiempos de búsqueda secuencial y búsqueda binaria.
4. **Paso 3: Presiona la opción `6` (Tabla Consolidada):**
   - Muestra la tabla en pantalla con los tiempos y factores de aceleración.
5. **Paso 4: Presiona la opción `7` (Salir):**
   - Señala que la última línea de ejecución vuelve a imprimir los nombres oficiales del equipo.

---

### Minuto 3:00 - 4:15 | Muestra de los 5 Archivos de Log Generados
1. **Qué hacer:** Al terminar el script, presiona `S` para abrir automáticamente los archivos de log o da doble clic a **`VER_LOGS.bat`**.
2. **Qué mostrar en pantalla:** Pon en pantalla los 5 blocs de notas:
   - `log_equipo_Giallo_nodo_0.txt`
   - `log_equipo_Giallo_nodo_1.txt`
   - `log_equipo_Giallo_nodo_2.txt`
   - `log_equipo_Giallo_nodo_3.txt`
   - `log_equipo_Giallo_nodo_4.txt`
3. **Qué decir:**
   > *"Como se puede observar en la auditoría, cada uno de los 5 procesos MPI generó síncronamente su propio archivo de registro identificando el nodo, el número de hilos OpenMP, los límites del bloque y el estado de la búsqueda con su índice global correspondiente."*

---

### Minuto 4:15 - 5:00 | Conclusiones y Cierre
1. **Qué decir:**
   > *"En conclusión, comprobamos que el ordenamiento mediante tareas de OpenMP acelera significativamente la preparación de grandes volúmenes de datos, y que la búsqueda binaria distribuida es sumamente eficiente gracias al descarte de bloques en $O(1)$ sin sobrecargar la red ni el procesador. Con esto concluimos la demostración de la Actividad 1.5. Muchas gracias."*
