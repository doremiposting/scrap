#ifndef MAIN_H
#define MAIN_H

#if defined(__APPLE__)
  #define forceinline inline __attribute__((__always_inline__))
/*
#elif defined(_MSC_VER)
  #define forceinline __forceinline
*/
#elif defined(__GNUC__)
  #define forceinline inline __attribute__((__always_inline__))
#endif /* DEFINED(PLATFORM) */

#define da_append(xs, x) do {\
  if (xs.count >= xs.capacity) {\
    if (xs.capacity == 0) { xs.capacity = 256; }\
    else { xs.capacity *= 2; }\
    xs.items = realloc(xs.items, xs.capacity*sizeof(*xs.items));\
  }\
  xs.items[xs.count++] = x;\
} while (0)

#define UNUSED(x) (void)(x)

#define EVTICKNS 600000000LL
#define GFXTICKNS 16666667LL
#define GETNS(ts) (clock_gettime(CLOCK_MONOTONIC, &ts))
#define DIFFNS(start, end) \
    ((int64_t)((end).tv_sec - (start).tv_sec) * 1000000000LL + \
     ((end).tv_nsec - (start).tv_nsec))

#endif /* MAIN_H */
