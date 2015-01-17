#include "str8.hpp"
#include "ensure.hpp"
#include "debug/print.hpp"
#include "util/dyn_array.hpp"

#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <algorithm>

#include <utf8.h>

namespace clover {
namespace util {
namespace {
	/// @see http://en.wikipedia.org/wiki/UTF-8#Description
	/// This uses RFC 3629, max 4-byte sequences
	/// @param unsafeStr Null-terminated string, hopefully in UTF-8 encoding
	std::size_t toUtf8(const char* unsafeStr, std::string& output){
		const unsigned char* it = reinterpret_cast<const unsigned char*>(unsafeStr);
		std::size_t charCount = 0;
		while (*it) {
			if (*it < 0x80)	{
				output += *it;
				++it;
			} else if (((it[0] & 0xe0) == 0xc0) && ((it[1] & 0xc0) == 0x80)) {
				output.append(it, it+2);
				it += 2;
			} else if (((it[0] & 0xf0) == 0xe0) && ((it[1] & 0xc0) == 0x80) && ((it[2] & 0xc0) == 0x80)) {
				output.append(it, it+3);
				it += 3;
			} else if (((it[0] & 0xf8) == 0xf0) && ((it[1] & 0xc0) == 0x80) &&
					   ((it[2] & 0xc0) == 0x80) && ((it[3] & 0xc0) == 0x80)) {
				output.append(it, it+4);
				it += 4;
			} else {
				// This isn't valid UTF-8 character, treat it as Windows-1252 or ~Latin1/ISO-8859-15
				// These rules were built with ruby, running something similar to:
				// puts ((128..255).to_a-[129,141,143,144,157]).map{|c| "#{c.to_s(16)}: "+c.chr.encode("UTF-8", "windows-1252").bytes.to_a.inspect}.join("\n")

				if (*it < 0xa0) {
					static const char * table[] = {
						"\xe2\x82\xac",
						"",
						"\xe2\x80\x9a",
						"\xc6\x92",
						"\xe2\x80\x9e",
						"\xe2\x80\xa6",
						"\xe2\x80\xa0",
						"\xe2\x80\xa1",
						"\xcb\x86",
						"\xe2\x80\xb0",
						"\xc5\xa0",
						"\xe2\x80\xb9",
						"\xc5\x92",
						"",
						"\xc5\xbd",
						"",
						"",
						"\xe2\x80\x98",
						"\xe2\x80\x99",
						"\xe2\x80\x9c",
						"\xe2\x80\x9d",
						"\xe2\x80\xa2",
						"\xe2\x80\x93",
						"\xe2\x80\x94",
						"\xcb\x9c",
						"\xe2\x84\xa2",
						"\xc5\xa1",
						"\xe2\x80\xba",
						"\xc5\x93",
						""
						"\xc5\xbe",
						"\xc5\xb8"
					};

					debug_ensure(*it-0x80 >= 0);

					const char* conv = table[*it-0x80];
					int len = strlen(conv);

					/// According to http://en.wikipedia.org/wiki/Windows-1252#Codepage_layout
					/// empty string in the table are illegal chars. We will just remove these from input
					if (len == 0)
						continue;

					output.append(conv);
					it += len;
				} else if (*it < 0xc0) {
					output += 0xc2;
					output += *it;
				} else {
					output += 0xc3;
					output += *it - 0x40;
				}
				++it;
			}
			++charCount;
		}
		return charCount;
	}

