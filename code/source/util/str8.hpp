#ifndef CLOVER_UTIL_STR8_HPP
#define CLOVER_UTIL_STR8_HPP

#include "build.hpp"
#include "hash.hpp"
#include "dyn_array.hpp"
#include "util/traits.hpp"
#include "debug/debugprint.hpp"

#include <cstdarg>
#include <iostream>
#include <string>

namespace clover {
namespace util {

/// UTF-8 string
class ENGINE_API Str8 {
public:
	Str8();

	Str8(const std::string& str);
	Str8(const Str8&) = default;
	Str8(Str8&&) = default;

	Str8& operator=(const Str8&) = default;
	Str8& operator=(Str8&&) = default;

	// Bufin pitää päättyä nollaan
	Str8(const char8* buf);

	Str8& operator()(const char8* buf, ...);

	/// @todo this is a bit confusing, since Str8("fo") << 'o' doesn't do what you would expect
	Str8& operator<<(int32 c);

	Str8& operator=(const char8* buf);

	inline Str8& operator+=(const std::string& str) { return *this += str.c_str(); }
	Str8& operator+=(const char8* buf);
	Str8& operator+=(const Str8& str);

	inline Str8 operator+(const std::string& str) const { return *this + str.c_str(); }
	Str8 operator+(const char8* buf) const;
	Str8 operator+(const Str8& buf) const;

	Str8& operator+=(uint32 unicode);

	void setFormattedArgList(const char8* buf, va_list);

	const char8* cStr() const;

	/// Returns unicode character
	uint32 operator[](uint32 i) const;

	bool operator<(const Str8& s) const {
		return data < s.data;
	}

	bool operator==(const Str8& s) const {
		return data == s.data;
	}

	bool operator!=(const Str8& s) const {
		return data != s.data;
	}

	void resize(uint32 s, uint32 unicode=0);

	uint32 length() const {
		return strLength;
	}

	bool empty() const {
		return strLength == 0;
	}

	// String size in bytes (excluding null byte)
	std::size_t sizeBytes() const {
		return data.length();
	}

	int32 find(uint32 unicode, int32 pos = 0) const;
	int32 findLast(uint32 unicode) const;

	void clear() {
		data.clear();
		strLength = 0;
		ascii = true;
	}

	void erase(uint32 pos, uint32 count=1);

	/// @return New string with last char being endline erased
	Str8 endNewlineErased() const;

	void insert(uint32 pos, uint32 unicode, uint32 count=1);

	/// Returns true if the string contains other-string
	/// @param notice_capitals If false, all letters are transformed to same case before comparison
	bool contains(const Str8& other, bool notice_capitals=true);

	/// Returns an upper-case version of the string
	Str8 upperCased() const;

	/// Returns a lower-case version of the string
	Str8 lowerCased() const;

	util::DynArray<Str8> splitted(uint32 unicode_separator, SizeType count= 1) const;

	SizeType count(uint32 unicode) const;

	template <typename Archive>
	void serialize(Archive& ar, const uint32 version){
		ar & ascii;
		ar & data;
		ar & strLength;
	}

	/// Replaces all unicode chars 'from' with 'to'
	/// Returns the number of characters replaced
	int replace(uint32 from, uint32 to);

	/// Returns a new string that has "\r\n", "\n\r" and "\r" converted to "\n"
	Str8 toUnixNewlines() const;

	Str8 substr(std::size_t pos = 0, std::size_t count = std::string::npos) const;

	static constexpr uint32 backspaceChar= 8;

	static Str8 format(const char8* buf, ...) PRINTF_FORMAT(1, 2);

private:
	/// Does the string have only ascii chars
	bool ascii;

	/// UTF-8 data
	std::string data;

	/// Number of characters in the string
	std::size_t strLength;
};

inline Str8 operator+(const char8* buf, const Str8& s){
	Str8 t(buf);
	t += s;
	return t;
}

template<>
class Hash32<Str8> {
public:
	uint32 operator()(const Str8& str) const {
		return rawArrayHash32(str.cStr(), str.sizeBytes());
	}

};

template <>
struct TypeStringTraits<Str8> {
	static Str8 type(){ return "::Str8"; }
};

} // util
} // clover

#endif // CLOVER_UTIL_STR8_HPP