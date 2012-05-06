#include <string>

#include <AlpinoCorpus/IterImpl.hh>

namespace alpinocorpus {
    std::string IterImpl::contents(CorpusReader const &rdr) const
    {
        //return rdr.read(current());
        return std::string(); // XXX - should be a null string
    }

    void IterImpl::interrupt()
    {
        // XXX no default behavior implemented
    }
}
