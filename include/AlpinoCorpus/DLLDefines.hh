#ifndef ALPINO_DLL_DEFINES_HH
#define ALPINO_DLL_DEFINES_HH

#if defined(_WIN32)
  #if defined(alpino_corpus_EXPORTS)
    #define ALPINO_CORPUS_EXPORT __declspec(dllexport)
  #else
    #define ALPINO_CORPUS_EXPORT __declspec(dllimport)
  #endif
#else // defined(_WIN32)
  #define ALPINO_CORPUS_EXPORT
#endif

#endif // ALPINO_DLL_DEFINES_HH
