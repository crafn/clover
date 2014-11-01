#ifndef CLOVER_GLOBAL_FILE_HPP
#define CLOVER_GLOBAL_FILE_HPP

#include "build.hpp"
#include "global/exception.hpp"
#include "file_impl.hpp"

namespace clover {
namespace global {

/// Synchronous file operations class. File is either a local file or inside a larger
/// archive file. Same File instance can be used for both writing and reading, but in
/// some cases reading and writing can actually use a different file on the filesystem.
/// That happens if the original file opened for reading is in a write-only location.
class File {
public:
	class Error : public global::Exception {
	public:
		Error(const util::Str8& message, const util::Str8& file, const util::Str8& error);
	};

public:
	/// Constructs a new File-object with given name.
	/// @param name relative or absolute path to a file, or path to a file inside
	///             a resource file. Relative paths go though resource lookup.
	///             Use '/' as directory separator on all platforms.
	/// This function doesn't throw Error
	File(const util::Str8& name);
	~File();

	void remove();

	/// Name that was given in the constructor, for example "images/monster.png"
	const util::Str8& getName() const { return name; }

	/// Name with full path. If this is a file in a local filesystem, returns
	/// something like: "/home/me/clover/images/monster.png". If this is a file
	/// inside a resource file, returns the resource name and the path inside
	/// the resource, example "/home/me/clover/gameresource.zip/images/monster.png"
	const util::Str8& getAbsoluteName() const;

	/// Returns a full path to the directory where the file exists, for example
    /// "/home/me/clover/images" or "/home/me/clover/gameresource.zip/images"
	util::Str8 getAbsoluteDirname() const;

	/// File size in bytes
	uint64 getSize() const;

	/// Normally functions like size() and timeModified() return cached values,
	/// this invalidates the cache and also reopens the file
	void reload();

	/// Reads the whole file as a text, converts all newlines to '\n'
	/// @todo Throw an Error if file doesn't exist
	util::Str8 readText();

	util::DynArray<uint8> readAll();

	/// Reads exactly the number of bytes asked, throws an exception on error or unexpected EOF
	void read(void* buffer, std::size_t bytesToRead);

	/// Reads max bytesToRead bytes from the file. Returns 0 on EOF
	/// (assuming bytesToRead > 0) and throws an exception on error
	std::size_t readSome(void* buffer, std::size_t bytesToRead);

	void write(const void* buffer, std::size_t bytesToWrite);

	static util::Str8 readText(const util::Str8& name) { return File(name).readText(); }
	static util::DynArray<uint8> readAll(const util::Str8& name){ return File(name).readAll(); }
	static util::Str8 extension(const util::Str8& name);

private:
	void openForWriting();

private:
	FileImplPtr impl;
	util::Str8 name;
};

} // global
} // clover

#endif // CLOVER_GLOBAL_FILE_HPP
