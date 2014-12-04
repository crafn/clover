#ifndef CLOVER_GAME_PROPERTYGRID_HPP
#define CLOVER_GAME_PROPERTYGRID_HPP

#include "build.hpp"
#include "util/map.hpp"
#include "util/properties.hpp"
#include "util/traits.hpp"
#include "util/vector.hpp"

namespace clover {
namespace physics {

class Grid;

} // physics
namespace game {

/// Grid used for spatial information exchange
class PropertyGrid {
public:
	using CellVec= util::Vec2i;

	PropertyGrid(physics::Grid& g)
		: physicsGrid(&g) { }

	template <typename T>
	void set(CellVec pos, const util::Str8& key, T&& value)
	{ cells[pos].set(key, std::forward<T>(value)); }

	template <typename T>
	T* find(CellVec pos, const util::Str8& key) const
	{
		auto it= cells.find(pos);
		if (it == cells.end())
			return nullptr;
		return it->second.find<T>(key);
	}

	void update();

	physics::Grid& getPhysicsGrid() const
	{ return *NONULL(physicsGrid); }

private:
	using Cell= util::Properties;

	util::Map<CellVec, Cell> cells;
	physics::Grid* physicsGrid;
};

template <typename T>
T tryGet(	const PropertyGrid& props,
			PropertyGrid::CellVec pos,
			const util::Str8& key,
			const T& not_found){
	auto found= props.find<T>(pos, key);
	return found ? found : not_found;
}

} // game
namespace util {

template <>
struct TypeStringTraits<game::PropertyGrid>{
	static util::Str8 type(){ return "game::PropertyGrid"; }
};

} // util
} // clover

#endif // CLOVER_GAME_PROPERTYGRID_HPP
