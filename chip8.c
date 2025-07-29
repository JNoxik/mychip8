#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "dbg.h"
#include "chip8.h"

/* Init/Destroy */
chip8_t* chip8_init(void) 
{
    srand(time(NULL));

    chip8_t *chip8 = (chip8_t*)malloc(sizeof(chip8_t));
    memset(chip8, 0, sizeof *chip8);
    memcpy(chip8->memory.memory, font, sizeof font);

    chip8->PC=0x200;
    chip8->I=0;
    chip8->SP=0;
    chip8->DT=0;
    chip8->ST=0;

    return chip8;
}

void chip8_destroy(chip8_t *c) {
    free(c);
}


static uint16_t chip8_fetch(chip8_t *c) {
    uint16_t hi = c->memory.memory[c->PC];
    uint16_t lo = c->memory.memory[c->PC + 1];
    return (hi << 8) | lo;
}

/* Cycle */
void chip8_cycle(chip8_t *c) {
    uint16_t op = chip8_fetch(c);
    c->PC += 2;

    debug_log(c, op);

    uint8_t  x = (op >> 8) & 0x0F;
    uint8_t  y = (op >> 4) & 0x0F;
    uint8_t  nibble = op   & 0x000F;
    uint8_t  byte   = op   & 0x00FF;
    uint16_t addr   = op   & 0x0FFF;

    switch (op & 0xF000) {
        case 0x0000:
            if (byte == 0xE0) {
                // 00E0: CLS
                memset(c->FB, 0, sizeof(c->FB));
            } else if (byte == 0xEE) {
                // 00EE: RET
                if (c->SP > 0) {
                    c->PC = c->memory.stack[--c->SP];
                }
            }
            break;

        case 0x1000:
            // 1nnn: JP addr
            c->PC = addr;
            break;

        case 0x2000:
            // 2nnn: CALL addr
            if (c->SP < 16) {
                c->memory.stack[c->SP++] = c->PC;
                c->PC = addr;
            }
            break;

        case 0x3000:
            // 3xkk: SE Vx, byte
            if (c->regs[x] == byte) c->PC += 2;
            break;

        case 0x4000:
            // 4xkk: SNE Vx, byte
            if (c->regs[x] != byte) c->PC += 2;
            break;

        case 0x5000:
            // 5xy0: SE Vx, Vy
            if (c->regs[x] == c->regs[y]) c->PC += 2;
            break;

        case 0x6000:
            // 6xkk: LD Vx, byte
            c->regs[x] = byte;
            break;

        case 0x7000:
            // 7xkk: ADD Vx, byte
            c->regs[x] += byte;
            break;

        case 0x8000:
            switch (nibble) {
                case 0x0: c->regs[x] = c->regs[y]; break;  // 8xy0: LD  Vx, Vy
                case 0x1: c->regs[x] |= c->regs[y]; break; // 8xy1: OR  Vx, Vy
                case 0x2: c->regs[x] &= c->regs[y]; break; // 8xy2: AND Vx, Vy
                case 0x3: c->regs[x] ^= c->regs[y]; break; // 8xy3: XOR Vx, Vy
                case 0x4: {
                    // 8xy4: ADD Vx, Vy
                    uint16_t sum = c->regs[x] + c->regs[y];
                    c->regs[0xF] = (sum > 0xFF);
                    c->regs[x] = sum & 0xFF;
                    break;
                }
                case 0x5:
                    // 8xy5: SUB Vx, Vy
                    c->regs[0xF] = (c->regs[x] >= c->regs[y]);
                    c->regs[x] -= c->regs[y];
                    break;
                case 0x6:
                    // 8xy6: SHR Vx, Vy
                    c->regs[0xF] = c->regs[x] & 1;
                    c->regs[x] >>= 1;
                    break;
                case 0x7:
                    // 8xy7: SUBN Vx, Vy
                    c->regs[0xF] = (c->regs[y] >= c->regs[x]);
                    c->regs[x] = c->regs[y] - c->regs[x];
                    break;
                case 0xE:
                    // 8xy0: SHL Vx, Vy
                    c->regs[0xF] = (c->regs[x] >> 7) & 1;
                    c->regs[x] <<= 1;
                    break;
            }
            break;

        case 0x9000:
            // 9xy0: SNE Vx, Vy
            if (c->regs[x] != c->regs[y]) c->PC += 2;
            break;

        case 0xA000:
            // Annn: LD I, addr
            c->I = addr;
            break;

        case 0xB000:
            // Bnnn: JP V0, addr
            c->PC = addr + c->regs[0];
            break;

        case 0xC000:
            // Cxkk: RND Vx, byte
            c->regs[x] = (rand() & 0xFF) & byte;
            break;

        case 0xD000: {
            // Dxyn: DRW Vx, Vy, nibble
            uint8_t height = nibble;
            uint8_t vx = c->regs[x];
            uint8_t vy = c->regs[y];
            c->regs[0xF] = 0;

            for (int row = 0; row < height; row++) {
                uint8_t sprite = c->memory.memory[c->I + row];
                for (int col = 0; col < 8; col++) {
                    if (sprite & (0x80 >> col)) {
                        int px = (vx + col) % 64;
                        int py = (vy + row) % 32;
                        int index = py * 64 + px;
                        if (c->FB[index]) {
                            c->regs[0xF] = 1;
                        }
                        c->FB[index] ^= 1;
                    }
                }
            }
            break;
        }

        case 0xE000:
            switch (byte) {
                case 0x9E:
                    if (c->keypad[c->regs[x]]) c->PC += 2;
                    break;
                case 0xA1:
                    if (!c->keypad[c->regs[x]]) c->PC += 2;
                    break;
            }
            break;

        case 0xF000:
            switch (byte) {
                case 0x07: c->regs[x] = c->DT; break;    // Fx07: LD  Vx, DT
                case 0x0A: {
                    // Fx0A: LD Vx, K — wait for key press
                    bool pressed = false;
                    for (int i = 0; i < 16; ++i) {
                        if (c->keypad[i]) {
                            c->regs[x] = i;
                            pressed = true;
                            break;
                        }
                    }
                    if (!pressed) {
                        c->PC -= 2; // Repeat instruction
                    }
                    break;
                }
                case 0x15: c->DT = c->regs[x]; break;    // Fx15: LD  DT, Vx
                case 0x18: c->ST = c->regs[x]; break;    // Fx18: LD  ST, Vx
                case 0x1E: c->I += c->regs[x]; break;    // Fx1E: ADD I, Vx
                case 0x29: c->I = c->regs[x] * 5; break; // Fx29: LD  F, Vx
                case 0x33: {
                    uint8_t val = c->regs[x];
                    c->memory.memory[c->I]     = val / 100;
                    c->memory.memory[c->I + 1] = (val / 10) % 10;
                    c->memory.memory[c->I + 2] = val % 10;
                    break;
                }
                case 0x55:
                    // Fx55: LD [I], Vx
                    for (int i = 0; i <= x; ++i)
                        c->memory.memory[c->I + i] = c->regs[i];
                    break;
                case 0x65:
                    // Fx65: LD Vx, [I]
                    for (int i = 0; i <= x; ++i)
                        c->regs[i] = c->memory.memory[c->I + i];
                    break;
            }
            break;
    }
}

void chip8_update(chip8_t *c) {
    if(c->DT > 0) --c->DT;
    if(c->ST > 0) --c->ST;
}
