#define _POSIX_C_SOURCE 199309L // Habilita CLOCK_MONOTONIC para medir tempos precisos

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include "pgm.h"
#include "fifo.h"
#include "filtro.h"

// Modos de operação
#define MODE_NEG 0
#define MODE_SLICE 1

// Tamanho máximo da fila de tarefas
#define QMAX 128

// Estrutura de uma tarefa: intervalo de linhas da imagem a processar
typedef struct {
    int y_start, y_end;
} Task;

// Fila circular de tarefas
Task queue_buf[QMAX];
int q_head=0, q_tail=0, q_count=0;

// Mutex para proteger acesso à fila de tarefas
pthread_mutex_t q_lock=PTHREAD_MUTEX_INITIALIZER;

// Semáforos para controle da fila:
// sem_items -> quantas tarefas estão disponíveis para consumir
// sem_space -> quantos espaços livres ainda existem para produção
sem_t sem_items,sem_space;

// Mutex e semáforo para controlar o término das tarefas
pthread_mutex_t done_lock=PTHREAD_MUTEX_INITIALIZER;
sem_t sem_done;
int remaining_tasks=0;

// Estruturas globais de imagem de entrada e saída
struct PGM g_in,g_out;

// Configurações do processamento 
int g_mode = MODE_NEG;  // modo padrão
int g_t1, g_t2;         // limites do slice
int g_nthreads = 4;     // número de threads padrão

// info de tempo das threads
typedef struct {
    int id;
    double tempo;
} ThreadInfo;
ThreadInfo *thread_info;

/**
 * Coloca uma tarefa na fila circular (produtor)
 * Bloqueia se a fila estiver cheia (sem_space)
 */
void queue_push(Task t) {
    sem_wait(&sem_space);               // aguarda espaço livre
    pthread_mutex_lock(&q_lock);        // entra na região crítica da fila
    queue_buf[q_tail] = t;              // adiciona tarefa
    q_tail = (q_tail + 1) % QMAX;       // incrementa índice circular
    q_count++;                          
    pthread_mutex_unlock(&q_lock);      // sai da região crítica
    sem_post(&sem_items);               // sinaliza que há nova tarefa
}

/**
 * Remove uma tarefa da fila circular (consumidor)
 * Bloqueia se a fila estiver vazia (sem_items)
 */
Task queue_pop(){
    sem_wait(&sem_items);               // aguarda tarefa disponível
    pthread_mutex_lock(&q_lock);        // entra na região crítica
    Task t = queue_buf[q_head];         
    q_head = (q_head + 1) % QMAX;      
    q_count--;
    pthread_mutex_unlock(&q_lock);      // sai da região crítica
    sem_post(&sem_space);               // sinaliza espaço livre

    return t;
}

/**
 * Função executada por cada thread do worker
 * Processa blocos da imagem e mede o tempo de execução individual
 */
void* worker_thread(void* arg){
    int tid = *(int*) arg;  // identifica a thread
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start); // tempo inicial da thread

    while(1){
        Task t = queue_pop(); // pega próxima tarefa da fila
        if (t.y_start == -1 && t.y_end == - 1) break; // sinal de término

        // Aplica o filtro correspondente
        if(g_mode == MODE_NEG) {
            apply_negative_block(&g_in, &g_out, t.y_start, t.y_end);
        } else {
            apply_slice_block(&g_in, &g_out, t.y_start, t.y_end, g_t1, g_t2);
        }

        // Atualiza contador de tarefas restantes de forma thread-safe
        pthread_mutex_lock(&done_lock);
        remaining_tasks--;
        if (remaining_tasks == 0) sem_post(&sem_done); // sinaliza término de todas
        pthread_mutex_unlock(&done_lock);
    }

    clock_gettime(CLOCK_MONOTONIC, &end); // tempo final da thread
    thread_info[tid].tempo = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Thread %d terminou em %.6f segundos\n", tid, thread_info[tid].tempo);
    return NULL;
}

/**
 * Funcao principal do worker
 * Recebe FIFO, parâmetros de saida e modo de filtro, cria threads e processa a imagem
 */
