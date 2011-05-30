#ifndef BUF_UTIL_HH
#define BUF_UTIL_HH

template <typename T>
void writeToBuf(char *buf, T n)
{
	for (size_t i = 0; i < sizeof(T); ++i)
		buf[i] = (n >> i * 8) & 0xff;
}

#endif // BUF_UTIL_HH

