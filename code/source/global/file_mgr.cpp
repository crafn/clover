#include "file_mgr.hpp"
#include "file_impl_default.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace global {

FileMgr* gFileMgr = nullptr;

FileMgr::FileMgr() {
	if (!gFileMgr)
		gFileMgr = this;
}

FileMgr::~FileMgr() {
	if (gFileMgr == this)
		gFileMgr = nullptr;
}

void FileMgr::add(const util::Str8& path, bool readOnly, int priority) {
	/// @todo detect if path is an archive
	/// @todo make path absolute
	util::Str8 p = path;
	p.replace('\\', '/');
	if (!p.empty() && p[p.length() - 1] != '/')
		p += '/';
	paths.insert({priority, {p, readOnly, nullptr}});
}

FileImplPtr FileMgr::findFile(const util::Str8& path, bool needWriteAccess) const {
	for (auto& it: paths) {
		const Path& p = it.second;
		const util::Str8 full = p.path + path;
		if (needWriteAccess && p.readOnly)
			continue;
		if (p.archive) {
			ensure_msg(false, "Not implemented");
		} else {
			FileImplPtr file = DefaultFileImpl::stat(full, p.readOnly);
			if (file)
				return file;
		}
	}
	return DefaultFileImpl::stat(path, true);
}

const util::Str8& FileMgr::getDefaultOutputPath() const {
	for (auto& it: paths) {
		const Path& p = it.second;
		if (p.readOnly || p.archive)
			continue;
		return p.path;
	}

	release_ensure_msg(false, "Couldn't find default output path");
	std::abort();
}

FileImplPtr FileMgr::create(const util::Str8& path) const {
	return FileImplPtr(new DefaultFileImpl(getDefaultOutputPath() + path, false, 0, 0));
}

} // global
} // clover
