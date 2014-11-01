#ifndef CLOVER_UTIL_PREPROC_JOIN_HPP
#define CLOVER_UTIL_PREPROC_JOIN_HPP

#define JOIN_INDIRECTION(x, y) x##y
/// Joins two things by expanding macros first
/// e.g. JOIN(test, __LINE__) == test6
#define JOIN(x, y) JOIN_INDIRECTION(x, y)

#endif // CLOVER_UTIL_PREPROC_JOIN_HPP
