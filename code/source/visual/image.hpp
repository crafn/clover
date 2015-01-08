#ifndef CLOVER_VISUAL_IMAGE_HPP
#define CLOVER_VISUAL_IMAGE_HPP

#include "build.hpp"
#include "hardware/glstate.hpp"
#include "util/atomic.hpp"
#include "util/ensure.hpp"
#include "util/string.hpp"
#include "util/unique_ptr.hpp"

#include <thread>

namespace clover {
namespace visual {

/// Can load different kind of images, synchronously or asynchronously
/// Differs from Bitmap so that image can be in compressed format and can contain metadata
/// @todo Outsource asynchronous loading
class Image {
public:
	Image();
	Image(Image&&)= default;
	Image& operator=(Image&&)= default;

	Image(const Image&)= delete; // Too lazy to implement these
	Image& operator=(const Image&)= delete;

	virtual ~Image();
	
	/// @todo Remove and use std::future to load asynchronously
	void loadAsync(const util::Str8& path);
	bool isLoaded() const;

	void load(const util::Str8& path);

	void clear();

	hardware::GlState::TexFormat getColorFormat() const { ensure(loaded); return image.colorFormat; }
	hardware::GlState::Type getDataTypeFormat() const { ensure(loaded); return hardware::GlState::Type::Uint8; } /// @todo different formats
	util::Vec2i getDimensions() const { ensure(loaded); return image.size; }
	const uint8* getRawData() const { ensure(loaded); return &image.data[0]; }

	/// Manipulation

	/// Uses box-filtering
	Image cheapShrinkedToHalf() const;

private:
	struct RawImage {
		util::DynArray<uint8> data;
		util::Vec2i size;
		hardware::GlState::TexFormat colorFormat;

		RawImage()= default;
		RawImage(RawImage&&)= default;
		RawImage& operator=(RawImage&&)= default;

		uint32 sizeBytes() const;
		uint32 bytesForRow() const;
	};

	Image(RawImage&&);

	void asyncLoadingThread(const util::Str8& path);

	static RawImage loadImpl(const util::Str8& path);
	static RawImage loadPngImpl(const util::Str8& path);

	RawImage image;
	util::Atomic<bool> asyncLoading;
	util::Atomic<bool> loaded;

	util::UniquePtr<std::thread> asyncThread;
	std::exception_ptr asyncException;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_IMAGE_HPP
