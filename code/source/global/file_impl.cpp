#include "file_impl.hpp"

namespace clover {
namespace global {

FileImpl::FileImpl(const util::Str8& name, bool readOnly)
	: name(name), read_only(readOnly)
{}

FileImpl::~FileImpl()
{
}

} // global
} // clover