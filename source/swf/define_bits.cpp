#include "swf/parser.hpp"
#include "image.hpp"
#include "stream.hpp"

extern "C" {
    #include "zlib.h"
    #include "jpeglib.h"
}

namespace openswf
{
    /// TAG = 20， 36
    enum class BitmapFormat : uint8_t
    {
        COLOR_MAPPED = 3,
        RGB15 = 4,
        RGB24 = 5
    };

    static void decompress(const uint8_t* source, int src_size, uint8_t* dst, int dst_size)
    {
        z_stream strm;

        /* allocate inflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;

        if( Z_OK != inflateInit(&strm) )
        {
            printf("deflateInit failed!\n");
            return;
        }

        /* decompress until deflate stream ends or end of file */
        strm.next_in = (uint8_t*)source;
        strm.avail_in = src_size;

        strm.next_out = dst;
        strm.avail_out = dst_size;

        assert( inflate(&strm, Z_NO_FLUSH) != Z_STREAM_ERROR );
        inflateEnd(&strm);
    }

    static BitmapPtr create_jpeg(const uint8_t* source, int jpeg_size, int alpha_size)
    {
        struct jpeg_decompress_struct jds;
        struct jpeg_error_mgr jem;
        JSAMPARRAY buffer;

        jds.err = jpeg_std_error(&jem);
        jpeg_create_decompress(&jds);
        jpeg_mem_src(&jds, (unsigned char*)source, (unsigned long)jpeg_size);

        (void)jpeg_read_header(&jds, TRUE);
        jpeg_start_decompress(&jds);

        auto width  = jds.output_width;
        auto height = jds.output_height;
        auto depth  = jds.output_components;

        buffer = (*jds.mem->alloc_sarray)((j_common_ptr)&jds, JPOOL_IMAGE, width*depth, 1);
        
        if( alpha_size <= 0 )
        {
            auto bitmap = BitmapRGB8::create(width, height);
            uint8_t* iterator = bitmap->get_ptr();
            while( jds.output_scanline < height )
            {
                (void)jpeg_read_scanlines(&jds, buffer, 1);
                memcpy(iterator, *buffer, width*depth);
                iterator += width*depth;
            }

            jpeg_finish_decompress(&jds);
            jpeg_destroy_decompress(&jds);
            return std::move(bitmap);
        }

        auto bitmap = BitmapRGBA8::create(width, height);
        auto iterator = bitmap->get_ptr();
        while( jds.output_scanline < height )
        {
            (void)jpeg_read_scanlines(&jds, buffer, 1);
            for( auto i=0; i < width; i++ )
            {
                memcpy(iterator, *buffer+i*3, 3);
                iterator += 4;
            }
        }

        auto bytes = new uint8_t[width*height];
        decompress(source+jpeg_size, alpha_size, bytes, width*height);

        for( int i=0; i<height; i++ )
            for( int j=0; j<width; j++ )
                bitmap->get(i, j).a = bytes[i*width+j];

        delete[] bytes;

        jpeg_finish_decompress(&jds);
        jpeg_destroy_decompress(&jds);
        return std::move(bitmap);
    }

    static Image* create_image(Stream& stream, TagHeader& header, uint16_t cid, uint32_t size)
    {
        auto start_pos = stream.get_position();
        auto byte1 = stream.read_uint8();
        if( byte1 == 0xFF )
        {
            auto byte2 = stream.read_uint8();

            // erroneous bytes before the jpeg soi marker for version before swf 8
            if( byte2 == 0xD9 )
            {
                assert( stream.read_uint8() == 0xFF && stream.read_uint8() == 0xD8 );
                start_pos = stream.get_position();
                assert( stream.read_uint8() == 0xFF && stream.read_uint8() == 0xD8 );
            }
            else
                assert( byte2 == 0xD8 );

            stream.set_position(start_pos);
            auto alpha = header.end_pos - start_pos - size;
            auto bitmap = create_jpeg(stream.get_current_ptr(), size, alpha);
            return Image::create(cid, std::move(bitmap));
        }
        else if( byte1 == 0x89 )
        {
            assert(
                stream.read_uint8() == 0x50 &&
                stream.read_uint8() == 0x4E &&
                stream.read_uint8() == 0x47 &&
                stream.read_uint8() == 0x0D &&
                stream.read_uint8() == 0x0A &&
                stream.read_uint8() == 0x1A &&
                stream.read_uint8() == 0x0A );
            // load png
        }
        else if( byte1 == 0x47 )
        {
            assert(
                stream.read_uint8() == 0x49 &&
                stream.read_uint8() == 0x46 &&
                stream.read_uint8() == 0x38 &&
                stream.read_uint8() == 0x39 &&
                stream.read_uint8() == 0x61 );
            // load non-animated GIF89a
        }

        return nullptr;
    }

