#include <stdio.h>
#include <stdint.h>

#include "memmap.h"

#define VMREGCNT 16
typedef int32_t vmreg;
#define VMCODEMAX 65536
#define VMMEMMAX 65536
typedef struct vm vm;
struct vm {
  vmreg r[VMREGCNT];
  uint8_t mem[VMMEMMAX];
  const uint8_t *code;
  size_t ip;
  int running;
  int (*hostcall[32])(struct vm *);
};
typedef enum {
  OPHALT = 0,
  OPMOV,
  OPLOADI,
  OPADD,
  OPSUB,
  OPMUL,
  OPDIV,
  OPJMP,
  OPJZ,
  OPCALLHOST,
  OPLOAD8,
  OPSTORE8,
  OPLOAD32,
  OPSTORE32
} opcode;

int hostprint(vm *v);

void
writei32(vm *v, uint16_t addr, int32_t val) {
  v->mem[addr] = (uint8_t)(val & 0xFF);
  v->mem[addr+1] = (uint8_t)((val >> 8) & 0xFF);
  v->mem[addr+2] = (uint8_t)((val >> 16) & 0xFF);
  v->mem[addr+3] = (uint8_t)((val >> 24) & 0xFF);
}

int32_t
readi32(const vm *v, uint16_t addr) {
  return (int32_t)(
      (uint32_t)v->mem[addr] |
      (uint32_t)v->mem[addr+1] << 8 |
      (uint32_t)v->mem[addr+2] << 16 |
      (uint32_t)v->mem[addr+3] << 24 
      );
}

void
vmrun(vm *v) {
  uint8_t op, A, B, C;
  int16_t imm, offset;
  uint32_t addr;
  v->running = 1;
  while (v->running) {
    op = v->code[v->ip++];
    A = v->code[v->ip++];
    B = v->code[v->ip++];
    C = v->code[v->ip++];
    switch (op) {
      case OPHALT:
        v->running = 0;
        break;
      case OPMOV:
        v->r[A] = v->r[B];
        break;
      case OPLOADI:
        imm = (int16_t)((B << 8) | C);
        v->r[A] = imm;
        break;
      case OPADD:
        v->r[A] = v->r[B] + v->r[C];
        break;
      case OPSUB:
        v->r[A] = v->r[B] - v->r[C];
        break;
      case OPMUL:
        v->r[A] = v->r[B] * v->r[C];
        break;
      case OPDIV:
        if (v->r[C] == 0) {
          fprintf(stderr, "VM: Encountered division by zero. Halting.\n");
          v->running = 0;
          break;
        }
        v->r[A] = v->r[B] / v->r[C];
        break;
      case OPJMP:
        offset = (int16_t)((B << 8) | C);
        v->ip = (size_t)((int)v->ip + offset);
        break;
      case OPJZ:
        offset = (int16_t)((B << 8) | C);
        if (!(v->r[A])) { v->ip = (size_t)(v->ip + offset); }
        break;
      case OPCALLHOST:
        v->hostcall[A](v);
        break;
      case OPLOAD8:
        addr = (uint32_t)v->r[B];
        if (addr >= VMMEMMAX) {
          fprintf(stderr, "VM: Load8 OOB @ %u).\n", addr);
          v->running = 0;
          break;
        }
        v->r[A] = (vmreg)v->mem[addr];
        break;
      case OPSTORE8:
        addr = (uint32_t)v->r[B];
        if (addr >= VMMEMMAX) {
          fprintf(stderr, "VM: Store8 OOB @ %u).\n", addr);
          v->running = 0;
          break;
        }
        v->mem[addr] = (uint8_t)(v->r[A] & 0xFF);
        break;
      case OPLOAD32:
        addr = (uint32_t)v->r[B];
        if (addr >= VMMEMMAX) {
          fprintf(stderr, "VM: Load32 OOB @ %u).\n", addr);
          v->running = 0;
          break;
        }
        v->r[A] = (vmreg)(
            (uint32_t)v->mem[addr] |
            (uint32_t)v->mem[addr+1] << 8 |
            (uint32_t)v->mem[addr+2] << 16 |
            (uint32_t)v->mem[addr+3] << 24 
            );
        break;
      case OPSTORE32:
        addr = (uint32_t)v->r[B];
        if (addr >= VMMEMMAX) {
          fprintf(stderr, "VM: Store32 OOB @ %u).\n", addr);
          v->running = 0;
          break;
        }
        v->mem[addr] = (uint8_t)(v->r[A] & 0xFF);
        v->mem[addr+1] = (uint8_t)((v->r[A] >> 8) & 0xFF);
        v->mem[addr+2] = (uint8_t)((v->r[A] >> 16) & 0xFF);
        v->mem[addr+3] = (uint8_t)((v->r[A] >> 24) & 0xFF);
        break;
      default:
        v->running = 0;
        break;
    }
  }
}

void
vmtest() {
  int32_t ncmds, i, base, type, a0, a1, a2, a3;
  const uint8_t prog[] = {
    OPLOADI, 1, 0, 5,
    OPLOADI, 2, 0, 1,
    OPSUB,   1, 1, 2,
    OPJZ,    1, 0, 12,
    OPMOV, 3, 1, 0,
    OPCALLHOST, 0, 0, 0,
    OPJMP,   0, 0xFF, 0xEC,
    OPCALLHOST, 0, 0, 0,
    OPHALT, 0, 0, 0
  };
  vm v = {0};
  v.code = prog;
  v.ip = 0;
  v.hostcall[0] = hostprint;

  writei32(&v, VM_ADDR_VERSION, VM_MEMMAP_VERSION);
  vmrun(&v);
  ncmds = readi32(&v, VM_ADDR_CMD_COUNT);
  if (ncmds > VM_CMD_MAX) { ncmds = VM_CMD_MAX; }
  for (i = 0; i < ncmds; i++) {
    base = VM_ADDR_CMD_BUF + i * VM_CMD_STRIDE;
    type = readi32(&v, base);
    a0 = readi32(&v, base+4);
    a1 = readi32(&v, base+8);
    a2 = readi32(&v, base+12);
    a3 = readi32(&v, base+16);
    /* switch (type) { */
    /* case VM_CMD_DRAW_SPRITE: break; */
    /* case VM_CMD_PLAY_SOUND: break; */
    /* default: break; */
    /* } */
  }
  writei32(&v, VM_ADDR_CMD_COUNT, 0); 
}

int
hostprint(vm *v) {
  fprintf(stderr, "r3 = %d\n", v->r[3]);
  return 0;
}
