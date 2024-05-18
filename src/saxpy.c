#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>

#define NUM_THREADS 16

// Estructura para pasar argumentos a la función de tarea
typedef struct {
    int thread_id;
    int max_iters;
    int p;
    double* X;
    double* Y;
    double a;
    double* Y_avgs;
} Task_Args;

// Función de tarea para cada hilo
void *saxpy_task(void *arguments) {
    Task_Args *args = (Task_Args *)arguments;
    int thread_id = args->thread_id;
    int max_iters = args->max_iters;
    int p = args->p;
    double* X = args->X;
    double* Y = args->Y;
    double a = args->a;
    double* Y_avgs = args->Y_avgs;

    int it, i;
    for(it = thread_id; it < max_iters; it += NUM_THREADS){
        for(i = 0; i < p; i++){
            Y[i] = Y[i] + a * X[i];
            Y_avgs[it] += Y[i];
        }
        Y_avgs[it] = Y_avgs[it] / p;
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
    // Variables de inicialización
    unsigned int seed = 1;
    int p = 1000000;
    int n_threads = NUM_THREADS;
    int max_iters = 1000;
    double* X;
    double a;
    double* Y;
    double* Y_avgs;
    int i;
    struct timeval t_start, t_end;
    double exec_time;

    // Obtener valores de la línea de comandos
    int opt;
    while((opt = getopt(argc, argv, ":p:s:n:i:")) != -1){  
        switch(opt){  
            case 'p':  
			printf("vector size: %s\n", optarg);
			p = strtol(optarg, NULL, 10);
			assert(p > 0 && p <= 2147483647);
			break;  
			case 's':  
			printf("seed: %s\n", optarg);
			seed = strtol(optarg, NULL, 10);
			break;
			case 'n':  
			printf("threads number: %s\n", optarg);
			n_threads = strtol(optarg, NULL, 10);
			break;  
			case 'i':  
			printf("max. iterations: %s\n", optarg);
			max_iters = strtol(optarg, NULL, 10);
			break;  
			case ':':  
			printf("option -%c needs a value\n", optopt);  
			break;  
			case '?':  
			fprintf(stderr, "Usage: %s [-p <vector size>] [-s <seed>] [-n <threads number>] [-i <maximum itertions>]\n", argv[0]);
			exit(EXIT_FAILURE);
        }  
    }  
    srand(seed);

    printf("p = %d, seed = %d, n_threads = %d, max_iters = %d\n", \
	 p, seed, n_threads, max_iters);	


    // Inicializar datos
    X = (double*) malloc(sizeof(double) * p);
    Y = (double*) malloc(sizeof(double) * p);
    Y_avgs = (double*) malloc(sizeof(double) * max_iters);

    for(i = 0; i < p; i++){
        X[i] = (double)rand() / RAND_MAX;
        Y[i] = (double)rand() / RAND_MAX;
    }
    for(i = 0; i < max_iters; i++){
        Y_avgs[i] = 0.0;
    }
    a = (double)rand() / RAND_MAX;

    // Inicializar hilos y sus argumentos
    pthread_t threads[NUM_THREADS];
    Task_Args args[NUM_THREADS];

    gettimeofday(&t_start, NULL);

    // Crear hilos
    for(int t = 0; t < NUM_THREADS; t++){
        args[t].thread_id = t;
        args[t].max_iters = max_iters;
        args[t].p = p;
        args[t].X = X;
        args[t].Y = Y;
        args[t].a = a;
        args[t].Y_avgs = Y_avgs;
        pthread_create(&threads[t], NULL, saxpy_task, (void *)&args[t]);
    }

    // Esperar a que todos los hilos terminen
    for(int t = 0; t < NUM_THREADS; t++){
        pthread_join(threads[t], NULL);
    }

    gettimeofday(&t_end, NULL);

    // Calcular tiempo de ejecución
    exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;  // sec to ms
    exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms
    printf("Execution time: %f ms \n", exec_time);
    printf("Last 3 values of Y: %f, %f, %f \n", Y[p-3], Y[p-2], Y[p-1]);
    printf("Last 3 values of Y_avgs: %f, %f, %f \n", Y_avgs[max_iters-3], Y_avgs[max_iters-2], Y_avgs[max_iters-1]);

    // Liberar memoria
    free(X);
    free(Y);
    free(Y_avgs);

    return 0;
}
