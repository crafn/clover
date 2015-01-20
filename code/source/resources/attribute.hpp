#ifndef CLOVER_RESOURCES_ATTRIBUTE_HPP
#define CLOVER_RESOURCES_ATTRIBUTE_HPP

#include "build.hpp"
#include "attributetype.hpp"
#include "attributeinitializer.hpp"
#include "attribute_def.hpp"
#include "baseattribute.hpp"
#include "debug/print.hpp"

namespace clover {
namespace resources {

template <typename T> // AttributeType
class Attribute : public BaseAttribute {
public:
	typedef T AttributeType;
	typedef typename AttributeDefImpl<T>::Value Value;
	typedef BaseAttribute Base;
	
	explicit Attribute(const Value& value);
	explicit Attribute(const AttributeInitializer<AttributeType>& init);
	Attribute(const Attribute&);
	Attribute(Attribute&&);
	virtual ~Attribute();
	
	
	Attribute& operator=(const Attribute&);
	Attribute& operator=(Attribute&&);
	
	virtual const RuntimeAttributeType getType() const { return T::runtimeType(); }
	virtual const util::Str8& getKey() const;
	virtual util::ObjectNode serialized() const;
	
	using BaseAttribute::set;
	
	/// Sets value and calls change callbacks
	void set(const Value&);
	
	/// Returns value
	const Value& get() const;
	Value& get();
	
	/// Resets to default value
	void reset();
	
	using Base::OnChangeCallback;
	
	/// Called when set(..) is called, NOT called if attribute is modified through a reference from non-const get()
	void setOnChangeCallback(const OnChangeCallback& f);
	
private:
	/// The key and container are needed in destruction
	/// Also, attributes can be reseted when the default value is saved
	AttributeInitializer<AttributeType> initializer;
	
	Value value;
	
	// Could be moved to BaseAttribute
	OnChangeCallback OnChange;
	
	bool addedToCont;
	
	/// True when attribute isn't in resource which is in some cache -> there's no container in initializer
	/// Resources in cache are never copied or moved, so copy-constructor and move-constructor set dummy to true
	//bool dummy; // use initializer.getContainer() == 0 instead
};

// A way to hide things into .cpp so Resource's methods can be called
void tryLaunchAttributeOnChangeCallbacks(const BaseAttribute::OnChangeCallback& cb, Resource* res);

#include "attribute.tpp"

} // resources
} // clover

#endif // CLOVER_RESOURCES_ATTRIBUTE_HPP
