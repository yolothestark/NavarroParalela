#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <sstream>
#include "FuerzaBruta.h"

// Función auxiliar para imprimir el encabezado oficial del equipo
void imprimir_encabezado_equipo(int rank) {
    if (rank == 0) {
        std::cout << "\n========================================================================\n";
        std::cout << "  CENTRO UNIVERSITARIO DE TONALA (CUTONALA) - UDG\n";
        std::cout << "  MATERIA: COMPUTACION PARALELA\n";
        std::cout << "  PRACTICA 1.6: ALGORITMO DE FUERZA BRUTA DISTRIBUIDO (MPI + OPENMP)\n";
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
        std::cout << "  FINALIZACION EXITOSA DE LA ACTIVIDAD 1.6\n";
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
        std::cout << " 1. Ingresar Clave Personalizada (A-Z, 0-9)\n";
        std::cout << " 2. Ejecutar Busqueda Secuencial (1 Nodo, 1 Hilo - Linea Base)\n";
        std::cout << " 3. Ejecutar Busqueda Paralela Distribuida (MPI + OpenMP + Early Term.)\n";
        std::cout << " 4. Ejecutar Secuencia de Pruebas de Escalabilidad (3 vs 10/5 caracteres)\n";
        std::cout << " 5. Mostrar Tabla Comparativa de Rendimiento (Speedup y Eficiencia)\n";
        std::cout << " 6. Salir del Sistema\n";
        std::cout << "------------------------------------------------------------------------\n";
        std::cout << " Seleccione una opcion [1-6]: ";
    }
}