    void Parser::DefineBitsJPEG2(Environment& env)
    {
        auto cid = env.stream.read_uint16();
        auto size = env.tag.end_pos - env.stream.get_position();
        auto image = create_image(env.stream, env.tag, cid, size);
        if( image != nullptr )
            env.player.set_character(image->get_character_id(), image);
    }

    void Parser::DefineBitsJPEG3(Environment& env)
    {
        auto cid = env.stream.read_uint16();
        auto size = env.stream.read_uint32();
        auto image = create_image(env.stream, env.tag, cid, size);
        if( image != nullptr )
            env.player.set_character(image->get_character_id(), image);
    }

    static Image* create_bits_lossless(Stream& stream, TagHeader& header)
    {
        auto cid = stream.read_uint16();
        auto format = (BitmapFormat)stream.read_uint8();
        auto width = stream.read_uint16();
        auto height = stream.read_uint16();

        if( format == BitmapFormat::COLOR_MAPPED )
        {
            auto table_size = stream.read_uint8();
            auto src_size = header.end_pos - stream.get_position();
            auto dst_size = table_size*3+width*height; // color table + indices
            auto bytes = BytesPtr(new (std::nothrow) uint8_t[dst_size]);
            decompress(stream.get_current_ptr(), src_size, bytes.get(), dst_size);

            auto bitmap = BitmapRGB8::create(width, height);
            for( auto i=0; i<height; i++ )
                for( auto j=0; j<width; j++ )
                {
                    auto base = bytes[table_size*3+i*width+j]*3;
                    bitmap->set(i, j, bytes.get()+base);
                }

            return Image::create(cid, std::move(bitmap));
        }
        else if( format == BitmapFormat::RGB15)
        {
            auto src_size = header.end_pos - stream.get_position();
            auto bitmap = BitmapRGB565::create(width, height);
            decompress(stream.get_current_ptr(), src_size, bitmap->get_ptr(), bitmap->get_size());

            for( auto i=0; i<height; i++ )
                for( auto j=0; j<width; j++ )
                    bitmap->get(i, j).cast_from_1555();

            return Image::create(cid, std::move(bitmap));
        }
        else if( format == BitmapFormat::RGB24 )
        {
            auto src_size = header.end_pos - stream.get_position();
            auto bitmap = BitmapRGB8::create(width, height);
            decompress(stream.get_current_ptr(), src_size, bitmap->get_ptr(), bitmap->get_size());

            return Image::create(cid, std::move(bitmap));
        }

        return nullptr;
    }

    void Parser::DefineBitsLossless(Environment& env)
    {
        auto image = create_bits_lossless(env.stream, env.tag);
        if( image != nullptr )
            env.player.set_character(image->get_character_id(), image);
    }

    static Image* create_bits_lossless2(Stream& stream, TagHeader& header)
    {
        auto cid = stream.read_uint16();
        auto format = (BitmapFormat)stream.read_uint8();
        auto width = stream.read_uint16();
        auto height = stream.read_uint16();

        if( format == BitmapFormat::COLOR_MAPPED )
        {
            auto table_size = stream.read_uint8();
            auto src_size = header.end_pos - stream.get_position();
            auto dst_size = table_size*4+width*height; // color table + indices
            auto bytes = BytesPtr(new (std::nothrow) uint8_t[dst_size]);
            decompress(stream.get_current_ptr(), src_size, bytes.get(), dst_size);

            auto bitmap = BitmapRGBA8::create(width, height);
            for( auto i=0; i<height; i++ )
                for( auto j=0; j<width; j++ )
                {
                    auto base = bytes[table_size*4+i*width+j]*4;
                    bitmap->set(i, j, bytes.get()+base);
                }

            return Image::create(cid, std::move(bitmap));
        }
        else if( format == BitmapFormat::RGB15 || format == BitmapFormat::RGB24 )
        {
            auto src_size = header.end_pos - stream.get_position();
            auto bitmap = BitmapRGBA8::create(width, height);
            decompress(stream.get_current_ptr(), src_size, bitmap->get_ptr(), bitmap->get_size());

            for( int i=0; i<height; i++ )
                for( int j=0; j<width; j++ )
                    bitmap->get(i, j).cast_from_argb();

            return Image::create(cid, std::move(bitmap));
        }

        return nullptr;
    }

    void Parser::DefineBitsLossless2(Environment& env)
    {
        auto image = create_bits_lossless2(env.stream, env.tag);
        if( image != nullptr )
            env.player.set_character(image->get_character_id(), image);
    }
}