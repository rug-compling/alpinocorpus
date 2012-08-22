#ifndef ALPINOCORPUS_ITERIMPL_HH
#define ALPINOCORPUS_ITERIMPL_HH

#include <AlpinoCorpus/Entry.hh>

namespace alpinocorpus {

    class CorpusReader;

    // Iterator body. We need handle-body/proxy/pimpl for polymorphic copy.
    struct IterImpl {
        virtual ~IterImpl() {}
        virtual IterImpl *copy() const = 0;
        //virtual bool equals(IterImpl const &) const = 0;
        virtual bool hasNext() = 0;
        virtual Entry next(CorpusReader const &rdr) = 0;

        // Query iterators must override this
        virtual void interrupt();
    };

}

#endif // ALPINOCORPUS_ITERIMPL_HH
