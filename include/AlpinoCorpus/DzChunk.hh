#ifndef ALPINO_DZ_CHUNK_HH
#define ALPINO_DZ_CHUNK_HH

#include <QtGlobal>

namespace alpinocorpus {

struct DzChunk
{
    DzChunk(quint64 newOffset, quint64 newSize) : offset(newOffset), size(newSize) {}
    DzChunk() : offset(0), size(0) {} // hmz
    quint64 offset;
    quint64 size;
};

}

#endif  // ALPINO_DZ_CHUNK_HH
