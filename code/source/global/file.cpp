#include "global/env.hpp"
#include "file.hpp"
#include "file_mgr.hpp"
#include "file_impl_default.hpp"
#include "util/ensure.hpp"

#include <boost/filesystem.hpp>

namespace clover {
namespace global {

File::Error::Error(	const util::Str8& message,
					const util::Str8& file,
					const util::Str8& error)
	: Exception("%s, %s, %s",
				message.cStr(),
				file.cStr(),
				error.cStr()) {
}

File::File(const util::Str8& name)
	: impl(global::g_env.fileMgr->findFile(name, false)),
	  name(name) {
	/// @todo make full path
	if (!impl)
		impl = global::g_env.fileMgr->create(name);
}

File::~File() {
}

void File::remove() {
	if (!impl->isReadOnly())
		impl->remove();
}

const util::Str8& File::getAbsoluteName() const {
	return impl->getName();
}

util::Str8 File::getAbsoluteDirname() const {
	return impl->getDirname();
}

uint64 File::getSize() const {
	return impl->getSize();
}

void File::reload() {
	File tmp(getAbsoluteName());
	std::swap(impl, tmp.impl);
}

util::Str8 File::readText() {
	std::string txt;
	txt.resize(getSize());
	read(&txt[0], txt.size());
	return util::Str8(txt).toUnixNewlines();
}

util::DynArray<uint8> File::readAll() {
	util::DynArray<uint8> out;
	out.resize(getSize());
	read(out.data(), out.size());
	return out;
}

void File::read(void* buffer, std::size_t bytesToRead) {
	char* charBuffer = static_cast<char*>(buffer);
	while (bytesToRead > 0) {
		std::size_t r = impl->readSome(charBuffer, bytesToRead);
		if (r == 0)
			throw Error("Failed to read from file", impl->getName(), "EOF");
		bytesToRead -= r;
		charBuffer += r;
	}
}

std::size_t File::readSome(void* buffer, std::size_t bytesToRead) {
	return impl->readSome(buffer, bytesToRead);
}

void File::write(const void* buffer, std::size_t bytesToWrite) {
	openForWriting();
	const char* charBuffer = static_cast<const char*>(buffer);
	while (bytesToWrite > 0) {
		std::size_t r = impl->writeSome(charBuffer, bytesToWrite);
		bytesToWrite -= r;
		charBuffer += r;
	}
}

util::Str8 File::extension(const util::Str8& name) {
	return boost::filesystem::extension(name.cStr());	
}

void File::openForWriting() {
	if (impl->isReadOnly())
		impl = global::g_env.fileMgr->create(name);
	ensure(!impl->isReadOnly());
}

} // global
} // clover
