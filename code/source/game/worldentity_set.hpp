#ifndef CLOVER_GAME_WORLDENTITY_SET_HPP
#define CLOVER_GAME_WORLDENTITY_SET_HPP

#include "build.hpp"
#include "worldentity_handle.hpp"
#include "util/linkedlist.hpp"

namespace clover {
namespace game {

/// @todo Replace with a bunch of functions for util::Set/util::DynArray
class WESet {
public:
    WESet();
    virtual ~WESet();

    typedef util::LinkedList<game::WeHandle>::Iter Iter;
    typedef util::LinkedList<game::WeHandle>::cIter cIter;

    Iter begin();
    Iter end();

    cIter begin() const;
    cIter end() const;

    int32 size();
    void clear();

    void add(const game::WorldEntity&);
    void add(const game::WeHandle&);
    void add(const WESet&);

	void remove(const game::WeHandle&);

	WESet& operator+=(const game::WorldEntity& w){ add(w); return *this; }
	WESet& operator+=(const game::WeHandle& w){ add(w); return *this; }
	WESet& operator+=(const WESet& w){ add(w); return *this; }

private:
    util::LinkedList<game::WeHandle> handles;
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDENTITY_SET_HPP
