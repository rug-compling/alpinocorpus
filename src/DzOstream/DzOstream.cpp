#include "DzOstream.ih"

DzOstream::DzOstream(char const *filename) : std::ostream(0)
{
        d_streamBuf = QSharedPointer<DzOstreamBuf>(new DzOstreamBuf(filename));
        rdbuf(d_streamBuf.data());
}
