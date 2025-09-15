#include <stdio.h>
#include <stdlib.h>
#include "pgm.h"
#include "fifo.h"
#include <unistd.h>

int main_sender(int argc, char* argv[]) {
    if(argc<3){
        fprintf(stderr,"Uso: %s sender <fifo_path> <entrada.pgm>\n",argv[0]);
        exit(1);
    }

    const char* fifo = argv[1];
    const char* inpath = argv[2];

    if(!criarFIFO(fifo)) exit(1);

    struct PGM img;
    
    if(!lerPGM(&img,inpath)) {
        fprintf(stderr,"Erro ao ler %s\n",inpath);
        exit(1);
    }

    int fd = abrirFIFOEscrita(fifo);

    if(fd==-1){
        free(img.data);
        exit(1);
    }

    int w=img.w, h=img.h, maxv=img.maxv, tamanho=w*h;

    write(fd,&w,sizeof(int));
    write(fd,&h,sizeof(int));
    write(fd,&maxv,sizeof(int));
    write(fd,img.data,tamanho);

    printf("Imagem %s enviada via FIFO: %s\n", inpath, fifo);
    close(fd);
    free(img.data);
    return 0;
}
