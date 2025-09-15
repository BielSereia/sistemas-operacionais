#ifndef FILTRO_H
#define FILTRO_H

#include "pgm.h"

// Aplica filtro negativo em um bloco de linhas [rs, re)
void apply_negative_block(struct PGM* in, struct PGM* out, int rs, int re);

// Aplica filtro de limiarização com fatiamento em um bloco de linhas [rs, re)
void apply_slice_block(struct PGM* in, struct PGM* out, int rs, int re, int t1, int t2);

#endif
