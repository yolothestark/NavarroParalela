#ifndef OPERACIONES_ARREGLOS_H
#define OPERACIONES_ARREGLOS_H

#include <string>
#include <fstream>
#include <mpi.h>

class OperacionesArreglos {
private:
    int* A_local;
    int* B_local;
    int* C_local;
    long long size_local;
    long long offset_global;
    int rank;
    int num_procs;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    std::ofstream log_file;
    bool debug_mode;

public:
    OperacionesArreglos(int rank, int num_procs, char* processor_name);
    ~OperacionesArreglos();

    void set_debug_mode(bool mode) { debug_mode = mode; }
    void registrar_log(const std::string& msg);
    std::string formatear_mensaje(int thread, long long pos, const std::string& op, double val);

    void crearArregloMPI(long long total_size);
    void llenarSecuencial();
    void llenarAleatorio();

    void sumarArreglos();
    void restarArreglos();
    void multiplicarArreglos();
    void cuadradoArreglo();

    long long sumatoriaLocal();
    int maximoLocal();
    int minimoLocal();

    void recopilarResultadosC(); // Para demostrar MPI_Gather

    int* getA() { return A_local; }
    int* getB() { return B_local; }
    int* getC() { return C_local; }
    long long getSizeLocal() { return size_local; }
};

#endif
