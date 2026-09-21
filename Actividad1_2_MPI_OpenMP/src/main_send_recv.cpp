#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "OperacionesArreglos.h"

// Función para imprimir los nombres de los integrantes al inicio y fin
void imprimirIntegrantes() {
    std::cout << "\n======================================================================\n";
    std::cout << "                  DATOS DE LOS INTEGRANTES DEL EQUIPO                 \n";
    std::cout << "            (Iniciando por Primer Apellido en Orden Alfabético)        \n";
    std::cout << "======================================================================\n";
    std::cout << " 1. ANGUIANO GARCIA, ANGEL YAHIR GUADALUPE\n";
    std::cout << " 2. FIGUEROA ROBLES, AXEL ISRAEL\n";
    std::cout << "======================================================================\n\n";
}

void imprimirArregloResumen(const double* arr, int N, const char* nombre) {
    std::cout << "\n>>> Arreglo " << nombre << " (" << N << " elementos):\n[ ";
    if (N <= 40) {
        for (int i = 0; i < N; i++) {
            std::cout << arr[i] << (i < N - 1 ? ", " : " ");
        }
    } else {
        for (int i = 0; i < 5; i++) std::cout << arr[i] << ", ";
        std::cout << "... ";
        for (int i = N - 5; i < N; i++) std::cout << arr[i] << (i < N - 1 ? ", " : " ");
    }
    std::cout << "]\n\n";
}

