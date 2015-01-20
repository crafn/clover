#include "debug/print.hpp"
#include "image.hpp"
#include "resources/exception.hpp"
#include "global/file.hpp"

#define PNG_SKIP_SETJMP_CHECK
#include <chrono>
#include <fstream>
#include <png.h>
#include <thread>

namespace clover {
namespace visual {

Image::Image()
		: asyncLoading(false)
		, loaded(false)
		, asyncException(nullptr){
}

Image::Image(RawImage&& raw)
		: image(std::move(raw))
		, asyncLoading(false)
		, loaded(true)
		, asyncException(nullptr){
}

Image::~Image(){
	if (asyncLoading){
		print(debug::Ch::Resources, debug::Vb::Critical, "Dude, you're destroying Image while loading it");
	}
	
	if (asyncThread){
		asyncThread->join();
	}
}

void Image::loadAsync(const util::Str8& path){
	ensure(!asyncLoading && !loaded);
	if (asyncThread)
		asyncThread->join();
	asyncException= nullptr;
	asyncThread= util::UniquePtr<std::thread>(new std::thread(std::bind(&Image::asyncLoadingThread, this, path)));
}

bool Image::isLoaded() const {
	if (loaded && asyncException){
		// Nothing going on threads, throw some exceptions
		std::rethrow_exception(asyncException);
	}
	return loaded;
}

void Image::load(const util::Str8& path){
	ensure(!asyncLoading && !loaded);
	image= std::move(loadImpl(path));
	loaded= true;
}

void Image::clear(){
	if (asyncLoading){
		asyncThread->join();
		asyncThread.reset();
	}

	image= RawImage();
	asyncLoading= false;
	loaded= false;
}

Image Image::cheapShrinkedToHalf() const {
	RawImage dest;
	const RawImage& src= image;

	dest.size= util::Vec2i { src.size.x/2, src.size.y/2 };
	dest.colorFormat= src.colorFormat;

	if (dest.size.x < 1) dest.size.x= 1;
	if (dest.size.y < 1) dest.size.y= 1;

	int32 src_data_size= src.sizeBytes();
	int32 dest_data_size= dest.sizeBytes();

	dest.data.resize(dest_data_size);

	int32 comp_count= 0;
	if (dest.colorFormat == hardware::GlState::TexFormat::Rgba8) comp_count= 4;
	else if (dest.colorFormat == hardware::GlState::TexFormat::Rgb8) comp_count= 3;
	else ensure_msg(0, "Not implemented");

	ensure(dest.size.x > 0 && dest.size.y > 0);

	int32 src_row_bytes= src.bytesForRow();
	int32 dest_row_bytes= dest.bytesForRow();

	for (int32 y=0; y<dest.size.y; ++y){
		for (int32 x=0; x<dest.size.x; ++x){
			ensure( x*comp_count + y*dest_row_bytes < dest_data_size );

			int32 dest_color_i= x*comp_count + y*dest_row_bytes;


			int32 src_color_i[4] = {
				x*2*comp_count + y*2*src_row_bytes, // Lower left
				x*2*comp_count + (y*2+1)*src_row_bytes, // Upper left
				(x*2 + 1)*comp_count + (y*2+1)*src_row_bytes, // Upper right
				(x*2 + 1)*comp_count + y*2*src_row_bytes // Lower right
			};


			// Don't read outside source image
			bool right_edge= x*2+1 >= src.size.x;
			bool top_edge= y*2+1 >= src.size.y;

			if (right_edge && !top_edge){
				// Right edge
				src_color_i[2]= src_color_i[1];
				src_color_i[3]= src_color_i[0];
			}
			else if (top_edge && !right_edge){
				// Top edge
				src_color_i[1]= src_color_i[0];
				src_color_i[2]= src_color_i[3];
			}
			else if (top_edge && right_edge){
				// Upper right corner
				src_color_i[1]= src_color_i[0];
				src_color_i[2]= src_color_i[0];
				src_color_i[3]= src_color_i[0];
			}

			ensure(	src_color_i[0] + comp_count <= src_data_size &&
					src_color_i[1] + comp_count <= src_data_size &&
					src_color_i[2] + comp_count <= src_data_size &&
					src_color_i[3] + comp_count <= src_data_size);



			for (int32 c=0; c<comp_count; ++c){

				// Average
				int32 color_sum= 0;
				for (int32 pix_i=0; pix_i<4; ++pix_i)
					color_sum += src.data[src_color_i[pix_i] + c];

				dest.data[dest_color_i + c]= color_sum/4;
			}
		}
	}
	
	return Image(std::move(dest));
}

uint32 Image::RawImage::sizeBytes() const {
	return bytesForRow()*size.y;
}

uint32 Image::RawImage::bytesForRow() const {
	if (colorFormat == hardware::GlState::TexFormat::Rgb8){
		if ((size.x*3)%4 == 0)
			return size.x*3; // No need for dummy bytes to make 4-byte alignment work
		else
			return size.x*3 + (4 - (size.x*3)%4); // Extra bytes
	}

	return size.x*4;
}

Image::RawImage Image::loadImpl(const util::Str8& path){
	// Only png is supported
	return (loadPngImpl(path));
}

void Image::asyncLoadingThread(const util::Str8& path){
	asyncLoading= true;
	try {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		image= std::move(loadImpl(path));
	}
	catch (...){
		asyncException= std::current_exception();
	}

	asyncLoading= false;
	loaded= true;
}

Image::RawImage Image::loadPngImpl(const util::Str8& path){
	try {
		//header for testing if it is a png
		png_byte header[8];

		global::File file(path);

		//read the header
		file.read(header, sizeof(header));

		//test if png
		int32 is_png = !png_sig_cmp(header, 0, 8);
		if (!is_png)
			throw resources::ResourceException("File %s is not png", file.getAbsoluteName().cStr());

		//create png struct
		png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
		NULL, NULL);
		if (!png_ptr)
			throw resources::ResourceException("Couldn't create png struct (while loading file %s)", file.getAbsoluteName().cStr());

		//create png info struct
		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
			throw resources::ResourceException("Couldn't create png info struct (while loading file %s)", file.getAbsoluteName().cStr());
		}

