#ifndef LIBPNG_PNGLIBCONF_H
#define LIBPNG_PNGLIBCONF_H

#define HAVE_MEMORY_H 1
#define HAVE_MEMSET 1
#define HAVE_POW 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define STDC_HEADERS 1

#define PNG_WRITE_SUPPORTED
#define PNG_USE_LOCAL_ARRAYS
#define PNG_ALWAYS_EXTERN
#define PNG_READ_SUPPORTED
#define PNG_READ_16_TO_8_SUPPORTED
#define PNG_READ_EXPAND_SUPPORTED
#define PNG_SEQUENTIAL_READ_SUPPORTED
#define PNG_sBIT_SUPPORTED
#define PNG_READ_RGB_TO_GRAY_SUPPORTED
#define PNG_SETJMP_SUPPORTED
#define PNG_WRITE_PACK_SUPPORTED
#define PNG_WRITE_SHIFT_SUPPORTED
#define PNG_hIST_SUPPORTED
#define PNG_INFO_IMAGE_SUPPORTED
#define LIBPNG_WAS_COMPILED_WITH__PNG_NO_SETJMP 1

#define PNG_API_RULE 0
#define PNG_CALLOC_SUPPORTED
#define PNG_COST_SHIFT 3
#define PNG_DEFAULT_READ_MACROS 1
#define PNG_GAMMA_THRESHOLD_FIXED 5000
#define PNG_IDAT_READ_SIZE PNG_ZBUF_SIZE
#define PNG_INFLATE_BUF_SIZE 1024
#define PNG_MAX_GAMMA_8 11
#define PNG_QUANTIZE_BLUE_BITS 5
#define PNG_QUANTIZE_GREEN_BITS 5
#define PNG_QUANTIZE_RED_BITS 5
#define PNG_TEXT_Z_DEFAULT_COMPRESSION (-1)
#define PNG_TEXT_Z_DEFAULT_STRATEGY 0
#define PNG_WEIGHT_SHIFT 8
#define PNG_ZBUF_SIZE 8192
#define PNG_ZLIB_VERNUM 0 /* unknown */
#define PNG_Z_DEFAULT_COMPRESSION (-1)
#define PNG_Z_DEFAULT_NOFILTER_STRATEGY 0
#define PNG_Z_DEFAULT_STRATEGY 1
#define PNG_sCAL_PRECISION 5
#define PNG_sRGB_PROFILE_CHECKS 2

#endif // LIBPNG_PNGLIBCONF_H