#ifndef CLOVER_UTIL_HANDLE_HPP
#define CLOVER_UTIL_HANDLE_HPP

#include "build.hpp"
#include "util/cb_listener.hpp"
#include "util/class_preproc.hpp"
#include "util/ensure.hpp"

#include <type_traits>

namespace clover {
namespace util {

/// Generic handle
/// Works like a pointer, but resets itself when the pointee is destroyed
template <typename T>
class Handle;

/// Default implementation by using destroy callback
template <typename T>
class Handle {
public:
	static_assert(
			std::is_move_constructible<T>::value == false,
			"Not supported (yet), callbacks should be adjusted properly");

	Handle()= default;
	DEFAULT_COPY(Handle<T>);
	DEFAULT_MOVE(Handle<T>);

	Handle(T* value){ set(value); }
	Handle& operator=(T* value){ set(value); return *this; }

	T* operator->() const { return NONULL(ptr); }
	T& operator*() const { return *NONULL(ptr); }
	explicit operator bool() const { return ptr != nullptr; }

	void clear(){
		ptr= nullptr;
		destroyListener.clear();
	}

private:
	void set(T* value){
		ptr= value;
		destroyListener.clear();

		if (ptr){
			destroyListener.listen(*ptr, [this](){
				ptr= nullptr;
			});
		}
	}

	T* ptr= nullptr;

	CbListener<OnDestroyCb> destroyListener;
};

} // util
} // clover

#endif // CLOVER_UTIL_HANDLE_HPP