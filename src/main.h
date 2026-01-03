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

#endif /* MAIN_H */