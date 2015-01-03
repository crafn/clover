#ifndef CLOVER_APP_HPP
#define CLOVER_APP_HPP

#include "build.hpp"
#include "global/env.hpp"
#include "util/string.hpp"

namespace clover {

class App {
public:
	App(const util::Str8& executablePath);
	~App();
	void run();

private:
	global::Env env;
};

} // clover

#endif // CLOVER_APP_HPP
