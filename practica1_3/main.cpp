#include <mpi.h>
#include <iostream>
#include <sstream>
#include "OperacionesArreglos.h"

void print_menu(int rank) {
    if (rank == 0) {
        std::cout << "\n=== MENU INTERACTIVO ===\n";
        std::cout << "1. Crear arreglos\n";
        std::cout << "2. Sumar arreglos\n";
        std::cout << "3. Restar arreglos\n";
        std::cout << "4. Multiplicar arreglos\n";
        std::cout << "5. Calcular el cuadrado de un arreglo (A)\n";
        std::cout << "6. Llenar secuencial\n";
        std::cout << "7. Llenar aleatorio\n";
        std::cout << "8. Sumatoria (MPI_Reduce)\n";
        std::cout << "9. Promedio (MPI_Allreduce)\n";
        std::cout << "10. Maximo (MPI_Reduce)\n";
        std::cout << "11. Minimo (MPI_Allreduce)\n";
        std::cout << "12. Salir\n";
        std::cout << "Seleccione una opcion: ";
    }
}

int main(int argc, char** argv) {
    // Inicializar entorno MPI
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    if (rank == 0) {
        std::cout << "========================================================\n";
        std::cout << "Integrantes del Equipo:\n";
        // NOTA PARA LOS ESTUDIANTES: Reemplacen con sus nombres reales en orden alfabetico
        std::cout << "- Anguiano Garcia Angel Yahir Guadalupe\n";
        std::cout << "- Figueroa Robles Axel Israel\n";
        std::cout << "========================================================\n";
    }

    OperacionesArreglos ops(rank, num_procs, processor_name);
    
    long long global_size = 40;
    if (rank == 0) {
        std::cout << "Ingrese el tamaño global del arreglo (Ej. 40 o 4000000): ";
        std::cin >> global_size;
    }
    
    // Transmitir el tamaño global a todos los procesos
    MPI_Bcast(&global_size, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    
    // Activar mensajes detallados sólo para arreglos pequeños (<= 100)
    bool debug_mode = (global_size <= 100);
    ops.set_debug_mode(debug_mode);

    int opcion = 0;
    while (opcion != 12) {
        if (rank == 0) {
            print_menu(rank);
            std::cin >> opcion;
        }
        
        // El maestro notifica a los demás la opción seleccionada
        MPI_Bcast(&opcion, 1, MPI_INT, 0, MPI_COMM_WORLD);

        double start_time = MPI_Wtime();

        switch (opcion) {
            case 1:
                ops.crearArregloMPI(global_size);
                break;
            case 2:
                ops.sumarArreglos();
                ops.recopilarResultadosC(); // Demostración de uso de MPI_Gather
                break;
            case 3:
                ops.restarArreglos();
                ops.recopilarResultadosC();
                break;
            case 4:
                ops.multiplicarArreglos();
                ops.recopilarResultadosC();
                break;
            case 5:
                ops.cuadradoArreglo();
                ops.recopilarResultadosC();
                break;
            case 6:
                ops.llenarSecuencial();
                break;
            case 7:
                ops.llenarAleatorio();
                break;
            case 8: {
                long long local_sum = ops.sumatoriaLocal();
                long long global_sum = 0;
                // Uso de MPI_Reduce: de workers a master
                MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
                if (rank == 0) {
                    std::cout << "-> Resultado Sumatoria Global: " << global_sum << "\n";
                }
                break;
            }
            case 9: {
                long long local_sum = ops.sumatoriaLocal();
                long long global_sum = 0;
                // Uso de MPI_Allreduce: todos conocen el resultado
                MPI_Allreduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
                double promedio = (double)global_sum / global_size;
                if (rank == 0) {
                    std::cout << "-> Resultado Promedio Global: " << promedio << " (conocido en todos los nodos)\n";
                }
                break;
            }
            case 10: {
                int local_max = ops.maximoLocal();
                int global_max = 0;
                // Uso de MPI_Reduce
                MPI_Reduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
                if (rank == 0) {
                    std::cout << "-> Resultado Máximo Global: " << global_max << "\n";
                }
                break;
            }
            case 11: {
                int local_min = ops.minimoLocal();
                int global_min = 0;
                // Uso de MPI_Allreduce
                MPI_Allreduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
                if (rank == 0) {
                    std::cout << "-> Resultado Mínimo Global: " << global_min << " (conocido en todos los nodos)\n";
                }
                break;
            }
            case 12:
                if (rank == 0) std::cout << "Saliendo...\n";
                break;
            default:
                if (rank == 0) std::cout << "Opcion no valida.\n";
                break;
        }

        double end_time = MPI_Wtime();
        double elapsed_time = end_time - start_time;

        if (opcion >= 1 && opcion <= 11) {
            std::ostringstream time_msg;
            time_msg << "[Equipo: " << processor_name << "] [Proceso MPI: " << rank 
                     << "] [Tiempo Operación " << opcion << "]: " << elapsed_time << " segundos.\n";
            ops.registrar_log(time_msg.str());
        }

        // Sincronización para que el menú principal del maestro no se imprima 
        // mientras otros procesos siguen escupiendo logs
        MPI_Barrier(MPI_COMM_WORLD);
    }

    if (rank == 0) {
        std::cout << "========================================================\n";
        std::cout << "Integrantes del Equipo:\n";
        std::cout << "- Anguiano Garcia Angel Yahir Guadalupe\n";
        std::cout << "- Figueroa Robles Axel Israel\n";
        std::cout << "========================================================\n";
    }

    MPI_Finalize();
    return 0;
}
