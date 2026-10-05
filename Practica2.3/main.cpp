#include <iostream>
#include <cstdlib> // Necesario para usar rand() y srand() (números aleatorios)
#include <ctime>   // Necesario para usar time() y generar números aleatorios diferentes cada vez
#include <omp.h>   // La librería mágica de OpenMP para paralelismo y medir tiempos

using namespace std;

// --- CLASE OPERACIONES VECTORIALES ---
class OperacionesVectoriales {
private:
    long long* matriz; // Puntero para crear nuestro arreglo dinámico
    int totalElementos;  // Guardará el tamaño total (ej. 10x10 = 100, o 1000x1000 = 1000000)
    bool esPequena;      // Para saber si debemos imprimirla en pantalla o no

public:
    // CONSTRUCTOR: Se ejecuta al crear el objeto. Prepara nuestra matriz.
    OperacionesVectoriales(int filas, int columnas, bool esAscendente) {
        totalElementos = filas * columnas;

        // Creamos la matriz dinámicamente en la memoria (sin usar std::vector)
        matriz = new long long[totalElementos];

        // Si tiene menos de 200 elementos, la consideramos pequeña (para poder imprimirla)
        esPequena = (totalElementos <= 100);

        if (esAscendente) {
            // Llenamos con valores ascendentes: 1, 2, 3, 4...
            for (int i = 0; i < totalElementos; i++) {
                matriz[i] = i + 1;
            }
        } else {
            // Llenamos con valores aleatorios entre 1 y 2,000,000
            srand(time(0)); // Inicializa la semilla de números aleatorios
            for (int i = 0; i < totalElementos; i++) {
                // Multiplicamos dos rand() por si el compilador genera números muy pequeños
                matriz[i] = ((rand() * rand()) % 2000000) + 1;
            }
        }
    }

    // DESTRUCTOR: Limpia la memoria cuando el programa termina (Muy importante en memoria dinámica)
    ~OperacionesVectoriales() {
        delete[] matriz;
    }

    // Método extra para imprimir la matriz (solo si es la de 10x10)
    void mostrarMatriz() {
        if (esPequena) {
            cout << "\n--- MATRIZ ACTUAL ---" << endl;
            for (int i = 0; i < totalElementos; i++) {
                cout << matriz[i] << "\t";
                // Cada 10 elementos, damos un salto de línea para que se vea como matriz
                if ((i + 1) % 10 == 0) cout << endl;
            }
            cout << "---------------------" << endl;
        }
    }

    // a. Método de Sumatoria con OpenMP
    long long calcularSumatoria() {
        long long suma = 0;

        // OpenMP: Divide el ciclo 'for' entre los hilos del procesador.
        // reduction(+:suma): Cada hilo suma su parte y al final OpenMP junta todo de forma segura en 'suma'
        #pragma omp parallel for reduction(+:suma)
        for (int i = 0; i < totalElementos; i++) {
            suma += matriz[i];
        }
        return suma;
    }

    // b. Método de Promedio
    double calcularPromedio() {
        // Aprovechamos el método que ya suma todo
        long long sumaTotal = calcularSumatoria();
        // El promedio es la suma dividida entre el total de elementos
        return (double)sumaTotal / totalElementos;
    }

    // c. Método de Máximo con OpenMP
    long long calcularMaximo() {
        long long maximo = matriz[0]; // Asumimos que el primero es el mayor

        // OpenMP: reduction(max:maximo) busca el número más grande usando varios hilos
        #pragma omp parallel for reduction(max:maximo)
        for (int i = 0; i < totalElementos; i++) {
            if (matriz[i] > maximo) {
                maximo = matriz[i];
            }
        }
        return maximo;
    }

    // d. Método de Mínimo con OpenMP
    long long calcularMinimo() {
        long long minimo = matriz[0]; // Asumimos que el primero es el menor

        // OpenMP: reduction(min:minimo) busca el número más pequeño usando varios hilos
        #pragma omp parallel for reduction(min:minimo)
        for (int i = 0; i < totalElementos; i++) {
            if (matriz[i] < minimo) {
                minimo = matriz[i];
            }
        }
        return minimo;
    }
};

