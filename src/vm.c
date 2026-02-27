#include <stdio.h>
#include <stdint.h>

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
  OPCALLHOST,
} opcode;

int hostprint(vm *v);

void
vmrun(vm *v) {
  uint8_t op, A, B, C;
  int16_t imm;
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
        v->r[A] = v->r[B] / v->r[C];
        break;
      case OPCALLHOST:
        v->hostcall[A](v);
        break;
      default:
        v->running = 0;
        break;
    }
  }
}

void
vmtest() {
  const uint8_t prog[] = {
    OPLOADI, 1, 0, 10,
    OPLOADI, 2, 0, 20,
    OPADD, 3, 1, 2,
    OPCALLHOST, 0, 0, 0,
    OPHALT, 0, 0, 0
  };
  vm v = {0};
  v.code = prog;
  v.ip = 0;
  v.hostcall[0] = hostprint;
  vmrun(&v);
}

int
hostprint(vm *v) {
  fprintf(stderr, "r3 = %d\n", v->r[3]);
  return 0;
}
