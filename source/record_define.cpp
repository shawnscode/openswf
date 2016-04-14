#include "debug.hpp"
#include "record.hpp"
#include "character.hpp"
#include "bitmap.hpp"
#include "shape.hpp"

#include <unordered_map>
#include <vector>
#include <memory>

extern "C" {
    #include "zlib.h"
}

namespace openswf
{
namespace record
{
    // TAG: 2, 22, 32, 83 DEFINE SHAPE
    const uint32_t  MAX_CURVE_SUBDIVIDE = 10;
    const float     CURVE_TOLERANCE     = 4.f;

    enum class StyleMode : uint8_t
    {
        SOLID                           = 0x00,
        LINEAR_GRADIENT                 = 0x10,
        RADIAL_GRADIENT                 = 0x12,
        FOCAL_RADIAL_GRADIENT           = 0x13,

        REPEATING_BITMAP                = 0x40,
        CLIPPED_BITMAP                  = 0x41,
        NON_SMOOTHED_REPEATING_BITMAP   = 0x42,
        NON_SMOOTHED_CLIPPED_BITMAP     = 0x43
    };

    enum DefineShapeMask
    {
        SHAPE_END           = 0x00,
        SHAPE_MOVE_TO       = 0x01,
        SHAPE_FILL_STYLE_0  = 0x02,
        SHAPE_FILL_STYLE_1  = 0x04,
        SHAPE_LINE_STYLE    = 0x08,
        SHAPE_NEW_STYLE     = 0x10
    };

    enum class GradientSpreadMode : uint8_t
    {
        PAD         = 0,
        REFLECT     = 1,
        REPEAT      = 2,
        RESERVED    = 3
    };

    enum class GradientInterpolationMode : uint8_t
    {
        NORMAL      = 0,
        LINEAR      = 1,
        RESERVED_1  = 2,
        RESERVED_2  = 3
    };

    struct ShapeEdge
    {
        ShapeEdge(const Point2f& anchor)
            : control(anchor), anchor(anchor) {}

        ShapeEdge(int32_t ax, int32_t ay, int32_t cx, int32_t cy)
            : control(Point2f(cx, cy)), anchor(Point2f(ax, ay)){}

        Point2f control, anchor;
    };

    struct ShapePath
    {
        uint32_t                left_fill;
        uint32_t                right_fill;
        uint32_t                line;

        Point2f                 start;
        std::vector<ShapeEdge>  edges;

        ShapePath() : left_fill(0), right_fill(0), line(0) {}
        void reset()
        {
            left_fill = right_fill = 0;
            line = 0;
        }

        void restart(const Point2f& cursor)
        {
            start.x = cursor.x;
            start.y = cursor.y;
            edges.clear();
        }
    };

    struct GradientPoint
    {
        int     ratio;
        Color   color;

        GradientPoint(int ratio, Color color) : ratio(ratio), color(color) {}
        GradientPoint() : ratio(0) {}
    };

    const static int MaxGradientPoint = 15;
    struct Gradient
    {
        int             count;
        GradientPoint   controls[MaxGradientPoint];
        // GradientSpreadMode              spread;
        // GradientInterpolationMode       interp;

        static Gradient read(Stream& stream, TagCode tag)
        {
            Gradient gradient;

            stream.read_bits_as_uint32(2); // (GradientSpreadMode)
            stream.read_bits_as_uint32(2); // (GradientInterpolationMode)

            gradient.count = stream.read_bits_as_uint32(4);
            assert(gradient.count > 0 && gradient.count < MaxGradientPoint);
            for( auto i=0; i<gradient.count; i++ )
            {
                gradient.controls[i] = GradientPoint(
                    stream.read_uint8(),
                    (tag == TagCode::DEFINE_SHAPE3 || tag == TagCode::DEFINE_SHAPE4) ?
                        stream.read_rgba() : stream.read_rgb());
            }

            return gradient;
        }

        Color sample(int ratio) const
        {
            assert( ratio >= 0 && ratio < 256 );

            if( ratio < this->controls[0].ratio )
                return this->controls[0].color;

            for( auto i=1; i<this->count; i++ )
            {
                if( this->controls[i].ratio >= ratio )
                {
                    const auto& last = this->controls[i-1];
                    const auto& now = this->controls[i];

                    auto percent = 0.0f;
                    if( last.ratio != now.ratio )
                        percent = (float)(ratio - last.ratio) / (float)(now.ratio - last.ratio);

                    return Color::lerp(last.color, now.color, percent);
                }
            }

            return this->controls[this->count-1].color;
        }