// --- PROGRAMA PRINCIPAL (MAIN) ---
int main() {
    int tipoEjecucion;
    cout << "" << endl;
    cout << "Anguiano Garcia Angel Yahir Guadalupe:" << endl;
    cout << "Figueroa Robles Axel Israel:" << endl;
    cout << "" << endl;
    cout << "Bienvenido. Selecciona la ejecucion:" << endl;
    cout << "1. Matriz 10x10 (Valores ascendentes, se imprime en pantalla)" << endl;
    cout << "2. Matriz 1000x1000 (Aleatorios hasta 2M, solo tiempos paralelos)" << endl;
    cout << "Opcion: ";
    cin >> tipoEjecucion;
    cout << "" << endl;
    cout << "Anguiano Garcia Angel Yahir Guadalupe:" << endl;
    cout << "Figueroa Robles Axel Israel:" << endl;

    // Dependiendo de lo que elija el usuario, configuramos la matriz
    OperacionesVectoriales* op;
    if (tipoEjecucion == 1) {
        op = new OperacionesVectoriales(10, 10, true); // 10x10, ascendente
    } else {
        op = new OperacionesVectoriales(1000, 1000, false); // 1000x1000, aleatorio
    }

    char opcionMenu;
    double tiempoInicio, tiempoFin; // Variables para guardar los tiempos

    // Bucle ciclado (Menú Interactivo)
    do {
        cout << "\n========== MENU DE OPERACIONES ==========" << endl;
        cout << "a. Calcular sumatoria" << endl;
        cout << "b. Calcular promedio" << endl;
        cout << "c. Encontrar el maximo" << endl;
        cout << "d. Encontrar el minimo" << endl;
        cout << "e. Salir" << endl;
        cout << "Elige una opcion: ";
        cin >> opcionMenu;

        switch (opcionMenu) {
            case 'a':
                op->mostrarMatriz(); // Mostrará la matriz solo si es la de 10x10
                tiempoInicio = omp_get_wtime(); // Iniciar cronómetro de OpenMP
                cout << "\n> Resultado Sumatoria: " << op->calcularSumatoria() << endl;
                tiempoFin = omp_get_wtime();    // Detener cronómetro
                cout << "> Tiempo en paralelo: " << (tiempoFin - tiempoInicio) << " segundos." << endl;
                break;

            case 'b':
                op->mostrarMatriz();
                tiempoInicio = omp_get_wtime();
                cout << "\n> Resultado Promedio: " << op->calcularPromedio() << endl;
                tiempoFin = omp_get_wtime();
                cout << "> Tiempo en paralelo: " << (tiempoFin - tiempoInicio) << " segundos." << endl;
                break;

            case 'c':
                op->mostrarMatriz();
                tiempoInicio = omp_get_wtime();
                cout << "\n> Resultado Maximo: " << op->calcularMaximo() << endl;
                tiempoFin = omp_get_wtime();
                cout << "> Tiempo en paralelo: " << (tiempoFin - tiempoInicio) << " segundos." << endl;
                break;

            case 'd':
                op->mostrarMatriz();
                tiempoInicio = omp_get_wtime();
                cout << "\n> Resultado Minimo: " << op->calcularMinimo() << endl;
                tiempoFin = omp_get_wtime();
                cout << "> Tiempo en paralelo: " << (tiempoFin - tiempoInicio) << " segundos." << endl;
                break;

            case 'e':
                cout << "Saliendo del programa... ¡Hasta luego!" << endl;
                break;

            default:
                cout << "Opcion no valida. Intenta de nuevo (usa a, b, c, d, e)." << endl;
        }

    } while (opcionMenu != 'e'); // El ciclo se repite mientras no elija 'e'

    // Liberamos el objeto creado
    delete op;

    return 0;
}
