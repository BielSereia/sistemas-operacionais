#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sender.h"
#include "worker.h"

/* 
    compilar: gcc main.c pgm.c fifo.c sender.c worker.c filtro.c -o prog -lpthread

    exeuctar worker negativo: ./prog worker meu_fifo saida_negativo.pgm negativo 4
        0: worker ou sender
        1: nome do fifo
        2: arquivo final
        3: tipo de filtro (negativo)
        4: 4 é o número de trheads que o worker vai processar a imagem em paralelo

    exeuctar worker slide: ./prog worker meu_fifo saida_slice.pgm slice 50 200 4
        0: worker ou sender
        1: nome do fifo
        2: arquivo final
        3: tipo de filtro (slice), mantem pixels dentro de um intervalo [t1, t2] e zera os demais
        4: t1 (valor mínimo do intervalo)
        5: t2 (valor máximo do intervalo)
        6: 4 é o número de trheads que o worker vai processar a imagem em paralelo
    
    executar sender: ./prog sender meu_fifo entrada.pgm
        0: worker ou sender
        1: nome do fifo
        2: arquivo de entrada
*/
int main(int argc,char* argv[]){
    if(argc<2){
        printf("Uso:\n  %s sender <fifo> <entrada.pgm>\n  %s worker <fifo> <saida.pgm> <negativo|slice> [t1 t2] [nthreads]\n",argv[0],argv[0]);
        return 1;
    }

    if(strcmp(argv[1],"sender")==0){
        if(argc<4){ fprintf(stderr,"Uso: %s sender <fifo> <entrada.pgm>\n",argv[0]); return 1; }
        return main_sender(argc-1,&argv[1]);
    }
    else if(strcmp(argv[1],"worker")==0){
        if(argc<5){ fprintf(stderr,"Uso: %s worker <fifo> <saida.pgm> <negativo|slice> [t1 t2] [nthreads]\n",argv[0]); return 1; }
        return main_worker(argc-1,&argv[1]);
    }
    else{
        fprintf(stderr,"Opção inválida: %s (use sender ou worker)\n",argv[1]);
        return 1;
    }
}
