#ifndef FIFO_H
#define FIFO_H

#include <stdbool.h>

// Cria o FIFO se não existir
bool criarFIFO(const char* caminho);

// Abre FIFO para escrita
int abrirFIFOEscrita(const char* caminho);

// Abre FIFO para leitura
int abrirFIFOLeitura(const char* caminho);

#endif
