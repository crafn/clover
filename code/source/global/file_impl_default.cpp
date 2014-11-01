#include "file_impl_default.hpp"
#include "file.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace clover {
namespace global {

DefaultFileImpl::DefaultFileImpl(const util::Str8& fullPath, bool readOnly, uint64 size, time_t mtime)
	: FileImpl(fullPath, readOnly), activeFlags(0), file(nullptr), size(size), mtime(mtime) {
}

DefaultFileImpl::~DefaultFileImpl() {
	if (file)
		fclose(file);
}

void DefaultFileImpl::remove() {
	if (unlink(getName().cStr()) != 0)
		throw File::Error("Failed to remove file", getName(), strerror(errno));
}

uint64 DefaultFileImpl::getSize() const {
	return size;
}

std::size_t DefaultFileImpl::readSome(void* buffer, std::size_t bufferSize) {
	this->open(Flag_Read);
	std::size_t r = fread(buffer, 1, bufferSize, file);
	if (r == 0 && ferror(file))
		throw File::Error("Failed to read from file", getName(), strerror(errno));
	return r;
}

std::size_t DefaultFileImpl::writeSome(const void* buffer, std::size_t bufferSize) {
	this->open(Flag_Write);
	std::size_t r = fwrite(buffer, 1, bufferSize, file);
	if (r == 0 && ferror(file))
		throw File::Error("Failed to write to file", getName(), strerror(errno));
	return r;
}

util::Str8 DefaultFileImpl::getDirname() const {
	util::Str8 str = getName();
	int32 p = str.findLast('/');
	if (p >= 0)
		return str.substr(0, p);
	return ".";
}

std::unique_ptr<DefaultFileImpl> DefaultFileImpl::stat(const util::Str8& path, bool readOnly) {
	struct stat buf;
	if (::stat(path.cStr(), &buf) == 0 && S_ISREG(buf.st_mode))
		return std::unique_ptr<DefaultFileImpl>(new DefaultFileImpl(path, readOnly, buf.st_size, buf.st_mtime));

	return nullptr;
}

void DefaultFileImpl::open(int flags) {
	if ((flags & activeFlags) == flags)
		return;

	const char * mode = flags == Flag_Write ? "wb" : flags == Flag_Read ? "rb" : "r+b";
	FILE* tmp = fopen(getName().cStr(), mode);
	if (!tmp)
		throw File::Error("Failed to open file", getName(), strerror(errno));

	if (file) {
		/// @todo error checking
		long pos = ftell(file);
		fseek(tmp, pos, SEEK_SET);
		fclose(file);
	}
	file = tmp;
	activeFlags = flags;
}

} // global
} // clover
