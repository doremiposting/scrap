#ifndef GPTABLE_H
#define GPTABLE_H

#include <stddef.h>

#define FIELDCOUNT(arr) (int)(sizeof(arr)/sizeof((arr)[0]))

typedef enum { FIELD_STR, FIELD_INT, FIELD_UINT, FIELD_FLOAT } Fieldkind;

typedef struct {
  const char *name;
  Fieldkind kind;
  size_t offset;
  size_t size;
} Fielddef;

int loadtable(const char *fn, const Fielddef *fields, int nfields,
    void *out, int recsize, int maxrecs);

#endif /* GPTABLE_H */
