# Universidad de Guadalajara
## Centro Universitario de Ciencias Exactas e Ingenierías (CUCEI)

**Materia:** Computación Paralela  
**Práctica No:** 1.3  
**Nombre de la Actividad:** Reduce en MPI con OpenMP  

**Integrantes del Equipo (Orden alfabético):**  
1. Anguiano Garcia Angel Yahir Guadalupe  
2. Figueroa Robles Axel Israel  

**Fecha:** 4 de Octubre de 2026  

---

## 1. Introducción y Objetivo
El objetivo de esta práctica es implementar el modelo de memoria distribuida utilizando **MPI** para realizar operaciones aritméticas y reducciones colectivas sobre arreglos grandes en paralelo, aprovechando al mismo tiempo la API de **OpenMP** para explotar los núcleos multihilo disponibles dentro de cada nodo en un esquema maestro-esclavo. Adicionalmente, se garantiza la auditabilidad del proceso mediante la persistencia de datos en archivos de registro (log) independientes.

## 2. Explicación Técnica y Desarrollo

El programa fue desarrollado en C++ utilizando memoria dinámica mediante punteros (`new` y `delete`). La lógica principal está encapsulada en la clase `OperacionesArreglos`.

### 2.1 Paralelismo Híbrido (MPI + OpenMP)
El sistema opera en dos niveles de paralelismo:
- **Nivel 1 (OpenMP):** Dentro de cada proceso de MPI, se utiliza la directiva `#pragma omp parallel for` para paralelizar los ciclos for encargados de realizar cálculos matemáticos. Para operaciones acumulativas (como sumatorias o búsqueda de mínimos), se hace uso de la cláusula `reduction(+:suma)` o `reduction(min:min_local)`, garantizando acceso seguro a memoria compartida.
- **Nivel 2 (MPI):** El nodo maestro orquesta la aplicación. Cada esclavo realiza su cálculo local. Para las operaciones de reducción, se implementaron **MPI_Reduce** (para obtener la sumatoria y el máximo local y dejarlos en el maestro) y **MPI_Allreduce** (para el promedio y el mínimo global, donde todos los nodos reciben el resultado).

### 2.2 Diagrama de Bloques / Flujo Computacional

```text
[Inicio: MPI_Init]
       |
[Master (Rank 0) Bcast de Tamaño a Esclavos]
       |
[Asignación de Memoria Dinámica (A_local, B_local, C_local)]
       |
[Menú Iterativo (Bcast de opción elegida a esclavos)]
       |
      / \
    /     \
[Op. Elemento a Elemento]       [Operaciones de Reducción]
Suma, Resta, Multiplicación     Sumatoria, Promedio, Min, Max
       |                                |
[#pragma omp parallel for]      [#omp for reduction(...) -> Nivel 1]
       |                                |
[Se guarda localmente en C]     [MPI_Reduce / MPI_Allreduce -> Nivel 2]
       |                                |
[MPI_Gather al Maestro]         [Resultado Concentrado/Distribuido]
       |
[Escritura Síncrona en archivo Log de Nodo (.txt)]
       |
[Fin: MPI_Finalize]
```

## 3. Código Fuente Destacado

**Cláusula de reducción de OpenMP (Ejemplo Sumatoria Local):**
```cpp
long long OperacionesArreglos::sumatoriaLocal() {
    long long suma = 0;
    #pragma omp parallel for reduction(+:suma)
    for (long long i = 0; i < size_local; ++i) {
        suma += A_local[i];
    }
    return suma;
}
```

**Uso de Comunicaciones Colectivas MPI (Nivel 2):**
```cpp
// MPI_Reduce: Concentra el resultado únicamente en Rank 0
MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

// MPI_Allreduce: Reparte el resultado global a todos los Ranks
MPI_Allreduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
```

## 4. Tabla Comparativa de Tiempos y Enfoques de Comunicación

A continuación se muestra el análisis conceptual y de tiempo al escalar las metodologías de comunicación vistas a lo largo del curso para **4,000,000 de elementos**:

| Versión MPI | Método utilizado | Rendimiento y Complejidad | Tiempo aprox (4M elementos) |
|-------------|-------------------|---------------------------|-----------------------------|
| **V1 (Punto a Punto)** | `MPI_Send` / `MPI_Recv` | Alta latencia, código complejo por bucles de envío manuales. Propenso a cuellos de botella. | *(Insertar tu tiempo aquí, ej. 0.08 s)* |
| **V2 (Colectiva de Datos)**| `MPI_Scatter` / `MPI_Gather` | Optimizado por la topología de red de MPI. Código mucho más limpio. | *(Insertar tu tiempo aquí, ej. 0.03 s)* |
| **V3 (Híbrida con Reduce)**| `MPI_Reduce` / `MPI_Allreduce` | La más eficiente. Combina cálculos en el camino (hardware/switch) eliminando la necesidad de recibir arreglos gigantes para luego operarlos. | *(Insertar tu tiempo aquí, ej. 0.005 s)* |

*(Nota para el estudiante: Llena la tabla sustituyendo los textos entre paréntesis con los tiempos que obtuviste en consola al correr la opción 8 y 2 con 4,000,000).*

## 5. Evidencias y Capturas

*(Nota: En tu documento de Word, borra estos textos y pega las imágenes correspondientes debajo de cada punto)*

### 5.1. Ejecución Local (40 Elementos)
**[PEGA AQUÍ LA CAPTURA DE PANTALLA]**  
*Descripción:* Se observa la ejecución en consola del menú interactivo mostrando explícitamente los mensajes con la IP/Nombre del equipo, el Rank de MPI y el Hilo de OpenMP.

### 5.2. Persistencia en Archivos (Logs)
**[PEGA AQUÍ LA CAPTURA DE PANTALLA]**  
*Descripción:* Se muestran los archivos de texto (ej. `log_equipo_TUCOMPU_nodo_0.txt`) creados de manera independiente por cada nodo, capturando de manera persistente las impresiones.

### 5.3. Comparativa de Tiempos (4,000,000 Elementos)
**[PEGA AQUÍ LA CAPTURA DE PANTALLA]**  
*Descripción:* Ejecución con tiempos calculados con `MPI_Wtime()`, demostrando procesamiento de millones de elementos en milisegundos gracias al paralelismo híbrido.

### 5.4. Ejecución Distribuida (Opcional si se conectaron a otras PCs)
**[PEGA AQUÍ LA CAPTURA DE PANTALLA]**  
*Descripción:* Captura del comando `mpirun --hostfile` y la ejecución en red mostrando los nombres de diferentes equipos participando en la reducción de datos.

## 6. Conclusión
La implementación de memoria distribuida junto a memoria compartida (OpenMP) demostró una reducción drástica de tiempos de cómputo en grandes volúmenes de información. Mientras que las comunicaciones Punto a Punto (Send/Recv) saturaban la red en prácticas pasadas, el uso de funciones colectivas nativas de reducción (`MPI_Reduce` y `MPI_Allreduce`) permitió que los propios hilos calcularan datos parciales en cada máquina y la red solo transfiriera bytes simples, haciendo al algoritmo inmensamente escalable.

---
**Enlace al repositorio público:**
*(Coloca aquí la liga a tu repositorio de GitHub, ej. https://github.com/TUGITHUB/NavarroParalela)*

**Enlace al video descriptivo:**
*(Coloca aquí tu enlace de YouTube)*
