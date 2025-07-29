#ifndef DBG_H
#define DBG_H

#include "chip8.h"

void debug_init(int mode);
void debug_log(chip8_t *c, uint16_t op);
void debug_sbs(void);
void debug_destroy(void);

#endif