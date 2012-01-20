#ifndef ALPINOCORPUS_BUF_UTIL_HH
#define ALPINOCORPUS_BUF_UTIL_HH

namespace alpinocorpus {

namespace util {

template <typename T>
void writeToBuf(unsigned char *buf, T n)
{
	for (size_t i = 0; i < sizeof(T); ++i)
		buf[i] = (n >> i * 8) & 0xff;
}

}
}

#endif // ALPINOCORPUS_BUF_UTIL_HH

