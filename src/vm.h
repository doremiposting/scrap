#ifndef VM_H
#define VM_H
#include <stdint.h>
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
  FLOADI,
  FMOV,
  FADD,
  FSUB,
  FMUL,
  FDIV,
  FCMP,
  FLOAD32,
  FSTORE32,
  ITOF,
  FTOI,
} opcode;
typedef struct vm vm;
void writei32(vm *v, uint32_t addr, int32_t val);
int32_t readi32(const vm *v, uint32_t addr);

void vmfrmstart(vm *v, int32_t deltams, int32_t input,
    int32_t mousex, int32_t mousey, int32_t screenw,
    int32_t screenh);
void vmfrmend(vm *v);

typedef struct {
  int32_t type;
  int32_t arg[4];
} vmcmd;
int vmcmdcount(const vm *v);
int vmcmdget(const vm *v, int i, vmcmd *out);

void vmtest();

#endif /* VM_H */
