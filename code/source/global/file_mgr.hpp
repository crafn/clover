#ifndef CLOVER_GLOBAL_FILE_MGR_HPP
#define CLOVER_GLOBAL_FILE_MGR_HPP

#include "file_impl.hpp"
#include "util/str8.hpp"

#include <map>

namespace clover {
namespace global {

class FileArchive;

class FileMgr {
public:
	FileMgr();
	~FileMgr();

	void add(const util::Str8& path, bool readOnly, int priority);
	FileImplPtr findFile(const util::Str8& path, bool needWriteAccess) const;

	/// For any writing happening outside File class, this path is the
	/// default output resource directory root for writing new files
	const util::Str8& getDefaultOutputPath() const;

	FileImplPtr create(const util::Str8& path) const;


private:
	struct Path {
		util::Str8 path;
		bool readOnly;
		std::shared_ptr<FileArchive> archive;
	};

	std::multimap<int, Path, std::greater<int>> paths;
};

} // global
} // clover

#endif // CLOVER_GLOBAL_FILE_MGR_HPP
