#include "OperacionesArreglos.h"
#include <omp.h>
#include <cstdlib>
#include <iomanip>

OperacionesArreglos::OperacionesArreglos() {}

OperacionesArreglos::~OperacionesArreglos() {}

void OperacionesArreglos::crearArregloMPI(double* arr, int localSize, int globalOffset, 
                                          const char* hostName, int mpiRank, bool verbose, 
                                          const char* arrayName, int genMode) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        unsigned int seed = (unsigned int)(mpiRank * 10007 + tid * 313 + 1);

        #pragma omp for schedule(static)
        for (int i = 0; i < localSize; i++) {
            int globalPos = globalOffset + i;
            if (genMode == 0) {
                // Modo consecutivo identificando nodo (para 40 elementos)
                arr[i] = (double)(globalPos + 1);
            } else {
                // Modo aleatorio entre 1 y 1,000,000 (para 4,000,000 elementos)
                seed = seed * 1103515245 + 12345;
                unsigned int randVal = (seed / 65536) % 1000000 + 1;
                arr[i] = (double)randVal;
            }

            if (verbose) {
                #pragma omp critical
                {
                    std::cout << "[Equipo: " << hostName 
                              << "] [Proceso MPI: " << mpiRank 
                              << "] [Hilo OpenMP: " << tid 
                              << "] [Posición: " << std::setw(2) << globalPos 
                              << "] [Valor generado: " << arrayName << "[" << globalPos << "] = " << arr[i] 
                              << "]" << std::endl;
                }
            }
        }
    }
}

void OperacionesArreglos::sumar(const double* a, const double* b, double* c, int localSize, 
                                int globalOffset, const char* hostName, int mpiRank, bool verbose) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        #pragma omp for schedule(static)
        for (int i = 0; i < localSize; i++) {
            c[i] = a[i] + b[i];
            int globalPos = globalOffset + i;

            if (verbose) {
                #pragma omp critical
                {
                    std::cout << "[Equipo: " << hostName 
                              << "] [Proceso MPI: " << mpiRank 
                              << "] [Hilo OpenMP: " << tid 
                              << "] [Posición: " << std::setw(2) << globalPos 
                              << "] [Operación: Suma -> " << a[i] << " + " << b[i] << " = " << c[i] 
                              << "]" << std::endl;
                }
            }
        }
    }
}

void OperacionesArreglos::restar(const double* a, const double* b, double* c, int localSize, 
                                 int globalOffset, const char* hostName, int mpiRank, bool verbose) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        #pragma omp for schedule(static)
        for (int i = 0; i < localSize; i++) {
            c[i] = a[i] - b[i];
            int globalPos = globalOffset + i;

            if (verbose) {
                #pragma omp critical
                {
                    std::cout << "[Equipo: " << hostName 
                              << "] [Proceso MPI: " << mpiRank 
                              << "] [Hilo OpenMP: " << tid 
                              << "] [Posición: " << std::setw(2) << globalPos 
                              << "] [Operación: Resta -> " << a[i] << " - " << b[i] << " = " << c[i] 
                              << "]" << std::endl;
                }
            }
        }
    }
}

void OperacionesArreglos::multiplicar(const double* a, const double* b, double* c, int localSize, 
                                      int globalOffset, const char* hostName, int mpiRank, bool verbose) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        #pragma omp for schedule(static)
        for (int i = 0; i < localSize; i++) {
            c[i] = a[i] * b[i];
            int globalPos = globalOffset + i;

            if (verbose) {
                #pragma omp critical
                {
                    std::cout << "[Equipo: " << hostName 
                              << "] [Proceso MPI: " << mpiRank 
                              << "] [Hilo OpenMP: " << tid 
                              << "] [Posición: " << std::setw(2) << globalPos 
                              << "] [Operación: Multiplicación -> " << a[i] << " * " << b[i] << " = " << c[i] 
                              << "]" << std::endl;
                }
            }
        }
    }
}

void OperacionesArreglos::cuadrado(const double* a, double* c, int localSize, 
                                   int globalOffset, const char* hostName, int mpiRank, bool verbose) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        #pragma omp for schedule(static)
        for (int i = 0; i < localSize; i++) {
            c[i] = a[i] * a[i];
            int globalPos = globalOffset + i;

            if (verbose) {
                #pragma omp critical
                {
                    std::cout << "[Equipo: " << hostName 
                              << "] [Proceso MPI: " << mpiRank 
                              << "] [Hilo OpenMP: " << tid 
                              << "] [Posición: " << std::setw(2) << globalPos 
                              << "] [Operación: Cuadrado -> (" << a[i] << ")^2 = " << c[i] 
                              << "]" << std::endl;
                }
            }
        }
    }
}
