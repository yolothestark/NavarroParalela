#include "BusquedaParalela.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <omp.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <algorithm>

BusquedaParalela::BusquedaParalela(int rank, int num_procs, const char* proc_name, const std::string& eq_nombre) {
    this->rank = rank;
    this->num_procs = num_procs;
    this->equipo_nombre = eq_nombre;
    this->max_threads = omp_get_max_threads();
    this->debug_mode = false;
    this->bloque_local = nullptr;
    this->size_local = 0;
    this->offset_global = 0;

    std::strncpy(this->processor_name, proc_name, MPI_MAX_PROCESSOR_NAME - 1);
    this->processor_name[MPI_MAX_PROCESSOR_NAME - 1] = '\0';

    // Crear y abrir archivo de log individual: log_equipo_[NOMBRE_PC]_nodo_[RANK].txt
    std::ostringstream filename;
    filename << "log_equipo_" << this->processor_name << "_nodo_" << rank << ".txt";
    log_file.open(filename.str(), std::ios::out);

    registrar_cabecera();
}

BusquedaParalela::~BusquedaParalela() {
    if (bloque_local != nullptr) {
        delete[] bloque_local;
        bloque_local = nullptr;
    }
    if (log_file.is_open()) {
        log_file << "\n========================================================================\n";
        log_file << "Cierre de sesion del nodo " << rank << " exitoso.\n";
        log_file << "========================================================================\n";
        log_file.close();
    }
}

void BusquedaParalela::registrar_cabecera() {
    if (!log_file.is_open()) return;

    time_t now = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    std::ostringstream oss;
    oss << "========================================================================\n";
    oss << "  REGISTRO DE AUDITORIA Y PERSISTENCIA POR NODO\n";
    oss << "  PRACTICA 1.5: BUSQUEDA PARALELA DISTRIBUIDA (MPI + OPENMP)\n";
    oss << "========================================================================\n";
    oss << "Nombre del Equipo / Host: " << processor_name << " (" << equipo_nombre << ")\n";
    oss << "Rango MPI (Rank):         " << rank << " de " << num_procs << " procesos\n";
    oss << "Hilos OpenMP detectados:  " << max_threads << " hilos concurrentes\n";
    oss << "Fecha y Hora de Inicio:   " << buf << "\n";
    oss << "========================================================================\n\n";

    log_file << oss.str();
    log_file.flush();
}

void BusquedaParalela::registrar_log(const std::string& mensaje, bool imprimir_consola) {
    #pragma omp critical(log_write)
    {
        if (log_file.is_open()) {
            log_file << mensaje;
            log_file.flush();
        }
        if (imprimir_consola) {
            std::cout << mensaje;
            std::cout.flush();
        }
    }
}

void BusquedaParalela::generarArregloAleatorio(int* arr, long long n, int max_val) {
    if (arr == nullptr || n <= 0) return;

    double t_ini = omp_get_wtime();
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        unsigned int seed = (unsigned int)(time(nullptr) ^ (tid * 7919) ^ (n * 104729));
        
        #pragma omp for schedule(static)
        for (long long i = 0; i < n; i++) {
            // Generador congruencial lineal rapido y distribuido por hilo
            seed = (seed * 1103515245U + 12345U) & 0x7fffffffU;
            arr[i] = (int)(seed % (unsigned int)(max_val + 1));
        }
    }
    double t_fin = omp_get_wtime();

    std::ostringstream oss;
    oss << "[Nodo " << rank << "] Arreglo dinamico de " << n << " elementos generado en " 
        << std::fixed << std::setprecision(6) << (t_fin - t_ini) << " s (Rango: 0 a " << max_val << ")\n";
    registrar_log(oss.str(), true);
}

void BusquedaParalela::merge(int* arr, int* temp, long long left, long long mid, long long right) {
    long long i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    for (long long p = left; p <= right; p++) {
        arr[p] = temp[p];
    }
}

void BusquedaParalela::mergeSortSeq(int* arr, int* temp, long long left, long long right) {
    if (left < right) {
        long long mid = left + (right - left) / 2;
        mergeSortSeq(arr, temp, left, mid);
        mergeSortSeq(arr, temp, mid + 1, right);
        merge(arr, temp, left, mid, right);
    }
}

void BusquedaParalela::mergeSortPar(int* arr, int* temp, long long left, long long right) {
    if (left < right) {
        long long mid = left + (right - left) / 2;

        // Umbral de corte secuencial para mitigar overhead de generacion de tareas
        if ((right - left) < 10000) {
            mergeSortSeq(arr, temp, left, right);
        } else {
            #pragma omp task shared(arr, temp)
            mergeSortPar(arr, temp, left, mid);

            #pragma omp task shared(arr, temp)
            mergeSortPar(arr, temp, mid + 1, right);

            #pragma omp taskwait
            merge(arr, temp, left, mid, right);
        }
    }
}

