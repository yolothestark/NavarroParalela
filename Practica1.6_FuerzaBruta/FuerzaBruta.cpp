#include "FuerzaBruta.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <omp.h>
#include <ctime>
#include <chrono>
#include <thread>

// Alfabeto estricto de 36 caracteres: A-Z (26) y 0-9 (10)
const char FuerzaBruta::ALFABETO[37] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

FuerzaBruta::FuerzaBruta(int rank, int num_procs, const char* proc_name, const std::string& eq_nombre) {
    this->rank = rank;
    this->num_procs = num_procs;
    this->equipo_nombre = eq_nombre;
    this->max_threads = omp_get_max_threads();
    this->debug_mode = false;

    std::strncpy(this->processor_name, proc_name, MPI_MAX_PROCESSOR_NAME - 1);
    this->processor_name[MPI_MAX_PROCESSOR_NAME - 1] = '\0';

    // Crear y abrir archivo de log individual por nodo: log_equipo_[NOMBRE_PC]_nodo_[RANK].txt
    std::ostringstream filename;
    filename << "log_equipo_" << this->processor_name << "_nodo_" << rank << ".txt";
    log_file.open(filename.str(), std::ios::out);

    registrar_cabecera();
}

FuerzaBruta::~FuerzaBruta() {
    if (log_file.is_open()) {
        log_file << "\n========================================================================\n";
        log_file << "[Nodo MPI: " << rank << "] Cierre de sesion y finalizacion limpia del proceso.\n";
        log_file << "========================================================================\n";
        log_file.close();
    }
}

void FuerzaBruta::registrar_cabecera() {
    if (!log_file.is_open()) return;

    time_t now = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    std::ostringstream oss;
    oss << "========================================================================\n";
    oss << "  REGISTRO DE AUDITORIA Y PERSISTENCIA POR NODO\n";
    oss << "  CENTRO UNIVERSITARIO DE TONALA (CUTONALA) - UDG\n";
    oss << "  PRACTICA 1.6: ALGORITMO DE FUERZA BRUTA DISTRIBUIDO (MPI + OPENMP)\n";
    oss << "========================================================================\n";
    oss << "Nombre del Equipo / Host: " << processor_name << " (" << equipo_nombre << ")\n";
    oss << "Rango MPI (Rank):         " << rank << " de " << num_procs << " procesos\n";
    oss << "Rol en la Arquitectura:   " << (rank == 0 ? "PROCESO MAESTRO (COORDINADOR)" : "PROCESO ESCLAVO (TRABAJADOR)") << "\n";
    oss << "Hilos OpenMP detectados:  " << max_threads << " hilos concurrentes\n";
    oss << "Fecha y Hora de Inicio:   " << buf << "\n";
    oss << "========================================================================\n\n";

    log_file << oss.str();
    log_file.flush();
}

