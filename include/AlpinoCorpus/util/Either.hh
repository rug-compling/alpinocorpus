#ifndef ALPINOCORPUS_EITHER_HH
#define ALPINOCORPUS_EITHER_HH

namespace alpinocorpus {

template <typename L, typename R>
class Either {
public:
	enum Result { LEFT, RIGHT };

	static Either left(L left);
	static Either right(R right);

	bool isLeft();
	bool isRight();
	L left();
	R right();
private:
	Either(Result result, L left, R right) :
		d_result(result), d_left(left), d_right(right) {}

	Result d_result;
	L d_left;
	R d_right;
};

template <typename L, typename R>
inline Either<L, R> Either<L, R>::left(L left)
{
	return Either(LEFT, left, R());
}

template <typename L, typename R>
inline Either<L, R> Either<L, R>::right(R right)
{
	return Either(RIGHT, L(), right);
}

template <typename L, typename R>
inline bool Either<L, R>::isLeft()
{
	return d_result == LEFT;
}

template <typename L, typename R>
inline bool Either<L, R>::isRight()
{
	return d_result == RIGHT;
}

template <typename L, typename R>
inline L Either<L, R>::left()
{
	return d_left;
}

template <typename L, typename R>
inline R Either<L, R>::right()
{
	return d_right;
}

struct Empty {};

}

#endif // ALPINOCORPUS_EITHER_HH
