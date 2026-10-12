#ifndef BUSQUEDA_PARALELA_H
#define BUSQUEDA_PARALELA_H

#include <string>
#include <fstream>
#include <mpi.h>

class BusquedaParalela {
private:
    int rank;
    int num_procs;
    int max_threads;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    std::string equipo_nombre;
    std::ofstream log_file;
    bool debug_mode;

    // Memoria local de cada nodo (punteros dinamicos estrictos)
    int* bloque_local;
    long long size_local;
    long long offset_global;

    // Métodos auxiliares de ordenamiento paralelo (Merge Sort con OpenMP)
    void merge(int* arr, int* temp, long long left, long long mid, long long right);
    void mergeSortSeq(int* arr, int* temp, long long left, long long right);
    void mergeSortPar(int* arr, int* temp, long long left, long long right);

public:
    BusquedaParalela(int rank, int num_procs, const char* proc_name, const std::string& eq_nombre = "Equipo_Giallo");
    ~BusquedaParalela();

    void set_debug_mode(bool debug) { debug_mode = debug; }
    bool is_debug_mode() const { return debug_mode; }

    // Manejo de Logs y Auditoría
    void registrar_log(const std::string& mensaje, bool imprimir_consola = true);
    void registrar_cabecera();

    // Generación y llenado dinámico (Nodo 0)
    void generarArregloAleatorio(int* arr, long long n, int max_val);

    // Ordenamiento Paralelo con OpenMP (Nodo 0)
    double ordenarArregloOpenMP(int* arr, long long n);
    double ordenarArregloSecuencial(int* arr, long long n);

    // Distribución y recepción de bloques (MPI)
    void asignarBloqueLocal(long long total_size);
    void distribuirBloques(int* arr_global, long long total_size);

    // Búsqueda Secuencial
    // 1. Local Multihilo (Nodo 0 sobre arreglo completo)
    long long busquedaSecuencialLocal(int* arr, long long n, int target, double& tiempo);
    // 2. Distribuida por bloques (MPI + OpenMP en cada nodo)
    long long busquedaSecuencialDistribuida(int target, double& tiempo);

    // Búsqueda Binaria
    // 1. Local Multihilo (Nodo 0 sobre arreglo completo ordenado)
    long long busquedaBinariaLocal(int* arr, long long n, int target, double& tiempo);
    // 2. Distribuida por bloques (MPI + OpenMP en cada nodo)
    long long busquedaBinariaDistribuida(int target, double& tiempo);

    // Getters
    int* getBloqueLocal() { return bloque_local; }
    long long getSizeLocal() const { return size_local; }
    long long getOffsetGlobal() const { return offset_global; }
};

#endif // BUSQUEDA_PARALELA_H
