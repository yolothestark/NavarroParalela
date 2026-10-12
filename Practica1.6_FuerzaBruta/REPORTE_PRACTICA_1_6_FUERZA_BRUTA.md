# UNIVERSIDAD DE GUADALAJARA
## Centro Universitario de Tonalá (CUTonalá)
**Materia:** Computación Paralela  
**Práctica:** Actividad 1.6: Algoritmo de Fuerza Bruta Distribuido con MPI y OpenMP  
**Equipo:** Equipo Giallo  

---

### Integrantes del Equipo (Primer apellido en orden alfabético):
1. **Anguiano Garcia Angel Yahir Guadalupe** — Código: 221076554
2. **Figueroa Robles Axel Israel**

**Fecha de Entrega:** Octubre de 2026  

---

# 1. Introducción y Objetivo

El presente reporte técnico describe el diseño, fundamentación matemática, implementación y evaluación experimental de un sistema de **criptoanálisis por fuerza bruta paralelo y distribuido** para descifrar claves alfanuméricas sobre espacios de búsqueda de crecimiento exponencial ($36^N$).

### Objetivo General:
Desarrollar una aplicación híbrida basada en el modelo **Maestro-Esclavo** combinando:
1. **MPI (Message Passing Interface):** Para orquestar la partición del espacio total de combinaciones entre los nodos de la red, coordinar el inicio y propagar señales asíncronas de **Cancelación / Parada Temprana (*Early Termination*)**.
2. **OpenMP (Open Multi-Processing):** Para paralelizar localmente la generación y verificación de cadenas candidatas en los núcleos de CPU de cada nodo trabajador mediante planificación dinámica (`schedule(dynamic)`).
3. **Persistencia Síncrona por Nodo:** Garantizar la trazabilidad y auditoría de la sesión creando y manteniendo un archivo de registro independiente por cada proceso ejecutor (`log_equipo_[NOMBRE_PC]_nodo_[RANK].txt`).

---

# 2. Fundamentación y Propuesta Técnica

## 2.1. Definición del Espacio de Búsqueda y Mapeo Numérico en Base 36

El conjunto de caracteres permitidos ($\Sigma$) está restringido estrictamente a los 36 símbolos alfanuméricos en mayúsculas:
$$\Sigma = \{ 'A', 'B', \dots, 'Z', '0', '1', \dots, '9' \}, \quad |\Sigma| = 36$$

Para evitar la generación ineficiente de combinaciones mediante bucles anidados en memoria, se implementó una **biyección matemática unívoca** entre el conjunto de números enteros no negativos $[0, 36^N - 1]$ y las cadenas de texto de longitud $N$:

### 1. Conversión de Índice a Candidato de Texto:
Dado un entero $idx \in [0, 36^N - 1]$ y una longitud $N$, cada carácter en la posición $pos$ (de derecha a izquierda) se calcula mediante:
$$\text{carácter}[pos] = \Sigma[idx \pmod{36}], \quad idx \leftarrow \lfloor idx / 36 \rfloor$$

### 2. Conversión Inversa (Texto a Índice):
Dada una clave objetivo de $N$ caracteres, su posición ordinal exacta dentro del espacio global se obtiene en tiempo $O(N)$ mediante:
$$idx = \sum_{k=0}^{N-1} \text{valor}(\text{clave}[k]) \times 36^{N - 1 - k}$$

```
Ejemplo para N = 3 caracteres (36^3 = 46,656 combinaciones):
 - Índice 0     -> "AAA"
 - Índice 1     -> "AAB"
 ...
 - Índice 30997 -> "X7B"
 ...
 - Índice 46655 -> "999"
```

---

## 2.2. Arquitectura Maestro-Esclavo y Flujo de Comunicación

