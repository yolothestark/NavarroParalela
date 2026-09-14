# Actividad 2.5: Ordenamientos con OpenMP

## Integrantes
- Anguiano Garcia Angel Yahir Guadalupe
- Figueroa Robles Axel Israel

## Algoritmos seleccionados
- **Iterativo:** Counting Sort (Secuencial y Paralelo)
- **Recursivo:** Merge Sort (Secuencial y Paralelo con Tareas)

## Directivas OpenMP utilizadas
- #pragma omp parallel y #pragma omp parallel for: Para crear equipos de hilos y distribuir las iteraciones de bucles (ej. llenado del arreglo y conteo paralelo).
- #pragma omp single: Para inicializar tareas y evitar impresión duplicada en pantalla.
- #pragma omp atomic: Para incrementar de manera segura y sincronizada los espacios de memoria compartida durante el Counting Sort.
- #pragma omp task y #pragma omp taskwait: Para implementar paralelismo basado en tareas en el algoritmo recursivo Merge Sort, dividiendo y unificando el árbol de ejecución de forma segura.

## Instrucciones para compilar y ejecutar
1. Asegúrese de tener instalado el compilador de C++ (g++) compatible con OpenMP.
2. Abra la terminal en el directorio del proyecto y ejecute el siguiente comando para compilar:
   \\\ash
   g++ main.cpp -fopenmp -o ordenamientos
   \\\
3. Ejecute el binario generado:
   - En Windows: .\ordenamientos.exe
   - En Linux/Mac: ./ordenamientos
4. El menú interactivo le solicitará el número de elementos y el rango máximo para comenzar las pruebas.
