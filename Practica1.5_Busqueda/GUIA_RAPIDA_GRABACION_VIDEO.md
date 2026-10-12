# 🎬 Guion Paso a Paso para tu Video (Nivel Principiante - Directo al Grano)

**Duración aproximada:** 3 a 4 minutos.  
**Recomendación:** Ten abierto el Bloc de Notas o esta guía en una esquinita para ir leyendo lo que tienes que decir.

---

## 🟢 PASO 1: Inicio y Presentación (30 segundos)

🖥️ **QUÉ MOSTRAR EN PANTALLA:**  
Abre la carpeta del proyecto en el explorador de archivos de Windows:  
`C:\Users\Yahir\OneDrive\Documentos\GitHub\NavarroParalela\Practica1.5_Busqueda\`

🗣️ **QUÉ DECIR (Lee tal cual):**  
> *"Hola, buenas tardes, profesor. Mi nombre es Angel Yahir Guadalupe Anguiano Garcia y junto con mi compañero Axel Israel Figueroa Robles, pertenecientes al Centro Universitario de Tonalá (CUTonalá), presentamos la Actividad 1.5: Búsqueda Paralela Distribuida con MPI y OpenMP.  
> En esta práctica combinamos MPI para repartir el trabajo entre 5 procesos y OpenMP para aprovechar los núcleos multihilo de cada equipo, usando exclusivamente memoria dinámica con punteros en C++."*

---

## 🟢 PASO 2: Vistazo rápido al código (45 segundos)

🖥️ **QUÉ MOSTRAR EN PANTALLA:**  
Abre el archivo `BusquedaParalela.h` o `BusquedaParalela.cpp` en tu editor (VS Code, Code::Blocks o Bloc de Notas).

🗣️ **QUÉ DECIR:**  
> *"Primero muestro rápidamente el código:  
> 1. Aquí se observa que no usamos vectores ni arreglos fijos; toda la memoria se reserva con `new` y se libera con `delete[]`.  
> 2. En `BusquedaParalela.cpp` tenemos el ordenamiento Merge Sort con tareas de OpenMP, que es el requisito para poder hacer búsqueda binaria.  
> 3. Y tenemos las funciones de búsqueda secuencial y búsqueda binaria distribuida, donde cada nodo revisa si el dato cae en su bloque antes de buscar."*

*(Cierras o minimizas el editor).*

---

## 🟢 PASO 3: Ejecución del Programa (1 minuto y medio)

🖥️ **QUÉ MOSTRAR EN PANTALLA:**  
Haz doble clic en el archivo: **`EJECUTAR_DEMO_LOCAL.bat`**  
*(Se abrirá la consola en color azul/verde).*  
Presiona cualquier tecla para iniciar.

🗣️ **QUÉ DECIR:**  
> *"Como se solicita en las instrucciones, al iniciar el programa lo primero que se imprime en pantalla son los nombres oficiales del equipo en orden alfabético."*

---

### 👉 En la consola, presiona el número `5` y dale Enter:

🗣️ **QUÉ DECIR (mientras ves pasar los datos de 100 elementos):**  
> *"Aquí selecciono la opción 5 para correr las pruebas automáticas.  
> Con el arreglo de control de 100 elementos vemos cómo trabaja cada nodo:  
> El Nodo 0 ordenó el arreglo con OpenMP.  
> Luego el Nodo 2 encontró el elemento en la posición 50 con su hilo de OpenMP.  
> Y algo muy importante: en la búsqueda binaria, los nodos 0, 1, 3 y 4 descartaron sus bloques al instante porque el valor estaba fuera de sus límites, ahorrando procesamiento."*

---

### 👉 Ahora señala la prueba masiva (10,000,000 de elementos):

🗣️ **QUÉ DECIR:**  
> *"En la prueba masiva con 10 millones de datos podemos ver el poder del paralelismo:  
> El ordenamiento secuencial tardó casi 2 segundos, mientras que con OpenMP bajó a solo 0.45 segundos, logrando una aceleración de más de 4 veces.  
> Y en la búsqueda, la binaria distribuida responde en fracciones de milisegundo."*

---

### 👉 En la consola, presiona el número `6` y dale Enter:

🗣️ **QUÉ DECIR:**  
> *"En la opción 6 podemos ver la tabla resumen consolidada con todos los tiempos y factores de Speedup."*

---

### 👉 En la consola, presiona el número `7` y dale Enter para salir:

🗣️ **QUÉ DECIR:**  
> *"Salimos del programa con la opción 7, y como pide la instrucción obligatoria, la última línea que se imprime vuelve a ser la de los integrantes del equipo."*

---

## 🟢 PASO 4: Mostrar los archivos de Log (45 segundos)

🖥️ **QUÉ MOSTRAR EN PANTALLA:**  
El script te preguntará en la consola:  
`Deseas abrir los 5 archivos de log en el Bloc de Notas para tu video? (S/N):`  
Escribe **`S`** y dale Enter (o haz doble clic a `VER_LOGS.bat`).  
*(Se abrirán los 5 blocs de notas en tu pantalla).*

🗣️ **QUÉ DECIR:**  
> *"Finalmente, muestro la evidencia de persistencia y auditoría. Cada uno de los 5 procesos MPI generó su propio archivo de log independiente:  
> Vemos el log del nodo 0, nodo 1, nodo 2, nodo 3 y nodo 4.  
> Cada archivo registra las especificaciones de la máquina, los hilos de OpenMP detectados, los límites del bloque asignado y el resultado de la búsqueda."*

---

## 🟢 PASO 5: Despedida (15 segundos)

🗣️ **QUÉ DECIR:**  
> *"Con esto demostramos el correcto funcionamiento del sistema de búsqueda paralela híbrido en memoria y auditoría. Muchas gracias por su atención."*

⏹️ **FIN DE LA GRABACIÓN.**
