#include <stdio.h>
#include "dbg.h"


static FILE* log;
static int   mode;

static const char *opcode(char *buf, size_t len, uint16_t op)
{
    uint8_t  x   = (op >> 8) & 0x0F;
    uint8_t  y   = (op >> 4) & 0x0F;
    uint8_t  kk  = op & 0xFF;
    uint16_t nnn = op & 0x0FFF;

    switch (op & 0xF000) {
        case 0x0000:
            if      (op == 0x00E0) return "CLS";
            else if (op == 0x00EE) return "RET";
            else                   { snprintf(buf,len,"SYS %03X",nnn); return buf; }

        case 0x1000: snprintf(buf,len,"JP %03X",nnn);           return buf;
        case 0x2000: snprintf(buf,len,"CALL %03X",nnn);         return buf;
        case 0x3000: snprintf(buf,len,"SE V%X,%02X",x,kk);      return buf;
        case 0x4000: snprintf(buf,len,"SNE V%X,%02X",x,kk);     return buf;
        case 0x5000: snprintf(buf,len,"SE V%X,V%X",x,y);        return buf;
        case 0x6000: snprintf(buf,len,"LD V%X,%02X",x,kk);      return buf;
        case 0x7000: snprintf(buf,len,"ADD V%X,%02X",x,kk);     return buf;

        case 0x8000:
            switch (op & 0xF) {
                case 0x0: snprintf(buf,len,"LD V%X,V%X",x,y);   return buf;
                case 0x1: snprintf(buf,len,"OR V%X,V%X",x,y);   return buf;
                case 0x2: snprintf(buf,len,"AND V%X,V%X",x,y);  return buf;
                case 0x3: snprintf(buf,len,"XOR V%X,V%X",x,y);  return buf;
                case 0x4: snprintf(buf,len,"ADD V%X,V%X",x,y);  return buf;
                case 0x5: snprintf(buf,len,"SUB V%X,V%X",x,y);  return buf;
                case 0x6: snprintf(buf,len,"SHR V%X,V%X",x,y);  return buf;
                case 0x7: snprintf(buf,len,"SUBN V%X,V%X",x,y); return buf;
                case 0xE: snprintf(buf,len,"SHL V%X,V%X",x,y);  return buf;
            }
            break;

        case 0x9000: snprintf(buf,len,"SNE V%X,V%X",x,y);       return buf;
        case 0xA000: snprintf(buf,len,"LD I,%03X",nnn);         return buf;
        case 0xB000: snprintf(buf,len,"JP V0,%03X",nnn);        return buf;
        case 0xC000: snprintf(buf,len,"RND V%X,%02X",x,kk);     return buf;
        case 0xD000: snprintf(buf,len,"DRW V%X,V%X,%X",x,y,op&0xF); return buf;

        case 0xE000:
            if      (kk == 0x9E) snprintf(buf,len,"SKP V%X",x);
            else if (kk == 0xA1) snprintf(buf,len,"SKNP V%X",x);
            else                 snprintf(buf,len,"???");
            return buf;

        case 0xF000:
            switch (kk) {
                case 0x07: snprintf(buf,len,"LD V%X,DT",x);     return buf;
                case 0x0A: snprintf(buf,len,"LD V%X,K",x);      return buf;
                case 0x15: snprintf(buf,len,"LD DT,V%X",x);     return buf;
                case 0x18: snprintf(buf,len,"LD ST,V%X",x);     return buf;
                case 0x1E: snprintf(buf,len,"ADD I,V%X",x);     return buf;
                case 0x29: snprintf(buf,len,"LD F,V%X",x);      return buf;
                case 0x33: snprintf(buf,len,"LD B,V%X",x);      return buf;
                case 0x55: snprintf(buf,len,"LD [I],V%X",x);    return buf;
                case 0x65: snprintf(buf,len,"LD V%X,[I]",x);    return buf;
            }
            break;
    }
    snprintf(buf,len,"???");
    return buf;
}

void debug_init(int m) {
    if (m == 1) {
        log = fopen("dbg.log", "w");
        mode=1;
    }
    else mode=2;
}

void debug_destroy(void) {
    if (log) fclose(log);
}

void debug_sbs(void) {
    fprintf(stdout, "\n--- press ENTER to step ---\n");
    fflush(stdout);
    while (getchar() != '\n') { }
}

void debug_log(chip8_t *c, uint16_t op) {
    char mnem[64];
    opcode(mnem,sizeof(mnem),op);
    if ( mode == 1 ) {
        fprintf(log,
            "%04X: %-15s I=%03X SP=%02X DT=%02X ST=%02X "
            "V0=%02X V1=%02X V2=%02X V3=%02X V4=%02X V5=%02X "
            "V6=%02X V7=%02X V8=%02X V9=%02X VA=%02X VB=%02X "
            "VC=%02X VD=%02X VE=%02X VF=%02X\n",
            c->PC-2, mnem, c->I, c->SP, c->DT, c->ST,
            c->regs[0],  c->regs[1],  c->regs[2],  c->regs[3],
            c->regs[4],  c->regs[5],  c->regs[6],  c->regs[7],
            c->regs[8],  c->regs[9],  c->regs[10], c->regs[11],
            c->regs[12], c->regs[13], c->regs[14], c->regs[15]);
        fflush(log);
    }
    else if ( mode == 2) {
        printf("%04X: %-15s  I:%03X  SP:%02X  DT:%02X  ST:%02X\n",
               c->PC-2, mnem, c->I, c->SP, c->DT, c->ST);
        for (int i = 0; i < 16; ++i) {
            printf("│ V%-2X:%02X%s", i, c->regs[i], (i==7||i==15)?" │\n":"");
        }
        debug_sbs();
    }
}