		//create png info struct
		png_infop end_info = png_create_info_struct(png_ptr);
		if (!end_info) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
			throw resources::ResourceException("Couldn't create png end info struct (while loading file %s)", file.getAbsoluteName().cStr());
		}

		//png error stuff, not sure libpng man suggests this.
		if (setjmp(png_jmpbuf(png_ptr))) {
			png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
			throw resources::ResourceException("\"Png error stuff\" (while loading file %s)", file.getAbsoluteName().cStr());
		}

		//init png reading
		png_set_read_fn(png_ptr, &file, [] (png_structp png_ptr, png_bytep data, png_size_t bytes) {
			try {
				global::File& file = *static_cast<global::File*>(png_get_io_ptr(png_ptr));
				file.read(data, bytes);
			}
			catch (const global::File::Error&){
				png_error(png_ptr, "Png reading failed");
			}
		});

		//let libpng know you already read the first 8 bytes
		png_set_sig_bytes(png_ptr, 8);

		// read all the info up to the image data
		png_read_info(png_ptr, info_ptr);

		//variables to pass to get info
		int32 bit_depth, color_type;
		png_uint_32 twidth, theight;

		// get info about png
		if (!png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type,
				NULL, NULL, NULL)){
			throw resources::ResourceException("Couldn't read png file %s", file.getAbsoluteName().cStr());
		}

		//update width and height based on png info
		int32 width = twidth;
		int32 height = theight;

		// Update the png info struct.
		png_read_update_info(png_ptr, info_ptr);

		// Row size in bytes.
		int32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);

		// Allocate the image_data as a big block, to be given to opengl
		util::DynArray<png_byte> image_data(rowbytes * height);

		//row_pointers is for pointing to image_data for reading the png with libpng
		std::vector<png_bytep> row_pointers(height);
		// set the individual row_pointers to point32 at the correct offsets of image_data
		for (int32 i = 0; i < height; ++i)
			row_pointers[height - 1 - i] = &image_data[0] + i * rowbytes;

		//read the png into image_data through row_pointers
		png_read_image(png_ptr, row_pointers.data());

		//clean up memory and close stuff
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

		hardware::GlState::TexFormat internal_format;

		if (color_type == PNG_COLOR_TYPE_RGB){
			internal_format= hardware::GlState::TexFormat::Rgb8;
		}
		else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA){
			internal_format= hardware::GlState::TexFormat::Rgba8;
		}
		else {
			release_ensure_msg(0, "util::Color type not supported for texture %s", path.cStr());
		}

		RawImage ret= { std::move(image_data), util::Vec2i{width, height}, internal_format };
		return ret;
	}
	catch (const global::File::Error&){
		throw resources::ResourceException("Image::loadPngImpl(..): Loading failed for %s", path.cStr());
	}
}

} // visual
} // clover
