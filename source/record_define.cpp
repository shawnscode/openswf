#include "debug.hpp"
#include "record.hpp"
#include "charactor.hpp"

#include <unordered_map>
#include <vector>
#include <memory>

extern "C" {
    #include "tesselator.h"
}

namespace openswf
{
namespace record
{
    // TAG: 2, 22, 32, 83 DEFINE SHAPE

    const uint32_t  MAX_CURVE_SUBDIVIDE = 20;
    const float     CURVE_TOLERANCE     = 1.0f;
    const uint32_t  MAX_POLYGON_SIZE    = 6;

    enum class FillStyleCode : uint8_t
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

    struct ShapeEdge
    {
        ShapeEdge(const Point2f& anchor)
            : control(anchor), anchor(anchor) {}

        ShapeEdge(int32_t cx, int32_t cy, int32_t ax, int32_t ay)
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

    static void read_gradient(Stream& stream, GradientFill* gradient, TagCode tag)
    {
        gradient->transform = stream.read_matrix();
        gradient->spread = (GradientFill::SpreadMode)stream.read_bits_as_uint32(2);
        gradient->interp = (GradientFill::InterpolationMode)stream.read_bits_as_uint32(2);

        auto count = stream.read_bits_as_uint32(4);
        assert( count > 0 );

        gradient->controls.reserve(count);
        for( auto i=0; i<count; i++ )
        {
            GradientFill::ControlPoint ctrl;
            ctrl.ratio = stream.read_uint8();
            if( tag == TagCode::DEFINE_SHAPE3 || tag == TagCode::DEFINE_SHAPE4 )
                ctrl.color = stream.read_rgba();
            else 
                ctrl.color = stream.read_rgb();
            gradient->controls.push_back(ctrl);
        }
    }

    static FillStyle* read_fill_style(Stream& stream, TagCode tag)
    {
        auto type = (FillStyleCode)stream.read_uint8();
        if( type == FillStyleCode::SOLID )
        {
            auto solid = new SolidFill();
            if( tag == TagCode::DEFINE_SHAPE3 || tag == TagCode::DEFINE_SHAPE4 )
                solid->color = stream.read_rgba();
            else 
                solid->color = stream.read_rgb();
            return solid;
        }
        else if( type == FillStyleCode::LINEAR_GRADIENT )
        {
            auto linear = new LinearGradientFill();
            read_gradient(stream, linear, tag);
            return linear;
        }
        else if( type == FillStyleCode::RADIAL_GRADIENT )
        {
            auto radial = new RadialGradientFill();
            read_gradient(stream, radial, tag);
            return radial;
        }
        else if( type == FillStyleCode::FOCAL_RADIAL_GRADIENT )
        {
            auto focal = new FocalRadialGradientFill();
            read_gradient(stream, focal, tag);
            focal->focal = stream.read_fixed16();
            return focal;
        }
            assert(false); // not supported yet
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
                    line->fill = FillPtr(read_fill_style(stream, type));
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

    static void read_fill_styles(Stream& stream, std::vector<FillPtr>& array, TagCode tag)
    {
        uint8_t count = stream.read_uint8();
        if( count == 0xFF ) count = stream.read_uint16();

        array.reserve(count + array.size());
        for( auto i=0; i<count; i++ )
            array.push_back(std::unique_ptr<FillStyle>(read_fill_style(stream, tag)));
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

        if( !contour_merge_segments(contours, segments) )
            contours.push_back(std::move(segments));
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

        std::vector<FillPtr>    fill_styles;
        std::vector<LinePtr>    line_styles;
        std::vector<ShapePath>  paths;

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
                    auto cx     = (float)stream.read_bits_as_int32(bits);
                    auto cy     = (float)stream.read_bits_as_int32(bits);
                    auto ax     = (float)stream.read_bits_as_int32(bits);
                    auto ay     = (float)stream.read_bits_as_int32(bits);

                    current_path.edges.push_back(ShapeEdge(cx, cy, ax, ay));
                    cursor.x = ax;
                    cursor.y = ay;
                }
            }
        }

        // tesselate polygons into simple triangles
        auto polygons = std::vector<Contours>(fill_styles.size(), Contours());
        auto lines = std::vector<Contours>(line_styles.size(), Contours());

        for( auto& path : paths )
        {
            assert( path.edges.size() != 0 );
            
            if( path.left_fill > 0 )
                contour_push_path( polygons[path.left_fill-1], path );

            if( path.right_fill > 0 )
                contour_push_path( polygons[path.right_fill-1], path );

            if( path.line > 0 )
                contour_push_path( lines[path.line-1], path );
        }

        // clean polygons with nothing
        for( int i=polygons.size()-1; i>=0; i-- )
        {
            if( polygons[i].size() == 0 )
            {
                polygons.erase(polygons.begin()+i);
                fill_styles.erase(fill_styles.begin()+i);
            }
        }

        for( int i=lines.size()-1; i>=0; i-- )
        {
            if( lines[i].size() == 0 )
            {
                lines.erase(lines.begin()+i);
                line_styles.erase(line_styles.begin()+i);
            }
        }

        assert( polygons.size() == fill_styles.size() );
        assert( lines.size() == line_styles.size() );

        std::vector<Point2f>    vertices;
        std::vector<uint16_t>   indices, contour_indices;

        for( auto& mesh_set : polygons )
        {
            assert( mesh_set.size() != 0 );

            auto tess = tessNewTess(nullptr);
            if( !tess ) return nullptr;

            for( auto& mesh : mesh_set )
                tessAddContour(tess, 2, &mesh[0], sizeof(Point2f), mesh.size());

            if( !tessTesselate(tess, TESS_WINDING_NONZERO, TESS_POLYGONS, MAX_POLYGON_SIZE, 2, 0) )
            {
                tessDeleteTess(tess);
                return nullptr;
            }

            const TESSreal* tess_vertices = tessGetVertices(tess);
            const TESSindex vcount = tessGetVertexCount(tess);
            const TESSindex nelems = tessGetElementCount(tess);
            const TESSindex* elems = tessGetElements(tess);

            auto vert_base_size = vertices.size() >> 1;
            vertices.reserve((vert_base_size+vcount) << 1);
            for( int i=0; i<vcount; i++ )
            {
                auto position = Point2f(tess_vertices[i*2], tess_vertices[i*2+1]).to_pixel();
                vertices.push_back( position );   // position

                auto texcoord = fill_styles[contour_indices.size()]->get_texcoord(position);
                vertices.push_back( texcoord );
            }

            auto ind_base_size = indices.size();
            indices.reserve(ind_base_size+nelems*(MAX_POLYGON_SIZE-2)*3);
            for( int i=0; i<nelems; i++ )
            {
                const int* p = &elems[i*MAX_POLYGON_SIZE];
                assert(p[0] != TESS_UNDEF && p[1] != TESS_UNDEF && p[2] != TESS_UNDEF);

                // triangle fans
                for( int j=2; j<MAX_POLYGON_SIZE && p[j] != TESS_UNDEF; j++ )
                {
                    indices.push_back(vert_base_size + p[0]);
                    indices.push_back(vert_base_size + p[j-1]);
                    indices.push_back(vert_base_size + p[j]);
                }
            }

            tessDeleteTess(tess);
            contour_indices.push_back( indices.size() );
        }

        assert( polygons.size() == contour_indices.size() );
        assert( contour_indices.back() == indices.size() );

        return Shape::create(character_id, bounds, fill_styles, line_styles, vertices, indices, contour_indices);
    }
}
}