        BitmapDataPtr create_bitmap_linear() const
        {
            static const int width = 64;
            static const int height = 1;

            auto source = BitmapData::create(TextureFormat::RGBA8, width, height);
            for( auto i=0; i<source->get_height(); i++ )
                for( auto j=0; j<source->get_width(); j++ )
                    source->set(i, j, sample(255.f*(float)j/(float)width).to_value());

            return std::move(source);
        }

        BitmapDataPtr create_bitmap_radial() const
        {
            static const int width = 16;
            static const int height = 16;

            auto source = BitmapData::create(TextureFormat::RGBA8, width, height);
            for( auto i=0; i<height; i++ )
            {
                for( auto j=0; j<width; j++ )
                {
                    float radius = (height - 1) / 2.0f;
                    float y = (j - radius) / radius;
                    float x = (i - radius) / radius;
                    int ratio = (int) floorf(255.5f * sqrt(x * x + y * y));
                    if( ratio > 255.f ) ratio = 255.f;
                    source->set(i, j, sample(ratio).to_value());
                }
            }

            return std::move(source);
        }
    };

    static ShapeFillPtr read_fill_style(Stream& stream, TagCode tag)
    {
        auto type = (StyleMode)stream.read_uint8();
        if( type == StyleMode::SOLID )
        {
            if( tag == TagCode::DEFINE_SHAPE3 || tag == TagCode::DEFINE_SHAPE4 )
                return ShapeFill::create(stream.read_rgba());
            else 
                return ShapeFill::create(stream.read_rgb());
        }
        else if( type == StyleMode::LINEAR_GRADIENT )
        {
            auto transform = stream.read_matrix().to_pixel();
            auto bitmap = Gradient::read(stream, tag).create_bitmap_linear();
            return ShapeFill::create(std::move(bitmap), transform);
        }
        else if( type == StyleMode::RADIAL_GRADIENT || type == StyleMode::FOCAL_RADIAL_GRADIENT )
        {
            auto transform = stream.read_matrix().to_pixel();
            auto bitmap = Gradient::read(stream, tag).create_bitmap_radial();
            return ShapeFill::create(std::move(bitmap), transform);
        }
        else if( type == StyleMode::REPEATING_BITMAP ||
            type == StyleMode::CLIPPED_BITMAP ||
            type == StyleMode::NON_SMOOTHED_REPEATING_BITMAP ||
            type == StyleMode::NON_SMOOTHED_CLIPPED_BITMAP )
        {
            auto cid = stream.read_uint16();
            auto transform = stream.read_matrix().to_pixel();
            return ShapeFill::create(cid, transform);
        }
        else
            assert(false);
    }

    static void read_line_styles(Stream& stream, std::vector<LinePtr>& array, TagCode type)
    {
        uint8_t count = stream.read_uint8();
        if( count == 0xFF ) count = stream.read_uint16();

        array.reserve(count + array.size());
        for( auto i=0; i<count; i++ )
        {
            LineStyle* line = new LineStyle();
            line->width = stream.read_uint16();

            if( type == TagCode::DEFINE_SHAPE4 )
            {   // line style 2
                line->start_cap  = (Capcode)stream.read_bits_as_uint32(2);
                line->join       = (Joincode)stream.read_bits_as_uint32(2);
                line->has_fill   = stream.read_bits_as_uint32(1) > 0;
                line->no_hscale  = stream.read_bits_as_uint32(1) > 0;
                line->no_vscale  = stream.read_bits_as_uint32(1) > 0;
                line->pixel_hinting = stream.read_bits_as_uint32(1) > 0;

                assert( stream.read_bits_as_uint32(5) == 0 ); //reserved bits
                
                line->no_close    = stream.read_bits_as_uint32(1) > 0;
                line->end_cap    = (Capcode)stream.read_bits_as_uint32(2);
                line->miter_limit_factor = line->join == Joincode::MITER ? stream.read_uint16() : 0;
                
                if( line->has_fill )
                    line->fill = read_fill_style(stream, type);
                else
                    line->color = stream.read_rgba();
            }
            else
            {   // line style
                if( type == TagCode::DEFINE_SHAPE3 )
                    line->color = stream.read_rgba();
                else
                    line->color = stream.read_rgb();
            }
            
            array.push_back(LinePtr(line));
        }
    }

    static void read_fill_styles(Stream& stream, std::vector<ShapeFillPtr>& array, TagCode tag)
    {
        uint8_t count = stream.read_uint8();
        if( count == 0xFF ) count = stream.read_uint16();

        array.reserve(count + array.size());
        for( auto i=0; i<count; i++ )
            array.push_back(read_fill_style(stream, tag));
    }