	std::size_t encodeCodePoint(uint32 maybeUnicode, char u[4]){
		/// @todo remove utf8.h dependency and code this faster, also use Windows-1252 conversion like toUtf8
		try {
			char* end = utf8::append(maybeUnicode, u);
			return end - u;
		} catch (utf8::invalid_code_point&) {
			return 0;
		}
	}
} // anonymous


Str8::Str8() : ascii(true), strLength(0){
}

Str8::Str8(const std::string& str){
	strLength = toUtf8(str.c_str(), data);
	ascii = strLength == data.size();
}


Str8::Str8(const char8 *buf){
	strLength = toUtf8(buf, data);
	ascii = strLength == data.size();
}

Str8& Str8::operator()(const char8* buf, ...){
	va_list arg_list;
	va_start(arg_list, buf);

	setFormattedArgList(buf, arg_list);

	va_end(arg_list);

	return *this;
}

Str8& Str8::operator<<(int32 c){
	char8 buf[11];
	sprintf(buf, "%d", c);

	data += buf;
	strLength += strlen(buf);

	return *this;
}

Str8& Str8::operator=(const char8* buf){
	data.clear();
	strLength = toUtf8(buf, data);
	ascii = strLength == data.size();
	return *this;
}

Str8& Str8::operator+=(const char8* buf){
	strLength += toUtf8(buf, data);
	ascii = strLength == data.size();
	return *this;
}

Str8& Str8::operator+=(const Str8& str){
	data += str.data;
	strLength += str.strLength;
	ascii = ascii && str.ascii;
	return *this;
}

Str8 Str8::operator+(const char8* buf) const{
	Str8 str= *this;
	str += buf;
	return str;

}

Str8 Str8::operator+(const Str8& str) const{
	Str8 cpy(*this);
	cpy += str;
	return cpy;
}

Str8& Str8::operator+=(uint32 unicode){
	char u[4];
	const std::size_t charSize = encodeCodePoint(unicode, u);
	++strLength;
	data.append(u, u + charSize);
	ascii = strLength == data.size();
	return *this;
}

void Str8::setFormattedArgList(const char8* buf, va_list arg_list){
	ensure(buf);

	if (*buf == '\0'){
		clear();
		return;
	}

	const uint32 format_buf_size= 10000;
	// Can't be static because accessed from threads
	char8 format_buf[format_buf_size];

	vsnprintf(format_buf, format_buf_size, buf, arg_list);

	*this = format_buf;
}

const char* Str8::cStr() const{
	return data.c_str();
}

uint32 Str8::operator[](uint32 i) const {
	ensure(i < strLength && i >= 0);
	if (ascii)
		return data[i];

	auto it = data.begin();
	utf8::unchecked::advance(it, i);
	return utf8::unchecked::peek_next(it);
}

void Str8::resize(uint32 s, uint32 unicode){
	if (strLength > s) {
		if (ascii) {
			data.resize(s);
			strLength = s;
		} else {
			auto it = data.begin();
			utf8::unchecked::advance(it, s);
			data.resize(it - data.begin());
			strLength = s;
			ascii = strLength == data.size();
		}
	} else if (strLength < s) {
		char u[4];
		const std::size_t charSize = encodeCodePoint(unicode, u);
		if (charSize == 0) return;

		const int newChars = s - strLength;
		const std::size_t b = data.size();

		data.resize(data.size() + newChars * charSize);
		for (std::size_t i = 0, e = newChars * charSize; i < e; ++i)
			data[b+i] = u[i % charSize];

		strLength = s;
		ascii = strLength == data.size();
	}
}

int32 Str8::find(uint32 unicode, int32 pos) const{
	if (unicode < 0x80 && ascii)
		return data.find(unicode, pos);

	auto it = data.begin(), end = data.end();

	if (pos >= int32(strLength))
		return -1;

	utf8::unchecked::advance(it, pos);

	for (; it < end; ++pos) {
		if (utf8::unchecked::next(it) == unicode)
			return pos;
	}

	return -1;
}

int32 Str8::findLast(uint32 unicode) const {
	if (unicode < 0x80 && ascii)
		return data.find_last_of(unicode);

	auto it = data.begin(), end = data.end();

	int32 match = -1;
	for (int pos = 0; it < end; ++pos) {
		if (utf8::unchecked::next(it) == unicode)
			match = pos;
	}

	return match;
}

void Str8::erase(uint32 pos, uint32 count){
	ensure(pos < strLength);
	ensure_msg(pos + count <= strLength, "Pos: %i, count: %i, strLength: %i", pos, count, strLength);
	ensure(count > 0);

	if (ascii) {
		data.erase(pos, count);
		strLength = data.size();
		return;
	}

	auto it = data.begin();
	utf8::unchecked::advance(it, pos);

	auto end = it;
	utf8::unchecked::advance(end, count);

	data.erase(it, end);
	strLength -= count;
	ascii = strLength == data.size();
}

Str8 Str8::endNewlineErased() const {
	Str8 ret= *this;

	if (!ret.empty() && ret[ret.length() - 1] == '\n'){
		ret.erase(ret.length() - 1, 1); // Erase newline at the end
	}

	return (ret);
}

void Str8::insert(uint32 pos, uint32 unicode, uint32 count){
	ensure(pos < strLength);
	ensure(count > 0);

	auto it = data.begin();
	if (ascii)
		it += count;
	else
		utf8::unchecked::advance(it, pos);

	char u[4];
	const std::size_t charSize = encodeCodePoint(unicode, u);
	if (charSize == 0) return;

	auto offset = it - data.begin();
	data.insert(it, count*charSize, u[0]);

	if (charSize > 1)
		for (uint32 i = offset, j = 0; i < offset + count*charSize; ++i, ++j)
			data[i] = u[j % charSize];

	strLength += count;
	ascii = strLength == data.size();
}

bool Str8::contains(const Str8& other, bool notice_capitals){
	if (other.length() > length()) return false;

	if (!notice_capitals){
		return lowerCased().contains(other.lowerCased());
	}

	uint32 match= 0;

	for(uint32 i=0; length() - i >= other.length() - match &&
					i < length() &&
					match < other.length(); ++i){

		if ((*this)[i] == other[match]){
			++match;
		}
		else {
			uint32 prev= match;
			match= 0;

			if (prev > 0){
				ensure(i >= match);
				// Go backwards
				i -= prev;
				continue;
			}
		}

	}

	return match == other.length();
}

Str8 Str8::upperCased() const {
	Str8 ret= *this;
	std::transform(ret.data.begin(), ret.data.end(), ret.data.begin(), ::toupper);
	return (ret);
}

Str8 Str8::lowerCased() const {
	Str8 ret= *this;
	std::transform(ret.data.begin(), ret.data.end(), ret.data.begin(), ::tolower);
	return (ret);
}

util::DynArray<Str8> Str8::splitted(uint32 unicode_separator, SizeType count) const {
	/// @todo Change to work really with unicodes
	util::DynArray<Str8> ret;

	bool last_was_separator= false;
	Str8 accum, comp_accum;
	for (auto it= data.begin(); it != data.end(); ++it){
		last_was_separator= false;

		if ((uint32)*it == unicode_separator){
			comp_accum += *it;
		}
		else {
			accum += comp_accum;
			accum += *it;
			comp_accum.clear();
		}

		if (comp_accum.length() == count){
			comp_accum.clear();
			ret.pushBack(accum);
			accum.clear();
			last_was_separator= true;
		}
	}

	if (!accum.empty())
		ret.pushBack(accum);

	if (last_was_separator)
		ret.pushBack(Str8());

	return (ret);
}

SizeType Str8::count(uint32 unicode) const {
	if (ascii) {
		if (unicode >= 0x80)
			return 0;
		char asciiChar = (char)unicode;
		return std::count(data.begin(), data.end(), asciiChar);
	}

	SizeType count= 0;
	for (auto it = data.begin(), end = data.end(); it != end;)
		if (utf8::unchecked::next(it) == unicode)
			++count;
	return count;
}

int Str8::replace(uint32 from, uint32 to) {
	int replaces = 0;
	// ascii replace is easy to do without reallocating
	if (from < 0x80 && to < 0x80) {
		if (ascii) {
			// std::replace doesn't return the number of replaces
			for (auto it = data.begin(), end = data.end(); it != end; ++it) {
				if (*it == char(from)) {
					*it = to;
					++replaces;
				}
			}
		} else {
			for (auto it = data.begin(), end = data.end(); it != end;) {
				if (*it == char(from)) {
					*it++ = to;
					++replaces;
				} else {
					utf8::unchecked::next(it);
				}
			}
		}
	} else {
		Str8 tmp;
		for (auto it = data.begin(), end = data.end(); it != end; ) {
			uint32 c = utf8::unchecked::next(it);
			if (c == from) {
				tmp += to;
				++replaces;
			} else {
				tmp += c;
			}
		}
		*this = tmp;
	}
	return replaces;
}

Str8 Str8::toUnixNewlines() const {
	Str8 out;
	out.data.reserve(data.size());

	char prev = '\0';
	for (auto it = data.begin(), end = data.end(); it != end;) {
		uint32 c = utf8::unchecked::next(it);
		if (c == '\r') {
			if (prev == '\n') {
				prev = '\0';
				continue;
			}
			out += '\n';
			prev = c;
		} else if (c == '\n') {
			if (prev == '\r') {
				prev = '\0';
				continue;
			}
			out += '\n';
			prev = c;
		} else {
			out += c;
			prev = '\0';
		}
	}

	return out;
}

Str8 Str8::substr(std::size_t pos, std::size_t count) const {
	if (ascii) {
		Str8 tmp;
		tmp.ascii = true;
		tmp.data = data.substr(pos, count);
		tmp.strLength = tmp.data.length();
		return tmp;
	}

	if (pos > strLength)
		throw std::out_of_range("Str8 substr pos > strLength");

	if (pos == strLength)
		return Str8();

	// if count is npos, it will overflow with pos + count
	if (count >= strLength || pos + count >= strLength)
		count = strLength - pos;

	auto it = data.begin();
	utf8::unchecked::advance(it, pos);
	auto end = it;
	utf8::unchecked::advance(end, count);

	Str8 tmp;
	tmp.data = std::string(it, end);
	tmp.strLength = count;
	tmp.ascii = count == tmp.data.length();
	return tmp;
}

Str8 Str8::format(const char8* buf, ...){
	Str8 tmp;
	va_list arg_list;
	va_start(arg_list, buf);

	tmp.setFormattedArgList(buf, arg_list);

	va_end(arg_list);

	return tmp;
}

} // util
} // clover
