#ifndef CLOVER_UTIL_PRINTF_FORMAT_HPP
#define CLOVER_UTIL_PRINTF_FORMAT_HPP

#ifdef __GNUC__
#define PRINTF_FORMAT(stringIndex, firstToCheck) \
  __attribute__ ((format (printf, (stringIndex), (firstToCheck))))
#else
#define PRINTF_FORMAT(a, b)
#endif

#endif // CLOVER_UTIL_PRINTF_FORMAT_HPP
