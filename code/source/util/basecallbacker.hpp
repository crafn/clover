#ifndef CLOVER_UTIL_BASECALLBACKER_HPP
#define CLOVER_UTIL_BASECALLBACKER_HPP

#include "base_cb_listener.hpp"
#include "build.hpp"
#include "linkedlist.hpp"
#include "util/ensure.hpp"

#include <functional>

namespace clover {
namespace util {

template <typename T>
class CbListener;

class ENGINE_API BaseCallbacker {
public:
	virtual ~BaseCallbacker(){}
protected:
};

/// Calls callbacks of a certain type
template <typename... Args>
class ENGINE_API SingleCallbacker : public BaseCallbacker {
public:

	using Cb= std::function<void (Args...)>;

	struct Link {
		BaseCbListener* listener;
		Cb callback;
	};

	using SingleCallbackerType= SingleCallbacker<Args...>;
	using LinkId= typename util::LinkedList<Link>::Iter;

	SingleCallbacker()= default;

	/// Don't copy callbacks
	SingleCallbacker(const SingleCallbacker&){}

	SingleCallbacker(SingleCallbacker&& other){
		operator=(std::move(other));
	}

	SingleCallbacker& operator=(const SingleCallbacker&){ return *this; }
	SingleCallbacker& operator=(SingleCallbacker&& other){
		// Transfer links
		links= std::move(other.links);

		for (auto& m : links){
			m.listener->setCallbacker(*this);
		}

		other.links.clear();
		return *this;
	}

	virtual ~SingleCallbacker(){
		clear();
	}

	void trigger(Args... args) const {
		// Iterating simply through the list won't work because callbacks can change links

		// Also can't just copy callbacks and iterate through that, because leading callback
		// can destroy listener of the following callback which causes a SIGSEGV

		// Solution: copy links, but detect removed listeners. Listeners added during callbacks
		// aren't taken account because it isn't intuitively expected and causes easily infinite recursive loops
		auto copy= links;
		for (auto& m : copy){

			// The simplest way to detect removed listeners, but a bit slow
			bool found= false;
			for (auto& m2 : links){
				if (m2.listener == m.listener){
					found= true;
					break;
				}
			}

			if (!found) continue;

			m.callback(args...);
		}
	}

protected:
	template <typename T>
	friend class CbListener;

	void clear(){
		while (!links.empty()){
			ensure(links.back().listener);
			links.back().listener->clear();
		}
	}

	LinkId addCb(BaseCbListener& l, const Cb& cb) const {
		links.pushBack({&l, cb});
		auto ret= links.end();
		--ret;
		return ret;
	}

	void removeCb(const LinkId& id) const { links.erase(id); }

private:
	// Callbacks should be possible to listen from const object too
	mutable util::LinkedList<Link> links;
};

// Some common callbacks
struct ENGINE_API OnChangeCb : public SingleCallbacker<>{};
struct ENGINE_API OnDestroyCb : public SingleCallbacker<>{};
	
} // util
} // clover

#endif // CLOVER_UTIL_BASECALLBACKER_HPP
