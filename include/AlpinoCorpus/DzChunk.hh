#ifndef DZ_CHUNK_HH
#define DZ_CHUNK_HH

#include <QtGlobal>

struct DzChunk
{
    DzChunk(quint64 newOffset, quint64 newSize) : offset(newOffset), size(newSize) {}
    DzChunk() : offset(0), size(0) {} // hmz
    quint64 offset;
    quint64 size;
};

#endif // DZ_CHUNK_HH
