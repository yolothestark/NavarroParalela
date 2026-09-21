/* ============================================================================
   Actividad 1.1: Modelo de programacion y memoria en MPI
   Integrantes:
   - Anguiano Garcia Angel Yahir Guadalupe
   - Figueroa Robles Axel Israel
   ============================================================================ */
#include <iostream>
#include <mpi.h>
#include <omp.h>
#include <cstdlib>
#include <ctime>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

class ProcesadorArreglo {
private:
    int* arreglo;
    long long N_local;
    int rank;
    int size;
    string hostname;

public:
    ProcesadorArreglo(long long total_elements, int rank, int size) {
        this->rank = rank;
        this->size = size;
        
        // Determinar cuantos elementos le tocan a este proceso
        long long base = total_elements / size;
        long long residuo = total_elements % size;
        
        if (rank < residuo) {
            this->N_local = base + 1;
        } else {
            this->N_local = base;
        }
        
        // Requisito: Uso estricto de arreglo dinámico (no vector)
        arreglo = new int[N_local];
        
        // Obtener el nombre del equipo (nodo)
        char host[256];
#ifdef _WIN32
        DWORD len = sizeof(host);
        GetComputerNameA(host, &len);
#else
        gethostname(host, sizeof(host));
#endif
        hostname = string(host);
    }
    
    ~ProcesadorArreglo() {
        delete[] arreglo;
    }
    
    void llenar() {
        long long elements_completed = 0;
        int thresholds_printed = 0;

        // OpenMP: paralelizar el trabajo dentro de este nodo/proceso
        #pragma omp parallel
        {
            // Semilla diferente por proceso y por hilo
            unsigned int semilla = time(NULL) ^ rank ^ omp_get_thread_num();
            
            #pragma omp for
            for (long long i = 0; i < N_local; i++) {
                // Generacion aleatoria (LCG simple para ser thread-safe)
                semilla = (semilla * 1103515245 + 12345) % 2147483648;
                arreglo[i] = semilla % 1000; // Valores aleatorios de 0 a 999
                
                long long local_completed;
                
                // Actualizacion segura del contador de avance
                #pragma omp atomic capture
                {
                    elements_completed++;
                    local_completed = elements_completed;
                }
                
                int percent = (local_completed * 100) / N_local;
                int current_threshold = percent / 10;
                
                // Zona critica para imprimir sin mezclar textos
                #pragma omp critical
                {
                    if (current_threshold > thresholds_printed) {
                        // Imprime los progresos pendientes (asegura salto exacto 10, 20... 100)
                        for (int t = thresholds_printed + 1; t <= current_threshold; t++) {
                            cout << "Avance: " << (t * 10) << "% | Proceso/Nodo: " << rank 
                                 << " | Equipo: " << hostname 
                                 << " | Hilo OMP: " << omp_get_thread_num() << "\n";
                        }
                        thresholds_printed = current_threshold;
                    }
                }
            }
        }
    }

    void imprimirArreglo() {
        // Barrera para que la impresion sea ordenada por procesos
        for (int p = 0; p < size; p++) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (p == rank) {
                cout << "\n[Equipo: " << hostname << " | Proceso: " << rank << "] Contenido del arreglo:\n";
                // Cumple con el requisito: Mostrar el contenido solo si no es muy grande (ejecucion 1)
                if (N_local <= 50) {
                    for (long long i = 0; i < N_local; i++) {
                        cout << arreglo[i] << " ";
                    }
                    cout << "\n";
                } else {
                    cout << "(Arreglo local de " << N_local << " elementos. Demasiado grande para mostrar en pantalla)\n";
                }
            }
        }
    }
};

int main(int argc, char** argv) {
    // Inicializacion de MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    long long N = 0;
    
    // El proceso 0 maneja la inicializacion del menu
    if (rank == 0) {
        int opcion = 0;
        cout << "======================================================\n";
        cout << "  Actividad 1.1: Modelo hibrido MPI + OpenMP\n";
        cout << "  Integrantes:\n";
        cout << "  - Anguiano Garcia Angel Yahir Guadalupe\n";
        cout << "  - Figueroa Robles Axel Israel\n";
        cout << "======================================================\n";
        cout << "1. Primera ejecucion (Arreglo 20 a 50 elementos)\n";
        cout << "2. Segunda/Tercera ejecucion (Arreglo 10,000,000 elementos)\n";
        cout << "Elige una opcion: ";
        cin >> opcion;

        if (opcion == 1) {
            srand(time(NULL));
            N = 20 + rand() % 31; // Rango 20 a 50
        } else {
            N = 10000000;
        }
        cout << "\nIniciando ejecucion con " << size << " procesos y N = " << N << " elementos totales.\n\n";
    }

    // Compartir el tamaño total N a todos los procesos
    MPI_Bcast(&N, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    // Clase que maneja el arreglo dinamico y la paralelizacion
    ProcesadorArreglo procesador(N, rank, size);

    // Llenado paralelo con OpenMP mostrando progreso de 10%
    procesador.llenar();
    
    // Imprimir el resultado final
    procesador.imprimirArreglo();

    // Finalizacion de MPI
    MPI_Finalize();
    return 0;
}
