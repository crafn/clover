#ifndef CLOVER_UTIL_CALLBACKER_HPP
#define CLOVER_UTIL_CALLBACKER_HPP

#include "build.hpp"
#include "basecallbacker.hpp"

namespace clover {
namespace util {
	
/// System to make easily registrable callback calls
/// Can be used when events would be an ovescaled (or too slow) solution

/// Usage:
///
///	CbListener<OnChangeCbType> listener;
///	ChangingThingy t;
///	listener.listen(t, <function-to-be-called>);
///	// ...
///	struct ChangingThingy : public Callbacker<OnChangeCbType> {
///		void changeMe(){
///			++a;
///			onChange(); // Calls <function-to-be-called>
///		}
///	};

/// This is just for readibility of inheritance
/// e.g. class ChangingThingy : public Callbacker<all_supported_callback_type-classes_here>
/// Making use of deadly diamond of death here..!
template <typename... CbTypes>
class Callbacker : public CbTypes... {
public:
	Callbacker()= default;
	Callbacker(const Callbacker&)= default;
	Callbacker(Callbacker&&)= default;
	virtual ~Callbacker(){}

	Callbacker& operator=(const Callbacker&)= default;
	Callbacker& operator=(Callbacker&&)= default;
};
	
} // util
} // clover

#endif // CLOVER_UTIL_CALLBACKER_HPP