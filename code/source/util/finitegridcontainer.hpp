#ifndef CLOVER_UTIL_FINITEGRIDCONTAINER_HPP
#define CLOVER_UTIL_FINITEGRIDCONTAINER_HPP

#include "build.hpp"
#include "util/containeralgorithms.hpp"
#include "util/linkedlist.hpp"
#include "util/dyn_pool.hpp"
#include "util/vector.hpp"

namespace clover {
namespace util {

/// Contains finite sized elements in a grid
/// @warning Makes copies of elements
/// @warning Handles only unique elements (could be changed without interface changes)
/// @warning Not tested with elements deeper than 1 unit
template <typename T, SizeType N, typename CompT= int32>
class FiniteGridContainer {
public:
	using Vec= IntegerVector<CompT, N>;
	using Vec2= IntegerVector<CompT, 2>;

	struct Element {
		T value;
		Vec origin, size;
		bool contains(Vec v) const {
			return v.componentsGreaterOrEqualThan(origin) && v.componentsLessThan(origin + size);
		}
	};

	void add(T value, Vec origin, Vec size);
	void remove(const T& value, Vec pos);

	const Element& get(const T& value, Vec pos) const;

	util::DynArray<T> coneQuery(Vec origin, Vec2 size, real64 z_slope) const;

private:
	static bool overlaps(Vec2 o1, Vec2 s1, Vec2 o2, Vec2 s2);
	util::DynArray<Vec> getVecs(Vec origin, Vec size) const;

	DynPool<Element> elements;
	// For fast query
	Map<T, SizeType> valueToElementIndex;
};

template <typename T, SizeType N, typename CompT>
void FiniteGridContainer<T, N, CompT>::add(T value, Vec origin, Vec size){
	PROFILE();
	ensure(size.x > 0 && size.y > 0);
	valueToElementIndex[value]= elements.add(Element({value, origin, size}));
}

template <typename T, SizeType N, typename CompT>
void FiniteGridContainer<T, N, CompT>::remove(const T& value, Vec pos){
	PROFILE();
	auto it= valueToElementIndex.find(value);
	if (it != valueToElementIndex.end()){
		SizeType i= it->second;

		Element* e= elements.find(i);
		ensure(e && e->value == value && e->contains(pos));

		elements.remove(i);
		valueToElementIndex.erase(it);
		return;
	}

	ensure_msg(0, "Not found");
}

template <typename T, SizeType N, typename CompT>
auto FiniteGridContainer<T, N, CompT>::get(const T& value, Vec pos) const -> const Element& {
	auto it= valueToElementIndex.find(value);
	ensure(it != valueToElementIndex.end());
	return elements.get(it->second);
}

template <typename T, SizeType N, typename CompT>
util::DynArray<T> FiniteGridContainer<T, N, CompT>::coneQuery(
		Vec origin,
		Vec2 size,
		real64 z_slope) const {
	PROFILE();

	CompT max_height= (CompT)(-(size.x/2.0)/z_slope + 0.5);
	if (z_slope == 0.0)
		max_height= std::numeric_limits<CompT>::max();

	util::DynArray<T> found;
	for (SizeType i= 0; i < elements.size(); ++i){
		const Element* e= elements.find(i);
		if (!e)
			continue;
		// This probably doesn't work when e->size.z > 1
		Vec2 offset= Vec2(1)*((e->origin.z - origin.z)*z_slope);
		//offset += Vec2(1);
		if (overlaps(	e->origin.xy(), e->size.xy(),
						origin.xy() - offset, size.xy() + offset*2) &&
			e->origin.z <= max_height){
			found.pushBack(e->value);
		}
	}
	return found;
}

template <typename T, SizeType N, typename CompT>
bool FiniteGridContainer<T, N, CompT>::overlaps(Vec2 o1, Vec2 s1, Vec2 o2, Vec2 s2){
	return !(	o1.x >= o2.x + s2.x || o1.y >= o2.y + s2.y ||
				o2.x >= o1.x + s1.x || o2.y >= o1.y + s1.y);
}

template <typename T, SizeType N, typename CompT>
auto FiniteGridContainer<T, N, CompT>::getVecs(Vec origin, Vec size) const -> util::DynArray<Vec> {
	ensure(size.componentsGreaterThan(Vec(0)));

	SizeType count= 1;
	for (SizeType n= 0; n < N; ++n)
		count *= size[n];

	util::DynArray<Vec> vecs;
	vecs.resize(count);

	Vec current= origin;
	for (SizeType i= 0; i < count; ++i){
		vecs[i]= current;

		// util::Set current to next position inside cube
		for (SizeType n= 0; n < N; ++n){
			++current[n];
			if (current[n] < origin[n] + size[n])
				break;
			else
				current[n]= origin[n];
		}
	}
	return vecs;
}

} // util
} // clover

#endif // CLOVER_UTIL_FINITEGRIDCONTAINER_HPP