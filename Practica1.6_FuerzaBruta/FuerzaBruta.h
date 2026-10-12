#ifndef FUERZA_BRUTA_H
#define FUERZA_BRUTA_H

#include <string>
#include <fstream>
#include <mpi.h>

// Tags de comunicacion MPI para arquitectura Maestro-Esclavo
#define TAG_RANGO_INICIO     10
#define TAG_RANGO_FIN        11
#define TAG_TARGET_INFO      12
#define TAG_HALLAZGO         20
#define TAG_PARADA_TEMPRANA  30
#define TAG_FINALIZADO       40

class FuerzaBruta {
private:
    int rank;
    int num_procs;
    int max_threads;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    std::string equipo_nombre;
    std::ofstream log_file;
    bool debug_mode;

    // Alfabeto estricto de 36 caracteres: A-Z (26) y 0-9 (10)
    static const char ALFABETO[37];

public:
    FuerzaBruta(int rank, int num_procs, const char* proc_name, const std::string& eq_nombre = "Equipo_Giallo");
    ~FuerzaBruta();

    void set_debug_mode(bool debug) { debug_mode = debug; }
    bool is_debug_mode() const { return debug_mode; }

    // Utilidades de Mapeo Matematico Base 36
    static unsigned long long calcularEspacioTotal(int longitud);
    static void convertirIndiceATexto(unsigned long long indice, char* buffer_salida, int longitud);
    static unsigned long long convertirTextoAIndice(const char* texto, int longitud);
    static bool validarClave(const char* texto, int longitud);

    // Auditoria y Persistencia de Logs por Nodo
    void registrar_cabecera();
    void registrar_log(const std::string& mensaje, bool imprimir_consola = true);

    // Ejecucion Secuencial Base (1 Nodo, 1 Hilo) para calcular Speedup
    bool ejecutarSecuencial(const char* target, int longitud, unsigned long long total_comb,
                            double& tiempo_salida, unsigned long long& idx_encontrado);

    // Ejecucion Distribuida Hibrida (MPI + OpenMP con Parada Temprana)
    // - Rol del Maestro (Nodo 0)
    bool ejecutarMaestro(const char* target, int longitud, unsigned long long total_comb,
                         double& tiempo_salida, int& nodo_ganador, int& hilo_ganador,
                         unsigned long long& idx_encontrado);

    // - Rol de los Esclavos (Nodos 1 .. num_procs - 1)
    void ejecutarEsclavo(const char* target, int longitud);
};

#endif // FUERZA_BRUTA_H
