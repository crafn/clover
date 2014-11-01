#ifndef CLOVER_GLOBAL_FILE_STREAM_HPP
#define CLOVER_GLOBAL_FILE_STREAM_HPP

#include "build.hpp"
#include "util/dyn_array.hpp"

#include <istream>
#include <memory>

namespace clover {
namespace global {

class File;

/// std::istream API for File
/// @todo also implement ostream
class FileStream : public std::istream {
public:
	FileStream(File& file);
	~FileStream();

private:
	class Buffer;
	std::unique_ptr<Buffer> buffer;
};

} // global
} // clover

#endif // CLOVER_GLOBAL_FILE_STREAM_HPP
