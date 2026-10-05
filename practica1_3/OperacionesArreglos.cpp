#include "OperacionesArreglos.h"
#include <iostream>
#include <sstream>
#include <omp.h>
#include <cstdlib>
#include <ctime>
#include <random>
#include <algorithm>

OperacionesArreglos::OperacionesArreglos(int rank, int num_procs, char* processor_name) {
    this->rank = rank;
    this->num_procs = num_procs;
    std::string pname(processor_name);
    for (int i = 0; processor_name[i] != '\0'; i++) {
        this->processor_name[i] = processor_name[i];
    }
    this->processor_name[pname.length()] = '\0';
    
    A_local = nullptr;
    B_local = nullptr;
    C_local = nullptr;
    size_local = 0;
    offset_global = 0;
    debug_mode = false;

    std::ostringstream filename;
    filename << "log_equipo_" << pname << "_nodo_" << rank << ".txt";
    log_file.open(filename.str());
}

OperacionesArreglos::~OperacionesArreglos() {
    if (A_local) delete[] A_local;
    if (B_local) delete[] B_local;
    if (C_local) delete[] C_local;
    if (log_file.is_open()) {
        log_file.close();
    }
}

void OperacionesArreglos::registrar_log(const std::string& msg) {
    #pragma omp critical
    {
        std::cout << msg;
        log_file << msg;
    }
}

void OperacionesArreglos::crearArregloMPI(long long total_size) {
    // Calculamos el tamaño local. Para simplificar asumimos que es divisible.
    size_local = total_size / num_procs;
    offset_global = rank * size_local;
    
    // Si hay residuo, se lo damos al ultimo proceso (opcional, asume divisible)
    if (rank == num_procs - 1) {
        size_local += total_size % num_procs;
    }

    if (A_local) delete[] A_local;
    if (B_local) delete[] B_local;
    if (C_local) delete[] C_local;

    A_local = new int[size_local];
    B_local = new int[size_local];
    C_local = new int[size_local];

    std::ostringstream oss;
    oss << "[Equipo: " << processor_name << "] [Proceso MPI: " << rank 
        << "] [Hilo OpenMP: 0] [Posición: N/A] [Operación: Crear Arreglo] [Valor: " << size_local << "]\n";
    registrar_log(oss.str());
}

std::string OperacionesArreglos::formatear_mensaje(int thread, long long pos, const std::string& op, double val) {
    std::ostringstream oss;
    oss << "[Equipo: " << processor_name << "] [Proceso MPI: " << rank 
        << "] [Hilo OpenMP: " << thread << "] [Posición: " << pos 
        << "] [Operación: " << op << "] [Valor: " << val << "]\n";
    return oss.str();
}

void OperacionesArreglos::llenarSecuencial() {
    #pragma omp parallel for
    for (long long i = 0; i < size_local; ++i) {
        long long pos_global = offset_global + i;
        A_local[i] = pos_global + 1; // 1-based para que no haya ceros
        B_local[i] = pos_global + 1;
        
        if (debug_mode) {
            std::string msg = formatear_mensaje(omp_get_thread_num(), pos_global, "Llenar Secuencial", A_local[i]);
            registrar_log(msg);
        }
    }
}

void OperacionesArreglos::llenarAleatorio() {
    #pragma omp parallel
    {
        // Semilla independiente por proceso y por hilo
        std::mt19937 rng(time(NULL) + rank + omp_get_thread_num());
        std::uniform_int_distribution<int> dist(1, 1000000);

        #pragma omp for
        for (long long i = 0; i < size_local; ++i) {
            A_local[i] = dist(rng);
            B_local[i] = dist(rng);

            if (debug_mode) {
                long long pos_global = offset_global + i;
                std::string msg = formatear_mensaje(omp_get_thread_num(), pos_global, "Llenar Aleatorio", A_local[i]);
                registrar_log(msg);
            }
        }
    }
}