int main(int argc, char** argv) {
    // Inicialización de MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char hostName[MPI_MAX_PROCESSOR_NAME];
    int nameLen;
    MPI_Get_processor_name(hostName, &nameLen);

    // Primera instrucción de impresión de integrantes por el Proceso Maestro (Rank 0)
    if (rank == 0) {
        imprimirIntegrantes();
        std::cout << ">>> INICIALIZANDO SISTEMA: Versión MPI_Send / MPI_Recv <<<\n";
        std::cout << "[Equipo Maestro: " << hostName << "] [Proceso MPI: " << rank 
                  << "] Total de procesos MPI activos: " << size << "\n";
        if (size < 2) {
            std::cerr << "ADVERTENCIA: Se recomiendan 5 procesos (1 Maestro + 4 Trabajadores).\n";
        }
    }

    int numWorkers = (size > 1) ? (size - 1) : 1;
    OperacionesArreglos op;

    // Configuración por defecto: 40 elementos (modo detallado)
    int N = 40;
    bool verbose = true;
    int genMode = 0; // 0 = Consecutivo, 1 = Aleatorio 1..1M

    double* A = nullptr;
    double* B = nullptr;
    double* C = nullptr;

    int localN = N / numWorkers;
    double* local_A = new double[localN];
    double* local_B = new double[localN];
    double* local_C = new double[localN];

    if (rank == 0) {
        A = new double[N];
        B = new double[N];
        C = new double[N];
        // Inicializar arreglos base
        for (int i = 0; i < N; i++) {
            A[i] = (double)(i + 1);
            B[i] = (double)((i + 1) * 2);
            C[i] = 0.0;
        }
    }

    int opcion = 0;

    do {
        if (rank == 0) {
            std::cout << "\n==================================================\n";
            std::cout << "        MENÚ DE OPERACIONES CON ARREGLOS         \n";
            std::cout << "       (Implementación: MPI_Send / MPI_Recv)      \n";
            std::cout << "   Tamaño actual: " << N << " elementos | Modo: " 
                      << (verbose ? "Detallado (40 elem)" : "Rendimiento (4M elem)") << "\n";
            std::cout << "==================================================\n";
            std::cout << " 1. Crear / Inicializar arreglos (A y B)\n";
            std::cout << " 2. Sumar arreglos (A + B)\n";
            std::cout << " 3. Restar arreglos (A - B)\n";
            std::cout << " 4. Multiplicar arreglos (A * B)\n";
            std::cout << " 5. Calcular cuadrado del arreglo (A^2)\n";
            std::cout << " 6. Cambiar tamaño (40 Detallado <-> 4,000,000 Rendimiento)\n";
            std::cout << " 7. Salir\n";
            std::cout << "Seleccione una opción: ";
            std::cin >> opcion;
        }

        // Difusión de la opción seleccionada a todos los procesos
        MPI_Bcast(&opcion, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (opcion == 6) {
            // Cambiar tamaño y modo
            if (rank == 0) {
                if (N == 40) {
                    N = 4000000;
                    verbose = false;
                    genMode = 1;
                    std::cout << "\n>>> Configurado a MODO RENDIMIENTO: N = 4,000,000 (Impresiones individuales desactivadas) <<<\n";
                } else {
                    N = 40;
                    verbose = true;
                    genMode = 0;
                    std::cout << "\n>>> Configurado a MODO DETALLADO: N = 40 (Impresiones completas por Hilo/Nodo) <<<\n";
                }
            }
            MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
            int vInt = verbose ? 1 : 0;
            MPI_Bcast(&vInt, 1, MPI_INT, 0, MPI_COMM_WORLD);
            verbose = (vInt == 1);
            MPI_Bcast(&genMode, 1, MPI_INT, 0, MPI_COMM_WORLD);

            // Reasignar memoria dinámica
            delete[] local_A;
            delete[] local_B;
            delete[] local_C;
            localN = N / numWorkers;
            local_A = new double[localN];
            local_B = new double[localN];
            local_C = new double[localN];

            if (rank == 0) {
                delete[] A;
                delete[] B;
                delete[] C;
                A = new double[N];
                B = new double[N];
                C = new double[N];
                // Llenar por defecto
                for (int i = 0; i < N; i++) {
                    A[i] = (double)(i + 1);
                    B[i] = (double)((i + 1) * 2);
                    C[i] = 0.0;
                }
            }
            continue;
        }

        if (opcion >= 1 && opcion <= 5) {
            double startTime = 0.0, endTime = 0.0;
            if (rank == 0) {
                startTime = MPI_Wtime();
            }

            if (opcion == 1) {
                // Crear arreglos distribuidos
                if (rank != 0) {
                    int globalOffset = (rank - 1) * localN;
                    op.crearArregloMPI(local_A, localN, globalOffset, hostName, rank, verbose, "A", genMode);
                    op.crearArregloMPI(local_B, localN, globalOffset, hostName, rank, verbose, "B", genMode);
                    // Enviar datos generados al maestro
                    MPI_Send(local_A, localN, MPI_DOUBLE, 0, 10, MPI_COMM_WORLD);
                    MPI_Send(local_B, localN, MPI_DOUBLE, 0, 20, MPI_COMM_WORLD);
                } else {
                    // Maestro recibe los datos generados por cada trabajador
                    for (int w = 1; w <= numWorkers; w++) {
                        int offset = (w - 1) * localN;
                        MPI_Recv(&A[offset], localN, MPI_DOUBLE, w, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        MPI_Recv(&B[offset], localN, MPI_DOUBLE, w, 20, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        if (verbose) {
                            std::cout << "[Equipo Maestro: " << hostName << "] [Proceso MPI: 0] "
                                      << "Recibida sección de arreglos A y B del Trabajador MPI: " << w << "\n";
                        }
                    }
                }
            } else {
                // Operaciones 2 (Suma), 3 (Resta), 4 (Multiplicación), 5 (Cuadrado)
                if (rank == 0) {
                    // Maestro distribuye secciones a los trabajadores usando MPI_Send
                    for (int w = 1; w <= numWorkers; w++) {
                        int offset = (w - 1) * localN;
                        MPI_Send(&A[offset], localN, MPI_DOUBLE, w, 100, MPI_COMM_WORLD);
                        if (opcion != 5) { // Para suma, resta y mult se envía también B
                            MPI_Send(&B[offset], localN, MPI_DOUBLE, w, 200, MPI_COMM_WORLD);
                        }
                        if (verbose) {
                            std::cout << "[Equipo Maestro: " << hostName << "] [Proceso MPI: 0] "
                                      << "Enviando sección (offset " << offset << ", tam " << localN 
                                      << ") al Trabajador MPI: " << w << "\n";
                        }
                    }
                } else {
                    // Trabajadores reciben su sección con MPI_Recv
                    int globalOffset = (rank - 1) * localN;
                    MPI_Recv(local_A, localN, MPI_DOUBLE, 0, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    if (opcion != 5) {
                        MPI_Recv(local_B, localN, MPI_DOUBLE, 0, 200, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }

                    // Ejecución de la operación correspondiente mediante OpenMP
                    switch (opcion) {
                        case 2: // Suma
                            op.sumar(local_A, local_B, local_C, localN, globalOffset, hostName, rank, verbose);
                            break;
                        case 3: // Resta
                            op.restar(local_A, local_B, local_C, localN, globalOffset, hostName, rank, verbose);
                            break;
                        case 4: // Multiplicación
                            op.multiplicar(local_A, local_B, local_C, localN, globalOffset, hostName, rank, verbose);
                            break;
                        case 5: // Cuadrado
                            op.cuadrado(local_A, local_C, localN, globalOffset, hostName, rank, verbose);
                            break;
                    }

                    // Trabajador envía resultado parcial al Maestro con MPI_Send
                    MPI_Send(local_C, localN, MPI_DOUBLE, 0, 300, MPI_COMM_WORLD);
                }

                if (rank == 0) {
                    // Maestro recibe resultados parciales con MPI_Recv y reconstruye el arreglo
                    for (int w = 1; w <= numWorkers; w++) {
                        int offset = (w - 1) * localN;
                        MPI_Recv(&C[offset], localN, MPI_DOUBLE, w, 300, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        if (verbose) {
                            std::cout << "[Equipo Maestro: " << hostName << "] [Proceso MPI: 0] "
                                      << "Recibido resultado parcial del Trabajador MPI: " << w << "\n";
                        }
                    }
                }
            }

            // Sincronización y medición de tiempo
            MPI_Barrier(MPI_COMM_WORLD);
            if (rank == 0) {
                endTime = MPI_Wtime();
                double elapsedTime = endTime - startTime;
                
                std::cout << "\n--------------------------------------------------\n";
                std::cout << ">>> Operación Completada Exitosamente <<<\n";
                std::cout << "Tiempo de ejecución (MPI_Wtime): " << std::fixed << std::setprecision(6) 
                          << elapsedTime << " segundos\n";
                std::cout << "--------------------------------------------------\n";

                if (opcion == 1) {
                    imprimirArregloResumen(A, N, "A");
                    imprimirArregloResumen(B, N, "B");
                } else {
                    imprimirArregloResumen(C, N, "Resultado C");
                }
            }
        }

    } while (opcion != 7);

    // Liberación de memoria dinámica
    delete[] local_A;
    delete[] local_B;
    delete[] local_C;

    if (rank == 0) {
        delete[] A;
        delete[] B;
        delete[] C;
        // Última instrucción de impresión de integrantes por el Proceso Maestro (Rank 0)
        imprimirIntegrantes();
        std::cout << ">>> Programa finalizado correctamente. Liberando recursos MPI. <<<\n";
    }

    MPI_Finalize();
    return 0;
}