double BusquedaParalela::ordenarArregloOpenMP(int* arr, long long n) {
    if (arr == nullptr || n <= 0) return 0.0;

    int* temp = new int[n];
    double t_ini = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp single
        {
            mergeSortPar(arr, temp, 0, n - 1);
        }
    }

    double t_fin = omp_get_wtime();
    delete[] temp;

    double tiempo_total = t_fin - t_ini;

    std::ostringstream oss;
    oss << "[AUDITORIA NODO 0] Ordenamiento previo con OpenMP (Parallel Merge Sort) completado en " 
        << std::fixed << std::setprecision(6) << tiempo_total << " segundos para " << n << " elementos.\n";
    registrar_log(oss.str(), true);

    return tiempo_total;
}

double BusquedaParalela::ordenarArregloSecuencial(int* arr, long long n) {
    if (arr == nullptr || n <= 0) return 0.0;

    int* temp = new int[n];
    double t_ini = omp_get_wtime();

    mergeSortSeq(arr, temp, 0, n - 1);

    double t_fin = omp_get_wtime();
    delete[] temp;

    return (t_fin - t_ini);
}

void BusquedaParalela::asignarBloqueLocal(long long total_size) {
    long long base = total_size / num_procs;
    long long rem = total_size % num_procs;

    // Reparto equitativo con remanente distribuido
    if (rank < rem) {
        size_local = base + 1;
        offset_global = rank * (base + 1);
    } else {
        size_local = base;
        offset_global = rem * (base + 1) + (rank - rem) * base;
    }

    if (bloque_local != nullptr) {
        delete[] bloque_local;
    }
    bloque_local = new int[size_local];
}

void BusquedaParalela::distribuirBloques(int* arr_global, long long total_size) {
    asignarBloqueLocal(total_size);

    int* sendcounts = new int[num_procs];
    int* displs = new int[num_procs];

    long long base = total_size / num_procs;
    long long rem = total_size % num_procs;

    long long cur_displ = 0;
    for (int i = 0; i < num_procs; i++) {
        long long sz = (i < rem) ? (base + 1) : base;
        sendcounts[i] = (int)sz;
        displs[i] = (int)cur_displ;
        cur_displ += sz;
    }

    MPI_Scatterv(arr_global, sendcounts, displs, MPI_INT,
                 bloque_local, (int)size_local, MPI_INT,
                 0, MPI_COMM_WORLD);

    delete[] sendcounts;
    delete[] displs;
}

long long BusquedaParalela::busquedaSecuencialLocal(int* arr, long long n, int target, double& tiempo) {
    double t_ini = omp_get_wtime();
    long long encontrado_idx = -1;

    #pragma omp parallel for schedule(static)
    for (long long i = 0; i < n; i++) {
        if (arr[i] == target) {
            #pragma omp critical(encontrado_sec_loc)
            {
                if (encontrado_idx == -1 || i < encontrado_idx) {
                    encontrado_idx = i;
                }
            }

            if (debug_mode) {
                int tid = omp_get_thread_num();
                std::ostringstream oss;
                oss << "[Equipo: " << processor_name << "] [Nodo MPI: " << rank 
                    << "] [Hilo OpenMP: " << tid << "] [Bloque: 0 - " << n << "] "
                    << "[Algoritmo: Búsqueda Secuencial Local] "
                    << "[Estado: Elemento Encontrado en Índice Global: " << i << "]\n";
                registrar_log(oss.str(), true);
            }
        }
    }

    double t_fin = omp_get_wtime();
    tiempo = t_fin - t_ini;
    return encontrado_idx;
}

long long BusquedaParalela::busquedaSecuencialDistribuida(int target, double& tiempo) {
    double t_ini = MPI_Wtime();
    long long encontrado_local = -1;

    #pragma omp parallel for schedule(static)
    for (long long i = 0; i < size_local; i++) {
        if (bloque_local[i] == target) {
            long long global_idx = offset_global + i;
            #pragma omp critical(encontrado_sec_dist)
            {
                if (encontrado_local == -1 || global_idx < encontrado_local) {
                    encontrado_local = global_idx;
                }
            }

            if (debug_mode) {
                int tid = omp_get_thread_num();
                std::ostringstream oss;
                oss << "[Equipo: " << processor_name << "] [Nodo MPI: " << rank 
                    << "] [Hilo OpenMP: " << tid 
                    << "] [Bloque: " << offset_global << " - " << (offset_global + size_local) << "] "
                    << "[Algoritmo: Búsqueda Secuencial Distribuida] "
                    << "[Estado: Elemento Encontrado en Índice Global: " << global_idx << "]\n";
                registrar_log(oss.str(), true);
            }
        }
    }

    if (debug_mode && encontrado_local == -1) {
        std::ostringstream oss;
        oss << "[Equipo: " << processor_name << "] [Nodo MPI: " << rank 
            << "] [Hilos: " << max_threads
            << "] [Bloque: " << offset_global << " - " << (offset_global + size_local) << "] "
            << "[Algoritmo: Búsqueda Secuencial Distribuida] "
            << "[Estado: Elemento NO encontrado en este bloque local]\n";
        registrar_log(oss.str(), true);
    }

    // Consolidacion con MPI_Allreduce
    // Para conservar el indice menor si se encuentra en varios nodos, mapeamos -1 a infinito temporal
    long long valor_envio = (encontrado_local == -1) ? 9223372036854775807LL : encontrado_local;
    long long valor_global = -1;

    MPI_Allreduce(&valor_envio, &valor_global, 1, MPI_LONG_LONG, MPI_MIN, MPI_COMM_WORLD);

    double t_fin = MPI_Wtime();
    tiempo = t_fin - t_ini;

    long long resultado_final = (valor_global == 9223372036854775807LL) ? -1 : valor_global;
    return resultado_final;
}

