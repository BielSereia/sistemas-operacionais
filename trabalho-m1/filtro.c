#include "filtro.h"

/*
    Percorre todas as linhas de rs até re-1 e todas as colunas, transformando cada pixel v em 255 - v

    in = ponteiro para imagem de entrada
    out = ponteiro para imagem de saida
    rs = indice da linha inicial do bloco a processar (row start)
    re = indice da linha final do bloco a processar (row end)

    novo_pixel[x, y] = 255 - valor_pixel_original[x, y]
*/
void apply_negative_block(struct PGM* in, struct PGM* out, int rs, int re) {
    for (int y = rs; y < re; y++) {
        for (int x = 0; x < in->w; x++) {
            int idx = y * in->w + x;
            out->data[idx] = 255 - in->data[idx];
        }
    }
}

/*
    Percorre todas as linhas de rs até re-1 e todas as colunas
    Se valor do pixel v estiver entre t1 e t2 (inclusive), mantém v
    caso contrário, zera o pixel (0)

    in  - ponteiro para a imagem de entrada
    out - ponteiro para a imagem de saída
    rs  - índice da linha inicial do bloco a processar (row start)
    re  - índice da linha final do bloco a processar (row end, não inclusiva)
    t1  - limite inferior do intervalo de intensidade
    t2  - limite superior do intervalo de intensidade
*/
void apply_slice_block(struct PGM* in, struct PGM* out, int rs, int re, int t1, int t2) {
    for (int y = rs; y < re; y++) {
        for (int x = 0; x < in->w; x++) {
            int idx = y * in->w + x;
            unsigned char v = in->data[idx];
            if (v >= t1 && v <= t2)
                out->data[idx] = v;
            else
                out->data[idx] = 0;
        }
    }
}
