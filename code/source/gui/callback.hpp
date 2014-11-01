#ifndef CLOVER_GUI_CALLBACK_HPP
#define CLOVER_GUI_CALLBACK_HPP

#include "build.hpp"
// .tpp
#include "global/event.hpp"

#include <functional>

namespace clover {
namespace gui {

template <typename ElementType>
using CallbackFunction = std::function<void (ElementType&)>;

class GuiMgr;

class BaseCallback {
public:
	virtual const Element& getOwner() const = 0;
	
protected:
	friend class GuiMgr;
	// Calls the actual callback-function
	virtual void call()=0;
};

template <typename T>
class Callback : public BaseCallback {
public:
	typedef T ElementType;
	
	Callback();
	Callback(Callback&& other)= default;
	Callback& operator=(Callback&& other)= default;
	
	void assign(const CallbackFunction<ElementType>& f);
	
	/// Notifies GuiMgr which fires callback after updating every element
	/// "element" must always be *this where called
	void trigger(ElementType& element);
	Callback& operator()(ElementType& element){ trigger(element); return *this; }
	
	virtual const ElementType& getOwner() const { ensure(owner); return *owner; }
private:
	
	virtual void call();
	
	ElementType* owner;
	CallbackFunction<ElementType> callback;
};

#include "callback.tpp"

} // gui
} // clover

#endif // CLOVER_GUI_CALLBACK_HPP