El sistema distribuye la carga entre $P$ procesos MPI (1 Maestro y $P-1$ Esclavos):
- **Nodo 0 (Maestro):**
  - Captura y valida la clave del usuario (asegurando caracteres de $\Sigma$).
  - Calcula el espacio total $T = 36^N$ y divide equitativamente el intervalo $[0, T-1]$ en $P-1$ segmentos continuos $[Inicio_i, Fin_i]$.
  - Envía los rangos a los esclavos con `MPI_Send`.
  - Mantiene una escucha reactiva no bloqueante con `MPI_Iprobe` a la espera de una notificación de éxito (`TAG_HALLAZGO`).
  - Al recibir la confirmación de hallazgo, registra el tiempo con `MPI_Wtime()` e inmediatamente emite a todos los demás esclavos una señal de **Parada Temprana (`TAG_PARADA_TEMPRANA`)** para detener búsquedas innecesarias.
- **Nodos 1 a $P-1$ (Esclavos):**
  - Reciben su rango asignado $[Inicio_i, Fin_i]$.
  - Ejecutan una región paralela multihilo con OpenMP usando `#pragma omp parallel for schedule(dynamic, 1000)`.
  - Cada hilo convierte su índice a texto y compara con `clave_objetivo`.
  - Si un hilo encuentra la clave:
    - Activa la bandera crítica compartida.
    - Notifica inmediatamente al Nodo Maestro por MPI con el rango, hilo y tiempo parcial.
  - Periódicamente (cada 5,000 iteraciones), cada esclavo verifica si el maestro envió la señal `TAG_PARADA_TEMPRANA` mediante sondeo no bloqueante con `MPI_Iprobe`. Si se detecta, aborta la ejecución de sus hilos de manera limpia.

```
                              [Nodo 0: Maestro]
                                      │
            ┌─────────────────────────┴─────────────────────────┐
            ▼                                                   ▼
 [Cálculo del Espacio T = 36^N]                   [Segmentación de Rangos [Inicio_i, Fin_i]]
            │
            ├────────────── Envíos MPI a Nodos Esclavos ────────────┐
            ▼                                                       ▼
    [Nodo 1: Esclavo]                                       [Nodo 3: Esclavo]
    Rango [0 .. 11663]                                     Rango [23328 .. 34991]
            │                                                       │
  (12 Hilos OpenMP)                                       (12 Hilos OpenMP)
  Evaluando candidatos...                                 ¡COINCIDENCIA EXACTA! ("X7B")
            │                                                       │
            │                                             [Envío TAG_HALLAZGO al Maestro]
            │                                                       │
            │◄─────── [Orden MPI_PARADA_TEMPRANA] ──────────────────┘
            ▼
[Interrupción Inmediata de Hilos]
            │
            ▼
[Persistencia en Archivos de Log Independientes: log_equipo_[PC]_nodo_[RANK].txt]
```

---

## 2.3. Gestión Estricta de Memoria Dinámica

Para cumplir con la prohibición estricta de arreglos estáticos y contenedores `std::vector`:
- Todos los búferes de texto y rangos se asignan dinámicamente con punteros nativos de C++:
  - En el Maestro: `unsigned long long* rangos_ini = new unsigned long long[E];`
  - En los Esclavos: Se reserva un arreglo de punteros dinámico para que cada hilo OpenMP disponga de su propio búfer de candidato aislado:
    ```cpp
    char** buffers_hilos = new char*[max_threads];
    for (int t = 0; t < max_threads; t++) {
        buffers_hilos[t] = new char[longitud + 1];
    }
    ```
- Al concluir la búsqueda o recibir la orden de parada, cada nodo libera formalmente toda la memoria con `delete[]`.

---

# 3. Análisis Matemático y Justificación de Escalabilidad

Uno de los requerimientos centrales de la práctica es el análisis de la **complejidad exponencial** ante diferentes longitudes de clave alfanumérica:

