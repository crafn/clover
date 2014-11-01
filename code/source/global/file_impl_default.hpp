#ifndef CLOVER_GLOBAL_FILE_IMPL_DEFAULT_HPP
#define CLOVER_GLOBAL_FILE_IMPL_DEFAULT_HPP

#include "file_impl.hpp"

namespace clover {
namespace global {

class DefaultFileImpl : public FileImpl {
public:
	DefaultFileImpl(const util::Str8& fullPath, bool readOnly, uint64 size, time_t mtime);
	~DefaultFileImpl();

	virtual void remove() override;
	virtual uint64 getSize() const override;

	virtual std::size_t readSome(void* buffer, std::size_t bufferSize) override;

	virtual std::size_t writeSome(const void* buffer, std::size_t bufferSize) override;

	virtual util::Str8 getDirname() const override;
	
	static std::unique_ptr<DefaultFileImpl> stat(const util::Str8& path, bool readOnly);

private:
	enum OpenFlags {
		Flag_Read  = 1 << 0,
		Flag_Write = 1 << 1
	};
	void open(int flags);

private:
	int activeFlags;
	FILE* file;
	uint64 size;
	time_t mtime;
};

} // global
} // clover

#endif // CLOVER_GLOBAL_FILE_IMPL_DEFAULT_HPP
