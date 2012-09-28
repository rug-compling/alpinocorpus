#ifndef ALPINOCORPUS_EITHER_HH
#define ALPINOCORPUS_EITHER_HH

namespace alpinocorpus {

/**
 * Either is an option type that resembles the Either type in Haskell.
 * It's used to indicate that computation results in two possible type:
 * the left type or the right type (which could have the same type).
 *
 * Unfortunately, we do not have algebraic data types in C++, so both
 * posibilities are encoded in the same type. The isLeft() and isRight()
 * methods can be used to probe whether we have a left or right value.
 *
 * In error handling the convention is that <tt>Left L</tt> is an error,
 * and information about the error is encoded using the type <tt>L</tt>.
 * <tt>Right R</tt> is a succesful computation with a result of the type
 * <tt>R</tt>.
 *
 * Note: we could have used boost::variant, but I think it is kinda
 *       heavyweight. It does more, but also has a more complex API.
 */
template <typename L, typename R>
class Either {
public:
	static Either left(L left);
	static Either right(R right);

	bool isLeft();
	bool isRight();
	L left();
	R right();
private:
	enum Result { LEFT, RIGHT };

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
