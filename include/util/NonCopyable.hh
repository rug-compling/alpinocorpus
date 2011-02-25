#ifndef _AC_UTIL_NONCOPYABLE_HPP
#define _AC_UTIL_NONCOPYABLE_HPP

namespace alpinocorpus { namespace util {

/**
 * Base class for objects without copy constructor and assignment.
 */
class NonCopyable {
  protected:
    NonCopyable() {}
    ~NonCopyable() {}
  private:
    NonCopyable(NonCopyable const &);
    NonCopyable &operator=(NonCopyable const &);
};

} }     // namespace alpinocorpus::util

#endif  // _AC_UTIL_NONCOPYABLE_HPP
