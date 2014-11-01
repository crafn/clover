#ifndef CLOVER_UTIL_CLASS_PREPROC_HPP
#define CLOVER_UTIL_CLASS_PREPROC_HPP

#define DEFAULT_COPY(Class)\
	Class(const Class&)= default;\
	Class& operator=(const Class&)= default

#define DELETE_COPY(Class)\
	Class(const Class&)= delete;\
	Class& operator=(const Class&)= delete

#define DEFAULT_MOVE(Class)\
	Class(Class&&)= default;\
	Class& operator=(Class&&)= default

#define DELETE_MOVE(Class)\
	Class(Class&&)= delete;\
	Class& operator=(Class&&)= delete

#endif // CLOVER_UTIL_CLASS_PREPROC_HPP
