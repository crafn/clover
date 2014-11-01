#include "file_stream.hpp"
#include "file.hpp"
#include "util/objectnode.hpp"

namespace clover {
namespace global {

class FileStream::Buffer : public std::streambuf {
public:
	Buffer(File& file);

private:
	virtual int_type underflow() override;

	File& file;
	util::DynArray<char> data;
};

FileStream::FileStream(File& file)
	: std::istream(new Buffer(file)),
	  buffer(static_cast<Buffer*>(rdbuf())) {}

FileStream::~FileStream() {}

FileStream::Buffer::Buffer(File& file) : file(file) {}

FileStream::Buffer::int_type FileStream::Buffer::underflow() {
	const std::size_t totalBufferSize = 1024;
	const std::size_t putBackSize = 16;

	if (gptr() < egptr())
		return traits_type::to_int_type(*gptr());

	char* start;
	if (data.empty()) {
		data.resize(totalBufferSize);
		start = data.data();
	} else {
		std::memmove(data.data(), egptr() - putBackSize, putBackSize);
		start = data.data() + putBackSize;
	}
	char* base = data.data();

	std::size_t n = file.readSome(start, data.size() - (start - base));
	if (n == 0)
		return traits_type::eof();

	setg(base, start, start + n);

	return traits_type::to_int_type(*gptr());
}

} // global
} // clover