#include <iostream>
#include <cstdlib>
#include <windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    int opc = 0;
    do {
        std::cout << "\n======================================================================\n";
        std::cout << "           ACTIVIDAD 1.2: CARGA DE MEMORIA DISTRIBUIDA EN MPI         \n";
        std::cout << "                     LANZADOR DE PROCESOS MPI (5 NODOS)               \n";
        std::cout << "======================================================================\n";
        std::cout << " 1. Ejecutar Versión 1: MPI_Send / MPI_Recv (Interactivo, 5 Procesos)\n";
        std::cout << " 2. Ejecutar Versión 2: MPI_Scatter / MPI_Gather (Interactivo, 5 Procesos)\n";
        std::cout << " 3. Ejecutar DEMO COMPLETA AUTOMATIZADA (Pruebas 40 elem y 4,000,000 elem)\n";
        std::cout << " 4. Salir\n";
        std::cout << "======================================================================\n";
        std::cout << "Seleccione una opcion [1-4]: ";
        std::cin >> opc;

        switch(opc) {
            case 1:
                std::cout << "\n>>> Iniciando Versión 1 con 5 procesos MPI (1 Maestro + 4 Trabajadores)... <<<\n";
                system("mpiexec -n 5 .\\main_send_recv.exe");
                break;
            case 2:
                std::cout << "\n>>> Iniciando Versión 2 con 5 procesos MPI (1 Maestro + 4 Trabajadores)... <<<\n";
                system("mpiexec -n 5 .\\main_scatter_gather.exe");
                break;
            case 3: {
                std::cout << "\n======================================================================\n";
                std::cout << ">>> EJECUTANDO DEMOSTRACIÓN AUTOMÁTICA COMPLETA PARA VIDEO / REPORTE <<<\n";
                std::cout << "======================================================================\n";
                
                std::cout << "\n--- [1/2] PRUEBA VERSIÓN 1: MPI_Send / MPI_Recv ---\n";
                system("powershell -Command \"'1`n2`n3`n4`n5`n6`n1`n2`n3`n4`n5`n7' | mpiexec -n 5 .\\main_send_recv.exe\"");

                std::cout << "\n--- [2/2] PRUEBA VERSIÓN 2: MPI_Scatter / MPI_Gather ---\n";
                system("powershell -Command \"'1`n2`n3`n4`n5`n6`n1`n2`n3`n4`n5`n7' | mpiexec -n 5 .\\main_scatter_gather.exe\"");

                std::cout << "\n>>> DEMOSTRACIÓN AUTOMÁTICA FINALIZADA <<<\n";
                break;
            }
            case 4:
                std::cout << "\nSaliendo del lanzador...\n";
                break;
            default:
                std::cout << "\nOpción inválida. Intente de nuevo.\n";
        }
    } while (opc != 4);

    return 0;
}
