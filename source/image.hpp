#pragma once

#include "character.hpp"
#include "shader.hpp"

namespace openswf
{
    class IBitmap
    {
    public:
        virtual uint32_t        get_width() const = 0;
        virtual uint32_t        get_height() const = 0;
        virtual uint32_t        get_size() const = 0;
        virtual TextureFormat   get_format() const = 0;
        virtual const uint8_t*  get_ptr() const = 0;
        virtual uint8_t*        get_ptr() = 0;
    };

    typedef std::unique_ptr<IBitmap> BitmapPtr;

    template<typename T> class Bitmap : public IBitmap
    {
    protected:
        uint32_t m_width, m_height;
        BytesPtr m_source;

    public:
        typedef std::unique_ptr<Bitmap<T>> Ptr;
        
        static std::unique_ptr<Bitmap<T>> create(uint32_t width, uint32_t height)
        {
            auto bitmap = new (std::nothrow) Bitmap<T>();
            if( bitmap == nullptr ) return Ptr(bitmap);

            bitmap->m_width  = width;
            bitmap->m_height = height;
            bitmap->m_source = BytesPtr(new uint8_t[width*height*T::size]);
            if( bitmap->m_source == nullptr )
            {
                delete bitmap;
                return nullptr;
            }

            return Ptr(bitmap);
        }

        void set(int row, int col, T pixel)
        {
            assert( row>=0 && row<m_height && col>=0 && col<m_width );
            auto offset = (row*m_width+col)*T::size;
            pixel.write(get_ptr()+offset);
        }

        void set(int row, int col, uint8_t* source)
        {
            assert( row>=0 && row<m_height && col>=0 && col<m_width );
            auto offset = (row*m_width+col)*T::size;
            memcpy(get_ptr()+offset, source, T::size);
        }

        T& get(int row, int col)
        {
            assert( row>=0 && row<m_height && col>=0 && col<m_width );
            return *(T*)(get_ptr() + (row*m_width+col)*T::size);
        }

        uint32_t        get_width() const { return m_width; }
        uint32_t        get_height() const { return m_height; }
        uint32_t        get_size() const { return m_width*m_height*T::size; }
        TextureFormat   get_format() const { return T::format; }
        const uint8_t*  get_ptr() const { return m_source.get(); }
        uint8_t*        get_ptr() { return m_source.get(); }
    };

    struct PixelRGBA8
    {
        static const TextureFormat format = TextureFormat::RGBA8;
        static const int size = 4;

        uint8_t r, g, b, a;

        PixelRGBA8(const Color& color)
        : r(color.r), g(color.g), b(color.b), a(color.a) {}

        void write(uint8_t* source)
        {
            source[0] = r; source[1] = g; source[2] = b; source[3] = a;
        }

        void cast_from_argb()
        {
            PixelRGBA8 tmp = *this;
            a = tmp.r;
            r = tmp.g;
            g = tmp.b;
            b = tmp.a;
        }
    };

    typedef Bitmap<PixelRGBA8> BitmapRGBA8;

    struct PixelRGB8
    {
        static const TextureFormat format = TextureFormat::RGB8;
        static const int size = 3;
        uint8_t r, g, b;

        PixelRGB8(const Color& color)
        : r(color.r), g(color.g), b(color.b) {}

        void write(uint8_t* source)
        {
            source[0] = r; source[1] = g; source[2] = b;
        }
    };

    typedef Bitmap<PixelRGB8> BitmapRGB8;

    struct PixelRGB565
    {
        static const TextureFormat format = TextureFormat::RGB565;
        static const int size = 2;

        uint8_t r : 5;
        uint8_t g : 6;
        uint8_t b : 5;

        void write(uint8_t* source)
        {
            memcpy(source, this, 2);
        }

        void cast_from_1555()
        {
            r = r << 1 | (g & 0x20);
            g = g & 0x1F;
        }
    };

    typedef Bitmap<PixelRGB565> BitmapRGB565;

    struct PixelRGBA4
    {
        static const TextureFormat format = TextureFormat::RGBA4;
        static const int size = 2;

        uint8_t a : 4;
        uint8_t r : 4;
        uint8_t g : 4;
        uint8_t b : 4;

        void write(uint8_t* source)
        {
            memcpy(source, this, 2);
        }
    };

    typedef Bitmap<PixelRGBA4> BitmapRGBA4;

    struct PixelA8
    {
        static const TextureFormat format = TextureFormat::ALPHA8;
        static const int size = 1;

        uint8_t a;

        void write(uint8_t* source)
        {
            source[0] = a;
        }
    };

    typedef Bitmap<PixelA8> BitmapA8;

    // The SWF file format specification supports a variety of bitmap formats.
    // All bitmaps are compressed to reduce file size. Lossy compression,
    // best for imprecise images such as photographs, is provided by JPEG bitmaps;
    // lossless compression, best for precise images such as diagrams, icons,
    // or screen captures, is provided by ZLIB bitmaps. Both types of bitmaps
    // can optionally contain alpha channel (opacity) information.
    class Image : public ICharacter
    {
    protected:
        uint16_t    m_character_id;
        BitmapPtr   m_bitmap;
        Rid         m_rid;

    public:
        static Image* create(uint16_t cid, BitmapPtr data);
        bool initialize(uint16_t cid, BitmapPtr data);

        virtual INode*   create_instance();
        virtual uint16_t get_character_id() const;

        Rid             get_texture_rid();
        TextureFormat   get_texture_format() const;
        float           get_width() const;
        float           get_height() const;
    };

    class ImageNode : public INode
    {
    protected:
        Image*  m_bitmap;

    public:
        ImageNode(Player* env, Image* bitmap)
        : INode(env, bitmap), m_bitmap(bitmap) {}

        virtual void update(float dt);
        virtual void render(const Matrix& matrix, const ColorTransform& cxform);
    };

    // INLINE METHODS
    inline TextureFormat Image::get_texture_format() const
    {
        return m_bitmap->get_format();
    }

    inline float Image::get_width() const
    {
        return m_bitmap->get_width();
    }

    inline float Image::get_height() const
    {
        return m_bitmap->get_height();
    }
}