    typedef std::vector<Point2f>    Segments;
    typedef std::vector<Segments>   Contours;

    static void contour_add_curve(Segments& segments, const Point2f& prev, const Point2f& ctrl, const Point2f& next, int depth = 0)
    {
        Point2f mid = (prev + next) * 0.5f;
        Point2f q = (mid + ctrl) * 0.5f;

        float dist = std::abs((mid.x - q.x)) + std::abs(mid.y - q.y);
        if( dist < CURVE_TOLERANCE || depth >= MAX_CURVE_SUBDIVIDE )
            segments.push_back(next);
        else
        {
            // subdivide
            contour_add_curve(segments, prev, (prev + ctrl) * 0.5f, q, depth + 1);
            contour_add_curve(segments, q, (ctrl + next) * 0.5f, next, depth + 1);
        }
    }

    static bool contour_merge_segments(Contours& contours, Segments& next)
    {
        for( auto& segments : contours )
        {
            if(segments.front() == next.back())
            {
                next.pop_back();

                next.reserve(next.size() + segments.size());
                next.insert(next.end(), segments.begin(), segments.end());
                segments = std::move(next);
                return true;
            }
            else if(segments.back() == next.front())
            {
                segments.pop_back();

                segments.reserve(segments.size() + next.size());
                segments.insert(segments.end(), next.begin(), next.end());
                return true;
            }
            else if(segments.back() == next.back())
            {
                next.pop_back();
                std::reverse(next.begin(), next.end());

                segments.reserve(next.size() + segments.size());
                segments.insert(segments.end(), next.begin(), next.end());
                return true;
            }
            else if(segments.front() == next.front())
            {
                std::reverse(next.begin(), next.end());
                next.pop_back();

                next.reserve(next.size() + segments.size());
                next.insert(next.end(), segments.begin(), segments.end());
                segments = std::move(next);
                return true;
            }
        }
        return false;
    }

    static void contour_push_path(Contours& contours, const record::ShapePath& path)
    {
        Segments segments;
        segments.reserve(path.edges.size() + 1);
        segments.push_back(path.start);

        Point2f last = path.start;
        for( auto& edge : path.edges )
        {
            if( edge.control == edge.anchor )
                segments.push_back(edge.anchor);
            else
                contour_add_curve(segments, last, edge.control, edge.anchor);

            last = edge.anchor;
        }

        contours.push_back(std::move(segments));
        // if( !contour_merge_segments(contours, segments) )
    }

