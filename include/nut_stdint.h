/*
 * nut_stdint.h - Network UPS Tools sets of integer types having specified widths 
 * 
 * Copyright (C) 2011	Arjen de Korte <adkorte-guest@alioth.debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include <limits.h>


#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)(-1LL))
#endif

#ifndef SSIZE_MAX
#define SSIZE_MAX ((ssize_t)(-1LL))
#endif

/* Printing format for size_t and ssize_t */
#ifndef PRIuSIZE
# ifdef PRIsize
#  define PRIuSIZE PRIsize
# else
#   define PRIuSIZE "zu"
# endif
#endif

#ifndef PRIxSIZE
#   define PRIxSIZE "zx"
#endif

/* Note: Windows headers are known to define at least "d" values,
 * so macros below revolve around that and not "i" directly */
#ifndef PRIiSIZE
# ifdef PRIssize
#  define PRIiSIZE PRIssize
# else
#  ifdef PRIdSIZE
#   define PRIiSIZE PRIdSIZE
#  else
#   define PRIiSIZE "zd"
#   define PRIdSIZE PRIiSIZE
#  endif
# endif
#else
# ifndef PRIdSIZE
#  define PRIdSIZE PRIiSIZE
# endif
#endif /* format for size_t and ssize_t */

/* Printing format for uintmax_t and intmax_t */
#ifndef PRIuMAX
/* assume new enough compiler and standard... check "%j" support via configure? */
#  define PRIuMAX "ju"
#endif /* format for uintmax_t and intmax_t */

#ifndef PRIdMAX
# ifdef PRIiMAX
#  define PRIdMAX PRIiMAX
# else
/* assume new enough compiler and standard... check "%j" support via configure? */
#  define PRIdMAX "jd"
#  define PRIiMAX PRIdMAX
# endif
#else
# ifndef PRIiMAX
#  define PRIiMAX PRIdMAX
# endif
#endif /* format for uintmax_t and intmax_t */

#ifndef PRIxMAX
/* assume new enough compiler and standard... check "%j" support via configure? */
#  define PRIxMAX "jx"
#endif /* format for uintmax_t and intmax_t */