int main(int argc, char* argv[]) {
    // 1. Inicializacion de MPI
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // INSTRUCCION ESPECIAL OBLIGATORIA: Nombres como PRIMERA linea de ejecucion
    imprimir_encabezado_equipo(rank);

    FuerzaBruta fb(rank, num_procs, processor_name, "Equipo_Giallo");

    // Clave y estado gestionados dinamicamente
    char* clave_objetivo = new char[128];
    std::strcpy(clave_objetivo, "X7B"); // Clave inicial por defecto de 3 caracteres
    int longitud_clave = 3;

    // Métricas históricas para comparativas
    double t_secuencial_base = 0.0;
    double t_paralelo_dist = 0.0;
    int ultimo_nodo_ganador = -1;
    int ultimo_hilo_ganador = -1;
    unsigned long long ultimo_idx = 0;
    char* ultima_clave_probada = new char[128];
    std::strcpy(ultima_clave_probada, "X7B");

    int opcion = 0;

    while (opcion != 6) {
        if (rank == 0) {
            mostrar_menu(rank);
            std::cin >> opcion;
            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                opcion = 0;
            }
        }

        // Difusión síncrona de la opción elegida
        MPI_Bcast(&opcion, 1, MPI_INT, 0, MPI_COMM_WORLD);

        switch (opcion) {
            case 1: { // Ingresar Clave Personalizada
                if (rank == 0) {
                    std::cout << "\n>>> INGRESO DE CLAVE PERSONALIZADA <<<\n";
                    std::cout << "Reglas: Caracteres estrictamente alfanumericos en MAYUSCULAS (A-Z, 0-9).\n";
                    std::cout << "Ingrese la clave a descifrar (ej. K9B, TONALA, 7X9B): ";
                    std::string input;
                    std::cin >> input;

                    // Convertir a mayusculas
                    for (char& c : input) c = toupper(c);

                    if (FuerzaBruta::validarClave(input.c_str(), (int)input.length())) {
                        std::strcpy(clave_objetivo, input.c_str());
                        longitud_clave = (int)input.length();
                        std::strcpy(ultima_clave_probada, clave_objetivo);

                        unsigned long long esp = FuerzaBruta::calcularEspacioTotal(longitud_clave);
                        unsigned long long idx_real = FuerzaBruta::convertirTextoAIndice(clave_objetivo, longitud_clave);

                        std::cout << "[OK] Clave valida: \"" << clave_objetivo << "\" (Longitud: " << longitud_clave << ")\n";
                        std::cout << "     Espacio total de combinaciones (36^" << longitud_clave << "): " << esp << "\n";
                        std::cout << "     Indice matematico exacto: " << idx_real << "\n";
                    } else {
                        std::cout << "[ERROR] La clave contiene caracteres no permitidos. Solo se admiten A-Z y 0-9.\n";
                    }
                }

                // Sincronizar clave con todos los nodos
                MPI_Bcast(&longitud_clave, 1, MPI_INT, 0, MPI_COMM_WORLD);
                MPI_Bcast(clave_objetivo, longitud_clave + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
                break;
            }

            case 2: { // Ejecutar Búsqueda Secuencial (Línea Base)
                unsigned long long total_comb = FuerzaBruta::calcularEspacioTotal(longitud_clave);

                if (rank == 0) {
                    std::cout << "\n>>> INICIANDO BUSQUEDA SECUENCIAL (1 NODO, 1 HILO) <<<\n";
                    std::cout << "Clave objetivo: \"" << clave_objetivo << "\" (Longitud: " << longitud_clave << ")\n";
                    std::cout << "Espacio de combinaciones: " << total_comb << "\n";
                    std::cout << "Evaluando secuencialmente desde el indice 0...\n";

                    double t_sec = 0.0;
                    unsigned long long idx_hallado = 0;
                    bool exito = fb.ejecutarSecuencial(clave_objetivo, longitud_clave, total_comb, t_sec, idx_hallado);

                    t_secuencial_base = t_sec;
                    std::strcpy(ultima_clave_probada, clave_objetivo);

                    if (exito) {
                        std::cout << "\n[EXITO SECUENCIAL] Clave \"" << clave_objetivo 
                                  << "\" descifrada en indice: " << idx_hallado << "\n";
                        std::cout << "Tiempo de ejecucion secuencial: " << std::fixed << std::setprecision(6) 
                                  << t_sec << " segundos.\n";
                    } else {
                        std::cout << "[FALLO] No se encontro la clave en el espacio de busqueda.\n";
                    }
                }
                break;
            }

            case 3: { // Ejecutar Búsqueda Paralela Distribuida (MPI + OpenMP + Early Termination)
                unsigned long long total_comb = FuerzaBruta::calcularEspacioTotal(longitud_clave);
                fb.set_debug_mode(longitud_clave <= 3);

                if (rank == 0) {
                    std::cout << "\n>>> INICIANDO BUSQUEDA PARALELA DISTRIBUIDA (MPI + OPENMP) <<<\n";
                    std::cout << "Clave objetivo: \"" << clave_objetivo << "\" (Longitud: " << longitud_clave << ")\n";
                    std::cout << "Total de Procesos MPI: " << num_procs << " (1 Maestro + " << (num_procs - 1) << " Esclavos)\n";
                    std::cout << "Mecanismo de cancelacion: Parada Temprana Activa (Early Termination)\n\n";

                    double t_par = 0.0;
                    int ganador_rank = -1;
                    int ganador_tid = -1;
                    unsigned long long idx_hallado = 0;

                    bool exito = fb.ejecutarMaestro(clave_objetivo, longitud_clave, total_comb,
                                                    t_par, ganador_rank, ganador_tid, idx_hallado);

                    t_paralelo_dist = t_par;
                    ultimo_nodo_ganador = ganador_rank;
                    ultimo_hilo_ganador = ganador_tid;
                    ultimo_idx = idx_hallado;
                    std::strcpy(ultima_clave_probada, clave_objetivo);

                    if (exito) {
                        std::cout << "\n========================================================================\n";
                        std::cout << "                  RESULTADOS DE LA FUERZA BRUTA                         \n";
                        std::cout << "========================================================================\n";
                        std::cout << "Clave descifrada con exito:  \"" << clave_objetivo << "\"\n";
                        std::cout << "Nodo MPI que la encontro:    " << ganador_rank << "\n";
                        std::cout << "Hilo OpenMP responsable:     " << ganador_tid << "\n";
                        std::cout << "Indice global correspondiente: " << idx_hallado << "\n";
                        std::cout << "Tiempo paralelo distribuido: " << std::fixed << std::setprecision(6) << t_par << " s\n";
                        if (t_secuencial_base > 0.0) {
                            double sp = t_secuencial_base / t_par;
                            std::cout << "Speedup obtenido:            " << sp << "x\n";
                            std::cout << "Eficiencia del cluster:      " << (sp / num_procs) * 100.0 << "%\n";
                        }
                        std::cout << "========================================================================\n";
                    }
                } else {
                    // Procesos Esclavos ejecutan la búsqueda en sus hilos OpenMP
                    fb.ejecutarEsclavo(clave_objetivo, longitud_clave);
                }
                break;
            }

            case 4: { // Secuencia Automatizada de Escalabilidad (3 caracteres vs Análisis Masivo)
                fb.set_debug_mode(true);

                // --- FASE 1: Prueba de Control de 3 Caracteres (ej. "X7B") ---
                const char* clave_control = "X7B";
                int len_ctrl = 3;
                unsigned long long total_ctrl = FuerzaBruta::calcularEspacioTotal(len_ctrl);

                if (rank == 0) {
                    std::cout << "\n========================================================================\n";
                    std::cout << "   FASE 1: PRUEBA DE CONTROL (3 CARACTERES - EJEMPLO \"X7B\")            \n";
                    std::cout << "========================================================================\n";
                    std::cout << "Proposito: Demostrar trazas detalladas por hilo, parada temprana y logs.\n";
                    std::cout << "Combinaciones totales: 36^3 = " << total_ctrl << "\n\n";

                    double t_sec = 0.0, t_par = 0.0;
                    unsigned long long idx_s = 0, idx_p = 0;
                    int win_rank = -1, win_tid = -1;

                    // Secuencial
                    fb.ejecutarSecuencial(clave_control, len_ctrl, total_ctrl, t_sec, idx_s);

                    // Distribuido
                    fb.ejecutarMaestro(clave_control, len_ctrl, total_ctrl, t_par, win_rank, win_tid, idx_p);

                    t_secuencial_base = t_sec;
                    t_paralelo_dist = t_par;
                    ultimo_nodo_ganador = win_rank;
                    ultimo_hilo_ganador = win_tid;
                    ultimo_idx = idx_p;
                    std::strcpy(ultima_clave_probada, clave_control);

                    std::cout << "\n[RESUMEN FASE 1 - 3 CARACTERES]:\n";
                    std::cout << " - Tiempo Secuencial (1 hilo): " << std::fixed << std::setprecision(6) << t_sec << " s\n";
                    std::cout << " - Tiempo Distribuido (MPI):   " << std::fixed << std::setprecision(6) << t_par << " s\n";
                    std::cout << " - Descifrado por: Nodo " << win_rank << " (Hilo OpenMP " << win_tid << ")\n";
                } else {
                    fb.ejecutarEsclavo(clave_control, len_ctrl);
                }

                // Sincronizar antes de pasar a la demostración matemática de escalabilidad
                MPI_Barrier(MPI_COMM_WORLD);

                // --- FASE 2: Justificación y Demostración de Escalabilidad Masiva ---
                if (rank == 0) {
                    std::cout << "\n========================================================================\n";
                    std::cout << "   FASE 2: ANALISIS MATEMATICO DE ESCALABILIDAD (10 VS 7 CARACTERES)    \n";
                    std::cout << "========================================================================\n";
                    std::cout << "Demostracion de por que 10 caracteres excede con creces la capacidad:\n\n";

                    unsigned long long c3 = 46656ULL;
                    unsigned long long c7 = 78364164096ULL;
                    double c10 = 3.656158440062976e15; // 36^10

                    // Suponiendo una tasa estimada de 50 millones de combinaciones/s en el cluster:
                    double tasa_est = 50000000.0;
                    double t_est_7 = (double)c7 / tasa_est;
                    double t_est_10 = c10 / tasa_est;

                    std::cout << " 1. Clave de 3 Caracteres:  36^3  = " << c3 << " comb. -> ~" 
                              << (c3 / tasa_est) << " segundos (Instantaneo)\n";
                    std::cout << " 2. Clave de 7 Caracteres:  36^7  = " << c7 << " comb. -> ~" 
                              << (t_est_7 / 60.0) << " minutos (~" << (t_est_7 / 3600.0) << " horas)\n";
                    std::cout << " 3. Clave de 10 Caracteres: 36^10 = 3,656,158,440,062,976 comb.\n";
                    std::cout << "    Tiempo estimado a 50M comb/s: ~" << (t_est_10 / 86400.0) << " DIAS ("
                              << (t_est_10 / (86400.0 * 365.25)) << " ANIOS continuos)\n\n";

                    std::cout << ">>> JUSTIFICACION FORMAL SEGUN LA RUBRICA <<<\n";
                    std::cout << "Ejecutar 10 caracteres de manera exhaustiva tomaria anios de computo.\n";
                    std::cout << "Por tanto, de acuerdo a la instruccion especial de escalabilidad,\n";
                    std::cout << "se valida y documenta el analisis matematico exponencial en el reporte PDF.\n";
                    std::cout << "========================================================================\n";
                }
                break;
            }

            case 5: { // Mostrar Tabla Comparativa de Rendimiento
                if (rank == 0) {
                    std::cout << "\n========================================================================\n";
                    std::cout << "       TABLA CONSOLIDADA DE RENDIMIENTO Y GANANCIA DE VELOCIDAD         \n";
                    std::cout << "========================================================================\n";
                    std::cout << "Ultima clave evaluada:       \"" << ultima_clave_probada << "\"\n";
                    std::cout << "Procesos MPI participantes:  " << num_procs << " (1 Maestro + " << (num_procs - 1) << " Esclavos)\n";
                    std::cout << "Nodo ganador del hallazgo:   Nodo " << ultimo_nodo_ganador 
                              << " (Hilo OpenMP " << ultimo_hilo_ganador << ")\n";
                    std::cout << "Indice numerico de la clave: " << ultimo_idx << "\n\n";

                    std::cout << std::left << std::setw(32) << "Modalidad de Ejecucion" 
                              << std::setw(18) << "Tiempo (s)" 
                              << std::setw(15) << "Speedup" 
                              << std::setw(15) << "Eficiencia" << "\n";
                    std::cout << "------------------------------------------------------------------------\n";

                    std::cout << std::left << std::setw(32) << "1. Secuencial (1 Hilo)" 
                              << std::setw(18) << std::fixed << std::setprecision(6) << t_secuencial_base 
                              << std::setw(15) << "1.00x (Ref)" 
                              << std::setw(15) << "100.0%" << "\n";

                    double speedup = (t_paralelo_dist > 0.0) ? (t_secuencial_base / t_paralelo_dist) : 0.0;
                    double eficiencia = (speedup > 0.0) ? ((speedup / num_procs) * 100.0) : 0.0;

                    std::cout << std::left << std::setw(32) << "2. Distribuido (MPI+OpenMP)" 
                              << std::setw(18) << std::fixed << std::setprecision(6) << t_paralelo_dist 
                              << std::setw(15) << (std::to_string(speedup) + "x") 
                              << std::setw(15) << (std::to_string(eficiencia) + "%") << "\n";
                    std::cout << "========================================================================\n";
                }
                break;
            }

            case 6: { // Salir
                if (rank == 0) {
                    std::cout << "\nFinalizando sesion en todos los nodos del cluster...\n";
                }
                break;
            }

            default: {
                if (rank == 0) {
                    std::cout << "[OPCION INVALIDA] Ingrese un numero entre 1 y 6.\n";
                }
                break;
            }
        }
    }

    // Liberación de memoria dinámica
    delete[] clave_objetivo;
    delete[] ultima_clave_probada;

    // Sincronización final antes de salir
    MPI_Barrier(MPI_COMM_WORLD);

    // INSTRUCCION ESPECIAL OBLIGATORIA: Nombres como ULTIMA linea de ejecucion
    imprimir_pie_equipo(rank);

    // Finalizar MPI
    MPI_Finalize();
    return 0;
}
