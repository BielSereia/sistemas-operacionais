#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

bool criarFIFO(const char* caminho) {
    /* mkfifo vem da lib POSIX, cria um FIFO no sistema de arquivos. 0666 quer dizer que terá leitura e escrita */
    if (mkfifo(caminho, 0666) == -1) {
        if (errno == EEXIST) {
            // Já existe, não é erro
            return true;
        } else {
            perror("mkfifo");
            return false;
        }
    }
    return true;
}

int abrirFIFOEscrita(const char* caminho) {
    int fd = open(caminho, O_WRONLY);
    if (fd == -1) {
        perror("open O_WRONLY");
    }
    return fd;
}

int abrirFIFOLeitura(const char* caminho) {
    int fd = open(caminho, O_RDONLY);
    if (fd == -1) {
        perror("open O_RDONLY");
    }
    return fd;
}
