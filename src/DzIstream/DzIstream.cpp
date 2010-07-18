#include "DzIstream.ih"

DzIstream::DzIstream(char const *filename) : std::istream(0)
{
        d_streamBuf = QSharedPointer<DzIstreamBuf>(new DzIstreamBuf(filename));
        rdbuf(d_streamBuf.data());
}
