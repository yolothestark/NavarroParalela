#include <iostream>
#include <omp.h>
#include <ctime>

using namespace std;

// Llena el arreglo dinamico en paralelo
void llenarArreglo(int* arr, int n, int max_val) {
    double inicio = omp_get_wtime();
    #pragma omp parallel
    {
        unsigned int semilla = time(NULL) ^ omp_get_thread_num();
        #pragma omp for
        for (int i = 0; i < n; i++) {
            semilla = (semilla * 1103515245 + 12345) % 2147483648;
            arr[i] = semilla % (max_val + 1);
        }
    }
    cout << "-> Tiempo de llenado: " << omp_get_wtime() - inicio << " segundos\n";
}

// Muestra el contenido del arreglo utilizando una region paralela
void imprimirArreglo(int* arr, int n, const string& titulo) {
    if (n > 100) {
        cout << "[" << titulo << "]: (Arreglo demasiado grande para mostrar)\n";
        return;
    }
    #pragma omp parallel
    {
        #pragma omp single
        {
            cout << "[" << titulo << "]: ";
            for (int i = 0; i < n; i++) cout << arr[i] << " ";
            cout << "\n";
        }
    }
}

// Counting Sort Secuencial
void countingSortSeq(int* arr, int n, int max_val) {
    int* conteo = new int[max_val + 2]();
    for (int i = 0; i < n; i++) conteo[arr[i]]++;

    int index = 0;
    for (int i = 0; i <= max_val; i++) {
        while (conteo[i] > 0) {
            arr[index++] = i;
            conteo[i]--;
        }
    }
    delete[] conteo;
}

// Counting Sort Paralelo
void countingSortPar(int* arr, int n, int max_val) {
    int* conteo = new int[max_val + 2]();

    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        #pragma omp atomic
        conteo[arr[i]]++;
    }

    int pos_actual = 0;
    for (int i = 0; i <= max_val; i++) {
        int c = conteo[i];
        conteo[i] = pos_actual;
        pos_actual += c;
    }
    conteo[max_val + 1] = n;

    #pragma omp parallel for
    for (int i = 0; i <= max_val; i++) {
        for (int j = conteo[i]; j < conteo[i + 1]; j++) {
            arr[j] = i;
        }
    }
    delete[] conteo;
}

// Fusion para Merge Sort
void merge(int* arr, int* temp, int left, int mid, int right) {
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    for (i = left; i <= right; i++) arr[i] = temp[i];
}

// Merge Sort Secuencial
void mergeSortSeq(int* arr, int* temp, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortSeq(arr, temp, left, mid);
        mergeSortSeq(arr, temp, mid + 1, right);
        merge(arr, temp, left, mid, right);
    }
}

// Merge Sort Paralelo usando tareas
void mergeSortPar(int* arr, int* temp, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        if ((right - left) < 10000) {
            mergeSortSeq(arr, temp, left, right);
        } else {
            #pragma omp task shared(arr, temp)
            mergeSortPar(arr, temp, left, mid);

            #pragma omp task shared(arr, temp)
            mergeSortPar(arr, temp, mid + 1, right);

            #pragma omp taskwait

            merge(arr, temp, left, mid, right);
        }
    }
}

// Menu principal de ejecucion
int main() {
    int n = 0, max_val = 0;
    cout << "Ingresa la cantidad de elementos (ej. 100 o 10000000): ";
    cin >> n;
    cout << "Ingresa el valor maximo aleatorio (ej. 200 o 2000000): ";
    cin >> max_val;

    int* original = new int[n];
    int* arr = new int[n];
    int* temp = new int[n];
    
    double t_iter_seq = 0.0, t_rec_seq = 0.0;
    int opcion = -1;

    while(opcion != 0) {
        cout << "\n======================================================\n";
        cout << "1. Llenar el arreglo\n";
        cout << "2. Ejecutar ordenamiento iterativo secuencial\n";
        cout << "3. Ejecutar ordenamiento iterativo paralelo\n";
        cout << "4. Ejecutar ordenamiento recursivo secuencial\n";
        cout << "5. Ejecutar ordenamiento recursivo paralelo\n";
        cout << "6. Mostrar resultados (Arreglos)\n";
        cout << "0. Salir\nElige: ";
        cin >> opcion;

        switch(opcion) {
            case 1:
                llenarArreglo(original, n, max_val);
                for(int i = 0; i < n; i++) arr[i] = original[i]; // Copiar a arr de trabajo
                break;
            case 2:
                for(int i = 0; i < n; i++) arr[i] = original[i]; // Restaurar arreglo
                t_iter_seq = omp_get_wtime();
                countingSortSeq(arr, n, max_val);
                t_iter_seq = omp_get_wtime() - t_iter_seq;
                cout << "Tiempo Iterativo Secuencial: " << t_iter_seq << " seg\n";
                break;
            case 3:
                for(int i = 0; i < n; i++) arr[i] = original[i];
                {
                    double t = omp_get_wtime();
                    countingSortPar(arr, n, max_val);
                    t = omp_get_wtime() - t;
                    cout << "Tiempo Iterativo Paralelo: " << t << " seg\n";
                    if (t_iter_seq > 0) cout << "Speedup Iterativo: " << (t_iter_seq / t) << "\n";
                }
                break;
            case 4:
                for(int i = 0; i < n; i++) arr[i] = original[i];
                t_rec_seq = omp_get_wtime();
                mergeSortSeq(arr, temp, 0, n - 1);
                t_rec_seq = omp_get_wtime() - t_rec_seq;
                cout << "Tiempo Recursivo Secuencial: " << t_rec_seq << " seg\n";
                break;
            case 5:
                for(int i = 0; i < n; i++) arr[i] = original[i];
                {
                    double t = omp_get_wtime();
                    #pragma omp parallel
                    {
                        #pragma omp single
                        mergeSortPar(arr, temp, 0, n - 1);
                    }
                    t = omp_get_wtime() - t;
                    cout << "Tiempo Recursivo Paralelo: " << t << " seg\n";
                    if (t_rec_seq > 0) cout << "Speedup Recursivo: " << (t_rec_seq / t) << "\n";
                }
                break;
            case 6:
                imprimirArreglo(original, n, "Arreglo Desordenado");
                imprimirArreglo(arr, n, "Arreglo Ordenado");
                break;
            case 0:
                cout << "Saliendo...\n";
                break;
            default:
                cout << "Opcion no valida.\n";
        }
    }

    delete[] original;
    delete[] arr;
    delete[] temp;
    return 0;
}
