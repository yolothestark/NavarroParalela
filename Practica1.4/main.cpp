/*
====================================================================
    Práctica 1.4: Sincronización Cliente-Servidor en MPI con OpenMP
    Equipo:
    - AXEL ISRAEL FIGUEROA ROBLES
    - ANGEL YAHIR GUADALUPE ANGUIANO GARCIA
====================================================================
*/
#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <random>

// Estructuras de Datos
struct Transaccion {
    int id_cliente;        // Rank del proceso MPI cliente
    int id_hilo_openmp;    // Hilo de OpenMP que originó la solicitud
    int tipo_operacion;    // 1: Depósito, 2: Retiro, 3: Consulta, 0: Finalizar
    double monto;          // Monto a depositar o retirar
};

struct RespuestaServidor {
    bool aprobada;         // true si la transacción fue exitosa
    double saldo_resultante;// Saldo actualizado tras la operación
    char mensaje[100];     // Mensaje descriptivo del servidor
};

// Gestión de memoria dinámica para el historial (Lista enlazada)
// Esto cumple la restricción de NO usar std::vector ni arreglos estáticos
struct NodoTransaccion {
    int id_cliente;
    int id_hilo_openmp;
    int tipo_operacion;
    double monto;
    bool aprobada;
    double saldo_resultante;
    NodoTransaccion* siguiente;
};

class HistorialTransacciones {
private:
    NodoTransaccion* cabeza;
    NodoTransaccion* cola;
public:
    HistorialTransacciones() : cabeza(nullptr), cola(nullptr) {}
    ~HistorialTransacciones() { limpiar(); }
    
    void agregar(int id_c, int id_h, int tipo_op, double monto, bool aprobada, double saldo) {
        NodoTransaccion* nuevo = new NodoTransaccion;
        nuevo->id_cliente = id_c;
        nuevo->id_hilo_openmp = id_h;
        nuevo->tipo_operacion = tipo_op;
        nuevo->monto = monto;
        nuevo->aprobada = aprobada;
        nuevo->saldo_resultante = saldo;
        nuevo->siguiente = nullptr;
        
        if (!cabeza) {
            cabeza = cola = nuevo;
        } else {
            cola->siguiente = nuevo;
            cola = nuevo;
        }
    }
    
    void limpiar() {
        NodoTransaccion* actual = cabeza;
        while (actual) {
            NodoTransaccion* aux = actual;
            actual = actual->siguiente;
            delete aux;
        }
        cabeza = cola = nullptr;
    }
    
    void escribir_a_archivo(FILE* file) {
        NodoTransaccion* actual = cabeza;
        while (actual) {
            const char* op_str = (actual->tipo_operacion == 1) ? "Depósito" : (actual->tipo_operacion == 2) ? "Retiro" : "Consulta";
            const char* est_str = actual->aprobada ? "Aprobado" : "Rechazado";
            fprintf(file, "[Equipo: PruebaNavarro] [Nodo MPI: %d] [Hilo OpenMP: %d] [Op: %s] [Monto: $%.2f] [Estado: %s] [Saldo: $%.2f]\n", 
                    actual->id_cliente, actual->id_hilo_openmp, op_str, actual->monto, est_str, actual->saldo_resultante);
            actual = actual->siguiente;
        }
    }
};

