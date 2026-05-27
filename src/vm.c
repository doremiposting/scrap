#include <stdio.h>
#include <stdint.h>

#include "memmap.h"

#define VMREGCNT 16
typedef int32_t vmreg;
#define VMCODEMAX 65536
#define VMMEMMAX 65536
typedef struct vm vm;
#define VMMAXCYCLESDEF 200000
struct vm {
  vmreg r[VMREGCNT];
  uint8_t mem[VMMEMMAX];
  const uint8_t *code;
  size_t ip;
  int running;
  int32_t cyclemax;
  int32_t cyclerem;
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
  OPSTORE32,
  OPCALL,
  OPRET,
  OPCMP,
  OPJNZ,
  OPJNEG,
  OPJPOS,
  OPAND,
  OPOR,
  OPXOR,
  OPSHL,
  OPSHR,
  OPNOT,
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
vmfrmstart(vm *v, int32_t deltams, int32_t input,
    int32_t mousex, int32_t mousey, int32_t screenw,
    int32_t screenh) {
  v->cyclerem = v->cyclemax;
  writei32(v, VM_ADDR_VERSION, VM_MEMMAP_VERSION);
  writei32(v, VM_ADDR_DELTA_MS, deltams);
  writei32(v, VM_ADDR_INPUT, input);
  writei32(v, VM_ADDR_MOUSE_X, mousex);
  writei32(v, VM_ADDR_MOUSE_Y, mousey);
  writei32(v, VM_ADDR_SCREEN_W, screenw);
  writei32(v, VM_ADDR_SCREEN_H, screenh);
}

void
vmfrmend(vm *v) {
  writei32(v, VM_ADDR_CMD_COUNT, 0);
}

void
vmrun(vm *v) {
  uint8_t op, A, B, C;
  int16_t imm, offset;
  uint32_t addr, sp;
  size_t retaddr;
  v->running = 1;
  while (v->running) {
    if (v->cyclemax > 0) {
      if (v->cyclerem <= 0) {
        fprintf(stderr, "Cycle limit exceeded.\n");
        /* TODO: Change this from halting to "skipping" until next frame. */
        v->running = 0;
        break;
      }
      v->cyclerem--;
    }
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
        if (!(v->r[A])) { v->ip = (size_t)((int)v->ip + offset); }
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
      case OPCALL:
        offset = (int16_t)((B << 8) | C);
        sp = (uint32_t)v->r[15] - 4;
        if (sp + 3 >= VMMEMMAX) {
          fprintf(stderr, "VM: Call stack overflow @ %u\n", sp);
          v->running = 0;
          break;
        }
        retaddr = v->ip;
        v->mem[sp] = (uint8_t)(retaddr & 0xFF);
        v->mem[sp+1] = (uint8_t)((retaddr >> 8) & 0xFF);
        v->mem[sp+2] = (uint8_t)((retaddr >> 16) & 0xFF);
        v->mem[sp+3] = (uint8_t)((retaddr >> 24) & 0xFF);
        v->r[15] = (vmreg)sp;
        v->ip = (size_t)((int)v->ip + offset);
        break;
      case OPRET:
        sp = (uint32_t)v->r[15];
        if (sp + 3 >= VMMEMMAX) {
          fprintf(stderr, "VM: Ret stack overflow @ %u\n", sp);
          v->running = 0;
          break;
        }
        retaddr = (size_t)(
          (uint32_t)v->mem[sp] |
          (uint32_t)v->mem[sp+1] << 8 |
          (uint32_t)v->mem[sp+2] << 16 |
          (uint32_t)v->mem[sp+3] << 24
        );
        if (retaddr >= VMCODEMAX) {
          fprintf(stderr, "VM: Ret to OOB address @ %u\n", sp);
          v->running = 0;
          break;
        }
        v->r[15] = (vmreg)(sp + 4);
        v->ip = retaddr;
        break;
      case OPCMP:
        if (v->r[B] < v->r[C]) { v->r[A] = -1; }
        else if (v->r[B] > v->r[C]) { v->r[A] = 1; }
        else { v->r[A] = 0; }
        break;
      case OPJNZ:
        offset = (int16_t)((B << 8) | C);
        if (v->r[A] != 0) { v->ip = (size_t)((int)v->ip + offset); }
        break;
      case OPJNEG:
        offset = (int16_t)((B << 8) | C);
        if (v->r[A] < 0) { v->ip = (size_t)((int)v->ip + offset); }
        break;
      case OPJPOS:
        offset = (int16_t)((B << 8) | C);
        if (v->r[A] > 0) { v->ip = (size_t)((int)v->ip + offset); }
        break;
      case OPAND:
        v->r[A] = v->r[B] & v->r[C];
        break;
      case OPOR:
        v->r[A] = v->r[B] | v->r[C];
        break;
      case OPXOR:
        v->r[A] = v->r[B] ^ v->r[C];
        break;
      case OPSHL:
        v->r[A] = (vmreg)((uint32_t)v->r[B] << (v->r[C] & 0x1F));
        break;
      case OPSHR:
        v->r[A] = (vmreg)((uint32_t)v->r[B] >> (v->r[C] & 0x1F));
        break;
      case OPNOT:
        v->r[A] = ~(v->r[B]);
        break;
      default:
        fprintf(stderr, "Unknown instruction encountered: %u\n", v->r[A]);
        v->running = 0;
        break;
    }
  }
}

