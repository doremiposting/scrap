#ifndef VM_H
#define VM_H
#include <stdint.h>
typedef struct vm vm;
void writei32(vm *v, uint16_t addr, int32_t val);
void readi32(const vm *v, uint16_t addr);

void vmtest();

#endif /* VM_H */
