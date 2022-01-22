#ifndef CLIP_H
#define CLIP_H
/**
 *  File used for declaring overflow/underflow-safe math macro operators.
 */

#include <climits>
#include <cfloat>
#include <cstdlib>

/**
 *  \brief Sum of two ints clipped at maximum or minimum int value, in case of overflow or underflow, respectively.
 *  \param a The first int operator.
 *  \param b The second int operator.
 *  \return Clipped sum result in int format.
 */
#define CLPD_INT_SUM(a,b) ( (a > 0)? ((INT_MAX - a < b)? INT_MAX : a + b) : ((INT_MIN - a > b)? INT_MIN : a + b) )

/**
 *  \brief Absolute difference between two integers, returned in unsigned int format due to possible range.
 *  \param a The first int operator.
 *  \param b The second int operator.
 *  \return Absolute difference result in unsigned int format.
 */
#define CLPD_INT_ABSDIFF(a,b) ( (unsigned int) labs((long int) a - (long int) b) )

/**
 *  \brief Conversion from signed integer representation to unsigned integer, clipping at 0 in case number is negative.
 *  \param a The signed int to be converted to unsigned int.
 *  \return Number in unsigned format.
 */
#define CLPD_INT2UINT(a) ( (a > 0)? (unsigned int) a : 0 )

/**
 *  \brief Sum of two unsigned ints clipped at maximum unsigned int value, in case of overflow.
 *  \param a The first unsigned int operator.
 *  \param b The second unsigned int operator.
 *  \return Clipped sum result in unsigned int format.
 */
#define CLPD_UINT_SUM(a,b) ( (UINT_MAX - a < b)? UINT_MAX : (a + b) )

/**
 *  \brief Subtraction of two unsigned ints clipped at 0, in case of underflow.
 *  \param a The first unsigned int operator.
 *  \param b The second unsigned int operator, to be subtracted from the first.
 *  \return Clipped subtraction result in unsigned int format.
 */
#define CLPD_UINT_DIFF(a,b) ( (a > b)? (a - b) : 0 )

/**
 *  \brief Sum of two floats clipped at maximum or minimum float value, in case of overflow or underflow, respectively.
 *  \param a The first float operator.
 *  \param b The second float operator.
 *  \return Clipped sum result in float format.
 */
#define CLPD_FLT_SUM(a,b) ( (a > 0)? ((FLT_MAX - a < b)? FLT_MAX : a + b) : ((-FLT_MAX - a > b)? -FLT_MAX : a + b) )

/**
 *  \brief Conversion of float number to int representation clipped at maximum or minimum int value, in case of overflow or underflow.
 *  \param a The float number to be converted to int.
 *  \return Number in int format.
 */
#define CLPD_FLT2INT(a) ( (a > 0)? ((a > (float) INT_MAX)? INT_MAX : (int) a) : ((a < (float) INT_MIN)? INT_MIN : (int) a) )

/**
 *  \brief Saturation of float number at maximum and minimum values of int representation.
 *  \param a The float number to be clipped.
 *  \return Clipped value in float format.
 */
#define CLPD_FLT_INTLIMS(a) ( (a > 0)? ((a > (float) INT_MAX)? (float) INT_MAX : a) : ((a < (float) INT_MIN)? (float) INT_MIN : a) )

#endif