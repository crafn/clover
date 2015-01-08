#ifndef CLOVER_GLOBAL_FILE_IMPL_HPP
#define CLOVER_GLOBAL_FILE_IMPL_HPP

#include "build.hpp"
#include "util/string.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace global {

class FileImpl {
public:
	FileImpl(const util::Str8& name, bool readOnly);
	virtual ~FileImpl();

	virtual void remove() = 0;
	virtual uint64 getSize() const = 0;

	virtual std::size_t readSome(void* buffer, std::size_t bufferSize) = 0;

	virtual std::size_t writeSome(const void* buffer, std::size_t bufferSize) = 0;

	const util::Str8& getName() const { return name; }
	virtual util::Str8 getDirname() const = 0;

	bool isReadOnly() const { return read_only; }

private:
	util::Str8 name;
	bool read_only;
};

typedef util::UniquePtr<FileImpl> FileImplPtr;

} // global
} // clover

#endif // CLOVER_GLOBAL_FILE_IMPL_HPP
