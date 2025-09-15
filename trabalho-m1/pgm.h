#ifndef PGM_H
#define PGM_H

#include <stdbool.h>

struct PGM {
    int w, h, maxv;
    unsigned char* data;
};

bool lerPGM(struct PGM* imagem, const char* nomeArquivo);
bool escreverPGM(const struct PGM* imagem, const char* nomeArquivo);
void imprimirDadosImagem(const struct PGM* imagem, const char* nomeArquivo);

#endif