int main_worker(int argc, char** argv){
    if (argc < 4) {
        fprintf(stderr,"Uso: %s <fifo_path> <saida.pgm> <negativo|slice> [t1 t2] [nthreads]\n",argv[0]);
        exit(1);
    }

    const char* fifo = argv[1];
    const char* outpth = argv[2];
    const char* mode = argv[3];

    // Configuracao do modo de processamento e numero de threads
    if (strcmp(mode, "negativo") == 0) {
        g_mode = MODE_NEG; 
        g_nthreads = (argc >= 5) ? atoi(argv[4]) : 4;
    } else if (strcmp(mode, "slice") == 0) {
        g_mode = MODE_SLICE;
        g_t1 = atoi(argv[4]);
        g_t2 = atoi(argv[5]);
        g_nthreads = (argc >= 7) ? atoi(argv[6]) : 4;
    } else{
        fprintf(stderr, "Modo inválido \n"); 
        exit(1);
    }

    // Inicializa semaforos
    sem_init(&sem_items, 0, 0);        // inicialmente nao ha tarefas
    sem_init(&sem_space, 0, QMAX);     // fila com QMAX espaços
    sem_init(&sem_done, 0, 0);         // semaforo de termino

    // Garante que o FIFO existe e abre para leitura
    if(!criarFIFO(fifo)) exit(1);
    int fd = abrirFIFOLeitura(fifo);
    if(fd == -1) exit(1);

    // Le metadados da imagem (largura, altura, max valor)
    read(fd, &g_in.w, sizeof(int));
    read(fd, &g_in.h, sizeof(int));
    read(fd, &g_in.maxv, sizeof(int));

    int tamanho = g_in.w * g_in.h;
    g_in.data = malloc(tamanho); 
    g_out.data = malloc(tamanho); 
    g_out.w = g_in.w; g_out.h = g_in.h; g_out.maxv = g_in.maxv;

    // Le os pixels da imagem
    read(fd,g_in.data,tamanho);
    close(fd);

    thread_info = malloc(sizeof(ThreadInfo) * g_nthreads);
    pthread_t tids[g_nthreads];
    int tids_id[g_nthreads];

    struct timespec start_total, end_total;
    clock_gettime(CLOCK_MONOTONIC, &start_total); // tempo inicial total

    // Cria threads worker
    for(int i = 0; i < g_nthreads; i++){
        tids_id[i] = i;
        pthread_create(&tids[i], NULL, worker_thread, &tids_id[i]);
    }

    // Divide a imagem em blocos de 32 linhas e adiciona tarefas na fila
    int linhas_por_tarefa=32;

    for (int y = 0; y < g_in.h; y += linhas_por_tarefa) {
        Task t = { y, (y + linhas_por_tarefa < g_in.h) ? y + linhas_por_tarefa : g_in.h };
        queue_push(t);

        // Incrementa contador de tarefas restantes de forma segura
        pthread_mutex_lock(&done_lock); 
        remaining_tasks++; 
        pthread_mutex_unlock(&done_lock);
    }

    // Espera até que todas as tarefas sejam concluídas
    sem_wait(&sem_done);

    // Envia sinal de término (-1, -1) para todas as threads
    for(int i = 0; i < g_nthreads; i++) {
        queue_push((Task){-1, -1});
    }

    // Espera todas as threads terminarem
    for(int i = 0; i < g_nthreads; i++) {
        pthread_join(tids[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_total); // tempo final total
    double tempo_total = (end_total.tv_sec - start_total.tv_sec) + 
                         (end_total.tv_nsec - start_total.tv_nsec)/1e9;
    printf("Tempo total de processamento: %.6f segundos\n", tempo_total);

    // Grava imagem de saída
    escreverPGM(&g_out, outpth);

    // Libera memória e destroi semáforos
    free(g_in.data); 
    free(g_out.data); 
    free(thread_info);
    sem_destroy(&sem_items); 
    sem_destroy(&sem_space); 
    sem_destroy(&sem_done);

    return 0;
}
