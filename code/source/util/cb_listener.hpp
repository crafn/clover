#ifndef CLOVER_UTIL_CB_LISTENER_HPP
#define CLOVER_UTIL_CB_LISTENER_HPP

#include "base_cb_listener.hpp"
#include "basecallbacker.hpp"
#include "build.hpp"
#include "script/typestring.hpp"

namespace clover {
namespace util {

template <typename CbType>
class CbListener : public BaseCbListener {
public:
	typedef typename CbType::Cb Cb;
	typedef typename CbType::LinkId LinkId;

	template <typename Script>
	static void registerToScript(Script& s);

	CbListener()
		: callbacker(nullptr){}

	CbListener(const CbListener& other)
		: CbListener(){
		operator=(other);
	}

	CbListener(CbListener&& other)
		: CbListener(){
		operator=(std::move(other));
	}

	CbListener& operator=(const CbListener& other){
		clear();
		if (other.callbacker)
			listen(*other.callbacker, other.linkId->callback);
		return *this;
	}

	CbListener& operator=(CbListener&& other){
		clear();
		if (other.callbacker)
			listen(*other.callbacker, other.linkId->callback);
		other.clear();
		return *this;
	}

	virtual ~CbListener(){
		clear();
	}

	void listen(const CbType& h, const Cb& func){
		ensure(!callbacker);
		callbacker= &h;
		linkId= h.addCb(*this, func);
	}

	bool isListening() const { return callbacker != nullptr; }

	const CbType& getCallbacker() const { ensure(callbacker); return *callbacker; }

	virtual void clear(){
		if (callbacker)
			callbacker->removeCb(linkId);
		callbacker= nullptr;
	}

private:
	void setCallbacker(const BaseCallbacker& c){
		callbacker= static_cast<const CbType*>(&c);
	}

	LinkId linkId;
	const CbType* callbacker;

};

/// Keeps track of multiple listeners, erases invalids
template <typename CbType>
class CbMultiListener {
public:
	typedef CbListener<CbType> ListenerType;
	typedef typename CbType::Cb Cb;

	void listen(const CbType& c, const Cb& func){
		listeners.pushBack(std::move(ListenerType()));
		listeners.back().listen(c, func);

		garbageCollect();
	}

	void stopListening(const CbType& c){
		for (auto it= listeners.begin(); it != listeners.end();){
			if (&it->getCallbacker() == &c){
				it= listeners.erase(it);
			}
			else
				++it;
		}
	}

	void clear(){
		listeners.clear();
	}

	bool empty() const {
		return listeners.empty();
	}

private:
	void garbageCollect(){
		for (auto it= listeners.begin(); it != listeners.end();){
			if (it->isListening()){
				 ++it;
			}
			else {
				it= listeners.erase(it);
			}
		}
	}

	util::LinkedList<CbListener<CbType>> listeners;
};

template <typename T>
template <typename Script>
void CbListener<T>::registerToScript(Script& s){
	using Type= CbListener<T>;
	s. template registerObjectType<Type>();
	s. template registerMethod(&Type::listen, "listen");
	s. template registerMethod(&Type::isListening, "isListening");
	s. template registerMethod(&Type::clear, "clear");
}

template <typename T>
struct TypeStringTraits<util::CbListener<T>> {
	static util::Str8 type(){ return "::CbListener<" + script::TypeString<T*>()() + ">"; }
};

} // util
} // clover

#endif // CLOVER_UTIL_CB_LISTENER_HPP