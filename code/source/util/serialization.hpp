#ifndef CLOVER_UTIL_SERIALIZATION_HPP
#define CLOVER_UTIL_SERIALIZATION_HPP

#include "build.hpp"

namespace clover {
namespace util {

/// Defines field layout and additional steps needed for serialization
/// @note Binary serialization doesn't save field names, so field order
///		  in serialization and deserialization should match
/// @todo Should be used instead of ObjectNodeTraits
/// @todo Version numbers
template <typename T>
struct SerializationTraits;
#if 0
{
	/// Example

	/// Handles serialization and deserialization
	template <typename Archive>
	static void convert(Archive& ar, T& obj){
		ar.field("name1", obj.field1);
		ar.field("name2", obj.field2);
	}

	/// Separate functions if serialization or deserialization
	/// needs to process some data.

	template <typename Archive>
	static void serialize(Archive& ar, const T& obj){
		ar.write("name1", obj.field1);

		// Omit field2 when it is empty
		ar.write("name2", obj.field2, !obj.field2.empty());
	}

	template <typename Archive>
	static T deserialize(Archive& ar){
		int32 field1;
		DynArray<int32> field2;

		ar.readTo("name1", field1);
		// Construct empty DynArray if field2 was omitted
		ar.readTo("name2", field2, {});

		return T{field1, field2};
	}
};
#endif // 0

} // util
} // clover

#endif // CLOVER_UTIL_SERIALIZATION_HPP