    Shape* DefineShape::create(Stream& stream, TagCode type)
    {
        assert( 
            type == TagCode::DEFINE_SHAPE ||
            type == TagCode::DEFINE_SHAPE2 ||
            type == TagCode::DEFINE_SHAPE3 ||
            type == TagCode::DEFINE_SHAPE4 );

        uint16_t character_id   = stream.read_uint16();
        Rect bounds             = stream.read_rect();
        Rect edge_bounds        = Rect();

        if( type == TagCode::DEFINE_SHAPE4 )
        {
            edge_bounds = stream.read_rect();
            stream.read_bits_as_uint32(5);
            stream.read_bits_as_uint32(1);
            stream.read_bits_as_uint32(1);
            stream.read_bits_as_uint32(1);
        }

        std::vector<ShapePath>      paths;
        std::vector<ShapeFillPtr>  fill_styles;
        std::vector<LinePtr>        line_styles;

        read_fill_styles(stream, fill_styles, type);
        read_line_styles(stream, line_styles, type);

        // parse shape records
        uint32_t fill_index_bits = stream.read_bits_as_uint32(4);
        uint32_t line_index_bits = stream.read_bits_as_uint32(4);
        uint32_t fill_index_base = 0, line_index_base = 0;
        Point2f cursor;

        ShapePath current_path;
        auto push_path = [&](bool reset = false)
        {
            if( !current_path.edges.empty() )
            {
                paths.push_back(current_path);
            }

            current_path.restart(cursor);
            if( reset ) current_path.reset();
        };

        // parse segments and appending styls of path
        while( true )
        {
            bool is_edge = stream.read_bits_as_uint32(1) > 0;
            if( !is_edge )
            {
                uint32_t mask = stream.read_bits_as_uint32(5);
                if( mask == SHAPE_END ) // EndShapeRecord
                {
                    push_path();
                    break;
                }

                // StyleChangeRecord
                if( mask & SHAPE_MOVE_TO ) // StateMoveTo
                {
                    uint8_t bits = stream.read_bits_as_uint32(5);
                    cursor.x = (float)stream.read_bits_as_int32(bits);
                    cursor.y = (float)stream.read_bits_as_int32(bits);

                    push_path(true);
                }

                if( (mask & SHAPE_FILL_STYLE_0) && fill_index_bits > 0 ) // StateFillStyle0
                {
                    push_path();

                    current_path.left_fill = stream.read_bits_as_uint32(fill_index_bits);
                    if( current_path.left_fill > 0 )
                        current_path.left_fill += fill_index_base;
                }

                if( (mask & SHAPE_FILL_STYLE_1) && fill_index_bits > 0 ) // StateFillStyle1
                {
                    push_path();
                    current_path.right_fill = stream.read_bits_as_uint32(fill_index_bits);
                    if( current_path.right_fill > 0 )
                        current_path.right_fill += fill_index_base;
                }

                if( (mask & SHAPE_LINE_STYLE) && line_index_bits > 0 ) // StateLineStyle
                {
                    push_path();
                    current_path.line = stream.read_bits_as_uint32(line_index_bits);
                }

                if( mask & SHAPE_NEW_STYLE ) // StateNewStyles, used by DefineShape2, DefineShape3 only.
                {
                    assert( type == TagCode::DEFINE_SHAPE3 || type == TagCode::DEFINE_SHAPE4 );
                    push_path();

                    fill_index_base = fill_styles.size();
                    line_index_base = line_styles.size();
                    read_fill_styles(stream, fill_styles, type);
                    read_line_styles(stream, line_styles, type);
                    fill_index_bits = stream.read_bits_as_uint32(4);
                    line_index_bits = stream.read_bits_as_uint32(4);
                }
            }
            else
            {
                bool is_straight = stream.read_bits_as_uint32(1) > 0;
                if( is_straight ) // StraightEdgeRecrod
                {
                    float dx = 0, dy = 0;
                    auto bits = stream.read_bits_as_uint32(4) + 2;
                    auto is_general = stream.read_bits_as_uint32(1) > 0;
                    if( is_general )
                    {
                        dx = (float)stream.read_bits_as_int32(bits);
                        dy = (float)stream.read_bits_as_int32(bits);
                    }
                    else
                    {
                        auto is_vertical = stream.read_bits_as_uint32(1) > 0;
                        if( is_vertical )
                            dy = (float)stream.read_bits_as_int32(bits);
                        else
                            dx = (float)stream.read_bits_as_int32(bits);
                    }

                    cursor.x += dx;
                    cursor.y += dy;

                    current_path.edges.push_back(ShapeEdge(cursor));
                }
                else // CurvedEdgeRecord
                {
                    auto bits   = stream.read_bits_as_uint32(4) + 2;
                    auto cx     = cursor.x + (float)stream.read_bits_as_int32(bits);
                    auto cy     = cursor.y + (float)stream.read_bits_as_int32(bits);
                    auto ax     = cx + (float)stream.read_bits_as_int32(bits);
                    auto ay     = cy + (float)stream.read_bits_as_int32(bits);

                    current_path.edges.push_back(ShapeEdge(ax, ay, cx, cy));
                    cursor.x = ax;
                    cursor.y = ay;
                }
            }
        }

        // tesselate polygons into simple triangles
        auto mesh_set = std::vector<Contours>(fill_styles.size(), Contours());
        // auto lines = std::vector<Contours>(line_styles.size(), Contours());

        for( auto& path : paths )
        {
            assert( path.edges.size() != 0 );
            
            if( path.left_fill > 0 )
                contour_push_path( mesh_set[path.left_fill-1], path );

            if( path.right_fill > 0 )
                contour_push_path( mesh_set[path.right_fill-1], path );

            // if( path.line > 0 )
            //     contour_push_path( lines[path.line-1], path );
        }

        int numv = 0;
        for( int i=mesh_set.size()-1; i>=0; i-- )
        {
            auto& mesh = mesh_set[i];
            if( mesh.size() == 0 )
            {
                fill_styles.erase(fill_styles.begin()+i);
                mesh_set.erase(mesh_set.begin()+i);
                continue;
            }

            for(;;)
            {
                if( mesh.size() == 1 ||
                    !contour_merge_segments(mesh, mesh.back()) )
                {
                    break;
                }
                mesh.pop_back();
            }

            assert(mesh.size() == 1);
            numv += mesh[0].size();
        }

        auto vertices = std::vector<Point2f>();
        auto contour_indices = std::vector<uint16_t>();

        vertices.reserve(numv);
        contour_indices.reserve(mesh_set.size());
        for( auto& mesh : mesh_set )
        {
            vertices.insert(vertices.end(), mesh[0].begin(), mesh[0].end());
            contour_indices.push_back(vertices.size());
        }

        assert( contour_indices.size() == fill_styles.size() );

        auto record = ShapeRecord::create(bounds, std::move(vertices), std::move(contour_indices));
        return Shape::create(character_id,
             std::move(fill_styles), std::move(line_styles), std::move(record));
    }

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

