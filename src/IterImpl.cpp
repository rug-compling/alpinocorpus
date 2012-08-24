#include <cmath>
#include <string>

#include <AlpinoCorpus/IterImpl.hh>

namespace alpinocorpus {
    bool IterImpl::hasProgress()
    {
        return false;
    }

    double IterImpl::progress()
    {
        return NAN;
    }

    void IterImpl::interrupt()
    {
        // XXX no default behavior implemented
    }
}