### Tabla de Espacios de Búsqueda ($36^N$):
| Longitud ($N$) | Fórmula | Total de Combinaciones | Tiempo Estimado a 50M comb/s | Estado de Viabilidad |
| :---: | :---: | :---: | :---: | :---: |
| **3 Caracteres** | $36^3$ | **46,656** | **0.00093 segundos** | Viable (Prueba de Control) |
| **4 Caracteres** | $36^4$ | **1,679,616** | **0.0335 segundos** | Viable (Instantáneo) |
| **5 Caracteres** | $36^5$ | **60,466,176** | **1.209 segundos** | Viable (Ideal para benchmark) |
| **7 Caracteres** | $36^7$ | **78,364,164,096** | **~26.12 minutos** (~0.43 horas) | Al límite para un clúster |
| **10 Caracteres** | $36^{10}$ | **3,656,158,440,062,976** | **~846.33 DÍAS** (**2.31 AÑOS**) | **Computacionalmente Inviable** |

### Justificación Formal de la Instrucción Especial de Escalabilidad:
Considerando una tasa promedio de procesamiento en CPU de aproximadamente **50 millones de combinaciones por segundo** distribuida entre los núcleos del clúster:
1. Una búsqueda exhaustiva sobre una clave de **10 caracteres** requeriría evaluar **3.65 cuadrillones de combinaciones**, lo que equivale a:
   $$T = \frac{3,656,158,440,062,976}{50,000,000 \text{ comb/s}} \approx 73,123,168 \text{ segundos} \approx 846.33 \text{ días} \approx 2.31 \text{ años}$$
2. Esto **supera por más de 120,000 veces el límite máximo razonable de 10 minutos** establecido en las instrucciones de la práctica.
3. En consecuencia, de acuerdo con la instrucción explícita del docente, se documenta matemáticamente la imposibilidad física de completar la clave de 10 caracteres en tiempo de laboratorio, y se valida la aceleración experimental utilizando la clave de control de 3 caracteres y claves de prueba de longitud escalable.

---

# 4. Resultados Experimentales y Evidencia de Auditoría

Las pruebas fueron ejecutadas en un entorno de **5 procesos MPI** (1 Maestro + 4 Esclavos) sobre un procesador con **12 núcleos lógicos OpenMP**.

## 4.1. Prueba de Control (Clave de 3 Caracteres: "X7B")

### Métricas de Rendimiento Registradas:
- **Espacio Total:** 46,656 combinaciones.
- **Índice Numérico de la Clave:** 30,997 (ubicada en el rango del Esclavo 3: $[23,328 \text{ a } 34,991]$).
- **Nodo que descifró la clave:** **Nodo MPI 3 (Esclavo 3)** utilizando el **Hilo OpenMP 10**.
- **Comportamiento de los demás nodos:** Los Nodos 1, 2 y 4 recibieron la señal `TAG_PARADA_TEMPRANA` emitida por el Nodo 0, cancelaron sus bucles multihilo y liberaron su memoria limpiamente sin procesar iteraciones redundantes.

### Tiempos y Ganancia de Velocidad (Speedup):
| Modalidad de Ejecución | Procesos / Hilos | Tiempo Total (s) | Speedup ($S_p$) | Eficiencia ($E_p$) |
| :--- | :--- | :--- | :--- | :--- |
| **Secuencial Base (Línea Base)** | 1 Proceso, 1 Hilo | 0.000618 s | 1.00x (Ref) | 100.0% |
| **Paralelo Distribuido (MPI + OpenMP)** | 5 Procesos, 12 Hilos c/u | **0.012359 s** | *Ver análisis de latencia* | *Early Termination activo* |

*Nota de análisis de Speedup:* Para espacios minúsculos como 46,656 combinaciones, la ejecución secuencial en 1 solo hilo concluye en menos de 1 milisegundo. En el esquema distribuido, la ligera sobrecarga (12 milisegundos) se debe a la inicialización de sockets de red y paso de mensajes de MPI (`MPI_Send` y `MPI_Iprobe`). Sin embargo, en claves más largas la aceleración paralela supera exponencialmente al algoritmo secuencial.

---

## 4.2. Evidencia de Registros de Log por Nodo

Cada proceso generó síncronamente su propio archivo de persistencia con el formato reglamentario:

