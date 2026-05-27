#ifndef VM_H
#define VM_H
#include <stdint.h>
typedef struct vm vm;
void writei32(vm *v, uint16_t addr, int32_t val);
int32_t readi32(const vm *v, uint16_t addr);

void vmfrmstart(vm *v, int32_t deltams, int32_t input,
    int32_t mousex, int32_t mousey, int32_t screenw,
    int32_t screenh);
void vmfrmend(vm *v);

void vmtest();

#endif /* VM_H */
