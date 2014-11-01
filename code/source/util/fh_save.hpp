#ifndef CLOVER_UTIL_FH_SAVE_HPP
#define CLOVER_UTIL_FH_SAVE_HPP

#include "build.hpp"
#include "fieldholder.hpp"

namespace clover {
namespace util {

/// @todo Get rid of this abomination
class SaveFieldHolder : public FieldHolder {
public:
	// Serialize
	const SaveFieldHolder& operator>>(util::RawPack& pack) const;
	
	// Deserialize
	SaveFieldHolder& operator<<(util::RawPack& pack);
	
protected:
};

} // util
} // clover

#endif // CLOVER_UTIL_FH_SAVE_HPP