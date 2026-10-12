# 🎬 Guion Paso a Paso para tu Video - Práctica 1.6 (Fuerza Bruta)

**Universidad de Guadalajara (UDG) - Centro Universitario de Tonalá (CUTonalá)**  
**Materia:** Computación Paralela  
**Equipo:** Anguiano Garcia Angel Yahir Guadalupe & Figueroa Robles Axel Israel  
**Duración aproximada:** 3 a 4 minutos.  

---

## 🟢 PASO 1: Inicio y Presentación (30 segundos)

🖥️ **QUÉ MOSTRAR EN PANTALLA:**  
Abre en el explorador de archivos la carpeta del proyecto:  
`C:\Users\Yahir\OneDrive\Documentos\GitHub\NavarroParalela\Practica1.6_FuerzaBruta\`

🗣️ **QUÉ DECIR (Léelo tal cual):**  
> *"Hola, buenas tardes, profesor. Mi nombre es Angel Yahir Guadalupe Anguiano Garcia y junto con mi compañero Axel Israel Figueroa Robles, del Centro Universitario de Tonalá (CUTonalá) de la Universidad de Guadalajara, presentamos la Práctica 1.6: Algoritmo de Fuerza Bruta Distribuido con MPI y OpenMP.*  
> 
> *En esta práctica desarrollamos un sistema bajo la arquitectura Maestro-Esclavo para descifrar contraseñas alfanuméricas de base 36, utilizando MPI para segmentar los rangos numéricos entre procesos y OpenMP con planificación dinámica en cada nodo, incorporando cancelación temprana (Early Termination) y gestión estricta de memoria dinámica con punteros en C++."*

---

## 🟢 PASO 2: Vistazo rápido al código (40 segundos)

🖥️ **QUÉ MOSTRAR EN PANTALLA:**  
Abre `FuerzaBruta.h` o `FuerzaBruta.cpp` en tu editor.

🗣️ **QUÉ DECIR:**  
> *"Muestro rápidamente los puntos clave de la implementación:*  
> 
> *1. El alfabeto se compone de 36 caracteres en mayúsculas: letras de la A a la Z y dígitos del 0 al 9.*  
> 
> *2. En lugar de usar bucles anidados en memoria, implementamos un mapeo biyectivo en base 36, donde cada combinación corresponde a un índice entero.*  
> 
> *3. El Nodo 0 actúa como Maestro dividiendo los intervalos equitativamente, mientras que los Nodos Esclavos paralelizan con OpenMP usando `#pragma omp parallel for schedule(dynamic)`.*  
> 
> *4. Y contamos con un mecanismo de parada temprana con `MPI_Iprobe`: en cuanto un hilo encuentra la clave exacta, notifica al Maestro y este propaga la orden de cancelación a los demás nodos para no desperdiciar ciclos de CPU."*

*(Minimiza o cierra la ventana del editor).*

---

## 🟢 PASO 3: Ejecución de la Demostración Local (1 minuto y medio)

🖥️ **QUÉ MOSTRAR EN PANTALLA:**  
Haz doble clic en: **`EJECUTAR_DEMO_LOCAL.bat`**  
*(Se abrirá la consola azul).*  
Presiona cualquier tecla para iniciar.

🗣️ **QUÉ DECIR:**  
> *"Como solicita la instrucción obligatoria, al iniciar el programa lo primero que se imprime en pantalla es el encabezado del CUTonalá y los nombres del equipo en orden alfabético por primer apellido."*

---

### 👉 Acción 1: En la consola escribe el número `4` y presiona Enter:

🗣️ **QUÉ DECIR (mientras se ven las trazas de la clave "X7B"):**  
> *"Selecciono la opción 4 para ejecutar la secuencia de pruebas y escalabilidad.*  
> 
> *En la Fase 1, evaluamos la clave de 3 caracteres 'X7B' con un espacio de 46,656 combinaciones.*  
> *Vemos cómo cada esclavo y sus hilos de OpenMP reportan su progreso:*  
> *- El Nodo 3 encontró la clave exacta en el índice 30,997 con su Hilo 10.*  
> *- E inmediatamente el Maestro emitió la orden de parada temprana, haciendo que los Nodos 1, 2 y 4 cancelaran sus hilos y finalizaran limpiamente."*

---

### 👉 Acción 2: Señala la Fase 2 (Análisis Matemático de Escalabilidad en pantalla):

🗣️ **QUÉ DECIR:**  
> *"En la Fase 2 mostramos la justificación matemática de por qué 10 caracteres es inviable:*  
> *- Con 3 caracteres son 46 mil combinaciones y tarda menos de un segundo.*  
> *- Con 7 caracteres son 78 mil millones de combinaciones y tomaría unos 26 minutos.*  
> *- Pero con 10 caracteres el espacio explota a 3.65 cuadrillones de combinaciones, lo que a una tasa de 50 millones por segundo tomaría más de 846 días o 2.3 años continuos.*  
> *Por ello, tal como autoriza la rúbrica, justificamos formalmente el límite de 10 caracteres y demostramos la aceleración en rangos medibles."*

---

### 👉 Acción 3: En la consola escribe el número `5` y presiona Enter:

🗣️ **QUÉ DECIR:**  
> *"En la opción 5 mostramos la tabla comparativa de tiempos y métricas de ganancia de velocidad."*

---

### 👉 Acción 4: En la consola escribe el número `6` y presiona Enter:

🗣️ **QUÉ DECIR:**  
> *"Salimos del sistema con la opción 6, y como pide la instrucción especial, la última línea que se imprime en pantalla vuelve a mostrar los nombres oficiales del equipo."*

---

## 🟢 PASO 4: Mostrar los archivos de Log en pantalla (40 segundos)

🖥️ **QUÉ MOSTRAR EN PANTALLA:**  
La consola te mostrará:  
`Presione cualquier tecla para abrir los 5 archivos de log en el Bloc de Notas...`  
**Presiona cualquier tecla**.  
*(Se abrirán los 5 blocs de notas en tu pantalla).*

🗣️ **QUÉ DECIR:**  
> *"Finalmente, mostramos la evidencia de persistencia por nodo.*  
> *Cada uno de los 5 procesos MPI generó de forma independiente su propio archivo de log:*  
> *- Aquí vemos el log del Nodo 0 (Maestro) coordinando y distribuyendo los rangos.*  
> *- Los logs de los Nodos 1, 2 y 4 registrando la orden de parada temprana.*  
> *- Y el log del Nodo 3 con el registro exacto del hallazgo con fecha, hilos y tiempo.*  
> *Todos los nodos cerraron limpiamente liberando su memoria."*

---

## 🟢 PASO 5: Despedida (15 segundos)

🗣️ **QUÉ DECIR:**  
> *"Con esto demostramos el correcto funcionamiento del algoritmo de fuerza bruta distribuido con cancelación temprana y persistencia. Muchas gracias por su atención."*

⏹️ **FIN DEL VIDEO.**
