#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "BusquedaParalela.h"

// Función auxiliar para imprimir el encabezado oficial del equipo
void imprimir_encabezado_equipo(int rank) {
    if (rank == 0) {
        std::cout << "\n========================================================================\n";
        std::cout << "  CENTRO UNIVERSITARIO DE TONALA (CUTONALA) - UDG\n";
        std::cout << "  MATERIA: COMPUTACION PARALELA\n";
        std::cout << "  ACTIVIDAD 1.5: BUSQUEDA PARALELA DISTRIBUIDA CON MPI Y OPENMP\n";
        std::cout << "------------------------------------------------------------------------\n";
        std::cout << "  INTEGRANTES DEL EQUIPO (PRIMER APELLIDO EN ORDEN ALFABETICO):\n";
        std::cout << "  1. ANGUIANO GARCIA ANGEL YAHIR GUADALUPE\n";
        std::cout << "  2. FIGUEROA ROBLES AXEL ISRAEL\n";
        std::cout << "========================================================================\n\n";
    }
}

// Función auxiliar para imprimir el pie oficial del equipo
void imprimir_pie_equipo(int rank) {
    if (rank == 0) {
        std::cout << "\n========================================================================\n";
        std::cout << "  FINALIZACION EXITOSA DE LA ACTIVIDAD 1.5\n";
        std::cout << "  INTEGRANTES DEL EQUIPO (PRIMER APELLIDO EN ORDEN ALFABETICO):\n";
        std::cout << "  1. ANGUIANO GARCIA ANGEL YAHIR GUADALUPE\n";
        std::cout << "  2. FIGUEROA ROBLES AXEL ISRAEL\n";
        std::cout << "========================================================================\n";
    }
}

void mostrar_menu(int rank) {
    if (rank == 0) {
        std::cout << "\n------------------------------------------------------------------------\n";
        std::cout << "                     MENU DE OPERACIONES Y PRUEBAS                      \n";
        std::cout << "------------------------------------------------------------------------\n";
        std::cout << " 1. Generar Arreglo Dinamico Aleatorio\n";
        std::cout << " 2. Ordenar Arreglo con OpenMP (Requisito para Busqueda Binaria)\n";
        std::cout << " 3. Ejecutar Busqueda Secuencial (Local OpenMP vs Distribuida MPI+OpenMP)\n";
        std::cout << " 4. Ejecutar Busqueda Binaria    (Local OpenMP vs Distribuida MPI+OpenMP)\n";
        std::cout << " 5. Ejecutar Prueba Automatizada de Rendimiento (100 vs 10,000,000 elem)\n";
        std::cout << " 6. Mostrar Tabla Resumen de Tiempos y Comparativa de Rendimiento\n";
        std::cout << " 7. Salir del Sistema\n";
        std::cout << "------------------------------------------------------------------------\n";
        std::cout << " Seleccione una opcion [1-7]: ";
    }
}

// Búsqueda secuencial base sin paralelismo (1 solo hilo) para medir Speedup real
long long busquedaSecuencialBase1Hilo(const int* arr, long long n, int target, double& tiempo) {
    double t_ini = MPI_Wtime();
    long long idx = -1;
    for (long long i = 0; i < n; i++) {
        if (arr[i] == target) {
            idx = i;
            break;
        }
    }
    double t_fin = MPI_Wtime();
    tiempo = t_fin - t_ini;
    return idx;
}