void FuerzaBruta::registrar_log(const std::string& mensaje, bool imprimir_consola) {
    #pragma omp critical(log_write_fb)
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

unsigned long long FuerzaBruta::calcularEspacioTotal(int longitud) {
    unsigned long long total = 1;
    for (int i = 0; i < longitud; i++) {
        total *= 36ULL;
    }
    return total;
}

void FuerzaBruta::convertirIndiceATexto(unsigned long long indice, char* buffer_salida, int longitud) {
    buffer_salida[longitud] = '\0';
    for (int i = longitud - 1; i >= 0; i--) {
        buffer_salida[i] = ALFABETO[indice % 36ULL];
        indice /= 36ULL;
    }
}

unsigned long long FuerzaBruta::convertirTextoAIndice(const char* texto, int longitud) {
    unsigned long long idx = 0;
    for (int i = 0; i < longitud; i++) {
        char c = texto[i];
        int val = -1;
        if (c >= 'A' && c <= 'Z') val = c - 'A';
        else if (c >= '0' && c <= '9') val = 26 + (c - '0');
        idx = idx * 36ULL + (unsigned long long)val;
    }
    return idx;
}

bool FuerzaBruta::validarClave(const char* texto, int longitud) {
    if (texto == nullptr || longitud <= 0) return false;
    for (int i = 0; i < longitud; i++) {
        char c = texto[i];
        bool es_letra = (c >= 'A' && c <= 'Z');
        bool es_digito = (c >= '0' && c <= '9');
        if (!es_letra && !es_digito) return false;
    }
    return true;
}

bool FuerzaBruta::ejecutarSecuencial(const char* target, int longitud, unsigned long long total_comb,
                                     double& tiempo_salida, unsigned long long& idx_encontrado) {
    double t_ini = MPI_Wtime();
    idx_encontrado = 0;
    bool encontrado = false;

    char* candidato = new char[longitud + 1];

    for (unsigned long long idx = 0; idx < total_comb; idx++) {
        convertirIndiceATexto(idx, candidato, longitud);
        if (std::strcmp(candidato, target) == 0) {
            encontrado = true;
            idx_encontrado = idx;
            break;
        }
    }

    delete[] candidato;
    double t_fin = MPI_Wtime();
    tiempo_salida = t_fin - t_ini;
    return encontrado;
}

bool FuerzaBruta::ejecutarMaestro(const char* target, int longitud, unsigned long long total_comb,
                                  double& tiempo_salida, int& nodo_ganador, int& hilo_ganador,
                                  unsigned long long& idx_encontrado) {
    double t_ini = MPI_Wtime();
    nodo_ganador = -1;
    hilo_ganador = -1;
    idx_encontrado = 0;

    int num_esclavos = num_procs - 1;

    // Caso de ejecucion con 1 solo proceso (modo sin esclavos independientes)
    if (num_esclavos <= 0) {
        bool res = ejecutarSecuencial(target, longitud, total_comb, tiempo_salida, idx_encontrado);
        nodo_ganador = 0;
        hilo_ganador = 0;
        return res;
    }

    // 1. Segmentacion equitativa del espacio de busqueda [0, total_comb - 1]
    unsigned long long* rangos_ini = new unsigned long long[num_esclavos];
    unsigned long long* rangos_fin = new unsigned long long[num_esclavos];

    unsigned long long base = total_comb / (unsigned long long)num_esclavos;
    unsigned long long rem = total_comb % (unsigned long long)num_esclavos;

    unsigned long long cur_start = 0;
    for (int i = 0; i < num_esclavos; i++) {
        unsigned long long sz = (i < (int)rem) ? (base + 1ULL) : base;
        rangos_ini[i] = cur_start;
        rangos_fin[i] = (sz > 0) ? (cur_start + sz - 1ULL) : cur_start;
        cur_start += sz;
    }

    // Registrar en log la distribucion de rangos
    std::ostringstream oss;
    oss << "[MAESTRO - Nodo 0] Espacio total de combinaciones: " << total_comb << "\n";
    oss << "[MAESTRO - Nodo 0] Distribuyendo rangos a " << num_esclavos << " nodos esclavos:\n";
    for (int i = 0; i < num_esclavos; i++) {
        int target_rank = i + 1;
        oss << "  - Esclavo " << target_rank << ": [" << rangos_ini[i] << " a " << rangos_fin[i] 
            << "] (" << (rangos_fin[i] - rangos_ini[i] + 1ULL) << " combinaciones)\n";
    }
    registrar_log(oss.str(), true);

    // Enviar rangos a cada esclavo
    for (int i = 0; i < num_esclavos; i++) {
        int target_rank = i + 1;
        MPI_Send(&rangos_ini[i], 1, MPI_UNSIGNED_LONG_LONG, target_rank, TAG_RANGO_INICIO, MPI_COMM_WORLD);
        MPI_Send(&rangos_fin[i], 1, MPI_UNSIGNED_LONG_LONG, target_rank, TAG_RANGO_FIN, MPI_COMM_WORLD);
    }

    // 2. Escucha no bloqueante de señales (Early Termination)
    bool clave_descifrada = false;
    int esclavos_finalizados = 0;
    bool* esclavo_activo = new bool[num_esclavos];
    for (int i = 0; i < num_esclavos; i++) esclavo_activo[i] = true;

    while (!clave_descifrada && esclavos_finalizados < num_esclavos) {
        int hay_mensaje = 0;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &hay_mensaje, &status);

        if (hay_mensaje) {
            int src = status.MPI_SOURCE;
            int tag = status.MPI_TAG;

            if (tag == TAG_HALLAZGO) {
                // Notificacion de éxito recibida
                unsigned long long info[3];
                MPI_Recv(info, 3, MPI_UNSIGNED_LONG_LONG, src, TAG_HALLAZGO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                double t_fin = MPI_Wtime();
                tiempo_salida = t_fin - t_ini;
                clave_descifrada = true;
                nodo_ganador = (int)info[0];
                hilo_ganador = (int)info[1];
                idx_encontrado = info[2];

                char* cand_buffer = new char[longitud + 1];
                convertirIndiceATexto(idx_encontrado, cand_buffer, longitud);

                std::ostringstream oss_succ;
                oss_succ << "\n========================================================================\n";
                oss_succ << ">>> [MAESTRO] CLAVE LOCALIZADA POR ESCLAVO " << nodo_ganador 
                         << " (Hilo OpenMP " << hilo_ganador << ") <<<\n";
                oss_succ << "Clave descifrada: \"" << cand_buffer << "\" en Indice Global: " << idx_encontrado << "\n";
                oss_succ << "Tiempo transcurrido: " << std::fixed << std::setprecision(6) << tiempo_salida << " s\n";
                oss_succ << "Propagando orden de cancelacion temprana (Early Termination) a los demas nodos...\n";
                oss_succ << "========================================================================\n\n";
                registrar_log(oss_succ.str(), true);
                delete[] cand_buffer;

                // Propagar orden de parada temprana a TODOS los demas esclavos
                int stop_signal = 1;
                for (int i = 0; i < num_esclavos; i++) {
                    int r = i + 1;
                    if (r != nodo_ganador && esclavo_activo[i]) {
                        MPI_Send(&stop_signal, 1, MPI_INT, r, TAG_PARADA_TEMPRANA, MPI_COMM_WORLD);
                    }
                }
            } else if (tag == TAG_FINALIZADO) {
                int dummy = 0;
                MPI_Recv(&dummy, 1, MPI_INT, src, TAG_FINALIZADO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int idx_esclavo = src - 1;
                if (esclavo_activo[idx_esclavo]) {
                    esclavo_activo[idx_esclavo] = false;
                    esclavos_finalizados++;
                }
            }
        } else {
            // Pequeno descanso para evitar 100% de ocupacion en el bucle de sondeo
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
    }

    // Esperar a que los esclavos restantes confirmen su finalizacion limpia
    while (esclavos_finalizados < num_esclavos) {
        int hay_msg = 0;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, TAG_FINALIZADO, MPI_COMM_WORLD, &hay_msg, &status);
        if (hay_msg) {
            int dummy = 0;
            MPI_Recv(&dummy, 1, MPI_INT, status.MPI_SOURCE, TAG_FINALIZADO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            int idx_esclavo = status.MPI_SOURCE - 1;
            if (esclavo_activo[idx_esclavo]) {
                esclavo_activo[idx_esclavo] = false;
                esclavos_finalizados++;
            }
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
    }

    delete[] rangos_ini;
    delete[] rangos_fin;
    delete[] esclavo_activo;

    if (!clave_descifrada) {
        tiempo_salida = MPI_Wtime() - t_ini;
    }

    return clave_descifrada;
}

void FuerzaBruta::ejecutarEsclavo(const char* target, int longitud) {
    // 1. Recepcion del rango asignado por el Maestro
    unsigned long long inicio_local = 0, fin_local = 0;
    MPI_Recv(&inicio_local, 1, MPI_UNSIGNED_LONG_LONG, 0, TAG_RANGO_INICIO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&fin_local, 1, MPI_UNSIGNED_LONG_LONG, 0, TAG_RANGO_FIN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    unsigned long long cant_local = (fin_local >= inicio_local) ? (fin_local - inicio_local + 1ULL) : 0;

    std::ostringstream oss;
    oss << "[Nodo MPI: " << rank << "] Rango recibido del Maestro: [" << inicio_local << " a " << fin_local 
        << "] (" << cant_local << " combinaciones asignadas). Iniciando busqueda con " 
        << max_threads << " hilos OpenMP.\n";
    registrar_log(oss.str(), true);

    // 2. Gestion de memoria dinamica para los buffers de los hilos
    char** buffers_hilos = new char*[max_threads];
    for (int t = 0; t < max_threads; t++) {
        buffers_hilos[t] = new char[longitud + 1];
    }

    bool encontrado_local = false;
    bool parada_solicitada = false;

    // 3. Paralelismo Hibrido Multihilo con OpenMP (Distribucion dinamica)
    #pragma omp parallel for schedule(dynamic, 1000) shared(encontrado_local, parada_solicitada)
    for (unsigned long long idx = inicio_local; idx <= fin_local; idx++) {
        if (encontrado_local || parada_solicitada) {
            continue;
        }

        int tid = omp_get_thread_num();
        char* cand = buffers_hilos[tid];
        convertirIndiceATexto(idx, cand, longitud);

        // Impresion y registro detallado durante pruebas de 3 caracteres
        if (debug_mode && (idx % 200 == 0 || idx == inicio_local)) {
            std::ostringstream oss_det;
            oss_det << "[Equipo: " << processor_name << "] [Nodo MPI: " << rank 
                    << "] [Hilo OpenMP: " << tid << "] [Candidato: " << cand << "] [Estado: Evaluando]\n";
            registrar_log(oss_det.str(), true);
        }

        // Comparacion de la clave
        if (std::strcmp(cand, target) == 0) {
            #pragma omp critical(notificar_hallazgo_maestro)
            {
                if (!encontrado_local) {
                    encontrado_local = true;

                    std::ostringstream oss_succ;
                    oss_succ << "\n[EXITO] [Equipo: " << processor_name << "] [Nodo MPI: " << rank 
                             << "] [Hilo OpenMP: " << tid << "] [Clave Encontrada: \"" << cand 
                             << "\"] [Indice Global: " << idx << "]\n";
                    registrar_log(oss_succ.str(), true);

                    // Notificacion inmediata al Maestro (Early Termination)
                    unsigned long long info[3];
                    info[0] = (unsigned long long)rank;
                    info[1] = (unsigned long long)tid;
                    info[2] = idx;
                    MPI_Send(info, 3, MPI_UNSIGNED_LONG_LONG, 0, TAG_HALLAZGO, MPI_COMM_WORLD);
                }
            }
        }

        // Verificacion periodica de señal de parada temprana del Maestro
        if (idx % 5000 == 0) {
            int hay_orden = 0;
            MPI_Status st;
            MPI_Iprobe(0, TAG_PARADA_TEMPRANA, MPI_COMM_WORLD, &hay_orden, &st);
            if (hay_orden) {
                int dummy = 0;
                MPI_Recv(&dummy, 1, MPI_INT, 0, TAG_PARADA_TEMPRANA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                parada_solicitada = true;
                encontrado_local = true;

                std::ostringstream oss_stop;
                oss_stop << "[Nodo MPI: " << rank << "] [AVISO] Orden de parada temprana recibida del Maestro. Cancelando hilos OpenMP.\n";
                registrar_log(oss_stop.str(), true);
            }
        }
    }

    // 4. Liberacion estricta de memoria dinamica de los hilos
    for (int t = 0; t < max_threads; t++) {
        delete[] buffers_hilos[t];
    }
    delete[] buffers_hilos;

    // 5. Notificar finalizacion limpia al Maestro
    int fin_signal = 1;
    MPI_Send(&fin_signal, 1, MPI_INT, 0, TAG_FINALIZADO, MPI_COMM_WORLD);

    std::ostringstream oss_end;
    oss_end << "[Nodo MPI: " << rank << "] Finalizacion limpia y liberacion de memoria completada.\n";
    registrar_log(oss_end.str(), true);
}
