#ifndef int32t_h
#define int32t_h

#include "config.h"

#if HAVE_STDINT_H
#include <stdint.h>
#elif HAVE_INTTYPES_H && HAVE_UINT_LEAST32_T
#include <inttypes.h>
#elif HAVE_SYS_TYPES_H && HAVE_U_INT32_T
#include <sys/types.h>
typedef int32_t int_least32_t;
typedef uint32_t uint_least32_t;
#elif __TURBOC__ || __PACIFIC__
typedef long int_least32_t;
typedef unsigned long uint_least32_t;
#elif !defined(_STDINT_H)
typedef int int_least32_t;
typedef unsigned uint_least32_t;
#endif

#endif /* !int32t_h */
