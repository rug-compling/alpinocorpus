#ifndef ALPINOCORPUS_EQUALSPREVIOUS_HH
#define ALPINOCORPUS_EQUALSPREVIOUS_HH

#include <functional>

#include <boost/shared_ptr.hpp>

template <typename T>
class EqualsPrevious : public std::unary_function<T, bool>
{
public:
  bool operator()(T const &val) const
  {
    if (!d_previous) {
      d_previous.reset(new T(val));
      return false;
    } else if (*d_previous != val) {
      *d_previous = val;
      return false;
    }

    return true;
  }

  // operator() is often expected to be const.
  mutable boost::shared_ptr<T> d_previous;
};

template <typename T>
class NotEqualsPrevious : public std::unary_function<T, bool>
{
public:
  bool operator()(T const &val) const
  {
  	return !d_equalsPrevious(val);
  }

  EqualsPrevious<T> d_equalsPrevious;
};

#endif // ALPINOCORPUS_EQUALSPREVIOUS
