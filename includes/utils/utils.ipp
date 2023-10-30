#include "utils/utils.hpp"

/* ************************************************************************** */
/* *                            BitMasks handling                           * */
/* ************************************************************************** */

inline void	toggleBits(int &mask, const int &bits)
{
	mask ^= bits;
}

inline void	setBits(int &mask, const int &bits)
{
	mask |= bits;
}

inline void	removeBits(int &mask, const int &bits)
{
	mask &= ~bits;
}