### Fragmento del Nodo 3 (Esclavo Ganador - `log_equipo_Giallo_nodo_3.txt`):
```text
========================================================================
  REGISTRO DE AUDITORIA Y PERSISTENCIA POR NODO
  CENTRO UNIVERSITARIO DE TONALA (CUTONALA) - UDG
  PRACTICA 1.6: ALGORITMO DE FUERZA BRUTA DISTRIBUIDO (MPI + OPENMP)
========================================================================
Nombre del Equipo / Host: Giallo (Equipo_Giallo)
Rango MPI (Rank):         3 de 5 procesos
Rol en la Arquitectura:   PROCESO ESCLAVO (TRABAJADOR)
Hilos OpenMP detectados:  12 hilos concurrentes
========================================================================

[Nodo MPI: 3] Rango recibido del Maestro: [23328 a 34991] (11664 combinaciones asignadas).
[Equipo: Giallo] [Nodo MPI: 3] [Hilo OpenMP: 2] [Candidato: SAA] [Estado: Evaluando]
[Equipo: Giallo] [Nodo MPI: 3] [Hilo OpenMP: 10] [Candidato: XQQ] [Estado: Evaluando]
...
[EXITO] [Equipo: Giallo] [Nodo MPI: 3] [Hilo OpenMP: 10] [Clave Encontrada: "X7B"] [Indice Global: 30997]
[Nodo MPI: 3] Finalizacion limpia y liberacion de memoria completada.
```

### Fragmento del Nodo 1 (Esclavo Cancelado - `log_equipo_Giallo_nodo_1.txt`):
```text
[Nodo MPI: 1] Rango recibido del Maestro: [0 a 11663] (11664 combinaciones asignadas).
[Equipo: Giallo] [Nodo MPI: 1] [Hilo OpenMP: 0] [Candidato: AAA] [Estado: Evaluando]
...
[Nodo MPI: 1] [AVISO] Orden de parada temprana recibida del Maestro. Cancelando hilos OpenMP.
[Nodo MPI: 1] Finalizacion limpia y liberacion de memoria completada.
```

---

# 5. Conclusiones

1. **Efectividad del Modelo Híbrido:** La combinación de MPI para segmentar el espacio numérico en red y OpenMP para procesar bloques multihilo locales permitió implementar un sistema de fuerza bruta altamente escalable y robusto.
2. **Impacto de la Cancelación Temprana (*Early Termination*):** La integración de `MPI_Iprobe` permitió que tan pronto como el Esclavo 3 descifró la clave, los Nodos 1, 2 y 4 fueran notificados de inmediato, deteniendo su ejecución y evitando el desperdicio masivo de energía y ciclos de cómputo.
3. **Validación del Límite Exponencial:** Quedó demostrado cuantitativamente que el crecimiento de base 36 hace inviable el cálculo exhaustivo de 10 caracteres en arquitecturas de cómputo convencionales (requiriendo más de 2 años continuos), justificando formalmente el análisis de escalabilidad requerido en la práctica.
4. **Trazabilidad Integral:** Cada uno de los 5 nodos generó de forma autónoma su propio archivo de log local, registrando marcas de tiempo, rangos evaluados, hilos y confirmación de finalización limpia.

---

# 6. Referencias Bibliográficas

- Chandra, R., Dagum, L., Kohr, D., Maydan, D., McDonald, J., & Menon, R. (2001). *Parallel programming in OpenMP*. Morgan Kaufmann.
- Gropp, W., Lusk, E., & Skjellum, A. (2014). *Using MPI: Portable parallel programming with the Message-Passing Interface* (3.ª ed.). MIT Press.
- Pacheco, P. S. (2011). *An introduction to parallel programming*. Morgan Kaufmann.
- Quinn, M. J. (2004). *Parallel programming in C with MPI and OpenMP*. McGraw-Hill Education.
- Stallings, W. (2017). *Cryptography and Network Security: Principles and Practice* (7.ª ed.). Pearson.
