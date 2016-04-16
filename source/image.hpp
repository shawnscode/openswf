#pragma once

#include "character.hpp"
#include "shader.hpp"

namespace openswf
{
    class Bitmap;
    typedef std::unique_ptr<Bitmap> BitmapPtr;

    class Bitmap
    {
    protected:
        TextureFormat   m_format;
        uint8_t         m_elesize;
        uint32_t        m_width, m_height;
        BytesPtr        m_source;

    public:
        static BitmapPtr create(TextureFormat format, uint32_t width, uint32_t height);
        static BitmapPtr create(BytesPtr bytes, TextureFormat format, uint32_t width, uint32_t height);

        void set(int row, int col, uint32_t value);
        uint32_t get_width() const;
        uint32_t get_height() const;
        TextureFormat get_format() const;
        const uint8_t* get_ptr() const;
    };

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
    inline uint32_t Bitmap::get_width() const
    {
        return m_width;
    }

    inline uint32_t Bitmap::get_height() const
    {
        return m_height;
    }

    inline TextureFormat Bitmap::get_format() const
    {
        return m_format;
    }

    inline const uint8_t* Bitmap::get_ptr() const
    {
        return m_source.get();
    }

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