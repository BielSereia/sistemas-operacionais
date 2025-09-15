#include "pgm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Ignora comentários e espaços em branco no arquivo
static void ignorarComentarios(FILE* fp) {
    int ch;
    char linha[256];

    // fgetc - le um caracter do arquivo. EOF = valor especial que indica se é o fim do arquivo. isspace = true se for espaço, tab, newline, etc.
    while ((ch = fgetc(fp)) != EOF && isspace(ch));

    if (ch == '#') {
        // le uma linha completa do arquivo ate ln
        fgets(linha, sizeof(linha), fp);
        ignorarComentarios(fp); // Recursivo
    } else {
        // Devolve o caractere para o arquivo, para ser lido de novo
        ungetc(ch, fp); // Volta um caractere
    }
}

bool lerPGM(struct PGM* imagem, const char* nomeArquivo) {
    FILE* arquivo = fopen(nomeArquivo, "rb");
    if (arquivo == NULL) {
        printf("Erro: não foi possível abrir o arquivo \"%s\".\n", nomeArquivo);
        return false;
    }

    char tipo[3];
    ignorarComentarios(arquivo);
    fscanf(arquivo, "%2s", tipo);
    tipo[2] = '\0';

    if (strcmp(tipo, "P5") != 0) {
        fprintf(stderr, "Erro: tipo de arquivo incorreto. Esperado 'P5'.\n");
        fclose(arquivo);
        return false;
    }

    ignorarComentarios(arquivo);
    fscanf(arquivo, "%d %d", &imagem->w, &imagem->h);

    ignorarComentarios(arquivo);
    fscanf(arquivo, "%d", &imagem->maxv);

    fgetc(arquivo); // Consome o '\n' após o valor máximo

    int tamanho = imagem->w * imagem->h;
    imagem->data = (unsigned char*) malloc(tamanho);

    if (imagem->data == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memória para os dados da imagem.\n");
        fclose(arquivo);
        return false;
    }

    size_t lidos = fread(imagem->data, sizeof(unsigned char), tamanho, arquivo);
    if (lidos != tamanho) {
        fprintf(stderr, "Erro: leitura incompleta dos dados da imagem.\n");
        free(imagem->data);
        fclose(arquivo);
        return false;
    }

    fclose(arquivo);
    return true;
}

bool escreverPGM(const struct PGM* imagem, const char* nomeArquivo) {
    FILE* f = fopen(nomeArquivo, "wb");
    if (!f) return false;

    fprintf(f, "P5\n%d %d\n%d\n", imagem->w, imagem->h, imagem->maxv);
    size_t escritos = fwrite(imagem->data, sizeof(unsigned char), imagem->w*imagem->h, f);
    fclose(f);

    return escritos == (size_t)(imagem->w*imagem->h);
}

void imprimirDadosImagem(const struct PGM* imagem, const char* nomeArquivo) {
    printf("Arquivo de imagem: %s\n", nomeArquivo);
    printf("Formato: P5 (Binário)\n");
    printf("Largura: %d px\n", imagem->w);
    printf("Altura : %d px\n", imagem->h);
    printf("Valor máximo de cinza: %d\n", imagem->maxv);
}
