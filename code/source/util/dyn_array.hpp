#ifndef CLOVER_UTIL_DYN_ARRAY_HPP
#define CLOVER_UTIL_DYN_ARRAY_HPP

#include "build.hpp"
#include "util/ensure.hpp"
#include "util/hash.hpp"

namespace clover {
namespace util {

/// Substituting std::vector with `DynArray` was a good choice, because
/// - std::vector<bool> is broken, needed a workaround
/// - replacing std::vector with this reduced compile time by 40s (j8)
template <typename T, template <typename> class Ator= std::allocator>
class DynArray {
public:
	using Value= T;

	using AtorT= Ator<T>;
	using This= DynArray<T, Ator>;
	using Iter= T*;
	/// @todo Rename to match naming conventions
	using cIter= const T*;

	DynArray()= default;
	~DynArray()
	{
		ensure(capacity_ >= size_);
		for (auto& m : *this)
			m.~Value();
		ator.deallocate(data_, capacity_);
	}

	DynArray(SizeType size)
	{ resize(size); }

	DynArray(std::initializer_list<T> l)
	{ reserve(l.size()); for (auto& m : l) pushBack(m); }

	DynArray(const AtorT& ator)
		: ator(ator) { }

	DynArray(const DynArray& other)
	{ operator=(other); }
	DynArray(DynArray&& other)
	{ operator=(std::move(other)); }

	DynArray& operator=(const DynArray& t)
	{
		if (this != &t) {
			clear(); /// @todo Don't clear
			reserve(t.size_);
			for (SizeType i= 0; i < t.size_; ++i)
				new (data_ + i) Value(t.data_[i]);
			size_= t.size_;
		}
		return *this;
	}

	DynArray& operator=(DynArray&& t)
	{
		if (this != &t) {
			clear(); /// @todo Don't clear
			data_= t.data_;
			size_= t.size_;
			capacity_= t.capacity_;

			t.data_= nullptr;
			t.size_= 0;
			t.capacity_= 0;
		}
		return *this;
	}

	Value& operator[](SizeType i){ return data_[i]; }
	const Value& operator[](SizeType i) const { return data_[i]; }

	Value& at(SizeType i)
	{ ensure(i < size_); return data_[i]; }
	const Value& at(SizeType i) const
	{ ensure(i < size_); return data_[i]; }

private:
	void enlarge()
	{
		SizeType new_capacity= 0;
		if (capacity_ == 0)
			new_capacity= 16;
		else
			new_capacity= capacity_*2;

		ensure(new_capacity >= size_ + 1);
		T* new_data= ator.allocate(new_capacity);
		for (SizeType i= 0; i < size_; ++i)
			new (new_data + i) Value(std::move(data_[i]));

		for (auto& m : *this)
			m.~Value();
		ator.deallocate(data_, capacity_);

		data_= new_data;
		capacity_= new_capacity;
	}
public:

	void pushBack(const Value& t)
	{
		if (size_ == capacity_)
			enlarge();
		new (data_ + size_) Value(t);
		++size_;
	}

	void pushBack(Value&& t)
	{
		if (size_ == capacity_)
			enlarge();
		new (data_ + size_) Value(std::forward<T>(t));
		++size_;
	}

	void pushBack(const DynArray& v)
	{ for (auto& m : v) pushBack(m); }

	template<typename... Args>
	void emplaceBack(Args&&... args)
	{
		if (size_ == capacity_)
			enlarge();
		new (data_ + size_) Value(std::forward<Args>(args)...);
		++size_;
	}

	SizeType size() const { return size_; }
	bool empty() const { return size_ == 0; }
	void clear()
	{
		// Following std::vector, although separate destruct() and
		// clear() would be better
		for (auto& m : *this)
			m.~Value();
		size_= 0;
	}

private:
	void resizeCapacity(SizeType new_capacity)
	{
		if (capacity_ == new_capacity)
			return;

		Value* new_data= ator.allocate(new_capacity);
		for (SizeType i= 0; i < new_capacity && i < size_; ++i)
			new (new_data + i) Value(std::move(data_[i])); // Copy objects to new buffer

		for (auto& m : *this)
			m.~Value(); // Destroy old, including those who didn't fit
		ator.deallocate(data_, capacity_);

		data_= new_data;
		capacity_= new_capacity;
	}

public:

