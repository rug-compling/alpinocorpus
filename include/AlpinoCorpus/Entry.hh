#ifndef ALPINOCORPUS_ENTRY_HH
#define ALPINOCORPUS_ENTRY_HH

#include <string>

namespace alpinocorpus
{

struct Entry {
  Entry() {}
  Entry(std::string name, std::string contents) :
      d_name(name), d_contents(contents) {}
  std::string const &contents();
  std::string const &name();
  std::string shortName();
private:
    // Note: we don't use PIMPL here to avoid overhead, since a user will
    // usually store many Entry instances. Adding a member breaks the ABI!
    std::string d_name;
    std::string d_contents;
};

inline std::string const &Entry::contents()
{
    return d_contents;
}

inline std::string const &Entry::name()
{
    return d_name;
}

}

#endif