    Bitmap* DefineBitsLossless::create(Stream& stream, TagHeader& header)
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

            auto bitmap = BitmapData::create(TextureFormat::RGB8, width, height);
            for( auto i=0; i<height; i++ )
            {
                for( auto j=0; j<width; j++ )
                {
                    auto base = bytes[table_size*3+i*width+j]*3;
                    bitmap->set(i, j,
                        ((uint32_t)bytes[base+1]<<16) |
                        ((uint32_t)bytes[base+2]<< 8) |
                        ((uint32_t)bytes[base+3]<< 0) );
                }
            }

            return Bitmap::create(cid, std::move(bitmap));
        }
        else if( format == BitmapFormat::RGB15)
        {
            auto src_size = header.end_pos - stream.get_position();
            auto dst_size = width * height * 2;
            auto bytes = BytesPtr(new (std::nothrow) uint8_t[dst_size]);
            decompress(stream.get_current_ptr(), src_size, bytes.get(), dst_size);

            auto bitmap = BitmapData::create(TextureFormat::RGB565, width, height);
            for( auto i=0; i<height; i++ )
            {
                for( auto j=0; j<width; j++ )
                {
                    auto base = (i*width+j)*2;
                    bitmap->set(i, j,
                        ((uint32_t)bytes[base+0] << 8) |
                        ((uint32_t)bytes[base+1] << 0) );
                }
            }

            return Bitmap::create(cid, std::move(bitmap));
        }
        else if( format == BitmapFormat::RGB24 )
        {
            auto src_size = header.end_pos - stream.get_position();
            auto dst_size = width * height * 4;
            auto bytes = BytesPtr(new (std::nothrow) uint8_t[dst_size]);
            decompress(stream.get_current_ptr(), src_size, bytes.get(), dst_size);

            auto bitmap = BitmapData::create(TextureFormat::RGB8, width, height);
            for( auto i=0; i<height; i++ )
            {
                for( auto j=0; j<width; j++ )
                {
                    auto base = (i*width+j)*4;
                    bitmap->set(i, j,
                        ((uint32_t)bytes[base+1]<<16) |
                        ((uint32_t)bytes[base+2]<< 8) |
                        ((uint32_t)bytes[base+3]<< 0) );
                }
            }

            return Bitmap::create(cid, std::move(bitmap));
        }

        assert(false);
        return nullptr;
    }

    Bitmap* DefineBitsLossless2::create(Stream& stream, TagHeader& header)
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

            auto bitmap = BitmapData::create(TextureFormat::RGBA8, width, height);
            for( auto i=0; i<height; i++ )
            {
                for( auto j=0; j<width; j++ )
                {
                    auto base = bytes[table_size*4+i*width+j]*4;
                    bitmap->set(i, j,
                        ((uint32_t)bytes[base+0]<< 0) |
                        ((uint32_t)bytes[base+1]<<24) |
                        ((uint32_t)bytes[base+2]<<16) |
                        ((uint32_t)bytes[base+3]<< 8) );
                }
            }

            return Bitmap::create(cid, std::move(bitmap));
        }
        else if( format == BitmapFormat::RGB15 || format == BitmapFormat::RGB24 )
        {
            auto src_size = header.end_pos - stream.get_position();
            auto dst_size = width * height * 4;
            auto bytes = BytesPtr(new (std::nothrow) uint8_t[dst_size]);
            decompress(stream.get_current_ptr(), src_size, bytes.get(), dst_size);

            auto bitmap = BitmapData::create(TextureFormat::RGBA8, width, height);
            for( auto i=0; i<height; i++ )
            {
                for( auto j=0; j<width; j++ )
                {
                    auto base = (i*width+j)*4;
                    bitmap->set(i, j,
                        ((uint32_t)bytes[base+0]<< 0) |
                        ((uint32_t)bytes[base+1]<<24) |
                        ((uint32_t)bytes[base+2]<<16) |
                        ((uint32_t)bytes[base+3]<< 8) );
                }
            }

            return Bitmap::create(cid, std::move(bitmap));
        }

        assert(false);
        return nullptr;
    }
}
}