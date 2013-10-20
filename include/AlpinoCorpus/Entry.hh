#ifndef ALPINOCORPUS_ENTRY_HH
#define ALPINOCORPUS_ENTRY_HH

#include <string>

namespace alpinocorpus
{

struct Entry {
  Entry() {}
  Entry(std::string name, std::string contents) :
      d_name(name), d_contents(contents) {}
  std::string const &contents() const;
  std::string const &name() const;
  std::string shortName() const;
private:
    // Note: we don't use PIMPL here to avoid overhead, since a user will
    // usually store many Entry instances. Adding a member breaks the ABI!
    std::string d_name;
    std::string d_contents;
};

inline std::string const &Entry::contents() const
{
    return d_contents;
}

inline std::string const &Entry::name() const
{
    return d_name;
}

}

#endif
