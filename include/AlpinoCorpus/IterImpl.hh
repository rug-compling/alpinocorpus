#ifndef ALPINOCORPUS_ITERIMPL_HH
#define ALPINOCORPUS_ITERIMPL_HH

namespace alpinocorpus {

    class CorpusReader;

    // Iterator body. We need handle-body/proxy/pimpl for polymorphic copy.
    struct IterImpl {
        virtual ~IterImpl() {}
        virtual IterImpl *copy() const = 0;
        virtual std::string current() const = 0;
        virtual bool equals(IterImpl const &) const = 0;
        virtual void next() = 0;

        // Query iterators must override this
        virtual std::string contents(CorpusReader const &rdr) const;
        virtual void interrupt();
    };

}

#endif // ALPINOCORPUS_ITERIMPL_HH
