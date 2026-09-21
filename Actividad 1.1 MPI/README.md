# Actividad 1.1: Modelo de programacion y memoria en MPI

## Integrantes
- Anguiano Garcia Angel Yahir Guadalupe
- Figueroa Robles Axel Israel

## Descripcion
Este proyecto implementa un modelo de programacion paralela hibrida utilizando **MPI** (Message Passing Interface) y **OpenMP**.
- **MPI:** Se utiliza para distribuir el arreglo dinamico total entre diferentes procesos y nodos de la red.
- **OpenMP:** Se encarga de paralelizar el procesamiento del llenado del sub-arreglo local dentro de cada nodo, aprovechando sus multiples nucleos.

## Clases y Memoria
El proyecto hace uso de una clase ProcesadorArreglo para encapsular la logica. Cumpliendo estrictamente con la rubrica, **no** se utiliza la clase vector. En su lugar, el almacenamiento se gestiona exclusivamente a traves de arreglos dinamicos (new int[] y delete[]).

## Instrucciones para compilar y ejecutar
1. **Prerrequisitos:** Asegurese de tener instalado un compilador de C++ con soporte OpenMP y una implementacion de MPI (MS-MPI en Windows, o OpenMPI/MPICH en Linux).
2. **Compilacion:**
   Abra una terminal en esta carpeta y ejecute (el comando puede variar segun su configuracion de MPI):
   mpicxx main.cpp -fopenmp -o hibrido
   *(En Windows con MS-MPI puede ser necesario enlazar las librerias manualmente o compilar a traves del IDE configurado)*
3. **Ejecucion Local (1 Maquina):**
   mpiexec -n 4 ./hibrido
4. **Ejecucion Distribuida (Red local - Ejemplo generico):**
   Cree un archivo hosts con las IPs de las 3 computadoras y ejecute:
   mpiexec -f hosts -n 6 ./hibrido
