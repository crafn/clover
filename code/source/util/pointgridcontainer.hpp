#ifndef CLOVER_UTIL_POINTGRIDCONTAINER_HPP_HPP
#define CLOVER_UTIL_POINTGRIDCONTAINER_HPP_HPP

#include "build.hpp"
#include "util/map.hpp"

namespace clover {
namespace util {

/// Container which stores stuff of type T to a grid of dimension N
template <typename T, SizeType N, typename CompT= int32>
class PointGridContainer {
public:
	using Vec= IntegerVector<CompT, N>;
	using util::Vec2= IntegerVector<CompT, 2>;

	void add(T value, Vec pos);
	void remove(const T& value, Vec pos);
	void remove(const T& value, const util::DynArray<Vec>& pos);

	const util::DynArray<T>& get(Vec pos) const { return cells.at(pos); }
	util::DynArray<T> query(util::DynArray<Vec> vecs) const;

	/// Finds elements in a cone with axis of (0, 0, 1)
	util::DynArray<T> coneQuery(Vec origin, util::Vec2 size) const;

private:
	using Cell= util::DynArray<T>;

	Map<Vec, Cell> cells;
};

template <typename T, SizeType N, typename CompT>
void PointGridContainer<T, N, CompT>::add(T value, Vec pos){
	cells[pos].pushBack(value);
}

template <typename T, SizeType N, typename CompT>
void PointGridContainer<T, N, CompT>::remove(const T& value, Vec pos){
	auto it= cells.find(pos);
	ensure (it != cells.end());

	it->second.remove(value);
	if (it->second.empty())
		cells.erase(it);
}

template <typename T, SizeType N, typename CompT>
void PointGridContainer<T, N, CompT>::remove(const T& value, const util::DynArray<Vec>& pos){
	for (auto p : pos){
		remove(value, p);
	}
}

template <typename T, SizeType N, typename CompT>
util::DynArray<T> PointGridContainer<T, N, CompT>::query(util::DynArray<Vec> vecs) const {
	util::DynArray<T> ret;
	for (Vec v : vecs){
		ret.append(cells.at(v));
	}
	return ret;
}

template <typename T, SizeType N, typename CompT>
util::DynArray<T> PointGridContainer<T, N, CompT>::coneQuery(Vec origin, util::Vec2 size) const {
	util::DynArray<T> found;
	for (const auto& pair : cells){
		Vec pos= pair.first;
		const Cell& cell= pair.second;
		
		/// @todo Changing size by z	
		if (pos.xy().componentsGreaterOrEqualThan(origin.xy()) &&
			pos.xy().componentsLessThan(origin.xy() + size.xy())){

			found.append(cell);
		}
	}
	return found;
}

} // util
} // clover

#endif // CLOVER_UTIL_POINTGRIDCONTAINER_HPP_HPP