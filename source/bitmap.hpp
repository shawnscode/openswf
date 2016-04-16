#pragma once

#include "character.hpp"
#include "shader.hpp"

namespace openswf
{
    class BitmapData;
    typedef std::unique_ptr<BitmapData> BitmapDataPtr;

    class BitmapData
    {
    protected:
        TextureFormat   m_format;
        uint8_t         m_elesize;
        uint32_t        m_width, m_height;
        BytesPtr        m_source;

    public:
        static BitmapDataPtr create(TextureFormat format, uint32_t width, uint32_t height);
        static BitmapDataPtr create(BytesPtr bytes, TextureFormat format, uint32_t width, uint32_t height);

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
    class Bitmap : public ICharacter
    {
    protected:
        uint16_t        m_character_id;
        BitmapDataPtr   m_bitmap;
        Rid             m_rid;

    public:
        static Bitmap* create(uint16_t cid, BitmapDataPtr data);
        bool initialize(uint16_t cid, BitmapDataPtr data);

        virtual INode*   create_instance();
        virtual uint16_t get_character_id() const;

        Rid             get_texture_rid();
        TextureFormat   get_texture_format() const;
        float           get_width() const;
        float           get_height() const;
    };

    class BitmapNode : public INode
    {
    protected:
        Bitmap*  m_bitmap;

    public:
        BitmapNode(Player* env, Bitmap* bitmap)
        : INode(env, bitmap), m_bitmap(bitmap) {}

        virtual void update(float dt);
        virtual void render(const Matrix& matrix, const ColorTransform& cxform);
    };

    // INLINE METHODS
    inline uint32_t BitmapData::get_width() const
    {
        return m_width;
    }

    inline uint32_t BitmapData::get_height() const
    {
        return m_height;
    }

    inline TextureFormat BitmapData::get_format() const
    {
        return m_format;
    }

    inline const uint8_t* BitmapData::get_ptr() const
    {
        return m_source.get();
    }

    inline TextureFormat Bitmap::get_texture_format() const
    {
        return m_bitmap->get_format();
    }

    inline float Bitmap::get_width() const
    {
        return m_bitmap->get_width();
    }

    inline float Bitmap::get_height() const
    {
        return m_bitmap->get_height();
    }
}