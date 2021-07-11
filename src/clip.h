#ifndef CLIP_H
#define CLIP_H
/**
 *  File used for declaring macro operators to be used for safe math operations, inside variable range.
 */

#include <climits>
#include <cfloat>

/**
 *  \brief TODO comment
 */
#define CLPD_INT_SUM(a,b) ( (a > 0)? ((INT_MAX - a < b)? INT_MAX : a + b) : ((INT_MIN - a > b)? INT_MIN : a + b) )

#define CLPD_FLT_SUM(a,b) ( (a > 0)? ((FLT_MAX - a < b)? FLT_MAX : a + b) : ((-FLT_MAX - a > b)? -FLT_MAX : a + b) )

#define CLPD_FLT2INT(a) ( (a > 0)? ((a > (float) INT_MAX)? INT_MAX : (int) a) : ((a < (float) INT_MIN)? INT_MIN : (int) a) )

#endif