void ejecutar_simulacion(int rank, int size, const char* processor_name, int num_ops_por_cliente, bool detallado, double& saldo_cuenta) {
    char filename[256];
    sprintf(filename, "log_equipo_PruebaNavarro_%s_nodo_%d.txt", processor_name, rank);
    FILE* log_file = fopen(filename, "a");
    if (!log_file) {
        printf("Error al abrir archivo log en nodo %d\n", rank);
        return;
    }
    
    HistorialTransacciones historial;
    
    if (rank == 0) {
        // Lógica del Servidor
        int clientes_activos = size - 1;
        int transacciones_procesadas = 0;
        double start_time = MPI_Wtime();
        
        fprintf(log_file, "--- Nueva simulacion iniciada en Servidor (Equipo: PruebaNavarro) ---\n");
        
        while (clientes_activos > 0) {
            Transaccion t;
            MPI_Status status;
            // Procesamiento atómico solicitud por solicitud
            MPI_Recv(&t, sizeof(Transaccion), MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            
            if (t.tipo_operacion == 0) {
                clientes_activos--;
            } else {
                RespuestaServidor r;
                r.aprobada = true;
                
                if (t.tipo_operacion == 1) { 
                    saldo_cuenta += t.monto;
                    strcpy(r.mensaje, "Depósito aprobado");
                } else if (t.tipo_operacion == 2) { 
                    if (saldo_cuenta >= t.monto) {
                        saldo_cuenta -= t.monto;
                        strcpy(r.mensaje, "Retiro aprobado");
                    } else {
                        r.aprobada = false;
                        strcpy(r.mensaje, "Fondos insuficientes");
                    }
                } else if (t.tipo_operacion == 3) { 
                    strcpy(r.mensaje, "Consulta exitosa");
                }
                r.saldo_resultante = saldo_cuenta;
                transacciones_procesadas++;
                
                historial.agregar(t.id_cliente, t.id_hilo_openmp, t.tipo_operacion, t.monto, r.aprobada, r.saldo_resultante);
                
                if (detallado) {
                    const char* op_str = (t.tipo_operacion == 1) ? "Depósito" : (t.tipo_operacion == 2) ? "Retiro" : "Consulta";
                    const char* est_str = r.aprobada ? "Aprobado" : "Rechazado";
                    printf("[Equipo: PruebaNavarro] [Nodo MPI: %d] [Hilo OpenMP: %d] [Op: %s] [Monto: $%.2f] [Estado: %s] [Saldo final: $%.2f]\n", 
                            t.id_cliente, t.id_hilo_openmp, op_str, t.monto, est_str, r.saldo_resultante);
                }
                
                MPI_Send(&r, sizeof(RespuestaServidor), MPI_BYTE, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            }
        }
        double end_time = MPI_Wtime();
        
        historial.escribir_a_archivo(log_file);
        fprintf(log_file, "Transacciones procesadas: %d, Tiempo: %f seg, Saldo Final: $%.2f\n\n", transacciones_procesadas, end_time - start_time, saldo_cuenta);
        
        if (!detallado) {
            printf("\n--- Resumen de Simulación Masiva ---\n");
            printf("Tiempo total: %f segundos\n", end_time - start_time);
            printf("Transacciones procesadas: %d\n", transacciones_procesadas);
            printf("Saldo consolidado: $%.2f\n", saldo_cuenta);
        }
    } else {
        // Lógica del Cliente
        double start_time = MPI_Wtime();
        int num_hilos_detectados = 0;
        
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            if (tid == 0) {
                num_hilos_detectados = omp_get_num_threads();
            }
            
            // Generador de números aleatorios seguro para hilos
            std::mt19937 rng(time(NULL) + rank * 100 + tid);
            std::uniform_int_distribution<int> op_dist(1, 3);
            std::uniform_real_distribution<double> monto_dist(1.0, 1000.0);
            
            #pragma omp for
            for (int i = 0; i < num_ops_por_cliente; ++i) {
                Transaccion t;
                t.id_cliente = rank;
                t.id_hilo_openmp = tid;
                t.tipo_operacion = op_dist(rng); 
                if (t.tipo_operacion == 3) t.monto = 0.0;
                else t.monto = monto_dist(rng);
                
                RespuestaServidor r;
                
                // Zona crítica para enviar y recibir de forma síncrona
                #pragma omp critical
                {
                    MPI_Send(&t, sizeof(Transaccion), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
                    MPI_Recv(&r, sizeof(RespuestaServidor), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    
                    historial.agregar(rank, tid, t.tipo_operacion, t.monto, r.aprobada, r.saldo_resultante);
                }
            }
        }
        
        // Enviar señal de término al servidor
        Transaccion t_fin;
        t_fin.id_cliente = rank;
        t_fin.id_hilo_openmp = 0;
        t_fin.tipo_operacion = 0;
        t_fin.monto = 0.0;
        MPI_Send(&t_fin, sizeof(Transaccion), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
        
        double end_time = MPI_Wtime();
        
        fprintf(log_file, "--- Nueva simulacion iniciada (Equipo: PruebaNavarro, Rango MPI: %d, Hilos OpenMP detectados: %d) ---\n", rank, num_hilos_detectados);
        historial.escribir_a_archivo(log_file);
        fprintf(log_file, "Operaciones locales generadas: %d, Tiempo: %f seg\n\n", num_ops_por_cliente, end_time - start_time);
    }
    
    fclose(log_file);
}

void mostrar_menu() {
    printf("\n==== MENU PRINCIPAL ====\n");
    printf("Desarrollado por:\n");
    printf("- AXEL ISRAEL FIGUEROA ROBLES\n");
    printf("- ANGEL YAHIR GUADALUPE ANGUIANO GARCIA\n");
    printf("========================\n");
    printf("1. Iniciar Simulacion Basica (5 operaciones por cliente)\n");
    printf("2. Iniciar Simulacion Masiva (100,000 operaciones por cliente)\n");
    printf("3. Consultar Saldo Final del Servidor\n");
    printf("4. Salir\n");
    printf("Seleccione una opcion: ");
}

int main(int argc, char** argv) {
    int provided;
    // Iniciamos con soporte serializado para que OpenMP pueda usar MPI en zonas críticas
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
    
    if (provided < MPI_THREAD_SERIALIZED) {
        printf("Advertencia: El soporte de hilos MPI es menor al requerido.\n");
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    
    double saldo_cuenta = 10000.00; // Saldo inicial base
    int opcion = 0;
    
    do {
        if (rank == 0) {
            mostrar_menu();
            fflush(stdout);
            if (scanf("%d", &opcion) != 1) {
                while(getchar() != '\n'); // Limpiar buffer en caso de letra
                opcion = 0; // Inválido, repetir menú
            }
        }
        // Broadcast de la opción elegida a todos los clientes
        MPI_Bcast(&opcion, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        if (opcion == 1) {
            if (rank == 0) printf("\nIniciando Simulacion Basica...\n");
            ejecutar_simulacion(rank, size, processor_name, 5, true, saldo_cuenta);
        } else if (opcion == 2) {
            if (rank == 0) printf("\nIniciando Simulacion Masiva...\n");
            ejecutar_simulacion(rank, size, processor_name, 100000, false, saldo_cuenta);
        } else if (opcion == 3) {
            if (rank == 0) {
                printf("\n================================\n");
                printf("Saldo final consolidado: $%.2f\n", saldo_cuenta);
                printf("================================\n");
            }
        }
    } while (opcion != 4);
    
    if (rank == 0) {
        printf("\nSaliendo del programa...\n");
        printf("====================================================\n");
        printf("  Trabajo realizado por:\n");
        printf("  - AXEL ISRAEL FIGUEROA ROBLES\n");
        printf("  - ANGEL YAHIR GUADALUPE ANGUIANO GARCIA\n");
        printf("====================================================\n");
    }
    
    MPI_Finalize();
    return 0;
}