// Búsqueda binaria base sin paralelismo (1 solo hilo)
long long busquedaBinariaBase1Hilo(const int* arr, long long n, int target, double& tiempo) {
    double t_ini = MPI_Wtime();
    long long idx = -1;
    long long low = 0, high = n - 1;
    while (low <= high) {
        long long mid = low + (high - low) / 2;
        if (arr[mid] == target) {
            idx = mid;
            break;
        } else if (arr[mid] < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    double t_fin = MPI_Wtime();
    tiempo = t_fin - t_ini;
    return idx;
}

int main(int argc, char* argv[]) {
    // 1. Inicialización del entorno MPI
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // INSTRUCCION ESPECIAL OBLIGATORIA: Nombres como PRIMERA linea de ejecucion
    imprimir_encabezado_equipo(rank);

    BusquedaParalela motor(rank, num_procs, processor_name, "Equipo_Giallo");

    // Variables globales gestionadas en el Nodo 0
    int* arr_global = nullptr;
    long long n_global = 0;
    int max_val_global = 0;
    bool esta_ordenado = false;

    // Métricas para historial y comparativa
    double t_ordenamiento_openmp = 0.0;
    double t_ordenamiento_seq = 0.0;
    double t_sec_seq = 0.0, t_sec_loc = 0.0, t_sec_dist = 0.0;
    double t_bin_seq = 0.0, t_bin_loc = 0.0, t_bin_dist = 0.0;
    long long last_n_tested = 0;

    int opcion = 0;

    while (opcion != 7) {
        if (rank == 0) {
            mostrar_menu(rank);
            std::cin >> opcion;
            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                opcion = 0;
            }
        }

        // Difusión síncrona de la opción elegida a todos los procesos MPI
        MPI_Bcast(&opcion, 1, MPI_INT, 0, MPI_COMM_WORLD);

        switch (opcion) {
            case 1: { // Generar Arreglo Dinámico
                if (rank == 0) {
                    std::cout << "\n>>> CONFIGURACION DEL ARREGLO DINAMICO <<<\n";
                    std::cout << "Ingrese cantidad de elementos N (ej. 100 para control, 10000000 para rendimiento): ";
                    std::cin >> n_global;
                    std::cout << "Ingrese valor maximo aleatorio (ej. 200 o 2000000): ";
                    std::cin >> max_val_global;

                    if (arr_global != nullptr) {
                        delete[] arr_global;
                    }
                    arr_global = new int[n_global];
                    motor.generarArregloAleatorio(arr_global, n_global, max_val_global);
                    esta_ordenado = false;

                    if (n_global <= 100) {
                        std::cout << "\n[Vista de Arreglo Desordenado (Primeros elementos)]:\n";
                        for (long long i = 0; i < n_global; i++) {
                            std::cout << arr_global[i] << " ";
                            if ((i + 1) % 20 == 0) std::cout << "\n";
                        }
                        std::cout << "\n";
                    }
                }

                // Sincronizar tamaño global y modo debug
                MPI_Bcast(&n_global, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
                motor.set_debug_mode(n_global <= 100);

                // Distribuir el arreglo inicial entre los nodos
                motor.distribuirBloques(arr_global, n_global);
                break;
            }

            case 2: { // Ordenar Arreglo con OpenMP
                if (rank == 0) {
                    if (arr_global == nullptr || n_global <= 0) {
                        std::cout << "[ERROR] Primero debe generar el arreglo (Opcion 1).\n";
                    } else {
                        std::cout << "\n>>> INICIANDO ORDENAMIENTO PARALELO CON OPENMP EN NODO 0 <<<\n";
                        t_ordenamiento_openmp = motor.ordenarArregloOpenMP(arr_global, n_global);
                        esta_ordenado = true;

                        if (n_global <= 100) {
                            std::cout << "\n[Vista de Arreglo Ordenado]:\n";
                            for (long long i = 0; i < n_global; i++) {
                                std::cout << arr_global[i] << " ";
                                if ((i + 1) % 20 == 0) std::cout << "\n";
                            }
                            std::cout << "\n";
                        }
                    }
                }

                MPI_Bcast(&esta_ordenado, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

                if (esta_ordenado) {
                    // Redistribuir el arreglo ya ordenado para que los bloques queden ordenados y continuos
                    motor.distribuirBloques(arr_global, n_global);
                    if (rank == 0) {
                        std::cout << "[OK] Arreglo ordenado redistribuido exitosamente a los nodos trabajadores.\n";
                    }
                }
                break;
            }

            case 3: { // Búsqueda Secuencial (Local vs Distribuida)
                if (n_global <= 0) {
                    if (rank == 0) std::cout << "[ERROR] Genere primero el arreglo en la Opcion 1.\n";
                    break;
                }

                int target = 0;
                if (rank == 0) {
                    std::cout << "\n>>> BUSQUEDA SECUENCIAL (LOCAL VS DISTRIBUIDA) <<<\n";
                    std::cout << "Ingrese el valor (target) a buscar: ";
                    std::cin >> target;
                }
                MPI_Bcast(&target, 1, MPI_INT, 0, MPI_COMM_WORLD);

                // 1. Ejecución Secuencial Base (1 hilo)
                long long idx_seq = -1;
                if (rank == 0) {
                    idx_seq = busquedaSecuencialBase1Hilo(arr_global, n_global, target, t_sec_seq);
                }

                // 2. Esquema Local Multihilo (OpenMP en Nodo 0)
                long long idx_local = -1;
                if (rank == 0) {
                    idx_local = motor.busquedaSecuencialLocal(arr_global, n_global, target, t_sec_loc);
                }

                // 3. Esquema Distribuido Multinivel (MPI + OpenMP)
                MPI_Barrier(MPI_COMM_WORLD);
                long long idx_dist = motor.busquedaSecuencialDistribuida(target, t_sec_dist);

                if (rank == 0) {
                    last_n_tested = n_global;
                    std::cout << "\n========================================================================\n";
                    std::cout << "                 RESULTADOS: BUSQUEDA SECUENCIAL                        \n";
                    std::cout << "========================================================================\n";
                    std::cout << "Elemento buscado (Target): " << target << "\n";
                    std::cout << "Indice detectado:          " << ((idx_dist != -1) ? std::to_string(idx_dist) : "NO ENCONTRADO") << "\n\n";
                    std::cout << std::left << std::setw(35) << "Esquema" 
                              << std::setw(18) << "Tiempo (s)" 
                              << std::setw(15) << "Speedup" << "\n";
                    std::cout << "------------------------------------------------------------------------\n";
                    std::cout << std::left << std::setw(35) << "1. Secuencial Base (1 hilo)" 
                              << std::setw(18) << std::fixed << std::setprecision(6) << t_sec_seq 
                              << std::setw(15) << "1.00x (Ref)" << "\n";
                    std::cout << std::left << std::setw(35) << "2. Local Multihilo (OpenMP)" 
                              << std::setw(18) << std::fixed << std::setprecision(6) << t_sec_loc 
                              << std::setw(15) << (t_sec_loc > 0 ? (t_sec_seq / t_sec_loc) : 0.0) << "x" << "\n";
                    std::cout << std::left << std::setw(35) << "3. Distribuido (MPI + OpenMP)" 
                              << std::setw(18) << std::fixed << std::setprecision(6) << t_sec_dist 
                              << std::setw(15) << (t_sec_dist > 0 ? (t_sec_seq / t_sec_dist) : 0.0) << "x" << "\n";
                    std::cout << "========================================================================\n";
                }
                break;
            }

            case 4: { // Búsqueda Binaria (Local vs Distribuida)
                if (n_global <= 0) {
                    if (rank == 0) std::cout << "[ERROR] Genere primero el arreglo en la Opcion 1.\n";
                    break;
                }
                if (!esta_ordenado) {
                    if (rank == 0) {
                        std::cout << "[ADVERTENCIA] El arreglo no esta ordenado. Ordenando automaticamente con OpenMP...\n";
                        t_ordenamiento_openmp = motor.ordenarArregloOpenMP(arr_global, n_global);
                        esta_ordenado = true;
                    }
                    MPI_Bcast(&esta_ordenado, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
                    motor.distribuirBloques(arr_global, n_global);
                }

                int target = 0;
                if (rank == 0) {
                    std::cout << "\n>>> BUSQUEDA BINARIA (LOCAL VS DISTRIBUIDA) <<<\n";
                    std::cout << "Ingrese el valor (target) a buscar: ";
                    std::cin >> target;
                }
                MPI_Bcast(&target, 1, MPI_INT, 0, MPI_COMM_WORLD);

                // 1. Binaria Secuencial Base (1 hilo)
                long long idx_seq = -1;
                if (rank == 0) {
                    idx_seq = busquedaBinariaBase1Hilo(arr_global, n_global, target, t_bin_seq);
                }

                // 2. Esquema Local Multihilo (OpenMP en Nodo 0)
                long long idx_local = -1;
                if (rank == 0) {
                    idx_local = motor.busquedaBinariaLocal(arr_global, n_global, target, t_bin_loc);
                }

                // 3. Esquema Distribuido Multinivel (MPI + OpenMP)
                MPI_Barrier(MPI_COMM_WORLD);
                long long idx_dist = motor.busquedaBinariaDistribuida(target, t_bin_dist);

                if (rank == 0) {
                    last_n_tested = n_global;
                    std::cout << "\n========================================================================\n";
                    std::cout << "                  RESULTADOS: BUSQUEDA BINARIA                          \n";
                    std::cout << "========================================================================\n";
                    std::cout << "Elemento buscado (Target): " << target << "\n";
                    std::cout << "Indice detectado:          " << ((idx_dist != -1) ? std::to_string(idx_dist) : "NO ENCONTRADO") << "\n\n";
                    std::cout << std::left << std::setw(35) << "Esquema" 
                              << std::setw(18) << "Tiempo (s)" 
                              << std::setw(15) << "Speedup" << "\n";
                    std::cout << "------------------------------------------------------------------------\n";
                    std::cout << std::left << std::setw(35) << "1. Secuencial Base (1 hilo)" 
                              << std::setw(18) << std::fixed << std::setprecision(8) << t_bin_seq 
                              << std::setw(15) << "1.00x (Ref)" << "\n";
                    std::cout << std::left << std::setw(35) << "2. Local Multihilo (OpenMP)" 
                              << std::setw(18) << std::fixed << std::setprecision(8) << t_bin_loc 
                              << std::setw(15) << (t_bin_loc > 0 ? (t_bin_seq / t_bin_loc) : 0.0) << "x" << "\n";
                    std::cout << std::left << std::setw(35) << "3. Distribuido (MPI + OpenMP)" 
                              << std::setw(18) << std::fixed << std::setprecision(8) << t_bin_dist 
                              << std::setw(15) << (t_bin_dist > 0 ? (t_bin_seq / t_bin_dist) : 0.0) << "x" << "\n";
                    std::cout << "========================================================================\n";
                }
                break;
            }

            case 5: { // Prueba Automatizada de Rendimiento (Control N=100 vs Masivo N=10,000,000)
                if (rank == 0) {
                    std::cout << "\n========================================================================\n";
                    std::cout << "     INICIANDO PRUEBAS AUTOMATIZADAS REQUERIDAS (100 vs 10,000,000)     \n";
                    std::cout << "========================================================================\n";
                }

                long long test_sizes[2] = {100, 10000000};
                for (int t = 0; t < 2; t++) {
                    long long n_test = test_sizes[t];
                    int max_v = (n_test <= 100) ? 200 : 20000000;

                    if (rank == 0) {
                        std::cout << "\n------------------------------------------------------------------------\n";
                        std::cout << ">>> EJECUTANDO PRUEBA CON N = " << n_test << " ELEMENTOS <<<\n";
                        std::cout << "------------------------------------------------------------------------\n";

                        if (arr_global != nullptr) delete[] arr_global;
                        arr_global = new int[n_test];
                        motor.generarArregloAleatorio(arr_global, n_test, max_v);
                    }

                    n_global = n_test;
                    MPI_Bcast(&n_global, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
                    motor.set_debug_mode(n_test <= 100);

                    // 1. Medir Ordenamiento Secuencial vs OpenMP
                    double t_sort_seq_local = 0.0, t_sort_omp_local = 0.0;
                    if (rank == 0) {
                        // Clon temporal para medir ordenamiento secuencial
                        int* arr_clone = new int[n_test];
                        for (long long i = 0; i < n_test; i++) arr_clone[i] = arr_global[i];
                        t_sort_seq_local = motor.ordenarArregloSecuencial(arr_clone, n_test);
                        delete[] arr_clone;

                        t_sort_omp_local = motor.ordenarArregloOpenMP(arr_global, n_test);
                        if (n_test == 10000000) {
                            t_ordenamiento_seq = t_sort_seq_local;
                            t_ordenamiento_openmp = t_sort_omp_local;
                        }
                    }

                    // Distribuir arreglo ordenado
                    motor.distribuirBloques(arr_global, n_test);

                    // Seleccionar un target que con seguridad existe (el elemento en la mitad)
                    int target_test = 0;
                    if (rank == 0) {
                        target_test = arr_global[n_test / 2];
                        std::cout << "Target seleccionado para la prueba: " << target_test << " (en indice ~" << (n_test / 2) << ")\n";
                    }
                    MPI_Bcast(&target_test, 1, MPI_INT, 0, MPI_COMM_WORLD);

                    // Búsqueda Secuencial
                    double t_s_seq = 0.0, t_s_loc = 0.0, t_s_dist = 0.0;
                    if (rank == 0) {
                        busquedaSecuencialBase1Hilo(arr_global, n_test, target_test, t_s_seq);
                        motor.busquedaSecuencialLocal(arr_global, n_test, target_test, t_s_loc);
                    }
                    MPI_Barrier(MPI_COMM_WORLD);
                    motor.busquedaSecuencialDistribuida(target_test, t_s_dist);

                    // Búsqueda Binaria
                    double t_b_seq = 0.0, t_b_loc = 0.0, t_b_dist = 0.0;
                    if (rank == 0) {
                        busquedaBinariaBase1Hilo(arr_global, n_test, target_test, t_b_seq);
                        motor.busquedaBinariaLocal(arr_global, n_test, target_test, t_b_loc);
                    }
                    MPI_Barrier(MPI_COMM_WORLD);
                    motor.busquedaBinariaDistribuida(target_test, t_b_dist);

                    if (rank == 0) {
                        if (n_test == 10000000) {
                            t_sec_seq = t_s_seq; t_sec_loc = t_s_loc; t_sec_dist = t_s_dist;
                            t_bin_seq = t_b_seq; t_bin_loc = t_b_loc; t_bin_dist = t_b_dist;
                            last_n_tested = 10000000;
                        }

                        std::cout << "\n[RESUMEN DE TIEMPOS N = " << n_test << "]:\n";
                        std::cout << " - Ordenamiento Secuencial: " << std::fixed << std::setprecision(6) << t_sort_seq_local << " s\n";
                        std::cout << " - Ordenamiento OpenMP:     " << std::fixed << std::setprecision(6) << t_sort_omp_local << " s "
                                  << "(Speedup: " << (t_sort_omp_local > 0 ? (t_sort_seq_local / t_sort_omp_local) : 0.0) << "x)\n";
                        std::cout << " - B. Secuencial (Seq / Loc / Dist): " 
                                  << t_s_seq << " s / " << t_s_loc << " s / " << t_s_dist << " s\n";
                        std::cout << " - B. Binaria    (Seq / Loc / Dist): " 
                                  << t_b_seq << " s / " << t_b_loc << " s / " << t_b_dist << " s\n";
                    }
                }
                break;
            }

            case 6: { // Mostrar Tabla Resumen
                if (rank == 0) {
                    std::cout << "\n========================================================================\n";
                    std::cout << "        TABLA CONSOLIDADA DE RENDIMIENTO Y COMPARATIVA PARALELA         \n";
                    std::cout << "========================================================================\n";
                    std::cout << "Elementos evaluados en prueba masiva: " << (last_n_tested > 0 ? last_n_tested : 10000000) << "\n";
                    std::cout << "Total de Procesos MPI:               " << num_procs << "\n\n";

                    std::cout << "1. ORDENAMIENTO PREVIO (Merge Sort):\n";
                    std::cout << "   - Secuencial: " << std::fixed << std::setprecision(6) << t_ordenamiento_seq << " s\n";
                    std::cout << "   - OpenMP:     " << std::fixed << std::setprecision(6) << t_ordenamiento_openmp << " s\n";
                    std::cout << "   - Speedup:    " << (t_ordenamiento_openmp > 0 ? (t_ordenamiento_seq / t_ordenamiento_openmp) : 0.0) << "x\n\n";

                    std::cout << "2. BUSQUEDA SECUENCIAL POR BLOQUES:\n";
                    std::cout << "   - Secuencial (1 Hilo):         " << t_sec_seq << " s (1.00x)\n";
                    std::cout << "   - Local Multihilo (OpenMP):    " << t_sec_loc << " s (" 
                              << (t_sec_loc > 0 ? (t_sec_seq / t_sec_loc) : 0.0) << "x)\n";
                    std::cout << "   - Distribuida (MPI + OpenMP):  " << t_sec_dist << " s (" 
                              << (t_sec_dist > 0 ? (t_sec_seq / t_sec_dist) : 0.0) << "x)\n\n";

                    std::cout << "3. BUSQUEDA BINARIA POR BLOQUES:\n";
                    std::cout << "   - Secuencial (1 Hilo):         " << t_bin_seq << " s (1.00x)\n";
                    std::cout << "   - Local Multihilo (OpenMP):    " << t_bin_loc << " s (" 
                              << (t_bin_loc > 0 ? (t_bin_seq / t_bin_loc) : 0.0) << "x)\n";
                    std::cout << "   - Distribuida (MPI + OpenMP):  " << t_bin_dist << " s (" 
                              << (t_bin_dist > 0 ? (t_bin_seq / t_bin_dist) : 0.0) << "x)\n";
                    std::cout << "========================================================================\n";
                }
                break;
            }

            case 7: { // Salir
                if (rank == 0) {
                    std::cout << "\nFinalizando ejecucion en todos los nodos del cluster...\n";
                }
                break;
            }

            default: {
                if (rank == 0) {
                    std::cout << "[OPCION INVALIDA] Seleccione un numero entre 1 y 7.\n";
                }
                break;
            }
        }
    }

    // Limpieza de memoria dinámica en el Nodo 0
    if (arr_global != nullptr) {
        delete[] arr_global;
        arr_global = nullptr;
    }

    // Sincronizar todos los procesos antes de salir
    MPI_Barrier(MPI_COMM_WORLD);

    // INSTRUCCION ESPECIAL OBLIGATORIA: Nombres como ULTIMA linea de ejecucion
    imprimir_pie_equipo(rank);

    // Finalizar el entorno MPI
    MPI_Finalize();
    return 0;
}