void OperacionesArreglos::sumarArreglos() {
    #pragma omp parallel for
    for (long long i = 0; i < size_local; ++i) {
        C_local[i] = A_local[i] + B_local[i];
        if (debug_mode) {
            long long pos_global = offset_global + i;
            std::string msg = formatear_mensaje(omp_get_thread_num(), pos_global, "Suma", C_local[i]);
            registrar_log(msg);
        }
    }
}

void OperacionesArreglos::restarArreglos() {
    #pragma omp parallel for
    for (long long i = 0; i < size_local; ++i) {
        C_local[i] = A_local[i] - B_local[i];
        if (debug_mode) {
            long long pos_global = offset_global + i;
            std::string msg = formatear_mensaje(omp_get_thread_num(), pos_global, "Resta", C_local[i]);
            registrar_log(msg);
        }
    }
}

void OperacionesArreglos::multiplicarArreglos() {
    #pragma omp parallel for
    for (long long i = 0; i < size_local; ++i) {
        C_local[i] = A_local[i] * B_local[i];
        if (debug_mode) {
            long long pos_global = offset_global + i;
            std::string msg = formatear_mensaje(omp_get_thread_num(), pos_global, "Multiplicacion", C_local[i]);
            registrar_log(msg);
        }
    }
}

void OperacionesArreglos::cuadradoArreglo() {
    #pragma omp parallel for
    for (long long i = 0; i < size_local; ++i) {
        C_local[i] = A_local[i] * A_local[i];
        if (debug_mode) {
            long long pos_global = offset_global + i;
            std::string msg = formatear_mensaje(omp_get_thread_num(), pos_global, "Cuadrado de A", C_local[i]);
            registrar_log(msg);
        }
    }
}

long long OperacionesArreglos::sumatoriaLocal() {
    long long suma = 0;
    #pragma omp parallel for reduction(+:suma)
    for (long long i = 0; i < size_local; ++i) {
        suma += A_local[i];
    }
    
    std::ostringstream oss;
    oss << "[Equipo: " << processor_name << "] [Proceso MPI: " << rank 
        << "] [Hilo OpenMP: N/A] [Posición: N/A] [Operación: Sumatoria Local] [Valor: " << suma << "]\n";
    registrar_log(oss.str());
    return suma;
}

int OperacionesArreglos::maximoLocal() {
    int max_val = A_local[0];
    #pragma omp parallel for reduction(max:max_val)
    for (long long i = 0; i < size_local; ++i) {
        if (A_local[i] > max_val) {
            max_val = A_local[i];
        }
    }
    std::ostringstream oss;
    oss << "[Equipo: " << processor_name << "] [Proceso MPI: " << rank 
        << "] [Hilo OpenMP: N/A] [Posición: N/A] [Operación: Maximo Local] [Valor: " << max_val << "]\n";
    registrar_log(oss.str());
    return max_val;
}

int OperacionesArreglos::minimoLocal() {
    int min_val = A_local[0];
    #pragma omp parallel for reduction(min:min_val)
    for (long long i = 0; i < size_local; ++i) {
        if (A_local[i] < min_val) {
            min_val = A_local[i];
        }
    }
    std::ostringstream oss;
    oss << "[Equipo: " << processor_name << "] [Proceso MPI: " << rank 
        << "] [Hilo OpenMP: N/A] [Posición: N/A] [Operación: Minimo Local] [Valor: " << min_val << "]\n";
    registrar_log(oss.str());
    return min_val;
}

void OperacionesArreglos::recopilarResultadosC() {
    // Para demostrar el uso de Gather de la "segunda versión"
    // Esto agruparía C_local en el nodo maestro
    int* C_global = nullptr;
    if (rank == 0) {
        C_global = new int[size_local * num_procs];
    }
    
    MPI_Gather(C_local, size_local, MPI_INT, C_global, size_local, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        std::ostringstream oss;
        oss << "[Equipo: " << processor_name << "] [Proceso MPI: 0] [Hilo OpenMP: N/A] [Posición: N/A] [Operación: MPI_Gather Completado] [Valor: 0]\n";
        registrar_log(oss.str());
        delete[] C_global;
    }
}
