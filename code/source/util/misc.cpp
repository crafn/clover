#include "misc.hpp"
#include "build.hpp"
#include "string.hpp"
#include "ensure.hpp"

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <stdio.h>
#include <cstdarg>
#include <cassert>
#include <sstream>

namespace clover {
namespace util {

std::string getFilenameFromPath(const char8* s){
	/// @todo Maybe use boost::filesystem?
	uint32 first=0;
	uint32 i=0;
	while(s[i] != 0){
		if (s[i] == '\\' ||s [i] == '/'){
			first= i+1;
		}
		i++;
	}

	util::Str8 ret;
	
	i=0;
	while(s[i] != 0){
		ret += s[i];
		i++;
	}

	return std::string(ret.cStr());
}

std::string getReducedFunctionName(const char8* ss){
	util::Str8 ret= ss;

	int32 pos= ret.find('(');
	if (pos < 0) return std::string(ret.cStr());

	pos += 1;

	int brackets= 1;
	int32 removed=0;
	while((int32)ret.length() > pos && (ret[pos] != ')' || brackets > 1)){
		removed ++;
		if (ret[pos] == ')') --brackets;
		if (ret[pos] == '(') ++brackets;
		ret.erase(pos, 1);
	}

	if (removed > 1){
		ret.insert(pos, '.', 2);
	}

	return std::string(ret.cStr());
}

} // util
} // clover