void
vmtest() {
  const uint8_t prog[] = {
    OPLOADI, 1, 0, 3,
    OPLOADI, 2, 0, 7,
    /* call/ret */
    OPCALL, 0, 0, 0xA0, /* offset 108 to call subrot @ 120 */
    OPCALLHOST, 0, 0, 0, /* expect: r3 = 99 */
    /* write then read 1234 to scratch */
    OPLOADI, 4, 0x05, 0x00, /* 0x0500 (VM_ADDR_SCRATCH) */
    OPLOADI, 3, 0x04, 0xD2,
    OPSTORE32, 3, 4, 0,
    OPLOAD32, 5, 4, 0,
    OPMOV, 3, 5, 0,
    OPCALLHOST, 0, 0, 0, /* expect: r3 = 1234 */
    /* write 171 to scratch+4 then read */
    OPLOADI, 6, 0x05, 0x04,
    OPLOADI, 3, 0x00, 0xAB,
    OPSTORE8, 3, 6, 0,
    OPLOAD8, 7, 6, 0,
    OPMOV, 3, 7, 0,
    OPCALLHOST, 0, 0, 0, /* expect: r3 = 171 */
    /* CMP + JPOS */
    OPCMP, 0, 2, 1,
    OPMOV, 3, 0, 0,
    OPCALLHOST, 0, 0, 0, /* expect: r3 = 1 */
    OPJPOS, 0, 0x00, 0x04,
    OPCALLHOST, 0, 0, 0, /* expect skip, should not print */
    /* CMP + JNZ */
    OPCMP, 0, 1, 2,
    OPJNZ, 0, 0x00, 0x04,
    OPCALLHOST, 0, 0, 0, /* expect skip, should not print */
    /* AND */
    OPLOADI, 8, 0x00, 0x02,
    OPAND, 3, 1, 8,
    OPCALLHOST, 0, 0, 0, /* expect r3 = 2 */
    /* OR */
    OPLOADI, 8, 0x00, 0x04,
    OPOR, 3, 1, 8,
    OPCALLHOST, 0, 0, 0, /* expect r3 = 7 */
    /* XOR */
    OPXOR, 3, 2, 1,
    OPCALLHOST, 0, 0, 0, /* expect r3 = 4 */
    /* SHL */
    OPLOADI, 8, 0x00, 0x01,
    OPLOADI, 9, 0x00, 0x03,
    OPSHL, 3, 8, 9,
    OPCALLHOST, 0, 0, 0, /* expect r3 = 8 */
    /* SHR */
    OPLOADI, 9, 0x00, 0x02,
    OPSHR, 3, 3, 9,
    OPCALLHOST, 0, 0, 0, /* expect r3 = 2 */
    /* NOT */
    OPLOADI, 8, 0x00, 0x00,
    OPNOT, 3, 8, 0,
    OPCALLHOST, 0, 0, 0, /* expect r3 = -1 */
    /* Done. */
    OPHALT, 0, 0, 0,
    /* subrot from earlier: r[3] = 99 then RET */
    OPLOADI, 3, 0x00, 0x63,
    OPRET, 0, 0, 0,
  };
  vm v = {0};
  v.code = prog;
  v.ip = 0;
  v.cyclemax = 0;
  v.r[15] = VM_STACK_INIT;
  v.hostcall[0] = hostprint;

  vmfrmstart(&v, 0, 0, 0, 0, 640, 480);
  vmrun(&v);
  vmfrmend(&v);
}

int
hostprint(vm *v) {
  fprintf(stderr, "r3 = %d\n", v->r[3]);
  return 0;
}
