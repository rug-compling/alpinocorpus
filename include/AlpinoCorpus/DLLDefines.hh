#ifndef ALPINO_DLL_DEFINES_HH
#define ALPINO_DLL_DEFINES_HH

#if defined(_WIN32)
  #if defined(corpus_EXPORTS)
    #define INDEXED_CORPUS_EXPORT __declspec(dllexport)
  #else
    #define INDEXED_CORPUS_EXPORT __declspec(dllimport)
  #endif
#else // defined(_WIN32)
  #define INDEXED_CORPUS_EXPORT
#endif

#endif // ALPINO_DLL_DEFINES_HH