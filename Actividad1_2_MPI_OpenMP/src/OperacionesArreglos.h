#ifndef OPERACIONES_ARREGLOS_H
#define OPERACIONES_ARREGLOS_H

#include <iostream>
#include <string>

class OperacionesArreglos {
public:
    OperacionesArreglos();
    ~OperacionesArreglos();

    // Generacion de valores en la seccion del arreglo asignada
    void crearArregloMPI(double* arr, int localSize, int globalOffset, 
                         const char* hostName, int mpiRank, bool verbose, 
                         const char* arrayName = "A", int genMode = 0);

    // Suma elemento a elemento: c[i] = a[i] + b[i]
    void sumar(const double* a, const double* b, double* c, int localSize, 
               int globalOffset, const char* hostName, int mpiRank, bool verbose);

    // Resta elemento a elemento: c[i] = a[i] - b[i]
    void restar(const double* a, const double* b, double* c, int localSize, 
                int globalOffset, const char* hostName, int mpiRank, bool verbose);

    // Multiplicacion elemento a elemento: c[i] = a[i] * b[i]
    void multiplicar(const double* a, const double* b, double* c, int localSize, 
                     int globalOffset, const char* hostName, int mpiRank, bool verbose);

    // Cuadrado de los elementos: c[i] = a[i] * a[i]
    void cuadrado(const double* a, double* c, int localSize, 
                  int globalOffset, const char* hostName, int mpiRank, bool verbose);
};

#endif // OPERACIONES_ARREGLOS_H
