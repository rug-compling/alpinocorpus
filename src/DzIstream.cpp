#include <iostream>

#include "DzIstream.hh"
#include "DzIstreamBuf.hh"

namespace alpinocorpus {

DzIstream::DzIstream(char const *filename) : std::istream(0)
{
	d_streamBuf.reset(new DzIstreamBuf(filename));
	rdbuf(d_streamBuf.get());
}
    
}