	void resize(SizeType new_size)
	{ 
		if (new_size == size_)
			return;
		resizeCapacity(new_size);
		if (new_size > size_) {
			for (SizeType i= size_; i < new_size; ++i)
				new (data_ + i) Value();
		}
		size_= new_size;
	}
	void resize(SizeType new_size, const Value& value)
	{
		if (new_size == size_)
			return;
		resizeCapacity(new_size);
		if (new_size > size_) {
			for (SizeType i= size_; i < new_size; ++i)
				new (data_ + i) Value(value);
		}
		size_= new_size;
	}

	void reserve(SizeType new_capacity)
	{
		if (new_capacity <= size_)
			return;

		resizeCapacity(new_capacity);
	}

	Iter begin() { return data_; }
	Iter end() { return data_ + size_; }
	cIter begin() const { return data_; }
	cIter end() const { return data_ + size_; }

	T* data() { return data_; }
	const T* data() const { return data_; }

	T& front() { return *data_; }
	T& back() { return *(data_ + size_ - 1); }
	const T& front() const { return *data_; }
	const T& back() const { return *(data_ + size_ - 1); }

	Iter insert(Iter it, const T& t)
	{
		if (empty() || it == end()) {
			pushBack(t);
			return end() - 1;
		}

		SizeType i_to_inserted= it - data_;
		if (size_ == capacity_)
			enlarge(); // This is inefficient (but simple)

		it= data_ + i_to_inserted;
		new (data_ + size_) Value(std::move(data_[size_ - 1]));
		for (SizeType i= size_ - 1; i > i_to_inserted; --i)
			data_[i]= std::move(data_[i - 1]);

		it->~Value();
		new (it) Value(t);
		++size_;
		return data_ + i_to_inserted;
	}

	Iter erase(Iter it) { return erase(it, it + 1); }
	Iter erase(Iter b, Iter e)
	{
		if (b == e)
			return b;
		SizeType gap= e - b;
		for (auto it= b; it + gap != end(); ++it)
			*it= std::move(*(it + gap));
		for (auto it= end() - gap; it != end(); ++it)
			it->~Value();
		size_ -= gap;
		return b;
	}

	void popBack() { erase(end() - 1); }
	void popFront() { erase(begin()); }

	/// @todo Remove these, can be implemented outside
	SizeType count(const T& t) const
	{
		SizeType a= 0;
		for (SizeType i= 0; i < size_; ++i)
			if(data_[i] == t)
				++a;
		return a;
	}
	cIter find(const T& t) const
	{
		for (cIter it= begin(); it != end(); ++it)
			if (*it == t) return it;
		return end();
	}
	Iter find(const T& t)
	{
		for (Iter it= begin(); it != end(); ++it)
			if (*it == t) return it;
		return end();
	}
	void remove(const T& t)
	{
		auto it= find(t);
		debug_ensure(it != end());
		erase(it);
	}

	DynArray<T> appended(const This& v) const
	{ DynArray<T> ret(*this); ret.pushBack(v); return ret; }
	void append(const This& other)
	{ *this= appended(other); }

	template <typename Archive>
	void serialize(Archive& ar, const uint32 version)
	{
		if (Archive::is_saving::value) {
			ar & size_;
			for (auto& m : *this)
				ar & m;
		} else {
			SizeType s;
			ar & s;
			resize(s);
			for (auto& m : *this)
				ar & m;
		}
	}

private:
	Value* data_= nullptr;
	SizeType size_= 0;
	SizeType capacity_= 0;
	AtorT ator;
};



template<typename T, template <typename> class Ator>
struct Hash32<DynArray<T, Ator>> {
	uint32 operator()(const DynArray<T, Ator>& arr) const {
		if (arr.empty()) return 0;
		return rawArrayHash(arr.data(), arr.size());
	}
};

template <typename T, template <typename> class Ator>
void fastInsert(DynArray<T, Ator>& container, T value){
	container.pushBack(std::move(value));
}

} // util
} // clover
#endif // CLOVER_UTIL_DYN_ARRAY_HPP
