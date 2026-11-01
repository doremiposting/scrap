#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gptable.h"

#define TABLELINEWIDTH 512

static char *
nexttoken(char **cursor) {
  char *s, *start;
  s = *cursor;
  while (*s == ' ' || *s == '\t') { s++; }
  if (*s == '\0' || *s == '\n') { return NULL; }
  if (*s == '"') {
    s++;
    start = s;
    while (*s && *s != '"' && *s != '\n') { s++; }
    if (*s == '"') { *s++ = '\0'; } else { *s = '\0'; }
    *cursor = s;
    return start;
  }
  start = s;
  while (*s && *s != ' ' && *s != '\t' && *s != '\n') { s++; }
  if (*s) { *s++ = '\0'; }
  *cursor = s;
  return start;
}

int
loadtable(const char *fn, const Fielddef *fields, int nfields,
    void *out, int recsize, int maxrecs) {
  FILE *f;
  char line[TABLELINEWIDTH];
  char *cursor, *tok, *endp, *rec, *dest;
  int lineno, nrec, fi;
  long sv;
  unsigned long uv;
  float fv;

  f = fopen(fn, "r");
  if (!f) {
    fprintf(stderr, "loadtable: couldn't open %s\n", fn);
    return -1;
  }
  lineno = 0;
  nrec = 0;
  while (fgets(line, sizeof(line), f)) {
    lineno++;
    cursor = line;
    while (*cursor == ' ' || *cursor == '\t') { cursor++; }
    if (*cursor == '#' || *cursor == '\n' || *cursor == '\0') { continue; }
    if (nrec >= maxrecs) {
      fprintf(stderr, "%s:%d: too many records (max %d)\n", fn, lineno, maxrecs);
      fclose(f);
      return -1;
    }
    rec = (char *)out + (size_t)nrec * (size_t)recsize;
    for (fi = 0; fi < nfields; fi++) {
      tok = nexttoken(&cursor);
      if (!tok) {
        fprintf(stderr, "%s:%d: expeected %d fields, got %d\n", fn, lineno, nfields, fi);
        fclose(f);
        return -1;
      }
      dest = rec + fields[fi].offset;
      switch (fields[fi].kind) {
        case FIELD_STR:
          if (strlen(tok) >= fields[fi].size) {
            fprintf(stderr, "%s:%d: field '%s' too long (max %d chars)\n",
                fn, lineno, fields[fi].name, (int)(fields[fi].size - 1));
          }
          strcpy(dest, tok);
          break;
        case FIELD_INT:
          sv = strtol(tok, &endp, 10);
          if (*endp != '\0') {
            fprintf(stderr, "%s:%d: field '%s' expected int, got '%s'\n",
                fn, lineno, fields[fi].name, tok);
            fclose(f);
            return -1;
          }
          *(int *)dest = (int)sv;
          break;
        case FIELD_UINT:
          if (*tok == '-') {
            fprintf(stderr, "%s:%d: field '%s' expectd unsigned, got '%s'\n",
                fn, lineno, fields[fi].name, tok);
            fclose(f);
            return -1;
          }
          uv = strtoul(tok, &endp, 10);
          if (*endp != '\0') {
            fprintf(stderr, "%s:%d: field '%s' expected unsigned, got '%s'\n",
                fn, lineno, fields[fi].name, tok);
            fclose(f);
            return -1;
          }
          *(unsigned *)dest = (unsigned)uv;
          break;
        case FIELD_FLOAT:
          fv = strtof(tok, &endp);
          if (*endp != '\0') {
            fprintf(stderr, "%s:%d: field '%s' expected float, got '%s'\n",
                fn, lineno, fields[fi].name, tok);
            fclose(f);
            return -1;
          }
          *(float *)dest = fv;
          break;
      }
    }
    if (nexttoken(&cursor)) {
      fprintf(stderr, "%s:%d: too many fields, expected %d)\n",
          fn, lineno, nfields);
      fclose(f);
      return -1;
    }
    nrec++;
  }
  fclose(f);
  return nrec;
}