long long BusquedaParalela::busquedaBinariaLocal(int* arr, long long n, int target, double& tiempo) {
    double t_ini = omp_get_wtime();
    long long encontrado_idx = -1;

    int threads = omp_get_max_threads();
    long long sub_size = n / threads;

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        long long start = tid * sub_size;
        long long end = (tid == threads - 1) ? (n - 1) : (start + sub_size - 1);

        if (start <= end && target >= arr[start] && target <= arr[end]) {
            long long low = start, high = end;
            while (low <= high) {
                long long mid = low + (high - low) / 2;
                if (arr[mid] == target) {
                    #pragma omp critical(encontrado_bin_loc)
                    {
                        if (encontrado_idx == -1 || mid < encontrado_idx) {
                            encontrado_idx = mid;
                        }
                    }
                    if (debug_mode) {
                        std::ostringstream oss;
                        oss << "[Equipo: " << processor_name << "] [Nodo MPI: 0] "
                            << "[Hilo OpenMP: " << tid << "] [Sub-intervalo: " << start << " - " << end << "] "
                            << "[Algoritmo: Búsqueda Binaria Local] "
                            << "[Estado: Elemento Encontrado en Índice Global: " << mid << "]\n";
                        registrar_log(oss.str(), true);
                    }
                    break;
                } else if (arr[mid] < target) {
                    low = mid + 1;
                } else {
                    high = mid - 1;
                }
            }
        }
    }

    double t_fin = omp_get_wtime();
    tiempo = t_fin - t_ini;
    return encontrado_idx;
}

long long BusquedaParalela::busquedaBinariaDistribuida(int target, double& tiempo) {
    double t_ini = MPI_Wtime();
    long long encontrado_local = -1;

    // 1. Verificación de rangos por bloque asignado
    if (size_local > 0 && target >= bloque_local[0] && target <= bloque_local[size_local - 1]) {
        // El valor cae dentro del rango de este nodo
        int threads = omp_get_max_threads();
        long long sub_size = size_local / threads;

        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            long long start = tid * sub_size;
            long long end = (tid == threads - 1) ? (size_local - 1) : (start + sub_size - 1);

            if (start <= end && target >= bloque_local[start] && target <= bloque_local[end]) {
                long long low = start, high = end;
                while (low <= high) {
                    long long mid = low + (high - low) / 2;
                    if (bloque_local[mid] == target) {
                        long long global_idx = offset_global + mid;
                        #pragma omp critical(encontrado_bin_dist)
                        {
                            if (encontrado_local == -1 || global_idx < encontrado_local) {
                                encontrado_local = global_idx;
                            }
                        }

                        if (debug_mode) {
                            std::ostringstream oss;
                            oss << "[Equipo: " << processor_name << "] [Nodo MPI: " << rank 
                                << "] [Hilo OpenMP: " << tid 
                                << "] [Bloque: " << (offset_global + start) << " - " << (offset_global + end) << "] "
                                << "[Algoritmo: Búsqueda Binaria Distribuida] "
                                << "[Estado: Elemento Encontrado en Índice Global: " << global_idx << "]\n";
                            registrar_log(oss.str(), true);
                        }
                        break;
                    } else if (bloque_local[mid] < target) {
                        low = mid + 1;
                    } else {
                        high = mid - 1;
                    }
                }
            }
        }
    } else {
        if (debug_mode) {
            std::ostringstream oss;
            oss << "[Equipo: " << processor_name << "] [Nodo MPI: " << rank 
                << "] [Bloque: " << offset_global << " - " << (offset_global + size_local) << "] "
                << "[Algoritmo: Búsqueda Binaria Distribuida] "
                << "[Estado: Descartado por evaluación de límites (Rango del bloque: " 
                << (size_local > 0 ? bloque_local[0] : 0) << " a " 
                << (size_local > 0 ? bloque_local[size_local - 1] : 0) << ")]\n";
            registrar_log(oss.str(), true);
        }
    }

    // Consolidación global
    long long valor_envio = (encontrado_local == -1) ? 9223372036854775807LL : encontrado_local;
    long long valor_global = -1;

    MPI_Allreduce(&valor_envio, &valor_global, 1, MPI_LONG_LONG, MPI_MIN, MPI_COMM_WORLD);

    double t_fin = MPI_Wtime();
    tiempo = t_fin - t_ini;

    long long resultado_final = (valor_global == 9223372036854775807LL) ? -1 : valor_global;
    return resultado_final;
}
